#include <ADS1115.h>
#include <stdio.h>
#include "mwifi.h"
#include "mdf_common.h"
#define LEAF_LED_GPIO GPIO_NUM_22   /// led amarillo 
#define ROOT_LED_GPIO GPIO_NUM_15   /// led azul 
#define IDLE_LED_GPIO GPIO_NUM_5    /// led verde 
#define NODE_LED_GPIO GPIO_NUM_21   /// led rojo 
#define TIME_READ_SAMPLES  (10000) 

extern QueueHandle_t xQueueReadSensor;

extern const char *TAG ;

typedef struct{
    uint8_t id_sensor[6] ; ///ID sensor is MAC ADDRESS  
    char data_sensor[6]  ; 
}msg_sensor_t ; 
SemaphoreHandle_t xMutex = NULL; 


void vTaskGetADC(void *pv){
    msg_sensor_t data_sensor_read ; 
  /*  ADS1115_alert_comparator_t alert = { OFF,0, 0,0 };
    ADS1115_config_t ads1115 = {
                CHANNEL_0_1,
                FSR_6144  ,
                CONTINIOUS_MODE,
                SPS_8,
                alert,
    };  */ 

  //  uint8_t init_ads_check = ADS1115init(0x48, &ads1115) ;
 //   MDF_LOGI("init ads check config: %d\r\n", init_ads_check) ;

    while (ESP_OK != esp_wifi_get_mac(WIFI_IF_STA ,data_sensor_read.id_sensor))
    { 
        MDF_LOGI(" WAIT GET MAC ADDRESS")  ; 
        vTaskDelay((500/portTICK_RATE_MS)) ; 
    }     
    printf("start get ADC task \r\n ") ; 
    xQueueReadSensor = xQueueCreate( 10, sizeof(msg_sensor_t ) );
    xMutex = xSemaphoreCreateMutex();
    assert(xQueueReadSensor!= NULL) ; 
    const TickType_t time_samples = TIME_READ_SAMPLES/portTICK_RATE_MS ; 
    uint32_t counter_send_queue_send = 0 ; 
  //  float voltage_sensor ; 
    while(1){
         if (!mwifi_is_connected()) {
            vTaskDelay(500 / portTICK_RATE_MS);
            continue;
        }
        xSemaphoreTake( xMutex, portMAX_DELAY );
        {
            /// @brief routinge for read sensors and voltage 
            /// @param pv 
     //       voltage_sensor = getVoltage() ; 
     //       sprintf(data_sensor_read.data_sensor,"%.2f",voltage_sensor) ; 
            data_sensor_read.data_sensor[0] = '2' ;
            data_sensor_read.data_sensor[1] = '2' ;
            data_sensor_read.data_sensor[2] = '.' ;
            data_sensor_read.data_sensor[3] = '2' ; 
        }
        xSemaphoreGive( xMutex );        ///! read sensors 
                 
        //taskEXIT_CRITICAL() ; 
        ///create functions for 
        xQueueSend(xQueueReadSensor,&data_sensor_read,(TickType_t) 0 ) ; 
        counter_send_queue_send++ ; 
        printf("queue send %d",counter_send_queue_send) ; 

        vTaskDelay(time_samples) ; 
    }
}

void vTaskInfoNode(void *pv){
    wifi_config_t conf_wifi_ap   ; 
    wifi_config_t conf_wifi_sta  ; 
    mesh_addr_t mesh_data_table[6]  ; //= NULL ; 
   // mwifi_data_type_t data_type = {0x0};
    uint8_t mac_address_ap[6] ; 
    uint8_t mac_address_sta[6] ; 
    int  size_table_routing_main ; 
    int  size_table_routing_response;//=NULL;  
    mesh_type_t mesh_type ; 
    gpio_reset_pin(LEAF_LED_GPIO) ; 
    gpio_reset_pin(ROOT_LED_GPIO) ; 
    gpio_reset_pin(IDLE_LED_GPIO) ; 
    gpio_reset_pin(NODE_LED_GPIO) ; 
    gpio_set_direction(LEAF_LED_GPIO , GPIO_MODE_OUTPUT) ; 
    gpio_set_direction(ROOT_LED_GPIO , GPIO_MODE_OUTPUT) ; 
    gpio_set_direction(IDLE_LED_GPIO , GPIO_MODE_OUTPUT) ; 
    gpio_set_direction(NODE_LED_GPIO , GPIO_MODE_OUTPUT) ; 
    while(1){ 
       printf(" --------------------------------------------------\r\n") ; 
       esp_wifi_get_config(WIFI_IF_AP ,&conf_wifi_ap)    ; 
       esp_wifi_get_mac(WIFI_IF_AP,mac_address_ap)  ;
       esp_wifi_get_config(WIFI_IF_STA ,&conf_wifi_sta)    ; 
       esp_wifi_get_mac(WIFI_IF_STA,mac_address_sta)  ;
       size_table_routing_main = esp_mesh_get_routing_table_size() ; 
       
       esp_mesh_get_routing_table((mesh_addr_t *)&mesh_data_table, sizeof(mesh_addr_t)*size_table_routing_main, &size_table_routing_response) ; 
       //esp_mesh_get_subnet_nodes_list(mesh_data_table, nodes_table ,size_table_routing_main) ; 
       mesh_type = esp_mesh_get_type() ; 
       printf("mesh type: %d   \r\n",mesh_type); 
       printf("ssid_ap:   %s   \r\n",conf_wifi_ap.sta.ssid) ; 
       printf("mac_ap: %02x %02x %02x %02x %02x %02x \r\n",mac_address_ap[0],mac_address_ap[1],mac_address_ap[2],
                                                                mac_address_ap[3], mac_address_ap[4],mac_address_ap[5]) ; 
       printf("ssid_sta: %s   \r\n",conf_wifi_sta.sta.ssid) ; 
       printf("mac_sta: %02x %02x %02x %02x %02x %02x \r\n",mac_address_sta[0],mac_address_sta[1],mac_address_sta[2],
                                                            mac_address_sta[3], mac_address_sta[4],mac_address_sta[5]) ; 
       printf("size mesh routing table: %d \r\n", size_table_routing_main) ; 
       printf("size mesh routing table response: %d \r\n", size_table_routing_response) ; 
       for (int i = 0; i < size_table_routing_main; i++) 
       {   
            printf("routing_table: %02x %02x %02x %02x %02x %02x \r\n", MAC2STR(mesh_data_table[i].addr)) ;        
       }
       printf("---------------------------------------------------------\r\n") ;         
       switch(mesh_type){
            case MESH_LEAF:
               printf("mesh leaf \r\n") ; 
               gpio_set_level(LEAF_LED_GPIO,1) ;
               gpio_set_level(ROOT_LED_GPIO,0) ;
               gpio_set_level(IDLE_LED_GPIO,0) ;
               gpio_set_level(NODE_LED_GPIO,0) ;
               break ; 
            case MESH_NODE:
                printf("mesh node \r\n") ; 
                gpio_set_level(LEAF_LED_GPIO,0) ;
                gpio_set_level(ROOT_LED_GPIO,0) ;
                gpio_set_level(IDLE_LED_GPIO,0) ;
                gpio_set_level(NODE_LED_GPIO,1) ;
                break ; 
            case MESH_ROOT:
                printf("mesh root \r\n") ; 
                gpio_set_level(LEAF_LED_GPIO,0) ;
                gpio_set_level(ROOT_LED_GPIO,1) ;
                gpio_set_level(IDLE_LED_GPIO,0) ;
                gpio_set_level(NODE_LED_GPIO,0) ;
                break ; 
            case MESH_IDLE:
                printf("mesh idle \r\n") ; 
                gpio_set_level(LEAF_LED_GPIO,0) ;
                gpio_set_level(ROOT_LED_GPIO,0) ;
                gpio_set_level(IDLE_LED_GPIO,1) ;
                gpio_set_level(NODE_LED_GPIO,0) ;
                break ; 
            default: 
                //// by default all leds off using board mesh 
                gpio_set_level(LEAF_LED_GPIO,0) ;
                gpio_set_level(ROOT_LED_GPIO,0) ;
                gpio_set_level(IDLE_LED_GPIO,0) ;
                gpio_set_level(NODE_LED_GPIO,0) ; 
                break ;  
        }
        vTaskDelay((TickType_t ) 1000/portTICK_RATE_MS) ; 
    }

}






 
void vTaskToogleLeds(void *arg){
    gpio_reset_pin(LEAF_LED_GPIO) ; 
    gpio_reset_pin(ROOT_LED_GPIO) ; 
    gpio_reset_pin(IDLE_LED_GPIO) ; 
    gpio_reset_pin(NODE_LED_GPIO) ; 
    gpio_set_direction(LEAF_LED_GPIO , GPIO_MODE_OUTPUT) ; 
    gpio_set_direction(ROOT_LED_GPIO , GPIO_MODE_OUTPUT) ; 
    gpio_set_direction(IDLE_LED_GPIO , GPIO_MODE_OUTPUT) ; 
    gpio_set_direction(NODE_LED_GPIO , GPIO_MODE_OUTPUT) ; 
    while(1){ 
        gpio_set_level(LEAF_LED_GPIO,1) ;
        gpio_set_level(ROOT_LED_GPIO,1) ;
        gpio_set_level(IDLE_LED_GPIO,1) ;
        gpio_set_level(NODE_LED_GPIO,1) ;
        vTaskDelay(1000/portTICK_RATE_MS) ; 
        gpio_set_level(LEAF_LED_GPIO,0) ;
        gpio_set_level(ROOT_LED_GPIO,0) ;
        gpio_set_level(IDLE_LED_GPIO,0) ;
        gpio_set_level(NODE_LED_GPIO,0) ;
        vTaskDelay(1000/portTICK_RATE_MS) ; 
    
    }

}