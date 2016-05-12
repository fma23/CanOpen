/*
 * CAN module object for Microchip STM32F103 microcontroller.
 *
 * @file        CO_driver.c
 * @version     SVN: \$Id: co_driver.c 42 2013-07-09 11:14:12Z jani22 $
 * @author      Janez Paternoster
 * @author      Ondrej Netik
 * @copyright   2004 - 2013 Janez Paternoster, Ondrej Netik
 *
 * This file is part of CANopenNode, an opensource CANopen Stack.
 * Project home page is <http://canopennode.sourceforge.net>.
 * For more information on CANopen see <http://www.can-cia.org/>.
 *
 * CANopenNode is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */


#include "CO_driver.h"
#include "CO_Emergency.h"
#include "led.h"
#include "stm32f0xx_hal_conf.h"
#include "stm32f0xx_hal_can.h"
#include "stm32f0xx_hal_gpio.h"
#include "stm32f0xx_hal_rcc.h"
#include "stm32f0xx_nucleo.h" 
#include "stm32f072xb.h"

#include "CLibrary.h"
#include "misc.h"
#include <string.h>


static void CO_CANClkSetting (void);
static void CO_CANsendToModule(CO_CANmodule_t *CANmodule, CO_CANtx_t *buffer, uint8_t transmit_mailbox);

//static GPIO_InitTypeDef  GPIO_InitStruct;  
CAN_InitTypeDef   CAN_InitStruct;

CO_ReturnError_t CO_CANmodule_init(

        CO_CANmodule_t *CANmodule,
        CAN_TypeDef *CANbaseAddress,
        CO_CANrx_t *rxArray,
        uint16_t rxSize,
        CO_CANtx_t *txArray,
        uint16_t txSize,
        uint16_t CANbitRate)
{
    CAN_InitTypeDef CAN_InitStruct;
    CAN_FilterConfTypeDef CAN_FilterInitStruct;    
    NVIC_InitTypeDef NVIC_InitStructure;
    int i;
    uint8_t result;

    CANmodule->CANbaseAddress = CANbaseAddress;
    CANmodule->rxArray = rxArray;
    CANmodule->rxSize = rxSize;
    CANmodule->txArray = txArray;
    CANmodule->txSize = txSize;
    CANmodule->bufferInhibitFlag = 0;
    CANmodule->firstCANtxMessage = 1;
    CANmodule->CANtxCount = 0;
    CANmodule->errOld = 0;
    CANmodule->em = 0;

    CAN_ITConfig(CANmodule->CANbaseAddress, 0x03, DISABLE);

    for (i = 0; i < rxSize; i++)
    {
        CANmodule->rxArray[i].ident = 0;
        CANmodule->rxArray[i].pFunct = 0;
    }
    for (i = 0; i < txSize; i++)
    {
        CANmodule->txArray[i].bufferFull = 0;
    }

        /* CO - VJ Changed */
        /* Setting Clock of CAN HW */
        CO_CANClkSetting();

        /* GPIO Config for CAN */
        CO_CANconfigGPIO();

    /* Init CAN controler */
    CAN_DeInit(CANmodule->CANbaseAddress); 

    CAN_StructInit(&CAN_InitStruct); 
   
    switch (CANbitRate)
    {
        case 1000: CAN_InitStruct.Prescaler = 2;
            break;
        case 500: CAN_InitStruct.Prescaler = 4;
            break;
        default:
        case 250: CAN_InitStruct.Prescaler = 8;
            break;
        case 125: CAN_InitStruct.Prescaler = 16;
            break;
        case 100: CAN_InitStruct.Prescaler = 20;
            break;
        case 50: CAN_InitStruct.Prescaler = 120;
            break;
        case 20: CAN_InitStruct.Prescaler = 300;
            break;
        case 10: CAN_InitStruct.Prescaler = 600;
            break;
    }
    CAN_InitStruct.SJW = CAN_SJW_4TQ;     // changed by VJ, old value = CAN_SJW_1tq;  CAN_SJW_4TQ
    CAN_InitStruct.BS1 = CAN_BS1_12TQ;    // changed by VJ, old value = CAN_BS1_3tq;
    CAN_InitStruct.BS2 = CAN_BS2_5TQ;     // changed by VJ, old value = CAN_BS2_2tq;
    CAN_InitStruct.NART = ENABLE;        // No Automatic retransmision

   /* CO - Changed VJ Start */

   result = CAN_Init(CANmodule->CANbaseAddress, &CAN_InitStruct); 

    if (result == 0)
        {
       // TRACE_DEBUG_WP("res=%d\n\r", result);
       return CO_ERROR_TIMEOUT;  /* CO- Return Init failed */
    }
       
    memset(&CAN_FilterInitStruct, 0, sizeof (CAN_FilterInitStruct));
    CAN_FilterInitStruct.FilterNumber = 0;
    CAN_FilterInitStruct.FilterIdHigh = 0;
    CAN_FilterInitStruct.FilterIdLow = 0;
    CAN_FilterInitStruct.FilterMaskIdHigh = 0;
    CAN_FilterInitStruct.FilterMaskIdLow = 0;
    CAN_FilterInitStruct.FilterFIFOAssignment = 0; // pouzivame jen FIFO0
    CAN_FilterInitStruct.FilterMode = CAN_FILTERMODE_IDMASK;
    CAN_FilterInitStruct.FilterScale = CAN_FILTERSCALE_32BIT;
    CAN_FilterInitStruct.FilterActivation = ENABLE;
    CAN_FilterInit(&CAN_FilterInitStruct);


    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStructure.NVIC_IRQChannel = CEC_CAN_IRQn; //USB_LP_CAN1_RX0_IRQn;
    NVIC_Init(&NVIC_InitStructure);

    // preruseni od vysilace
   // NVIC_InitStructure.NVIC_IRQChannel = USB_HP_CAN1_TX_IRQn
    //NVIC_Init(&NVIC_InitStructure);

   // CAN_OperatingModeRequest(CANmodule->CANbaseAddress, CAN_Mode_Normal); // Not needed as after init Can_init functions puts the controller in normal mode - VJ

    CAN_ITConfig(CANmodule->CANbaseAddress, 0x03, ENABLE);

    return CO_ERROR_NO;
}

/******************************************************************************/
void CO_CANmodule_disable(CO_CANmodule_t *CANmodule)
{
    CAN_DeInit(CANmodule->CANbaseAddress);
}

/******************************************************************************/
CO_ReturnError_t CO_CANrxBufferInit(
        CO_CANmodule_t         *CANmodule,
        uint16_t                index,
        uint16_t                ident,
        uint16_t                mask,
        int8_t                  rtr,
        void                   *object,
        void                  (*pFunct)(void *object, CO_CANrxMsg_t *message))
{
    CO_CANrx_t *rxBuffer;
	//CanRxMsg *rxBuffer;
    uint16_t RXF, RXM;

    //safety
    if (!CANmodule || !object || !pFunct || index >= CANmodule->rxSize)
    {
        return CO_ERROR_ILLEGAL_ARGUMENT;
    }

    //buffer, which will be configured
    rxBuffer = CANmodule->rxArray + index;  

    //Configure object variables
    rxBuffer->object = object;
    rxBuffer->pFunct = pFunct;


    //CAN identifier and CAN mask, bit aligned with CAN module registers
    RXF = (ident & 0x07FF) << 2;
    if (rtr) RXF |= 0x02;
    RXM = (mask & 0x07FF) << 2;
    RXM |= 0x02;

    //configure filter and mask
    if (RXF != rxBuffer->ident || RXM != rxBuffer->mask)
    {
        rxBuffer->ident = RXF;
        rxBuffer->mask = RXM;
    }

    return CO_ERROR_NO;
}

/******************************************************************************/
CO_CANtx_t *CO_CANtxBufferInit(
        CO_CANmodule_t         *CANmodule,
        uint16_t                index,
        uint16_t                ident,
        int8_t                  rtr,
        uint8_t                 noOfBytes,
        int8_t                  syncFlag)
{
    uint32_t TXF;
    CO_CANtx_t *buffer;

    //safety
    if (!CANmodule || CANmodule->txSize <= index) return 0;

    //get specific buffer
    buffer = &CANmodule->txArray[index];

    //CAN identifier, bit aligned with CAN module registers

    TXF = ident << 21;
    TXF &= 0xFFE00000;
    if (rtr) TXF |= 0x02;

    //write to buffer
    buffer->ident = TXF;
    buffer->DLC = noOfBytes;
    buffer->bufferFull = 0;
    buffer->syncFlag = syncFlag ? 1 : 0;

    return buffer;
}

int8_t getFreeTxBuff(CO_CANmodule_t *CANmodule)
{
    uint8_t txBuff = 0;
    for (txBuff = 0; txBuff <= 3; txBuff++)
        //if (CAN_TransmitStatus(CANmodule->CANbaseAddress, txBuff) == CAN_TxStatus_Ok)
        switch (txBuff)
        {
        case (CAN_TXMAILBOX_0 ):
            if (CANmodule->CANbaseAddress->TSR & CAN_TSR_TME0 )
                return txBuff;
            else
                break;
        case (CAN_TXMAILBOX_1 ):
            if (CANmodule->CANbaseAddress->TSR & CAN_TSR_TME1 )
                return txBuff;
            else
                break;
        case (CAN_TXMAILBOX_2 ):
            if (CANmodule->CANbaseAddress->TSR & CAN_TSR_TME2 )
                return txBuff;
            else
                break;
        }
    return -1;
}

/******************************************************************************/
CO_ReturnError_t CO_CANsend(CO_CANmodule_t *CANmodule, CO_CANtx_t *buffer)
{
    CO_ReturnError_t err = CO_ERROR_NO;
    int8_t txBuff;

    /* Verify overflow */
    if(buffer->bufferFull)
    {
        if(!CANmodule->firstCANtxMessage)/* don't set error, if bootup message is still on buffers */
            CO_errorReport((CO_EM_t*)CANmodule->em, CO_EM_CAN_TX_OVERFLOW, CO_EMC_CAN_OVERRUN, 0);
        err = CO_ERROR_TX_OVERFLOW;
    }

    CO_DISABLE_INTERRUPTS();
    //if CAN TB buffer0 is free, copy message to it
     txBuff = getFreeTxBuff(CANmodule);
   // #error change this - use only one buffer for transmission - see generic driver
    if(txBuff != -1)
    {
        CANmodule->bufferInhibitFlag = buffer->syncFlag;
        CO_CANsendToModule(CANmodule, buffer, txBuff);
    }
    //if no buffer is free, message will be sent by interrupt
    else
    {
        buffer->bufferFull = 1;
        CANmodule->CANtxCount++;
        // vsechny buffery jsou plny, musime povolit preruseni od vysilace, odvysilat az v preruseni
        CAN_ITConfig(CANmodule->CANbaseAddress, CAN_IT_TME, ENABLE);
    }
    CO_ENABLE_INTERRUPTS();

    return err;
}

/******************************************************************************/
void CO_CANclearPendingSyncPDOs(CO_CANmodule_t *CANmodule)
{

    /* See generic driver for implemetation. */
}

/******************************************************************************/
void CO_CANverifyErrors(CO_CANmodule_t *CANmodule)
{
   uint32_t err;
   CO_EM_t* em = (CO_EM_t*)CANmodule->em;

   err = CANmodule->CANbaseAddress->ESR;
   // if(CAN_REG(CANmodule->CANbaseAddress, C_INTF) & 4) err |= 0x80;

   if(CANmodule->errOld != err)
   {
      CANmodule->errOld = err;

      //CAN RX bus overflow
      if(CANmodule->CANbaseAddress->RF0R & 0x08)
      {
         CO_errorReport(em, CO_EM_CAN_RXB_OVERFLOW, CO_EMC_CAN_OVERRUN, err);
         CANmodule->CANbaseAddress->RF0R &=~0x08;//clear bits
      }

      //CAN TX bus off
      if(err & 0x04) CO_errorReport(em, CO_EM_CAN_TX_BUS_OFF, CO_EMC_BUS_OFF_RECOVERED, err);
      else           CO_errorReset(em, CO_EM_CAN_TX_BUS_OFF, err);

      //CAN TX or RX bus passive
      if(err & 0x02)
      {
         if(!CANmodule->firstCANtxMessage) CO_errorReport(em, CO_EM_CAN_TX_BUS_PASSIVE, CO_EMC_CAN_PASSIVE, err);
      }
      else
      {
        // int16_t wasCleared;
        /* wasCleared = */CO_errorReset(em, CO_EM_CAN_TX_BUS_PASSIVE, err);
        /* if(wasCleared == 1) */CO_errorReset(em, CO_EM_CAN_TX_OVERFLOW, err);
      }


      //CAN TX or RX bus warning
      if(err & 0x01)
      {
         CO_errorReport(em, CO_EM_CAN_BUS_WARNING, CO_EMC_NO_ERROR, err);
      }
      else
      {
         CO_errorReset(em, CO_EM_CAN_BUS_WARNING, err);
      }
   }
}

/******************************************************************************/
// Interrupt from Receiver
void CO_CANinterrupt_Rx(CO_CANmodule_t *CANmodule)
{
	CanRxMsgTypeDef     CAN1_RxMsg;  

	CAN_Receive(CANmodule->CANbaseAddress, CAN_FilterFIFO0, &CAN1_RxMsg);
	{
	        uint16_t index;
	        uint8_t msgMatched = 0;
	        CO_CANrx_t *msgBuff = CANmodule->rxArray;
	        for (index = 0; index < CANmodule->rxSize; index++)
	        {
	            uint16_t msg = (CAN1_RxMsg.StdId << 2) | (CAN1_RxMsg.RTR ? 2 : 0);
	            if (((msg ^ msgBuff->ident) & msgBuff->mask) == 0)
	            {
	                msgMatched = 1;
	                break;
	            }
	            msgBuff++;
	        }
	        //Call specific function, which will process the message
	        if (msgMatched && msgBuff->pFunct)
            msgBuff->pFunct(msgBuff->object, &CAN1_RxMsg);
		}
}

/******************************************************************************/
// Interrupt from Transeiver
void CO_CANinterrupt_Tx(CO_CANmodule_t *CANmodule)
{

     int8_t txBuff;
    /* Clear interrupt flag */
    CAN_ITConfig(CANmodule->CANbaseAddress, CAN_IT_TME, DISABLE); // Transmit mailbox empty interrupt
    /* First CAN message (bootup) was sent successfully */
    CANmodule->firstCANtxMessage = 0;
    /* clear flag from previous message */
    CANmodule->bufferInhibitFlag = 0;
    /* Are there any new messages waiting to be send */
    if(CANmodule->CANtxCount > 0)
    {
        uint16_t i;             /* index of transmitting message */

        /* first buffer */
        CO_CANtx_t *buffer = CANmodule->txArray;
        /* search through whole array of pointers to transmit message buffers. */
        for(i = CANmodule->txSize; i > 0; i--)
        {
            /* if message buffer is full, send it. */
            if(buffer->bufferFull)
            {
                buffer->bufferFull = 0;
                CANmodule->CANtxCount--;
    txBuff = getFreeTxBuff(CANmodule);    //VJ
                /* Copy message to CAN buffer */
                CANmodule->bufferInhibitFlag = buffer->syncFlag;
                CO_CANsendToModule(CANmodule, buffer, txBuff);
                break;                      /* exit for loop */
            }
            buffer++;
        }/* end of for loop */

        /* Clear counter if no more messages */
        if(i == 0) CANmodule->CANtxCount = 0;
    }
}

/******************************************************************************/
void CO_CANinterrupt_Status(CO_CANmodule_t *CANmodule)
{
  // status is evalved with pooling
}

/******************************************************************************/
static void CO_CANsendToModule(CO_CANmodule_t *CANmodule, CO_CANtx_t *buffer, uint8_t transmit_mailbox)
{

    CanTxMsgTypeDef      CAN1_TxMsg;
    int i;

    /*Test code, VJ using Drivers Start*/
    CAN1_TxMsg.IDE = CAN_ID_STD;
    CAN1_TxMsg.DLC = buffer->DLC;
    for (i = 0; i < 8; i++) CAN1_TxMsg.Data[i] = buffer->data[i];
    CAN1_TxMsg.StdId = ((buffer->ident) >> 21);
    CAN1_TxMsg.RTR = CAN_RTR_DATA;


    CAN_Transmit(CANmodule->CANbaseAddress, &CAN1_TxMsg);
    CAN_ITConfig(CANmodule->CANbaseAddress, CAN_IT_TME, ENABLE);



}

/******************************************************************************/
static void CO_CANClkSetting (void)
{
 
    __HAL_RCC_CAN1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
}

/******************************************************************************/         

