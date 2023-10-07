#define _POSIX_C_SOURCE 199309L

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "../api.h"
#include "../fors.h"
#include "../wots.h"
#include "../params.h"
#include "../rng.h"

#include "../../dbg_printf.h"
#include "../../my_utils.h"
#include "../../myTime.h"
#include "../../my_flags.h"

#define SPX_MLEN 32

unsigned char m[SPX_MLEN];
unsigned char sm[(SPX_BYTES + SPX_MLEN)];
unsigned char mout[(SPX_BYTES + SPX_MLEN)];

int benchmark()
{
	dbg_printf(DEBUG_LEVEL_0, "\r\nBenchmark initializing... m_len:%d, sm_len:%d, mout_len:%d\r\n", SPX_MLEN, (SPX_BYTES + SPX_MLEN), (SPX_BYTES + SPX_MLEN));

	int iStatus = 0;
	unsigned int uiTotalTime_ms = 0, uiTimeDiff_ms = 0;
    unsigned char pk[SPX_PK_BYTES];
    unsigned char sk[SPX_SK_BYTES];

    unsigned char fors_pk[SPX_FORS_PK_BYTES];
    unsigned char fors_m[SPX_FORS_MSG_BYTES];
    unsigned char fors_sig[SPX_FORS_BYTES];
    unsigned char addr[SPX_ADDR_BYTES];

    unsigned char wots_sig[SPX_WOTS_BYTES];
    unsigned char wots_m[SPX_N];
    unsigned char wots_pk[SPX_WOTS_PK_BYTES];

    unsigned long long smlen;
    unsigned long long mlen;

    dbg_printf(DEBUG_LEVEL_0, "Parameters: n = %d, h = %d, d = %d, b = %d, k = %d, w = %d\r\n",
             SPX_N, SPX_FULL_HEIGHT, SPX_D, SPX_FORS_HEIGHT, SPX_FORS_TREES,
             SPX_WOTS_W);

    //dbg_printf(DEBUG_LEVEL_0, "Signature size: %d (%.2f KiB)\r\n", SPX_BYTES, SPX_BYTES / 1024.0);
    //dbg_printf(DEBUG_LEVEL_0, "Public key size: %d (%.2f KiB)\r\n", SPX_PK_BYTES, SPX_PK_BYTES / 1024.0);
    //dbg_printf(DEBUG_LEVEL_0, "Secret key size: %d (%.2f KiB)\r\n", SPX_SK_BYTES, SPX_SK_BYTES / 1024.0);

    randombytes(m, SPX_MLEN);
    randombytes(addr, SPX_ADDR_BYTES);

    dbg_printf(DEBUG_LEVEL_0, "SPHINCS+ algorithm is running...\r\n");

    resetTimeStamp();
    getTimeStamp();

    crypto_sign_keypair(pk, sk);
    getTimeStamp();

    wots_gen_pk(wots_pk, sk, pk, (uint32_t *) addr);
    getTimeStamp();

    crypto_sign(sm, &smlen, m, SPX_MLEN, sk);
    getTimeStamp();

    fors_sign(fors_sig, fors_pk, fors_m, sk, pk, (uint32_t *) addr);
    getTimeStamp();

    wots_sign(wots_sig, wots_m, sk, pk, (uint32_t *) addr);
    getTimeStamp();

    wots_gen_pk(wots_pk, sk, pk, (uint32_t *) addr);
    getTimeStamp();

    iStatus = crypto_sign_open(mout, &mlen, sm, smlen, pk);
    getTimeStamp();

    uiTotalTime_ms = getTimeDiff_ms(7, 0);

    uiTimeDiff_ms = getTimeDiff_ms(1, 0);
    dbg_printf(DEBUG_LEVEL_0, "Generating keypair... %-10d ms  Percentage: %", uiTimeDiff_ms);
    //printFloat(((float)uiTimeDiff_ms / uiTotalTime_ms) * 100.0);
    dbg_printf(DEBUG_LEVEL_0, "\r\n");

    uiTimeDiff_ms = getTimeDiff_ms(2, 1);
    dbg_printf(DEBUG_LEVEL_0, "  - WOTS pk gen...    %-10d ms  Percentage: %", uiTimeDiff_ms);
    //printFloat(((float)uiTimeDiff_ms / uiTotalTime_ms) * 100.0);
    dbg_printf(DEBUG_LEVEL_0, "\r\n");

    uiTimeDiff_ms = getTimeDiff_ms(3, 2);
    dbg_printf(DEBUG_LEVEL_0, "Signing...            %-10d ms  Percentage: %", uiTimeDiff_ms);
    //printFloat(((float)uiTimeDiff_ms / uiTotalTime_ms) * 100.0);
    dbg_printf(DEBUG_LEVEL_0, "\r\n");

    uiTimeDiff_ms = getTimeDiff_ms(4, 3);
    dbg_printf(DEBUG_LEVEL_0, "  - FORS signing...   %-10d ms  Percentage: %", uiTimeDiff_ms);
    //printFloat(((float)uiTimeDiff_ms / uiTotalTime_ms) * 100.0);
    dbg_printf(DEBUG_LEVEL_0, "\r\n");

    uiTimeDiff_ms = getTimeDiff_ms(5, 4);
    dbg_printf(DEBUG_LEVEL_0, "  - WOTS signing...   %-10d ms  Percentage: %", uiTimeDiff_ms);
    //printFloat(((float)uiTimeDiff_ms / uiTotalTime_ms) * 100.0);
    dbg_printf(DEBUG_LEVEL_0, "\r\n");

    uiTimeDiff_ms = getTimeDiff_ms(6, 5);
    dbg_printf(DEBUG_LEVEL_0, "  - WOTS pk gen...    %-10d ms  Percentage: %", uiTimeDiff_ms);
    //printFloat(((float)uiTimeDiff_ms / uiTotalTime_ms) * 100.0);
    dbg_printf(DEBUG_LEVEL_0, "\r\n");

    uiTimeDiff_ms = getTimeDiff_ms(7, 6);
    dbg_printf(DEBUG_LEVEL_0, "Verifying...          %-10d ms  Percentage: %", uiTimeDiff_ms);
    //printFloat(((float)uiTimeDiff_ms / uiTotalTime_ms) * 100.0);
    dbg_printf(DEBUG_LEVEL_0, "\r\n");

    dbg_printf(DEBUG_LEVEL_0, "Algorithm Total Time  %-10d ms  Percentage: %", uiTotalTime_ms);
    //printFloat(((float)uiTotalTime_ms / uiTotalTime_ms) * 100.0);
    dbg_printf(DEBUG_LEVEL_0, "\r\n");

    if (iStatus != 0)
    	dbg_printf(DEBUG_LEVEL_0, "ERROR: Verifying Operation Failed!\r\n");
    else
    	dbg_printf(DEBUG_LEVEL_0, "SUCCESS: Verifying Operation OK.\r\n");

    return 0;
}
