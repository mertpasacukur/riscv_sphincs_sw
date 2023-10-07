/*
 * myTime.h
 *
 *  Created on: 26 �ub 2023
 *      Author: QP
 */

#ifndef MYTIME_H_
#define MYTIME_H_

unsigned int getCpuCycleLow(void);
unsigned int getCpuCycleHigh();
unsigned int getCpuCycles();
unsigned int getTime_ms();
void resetTimeStamp();
void getTimeStamp();
unsigned int getTimeDiff_ms(unsigned int uiTimeIndexLast, unsigned int uiTimeIndexFirst);

#endif /* MYTIME_H_ */
