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


// Zustände der State Machine
typedef enum {
    Idle,
    Run_Leibniz,
    Run_Wallis,
    Reset
} State_t;

static volatile State_t state = Idle;
static volatile bool Leibniz_Reset = false;
static volatile bool Wallis_Reset = false;

/*********************************************************************************************
 *   SteuerTask
 *********************************************************************************************/

void Steuertask(void* param) {
    
    while(1) {
    
        vTaskDelay(pdMS_TO_TICKS(100));
        
        if(button_get_state(SW0, true) == SHORT_PRESSED) {
            state = Idle;
        }
        if(button_get_state(SW1, true) == SHORT_PRESSED) {
             state = Run_Leibniz;
        }
        if(button_get_state(SW2, true) == SHORT_PRESSED) {
            state = Run_Wallis;
        }
        if(button_get_state(SW3, true) == SHORT_PRESSED) {
            state = Reset;
            while (!(Leibniz_Reset && Wallis_Reset)){
                vTaskDelay(pdMS_TO_TICKS(10));      // Warte bis Reset abgeschlossen
            }  
            Leibniz_Reset = false;
            Wallis_Reset = false;
            state = Idle;
        }

    }
}

/*********************************************************************************************
 *   Leibnitz Reihe
 *********************************************************************************************/

static double Leibniz_Pi = 0.0;

void LeibnizTask(void *pvParameters) {
  (void) pvParameters;
    
    static bool add = (true);
    static double Leibniz_sum = 0.0;
    uint64_t k = 1;

    while(1)
    {
        if (state == Idle)
            vTaskDelay(pdMS_TO_TICKS(100));

        while (state == Run_Leibniz) {

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

        if (state == Reset)
        {
            Leibniz_Pi = 0.0;
            Leibniz_sum = 0.0;
            add = (true);
            k = 1;
            Leibniz_Reset = true;
        }
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

    while(1)
    {
        if (state == Idle)
            vTaskDelay(pdMS_TO_TICKS(100));
       
        while (state == Run_Wallis) {

            vTaskDelay(pdMS_TO_TICKS(1));

            Wallis_prod *= ((double)k/ ((double)k-1)) * ((double)k/ ((double)k+1));
            k++;
            k++;
            
            Wallis_Pi = Wallis_prod*2;
        }

        if (state == Reset)
        {
            Wallis_Pi = 0.0;
            Wallis_prod = 1.0;
            k = 2;
            Wallis_Reset = true;
        }
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
    
    //Create SteuertTask
    xTaskCreate(Steuertask,     //Subroutine
                "Steuertask",   //Name
                2*2048,         //Stacksize
                NULL,           //Parameters
                15,             //Priority
                NULL);          //Taskhandle


    //Create LeibnizTask
    xTaskCreate(LeibnizTask,    //Subroutine
                "LeibnizTask",  //Name
                2*2048,         //Stacksize
                NULL,           //Parameters
                5,              //Priority
                NULL);          //Taskhandle

    //Create WallisTask
    xTaskCreate(WallisTask,      //Subroutine
                "WallisTask",    //Name
                2*2048,          //Stacksize
                NULL,            //Parameters
                5,               //Priority
                NULL);           //Taskhandle

    //Create LCD Update
    xTaskCreate(LCD_update,     //Subroutine
                "LCD_update",   //Name
                2*2048,         //Stacksize
                NULL,           //Parameters
                10,             //Priority
                NULL);          //Taskhandle


    for(;;) {

        vTaskDelay(2000/portTICK_PERIOD_MS);
    }
}