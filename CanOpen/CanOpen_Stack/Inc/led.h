/**
 ******************************************************************************
 * @file    led.h
 * @author  Farid Mabrouk
 * @version v1.0
 * @date    21-Nov-2012
 * @brief   led file header
 ******************************************************************************/
 
#ifndef __LED_H
#define __LED_H

/* Includes ------------------------------------------------------------------*/
#include "common.h"
#include "stm32f0xx.h"
#include "stdint.h"
#include "stm32f0xx_hal.h"
#include "stm32f0xx_hal_def.h"
#include "stm32f072xb.h"
#include "stm32f0xx_hal_can.h"
#include "CO_driver.h"
#include "stm32f072xb.h" 
#include "stm32f0xx_hal_tim.h"


/* Init CAN Led Interface */
typedef enum {
    CoLed_None = 0,
    CoLed_Green = 1,
    CoLed_Red = 2,
    CoLed_Blue = 3,
    CoLed_Yellow = 4,
} CoLeds;



/* LEDs Functions           */
void InitCanLeds(void);
void CanLedsSet(CoLeds LED);
void CanLedsOFF(CoLeds LED);
void CanLedsTOGGLE(CoLeds LED);



#endif /* __LED_H */




