/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : usbd_cdc_if.c
  * @version        : v2.0_Cube
  * @brief          : Usb device for Virtual Com Port.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "usbd_cdc_if.h"

/* USER CODE BEGIN INCLUDE */

/* USER CODE END INCLUDE */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

/* USER CODE END PV */

/** @addtogroup STM32_USB_OTG_DEVICE_LIBRARY
  * @brief Usb device library.
  * @{
  */

/** @addtogroup USBD_CDC_IF
  * @{
  */

/** @defgroup USBD_CDC_IF_Private_TypesDefinitions USBD_CDC_IF_Private_TypesDefinitions
  * @brief Private types.
  * @{
  */

/* USER CODE BEGIN PRIVATE_TYPES */
typedef struct
{
  uint8_t *data;    //Will point to the buffer  
  int wr_index;           //write index
  int rd_index;           //read index
  int lb_index;           //loopback index
} VCP_FIFO;

/* USER CODE END PRIVATE_TYPES */

/**
  * @}
  */

/** @defgroup USBD_CDC_IF_Private_Defines USBD_CDC_IF_Private_Defines
  * @brief Private defines.
  * @{
  */

/* USER CODE BEGIN PRIVATE_DEFINES */

#define RX_BUFFER_MAX_WRITE_INDEX (APP_RX_DATA_SIZE - CDC_DATA_FS_MAX_PACKET_SIZE)    //the limit index after which the circular buffer will loopback

/* USER CODE END PRIVATE_DEFINES */

/**
  * @}
  */

/** @defgroup USBD_CDC_IF_Private_Macros USBD_CDC_IF_Private_Macros
  * @brief Private macros.
  * @{
  */

/* USER CODE BEGIN PRIVATE_MACRO */

/* USER CODE END PRIVATE_MACRO */

/**
  * @}
  */

/** @defgroup USBD_CDC_IF_Private_Variables USBD_CDC_IF_Private_Variables
  * @brief Private variables.
  * @{
  */
/* Create buffer for reception and transmission           */
/* It's up to user to redefine and/or remove those define */
/** Received data over USB are stored in this buffer      */
uint8_t UserRxBufferFS[APP_RX_DATA_SIZE];

/** Data to send over USB CDC are stored in this buffer   */
uint8_t UserTxBufferFS[APP_TX_DATA_SIZE];

/* USER CODE BEGIN PRIVATE_VARIABLES */
//Circular FIFO to store outgoing data until is can be sent over USB
VCP_FIFO vcp_tx_fifo;
//Circular FIFO to store incoming data until is can be sent over USB
VCP_FIFO vcp_rx_fifo;

/* USER CODE END PRIVATE_VARIABLES */

/**
  * @}
  */

/** @defgroup USBD_CDC_IF_Exported_Variables USBD_CDC_IF_Exported_Variables
  * @brief Public variables.
  * @{
  */

extern USBD_HandleTypeDef hUsbDeviceFS;

/* USER CODE BEGIN EXPORTED_VARIABLES */

/* USER CODE END EXPORTED_VARIABLES */

/**
  * @}
  */

/** @defgroup USBD_CDC_IF_Private_FunctionPrototypes USBD_CDC_IF_Private_FunctionPrototypes
  * @brief Private functions declaration.
  * @{
  */

static int8_t CDC_Init_FS(void);
static int8_t CDC_DeInit_FS(void);
static int8_t CDC_Control_FS(uint8_t cmd, uint8_t* pbuf, uint16_t length);
static int8_t CDC_Receive_FS(uint8_t* pbuf, uint32_t *Len);

/* USER CODE BEGIN PRIVATE_FUNCTIONS_DECLARATION */

/* USER CODE END PRIVATE_FUNCTIONS_DECLARATION */

/**
  * @}
  */

USBD_CDC_ItfTypeDef USBD_Interface_fops_FS =
{
  CDC_Init_FS,
  CDC_DeInit_FS,
  CDC_Control_FS,
  CDC_Receive_FS
};

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Initializes the CDC media low layer over the FS USB IP
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_Init_FS(void)
{
  /* USER CODE BEGIN 3 */
  //Circular FIFO initializations :
  vcp_tx_fifo.data = UserTxBufferFS;     //Use the default buffer
  vcp_tx_fifo.wr_index   = 0;
  vcp_tx_fifo.rd_index   = 0;
  vcp_tx_fifo.lb_index   = 0;

  vcp_rx_fifo.data = UserRxBufferFS;    //Use the default buffer
  vcp_rx_fifo.wr_index   = 0;
  vcp_rx_fifo.rd_index   = 0;
  vcp_rx_fifo.lb_index   = 0;

  /* Set Application Buffers */
  USBD_CDC_SetTxBuffer(&hUsbDeviceFS, UserTxBufferFS, 0);
  USBD_CDC_SetRxBuffer(&hUsbDeviceFS, UserRxBufferFS);
  return (USBD_OK);
  /* USER CODE END 3 */
}

/**
  * @brief  DeInitializes the CDC media low layer
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_DeInit_FS(void)
{
  /* USER CODE BEGIN 4 */
  return (USBD_OK);
  /* USER CODE END 4 */
}

/**
  * @brief  Manage the CDC class requests
  * @param  cmd: Command code
  * @param  pbuf: Buffer containing command data (request parameters)
  * @param  length: Number of data to be sent (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_Control_FS(uint8_t cmd, uint8_t* pbuf, uint16_t length)
{
  /* USER CODE BEGIN 5 */
  switch(cmd)
  {
    case CDC_SEND_ENCAPSULATED_COMMAND:

    break;

    case CDC_GET_ENCAPSULATED_RESPONSE:

    break;

    case CDC_SET_COMM_FEATURE:

    break;

    case CDC_GET_COMM_FEATURE:

    break;

    case CDC_CLEAR_COMM_FEATURE:

    break;

  /*******************************************************************************/
  /* Line Coding Structure                                                       */
  /*-----------------------------------------------------------------------------*/
  /* Offset | Field       | Size | Value  | Description                          */
  /* 0      | dwDTERate   |   4  | Number |Data terminal rate, in bits per second*/
  /* 4      | bCharFormat |   1  | Number | Stop bits                            */
  /*                                        0 - 1 Stop bit                       */
  /*                                        1 - 1.5 Stop bits                    */
  /*                                        2 - 2 Stop bits                      */
  /* 5      | bParityType |  1   | Number | Parity                               */
  /*                                        0 - None                             */
  /*                                        1 - Odd                              */
  /*                                        2 - Even                             */
  /*                                        3 - Mark                             */
  /*                                        4 - Space                            */
  /* 6      | bDataBits  |   1   | Number Data bits (5, 6, 7, 8 or 16).          */
  /*******************************************************************************/
    case CDC_SET_LINE_CODING:

    break;

    case CDC_GET_LINE_CODING:

    break;

    case CDC_SET_CONTROL_LINE_STATE:

    break;

    case CDC_SEND_BREAK:

    break;

  default:
    break;
  }

  return (USBD_OK);
  /* USER CODE END 5 */
}

/**
  * @brief  Data received over USB OUT endpoint are sent over CDC interface
  *         through this function.
  *
  *         @note
  *         This function will issue a NAK packet on any OUT packet received on
  *         USB endpoint until exiting this function. If you exit this function
  *         before transfer is complete on CDC interface (ie. using DMA controller)
  *         it will result in receiving more data while previous ones are still
  *         not sent.
  *
  * @param  Buf: Buffer of data to be received
  * @param  Len: Number of data received (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_Receive_FS(uint8_t* Buf, uint32_t *Len)
{
  /* USER CODE BEGIN 6 */
  vcp_rx_fifo.wr_index += *Len;
  // If the new value is too close to the end of the ring buffer
  if(vcp_rx_fifo.wr_index >= RX_BUFFER_MAX_WRITE_INDEX)
  {
    //Wrap around buffer and save wr on lb
    vcp_rx_fifo.lb_index = vcp_rx_fifo.wr_index;
    vcp_rx_fifo.wr_index = 0;
  }
  //Where(on the rx ring buffer) to write the received data
  USBD_CDC_SetRxBuffer(&hUsbDeviceFS, vcp_rx_fifo.data + vcp_rx_fifo.wr_index);
  USBD_CDC_ReceivePacket(&hUsbDeviceFS);
  return (USBD_OK);
  /* USER CODE END 6 */
}

/**
  * @brief  CDC_Transmit_FS
  *         Data to send over USB IN endpoint are sent over CDC interface
  *         through this function.
  *         @note
  *
  *
  * @param  Buf: Buffer of data to be sent
  * @param  Len: Number of data to be sent (in bytes)
  * @retval USBD_OK if all operations are OK else USBD_FAIL or USBD_BUSY
  */
uint8_t CDC_Transmit_FS(uint8_t* Buf, uint16_t Len)
{
  uint8_t result = USBD_OK;
  /* USER CODE BEGIN 7 */
  USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef*)hUsbDeviceFS.pClassData;
  if (hcdc->TxState != 0){
    return USBD_BUSY;
  }
  USBD_CDC_SetTxBuffer(&hUsbDeviceFS, Buf, Len);
  result = USBD_CDC_TransmitPacket(&hUsbDeviceFS);
  /* USER CODE END 7 */
  return result;
}




/* USER CODE BEGIN PRIVATE_FUNCTIONS_IMPLEMENTATION */
int vcp_receive_data(uint8_t* buf, uint32_t len)
{
  // Compute how much data is in the FIFO
  int cap = vcp_rx_fifo.wr_index - vcp_rx_fifo.rd_index;
  if (cap == 0)
    return 0;      // Empty FIFO, no data to read
  if (cap < 0)  // FIFO contents wrap around
    cap += vcp_rx_fifo.lb_index;  // Notice the use of lb
  // Limit the FIFO read to the available data
  if (len > cap)
    len = cap;
  // Save len : it'll be the return value
  int retval = len;
  // Read the data
  while (len)
  {
    len--;
    *buf = vcp_rx_fifo.data[vcp_rx_fifo.rd_index];
    buf++;
    vcp_rx_fifo.rd_index++;    // Update read index
    if (vcp_rx_fifo.rd_index == vcp_rx_fifo.lb_index)  // Check for wrap-around
      vcp_rx_fifo.rd_index = 0;      // Follow wrap-around
  }
  return retval;
}


int vcp_send (uint8_t* buf, uint16_t len)
{
  // Step 1 : calculate the occupied space in the Tx FIFO
  int cap = vcp_tx_fifo.wr_index - vcp_tx_fifo.rd_index;   // occupied capacity
  if (cap < 0)    // FIFO contents wrap around
    cap += APP_TX_DATA_SIZE;
  cap = APP_TX_DATA_SIZE - cap;      // available capacity

  // Step 2 : compare with argument
  if (cap < len)
    return -1;   // Not enough room to copy "buf" into the FIFO => error

  // Step 3 : does buf fit in the tail ?
  int tail = APP_TX_DATA_SIZE - vcp_tx_fifo.wr_index;
  if (tail >= len)
  {
    // Copy buf into the tail of the FIFO
    memcpy (&vcp_tx_fifo.data[vcp_tx_fifo.wr_index], buf, len);
    // Update "wr" index
    vcp_tx_fifo.wr_index += len;
    // In case "len" == "tail", next write goes to the head
    if (vcp_tx_fifo.wr_index == APP_TX_DATA_SIZE)
      vcp_tx_fifo.wr_index = 0;   
  }
  else
  {
    // Copy the head of "buf" to the tail of the FIFO
    memcpy (&vcp_tx_fifo.data[vcp_tx_fifo.wr_index], buf, tail);
    // Copy the tail of "buf" to the head of the FIFO :
    memcpy (vcp_tx_fifo.data, &buf[tail], len - tail);
    // Update the "wr" index
    vcp_tx_fifo.wr_index = len - tail;
  }
  return 0;  // successful completion
}
/* USER CODE END PRIVATE_FUNCTIONS_IMPLEMENTATION */

/**
  * @}
  */

/**
  * @}
  */
