
#include <stdint.h>
#include <stddef.h>

#include "vicuna_test.h"
#include <riscv_vector.h>
#include "../dbg_printf.h"

/*static int8_t array[512] __attribute__ ((aligned (4)));

void vicuna_test()
{
	int8_t target[sizeof(array)], *src = array, *dst = target;
    size_t vl;

    for (int i = 0; i < sizeof(array); i++)
    	array[i] = i + 1;

    dbg_printf(DEBUG_LEVEL_0, "vicuna_test started...\r\n");

    for (int n = sizeof(array); n > 0; n -= vl, src += vl, dst += vl)
    {
        vl            = __riscv_vsetvl_e8m1(n);
        vint8m1_t vec = __riscv_vle8_v_i8m1(src, vl);
        vec           = __riscv_vmul_vx_i8m1(vec, 5, vl);
        vec           = __riscv_vadd_vx_i8m1(vec, 7, vl);
        __riscv_vse8_v_i8m1(dst, vec, vl);

        dbg_printf(DEBUG_LEVEL_0, "vl: %d\r\n", vl);
    }
    for (int i = 0; i < sizeof(array); i++)
    {
    	dbg_printf(DEBUG_LEVEL_0, "%d * 5 + 7 = %d\r\n", array[i], target[i]);
    }

    dbg_printf(DEBUG_LEVEL_0, "vicuna_test completed...\r\n");
}*/

void vicuna_test_custom_load_bigendian()
{
	int32_t iArrMyArray[8] = {0x10203040, 0x11213141, 0x12223242, 0x13233343, 0x14243444, 0x15253545, 0x16263646, 0x17273747};
	size_t vl;
	int32_t target[sizeof(iArrMyArray)], *src = iArrMyArray, *dst = target;
	vint32m1_t vec;
	int n;

	dbg_printf(DEBUG_LEVEL_0, "vicuna_test_custom_load_bigendian started...\r\n");

	for (n = sizeof(iArrMyArray)/4; n > 0; n -= vl, src += vl/4, dst += vl/4)
	{
		vl  = __riscv_vsetvl_e32m1(n);
	    vec = __builtin_riscv_vlebe32_v(src, vl); // --> Intrinsic for Custom Big Endian Instruction
		__riscv_vse32_v_i32m1(dst, vec, vl);

		dbg_printf(DEBUG_LEVEL_0, "vl: %d\r\n", vl);
	}

	for (int i = 0; i < sizeof(iArrMyArray)/4; i++)
	{
		dbg_printf(DEBUG_LEVEL_0, "CustomLoadBigEndian:: List[%d]: 0x%08x --> 0x%08x\r\n", i, iArrMyArray[i], target[i]);
	}

	dbg_printf(DEBUG_LEVEL_0, "vicuna_test_custom_load_bigendian completed...\r\n");
}

void vicuna_test_slide()
{
	int iArrSrcArray[16] = {0x1111, 0x2222,0x3333,0x4444,0x5555,0x6666,0x7777,0x8888,0x9999,0xAAAA,0xBBBB,0xCCCC,0xDDDD,0xEEEE,0xFFFF,0x1234};
	int iArrDstArray[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	vint32m1_t srcReg;
	vint32m1_t destReg;

	size_t offset = 0;
	size_t vl  = __riscv_vsetvl_e32m1(16);

	for (offset = 0; offset < 16; offset++)
	{
		dbg_printf(DEBUG_LEVEL_0, "vl: %d, offset: %d\r\n", vl, offset);

		srcReg = __riscv_vle32_v_i32m1(iArrSrcArray, vl);
		destReg = __riscv_vle32_v_i32m1(iArrDstArray, vl);

		// Perform the slide-up operation
		destReg = __riscv_vslideup_vx_i32m1(destReg, srcReg, offset, vl);

		__riscv_vse32_v_i32m1(iArrDstArray, destReg, vl);

		for (int i = 0; i < sizeof(iArrSrcArray)/4; i++)
		{
			dbg_printf(DEBUG_LEVEL_0, "SlideUpList[%d]: 0x%08x --> 0x%08x\r\n", i, iArrSrcArray[i], iArrDstArray[i]);
		}
	}

	dbg_printf(DEBUG_LEVEL_0, "\r\n\r\n\r\n");
}

void vicuna_test_custom_rot()
{
	int iArrSrcArray[16] = {0x1111, 0x2222,0x3333,0x4444,0x5555,0x6666,0x7777,0x8888,0x9999,0xAAAA,0xBBBB,0xCCCC,0xDDDD,0xEEEE,0xFFFF,0x1234};
	int iArrDstArray[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	vint32m1_t srcReg;
	vint32m1_t destReg;

	size_t offset = 1;
	size_t vl  = __riscv_vsetvl_e32m1(16);

	for (offset = 0; offset < 16; offset++)
	{
		dbg_printf(DEBUG_LEVEL_0, "vl: %d, offset: %d\r\n", vl, offset);

		srcReg = __riscv_vle32_v_i32m1(iArrSrcArray, vl);
		destReg = __riscv_vle32_v_i32m1(iArrDstArray, vl);

		// Perform the rotup operation
		destReg = __builtin_riscv_vrotup_vx(destReg, srcReg, offset, vl);

		__riscv_vse32_v_i32m1(iArrDstArray, destReg, vl);

		for (int i = 0; i < sizeof(iArrSrcArray)/4; i++)
		{
			dbg_printf(DEBUG_LEVEL_0, "RotUpList[%d]: 0x%08x --> 0x%08x\r\n", i, iArrSrcArray[i], iArrDstArray[i]);
		}
	}

	dbg_printf(DEBUG_LEVEL_0, "\r\n\r\n\r\n");

	for (offset = 0; offset < 16; offset++)
	{
		dbg_printf(DEBUG_LEVEL_0, "vl: %d, offset: %d\r\n", vl, offset);

		srcReg = __riscv_vle32_v_i32m1(iArrSrcArray, vl);
		destReg = __riscv_vle32_v_i32m1(iArrDstArray, vl);

		// Perform the rotdown operation
		destReg = __builtin_riscv_vrotdown_vx(destReg, srcReg, offset, vl);

		__riscv_vse32_v_i32m1(iArrDstArray, destReg, vl);

		for (int i = 0; i < sizeof(iArrSrcArray)/4; i++)
		{
			dbg_printf(DEBUG_LEVEL_0, "RotDownList[%d]: 0x%08x --> 0x%08x\r\n", i, iArrSrcArray[i], iArrDstArray[i]);
		}
	}
}
