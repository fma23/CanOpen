/******************************************************************************************
 * CAN Open 
 *
 * @file        LED.c
 * @version    
 * @author      Farid Mabrouk
 ******************************************************************************************/

#include "stm32f0xx_hal.h"
#include "stm32f0xx_hal_def.h"
#include "stm32f072xb.h"
#include "stm32f0xx_hal_can.h"
#include "CO_driver.h"
#include "CLibrary.h"
#include "stm32f0xx_hal_gpio.h"
#include "stm32f0xx_nucleo.h"
#include "stm32f072xb.h"
#include "timer.h"
#include "led.h"

/************************************************************************************/
/* LEDS Initialization */
/************************************************************************************/
void InitCanLeds(void)
{

  /*configure IOs in output push-pull mode to drive external LEDs */
  GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull  = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;


  GPIO_InitStruct.Pin = LED1|LED2|LED3|LED4;              // equivalent to GPIO_PIN_5|GPIO_PIN_6;

  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

}

/************************************************************************************
LEDS Turn ON
************************************************************************************/

void CanLedsSet(CoLeds led)
{
   if (led==CoLed_Green)
      HAL_GPIO_WritePin(GPIOA, LED1, GPIO_PIN_SET);

  if(led == CoLed_Red)
       HAL_GPIO_WritePin(GPIOA, LED2, GPIO_PIN_SET);

    else if(led == CoLed_Blue)
       HAL_GPIO_WritePin(GPIOA, LED3, GPIO_PIN_SET);
          
    else if(led == CoLed_Yellow)
       HAL_GPIO_WritePin(GPIOA, LED4, GPIO_PIN_SET);

     else 
       HAL_GPIO_WritePin(GPIOA, (LED1|LED2|LED3|LED4), GPIO_PIN_RESET);

}
/************************************************************************************
LEDS turn OFF 
************************************************************************************/
 void CanLedsOFF(CoLeds led)
{
    if (led & CoLed_Green)
       HAL_GPIO_WritePin(GPIOA, LED1, GPIO_PIN_RESET);
    else if(led & CoLed_Red)
       HAL_GPIO_WritePin(GPIOA, LED2, GPIO_PIN_RESET);
    else if(led & CoLed_Blue)
       HAL_GPIO_WritePin(GPIOA, LED3, GPIO_PIN_RESET);   
    else if(led & CoLed_Yellow)
       HAL_GPIO_WritePin(GPIOA, LED4, GPIO_PIN_RESET);
    else 
       HAL_GPIO_WritePin(GPIOA, (LED1|LED2|LED3|LED4), GPIO_PIN_RESET);
}    
/************************************************************************************
LEDS turn OFF 
************************************************************************************/
 void CanLedsTOGGLE(CoLeds led)
{
    if (led & CoLed_Green)
       HAL_GPIO_TogglePin(GPIOA, LED1);
    else if(led & CoLed_Red)
       HAL_GPIO_TogglePin(GPIOA, LED2);
    else if(led & CoLed_Blue)
       HAL_GPIO_TogglePin(GPIOA, LED3);   
    else if(led & CoLed_Yellow)
        HAL_GPIO_TogglePin(GPIOA, LED4);
    else 
       HAL_GPIO_WritePin(GPIOA, (LED1|LED2|LED3|LED4), GPIO_PIN_RESET);
}   
