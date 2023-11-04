#include <stdint.h>
#include <stddef.h>

#include "vicuna_test.h"
#include <riscv_vector.h>
#include "../dbg_printf.h"

//static int8_t array[16] __attribute__ ((aligned (4))) = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
static int8_t array[512] __attribute__ ((aligned (4)));

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
}

static int32_t S_iArrMyArray[8] = {0x10203040, 0x11213141, 0x12223242, 0x13233343, 0x14243444, 0x15253545, 0x16263646, 0x17273747};

void vicuna_test_custom_load_bigendian()
{
	size_t vl;
	int32_t target[sizeof(S_iArrMyArray)], *src = S_iArrMyArray, *dst = target;
	vint32m1_t vec;
	int n;

	dbg_printf(DEBUG_LEVEL_0, "vicuna_test_custom_load_bigendian started...\r\n");

	for (n = sizeof(S_iArrMyArray); n > 0; n -= vl, src += vl/4, dst += vl/4)
	{
		vl  = __riscv_vsetvl_e32m1(n);
	    vec = __builtin_riscv_vlebe32_v(src, vl); // --> Intrinsic for Custom Big Endian Instruction
		__riscv_vse32_v_i32m1(dst, vec, vl);

		dbg_printf(DEBUG_LEVEL_0, "vl: %d\r\n", vl);
	}

	for (int i = 0; i < sizeof(S_iArrMyArray)/4; i++)
	{
		dbg_printf(DEBUG_LEVEL_0, "CustomLoadBigEndian:: List[%d]: 0x%08x --> 0x%08x\r\n", i, S_iArrMyArray[i], target[i]);
	}

	dbg_printf(DEBUG_LEVEL_0, "vicuna_test_custom_load_bigendian completed...\r\n");
}
