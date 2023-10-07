/*
 ============================================================================
 Name        : dbg_printf.h
 Author      : mpcukur
 Version     :
 Copyright   : Your copyright notice
 Description :
 Date        : 4 Subat 2023
 ============================================================================
 */

#ifndef DBG_PRINTF_H_
#define DBG_PRINTF_H_

#define DEBUG_LEVEL_0	0
#define DEBUG_LEVEL_1	1
#define DEBUG_LEVEL_2	2
#define DEBUG_LEVEL_3	3
#define DEBUG_LEVEL_4	4
#define DEBUG_LEVEL_5	5
#define DEBUG_LEVEL_6	6

void setDebugLevel(unsigned int uiDebugLevel);
void dbg_printf(unsigned int uiDebugLevel, const char *format, ...);

void __attribute__((no_instrument_function)) printFloat(float value);

#endif /* DBG_PRINTF_H_ */
