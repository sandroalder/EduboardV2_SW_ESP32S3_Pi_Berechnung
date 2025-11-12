/********************************************************************************************* */
//    Eduboard2 ESP32-S3 Template with BSP
//    Author: Martin Burger
//    Juventus Technikerschule
//    Version: 1.0.0
//    
//    This is the ideal starting point for a new Project. BSP for most of the Eduboard2
//    Hardware is included under components/eduboard2.
//    Hardware support can be activated/deactivated in components/eduboard2/eduboard2_config.h
/********************************************************************************************* */
#include "eduboard2.h"
#include "memon.h"

#include "math.h"

#define TAG "TEMPLATE"

  // double hat auf ESP32 64-Bit Präzision
static double Leibniz_sum = 0.0;


void LeibnizTask(void *pvParameters) {
  (void) pvParameters;
    
  uint64_t k = 0;

    while (1) {
        
        // Berechne Term und addiere
        // term = (-1)^k / (2k+1)
        double denom = 2.0 * (double)k + 1.0;
        double term = ((k & 1ULL) == 0ULL) ? (1.0 / denom) : (-1.0 / denom);
        Leibniz_sum += term;

        taskYIELD();
    }
}



void app_main()
{
    //Initialize Eduboard2 BSP
    eduboard2_init();
    
    //Create LeibnizTask
    xTaskCreate(LeibnizTask,   //Subroutine
                "LeibnizTask",     //Name
                2*2048,         //Stacksize
                NULL,           //Parameters
                10,             //Priority
                NULL);          //Taskhandle
    for(;;) {

        vTaskDelay(2000/portTICK_PERIOD_MS);
        ESP_LOGI(TAG, "Pi_calculation");
    }
}