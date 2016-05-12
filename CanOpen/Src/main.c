/********************************************************************************
  * @file    CanOpen.c
  * @author  Farid Mabrouk
  * @version V1.5.0
  * @date    29-January-2016
  * @brief   This example describes how to configure and use GPIOs through
  *          the STM32F0xx HAL API.
********************************************************************************/

#include "main.h"
#include "CO_NMT_Heartbeat.h"
#include "CO_Emergency.h"
#include "CO_HBconsumer.h"
#include "CO_SYNC.h"
#include "CO_PDO.h"
#include "CO_SDO.h"
#include "stm32f0xx_hal.h"
#include "stm32f0xx_nucleo.h"
#include "CO_driver.h"
#include "CLibrary.h"
#include "led.h"

#include <cross_studio_io.h>


static void SystemClock_Config(void);
static void Error_Handler(void);



int main(void)
{


    uint32_t timer1msDiff;  
    uint32_t t;
    static uint32_t timer1msPrevious;

    CO_NMT_reset_cmd_t reset = CO_RESET_NOT;

    HAL_Init();

  /* Configure the system clock to 48 MHz */
     SystemClock_Config();
  
  /* Enable GPIOA Clock                  */
     LED_GPIOA_CLK_ENABLE();

  /* Enable CAN peripherals              */
     CAN_CLK_ENABLE();            
     CAN_GPIO_CLK_ENABLE();
      
    // debug_printf("This is a pass test");
         
  /* Initialize LEDs                     */
    InitCanLeds();  /* -3- Toggle IOs in an infinite loop */

    CanLedsSet(CoLed_Green);
   // CanLedsSet(CoLed_Red);
   // CanLedsSet(CoLed_Blue);
   // CanLedsSet(CoLed_Yellow);
  

  /* Configure Timer interrupt function for execution every 1 ms*/
    TIMER_InitRCC();
    TIMER_InitGeneral();

    /* Program start - Application Call     */
    programStart();


    while (reset != CO_RESET_APP) {
       
    /* disable timer interrupts */
    NVIC_DisableIRQ(TIM2_IRQn);
    
    /* CANopen communication reset - initialize CANopen objects; CanOpen is initialized from within communication reset function */
    communicationReset();   

      /* start Timer interupts*/
      NVIC_EnableIRQ(TIM2_IRQn);

      reset = CO_RESET_NOT;
      timer1msPrevious = timer1ms;

    while (reset == CO_RESET_NOT) {

   /* loop for normal program execution */
      timer1msDiff = timer1ms - timer1msPrevious;
      timer1msPrevious = timer1ms;


    /* Program Async for SDO and NMT messages */
       reset = programAsync(timer1msDiff);


    // t = getTime_us(&tprof);   // 
       t = 100;
       OD_performance[ODA_performance_mainCycleTime] = t;
       if (t > OD_performance[ODA_performance_mainCycleMaxTime])
       OD_performance[ODA_performance_mainCycleMaxTime] = t;

    } /*  while (reset != 0) */
}     /*  while (reset != 2) */
   
    
    /* program exit  */

    CO_DISABLE_INTERRUPTS();

    CanLedsSet(CoLed_None);
    CanLedsSet(CoLed_Red);

    /* delete CANopen object from memory */
    CO_delete();



}


/*******************************************************************************************************
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow : 
  *            System Clock source            = PLL (HSI48)
  *            SYSCLK(Hz)                     = 48000000
  *            HCLK(Hz)                       = 48000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 1
  *            HSI Frequency(Hz)              = 48000000
  *            PREDIV                         = 2
  *            PLLMUL                         = 2
  *            Flash Latency(WS)              = 1
  * @param  None
  * @retval None
  **********************************************************************************************************/
static void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;
  
  /* Select HSI48 Oscillator as PLL source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48;
  RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI48;
  RCC_OscInitStruct.PLL.PREDIV = RCC_PREDIV_DIV2;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct)!= HAL_OK)
  {
    Error_Handler();
  }

  /* Select PLL as system clock source and configure the HCLK and PCLK1 clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1)!= HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
static void Error_Handler(void)
{
  /* User may add here some code to deal with this error */
  while(1)
  {
  }
}

#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
























