#include "mdl_usb.h"
#include "usbd_cdc_if.h"
#include "stdbool.h"

static uint8_t usb_cdc_rdy;
uint8_t temp_rx_buffer[1000];

// bool Mdl_Usb_CDC_Init()
// {
//     usb_cdc_rdy = CDC_Init_FS();
//     if (usb_cdc_rdy==0)
//     {
//         return true;
//     }
//     else
//     {
//         return false;
//     }
// }

void Mdl_Usb_CDC_Update()
{
    if ((data_received_flag) && (packet_is_pending == false))       // if previous packet is not read, then don't receive any other data from UART
    {
        data_received_flag = false ;
        packet_length = Drv_USART_Bytes_To_Read();
        Drv_USART_Read(temp_rx_buffer, &packet_length);
        bool received_packet_rdy = Comm_Parse_Received_Data(&com_control.received_struct, temp_rx_buffer, packet_length);
        if (received_packet_rdy == true)
        {
            if (read_data(&com_control.received_struct))    //if everything ok then parse data frame and save on led_info struct
            {
                packet_is_pending       = true;
            }
        }
    }
}