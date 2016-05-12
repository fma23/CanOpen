/**
 ******************************************************************************
 * @file    isr.c
 * @author  VJ
 * @version v1.0
 * @date    19-Jul-2013
 * @brief   Interrupt Service Routine source file
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "isr.h"
#include "timer.h"
#include "stm32f0xx.h"
#include "CANopen.h"
#include "application.h"
#include "stm32f0xx_hal_tim.h"
#include "CLibrary.h"
#include  "led.h"



void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
 * @brief  Description
 * @input  None
 * @output None
 * @return None
*/
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
 * @brief  Description
 * @input  None
 * @output None
 * @return None
*/
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}


///******************************************************************************/
///*            Peripherals Interrupt Handlers                                  */
///******************************************************************************/
//
///**
// * @brief  ISR for USB Low Priority Interrupt
// * @input  None
// * @output None
// * @return None
//*/
//void USB_LP_IRQHandler(void)
//{
//  //USB_Istr();
//}
//
///**
// * @brief  ISR for CAN RX Interrupt
// * @input  None
// * @output None
// * @return None
//*/
void USB_LP_CAN1_RX0_IRQHandler(void)
{
    CO_CANinterrupt_Rx(CO->CANmodule[0]);
}

/**
 * @brief  ISR for CAN TX Interrupt
 * @input  None
 * @output None
 * @return None
*/
void USB_HP_CAN1_TX_IRQHandler(void)
{
    CO_CANinterrupt_Tx(CO->CANmodule[0]);
}

/**
  * @brief  This function handles TIM2 overflow and update interrupt request.
            This is 1ms periodic interrupt.
  * @param  None
  * @retval None
 */
void TIM2_IRQHandler(void)
{
  TIM_ClearITPendingBit(TIM2, TIM_IT_UPDATE);      /* Clear the Update pending bit */

  timer1ms++;

  /* Periodic cyclic function call */
  //if(timer1ms % 1 == 0)
  //{
    CanLedsTOGGLE(CoLed_Green);  // used for testing timer.
    CanLedsTOGGLE(CoLed_Red);  // used for testing timer.
  //  debug_printf("This is a timer interrupt");
  //  program1ms();
  //}

}


