
#include "comm_usb.h"
#include "stdint.h"
#include "stdbool.h"
#include "usbd_cdc_if.h"
#include "stm32f0xx_hal_tim.h"


#define _RX_BUFFER_SIZE 500
#define _COMM_USB_PACKET_TIMEOUT_US 500     //us

static TIM_HandleTypeDef timer3;
int8_t vcp_rx_bfr[_RX_BUFFER_SIZE];
static uint32_t message_length, previous_message_length;
// static uint32_t vcp_received_packet_length;
// static uint8_t packet_received;

void COMM_USB_receive_last_packet(void);

void COMM_USB_Init()
{

    timer3.Instance = TIM3;
    timer3.Init.Prescaler = 48;                              // TIM3 clock = 1MHz (APB1 = 48 Mhz)
    timer3.Init.Period = _COMM_USB_PACKET_TIMEOUT_US - 1;    // timeout sto 0.5ms (499us)
    timer3.Init.ClockDivision = 0;
    timer3.Init.CounterMode = TIM_COUNTERMODE_UP;
    
    /*Configure the TIM3 IRQ priority */
    HAL_NVIC_SetPriority(TIM3_IRQn, 2, 0);
    /* Enable the TIM3 global Interrupt */
    HAL_NVIC_EnableIRQ(TIM3_IRQn);
    /* Enable TIM3 clock (Enable the Low Speed APB (APB1) peripheral clock.)*/
    __HAL_RCC_TIM3_CLK_ENABLE();         
    HAL_TIM_Base_Init(&timer3);
    // if (HAL_TIM_Base_Init(&timer3) != HAL_OK)
    // {
    //     return false;
    // }
    // return true;
           
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


void COMM_USB_receive_packet(uint8_t *data_bfr, uint32_t packet_length)
{
    if (message_length + packet_length > _RX_BUFFER_SIZE)
    {
        return;
    }

    memcpy(&vcp_rx_bfr[message_length], data_bfr, packet_length);
    message_length += packet_length;

    //Start the timer used to timeout the received packet
    if(!(timer3.Instance->CR1 & TIM_CR1_CEN))             // If timer not enabled
    {
        timer3.Instance->EGR = TIM_EGR_UG;               // Set UG bit on EGR to generate an update event
        __HAL_TIM_ENABLE_IT(&timer3, TIM_IT_UPDATE);     // Enable interrupts - UIE bit on DIER      = htim.Instance->DIER = TIM_DIER_UIE;
        __HAL_TIM_ENABLE(&timer3);                       // Enable(start) timer - CEN bit on CR1     = htim.Instance->CR1 = TIM_CR1_CEN;
    }
}

void TIM3_IRQHandler(void)
{
    
    if (__HAL_TIM_GET_FLAG(&timer3, TIM_FLAG_UPDATE) == SET)       //check if Update Interrupt Flag is set
    {
        if (__HAL_TIM_GET_IT_SOURCE(&timer3, TIM_IT_UPDATE) == SET)    //Check if the UIE interrupt source is enabled
        {
            COMM_USB_receive_last_packet();
            __HAL_TIM_CLEAR_IT(&timer3, TIM_IT_UPDATE);      //Clear the TIM UIE bit
        }
    }
}

// void COMM_USB_receive_last_packet()
// {
    
//     previous_message_length = message_length;
// }