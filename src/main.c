/********************************************************************************************* */
//    Pi calculation
//    Author: Sandro Alder
//    Juventus Technikerschule
//    Version: 1.0.0
//    
/********************************************************************************************* */


#include "eduboard2.h"
#include "memon.h"
#include "math.h"


#include <stdlib.h>          
#include "esp_system.h"      


/*********************************************************************************************
 *   Leibnitz Reihe
 *********************************************************************************************/

static double Leibniz_Pi = 0.0;

void LeibnizTask(void *pvParameters) {
  (void) pvParameters;
    
    static bool add = (true);
    static double Leibniz_sum = 0.0;
    uint64_t k = 1;

    while (1) {

    vTaskDelay(pdMS_TO_TICKS(1));
        
        if (add)
        {
            add = false;
            Leibniz_sum += 1/(double)k;
        }
        else
        {
            add = true;
            Leibniz_sum -= 1/(double)k;
        }
        k++;
        k++;

        Leibniz_Pi = Leibniz_sum*4;
    }
}

/*********************************************************************************************
 *   Wallissches Produkt
 *********************************************************************************************/

static double Wallis_Pi = 0.0;

void WallisTask(void *pvParameters) {
  (void) pvParameters;
    
    static double Wallis_prod = 1.0;
    uint64_t k = 2;

    while (1) {
        
        vTaskDelay(pdMS_TO_TICKS(1));

        Wallis_prod *= ((double)k/ ((double)k-1)) * ((double)k/ ((double)k+1));
        k++;
        k++;
        
        Wallis_Pi = Wallis_prod*2;


    }
}



/*********************************************************************************************
 *   LCD update
 *********************************************************************************************/

void LCD_update(void* param) {
    
    
    while(1) {
    
        vTaskDelay(pdMS_TO_TICKS(500));
        lcdFillScreen(BLACK);

        lcdDrawString(fx32M, 10, 30, "Pi calculation", WHITE);
        
        char Pi_string[32];
        //Leibnitz
        sprintf(Pi_string, "%.10f", Leibniz_Pi);
        lcdDrawString(fx24M, 10, 100, "Leibnitz", WHITE);
        lcdDrawString(fx24M, 200, 100, Pi_string, WHITE);
        //Wallis
        sprintf(Pi_string, "%.10f", Wallis_Pi);
        lcdDrawString(fx24M, 10, 200, "Wallis", WHITE);
        lcdDrawString(fx24M, 200, 200, Pi_string, WHITE);

        lcdUpdateVScreen();

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
                5,             //Priority
                NULL);          //Taskhandle

    //Create WallisTask
    xTaskCreate(WallisTask,   //Subroutine
                "WallisTask",     //Name
                2*2048,         //Stacksize
                NULL,           //Parameters
                5,             //Priority
                NULL);          //Taskhandle

    //Create LCD Update
    xTaskCreate(LCD_update,   //Subroutine
                "LCD_update",     //Name
                2*2048,         //Stacksize
                NULL,           //Parameters
                10,             //Priority
                NULL);          //Taskhandle


    for(;;) {

        vTaskDelay(2000/portTICK_PERIOD_MS);
    }
}