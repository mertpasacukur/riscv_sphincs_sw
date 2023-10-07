/*
 * my_flags.h
 *
 *  Created on: Aug 14, 2023
 *      Author: mpcukur
 */

#ifndef MY_FLAGS_H_
#define MY_FLAGS_H_

/***************************************/
//#define STATIC_FUNCTIONS_ENABLED  PASA: Static fonksiyonlarin statik ozelligini kaldirarak main.h'ta sure olcumu amaciyla kullanabilmek icin olusturuldu.

#ifdef STATIC_FUNCTIONS_ENABLED
	#define MY_STATIC static
#else
	#define MY_STATIC
#endif
/***************************************/

/***************************************/

//#define SPIKE_ENABLED			PASA: Makefile'dan compiler ayarine ekleniyor, "SPIKE_ENABLED" degiskeninin durumuna gore

/***************************************/

#endif /* MY_FLAGS_H_ */
