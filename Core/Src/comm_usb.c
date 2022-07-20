
#include "comm_usb.h"
#include "stdint.h"
#include "stdbool.h"
#include "usbd_cdc_if.h"
#include "stm32f0xx_hal_tim.h"


#define _RX_BUFFER_SIZE 500
#define _COMM_USB_PACKET_TIMEOUT_US 500     //us

static TIM_HandleTypeDef htim3;
int8_t vcp_rx_bfr[_RX_BUFFER_SIZE];
static uint16_t previous_packet_length;
// static uint32_t vcp_received_packet_length;
// static uint8_t packet_received;

void COMM_USB_Init()
{

    htim3.Instance = TIM3;
    htim3.Init.Prescaler = 48;                              // TIM3 clock = 1MHz (APB1 = 48 Mhz)
    htim3.Init.Period = _COMM_USB_PACKET_TIMEOUT_US - 1;    // timeout sto 0.5ms (499us)
    htim3.Init.ClockDivision = 0;
    htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
    
    /*Configure the TIM3 IRQ priority */
    HAL_NVIC_SetPriority(TIM3_IRQn, 2, 0);
    /* Enable the TIM3 global Interrupt */
    HAL_NVIC_EnableIRQ(TIM3_IRQn);
    /* Enable TIM3 clock (Enable the Low Speed APB (APB1) peripheral clock.)*/
    __HAL_RCC_TIM3_CLK_ENABLE();         
    HAL_TIM_Base_Init(&htim3);
    // if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
    // {
    //     return false;
    // }
    // return true;
           
}

void COMM_USB_receive_packet(uint8_t *data_bfr, uint32_t packet_length)
{
    //uint8_t bfr_pointer = 
    if (previous_packet_length != packet_length)            //auto einai lathos gt mporei na erthoun 2 64byte paketa(max size)
    {
        memcpy(&vcp_rx_bfr[previous_packet_length], data_bfr, packet_length);
        previous_packet_length = packet_length;
    }

    if(!(htim3.Instance->CR1 & TIM_CR1_CEN))             // If timer not enabled
    {
        htim3.Instance->EGR = TIM_EGR_UG;               // Set UG bit on EGR to generate an update event
        __HAL_TIM_ENABLE_IT(&htim3, TIM_IT_UPDATE);     // Enable interrupts - UIE bit on DIER      = htim.Instance->DIER = TIM_DIER_UIE;
        __HAL_TIM_ENABLE(&htim3);                       // Enable(start) timer - CEN bit on CR1     = htim.Instance->CR1 = TIM_CR1_CEN;
    }
}

bool COMM_USB_send_packet(uint8_t *data_bfr, uint16_t packet_length)
{
    static bool result;
    if ( CDC_Transmit_FS(data_bfr, packet_length) == USBD_OK)
    {
        result = true;
    }
    return result;
}


// void TIM3_IRQHandler(void)
// {
//     if (__HAL_TIM_GET_FLAG(&htim3, TIM_FLAG_UPDATE) != RESET)       //check Update interrupt flag
//     {

//     }
// }