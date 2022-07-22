
#include "comm_usb.h"
#include "stdint.h"
#include "stdbool.h"
#include "usbd_cdc_if.h"
#include "stm32f0xx_hal_tim.h"

#define _RX_BUFFER_SIZE (500)
#define _COMM_USB_PACKET_TIMEOUT_US (500)     //us
#define _MAX_PACKET_SIZE (64)

// static COMM_PROTOCOL_Control_t pc_comm_slave_control;
// static bool pc_comm_transmit(COMM_PROTOCOL_Transmit_Packet_t *tx_struct);

static TIM_HandleTypeDef htim3;
static TIM_MasterConfigTypeDef sMasterConfig = {0};

int8_t vcp_rx_bfr[_RX_BUFFER_SIZE];
static uint32_t message_length;
static bool message_received_flag;

// static uint32_t vcp_received_packet_length;
// static uint8_t packet_received;
static uint32_t counter;

void COMM_USB_last_packet_is_64_bytes(void);

void COMM_USB_Init()
{
    /* Initialize TIM3 peripheral 
        Prescaler = 480000000 / 48 = 1MHz timer counter clock
        Period = XXX us (1/1000000 = 1us each timer clock cycle)
    */
    htim3.Instance = TIM3;
    htim3.Init.Prescaler = 65500;                          // TIM3 clock = 1MHz (APB1 = 48 Mhz)
    htim3.Init.Period = _COMM_USB_PACKET_TIMEOUT_US;        // timeout at 0.5ms
    htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    
    /* Enable TIM3 clock (Enable the Low Speed APB (APB1) peripheral clock.)*/
    __HAL_RCC_TIM3_CLK_ENABLE();
    HAL_TIM_Base_Init(&htim3);     //inits timer too?

    /*Configure the TIM3 IRQ priority */
    HAL_NVIC_SetPriority(TIM3_IRQn, 0, 0);
    /* Enable the TIM3 global Interrupt */
    HAL_NVIC_EnableIRQ(TIM3_IRQn);
    
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig);

    // if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
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

// void COMM_USB_transmit_packet(COMM_PROTOCOL_Transmit_Packet_t *tx_struct)
// {

// }

void COMM_USB_receive_packet(uint8_t *data_bfr, uint32_t packet_length)
{
    if (message_length + packet_length > _RX_BUFFER_SIZE)           //If packet is bigger than my buffer then ignore it
    {
        return;
    }

    if (message_received_flag)
    {
        message_received_flag = false;
        message_length = 0;                     // If message completed write vcp_rx_bfr from start
    }

    
    if (packet_length < _MAX_PACKET_SIZE)                  //If packet_length<64 then stop TIM3 since the message is complete     
    {
        if (htim3.Instance->CR1 & TIM_CR1_CEN)           //If TIM3 is enabled  
        {
            __HAL_TIM_DISABLE_IT(&htim3, TIM_IT_UPDATE);
            __HAL_TIM_DISABLE(&htim3);
        }
        memcpy(&vcp_rx_bfr[message_length], data_bfr, packet_length);
        message_length += packet_length;
        message_received_flag = true;
    }
    else if (packet_length == _MAX_PACKET_SIZE)
    {
        // Start the timer used to timeout the received packet
        //If there is an interrupt then it means that the last packet was exactly 64 bytes
        if (!(htim3.Instance->CR1 & TIM_CR1_CEN)) // If timer not enabled
        {
            htim3.Instance->EGR = TIM_EGR_UG;           // Set UG bit on EGR to generate an update event
            __HAL_TIM_ENABLE_IT(&htim3, TIM_IT_UPDATE); // Enable interrupts - UIE bit on DIER      = htim.Instance->DIER = TIM_DIER_UIE;
            __HAL_TIM_ENABLE(&htim3);                   // Enable(start) timer - CEN bit on CR1     = htim.Instance->CR1 = TIM_CR1_CEN;
        }
        else
        {
            htim3.Instance->CR1 = TIM_CR1_UDIS;        //Disable UEV to restart counter     or __HAL_TIM_SET_COUNTER(&htim3, 0);
            htim3.Instance->EGR = TIM_EGR_UG;
        }
        memcpy(&vcp_rx_bfr[message_length], data_bfr, packet_length);
        message_length += packet_length;
    }
}


void TIM3_IRQHandler(void)
{
    
    if (__HAL_TIM_GET_FLAG(&htim3, TIM_FLAG_UPDATE) == SET)       //check if Update Interrupt Flag is set
    {
        if (__HAL_TIM_GET_IT_SOURCE(&htim3, TIM_IT_UPDATE) == SET)    //Check if the UIE interrupt source is enabled
        {

            __HAL_TIM_GET_COUNTER(&htim3);
            COMM_USB_last_packet_is_64_bytes();
            __HAL_TIM_CLEAR_IT(&htim3, TIM_IT_UPDATE);      //Clear the TIM UIE bit
        }
    }
}

void COMM_USB_last_packet_is_64_bytes()
{
    message_received_flag = true;
}