
#include "mdl_usb.h"
#include "stdint.h"
#include "usbd_cdc_if.h"


#define RX_BUFFER_SIZE 500

uint8_t vcp_received_bfr[RX_BUFFER_SIZE];
static uint32_t vcp_received_packet_length;
static uint8_t packet_received;

void Mdl_USB_Update()
{
    //packet_received = CDC_Receive_FS(uart_rx_ring_buffer, 500);
}

void VCP_receive(uint8_t *data, uint32_t packet_length)
{

}