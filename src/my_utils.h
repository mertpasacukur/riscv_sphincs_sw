/*
 ============================================================================
 Name        : my_utils.h
 Author      : mpcukur
 Version     :
 Copyright   : Your copyright notice
 Description :
 Date        : 4 Subat 2023
 ============================================================================
 */

#ifndef MY_UTILS_H_
#define MY_UTILS_H_

#include <stdint.h>

#include "dbg_printf.h"

#define MAX(A, B) ((A > B) ? A : B)
#define MIN(A, B) ((A < B) ? A : B)

#define DBG_LINE() dbg_printf(DEBUG_LEVEL_0, "File: %s, Function: %s, Line: %d\r\n", __FILE__, __func__, __LINE__);

void write8BitDataToAddress(unsigned int uiAddress, char cData);
unsigned int read32BitDataFromAddress(unsigned int uiAddress);
void write32BitDataToAddress(unsigned int uiAddress, unsigned int uiValue);

void intToString(int iNumber, char* cpString);
void intToHexString(uint64_t value, char *buffer, int bits);

unsigned long long getU64FromTwoU32s(unsigned int uiMSU32Val, unsigned int uiLSU32Val);

#endif /* MY_UTILS_H_ */
