/*
 ============================================================================
 Name        : uart.c
 Author      : mpcukur
 Version     :
 Copyright   : Your copyright notice
 Description :
 Date        : 4 Subat 2023
 ============================================================================
 */

#include "my_uart.h"
#include "my_utils.h"

void sendUartByteWaitTransmitDone(char cSendByte)
{
	// wait until transmitter ready:
	while((read32BitDataFromAddress(UART_TX_TRANSMIT_DONE_ADDRESS) & 0x1) == enUartTxBusy);
	write8BitDataToAddress(UART_TX_DATA_ADDRESS, cSendByte);
}
