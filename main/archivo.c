#include <ADS1115.h>
#include <stdio.h>
#include "mwifi.h"

#define LEAF_LED_GPIO GPIO_NUM_22 
#define ROOT_LED_GPIO GPIO_NUM_15 
#define IDLE_LED_GPIO GPIO_NUM_5 
#define NODE_LED_GPIO GPIO_NUM_21 




void vTaskGetADC(void *pv){
    float voltage ; 
    ///!adc_init ! 
    while(1){
        voltage =getVoltage() ; 
        printf("voltage: %f",voltage ) ; 
    }

    //! tirar abajo la tarea 
}

void vTaskInfoNode(void *pv){
    // mwifi_config_t config_mesh ;
    // CONFIGURACIÒN DE PUERTOS GPIO para realizar el prendido de leds para debugger  
    // wifi_ap_record_t ap_data ; 
    wifi_config_t conf_wifi_ap   ; 
    wifi_config_t conf_wifi_sta  ; 
    mesh_addr_t mesh_data_table[6]  ; //= NULL ; 
    //mesh_addr_t *nodes_table     ; // = NULL ; 
     mwifi_data_type_t data_type = {0x0};
    uint8_t mac_address_ap[6] ; 
    uint8_t mac_address_sta[6] ; 
    int  size_table_routing_main ; 
    int  size_table_routing_response;//=NULL;  
    mesh_type_t mesh_type ; 
    //char *data    = MDF_MALLOC(MWIFI_PAYLOAD_LEN);

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
       for (int i = 0; i < size_table_routing_main; i++) {   
//            printf (mesh_data_table[i].addr[0],mesh_data_table[i].addr[1],mesh_data_table[i].addr[2],mesh_data_table[i].addr[3]
//                mesh_data_table[i].addr[4], mesh_data_table[i].addr[5]); 
        printf("routing_table: %02x %02x %02x %02x %02x %02x \r\n", MAC2STR(mesh_data_table[i].addr)) ;        
       }

     
       printf("---------------------------------------------------------\r\n") ;         
       


        switch(mesh_type){
            case MESH_LEAF:
                printf("mesh leaf \r\n") ; 
                gpio_set_level(LEAF_LED_GPIO,0) ;
                gpio_set_level(ROOT_LED_GPIO,0) ;
                gpio_set_level(IDLE_LED_GPIO,1) ;
                gpio_set_level(NODE_LED_GPIO,0) ;
                break ; 
            case MESH_NODE:
                printf("mesh node \r\n") ; 
                gpio_set_level(LEAF_LED_GPIO,0) ;
                gpio_set_level(ROOT_LED_GPIO,1) ;
                gpio_set_level(IDLE_LED_GPIO,0) ;
                gpio_set_level(NODE_LED_GPIO,0) ;
                break ; 
            case MESH_ROOT:
                printf("mesh root \r\n") ; 
                gpio_set_level(LEAF_LED_GPIO,0) ;
                gpio_set_level(ROOT_LED_GPIO,0) ;
                gpio_set_level(IDLE_LED_GPIO,0) ;
                gpio_set_level(NODE_LED_GPIO,1) ;
                break ; 
            case MESH_IDLE:
                printf("mesh idle \r\n") ; 

                gpio_set_level(LEAF_LED_GPIO,1) ;
                gpio_set_level(ROOT_LED_GPIO,0) ;
                gpio_set_level(IDLE_LED_GPIO,0) ;
                gpio_set_level(NODE_LED_GPIO,0) ;
                break ; 
            default: 
                gpio_set_level(LEAF_LED_GPIO,0) ;
                gpio_set_level(ROOT_LED_GPIO,0) ;
                gpio_set_level(IDLE_LED_GPIO,0) ;
                gpio_set_level(NODE_LED_GPIO,0) ; 
                break ;  
        }
        
        vTaskDelay((TickType_t ) 1000/portTICK_RATE_MS) ; 

    }




}

