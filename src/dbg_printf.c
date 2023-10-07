/*
 ============================================================================
 Name        : dbg_printf.c
 Author      : mpcukur
 Version     :
 Copyright   : Your copyright notice
 Description :
 Date        : 4 Subat 2023
 ============================================================================
 */

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "dbg_printf.h"

#include "my_utils.h"
#include "my_flags.h"
#include "my_uart.h"

static unsigned int S_uiDebugLevel = DEBUG_LEVEL_2;

void setDebugLevel(unsigned int uiDebugLevel)
{
	if (uiDebugLevel > DEBUG_LEVEL_6)
		uiDebugLevel = DEBUG_LEVEL_6;

	S_uiDebugLevel = uiDebugLevel;
}

void dbg_printf(unsigned int uiDebugLevel, const char *format, ...)
{
	#ifdef SPIKE_ENABLED

	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);

	#else

	if (uiDebugLevel <= S_uiDebugLevel)
	{
		va_list args;
		va_start(args, format);

		while (*format)
		{
			if (*format == '%')
			{
				format++;
				int width = 0;

				if (*format == '-')
				{
					format++;
					while (*format >= '0' && *format <= '9')
					{
						width = width * 10 + (*format - '0');
						format++;
					}
				}

				switch (*format)
				{
				case 'd':
				{
					int i = va_arg(args, int);
					char buf[16];
					intToString(i, buf);
					int strLength = strlen(buf);

					for (char *p = buf; *p; p++)
					{
						sendUartByteWaitTransmitDone(*p);
					}
					// Pad with spaces if alignment is specified
					for (int i = strLength; i < width; i++)
					{
						sendUartByteWaitTransmitDone(' ');
					}
				}
				break;
				case '0':
					width = 0;
					// Parse the width value
					while (*format >= '0' && *format <= '9')
					{
						width = width * 10 + (*format - '0');
						format++;
					}

		            if (*format == 'x')
		            {
		                uint32_t value = va_arg(args, uint32_t);
		                char buf[9];
		                intToHexString(value, buf, 32); // 32 bits for uint32_t
		                // Pad with leading zeros to achieve the desired width
		                for (int i = strlen(buf); i < width; i++)
		                {
		                    sendUartByteWaitTransmitDone('0');
		                }
		                for (char *p = buf; *p; p++)
		                {
		                    sendUartByteWaitTransmitDone(*p);
		                }
		            }
				break;
				case 'l':
		            if (*format == 'l' && *(format + 1) == 'l' && (*(format + 2) == 'x' || *(format + 2) == 'X'))
		            {
		                // Extract the 64-bit value using two 32-bit integers
		                uint32_t high = va_arg(args, uint32_t);
		                uint32_t low = va_arg(args, uint32_t);
		                uint64_t value = ((uint64_t)high << 32) | low;
		                char buf[17];
		                intToHexString(value, buf, 64); // 64 bits for uint64_t
		                for (char *p = buf; *p; p++) {
		                    sendUartByteWaitTransmitDone(*p);
		                }
		                format += 2; // Skip the 'llX'
		            }
				break;
	            case 's':
	            {
	            	char *str = va_arg(args, char*);
	            	int strLength = strlen(str);
	            	for (int i = 0; i < strLength; i++)
	            	{
	            		sendUartByteWaitTransmitDone(str[i]);
	            	}
	            	// Pad with spaces to achieve the desired width
	            	for (int i = strLength; i < width; i++)
	            	{
	            		sendUartByteWaitTransmitDone(' ');
	            	}
	            }
				break;
				default:
					sendUartByteWaitTransmitDone(*format);
					break;
				}
			}
			else
			{
				sendUartByteWaitTransmitDone(*format);
			}
			format++;
		}

		va_end(args);
	}

	#endif
}

static void __attribute__((no_instrument_function)) floatToString(float value, char *buffer, int width, int precision)
{
	int intPart = (int)value;
	float fractionalPart = value - intPart;
	int i = 0;

	// Convert integer part
	int intPartLength = 0;
	int temp = intPart;
	do {
		intPartLength++;
		temp /= 10;
	} while (temp);

	// Padding with spaces
	while (i < width - intPartLength - precision - 1) {
		buffer[i++] = ' ';
	}

	// Convert integer part
	int digitsIndex = i + intPartLength - 1;
	do {
		buffer[digitsIndex--] = (intPart % 10) + '0';
		intPart /= 10;
	} while (intPart);

	i += intPartLength;

	// Add decimal point
	buffer[i++] = '.';

	// Convert fractional part with specified precision
	for (int p = 0; p < precision; p++) {
		fractionalPart *= 10;
	}
	int fracPartAsInt = (int)(fractionalPart + 0.5); // Round to nearest integer

	// Add fractional digits
	for (int p = 0; p < precision; p++) {
		buffer[i + precision - p - 1] = (fracPartAsInt % 10) + '0';
		fracPartAsInt /= 10;
	}

	i += precision;

	// Null-terminate the string
	buffer[i] = '\0';
}

void __attribute__((no_instrument_function)) printFloat(float value)
{
	#ifdef SPIKE_ENABLED

	printf("%8.4f", value);

	#else

    char buf[32];
    floatToString(value, buf, 8, 4);

    for (char *p = buf; *p; p++)
    {
        sendUartByteWaitTransmitDone(*p);
    }

	#endif
}
