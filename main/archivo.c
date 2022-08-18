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
    
    uint8_t mac_address_ap[6] ; 
    uint8_t mac_address_sta[6] ; 
    int  size_table_routing_main ; 
    int  *size_table_routing_response ;  
    //*size_table_routing_response  = 2 ;  
    
    gpio_reset_pin(LEAF_LED_GPIO) ; 
    gpio_reset_pin(ROOT_LED_GPIO) ; 
    gpio_reset_pin(IDLE_LED_GPIO) ; 
    gpio_reset_pin(IDLE_LED_GPIO) ; 
    gpio_set_direction(LEAF_LED_GPIO , GPIO_MODE_OUTPUT) ; 
    gpio_set_direction(ROOT_LED_GPIO , GPIO_MODE_OUTPUT) ; 
    gpio_set_direction(IDLE_LED_GPIO , GPIO_MODE_OUTPUT) ; 
    gpio_set_direction(NODE_LED_GPIO , GPIO_MODE_OUTPUT) ; 
    while(1){ 
       printf(" --------------------------------------------------\r\n") ; 
       // mwifi_get_config(&config_mesh) ;   
       // esp_mesh_get_config(&config) ; 
       // esp_wifi_sta_get_ap_info(&ap_data) ; 
       // printf("config_router_ssid: %s \r\n", conf_wifi.sta.ssid) ;  
       // printf("config_router_pass: %s \r\n", conf_wifi.sta.password) ;  
       // mwifi_print_config() ; 
       /*
         printf("messhh value: %d  \r\n", config_mesh.mesh_type); 
         printf("ap config: %s\r\n",config_mesh.router_ssid) ; 
         printf("ap config: %s\r\n",config_mesh.router_password) ; 
       */  
       esp_wifi_get_config(WIFI_IF_AP ,&conf_wifi_ap)    ; 
       esp_wifi_get_mac(WIFI_IF_AP,mac_address_ap)  ;
       esp_wifi_get_config(WIFI_IF_STA ,&conf_wifi_sta)    ; 
       esp_wifi_get_mac(WIFI_IF_STA,mac_address_sta)  ;
       size_table_routing_main = esp_mesh_get_routing_table_size() ; 
       esp_mesh_get_routing_table((mesh_addr_t *)&mesh_data_table, sizeof(mesh_addr_t)*6, &size_table_routing_response) ; 
       //esp_mesh_get_subnet_nodes_list(mesh_data_table, nodes_table ,size_table_routing_main) ; 
       printf("mesh type: %d   \r\n",  esp_mesh_get_type()); 
       printf("ssid_ap:   %s   \r\n",conf_wifi_ap.sta.ssid) ; 
       printf("mac_ap: %02x %02x %02x %02x %02x %02x \r\n",mac_address_ap[0],mac_address_ap[1],mac_address_ap[2],
                                                                mac_address_ap[3], mac_address_ap[4],mac_address_ap[5]) ; 
       printf("ssid_sta: %s   \r\n",conf_wifi_sta.sta.ssid) ; 
       printf("mac_sta: %02x %02x %02x %02x %02x %02x \r\n",mac_address_sta[0],mac_address_sta[1],mac_address_sta[2],
                                                            mac_address_sta[3], mac_address_sta[4],mac_address_sta[5]) ; 
        
       printf("size mesh routing table: %d \r\n", size_table_routing_main) ; 
       if (size_table_routing_main != 0){
            int index = 0 ; 
            for (index = 0 ; index<size_table_routing_main;index++){
                uint8_t mac_0 =(uint8_t *) mesh_data_table[0].addr[0] ; 
                printf("MAC_ADDRESSS_NODES: %02x  \r\n",mac_0 ) ; 
            }

       }

       printf("---------------------------------------------------------\r\n") ;         
       
       /* 
        switch(config_mesh.mesh_type){
            case MWIFI_MESH_IDLE:
                gpio_set_level(LEAF_LED_GPIO,0) ;
                gpio_set_level(ROOT_LED_GPIO,0) ;
                gpio_set_level(IDLE_LED_GPIO,1) ;
                gpio_set_level(NODE_LED_GPIO,0) ;
                break ; 
            case MWIFI_MESH_ROOT:
                gpio_set_level(LEAF_LED_GPIO,0) ;
                gpio_set_level(ROOT_LED_GPIO,1) ;
                gpio_set_level(IDLE_LED_GPIO,0) ;
                gpio_set_level(NODE_LED_GPIO,0) ;
                break ; 
            case MWIFI_MESH_NODE:
                gpio_set_level(LEAF_LED_GPIO,0) ;
                gpio_set_level(ROOT_LED_GPIO,0) ;
                gpio_set_level(IDLE_LED_GPIO,0) ;
                gpio_set_level(NODE_LED_GPIO,1) ;
                break ; 
            case MWIFI_MESH_LEAF:
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
            
        }
        */ 
        vTaskDelay((TickType_t ) 1000/portTICK_RATE_MS) ; 

    }




}

