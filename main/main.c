#include "freertos/FreeRTOS.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "freertos/semphr.h"
#include "ADS1115.h"

#define BLINK_GPIO 19
uint8_t state_led  = 0 ;
SemaphoreHandle_t  xMutexADS1115 ; //!< MUTEX FOR TESTING CHANNEL


//mode_ads1115 mode_adc ;
esp_err_t event_handler(void *ctx, system_event_t *event)
{
    printf("error_event_handler") ;
	return ESP_OK;
}






void vTaskVgnd(void *par){
	float voltage ;
	while(1){
		xSemaphoreTake(xMutexADS1115, portMAX_DELAY) ;
		{
			selectChannel(CHANNEL_2_GND) ;
			vTaskDelay(100 / portTICK_RATE_MS); ///! wait for read channel
			voltage = getVoltage() ;
			printf("voltage A2 = %f\r\n",voltage) ;
			xSemaphoreGive(xMutexADS1115) ;
		}


		vTaskDelay(1000 / portTICK_RATE_MS); ///! wait for read channel

	}

}





void xTaskVdif(void *par){
	float voltage ;
	while(1){
		xSemaphoreTake(xMutexADS1115, portMAX_DELAY);
		{
			selectChannel(CHANNEL_0_1) ;
			vTaskDelay(100 / portTICK_RATE_MS); ///! wait for read channel
			voltage = getVoltage() ;
			printf("voltage dif A0-A1 = %f\r\n",voltage) ;
			xSemaphoreGive(xMutexADS1115) ;

		}
		vTaskDelay(1000 / portTICK_RATE_MS);

	}

}





void app_main(void)
{
    nvs_flash_init();
    printf("Hello world!\n");
      /* Print chip information */
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    printf("This is %s chip with %d CPU core(s), WiFi%s%s, ",
           CONFIG_IDF_TARGET,
           chip_info.cores  ,
          (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
          (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");
    printf("silicon revision %d, ", chip_info.revision);
    printf("%uMB %s flash\n", spi_flash_get_chip_size() / (1024 * 1024)			    ,
          (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external")  ;
    printf("Minimum free heap size: %d bytes\n", esp_get_minimum_free_heap_size())  ;

    ADS1115_alert_comparator_t alert = { OFF,0, 0,0 };
    ADS1115_config_t ads1115 = {
    			CHANNEL_2_GND,
				FSR_6144  ,
				CONTINIOUS_MODE,
    			SPS_8,
    			alert,
    	  };
    uint8_t init_ads_check = ADS1115init(0b01001000, &ads1115) ;
    printf ("init ads check config: %d\r\n", init_ads_check) ;
    printf("semaphore mutex create ! ") ;
    xMutexADS1115 = xSemaphoreCreateMutex();
    printf("assert_config!") ;
    configASSERT(xMutexADS1115 != NULL) ;
    xTaskCreate(&vTaskVgnd, "voltageDif",2048*10 , (void *)NULL, 5, NULL) ;
    xTaskCreate(&xTaskVdif, "VtaskDif", 2048*10,(void *) NULL, 5  , NULL)				    ;


}

