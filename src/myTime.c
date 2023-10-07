/*
 * myTime.c
 *
 *  Created on: 26 �ub 2023
 *      Author: QP
 */

#include "myTime.h"
#include "my_utils.h"

static unsigned int S_uiArrTimeStampArray[10];
static unsigned int S_uiCounter = 0;

#define POWER_2_32_DIVIDED_BY_50000   85898 // (2 ^ 32) / 50000

unsigned int getCpuCycleLow(void)
{
    unsigned int uiCpuCycleLow = 0;

    asm volatile("csrr %0,mcycle"  : "=r" (uiCpuCycleLow));

    return uiCpuCycleLow;
}

unsigned int getCpuCycleHigh()
{
	unsigned int uiCpuCycleHigh = 0;

	asm volatile("csrr %0,mcycleh"  : "=r" (uiCpuCycleHigh));

	return uiCpuCycleHigh;
}

unsigned int getTime_ms()
{
	//unsigned long long ullCpuCycle = 0;
	unsigned int uiTime_ms = 0;

	// TODO PASA: Dogrudan U64 degeri 100000'e bolerek elde ettigim instructionu (64 bit division) calistiramadigimdan asagidaki yontemi tercih ettim
	//ullCpuCycle = getU64FromTwoU32s(getCpuCycleHigh(), getCpuCycleLow());
	//uiTime_ms = ullCpuCycle / 100000;
	uiTime_ms = (getCpuCycleHigh() * POWER_2_32_DIVIDED_BY_50000) + (getCpuCycleLow() / 50000);

	return uiTime_ms;
}

void resetTimeStamp()
{
	S_uiCounter = 0;
}

void getTimeStamp()
{
	S_uiArrTimeStampArray[S_uiCounter] = getTime_ms();
	S_uiCounter++;
}

unsigned int getTimeDiff_ms(unsigned int uiTimeIndexLast, unsigned int uiTimeIndexFirst)
{
	unsigned int uiTimeDiff_ms = 0;

	if ((uiTimeIndexFirst < S_uiCounter) && (uiTimeIndexLast < S_uiCounter) && (uiTimeIndexLast >= uiTimeIndexFirst))
		uiTimeDiff_ms = S_uiArrTimeStampArray[uiTimeIndexLast] - S_uiArrTimeStampArray[uiTimeIndexFirst];

	return uiTimeDiff_ms;
}
