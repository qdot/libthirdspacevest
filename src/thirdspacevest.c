/*
 * Third Space Vest Driver - General Functions
 *
 * Copyright (c) 2010 Kyle Machulis/Nonpolynomial Labs <kyle@nonpolynomial.com>
 *
 * More info on Nonpolynomial Labs @ http://www.nonpolynomial.com
 *
 * Source code available at http://www.github.com/qdot/libthirdspacevest/
 *
 * This library is covered by the BSD License
 * Read LICENSE_BSD.txt for details.
 */

#include "thirdspacevest/thirdspacevest.h"
#include <stdio.h>

int thirdspacevest_form_checksum(uint8_t index, uint8_t speed)
{
	uint8_t a, b, c, d;
	a = (THIRDSPACEVEST_CRC8_TABLE[index] << 4);
	b = (THIRDSPACEVEST_CRC8_TABLE[index] >> 4) ^ (speed >> 4);
	c = a ^ THIRDSPACEVEST_CRC8_TABLE[b];
	d = (c << 4);
	return THIRDSPACEVEST_CRC8_TABLE[((c >> 4) ^ speed) & 0x0F] ^ d;
}

// Taken from http://en.wikipedia.org/wiki/Tiny_Encryption_Algorithm
void thirdspacevest_encrypt (uint32_t* v, uint32_t* k) {
    uint32_t v0=v[0], v1=v[1], sum=0, i;           /* set up */
    uint32_t delta=0x9e3779b9;                     /* a key schedule constant */
    uint32_t k0=k[0], k1=k[1], k2=k[2], k3=k[3];   /* cache key */

    for (i=0; i < 32; i++) {                       /* basic cycle start */
        sum += delta;
        v0 += ((v1<<4) + k0) ^ (v1 + sum) ^ ((v1>>5) + k1);
        v1 += ((v0<<4) + k2) ^ (v0 + sum) ^ ((v0>>5) + k3);  
    }                                              /* end cycle */
    v[0]=v0; v[1]=v1;
}

// Taken from http://en.wikipedia.org/wiki/Tiny_Encryption_Algorithm
void thirdspacevest_decrypt (uint32_t* v, uint32_t* k) {
    uint32_t v0=v[0], v1=v[1], sum=0xC6EF3720, i;  /* set up */
    uint32_t delta=0x9e3779b9;                     /* a key schedule constant */
    uint32_t k0=k[0], k1=k[1], k2=k[2], k3=k[3];   /* cache key */
    for (i=0; i<32; i++) {                         /* basic cycle start */
        v1 -= ((v0<<4) + k2) ^ (v0 + sum) ^ ((v0>>5) + k3);
        v0 -= ((v1<<4) + k0) ^ (v1 + sum) ^ ((v1>>5) + k1);
        sum -= delta;                                   
    }                                              /* end cycle */
    v[0]=v0; v[1]=v1;
}

void thirdspacevest_form_cache_key(uint8_t* cache_key_index, uint32_t* key_store)
{
	int i, j;
	srand ( time(NULL) );
	*cache_key_index = 0x1D;//rand() % 255;
	for(i = 0; i < 4; ++i)
	{
		key_store[i] =
			THIRDSPACEVEST_CACHE_KEY_TABLE[*cache_key_index + (4 * i) + 3] << 24 |
			THIRDSPACEVEST_CACHE_KEY_TABLE[*cache_key_index + (4 * i) + 2] << 16 |
			THIRDSPACEVEST_CACHE_KEY_TABLE[*cache_key_index + (4 * i) + 1] << 8 |
			THIRDSPACEVEST_CACHE_KEY_TABLE[*cache_key_index + (4 * i) + 0] << 0;
	}
}

int thirdspacevest_send_effect(thirdspacevest_device* dev, uint8_t index, uint8_t speed)
{
	uint8_t cache_key_index;
	uint32_t cache_key[4];
	int i = 0;
	thirdspacevest_form_cache_key(&cache_key_index, cache_key);
	{
		uint8_t packet[10] = {
			0x2, cache_key_index,
			0x0, 0x0, 0x0, 0x0,
			0x0, thirdspacevest_form_checksum(index, speed), index, speed};
		uint8_t ret[10];
		thirdspacevest_encrypt((uint32_t*)(packet+2), cache_key);
		thirdspacevest_write_data(dev, packet);
		thirdspacevest_read_data(dev, ret);
	}
}
