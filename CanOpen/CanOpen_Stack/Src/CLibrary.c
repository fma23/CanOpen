/******************************************************************************************
 * CAN Open 
 *
 * @file        Clibrary.c
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


/*****************************************************************************************
CAN DEINITIALIZATION

******************************************************************************************/
void CAN_DeInit(CAN_TypeDef* CANx)                                 
{
  /* Check the parameters */
  assert_param(IS_CAN_ALL_PERIPH(CANx));
 
  if (CANx == CAN1)
  {
    /* Enable CAN1 reset state */
    __HAL_RCC_CAN1_CLK_ENABLE(); 

    /* Release CAN1 from reset state */
    __HAL_RCC_CAN1_CLK_ENABLE(); 
  }
}

/****************************************************************************
  * @brief  Initializes the CAN peripheral according to the specified
  *         parameters in the CAN_InitStruct.
  * @param  CANx:           where x can be 1 or 2 to to select the CAN 
  *                         peripheral.
  * @param  CAN_InitStruct: pointer to a CAN_InitTypeDef structure that
  *                         contains the configuration information for the 
  *                         CAN peripheral.
  * @retval Constant indicates initialization succeed which will be 
  *         CAN_InitStatus_Failed or CAN_InitStatus_Success.
 *****************************************************************************/

uint8_t CAN_Init(CAN_TypeDef* CANx, CAN_InitTypeDef* CAN_InitStruct){

  uint32_t InitStatus = CAN_INITSTATUS_FAILED;        //data type changed from uint8_t to uint32_t
  uint32_t wait_ack = 0x00000000;

  /* Check the parameters */
  assert_param(IS_CAN_ALL_PERIPH(CANx));
  assert_param(IS_FUNCTIONAL_STATE(CAN_InitStruct->CAN_TTCM));
  assert_param(IS_FUNCTIONAL_STATE(CAN_InitStruct->CAN_ABOM));
  assert_param(IS_FUNCTIONAL_STATE(CAN_InitStruct->CAN_AWUM));
  assert_param(IS_FUNCTIONAL_STATE(CAN_InitStruct->CAN_NART));
  assert_param(IS_FUNCTIONAL_STATE(CAN_InitStruct->CAN_RFLM));
  assert_param(IS_FUNCTIONAL_STATE(CAN_InitStruct->CAN_TXFP));
  assert_param(IS_CAN_MODE(CAN_InitStruct->CAN_Mode));
  assert_param(IS_CAN_SJW(CAN_InitStruct->CAN_SJW));
  assert_param(IS_CAN_BS1(CAN_InitStruct->CAN_BS1));
  assert_param(IS_CAN_BS2(CAN_InitStruct->CAN_BS2));
  assert_param(IS_CAN_PRESCALER(CAN_InitStruct->CAN_Prescaler));

  /* Exit from sleep mode */
  CANx->MCR &= (~(uint32_t)CAN_MCR_SLEEP);                     

  /* Request initialisation */
  CANx->MCR |= CAN_MCR_INRQ ;

  /* Wait the acknowledge */
  while (((CANx->MSR & CAN_MSR_INAK) != CAN_MSR_INAK) && (wait_ack != INAK_TIMEOUT))
      {
        wait_ack++;
      }

      /* Check acknowledge */
      if ((CANx->MSR & CAN_MSR_INAK) != CAN_MSR_INAK)
      {
        InitStatus = CAN_INITSTATUS_FAILED;        //data type changed from uint8_t to uint32_t
      }
      else 
      {
            /* Set the time triggered communication mode */
            if (CAN_InitStruct->TTCM == ENABLE)
            {
              CANx->MCR |= CAN_MCR_TTCM;
            }
            else
            {
              CANx->MCR &= ~(uint32_t)CAN_MCR_TTCM;
            }

            /* Set the automatic bus-off management */
            if (CAN_InitStruct->ABOM == ENABLE)
            {
              CANx->MCR |= CAN_MCR_ABOM;
            }
            else
            {
              CANx->MCR &= ~(uint32_t)CAN_MCR_ABOM;
            }

            /* Set the automatic wake-up mode */
            if (CAN_InitStruct->AWUM == ENABLE)
            {
              CANx->MCR |= CAN_MCR_AWUM;
            }
            else
            {
              CANx->MCR &= ~(uint32_t)CAN_MCR_AWUM;
            }

            /* Set the no automatic retransmission */
            if (CAN_InitStruct->NART == ENABLE)
            {
              CANx->MCR |= CAN_MCR_NART;
            }
            else
            {
              CANx->MCR &= ~(uint32_t)CAN_MCR_NART;
            }

            /* Set the receive FIFO locked mode */
            if (CAN_InitStruct->RFLM == ENABLE)
            {
              CANx->MCR |= CAN_MCR_RFLM;
            }
            else
            {
              CANx->MCR &= ~(uint32_t)CAN_MCR_RFLM;
            }

            /* Set the transmit FIFO priority */
            if (CAN_InitStruct->TXFP == ENABLE)
            {
              CANx->MCR |= CAN_MCR_TXFP;
            }
            else
            {
              CANx->MCR &= ~(uint32_t)CAN_MCR_TXFP;
            }

    /* Set the bit timing register */
    CANx->BTR = (uint32_t)((uint32_t)CAN_InitStruct->Mode << 30) | \
                ((uint32_t)CAN_InitStruct->SJW << 24) | \
                ((uint32_t)CAN_InitStruct->BS1 << 16) | \
                ((uint32_t)CAN_InitStruct->BS2 << 20) | \
               ((uint32_t)CAN_InitStruct->Prescaler - 1);

    /* Request leave initialisation */
    CANx->MCR &= ~(uint32_t)CAN_MCR_INRQ;

   /* Wait the acknowledge */
   wait_ack = 0;

   while (((CANx->MSR & CAN_MSR_INAK) == CAN_MSR_INAK) && (wait_ack != INAK_TIMEOUT))
   {
     wait_ack++;
   }

        /* ...and check acknowledged */
        if ((CANx->MSR & CAN_MSR_INAK) == CAN_MSR_INAK)
        {
          InitStatus = CAN_INITSTATUS_FAILED;        //data type changed from uint8_t to uint32_t  
        }
        else
        {
          InitStatus = CAN_INITSTATUS_SUCCESS;        //data type changed from uint8_t to uint32_t
        }
 
    }
  /* At this step, return the status of initialization */
  return InitStatus;

}


/**
  * @brief  Initializes the CAN peripheral according to the specified
  *         parameters in the CAN_FilterInitStruct.
  * @param  CAN_FilterInitStruct: pointer to a CAN_FilterInitTypeDef
  *                               structure that contains the configuration 
  *                               information.
  * @retval None.
  */
void CAN_FilterInit(CAN_FilterConfTypeDef* CAN_FilterInitStruct)        // data type changed from CAN_FilterInitTypeDef to CAN_FilterConfTypeDef
{
  uint32_t filter_number_bit_pos = 0;
  /* Check the parameters */
  assert_param(IS_CAN_FILTER_NUMBER(CAN_FilterInitStruct->CAN_FilterNumber));
  assert_param(IS_CAN_FILTER_MODE(CAN_FilterInitStruct->CAN_FilterMode));
  assert_param(IS_CAN_FILTER_SCALE(CAN_FilterInitStruct->CAN_FilterScale));
  assert_param(IS_CAN_FILTER_FIFO(CAN_FilterInitStruct->CAN_FilterFIFOAssignment));
  assert_param(IS_FUNCTIONAL_STATE(CAN_FilterInitStruct->CAN_FilterActivation));

  filter_number_bit_pos = ((uint32_t)1) << CAN_FilterInitStruct->FilterNumber;

  /* Initialisation mode for the filter */
  CAN1->FMR |= (uint32_t)CAN_FMR_FINIT; // Originally value FMR_FINIT;

  /* Filter Deactivation */
  CAN1->FA1R &= ~(uint32_t)filter_number_bit_pos;

  /* Filter Scale */
  if (CAN_FilterInitStruct->FilterScale == CAN_FILTERSCALE_32BIT) //  it was CAN_FilterScale_16bit)        
  {
    /* 16-bit scale for the filter */
    CAN1->FS1R &= ~(uint32_t)filter_number_bit_pos;

    /* First 16-bit identifier and First 16-bit mask */
    /* Or First 16-bit identifier and Second 16-bit identifier */
    CAN1->sFilterRegister[CAN_FilterInitStruct->FilterNumber].FR1 = 
    ((0x0000FFFF & (uint32_t)CAN_FilterInitStruct->FilterMaskIdLow) << 16) |
        (0x0000FFFF & (uint32_t)CAN_FilterInitStruct->FilterIdLow);

    /* Second 16-bit identifier and Second 16-bit mask */
    /* Or Third 16-bit identifier and Fourth 16-bit identifier */
    CAN1->sFilterRegister[CAN_FilterInitStruct->FilterNumber].FR2 = 
    ((0x0000FFFF & (uint32_t)CAN_FilterInitStruct->FilterMaskIdHigh) << 16) |
        (0x0000FFFF & (uint32_t)CAN_FilterInitStruct->FilterIdHigh);
  }

  if (CAN_FilterInitStruct->FilterScale == CAN_FILTERSCALE_32BIT)
  {
    /* 32-bit scale for the filter */
    CAN1->FS1R |= filter_number_bit_pos;
    /* 32-bit identifier or First 32-bit identifier */
    CAN1->sFilterRegister[CAN_FilterInitStruct->FilterNumber].FR1 = 
    ((0x0000FFFF & (uint32_t)CAN_FilterInitStruct->FilterIdHigh) << 16) |
        (0x0000FFFF & (uint32_t)CAN_FilterInitStruct->FilterIdLow);
    /* 32-bit mask or Second 32-bit identifier */
    CAN1->sFilterRegister[CAN_FilterInitStruct->FilterNumber].FR2 = 
    ((0x0000FFFF & (uint32_t)CAN_FilterInitStruct->FilterMaskIdHigh) << 16) |
        (0x0000FFFF & (uint32_t)CAN_FilterInitStruct->FilterMaskIdLow);
  }

  /* Filter Mode */
  if (CAN_FilterInitStruct->FilterMode == CAN_FILTERMODE_IDMASK) // it was CAN_FilterMode_IdMask)
  {
    /*Id/Mask mode for the filter*/
    CAN1->FM1R &= ~(uint32_t)filter_number_bit_pos;
  }
  else /* CAN_FilterInitStruct->CAN_FilterMode == CAN_FilterMode_IdList */
  {
    /*Identifier list mode for the filter*/
    CAN1->FM1R |= (uint32_t)filter_number_bit_pos;
  }

  /* Filter FIFO assignment */
  if (CAN_FilterInitStruct->FilterFIFOAssignment == CAN_FILTER_FIFO0) //it was  CAN_Filter_FIFO0)
  {
    /* FIFO 0 assignation for the filter */
    CAN1->FFA1R &= ~(uint32_t)filter_number_bit_pos;
  }

  if (CAN_FilterInitStruct->FilterFIFOAssignment == CAN_FILTER_FIFO1) 
  {
    /* FIFO 1 assignation for the filter */
    CAN1->FFA1R |= (uint32_t)filter_number_bit_pos;
  }
  
  /* Filter activation */
  if (CAN_FilterInitStruct->FilterActivation == ENABLE)
  {
    CAN1->FA1R |= filter_number_bit_pos;
  }

  /* Leave the initialisation mode for the filter */
  CAN1->FMR &=  ~((uint32_t)CAN_FMR_FINIT); //~FMR_FINIT;


}

/**
  * @brief  Fills each CAN_InitStruct member with its default value.
  * @param  CAN_InitStruct: pointer to a CAN_InitTypeDef structure which
  *                         will be initialized.
  * @retval None.
  */
void CAN_StructInit(CAN_InitTypeDef* CAN_InitStruct)
{
  /* Reset CAN init structure parameters values */
  
  /* Initialize the time triggered communication mode */
  CAN_InitStruct->TTCM = DISABLE;
  
  /* Initialize the automatic bus-off management */
  CAN_InitStruct->ABOM = DISABLE;
  
  /* Initialize the automatic wake-up mode */
  CAN_InitStruct->AWUM = DISABLE;
  
  /* Initialize the no automatic retransmission */
  CAN_InitStruct->NART = DISABLE;
  
  /* Initialize the receive FIFO locked mode */
  CAN_InitStruct->RFLM = DISABLE;
  
  /* Initialize the transmit FIFO priority */
  CAN_InitStruct->TXFP = DISABLE;

  /* Initialize the CAN_Mode member */
  CAN_InitStruct->Mode = CAN_MODE_NORMAL;  
  
  /* Initialize the CAN_SJW member */
  CAN_InitStruct->SJW = CAN_SJW_1TQ;
  
  /* Initialize the CAN_BS1 member */
  CAN_InitStruct->BS1 = CAN_BS1_4TQ;
  
  /* Initialize the CAN_BS2 member */
  CAN_InitStruct->BS2 = CAN_BS2_3TQ;
  
  /* Initialize the CAN_Prescaler member */
  CAN_InitStruct->Prescaler = 1;
 
}

///**********************************************************************************/
void CAN_ITConfig(CAN_TypeDef* CANx, uint32_t CAN_IT, FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_CAN_ALL_PERIPH(CANx));
  assert_param(IS_CAN_IT(CAN_IT));
  assert_param(IS_FUNCTIONAL_STATE(NewState));

    if (NewState != DISABLE)
    {
      /* Enable the selected CANx interrupt */
      CANx->IER |= CAN_IT;
    }
    else
    {
      /* Disable the selected CANx interrupt */
      CANx->IER &= ~CAN_IT;
    }
}

/**
  * @brief  Initiates the transmission of a message.
  * @param  CANx:      where x can be 1 or 2 to to select the CAN peripheral.
  * @param  TxMessage: pointer to a structure which contains CAN Id, CAN
  *                    DLC and CAN data.
  * @retval The number of the mailbox that is used for transmission
  *                    or CAN_TxStatus_NoMailBox if there is no empty mailbox.
  */
uint8_t CAN_Transmit(CAN_TypeDef* CANx, CanTxMsgTypeDef* TxMessage)  
{
  uint8_t transmit_mailbox = 0;
  /* Check the parameters */
  assert_param(IS_CAN_ALL_PERIPH(CANx));
  assert_param(IS_CAN_IDTYPE(TxMessage->IDE));
  assert_param(IS_CAN_RTR(TxMessage->RTR));
  assert_param(IS_CAN_DLC(TxMessage->DLC));

      /* Select one empty transmit mailbox */
      if ((CANx->TSR&CAN_TSR_TME0) == CAN_TSR_TME0)
      {
        transmit_mailbox = 0;
      }
      else if ((CANx->TSR&CAN_TSR_TME1) == CAN_TSR_TME1)
      {
        transmit_mailbox = 1;
      }
      else if ((CANx->TSR&CAN_TSR_TME2) == CAN_TSR_TME2)
      {
        transmit_mailbox = 2;
      }
      else
      {
        transmit_mailbox = CAN_TXSTATUS_NOMAILBOX;     /* CAN cell did not provide CAN_TxStatus_NoMailBox */
      }

  if (transmit_mailbox != CAN_TXSTATUS_NOMAILBOX)
  {
    /* Set up the Id */
    CANx->sTxMailBox[transmit_mailbox].TIR &= TMIDxR_TXRQ;
        if (TxMessage->IDE == CAN_ID_STD)  // it was CAN_Id_Standard
        {
          assert_param(IS_CAN_STDID(TxMessage->StdId));  
          CANx->sTxMailBox[transmit_mailbox].TIR |= ((TxMessage->StdId << 21) | TxMessage->RTR);
        }
        
        else
         {
      assert_param(IS_CAN_EXTID(TxMessage->ExtId));
      CANx->sTxMailBox[transmit_mailbox].TIR |= ((TxMessage->ExtId << 3) |TxMessage->IDE | TxMessage->RTR);
         } 
    /* Set up the DLC */
    TxMessage->DLC &= (uint8_t)0x0000000F;
    CANx->sTxMailBox[transmit_mailbox].TDTR &= (uint32_t)0xFFFFFFF0;
    CANx->sTxMailBox[transmit_mailbox].TDTR |= TxMessage->DLC;

    /* Set up the data field */
    CANx->sTxMailBox[transmit_mailbox].TDLR = (((uint32_t)TxMessage->Data[3] << 24) | 
                                             ((uint32_t)TxMessage->Data[2] << 16) |
                                             ((uint32_t)TxMessage->Data[1] << 8) | 
                                             ((uint32_t)TxMessage->Data[0]));

    CANx->sTxMailBox[transmit_mailbox].TDHR = (((uint32_t)TxMessage->Data[7] << 24) | 
                                             ((uint32_t)TxMessage->Data[6] << 16) |
                                             ((uint32_t)TxMessage->Data[5] << 8) |
                                             ((uint32_t)TxMessage->Data[4]));
    /* Request transmission */
    CANx->sTxMailBox[transmit_mailbox].TIR |= TMIDxR_TXRQ;
  }

  return transmit_mailbox;

}

/*********************************************************************************
  * @brief  Receives a message.
  * @param  CANx:       where x can be 1 or 2 to to select the CAN peripheral.
  * @param  FIFONumber: Receive FIFO number, CAN_FIFO0 or CAN_FIFO1.
  * @param  RxMessage:  pointer to a structure receive message which contains
  *                     CAN Id, CAN DLC, CAN datas and FMI number.
  * @retval None.
  ********************************************************************************/

void CAN_Receive(CAN_TypeDef* CANx, uint8_t FIFONumber, CanRxMsgTypeDef* RxMessage)
{
  /* Check the parameters */
  assert_param(IS_CAN_ALL_PERIPH(CANx));
  assert_param(IS_CAN_FIFO(FIFONumber));
  /* Get the Id */
  RxMessage->IDE = (uint8_t)0x04 & CANx->sFIFOMailBox[FIFONumber].RIR;
  if (RxMessage->IDE == CAN_ID_STD)
  {
    RxMessage->StdId = (uint32_t)0x000007FF & (CANx->sFIFOMailBox[FIFONumber].RIR >> 21);
  }
  else
  {
    RxMessage->ExtId = (uint32_t)0x1FFFFFFF & (CANx->sFIFOMailBox[FIFONumber].RIR >> 3);
  }
  
  RxMessage->RTR = (uint8_t)0x02 & CANx->sFIFOMailBox[FIFONumber].RIR;
  /* Get the DLC */
  RxMessage->DLC = (uint8_t)0x0F & CANx->sFIFOMailBox[FIFONumber].RDTR;
  /* Get the FMI */
  RxMessage->FMI = (uint8_t)0xFF & (CANx->sFIFOMailBox[FIFONumber].RDTR >> 8);
  /* Get the data field */
  RxMessage->Data[0] = (uint8_t)0xFF & CANx->sFIFOMailBox[FIFONumber].RDLR;
  RxMessage->Data[1] = (uint8_t)0xFF & (CANx->sFIFOMailBox[FIFONumber].RDLR >> 8);
  RxMessage->Data[2] = (uint8_t)0xFF & (CANx->sFIFOMailBox[FIFONumber].RDLR >> 16);
  RxMessage->Data[3] = (uint8_t)0xFF & (CANx->sFIFOMailBox[FIFONumber].RDLR >> 24);
  RxMessage->Data[4] = (uint8_t)0xFF & CANx->sFIFOMailBox[FIFONumber].RDHR;
  RxMessage->Data[5] = (uint8_t)0xFF & (CANx->sFIFOMailBox[FIFONumber].RDHR >> 8);
  RxMessage->Data[6] = (uint8_t)0xFF & (CANx->sFIFOMailBox[FIFONumber].RDHR >> 16);
  RxMessage->Data[7] = (uint8_t)0xFF & (CANx->sFIFOMailBox[FIFONumber].RDHR >> 24);
  /* Release the FIFO */
  /* Release FIFO0 */
  if (FIFONumber == CAN_FIFO0)
  {
    CANx->RF0R |= CAN_RF0R_RFOM0;
  }
  /* Release FIFO1 */
  else /* FIFONumber == CAN_FIFO1 */
  {
    CANx->RF1R |= CAN_RF1R_RFOM1;
  }
}
  
   
/**********************************************************************************/
/**CAN GPIOs configuration
/*********************************************************************************/
void CO_CANconfigGPIO (void)
{

    GPIO_InitTypeDef GPIO_InitStructure;     

        /* Remap */
   // GPIO_PinRemapConfig(GPIO_Remapping_CAN, GPIO_CAN_Remap_State);
    /* Configure CAN pin: RX */
    GPIO_InitStructure.Pin = GPIO_Pin_CAN_RX;
    GPIO_InitStructure.Mode = GPIO_MODE_INPUT; //was af_IPU
    HAL_GPIO_Init(CAN_RX_GPIO_PORT, &GPIO_InitStructure);      

    /* Configure CAN pin: TX */
    GPIO_InitStructure.Pin = GPIO_Pin_CAN_TX;
    GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;  // it was GPIO_Mode_AF_PP;
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH; // it waas GPIO_Speed_50MHz;
    HAL_GPIO_Init(CAN_TX_GPIO_PORT, &GPIO_InitStructure);

}

