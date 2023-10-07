/*
 ============================================================================
 Name        : my_utils.c
 Author      : mpcukur
 Version     :
 Copyright   : Your copyright notice
 Description :
 Date        : 4 Subat 2023
 ============================================================================
 */

#include <string.h>
#include <stdlib.h>

#include "my_utils.h"

void write8BitDataToAddress(unsigned int uiAddress, char cData)
{
	*(volatile char*) uiAddress = cData;
}

unsigned int read32BitDataFromAddress(unsigned int uiAddress)
{
	return *(volatile unsigned int *) uiAddress;
}

void write32BitDataToAddress(unsigned int uiAddress, unsigned int uiValue)
{
	*(volatile unsigned int *) uiAddress = uiValue;
}

void intToString(int iNumber, char* cpString)
{
	int i = 0, j = 0, len = 0, temp = iNumber;

	if (iNumber == 0)
	{
		cpString[0] = '0';
		cpString[1] = '\0';
		return;
	}

	if (iNumber < 0)
	{
		cpString[0] = '-';
		iNumber = -iNumber;
		i = 1;
	}

	while (temp != 0)
	{
		len++;
		temp /= 10;
	}

	while (iNumber != 0)
	{
		cpString[i + len - 1 - j] = (iNumber % 10) + '0';
		iNumber /= 10;
		j++;
	}
	cpString[i + len] = '\0';
}

void intToHexString(uint64_t value, char *buffer, int bits)
{
    int numHexDigits = bits / 4;
    for (int i = 0; i < numHexDigits; i++)
    {
        int digitValue = (value >> ((numHexDigits - 1 - i) * 4)) & 0xF;
        buffer[i] = (digitValue < 10) ? ('0' + digitValue) : ('A' + digitValue - 10);
    }
    buffer[numHexDigits] = '\0';
}

unsigned long long getU64FromTwoU32s(unsigned int uiMSU32Val, unsigned int uiLSU32Val)
{
	unsigned long long ullMSU64Val = 0, ullLSU64Val = 0;

	ullMSU64Val = (((unsigned long long)uiMSU32Val << 32) & 0xFFFFFFFF00000000);
	ullLSU64Val = ((unsigned long long)uiLSU32Val & 0x00000000FFFFFFFF);

	return (ullMSU64Val | ullLSU64Val);
}
