// Copyright 2017 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#define ROUTER_SSID "" 
#define ROUTER_PASSWORD "" 
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

#include "archivo.c"
#include "mwifi.h"
#include "mdf_common.h"
// #define MEMORY_DEBUG
static esp_netif_t *netif_sta = NULL; 
static const char *TAG = "get_started";
static int g_sockfd = -1 ;


static int socket_tcp_client_create(const char *ip, uint16_t port)
{
    MDF_PARAM_CHECK(ip);

    MDF_LOGI("Create a tcp client, ip: %s, port: %d", ip, port);

    mdf_err_t ret = ESP_OK;
    int sockfd    = -1;
    struct sockaddr_in server_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(port),
        .sin_addr.s_addr = inet_addr(ip),
    };

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    MDF_ERROR_GOTO(sockfd < 0, ERR_EXIT, "socket create, sockfd: %d", sockfd);

    ret = connect(sockfd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_in));
    MDF_ERROR_GOTO(ret < 0, ERR_EXIT, "socket connect, ret: %d, ip: %s, port: %d",
                   ret, ip, port);
    return sockfd;

ERR_EXIT:

    if (sockfd != -1) {
        close(sockfd);
    }

    return -1;
}

void tcp_client_read_task(void *arg)
{
    mdf_err_t ret                     = MDF_OK;
    char *data                        = MDF_MALLOC(MWIFI_PAYLOAD_LEN);
    size_t size                       = MWIFI_PAYLOAD_LEN;
    uint8_t dest_addr[MWIFI_ADDR_LEN] = {0x0};
    mwifi_data_type_t data_type       = {0x0};
    cJSON *json_root                  = NULL;
    cJSON *json_addr                  = NULL;
    cJSON *json_group                 = NULL;
    cJSON *json_data                  = NULL;
    cJSON *json_dest_addr             = NULL;

    MDF_LOGI("TCP client read task is running");

    while (mwifi_is_connected()) {
        if (g_sockfd == -1) {
            g_sockfd = socket_tcp_client_create(CONFIG_SERVER_IP, CONFIG_SERVER_PORT);

            if (g_sockfd == -1) {
                vTaskDelay(500 / portTICK_RATE_MS);
                continue;
            }
        }

        memset(data, 0, MWIFI_PAYLOAD_LEN);
        ret = read(g_sockfd, data, size);
        MDF_LOGD("TCP read, %d, size: %d, data: %s", g_sockfd, size, data);

        if (ret <= 0) {
            MDF_LOGW("<%s> TCP read", strerror(errno));
            close(g_sockfd);
            g_sockfd = -1;
            continue;
        }

        json_root = cJSON_Parse(data);
        MDF_ERROR_CONTINUE(!json_root, "cJSON_Parse, data format error");

        /**
         * @brief Check if it is a group address. If it is a group address, data_type.group = true.
         */
        json_addr = cJSON_GetObjectItem(json_root, "dest_addr");
        json_group = cJSON_GetObjectItem(json_root, "group");

        if (json_addr) {
            data_type.group = false;
            json_dest_addr = json_addr;
        } else if (json_group) {
            data_type.group = true;
            json_dest_addr = json_group;
        } else {
            MDF_LOGW("Address not found");
            cJSON_Delete(json_root);
            continue;
        }

        /**
         * @brief  Convert mac from string format to binary
         */
        do {
            uint32_t mac_data[MWIFI_ADDR_LEN] = {0};
            sscanf(json_dest_addr->valuestring, MACSTR,
                   mac_data, mac_data + 1, mac_data + 2,
                   mac_data + 3, mac_data + 4, mac_data + 5);

            for (int i = 0; i < MWIFI_ADDR_LEN; i++) {
                dest_addr[i] = mac_data[i];
            }
        } while (0);

        json_data = cJSON_GetObjectItem(json_root, "data");
        char *send_data = cJSON_PrintUnformatted(json_data);

        ret = mwifi_write(dest_addr, &data_type, send_data, strlen(send_data), true);
        MDF_ERROR_GOTO(ret != MDF_OK, FREE_MEM, "<%s> mwifi_root_write", mdf_err_to_name(ret));

FREE_MEM:
        MDF_FREE(send_data);
        cJSON_Delete(json_root);
    }

    MDF_LOGI("TCP client read task is exit");

    close(g_sockfd);
    g_sockfd = -1;
    MDF_FREE(data);
    vTaskDelete(NULL);
}










///! tcp_client_write_task(void *arg)
static void root_task(void *arg)
{
    mdf_err_t ret                    = MDF_OK;
    char *data                       = MDF_MALLOC(MWIFI_PAYLOAD_LEN);
    size_t size                      = MWIFI_PAYLOAD_LEN;
    uint8_t src_addr[MWIFI_ADDR_LEN] = {0x0};
    mwifi_data_type_t data_type      = {0x0};
 
    MDF_LOGI("Root is running");

    while (mwifi_is_connected()) 
    {
        if (g_sockfd == -1){
            vTaskDelay(500/portTICK_RATE_MS) ; 
            continue; 
        }
        size = MWIFI_PAYLOAD_LEN;
        memset(data, 0, MWIFI_PAYLOAD_LEN);
        ret = mwifi_root_read(src_addr, &data_type, data, &size, portMAX_DELAY);
        MDF_ERROR_CONTINUE(ret != MDF_OK, "<%s> mwifi_root_read", mdf_err_to_name(ret));
        MDF_LOGI("Root receive, addr: " MACSTR ", size: %d, data: %s", MAC2STR(src_addr), size, data);
        MDF_LOGD("TCP write, size: %d, data: %s", size, data) ; 
        ret = write(g_sockfd,data,size) ; 
        MDF_ERROR_CONTINUE(ret <= 0, "<%s> TCP write", strerror(errno));
//        size = sprintf(data, "(%d) json_data!!", i);
//        ret = mwifi_root_write(src_addr, 1, &data_type, data, size, true);
//        MDF_ERROR_CONTINUE(ret != MDF_OK, "mwifi_root_recv, ret: %x", ret);
//        MDF_LOGI("Root send, addr: " MACSTR ", size: %d, data: %s", MAC2STR(src_addr), size, data);
    }

    MDF_LOGW("Root is exit");

    MDF_FREE(data);
    vTaskDelete(NULL);
}

static void node_read_task(void *arg)
{
    mdf_err_t ret = MDF_OK;
    char *data    = MDF_MALLOC(MWIFI_PAYLOAD_LEN);
    size_t size   = MWIFI_PAYLOAD_LEN;
    mwifi_data_type_t data_type      = {0x0};
    uint8_t src_addr[MWIFI_ADDR_LEN] = {0x0};

    MDF_LOGI("Note read task is running");

    for (;;) {
        if (!mwifi_is_connected()) {
            vTaskDelay(500 / portTICK_RATE_MS);
            continue;
        }

        size = MWIFI_PAYLOAD_LEN;
        memset(data, 0, MWIFI_PAYLOAD_LEN);
        ret = mwifi_read(src_addr, &data_type, data, &size, portMAX_DELAY);
        MDF_ERROR_CONTINUE(ret != MDF_OK, "mwifi_read, ret: %x", ret);
        MDF_LOGI("Node receive, addr: " MACSTR ", size: %d, data: %s", MAC2STR(src_addr), size, data);
    }

    MDF_LOGW("Note read task is exit");
    MDF_FREE(data);
    vTaskDelete(NULL);
}

void node_write_task(void *arg)
{
    mdf_err_t ret = MDF_OK;
    int count     = 0;
    size_t size   = 0;
    char *data    = MDF_MALLOC(MWIFI_PAYLOAD_LEN);
    mwifi_data_type_t data_type = {0x0};
    uint8_t sta_mac[MWIFI_ADDR_LEN] = {0}; 
    MDF_LOGI("Node write task is running");
    esp_wifi_get_mac(ESP_IF_WIFI_STA,sta_mac) ; 
    for (;;) {
        if (!mwifi_is_connected()) {
            vTaskDelay(500 / portTICK_RATE_MS);
            continue;
        }

        size = sprintf(data, "(%d) Hello root!", count++);
        ret = mwifi_write(NULL, &data_type, data, size, true);
        MDF_ERROR_CONTINUE(ret != MDF_OK, "mwifi_write, ret: %x", ret);

        vTaskDelay(3000 / portTICK_RATE_MS);
    }

    MDF_LOGW("Node write task is exit");

    MDF_FREE(data);
    vTaskDelete(NULL);
}

/**
 * @brief Timed printing system information
 */
static void print_system_info_timercb(void *timer)
{
    uint8_t primary                 = 0;
    wifi_second_chan_t second       = 0;
    mesh_addr_t parent_bssid        = {0};
    uint8_t sta_mac[MWIFI_ADDR_LEN] = {0};
    wifi_sta_list_t wifi_sta_list   = {0x0};

    esp_wifi_get_mac(ESP_IF_WIFI_STA, sta_mac);
    esp_wifi_ap_get_sta_list(&wifi_sta_list);
    esp_wifi_get_channel(&primary, &second);
    esp_mesh_get_parent_bssid(&parent_bssid);

    MDF_LOGI("System information, channel: %d, layer: %d, self mac: " MACSTR ", parent bssid: " MACSTR
             ", parent rssi: %d, node num: %d, free heap: %u", primary,
             esp_mesh_get_layer(), MAC2STR(sta_mac), MAC2STR(parent_bssid.addr),
             mwifi_get_parent_rssi(), esp_mesh_get_total_node_num(), esp_get_free_heap_size());

    for (int i = 0; i < wifi_sta_list.num; i++) {
        MDF_LOGI("Child mac: " MACSTR, MAC2STR(wifi_sta_list.sta[i].mac));
    }

#ifdef MEMORY_DEBUG

    if (!heap_caps_check_integrity_all(true)) {
        MDF_LOGE("At least one heap is corrupt");
    }

    mdf_mem_print_heap();
    mdf_mem_print_record();
    mdf_mem_print_task();
#endif /**< MEMORY_DEBUG */
}

static mdf_err_t wifi_init()
{
    //!CREATE EVENT FOR WIFI FOR OBTAIN LOCAL Ip
   
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT(); //! obtiene configuraciòn wifi por defecto(use idf.py menuconfig)
     
    //printf("station_interface: %s \r\n",cfg.ap.ssid) ; 
    mdf_err_t ret          = nvs_flash_init();
   
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        MDF_ERROR_ASSERT(nvs_flash_erase());
        ret = nvs_flash_init();
    }

    //tcpip_adapter_init();           ///! instrucción magica ... no tocar ... sin esto no vivimos  
    MDF_ERROR_ASSERT(esp_netif_init());
    MDF_ERROR_ASSERT(ret);
    
    MDF_ERROR_ASSERT(esp_event_loop_create_default());
    ESP_ERROR_CHECK(esp_netif_create_default_wifi_mesh_netifs(&netif_sta, NULL));
    MDF_ERROR_ASSERT(esp_wifi_init(&cfg)); ///! 
    MDF_ERROR_ASSERT(esp_wifi_set_storage(WIFI_STORAGE_FLASH));
    MDF_ERROR_ASSERT(esp_wifi_set_mode(WIFI_MODE_STA));
    MDF_ERROR_ASSERT(esp_wifi_set_ps(WIFI_PS_NONE));
    MDF_ERROR_ASSERT(esp_mesh_set_6m_rate(false)); /// tasa de transmisión API-REFERENCE no es clara 
    MDF_ERROR_ASSERT(esp_wifi_start());
   
    printf("---------------------------end wifi init --------------------------------------\r\n") ; 

    return MDF_OK;
}

/**
 * @brief All module events will be sent to this task in esp-mdf
 *
 * @Note:
 *     1. Do not block or lengthy operations in the callback function.
 *     2. Do not consume a lot of memory in the callback function.
 *        The task memory of the callback function is only 4KB.
 */
static mdf_err_t event_loop_cb(mdf_event_loop_t event, void *ctx)
{
    MDF_LOGI("event_loop_cb, event: %d", event);

    switch (event) {
        case MDF_EVENT_MWIFI_STARTED:
            MDF_LOGI("MESH is started");
            break;
        case MDF_EVENT_MWIFI_PARENT_CONNECTED:
            MDF_LOGI("Parent is connected on station interface");
            if (esp_mesh_is_root()) {
                printf("im root \r\n") ; 
                esp_netif_dhcpc_start(netif_sta);
            }
            break;
        case MDF_EVENT_MWIFI_ROOT_ADDRESS:
            printf("mdf_event_root_address \r\n") ; 
            break ; 
        case MDF_EVENT_MWIFI_CHILD_CONNECTED:
            MDF_LOGI("child connected event ") ; 
           // printf(" ----------------end event child ------------------\r\n ") ; 
            break ;
        case MDF_EVENT_MWIFI_CHILD_DISCONNECTED:
            printf(" ----------------event child_disc ------------------\r\n ") ; 
            MDF_LOGI("child disconnected event ") ; 
            printf(" ----------------end event child_disc ------------------\r\n") ; 
            break ;
        case MDF_EVENT_MWIFI_PARENT_DISCONNECTED:
            MDF_LOGI("Parent is disconnected on station interface");
            break;
        case MDF_EVENT_MWIFI_ROOT_GOT_IP://MDF_EVENT_MWIFI_ROOT_GOT_IP
            MDF_LOGI("Root obtains the IP address. It is posted by LwIP stack automatically");
            xTaskCreate(root_task,"rooting_task",4*1024,NULL,CONFIG_MDF_TASK_DEFAULT_PRIOTY,NULL ) ; 
            xTaskCreate(tcp_client_read_task,"rooting_task",4*1024,NULL,CONFIG_MDF_TASK_DEFAULT_PRIOTY,NULL ) ; 
            
            break; 
        case MDF_EVENT_MWIFI_ROUTING_TABLE_ADD:
        case MDF_EVENT_MWIFI_ROUTING_TABLE_REMOVE:
            MDF_LOGI("total_num: %d", esp_mesh_get_total_node_num());
            break;
        case MDF_EVENT_MWIFI_EXCEPTION:
            printf("MDF_MWIFI_EXCEPTION\r\n") ; 
            break ; 
        case MDF_EVENT_MWIFI_STOPPED:
            MDF_LOGI("MWIFI IS IS TOPPED ") ; 
            break ; 
        case MESH_EVENT_FIND_NETWORK:
            MDF_LOGI("mesh_event_find_network") ; 
            break ; 
            
        default:
            break;
    }

    return MDF_OK;
}


void vPrintMeshInfo(void *pv){
    TickType_t ticks_delay = 4000/portTICK_RATE_MS ; 
 
    for(;;){
        printf(" ------------------------------configuration of mesh wifi------------\r\n") ;         
        printf(" ------------------------------end of mesh wifi------------\r\n") ; 
        vTaskDelay(ticks_delay) ; 
    }
}

void app_main()
{
    mwifi_init_config_t cfg = MWIFI_INIT_CONFIG_DEFAULT(); ///! configure using idf.py menuconfig
//    mwifi_init_config_t cfg = { -.. } custom configurations 
    mwifi_config_t config ={
        .router_ssid = "local", 
        .router_password  = "iarpublicas", 
        .channel   = CONFIG_MESH_CHANNEL,
        .mesh_id   = CONFIG_MESH_ID,
        .mesh_password = "hola-mundo",
        /*
            !FIXED USING THIS VALUES: MESH_ROOT,MESH_IDLE ,MESH_NODE. IF USE "CONFIG_DEVICE_TYPE "
            CONFIGURE DEVICE WITH "make menuconfig" or idf.py menuconfig ! 
        */
        //.mesh_type =  MWIFI_MESH_LEAF 
    };

    /**
     * @brief Set the log level for serial port printing.
     */
    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set(TAG, ESP_LOG_DEBUG);

    /**
     * @brief Initialize wifi mesh.
     */
    MDF_ERROR_ASSERT(mdf_event_loop_init(event_loop_cb)); //??   
    MDF_ERROR_ASSERT(wifi_init());
    MDF_ERROR_ASSERT(mwifi_init(&cfg));
    MDF_ERROR_ASSERT(mwifi_set_config(&config));
    MDF_ERROR_ASSERT(mwifi_start());
    ///! ver que son los grupos 
    //const uint8_t group_id_list[2][6] = {{0x01, 0x00, 0x5e, 0xae, 0xae, 0xae},
    //                                    {0x01, 0x00, 0x5e, 0xae, 0xae, 0xaf}};

    //MDF_ERROR_ASSERT(esp_mesh_set_group_id((mesh_addr_t *)group_id_list,
      //                                     sizeof(group_id_list) / sizeof(group_id_list[0])));



    xTaskCreate(node_write_task, "node_write_task", 4 * 1024,
                    NULL, CONFIG_MDF_TASK_DEFAULT_PRIOTY, NULL);
    xTaskCreate(node_read_task, "node_read_task", 4 * 1024,
                    NULL, CONFIG_MDF_TASK_DEFAULT_PRIOTY, NULL);
   
    xTaskCreate(vTaskInfoNode,"togle_led_info", 4*4096,NULL, CONFIG_MDF_TASK_DEFAULT_PRIOTY,NULL) ; 
    // TimerHandle_t timer = xTimerCreate("print_system_info", 10000 / portTICK_RATE_MS,
    //                                    true, NULL, print_system_info_timercb);
    // xTimerStart(timer, 0);
}
