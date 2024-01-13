/* Based on the public domain implementation in
 * crypto_hash/sha512/ref/ from http://bench.cr.yp.to/supercop.html
 * by D. J. Bernstein */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "utils.h"
#include "sha256.h"
#include "../my_flags.h"
#include <riscv_vector.h>

MY_STATIC uint32_t load_bigendian_32(const uint8_t *x) {
    return (uint32_t)(x[3]) | (((uint32_t)(x[2])) << 8) |
           (((uint32_t)(x[1])) << 16) | (((uint32_t)(x[0])) << 24);
}

MY_STATIC uint64_t load_bigendian_64(const uint8_t *x) {
    return (uint64_t)(x[7]) | (((uint64_t)(x[6])) << 8) |
           (((uint64_t)(x[5])) << 16) | (((uint64_t)(x[4])) << 24) |
           (((uint64_t)(x[3])) << 32) | (((uint64_t)(x[2])) << 40) |
           (((uint64_t)(x[1])) << 48) | (((uint64_t)(x[0])) << 56);
}

MY_STATIC void store_bigendian_32(uint8_t *x, uint64_t u) {
    x[3] = (uint8_t) u;
    u >>= 8;
    x[2] = (uint8_t) u;
    u >>= 8;
    x[1] = (uint8_t) u;
    u >>= 8;
    x[0] = (uint8_t) u;
}

MY_STATIC void store_bigendian_64(uint8_t *x, uint64_t u) {
    x[7] = (uint8_t) u;
    u >>= 8;
    x[6] = (uint8_t) u;
    u >>= 8;
    x[5] = (uint8_t) u;
    u >>= 8;
    x[4] = (uint8_t) u;
    u >>= 8;
    x[3] = (uint8_t) u;
    u >>= 8;
    x[2] = (uint8_t) u;
    u >>= 8;
    x[1] = (uint8_t) u;
    u >>= 8;
    x[0] = (uint8_t) u;
}

#define SHR(x, c) ((x) >> (c))
#define ROTR_32(x, c) (((x) >> (c)) | ((x) << (32 - (c))))
#define ROTR_64(x, c) (((x) >> (c)) | ((x) << (64 - (c))))

#define Ch(x, y, z) (((x) & (y)) ^ (~(x) & (z)))
#define Maj(x, y, z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))

#define Sigma0_32(x) (ROTR_32(x, 2) ^ ROTR_32(x,13) ^ ROTR_32(x,22))
#define Sigma1_32(x) (ROTR_32(x, 6) ^ ROTR_32(x,11) ^ ROTR_32(x,25))
#define sigma0_32(x) (ROTR_32(x, 7) ^ ROTR_32(x,18) ^ SHR(x, 3))
#define sigma1_32(x) (ROTR_32(x,17) ^ ROTR_32(x,19) ^ SHR(x,10))

/******************************************/
// Added for custom instruction
#define VECTOR_ELEMENT_WIDTH_BYTES		4

uint32_t w[16];
uint32_t* wp = w;
uint32_t letters[8];	// a,b,c,d,e,f,g,h

typedef enum
{
	a = 0,
	b = 1,
	c = 2,
	d = 3,
	e = 4,
	f = 5,
	g = 6,
	h = 7
} ELetters;
/******************************************/

//	w[i] = sigma1_32(w[i-2]) + w[i-7] + sigma0_32(w[i-15]) + w[i-16]
#define M_32(w0, w14, w9, w1) w0 = sigma1_32(w14) + (w9) + sigma0_32(w1) + (w0);

#define EXPAND_32           		\
    M_32(w[0], w[14], w[9], w[1])   \
    M_32(w[1], w[15], w[10], w[2])  \
    M_32(w[2], w[0], w[11], w[3])   \
    M_32(w[3], w[1], w[12], w[4])   \
    M_32(w[4], w[2], w[13], w[5])   \
    M_32(w[5], w[3], w[14], w[6])   \
    M_32(w[6], w[4], w[15], w[7])   \
    M_32(w[7], w[5], w[0], w[8])    \
    M_32(w[8], w[6], w[1], w[9])    \
    M_32(w[9], w[7], w[2], w[10])   \
    M_32(w[10], w[8], w[3], w[11])  \
    M_32(w[11], w[9], w[4], w[12])  \
    M_32(w[12], w[10], w[5], w[13]) \
    M_32(w[13], w[11], w[6], w[14]) \
    M_32(w[14], w[12], w[7], w[15]) \
    M_32(w[15], w[13], w[8], w[0])

// Updated for custom instruction a,b,c,d,e,f,g,h --> letters[a,b,c,d,e,f,g,h]
#define F_32(w, k)                                   												\
    T1 = letters[h] + Sigma1_32(letters[e]) + Ch(letters[e], letters[f], letters[g]) + (k) + (w); 	\
    T2 = Sigma0_32(letters[a]) + Maj(letters[a], letters[b], letters[c]);                			\
    letters[h] = letters[g];                                           								\
    letters[g] = letters[f];                                           								\
    letters[f] = letters[e];                                           								\
    letters[e] = letters[d] + T1;                                      								\
    letters[d] = letters[c];                                           								\
    letters[c] = letters[b];                                           								\
    letters[b] = letters[a];                                           								\
    letters[a] = T1 + T2;

MY_STATIC size_t crypto_hashblocks_sha256(uint8_t *statebytes,
                                       const uint8_t *in, size_t inlen)
{
	uint32_t AVL;	// Application vector length, desired number of elements we want to process.
	uint32_t VL;	// VL is the smaller of desired vector length, AVL and vector length limit set by hardware, VLMAX.
	vint32m1_t vectorReg; // Vector register that will be used during the vector operations

    uint32_t state[8];
    uint32_t T1;
    uint32_t T2;

    /**********************************************************/
    /*a = load_bigendian_32(statebytes + 0);
    state[0] = a;
    b = load_bigendian_32(statebytes + 4);
    state[1] = b;
    c = load_bigendian_32(statebytes + 8);
    state[2] = c;
    d = load_bigendian_32(statebytes + 12);
    state[3] = d;
    e = load_bigendian_32(statebytes + 16);
    state[4] = e;
    f = load_bigendian_32(statebytes + 20);
    state[5] = f;
    g = load_bigendian_32(statebytes + 24);
    state[6] = g;
    h = load_bigendian_32(statebytes + 28);
    state[7] = h;*/

    /*letters[a] = load_bigendian_32(statebytes + 0);
    state[0] = letters[a];
    letters[b] = load_bigendian_32(statebytes + 4);
    state[1] = letters[b];
    letters[c] = load_bigendian_32(statebytes + 8);
    state[2] = letters[c];
    letters[d] = load_bigendian_32(statebytes + 12);
    state[3] = letters[d];
    letters[e] = load_bigendian_32(statebytes + 16);
    state[4] = letters[e];
    letters[f] = load_bigendian_32(statebytes + 20);
    state[5] = letters[f];
    letters[g] = load_bigendian_32(statebytes + 24);
    state[6] = letters[g];
    letters[h] = load_bigendian_32(statebytes + 28);
    state[7] = letters[h];*/

    AVL = sizeof(state) / VECTOR_ELEMENT_WIDTH_BYTES;
	VL  = __riscv_vsetvl_e32m1(AVL);
	vectorReg = __builtin_riscv_vlebe32_v((uint32_t*)statebytes, VL);
	__riscv_vse32_v_i32m1((int*)letters, vectorReg, VL);
	__riscv_vse32_v_i32m1((int*)state, vectorReg, VL);

    /*state[0] = letters[a];
    state[1] = letters[b];
    state[2] = letters[c];
    state[3] = letters[d];
    state[4] = letters[e];
    state[5] = letters[f];
    state[6] = letters[g];
    state[7] = letters[h];*/
	/**********************************************************/

    while (inlen >= 64) {
        /**********************************************/
    	/*uint32_t w0  = load_bigendian_32(in + 0);
        uint32_t w1  = load_bigendian_32(in + 4);
        uint32_t w2  = load_bigendian_32(in + 8);
        uint32_t w3  = load_bigendian_32(in + 12);
        uint32_t w4  = load_bigendian_32(in + 16);
        uint32_t w5  = load_bigendian_32(in + 20);
        uint32_t w6  = load_bigendian_32(in + 24);
        uint32_t w7  = load_bigendian_32(in + 28);
        uint32_t w8  = load_bigendian_32(in + 32);
        uint32_t w9  = load_bigendian_32(in + 36);
        uint32_t w10 = load_bigendian_32(in + 40);
        uint32_t w11 = load_bigendian_32(in + 44);
        uint32_t w12 = load_bigendian_32(in + 48);
        uint32_t w13 = load_bigendian_32(in + 52);
        uint32_t w14 = load_bigendian_32(in + 56);
        uint32_t w15 = load_bigendian_32(in + 60);*/

    	AVL = sizeof(w) / VECTOR_ELEMENT_WIDTH_BYTES;
		VL  = __riscv_vsetvl_e32m1(AVL);
		vectorReg = __builtin_riscv_vlebe32_v((uint32_t*)in, VL);
		__riscv_vse32_v_i32m1((int*)w, vectorReg, VL);
    	/*********************************************/

        F_32(w[0], 0x428a2f98)
        F_32(w[1], 0x71374491)
        F_32(w[2], 0xb5c0fbcf)
        F_32(w[3], 0xe9b5dba5)
        F_32(w[4], 0x3956c25b)
        F_32(w[5], 0x59f111f1)
        F_32(w[6], 0x923f82a4)
        F_32(w[7], 0xab1c5ed5)
        F_32(w[8], 0xd807aa98)
        F_32(w[9], 0x12835b01)
        F_32(w[10], 0x243185be)
        F_32(w[11], 0x550c7dc3)
        F_32(w[12], 0x72be5d74)
        F_32(w[13], 0x80deb1fe)
        F_32(w[14], 0x9bdc06a7)
        F_32(w[15], 0xc19bf174)

        EXPAND_32

        F_32(w[0], 0xe49b69c1)
        F_32(w[1], 0xefbe4786)
        F_32(w[2], 0x0fc19dc6)
        F_32(w[3], 0x240ca1cc)
        F_32(w[4], 0x2de92c6f)
        F_32(w[5], 0x4a7484aa)
        F_32(w[6], 0x5cb0a9dc)
        F_32(w[7], 0x76f988da)
        F_32(w[8], 0x983e5152)
        F_32(w[9], 0xa831c66d)
        F_32(w[10], 0xb00327c8)
        F_32(w[11], 0xbf597fc7)
        F_32(w[12], 0xc6e00bf3)
        F_32(w[13], 0xd5a79147)
        F_32(w[14], 0x06ca6351)
        F_32(w[15], 0x14292967)

        EXPAND_32

        F_32(w[0], 0x27b70a85)
        F_32(w[1], 0x2e1b2138)
        F_32(w[2], 0x4d2c6dfc)
        F_32(w[3], 0x53380d13)
        F_32(w[4], 0x650a7354)
        F_32(w[5], 0x766a0abb)
        F_32(w[6], 0x81c2c92e)
        F_32(w[7], 0x92722c85)
        F_32(w[8], 0xa2bfe8a1)
        F_32(w[9], 0xa81a664b)
        F_32(w[10], 0xc24b8b70)
        F_32(w[11], 0xc76c51a3)
        F_32(w[12], 0xd192e819)
        F_32(w[13], 0xd6990624)
        F_32(w[14], 0xf40e3585)
        F_32(w[15], 0x106aa070)

        EXPAND_32

        F_32(w[0], 0x19a4c116)
        F_32(w[1], 0x1e376c08)
        F_32(w[2], 0x2748774c)
        F_32(w[3], 0x34b0bcb5)
        F_32(w[4], 0x391c0cb3)
        F_32(w[5], 0x4ed8aa4a)
        F_32(w[6], 0x5b9cca4f)
        F_32(w[7], 0x682e6ff3)
        F_32(w[8], 0x748f82ee)
        F_32(w[9], 0x78a5636f)
        F_32(w[10], 0x84c87814)
        F_32(w[11], 0x8cc70208)
        F_32(w[12], 0x90befffa)
        F_32(w[13], 0xa4506ceb)
        F_32(w[14], 0xbef9a3f7)
        F_32(w[15], 0xc67178f2)

        letters[a] += state[0];
        letters[b] += state[1];
        letters[c] += state[2];
        letters[d] += state[3];
        letters[e] += state[4];
        letters[f] += state[5];
        letters[g] += state[6];
        letters[h] += state[7];

        state[0] = letters[a];
        state[1] = letters[b];
        state[2] = letters[c];
        state[3] = letters[d];
        state[4] = letters[e];
        state[5] = letters[f];
        state[6] = letters[g];
        state[7] = letters[h];

        in += 64;
        inlen -= 64;
    }

    store_bigendian_32(statebytes + 0, state[0]);
    store_bigendian_32(statebytes + 4, state[1]);
    store_bigendian_32(statebytes + 8, state[2]);
    store_bigendian_32(statebytes + 12, state[3]);
    store_bigendian_32(statebytes + 16, state[4]);
    store_bigendian_32(statebytes + 20, state[5]);
    store_bigendian_32(statebytes + 24, state[6]);
    store_bigendian_32(statebytes + 28, state[7]);

    return inlen;
}

static const uint8_t iv_256[32] = {
    0x6a, 0x09, 0xe6, 0x67, 0xbb, 0x67, 0xae, 0x85,
    0x3c, 0x6e, 0xf3, 0x72, 0xa5, 0x4f, 0xf5, 0x3a,
    0x51, 0x0e, 0x52, 0x7f, 0x9b, 0x05, 0x68, 0x8c,
    0x1f, 0x83, 0xd9, 0xab, 0x5b, 0xe0, 0xcd, 0x19
};

void sha256_inc_init(uint8_t *state) {
    for (size_t i = 0; i < 32; ++i) {
        state[i] = iv_256[i];
    }
    for (size_t i = 32; i < 40; ++i) {
        state[i] = 0;
    }
}

void sha256_inc_blocks(uint8_t *state, const uint8_t *in, size_t inblocks) {
    uint64_t bytes = load_bigendian_64(state + 32);

    crypto_hashblocks_sha256(state, in, 64 * inblocks);
    bytes += 64 * inblocks;

    store_bigendian_64(state + 32, bytes);
}

void sha256_inc_finalize(uint8_t *out, uint8_t *state, const uint8_t *in, size_t inlen) {
    uint8_t padded[128];
    uint64_t bytes = load_bigendian_64(state + 32) + inlen;

    crypto_hashblocks_sha256(state, in, inlen);
    in += inlen;
    inlen &= 63;
    in -= inlen;

    for (size_t i = 0; i < inlen; ++i) {
        padded[i] = in[i];
    }
    padded[inlen] = 0x80;

    if (inlen < 56) {
        for (size_t i = inlen + 1; i < 56; ++i) {
            padded[i] = 0;
        }
        padded[56] = (uint8_t) (bytes >> 53);
        padded[57] = (uint8_t) (bytes >> 45);
        padded[58] = (uint8_t) (bytes >> 37);
        padded[59] = (uint8_t) (bytes >> 29);
        padded[60] = (uint8_t) (bytes >> 21);
        padded[61] = (uint8_t) (bytes >> 13);
        padded[62] = (uint8_t) (bytes >> 5);
        padded[63] = (uint8_t) (bytes << 3);
        crypto_hashblocks_sha256(state, padded, 64);
    } else {
        for (size_t i = inlen + 1; i < 120; ++i) {
            padded[i] = 0;
        }
        padded[120] = (uint8_t) (bytes >> 53);
        padded[121] = (uint8_t) (bytes >> 45);
        padded[122] = (uint8_t) (bytes >> 37);
        padded[123] = (uint8_t) (bytes >> 29);
        padded[124] = (uint8_t) (bytes >> 21);
        padded[125] = (uint8_t) (bytes >> 13);
        padded[126] = (uint8_t) (bytes >> 5);
        padded[127] = (uint8_t) (bytes << 3);
        crypto_hashblocks_sha256(state, padded, 128);
    }

    for (size_t i = 0; i < 32; ++i) {
        out[i] = state[i];
    }
}

void sha256(uint8_t *out, const uint8_t *in, size_t inlen) {
    uint8_t state[40];

    sha256_inc_init(state);
    sha256_inc_finalize(out, state, in, inlen);
}

/**
 * Note that inlen should be sufficiently small that it still allows for
 * an array to be allocated on the stack. Typically 'in' is merely a seed.
 * Outputs outlen number of bytes
 */
void mgf1(unsigned char *out, unsigned long outlen,
          const unsigned char *in, unsigned long inlen)
{
    unsigned char inbuf[inlen + 4];
    unsigned char outbuf[SPX_SHA256_OUTPUT_BYTES];
    unsigned long i;

    memcpy(inbuf, in, inlen);

    /* While we can fit in at least another full block of SHA256 output.. */
    for (i = 0; (i+1)*SPX_SHA256_OUTPUT_BYTES <= outlen; i++) {
        u32_to_bytes(inbuf + inlen, i);
        sha256(out, inbuf, inlen + 4);
        out += SPX_SHA256_OUTPUT_BYTES;
    }
    /* Until we cannot anymore, and we fill the remainder. */
    if (outlen > i*SPX_SHA256_OUTPUT_BYTES) {
        u32_to_bytes(inbuf + inlen, i);
        sha256(outbuf, inbuf, inlen + 4);
        memcpy(out, outbuf, outlen - i*SPX_SHA256_OUTPUT_BYTES);
    }
}

uint8_t state_seeded[40];

/**
 * Absorb the constant pub_seed using one round of the compression function
 * This initializes state_seeded, which can then be reused in thash
 **/
void seed_state(const unsigned char *pub_seed) {
    uint8_t block[SPX_SHA256_BLOCK_BYTES];
    size_t i;

    for (i = 0; i < SPX_N; ++i) {
        block[i] = pub_seed[i];
    }
    for (i = SPX_N; i < SPX_SHA256_BLOCK_BYTES; ++i) {
        block[i] = 0;
    }

    sha256_inc_init(state_seeded);
    sha256_inc_blocks(state_seeded, block, 1);
}
