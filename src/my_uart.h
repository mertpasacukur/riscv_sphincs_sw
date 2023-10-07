/*
 ============================================================================
 Name        : uart.h
 Author      : mpcukur
 Version     :
 Copyright   : Your copyright notice
 Description :
 Date        : 4 Subat 2023
 ============================================================================
 */

#ifndef MY_UART_H_
#define MY_UART_H_

#define UART_TX_DATA_ADDRESS   			0xFF000000	// PicoRV32 Implementation: 0x40000000
#define UART_TX_TRANSMIT_DONE_ADDRESS   0xFF000004	// PicoRV32 Implementation: 0x40000004

typedef enum
{
	enUartTxDone = 0x0,
	enUartTxBusy = 0x1
} EUartTxDoneStatus;

void sendUartByteWaitTransmitDone(char cSendByte);

#endif /* UART_H_ */
