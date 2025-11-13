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


//#define configTICK_RATE_HZ 1000

// Limite für die Time function
#define Pi_low_limit 3.14159
#define Pi_high_limit 3.14160

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

static volatile uint64_t Leibniz_Tick;
static volatile uint64_t Wallis_Tick;
static volatile bool Leibniz_fin = false;
static volatile bool Wallis_fin = false;


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

static volatile double Leibniz_Pi = 0.0;

void LeibnizTask(void *pvParameters) {
  (void) pvParameters;
    
    static bool add = (true);
    static double Leibniz_sum = 0.0;
    uint64_t k = 1;

    while(1)
    {
        vTaskDelay(pdMS_TO_TICKS(1));
        
        switch (state)
        {
            case Run_Leibniz:

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
            break;

            case Reset:
            
                Leibniz_Pi = 0.0;
                Leibniz_sum = 0.0;
                add = (true);
                k = 1;
                Leibniz_Tick = 0;
                Leibniz_Reset = true;
            break;
            
            default:
                vTaskDelay(pdMS_TO_TICKS(100));
            }
    }
}


/*********************************************************************************************
 *   Wallissches Produkt
 *********************************************************************************************/

static volatile double Wallis_Pi = 0.0;

void WallisTask(void *pvParameters) {
  (void) pvParameters;
    
    static double Wallis_prod = 1.0;
    uint64_t k = 2;

    while(1)
    {
        vTaskDelay(pdMS_TO_TICKS(1));

        switch (state)
        {
            case Run_Wallis:
                Wallis_prod *= ((double)k/ ((double)k-1)) * ((double)k/ ((double)k+1));
                k++;
                k++;
                
                Wallis_Pi = Wallis_prod*2;
            break;

            case Reset:
                Wallis_Pi = 0.0;
                Wallis_prod = 1.0;
                k = 2;
                Wallis_Tick = 0;
                Wallis_Reset = true;
            break;
            
            default:
                vTaskDelay(pdMS_TO_TICKS(100));

        }
    }
}


 /*********************************************************************************************
 *   Time Function
 *********************************************************************************************/

void Time_Function(void *pvParameters) {
  (void) pvParameters;
    
    uint64_t Tick_old = 0;
    uint64_t Tick = 0;
    uint64_t Tick_passed = 0;
    
    
    while(1)
    {
        vTaskDelay(pdMS_TO_TICKS(1));

        Tick = xTaskGetTickCount();
        Tick_passed = Tick - Tick_old;
        Tick_old = Tick;

        switch (state)
        {
            case Run_Leibniz:        
                if (!((Leibniz_Pi > Pi_low_limit) &&  (Leibniz_Pi < Pi_high_limit)))
                {
                    Leibniz_Tick += Tick_passed;
                }
            break;

            case Run_Wallis:
                if (!((Wallis_Pi > Pi_low_limit) &&  (Wallis_Pi < Pi_high_limit)))
                {
                    Wallis_Tick += Tick_passed;
                }
            break;

            default:
                vTaskDelay(pdMS_TO_TICKS(100));
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
        char Time_string[32];
        //Leibnitz
        sprintf(Pi_string, "%.10f", Leibniz_Pi);
        sprintf(Time_string, "%llu", (Leibniz_Tick/configTICK_RATE_HZ));
        lcdDrawString(fx24M, 10, 100, "Leibniz", WHITE);
        lcdDrawString(fx24M, 150, 100, Pi_string, WHITE);
        lcdDrawString(fx24M, 350, 100, Time_string, WHITE);
        lcdDrawString(fx24M, 400, 100, "sek", WHITE);

        //Wallis
        sprintf(Pi_string, "%.10f", Wallis_Pi);
        sprintf(Time_string, "%llu", (Wallis_Tick/configTICK_RATE_HZ));
        lcdDrawString(fx24M, 10, 150, "Wallis", WHITE);
        lcdDrawString(fx24M, 150, 150, Pi_string, WHITE);
        lcdDrawString(fx24M, 350, 150, Time_string, WHITE);
        lcdDrawString(fx24M, 400, 150, "sek", WHITE);

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

    //Create Time Function
    xTaskCreate(Time_Function,   //Subroutine
                "Time_Function", //Name
                2*2048,          //Stacksize
                NULL,            //Parameters
                3,               //Priority
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