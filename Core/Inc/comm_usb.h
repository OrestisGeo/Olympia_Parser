#pragma once

#include "stdint.h"
#include "stdbool.h"

void COMM_USB_Init(void);
void COMM_USB_receive_packet(uint8_t* data_bfr, uint32_t length);
bool COMM_USB_send_packet(uint8_t *data_bfr, uint16_t packet_length);