
#if !defined(__TWOFISH_H__)
#define __TWOFISH_H__


#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#define u32 unsigned int
#if !defined(BYTE)
#define BYTE unsigned char
#endif

#define RS_MOD 0x14D
#define RHO 0x01010101L


void keySched( BYTE[], int, u32**, u32[40], int* );
void fullKey( u32[4], int, u32[4][256] );
void decrypt( u32[40], u32[4][256], BYTE* );
void encrypt( u32[40], u32[4][256], BYTE* );


#endif