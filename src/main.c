/*
 ============================================================================
 Name        : main.c
 Author      : mpcukur
 Version     :
 Copyright   : Your copyright notice
 Description :
 ============================================================================
 */

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "dbg_printf.h"
#include "my_flags.h"
#include "my_utils.h"
#include "myTime.h"
#include "vicuna_test/vicuna_test.h"
#include "sphincs-sha256-256s-simple/test/test.h"
#include "tiny_aes/aes.h"
#include "sphincs-sha256-256s-simple/sha256.h"
#include "sphincs-sha256-256s-simple/address.h"
#include "sphincs-sha256-256s-simple/wots.h"
#include "sphincs-sha256-256s-simple/api.h"
#include "sphincs-sha256-256s-simple/utils.h"
#include "sphincs-sha256-256s-simple/rng.h"
#include "sphincs-sha256-256s-simple/fors.h"
#include "sphincs-sha256-256s-simple/thash.h"
#include "sphincs-sha256-256s-simple/hash.h"
#include "main.h"
#include "my_uart.h"

static void __attribute__((no_instrument_function)) initFunctionNames();

#define MAX_FUNCTIONS 	200

typedef struct
{
	void* vpFunction;
	const char* cpFunctionName;
} SFUnctionMapping;

typedef struct
{
    void* vpFuncAddress;
    uint64_t ullInstructionCountTotal;
    uint64_t ullInstructionCountCurrent;
} SFunctionProfile;

static SFUnctionMapping S_sArrFunctionMapping[MAX_FUNCTIONS];
static SFunctionProfile S_sArrFunctions[MAX_FUNCTIONS];
static uint32_t S_uiInstructionCountExit_LS32 = 0, S_uiInstructionCountExit_MS32 = 0; // Normalde statik olmasina ihtiyac yok, daha duzgun olcum yapabilmek icin fonksiyon icine alinmadi

void __attribute__((no_instrument_function)) __cyg_profile_func_enter(void *func, void *caller)
{
	int iIndex = -1;
	uint32_t uiInstructionCount_LS32 = 0, uiInstructionCount_MS32 = 0;

    for (int i = 0; i < MAX_FUNCTIONS; i++)
    {
    	if (i == (MAX_FUNCTIONS - 1))
    		dbg_printf(DEBUG_LEVEL_0, "WARNING: Maximum function count is reached!\r\n");

        if (S_sArrFunctions[i].vpFuncAddress == func)
        {
        	iIndex = i;
            break;
        }
        if (S_sArrFunctions[i].vpFuncAddress == NULL)
        {
        	S_sArrFunctions[i].vpFuncAddress = func;
        	iIndex = i;
            break;
        }
    }

    if (iIndex == -1)
    {
        dbg_printf(DEBUG_LEVEL_0, "ERROR: Function 0x%08x was not found and the array is full!\r\n", func);
    }

	asm volatile ("csrr %0,minstret"  : "=r"(uiInstructionCount_LS32));
	asm volatile ("csrr %0,minstreth" : "=r"(uiInstructionCount_MS32));

    S_sArrFunctions[iIndex].ullInstructionCountCurrent = (uint64_t)uiInstructionCount_LS32 | ((uint64_t)uiInstructionCount_MS32 << 32);
}

void __attribute__((no_instrument_function)) __cyg_profile_func_exit(void *func, void *caller)
{
	asm volatile ("csrr %0,minstret"  : "=r"(S_uiInstructionCountExit_LS32));
	asm volatile ("csrr %0,minstreth" : "=r"(S_uiInstructionCountExit_MS32));

	int iIndex = -1;
	for (int i = 0; i < MAX_FUNCTIONS; i++)
	{
		if (S_sArrFunctions[i].vpFuncAddress == func)
		{
			iIndex = i;
			break;
		}
	}

	if (iIndex == -1)
		dbg_printf(DEBUG_LEVEL_0, "ERROR: Function 0x%08x was not found!\r\n", func);

	S_sArrFunctions[iIndex].ullInstructionCountTotal += (((uint64_t)S_uiInstructionCountExit_LS32 | ((uint64_t)S_uiInstructionCountExit_MS32 << 32)) - S_sArrFunctions[iIndex].ullInstructionCountCurrent);
}

static void __attribute__((no_instrument_function)) initFunctions()
{
	for (int i = 0; i < MAX_FUNCTIONS; i++)
	{
		S_sArrFunctions[i].vpFuncAddress = NULL;
		S_sArrFunctions[i].ullInstructionCountTotal = 0;
		S_sArrFunctions[i].ullInstructionCountCurrent = 0;
	}
}

static void __attribute__((no_instrument_function)) getFunctionName(void* vpFunctionAddress, char* buffer, size_t bufferSize)
{
    strncpy(buffer, "NULL", bufferSize - 1);
    buffer[bufferSize - 1] = '\0'; // Ensure null termination

    for (int i = 0; i < MAX_FUNCTIONS; i++)
    {
        if (S_sArrFunctionMapping[i].vpFunction == vpFunctionAddress)
        {
            strncpy(buffer, S_sArrFunctionMapping[i].cpFunctionName, bufferSize - 1);
            buffer[bufferSize - 1] = '\0'; // Ensure null termination
            break;
        }
    }
}

static int __attribute__((no_instrument_function)) compareUsage(const void* a, const void* b)
{
	uint64_t usageA = (((SFunctionProfile*)a)->ullInstructionCountTotal);
	uint64_t usageB = (((SFunctionProfile*)b)->ullInstructionCountTotal);
    return (usageB > usageA) - (usageB < usageA); // descending order
}

void __attribute__((no_instrument_function)) printFunctionsIstatistics()
{
	int iIndex = -1;
	unsigned int uiInstrCountTotal_LS32 = 0, uiInstrCountTotal_MS32;
	uint32_t uiInstructionCount_LS32 = 0, uiInstructionCount_MS32 = 0;
	uint64_t ullInstrCntCurr = 0;
	char cArrFunctionName[30];
	//float fUsagePercentage = 0.0;

	asm volatile ("csrr %0,minstret"  : "=r"(uiInstructionCount_LS32));
	asm volatile ("csrr %0,minstreth" : "=r"(uiInstructionCount_MS32));

	ullInstrCntCurr = getU64FromTwoU32s(uiInstructionCount_MS32, uiInstructionCount_LS32);

	#ifdef SPIKE_ENABLED
	dbg_printf(DEBUG_LEVEL_0, "Current Instruction Numbers Executed so far: 0x%llx\r\n", ullInstrCntCurr);
	#else
	dbg_printf(DEBUG_LEVEL_0, "Current Instruction Numbers Executed so far: 0x%llx\r\n", uiInstructionCount_MS32, uiInstructionCount_LS32);
	#endif

    // Sort the functions by usage
    qsort(S_sArrFunctions, MAX_FUNCTIONS, sizeof(SFunctionProfile), compareUsage);

	for (iIndex = 0; iIndex < MAX_FUNCTIONS; iIndex++)
	{
		if (S_sArrFunctions[iIndex].vpFuncAddress == NULL)
			break;
		else
		{
			uiInstrCountTotal_LS32 = S_sArrFunctions[iIndex].ullInstructionCountTotal & 0xFFFFFFFF;
			uiInstrCountTotal_MS32 = (S_sArrFunctions[iIndex].ullInstructionCountTotal >> 32) & 0xFFFFFFFF;

			//fUsagePercentage = ((float)S_sArrFunctions[iIndex].ullInstructionCountTotal / ullInstrCntCurr) * 100.0;

			getFunctionName(S_sArrFunctions[iIndex].vpFuncAddress, cArrFunctionName, sizeof(cArrFunctionName));

			#ifdef SPIKE_ENABLED
			dbg_printf(DEBUG_LEVEL_0, "Func %-30s 0x%08x - InstrCount: 0x%016llx   Usage: %8.4f\r\n", cArrFunctionName, S_sArrFunctions[iIndex].vpFuncAddress, S_sArrFunctions[iIndex].ullInstructionCountTotal, fUsagePercentage);
			#else
			dbg_printf(DEBUG_LEVEL_0, "Func %-30s 0x%08x - InstrCount: 0x%llx   Usage: ", cArrFunctionName, S_sArrFunctions[iIndex].vpFuncAddress, uiInstrCountTotal_MS32, uiInstrCountTotal_LS32);
			//printFloat(fUsagePercentage);
			dbg_printf(DEBUG_LEVEL_0, "\r\n");
			#endif
		}
	}

	dbg_printf(DEBUG_LEVEL_0, "Code have entered %d functions so far.\r\n", iIndex);
}

#ifndef STATIC_FUNCTIONS_ENABLED

extern MY_STATIC void KeyExpansion(uint8_t* RoundKey, const uint8_t* Key);
extern MY_STATIC void SubBytes(state_t* state);
extern MY_STATIC void ShiftRows(state_t* state);
extern MY_STATIC uint8_t xtime(uint8_t x);
extern MY_STATIC void MixColumns(state_t* state);
extern MY_STATIC void AddRoundKey(uint8_t round, state_t* state, const uint8_t* RoundKey);
extern MY_STATIC void Cipher(state_t* state, const uint8_t* RoundKey);
extern MY_STATIC void InvShiftRows(state_t* state);
extern MY_STATIC void InvSubBytes(state_t* state);
extern MY_STATIC void InvMixColumns(state_t* state);
extern MY_STATIC void InvCipher(state_t* state, const uint8_t* RoundKey);
extern MY_STATIC void fors_sk_to_leaf(unsigned char *leaf, const unsigned char *sk, const unsigned char *pub_seed, uint32_t fors_leaf_addr[8]);
extern MY_STATIC void message_to_indices(uint32_t *indices, const unsigned char *m);
extern MY_STATIC void fors_gen_sk(unsigned char *sk, const unsigned char *sk_seed, uint32_t fors_leaf_addr[8]);
extern MY_STATIC void fors_gen_leaf(unsigned char *leaf, const unsigned char *sk_seed, const unsigned char *pub_seed, uint32_t addr_idx, const uint32_t fors_tree_addr[8]);
extern MY_STATIC uint64_t load_bigendian_64(const uint8_t *x);
extern MY_STATIC uint32_t load_bigendian_32(const uint8_t *x);
extern MY_STATIC void store_bigendian_32(uint8_t *x, uint64_t u);
extern MY_STATIC void store_bigendian_64(uint8_t *x, uint64_t u);
extern MY_STATIC size_t crypto_hashblocks_sha256(uint8_t *statebytes, const uint8_t *in, size_t inlen);
extern MY_STATIC void wots_gen_leaf(unsigned char *leaf, const unsigned char *sk_seed, const unsigned char *pub_seed, int32_t addr_idx, const uint32_t tree_addr[8]);
extern MY_STATIC void base_w(unsigned int *output, const int out_len, const unsigned char *input);
extern MY_STATIC void wots_gen_sk(unsigned char *sk, const unsigned char *sk_seed, uint32_t wots_addr[8]);
extern MY_STATIC void gen_chain(unsigned char *out, const unsigned char *in, unsigned int start, unsigned int steps, const unsigned char *pub_seed, uint32_t addr[8]);
extern MY_STATIC void wots_checksum(unsigned int *csum_base_w, const unsigned int *msg_base_w);
extern MY_STATIC void chain_lengths(unsigned int *lengths, const unsigned char *msg);

#endif

int main()
{
	dbg_printf(DEBUG_LEVEL_0, "Code started...\r\n");

	vicuna_test();
	vicuna_test_pasa();

	/*initFunctionNames();
	initFunctions();

	benchmark();

	printFunctionsIstatistics();*/

	while(1);

	return 0;
}

static void __attribute__((no_instrument_function)) initFunctionNames()
{
	for (int i = 0; i < MAX_FUNCTIONS; i++)
	{
		S_sArrFunctionMapping[i].vpFunction = NULL;
	}

	S_sArrFunctionMapping[en_KeyExpansion].cpFunctionName = "KeyExpansion";
	S_sArrFunctionMapping[en_KeyExpansion].vpFunction = KeyExpansion;

	S_sArrFunctionMapping[en_SubBytes].cpFunctionName = "SubBytes";
	S_sArrFunctionMapping[en_SubBytes].vpFunction = SubBytes;

	S_sArrFunctionMapping[en_ShiftRows].cpFunctionName = "ShiftRows";
	S_sArrFunctionMapping[en_ShiftRows].vpFunction = ShiftRows;

	S_sArrFunctionMapping[en_xtime].cpFunctionName = "xtime";
	S_sArrFunctionMapping[en_xtime].vpFunction = xtime;

	S_sArrFunctionMapping[en_MixColumns].cpFunctionName = "MixColumns";
	S_sArrFunctionMapping[en_MixColumns].vpFunction = MixColumns;

	S_sArrFunctionMapping[en_AddRoundKey].cpFunctionName = "AddRoundKey";
	S_sArrFunctionMapping[en_AddRoundKey].vpFunction = AddRoundKey;

	S_sArrFunctionMapping[en_Cipher].cpFunctionName = "Cipher";
	S_sArrFunctionMapping[en_Cipher].vpFunction = Cipher;

	S_sArrFunctionMapping[en_InvShiftRows].cpFunctionName = "InvShiftRows";
	S_sArrFunctionMapping[en_InvShiftRows].vpFunction = InvShiftRows;

	S_sArrFunctionMapping[en_InvSubBytes].cpFunctionName = "InvSubBytes";
	S_sArrFunctionMapping[en_InvSubBytes].vpFunction = InvSubBytes;

	S_sArrFunctionMapping[en_InvMixColumns].cpFunctionName = "InvMixColumns";
	S_sArrFunctionMapping[en_InvMixColumns].vpFunction = InvMixColumns;

	S_sArrFunctionMapping[en_InvCipher].cpFunctionName = "InvCipher";
	S_sArrFunctionMapping[en_InvCipher].vpFunction = InvCipher;

	S_sArrFunctionMapping[en_fors_sk_to_leaf].cpFunctionName = "fors_sk_to_leaf";
	S_sArrFunctionMapping[en_fors_sk_to_leaf].vpFunction = fors_sk_to_leaf;

	S_sArrFunctionMapping[en_message_to_indices].cpFunctionName = "message_to_indices";
	S_sArrFunctionMapping[en_message_to_indices].vpFunction = message_to_indices;

	S_sArrFunctionMapping[en_fors_gen_sk].cpFunctionName = "fors_gen_sk";
	S_sArrFunctionMapping[en_fors_gen_sk].vpFunction = fors_gen_sk;

	S_sArrFunctionMapping[en_fors_gen_leaf].cpFunctionName = "fors_gen_leaf";
	S_sArrFunctionMapping[en_fors_gen_leaf].vpFunction = fors_gen_leaf;

	S_sArrFunctionMapping[en_load_bigendian_64].cpFunctionName = "load_bigendian_64";
	S_sArrFunctionMapping[en_load_bigendian_64].vpFunction = load_bigendian_64;

	S_sArrFunctionMapping[en_load_bigendian_32].cpFunctionName = "load_bigendian_32";
	S_sArrFunctionMapping[en_load_bigendian_32].vpFunction = load_bigendian_32;

	S_sArrFunctionMapping[en_store_bigendian_32].cpFunctionName = "store_bigendian_32";
	S_sArrFunctionMapping[en_store_bigendian_32].vpFunction = store_bigendian_32;

	S_sArrFunctionMapping[en_crypto_hashblocks_sha256].cpFunctionName = "crypto_hashblocks_sha256";
	S_sArrFunctionMapping[en_crypto_hashblocks_sha256].vpFunction = crypto_hashblocks_sha256;

	S_sArrFunctionMapping[en_store_bigendian_64].cpFunctionName = "store_bigendian_64";
	S_sArrFunctionMapping[en_store_bigendian_64].vpFunction = store_bigendian_64;

	S_sArrFunctionMapping[en_wots_gen_leaf].cpFunctionName = "wots_gen_leaf";
	S_sArrFunctionMapping[en_wots_gen_leaf].vpFunction = wots_gen_leaf;

	S_sArrFunctionMapping[en_base_w].cpFunctionName = "base_w";
	S_sArrFunctionMapping[en_base_w].vpFunction = base_w;

	S_sArrFunctionMapping[en_wots_gen_sk].cpFunctionName = "wots_gen_sk";
	S_sArrFunctionMapping[en_wots_gen_sk].vpFunction = wots_gen_sk;

	S_sArrFunctionMapping[en_gen_chain].cpFunctionName = "gen_chain";
	S_sArrFunctionMapping[en_gen_chain].vpFunction = gen_chain;

	S_sArrFunctionMapping[en_wots_checksum].cpFunctionName = "wots_checksum";
	S_sArrFunctionMapping[en_wots_checksum].vpFunction = wots_checksum;

	S_sArrFunctionMapping[en_chain_lengths].cpFunctionName = "chain_lengths";
	S_sArrFunctionMapping[en_chain_lengths].vpFunction = chain_lengths;

	S_sArrFunctionMapping[en_seed_state].cpFunctionName = "seed_state";
	S_sArrFunctionMapping[en_seed_state].vpFunction = seed_state;

	S_sArrFunctionMapping[en_set_layer_addr].cpFunctionName = "set_layer_addr";
	S_sArrFunctionMapping[en_set_layer_addr].vpFunction = set_layer_addr;

	S_sArrFunctionMapping[en_set_keypair_addr].cpFunctionName = "set_keypair_addr";
	S_sArrFunctionMapping[en_set_keypair_addr].vpFunction = set_keypair_addr;

	S_sArrFunctionMapping[en_wots_sign].cpFunctionName = "wots_sign";
	S_sArrFunctionMapping[en_wots_sign].vpFunction = wots_sign;

	S_sArrFunctionMapping[en_sha256_inc_finalize].cpFunctionName = "sha256_inc_finalize";
	S_sArrFunctionMapping[en_sha256_inc_finalize].vpFunction = sha256_inc_finalize;

	S_sArrFunctionMapping[en_setDebugLevel].cpFunctionName = "setDebugLevel";
	S_sArrFunctionMapping[en_setDebugLevel].vpFunction = setDebugLevel;

	S_sArrFunctionMapping[en_getTime_ms].cpFunctionName = "getTime_ms";
	S_sArrFunctionMapping[en_getTime_ms].vpFunction = getTime_ms;

	S_sArrFunctionMapping[en_AES_ECB_decrypt].cpFunctionName = "AES_ECB_decrypt";
	S_sArrFunctionMapping[en_AES_ECB_decrypt].vpFunction = AES_ECB_decrypt;

	S_sArrFunctionMapping[en_memmove].cpFunctionName = "memmove";
	S_sArrFunctionMapping[en_memmove].vpFunction = memmove;

	S_sArrFunctionMapping[en_crypto_sign_seed_keypair].cpFunctionName = "crypto_sign_seed_keypair";
	S_sArrFunctionMapping[en_crypto_sign_seed_keypair].vpFunction = crypto_sign_seed_keypair;

	S_sArrFunctionMapping[en_set_tree_index].cpFunctionName = "set_tree_index";
	S_sArrFunctionMapping[en_set_tree_index].vpFunction = set_tree_index;

	S_sArrFunctionMapping[en_sha256_inc_blocks].cpFunctionName = "sha256_inc_blocks";
	S_sArrFunctionMapping[en_sha256_inc_blocks].vpFunction = sha256_inc_blocks;

	S_sArrFunctionMapping[en_crypto_sign_secretkeybytes].cpFunctionName = "crypto_sign_secretkeybytes";
	S_sArrFunctionMapping[en_crypto_sign_secretkeybytes].vpFunction = crypto_sign_secretkeybytes;

	S_sArrFunctionMapping[en_u32_to_bytes].cpFunctionName = "u32_to_bytes";
	S_sArrFunctionMapping[en_u32_to_bytes].vpFunction = u32_to_bytes;

	S_sArrFunctionMapping[en_memcpy].cpFunctionName = "memcpy";
	S_sArrFunctionMapping[en_memcpy].vpFunction = memcpy;

	S_sArrFunctionMapping[en_AES256_CTR_DRBG_Update].cpFunctionName = "AES256_CTR_DRBG_Update";
	S_sArrFunctionMapping[en_AES256_CTR_DRBG_Update].vpFunction = AES256_CTR_DRBG_Update;

	S_sArrFunctionMapping[en_thash].cpFunctionName = "thash";
	S_sArrFunctionMapping[en_thash].vpFunction = thash;

	S_sArrFunctionMapping[en_sendUartByteWaitTransmitDone].cpFunctionName = "sendUartByteWaitTransmitDone";
	S_sArrFunctionMapping[en_sendUartByteWaitTransmitDone].vpFunction = sendUartByteWaitTransmitDone;

	S_sArrFunctionMapping[en_ull_to_bytes].cpFunctionName = "ull_to_bytes";
	S_sArrFunctionMapping[en_ull_to_bytes].vpFunction = ull_to_bytes;

	S_sArrFunctionMapping[en_seedexpander].cpFunctionName = "seedexpander";
	S_sArrFunctionMapping[en_seedexpander].vpFunction = seedexpander;

	S_sArrFunctionMapping[en_wots_gen_pk].cpFunctionName = "wots_gen_pk";
	S_sArrFunctionMapping[en_wots_gen_pk].vpFunction = wots_gen_pk;

	S_sArrFunctionMapping[en_crypto_sign_bytes].cpFunctionName = "crypto_sign_bytes";
	S_sArrFunctionMapping[en_crypto_sign_bytes].vpFunction = crypto_sign_bytes;

	S_sArrFunctionMapping[en_read32BitDataFromAddress].cpFunctionName = "read32BitDataFromAddress";
	S_sArrFunctionMapping[en_read32BitDataFromAddress].vpFunction = read32BitDataFromAddress;

	S_sArrFunctionMapping[en_mgf1].cpFunctionName = "mgf1";
	S_sArrFunctionMapping[en_mgf1].vpFunction = mgf1;

	S_sArrFunctionMapping[en_crypto_sign_publickeybytes].cpFunctionName = "crypto_sign_publickeybytes";
	S_sArrFunctionMapping[en_crypto_sign_publickeybytes].vpFunction = crypto_sign_publickeybytes;

	S_sArrFunctionMapping[en_copy_subtree_addr].cpFunctionName = "copy_subtree_addr";
	S_sArrFunctionMapping[en_copy_subtree_addr].vpFunction = copy_subtree_addr;

	S_sArrFunctionMapping[en_AES_ECB_encrypt].cpFunctionName = "AES_ECB_encrypt";
	S_sArrFunctionMapping[en_AES_ECB_encrypt].vpFunction = AES_ECB_encrypt;

	S_sArrFunctionMapping[en_initialize_hash_function].cpFunctionName = "initialize_hash_function";
	S_sArrFunctionMapping[en_initialize_hash_function].vpFunction = initialize_hash_function;

	S_sArrFunctionMapping[en_resetTimeStamp].cpFunctionName = "resetTimeStamp";
	S_sArrFunctionMapping[en_resetTimeStamp].vpFunction = resetTimeStamp;

	S_sArrFunctionMapping[en_randombytes].cpFunctionName = "randombytes";
	S_sArrFunctionMapping[en_randombytes].vpFunction = randombytes;

	S_sArrFunctionMapping[en_wots_pk_from_sig].cpFunctionName = "wots_pk_from_sig";
	S_sArrFunctionMapping[en_wots_pk_from_sig].vpFunction = wots_pk_from_sig;

	S_sArrFunctionMapping[en_set_hash_addr].cpFunctionName = "set_hash_addr";
	S_sArrFunctionMapping[en_set_hash_addr].vpFunction = set_hash_addr;

	S_sArrFunctionMapping[en_sha256_inc_init].cpFunctionName = "sha256_inc_init";
	S_sArrFunctionMapping[en_sha256_inc_init].vpFunction = sha256_inc_init;

	S_sArrFunctionMapping[en_crypto_sign_keypair].cpFunctionName = "crypto_sign_keypair";
	S_sArrFunctionMapping[en_crypto_sign_keypair].vpFunction = crypto_sign_keypair;

	S_sArrFunctionMapping[en_randombytes_init].cpFunctionName = "randombytes_init";
	S_sArrFunctionMapping[en_randombytes_init].vpFunction = randombytes_init;

	S_sArrFunctionMapping[en_crypto_sign_signature].cpFunctionName = "crypto_sign_signature";
	S_sArrFunctionMapping[en_crypto_sign_signature].vpFunction = crypto_sign_signature;

	S_sArrFunctionMapping[en_seedexpander_init].cpFunctionName = "seedexpander_init";
	S_sArrFunctionMapping[en_seedexpander_init].vpFunction = seedexpander_init;

	S_sArrFunctionMapping[en_write32BitDataToAddress].cpFunctionName = "write32BitDataToAddress";
	S_sArrFunctionMapping[en_write32BitDataToAddress].vpFunction = write32BitDataToAddress;

	S_sArrFunctionMapping[en_compute_root].cpFunctionName = "compute_root";
	S_sArrFunctionMapping[en_compute_root].vpFunction = compute_root;

	S_sArrFunctionMapping[en_set_chain_addr].cpFunctionName = "set_chain_addr";
	S_sArrFunctionMapping[en_set_chain_addr].vpFunction = set_chain_addr;

	S_sArrFunctionMapping[en_dbg_printf].cpFunctionName = "dbg_printf";
	S_sArrFunctionMapping[en_dbg_printf].vpFunction = dbg_printf;

	S_sArrFunctionMapping[en_getU64FromTwoU32s].cpFunctionName = "getU64FromTwoU32s";
	S_sArrFunctionMapping[en_getU64FromTwoU32s].vpFunction = getU64FromTwoU32s;

	S_sArrFunctionMapping[en_crypto_sign].cpFunctionName = "crypto_sign";
	S_sArrFunctionMapping[en_crypto_sign].vpFunction = crypto_sign;

	S_sArrFunctionMapping[en_fors_sign].cpFunctionName = "fors_sign";
	S_sArrFunctionMapping[en_fors_sign].vpFunction = fors_sign;

	S_sArrFunctionMapping[en_gen_message_random].cpFunctionName = "gen_message_random";
	S_sArrFunctionMapping[en_gen_message_random].vpFunction = gen_message_random;

	S_sArrFunctionMapping[en_crypto_sign_verify].cpFunctionName = "crypto_sign_verify";
	S_sArrFunctionMapping[en_crypto_sign_verify].vpFunction = crypto_sign_verify;

	S_sArrFunctionMapping[en_memcmp].cpFunctionName = "memcmp";
	S_sArrFunctionMapping[en_memcmp].vpFunction = memcmp;

	S_sArrFunctionMapping[en_getTimeDiff_ms].cpFunctionName = "getTimeDiff_ms";
	S_sArrFunctionMapping[en_getTimeDiff_ms].vpFunction = getTimeDiff_ms;

	S_sArrFunctionMapping[en_benchmark].cpFunctionName = "benchmark";
	S_sArrFunctionMapping[en_benchmark].vpFunction = benchmark;

	S_sArrFunctionMapping[en_set_tree_height].cpFunctionName = "set_tree_height";
	S_sArrFunctionMapping[en_set_tree_height].vpFunction = set_tree_height;

	S_sArrFunctionMapping[en_memset].cpFunctionName = "memset";
	S_sArrFunctionMapping[en_memset].vpFunction = memset;

	S_sArrFunctionMapping[en_main].cpFunctionName = "main";
	S_sArrFunctionMapping[en_main].vpFunction = main;

	S_sArrFunctionMapping[en_AES256_ECB].cpFunctionName = "AES256_ECB";
	S_sArrFunctionMapping[en_AES256_ECB].vpFunction = AES256_ECB;

	S_sArrFunctionMapping[en_prf_addr].cpFunctionName = "prf_addr";
	S_sArrFunctionMapping[en_prf_addr].vpFunction = prf_addr;

	S_sArrFunctionMapping[en_set_type].cpFunctionName = "set_type";
	S_sArrFunctionMapping[en_set_type].vpFunction = set_type;

	S_sArrFunctionMapping[en_crypto_sign_seedbytes].cpFunctionName = "crypto_sign_seedbytes";
	S_sArrFunctionMapping[en_crypto_sign_seedbytes].vpFunction = crypto_sign_seedbytes;

	S_sArrFunctionMapping[en_hash_message].cpFunctionName = "hash_message";
	S_sArrFunctionMapping[en_hash_message].vpFunction = hash_message;

	S_sArrFunctionMapping[en_copy_keypair_addr].cpFunctionName = "copy_keypair_addr";
	S_sArrFunctionMapping[en_copy_keypair_addr].vpFunction = copy_keypair_addr;

	S_sArrFunctionMapping[en_sha256].cpFunctionName = "sha256";
	S_sArrFunctionMapping[en_sha256].vpFunction = sha256;

	S_sArrFunctionMapping[en_getCpuCycleLow].cpFunctionName = "getCpuCycleLow";
	S_sArrFunctionMapping[en_getCpuCycleLow].vpFunction = getCpuCycleLow;

	S_sArrFunctionMapping[en_crypto_sign_open].cpFunctionName = "crypto_sign_open";
	S_sArrFunctionMapping[en_crypto_sign_open].vpFunction = crypto_sign_open;

	S_sArrFunctionMapping[en_AES_init_ctx].cpFunctionName = "AES_init_ctx";
	S_sArrFunctionMapping[en_AES_init_ctx].vpFunction = AES_init_ctx;

	S_sArrFunctionMapping[en_set_tree_addr].cpFunctionName = "set_tree_addr";
	S_sArrFunctionMapping[en_set_tree_addr].vpFunction = set_tree_addr;

	S_sArrFunctionMapping[en_bytes_to_ull].cpFunctionName = "bytes_to_ull";
	S_sArrFunctionMapping[en_bytes_to_ull].vpFunction = bytes_to_ull;

	S_sArrFunctionMapping[en_strlen].cpFunctionName = "strlen";
	S_sArrFunctionMapping[en_strlen].vpFunction = strlen;

	S_sArrFunctionMapping[en_intToString].cpFunctionName = "intToString";
	S_sArrFunctionMapping[en_intToString].vpFunction = intToString;

	S_sArrFunctionMapping[en_write8BitDataToAddress].cpFunctionName = "write8BitDataToAddress";
	S_sArrFunctionMapping[en_write8BitDataToAddress].vpFunction = write8BitDataToAddress;

	S_sArrFunctionMapping[en_treehash].cpFunctionName = "treehash";
	S_sArrFunctionMapping[en_treehash].vpFunction = treehash;

	S_sArrFunctionMapping[en_fors_pk_from_sig].cpFunctionName = "fors_pk_from_sig";
	S_sArrFunctionMapping[en_fors_pk_from_sig].vpFunction = fors_pk_from_sig;

	S_sArrFunctionMapping[en_getTimeStamp].cpFunctionName = "getTimeStamp";
	S_sArrFunctionMapping[en_getTimeStamp].vpFunction = getTimeStamp;

	S_sArrFunctionMapping[en_getCpuCycleHigh].cpFunctionName = "getCpuCycleHigh";
	S_sArrFunctionMapping[en_getCpuCycleHigh].vpFunction = getCpuCycleHigh;

	S_sArrFunctionMapping[en_intToHexString].cpFunctionName = "intToHexString";
	S_sArrFunctionMapping[en_intToHexString].vpFunction = intToHexString;
}
