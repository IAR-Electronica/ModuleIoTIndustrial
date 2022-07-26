#include <ADS1115.h>
#include <stdio.h>


void vTaskGetADC(void *pv){
    float voltage ; 
    ///!adc_init ! 
    while(1){
        voltage =getVoltage() ; 
        printf("voltage: %f",voltage ) ; 
    }

    //! tirar abajo la tarea 
}

