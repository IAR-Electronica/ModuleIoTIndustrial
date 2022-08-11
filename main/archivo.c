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
//    mwifi_config_t config_mesh ;
    ///! CONFIGURACIÒN DE PUERTOS GPIO para realizar el prendido de leds para debugger  
    //wifi_ap_record_t ap_data ; 
    wifi_config_t conf_wifi ; 
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
        //mwifi_get_config(&config_mesh) ;   
        //esp_mesh_get_config(&config) ; 
        //esp_wifi_sta_get_ap_info(&ap_data) ; 
       // esp_wifi_get_config(WIFI_IF_AP ,&conf_wifi) ; 
       // printf("config_router_ssid: %s \r\n", conf_wifi.sta.ssid) ;  
       // printf("config_router_pass: %s \r\n", conf_wifi.sta.password) ;  
        //mwifi_print_config() ; 
       /*
        printf("messhh value: %d  \r\n", config_mesh.mesh_type); 
        printf("ap config: %s\r\n",config_mesh.router_ssid) ; 
        printf("ap config: %s\r\n",config_mesh.router_password) ; 
        */  
        printf("messhh value: %d  \r\n",  esp_mesh_get_type()); 
        // mwifi_print_config() ; 
        // mwifi_get_config()
        printf(" --------------------------------------------------\r\n") ;         
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

