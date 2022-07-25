#include "comm_usb.h"
#include "stdint.h"
#include "stdbool.h"
#include "usbd_cdc_if.h"
#include "stm32f0xx_hal_tim.h"
#include "stm32f0xx_ll_tim.h"
#include "stm32f0xx_ll_bus.h"

#define _RX_BUFFER_SIZE (500)
#define _COMM_USB_PACKET_TIMEOUT_US (10000)     //us
#define _MAX_PACKET_SIZE (64)

// static COMM_PROTOCOL_Control_t pc_comm_slave_control;
// static bool pc_comm_transmit(COMM_PROTOCOL_Transmit_Packet_t *tx_struct);

static TIM_HandleTypeDef htim3;
//static TIM_MasterConfigTypeDef sMasterConfig = {0};
LL_TIM_InitTypeDef TIM_InitStruct = {0};

int8_t vcp_rx_bfr[_RX_BUFFER_SIZE];
static uint32_t message_length;
static bool message_received_flag;

// static uint32_t vcp_received_packet_length;
// static uint8_t packet_received;

void COMM_USB_last_packet_is_64_bytes(void);

void COMM_USB_Init()
{
    /* USER CODE END TIM3_Init 1 */
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM3);

    uint32_t prescaler = 47;
    TIM_InitStruct.Prescaler = prescaler;
    TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
    TIM_InitStruct.Autoreload = 500;
    TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
    LL_TIM_Init(TIM3, &TIM_InitStruct);
    LL_TIM_EnableARRPreload(TIM3);
    LL_TIM_SetClockSource(TIM3, LL_TIM_CLOCKSOURCE_INTERNAL);
    LL_TIM_SetTriggerOutput(TIM3, LL_TIM_TRGO_RESET);
    LL_TIM_SetUpdateSource(TIM3, LL_TIM_UPDATESOURCE_COUNTER);
    LL_TIM_DisableMasterSlaveMode(TIM3);

    /*Configure the TIM3 IRQ priority */
    NVIC_SetPriority(TIM3_IRQn, 0);
    /* Enable the TIM3 global Interrupt */
    NVIC_EnableIRQ(TIM3_IRQn);
           
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
        if (LL_TIM_IsEnabledCounter(TIM3))           //If TIM3 is enabled  (TIM3->CR1 & TIM_CR1_CEN)
        {
            LL_TIM_DisableIT_UPDATE(TIM3);
            LL_TIM_DisableCounter(TIM3);
        }
        memcpy(&vcp_rx_bfr[message_length], data_bfr, packet_length);
        message_length += packet_length;
        message_received_flag = true;
    }
    else if (packet_length == _MAX_PACKET_SIZE)
    {
        // Start the timer used to timeout the received packet
        //If there is an interrupt then it means that the last packet was exactly 64 bytes
        if (!(LL_TIM_IsEnabledCounter(TIM3))) // If timer not enabled
        {
            //TIM3->EGR = TIM_EGR_UG;           // Set UG bit on EGR to generate an update event- W R O N G it causes interrupt as long as timer starts
            LL_TIM_ClearFlag_UPDATE(TIM3);
            LL_TIM_EnableIT_UPDATE(TIM3);
            LL_TIM_EnableCounter(TIM3);
        }
        else
        {
            /* Restart counter */
            LL_TIM_DisableUpdateEvent(TIM3);    //Disable UEV to restart counter
            LL_TIM_EnableUpdateEvent(TIM3);

            // TIM3->CR1 = TIM_CR1_UDIS;        //Disable UEV to restart counter
            // CLEAR_BIT(TIM3->CR1, TIM_CR1_UDIS);
        }
        memcpy(&vcp_rx_bfr[message_length], data_bfr, packet_length);
        message_length += packet_length;
    }
}


void TIM3_IRQHandler(void)
{
    
    if (LL_TIM_IsActiveFlag_UPDATE(TIM3))       //check if Update Interrupt Flag is set
    {
        if (LL_TIM_IsEnabledIT_UPDATE(TIM3))    //Check if the UIE interrupt source is enabled
        {
            COMM_USB_last_packet_is_64_bytes();
            LL_TIM_ClearFlag_UPDATE(TIM3);
            LL_TIM_DisableIT_UPDATE(TIM3);
        }
    }
}

void COMM_USB_last_packet_is_64_bytes()
{
    message_received_flag = true;
    if (LL_TIM_IsEnabledCounter(TIM3))
    {
        LL_TIM_DisableCounter(TIM3);
    }
}