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
#define PORT_SENSOR_JUMPER GPIO_NUM_23
#include "archivo.c"
#include "mwifi.h"
#include "mdf_common.h"
// #define MEMORY_DEBUG
static esp_netif_t *netif_sta = NULL; 
const char *TAG = "IAR MESH ";
static int g_sockfd = -1 ;
QueueHandle_t xQueueReadSensor;
static bool isSensorNode(void) ; 


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

    while (mwifi_is_connected()) 
    {
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

static void root_task(void *arg)
{
    mdf_err_t ret = MDF_OK;
    char *data    = MDF_CALLOC(1, MWIFI_PAYLOAD_LEN);
    size_t size   = MWIFI_PAYLOAD_LEN;
    uint8_t src_addr[MWIFI_ADDR_LEN] = {0x0};
    mwifi_data_type_t data_type      = {0x0};

    MDF_LOGI("task root is running ");

    while (mwifi_is_connected()) {
        size = MWIFI_PAYLOAD_LEN - 1;
        memset(data, 0, MWIFI_PAYLOAD_LEN);
        MDF_LOGI("--------------------MWIFI ROOT READ WAITING -------------------") ; 
        ret = mwifi_root_read(src_addr, &data_type, data, &size, portMAX_DELAY);
        printf("%s",data) ; 
        //MDF_LOGD("root_read_data, size: %d, data: %s", size, data);
        MDF_ERROR_CONTINUE(ret != MDF_OK, "<%s> mwifi_root_read", mdf_err_to_name(ret));
        MDF_LOGI("--------------------END MWIFI ROOT READ WAITING -------------------") ; 

    }

    MDF_LOGI("TCP client write task is exit");

    close(g_sockfd);
    g_sockfd = -1;
    MDF_FREE(data);
    vTaskDelete(NULL);
    
}




void node_write_task_sensor(void *arg)
{
    mdf_err_t ret = MDF_OK;
    int count     = 0;
    size_t size   = 0;
    char *data    = NULL ; //MDF_MALLOC(MWIFI_PAYLOAD_LEN);
    mwifi_data_type_t data_type = {0x0};
    uint8_t sta_mac[MWIFI_ADDR_LEN] = {0}; 
    MDF_LOGI("Node write task is running");
    esp_wifi_get_mac(ESP_IF_WIFI_STA,sta_mac) ; 
    msg_sensor_t rx_value_from_sensor ; 
    for (;;) {
        if (!mwifi_is_connected()) {
            vTaskDelay(500 / portTICK_RATE_MS);
            continue;
        }
        xQueueReceive( xQueueReadSensor, &rx_value_from_sensor, portMAX_DELAY ) ; 
        size = asprintf(&data, "{\"src_addr\": \"" MACSTR "\",\"data\": \"%s\",\"count\": %d}",
                        MAC2STR(rx_value_from_sensor.id_sensor),rx_value_from_sensor.data_sensor ,count++);
        printf("queue received %s\r\n",rx_value_from_sensor.data_sensor) ; 
        MDF_LOGD("Node send, size: %d, data: %s", size, data); 
        printf("sending info data") ; 

        ret = mwifi_write(NULL, &data_type, data, size, true); ///!data to send root 
        printf("end sending info data") ; 

        MDF_FREE(data);       
        MDF_ERROR_CONTINUE(ret != MDF_OK, "<%s> mwifi_write", mdf_err_to_name(ret));
    }
    MDF_LOGW("Node write task is exit");
    MDF_FREE(data);
    vTaskDelete(NULL);
}


void node_read(void *arg)
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
        ret = mwifi_read(src_addr, &data_type, data, &size, portMAX_DELAY); ///!one second for received 
        MDF_ERROR_CONTINUE(ret != MDF_OK, "mwifi_read, ret: %x", ret);
        MDF_LOGI("Node receive, addr: " MACSTR ", size: %d, data: %s", MAC2STR(src_addr), size, data);
    }

    MDF_LOGW("node read task is exit");

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
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT(); //! obtiene configuraciòn wifi por defecto(use idf.py menuconfig)     
    mdf_err_t ret          = nvs_flash_init();
   
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        MDF_ERROR_ASSERT(nvs_flash_erase());
        ret = nvs_flash_init();
    }

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
            MDF_LOGI("-----------------------------") ; 
            MDF_LOGI("MESH is started");
            MDF_LOGI("-----------------------------") ; 

            break;
        case MDF_EVENT_MWIFI_PARENT_CONNECTED:
            MDF_LOGI("-----------------------------") ; 
            MDF_LOGI("MDF_EVENT_MWIFI_PARENT_CONNECTED");
            MDF_LOGI("-----------------------------") ; 
            if (esp_mesh_is_root()) 
            {
                MDF_LOGI("SOY EL ROOT") ; 
                esp_netif_dhcpc_start(netif_sta);
                xTaskCreate(root_task, "root_task", 4 * 1024,
                    NULL, CONFIG_MDF_TASK_DEFAULT_PRIOTY, NULL);            
            }
            break;
        case MDF_EVENT_MWIFI_VOTE_STOPPED:
            MDF_LOGI("-----------------------------") ; 
            MDF_LOGI("MDF_EVENT_MWIFI_VOTE_STOPPED") ; 
            MDF_LOGI("-----------------------------") ; 
            break ; 
        case MDF_EVENT_MWIFI_CHANNEL_SWITCH: 
            MDF_LOGI("-----------------------------") ; 
            MDF_LOGI("MDF_EVENT_MWIFI_CHANNEL_SWITCH") ; 
            MDF_LOGI("-----------------------------") ; 
            break ; 
        case MDF_EVENT_MWIFI_VOTE_STARTED:
            MDF_LOGI("-----------------------------") ; 
            MDF_LOGI("MDF_EVENT_MWIFI_VOTE_STARTED") ; 
            MDF_LOGI("-----------------------------") ; 
            break ; 
        case MDF_EVENT_MWIFI_TODS_STATE: 
            MDF_LOGI("-----------------------------") ; 
            MDF_LOGI("MDF_EVENT_MWIFI_TODS_STATE") ; 
            MDF_LOGI("-----------------------------") ; 
            break ; 
        case MDF_EVENT_MWIFI_ROOT_ADDRESS:
            MDF_LOGI("-----------------------------") ; 
            MDF_LOGI("MDF_EVENT_MWIFI_ROOT_ADDRESS") ; 
            MDF_LOGI("-----------------------------") ; 
            break ;   
        case MDF_EVENT_MWIFI_CHILD_CONNECTED:
            MDF_LOGI("-----------------------------") ; 
            MDF_LOGI("MDF_EVENT_MWIFI_CHILD_CONNECTED") ; 
            MDF_LOGI("-----------------------------") ; 
            break ;
        case MDF_EVENT_MWIFI_CHILD_DISCONNECTED:
            MDF_LOGI("-----------------------------") ; 
            MDF_LOGI("MDF_EVENT_MWIFI_CHILD_DISCONNECTED") ; 
            MDF_LOGI("-----------------------------") ; 
            break ;
        case MDF_EVENT_MWIFI_PARENT_DISCONNECTED:
            MDF_LOGI("-----------------------------") ; 
            MDF_LOGI("MDF_EVENT_MWIFI_PARENT_DISCONNECTED") ; 
            MDF_LOGI("-----------------------------") ;         
            break;
        case MDF_EVENT_MWIFI_ROOT_GOT_IP:
            MDF_LOGI("-----------------------------") ; 
            MDF_LOGI("MDF_EVENT_MWIFI_ROOT_GOT_IP") ; 
            MDF_LOGI("-----------------------------") ; 
            break; 
        case MDF_EVENT_MWIFI_ROUTING_TABLE_ADD:
            MDF_LOGI("-----------------------------") ; 
            MDF_LOGI("MDF_EVENT_MWIFI_ROUTING_TABLE_ADD") ; 
            MDF_LOGI("-----------------------------") ; 
            break ; 
        case MDF_EVENT_MWIFI_ROUTING_TABLE_REMOVE:
            MDF_LOGI("-----------------------------") ; 
            MDF_LOGI("MDF_EVENT_MWIFI_ROUTING_TABLE_REMOVE") ; 
            MDF_LOGI("-----------------------------") ; 
            break;
        case MDF_EVENT_MWIFI_EXCEPTION:
            MDF_LOGI("-----------------------------") ; 
            MDF_LOGI("MDF_EVENT_MWIFI_EXCEPTION") ; 
            MDF_LOGI("-----------------------------") ; 
            break ; 
        case MDF_EVENT_MWIFI_NO_PARENT_FOUND: 
            MDF_LOGI("-----------------------------") ; 
            MDF_LOGI("MDF_EVENT_PARENT_FOUND") ; 
            MDF_LOGI("-----------------------------") ; 
            break ; 
        case MDF_EVENT_MWIFI_STOPPED:
            MDF_LOGI("-----------------------------") ; 
            MDF_LOGI("MDF_EVENT_MWIFI_STOPPED") ; 
            MDF_LOGI("-----------------------------") ; 
            break ; 
        case MDF_EVENT_MWIFI_LAYER_CHANGE: 
            MDF_LOGI("-----------------------------") ; 
            MDF_LOGI("MDF_EVENT_MWIFI_LAYER_CHANGE") ; 
            MDF_LOGI("-----------------------------") ; 
            break ; 
        case MDF_EVENT_MWIFI_ROOT_SWITCH_REQ: 
            MDF_LOGI("-----------------------------") ; 
            MDF_LOGI("MDF_EVENT_MWIFI_ROOT_SWITCH_REQ") ; 
            MDF_LOGI("-----------------------------") ; 
            break ; 
        case MDF_EVENT_MWIFI_ROOT_SWITCH_ACK:
            MDF_LOGI("-----------------------------") ; 
            MDF_LOGI("MDF_EVENT_MWIFI_ROOT_SWITCH_ACK") ; 
            MDF_LOGI("-----------------------------") ; 
            break ; 
        case MDF_EVENT_MWIFI_ROOT_ASKED_YIELD: 
            MDF_LOGI("-----------------------------") ; 
            MDF_LOGI("MDF_EVENT_MWIFI_ROOT_ASKED_YIELD") ; 
            MDF_LOGI("-----------------------------") ; 
            break ; 
        case MDF_EVENT_MWIFI_SCAN_DONE: 
            MDF_LOGI("-----------------------------") ; 
            MDF_LOGI("MDF_EVENT_MWIFI_SCAN_DONE")     ; 
            MDF_LOGI("-----------------------------") ; 
            break; 
        case MDF_EVENT_MWIFI_NETWORK_STATE: 
            MDF_LOGI("-----------------------------") ; 
            MDF_LOGI("MDF_EVENT_MWIFI_NETWORK_STATE") ; 
            MDF_LOGI("-----------------------------") ; 
            break ; 
        case MDF_EVENT_MWIFI_STOP_RECONNECTION: 
            MDF_LOGI("-----------------------------") ; 
            MDF_LOGI("MDF_EVENT_MWIFI_STOP_RECONNECTION") ; 
            MDF_LOGI("-----------------------------") ; 
            break ;  
        case MDF_EVENT_MWIFI_FIND_NETWORK: 
            MDF_LOGI("-----------------------------"); 
            MDF_LOGI("MDF_EVENT_MWIFI_FIND_NETWORK") ; 
            MDF_LOGI("-----------------------------"); 
            break ;    
        case MDF_EVENT_MWIFI_ROUTER_SWITCH: 
            MDF_LOGI("-----------------------------") ; 
            MDF_LOGI("MDF_EVENT_MWIFI_ROUTER_SWITCH") ; 
            MDF_LOGI("-----------------------------") ; 
            break ;    
        case MDF_EVENT_MWIFI_CHANNEL_NO_FOUND: 
            MDF_LOGI("-----------------------------") ; 
            MDF_LOGI("MDF_EVENT_MWIFI_CHANNEL_NO_FOUND") ; 
            MDF_LOGI("-----------------------------") ; 
            break ;    
        case MDF_EVENT_MWIFI_ROOT_LOST_IP: 
            MDF_LOGI("-----------------------------") ; 
            MDF_LOGI("MDF_EVENT_MWIFI_ROOT_LOST_IP")  ; 
            MDF_LOGI("-----------------------------") ; 
            break ;    
         default:
            break;
    }

    return MDF_OK;
}


void app_main()
{
    mwifi_init_config_t cfg = MWIFI_INIT_CONFIG_DEFAULT(); ///! configure using idf.py menuconfig
    mwifi_config_t config ={
        .router_ssid     = CONFIG_ROUTER_SSID,
        .router_password = CONFIG_ROUTER_PASSWORD,
        .mesh_id         = CONFIG_MESH_ID,
        .mesh_password   = "hola mundo",
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
    ///MDF_ERROR_ASSERT(wifi_init());
    /**
     * @brief Initialize wifi mesh.
     */
    MDF_ERROR_ASSERT(mdf_event_loop_init(event_loop_cb)); //??   
    MDF_ERROR_ASSERT(wifi_init());
    MDF_ERROR_ASSERT(mwifi_init(&cfg));
    MDF_ERROR_ASSERT(mwifi_set_config(&config));
    MDF_ERROR_ASSERT(mwifi_start()); 
    
    if (isSensorNode() == true){ 
        printf("hay sensor \r\n") ; 
        xTaskCreate(vTaskGetADC,
                    "sensor_read",
                    4*1024,
                    NULL , //parameters for functions
                    CONFIG_MDF_TASK_DEFAULT_PRIOTY+1,
                    NULL) ; 
                    xTaskCreate(node_write_task_sensor, "node_write_task", 4 * 1024,
                    NULL, CONFIG_MDF_TASK_DEFAULT_PRIOTY, NULL);    
    }else{
      printf("no hay sensor \r\n") ; 
      xTaskCreate(node_read,
                  "node_read",
                  4*1024,
                  NULL , //parameters for functions
                  CONFIG_MDF_TASK_DEFAULT_PRIOTY+1,
                  NULL) ;  

    } 
  
   /// xTaskCreate(vTaskInfoNode,"togle_led_info", 4*4096,NULL, CONFIG_MDF_TASK_DEFAULT_PRIOTY,NULL) ; 


}



/**
 * @brief 
 * 
 * @return true: hay sensor  
 * @return false no hay sensor 
 */
static bool isSensorNode(void){ 
    ///    gpio_config(&io_conf);
    bool response = false ; 
    int  level = 0 ;  
    gpio_pullup_en(PORT_SENSOR_JUMPER) ; 
    gpio_set_direction(PORT_SENSOR_JUMPER,GPIO_MODE_INPUT) ; 
    level = gpio_get_level(PORT_SENSOR_JUMPER)     ; 
    if (level == 0){ 
        response = true ; 
    }else if (level == 1 ){ 
        response = false ; 
    }else{ 
        response = false ; 
        
    }
    gpio_reset_pin(PORT_SENSOR_JUMPER) ; 
    return response ; 
}