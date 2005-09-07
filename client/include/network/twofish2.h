/* aes.h */

/* ---------- See examples at end of this file for typical usage -------- */

/* AES Cipher header file for ANSI C Submissions
	Lawrence E. Bassham III
	Computer Security Division
	National Institute of Standards and Technology

	This sample is to assist implementers developing to the
Cryptographic API Profile for AES Candidate Algorithm Submissions.
Please consult this document as a cross-reference.

	ANY CHANGES, WHERE APPROPRIATE, TO INFORMATION PROVIDED IN THIS FILE
MUST BE DOCUMENTED. CHANGES ARE ONLY APPROPRIATE WHERE SPECIFIED WITH
THE STRING "CHANGE POSSIBLE". FUNCTION CALLS AND THEIR PARAMETERS
CANNOT BE CHANGED. STRUCTURES CAN BE ALTERED TO ALLOW IMPLEMENTERS TO
INCLUDE IMPLEMENTATION SPECIFIC INFORMATION.
*/

/* Includes:
	Standard include files
*/

#include	<stdio.h>

/***************************************************************************
	PLATFORM.H	-- Platform-specific defines for TWOFISH code

	Submitters:
		Bruce Schneier, Counterpane Systems
		Doug Whiting,	Hi/fn
		John Kelsey,	Counterpane Systems
		Chris Hall,		Counterpane Systems
		David Wagner,	UC Berkeley

	Code Author:		Doug Whiting,	Hi/fn

	Version  1.00		April 1998

	Copyright 1998, Hi/fn and Counterpane Systems.  All rights reserved.

	Notes:
		*	Tab size is set to 4 characters in this file

***************************************************************************/

#include <qglobal.h>

/* use intrinsic rotate if possible */
#define ROL( x, y ) ( ( (x) << (y) ) | ( (x) >> ( 32 - (y) ) ) )
#define ROR( x, y ) ( ( (x) >> (y) ) | ( (x) << ( 32 - (y) ) ) )

#if defined(_MSC_VER)
#include	<stdlib.h>					/* get prototypes for rotation functions */
#undef	ROL
#undef	ROR
#pragma intrinsic(_lrotl,_lrotr)		/* use intrinsic compiler rotations */
#define	ROL(x,n)	_lrotl(x,n)
#define	ROR(x,n)	_lrotr(x,n)
#endif

// Change this to compile on a BigEndian machine

#define ALIGN32 1

#if !defined(Q_OS_MAC)
#define		Bswap(x)			(x)		/* NOP for little-endian machines */
#define		ADDR_XOR			0		/* NOP for little-endian machines */
#else
#define		Bswap(x)			((ROR(x,8) & 0xFF00FF00) | (ROL(x,8) & 0x00FF00FF))
#define		ADDR_XOR			3		/* convert uchar address in ulong */
#endif

/*	Macros for extracting uchars from ulongs (correct for endianness) */
#define	_b(x,N)	(((uchar *)&x)[((N) & 3) ^ ADDR_XOR]) /* pick uchars out of a ulong */

#define		b0(x)			_b(x,0)		/* extract LSB of ulong */
#define		b1(x)			_b(x,1)
#define		b2(x)			_b(x,2)
#define		b3(x)			_b(x,3)		/* extract MSB of ulong */

/*	Defines:
		Add any additional defines you need
*/

namespace Twofish2 {

#define 	DIR_ENCRYPT 	0 		/* Are we encrpyting? */
#define 	DIR_DECRYPT 	1 		/* Are we decrpyting? */
#define 	MODE_ECB 		1 		/* Are we ciphering in ECB mode? */
#define 	MODE_CBC 		2 		/* Are we ciphering in CBC mode? */
#define 	MODE_CFB1 		3 		/* Are we ciphering in 1-bit CFB mode? */

#if !defined(TRUE)
#define 	TRUE 			1
#endif
#if !defined(FALSE)
#define 	FALSE 			0
#endif

#define 	BAD_KEY_DIR 		-1	/* Key direction is invalid (unknown value) */
#define 	BAD_KEY_MAT 		-2	/* Key material not of correct length */
#define 	BAD_KEY_INSTANCE 	-3	/* Key passed is not valid */
#define 	BAD_CIPHER_MODE 	-4 	/* Params struct passed to cipherInit invalid */
#define 	BAD_CIPHER_STATE 	-5 	/* Cipher in wrong state (e.g., not initialized) */

/* CHANGE POSSIBLE: inclusion of algorithm specific defines */
/* TWOFISH specific definitions */
#define		MAX_KEY_SIZE		64	/* # of ASCII chars needed to represent a key */
#define		MAX_IV_SIZE			16	/* # of uchars needed to represent an IV */
#define		BAD_INPUT_LEN		-6	/* inputLen not a multiple of block size */
#define		BAD_PARAMS			-7	/* invalid parameters */
#define		BAD_IV_MAT			-8	/* invalid IV text */
#define		BAD_ENDIAN			-9	/* incorrect endianness define */
#define		BAD_ALIGN32			-10	/* incorrect 32-bit alignment */

#define		BLOCK_SIZE			128	/* number of bits per block */
#define		MAX_ROUNDS			 16	/* max # rounds (for allocating subkey array) */
#define		ROUNDS_128			 16	/* default number of rounds for 128-bit keys*/
#define		ROUNDS_192			 16	/* default number of rounds for 192-bit keys*/
#define		ROUNDS_256			 16	/* default number of rounds for 256-bit keys*/
#define		MAX_KEY_BITS		256	/* max number of bits of key */
#define		MIN_KEY_BITS		128	/* min number of bits of key (zero pad) */
#define		VALID_SIG	 0x48534946	/* initialization signature ('FISH') */
#define		MCT_OUTER			400	/* MCT outer loop */
#define		MCT_INNER		  10000	/* MCT inner loop */
#define		REENTRANT			  1	/* nonzero forces reentrant code (slightly slower) */

#define		INPUT_WHITEN		0	/* subkey array indices */
#define		OUTPUT_WHITEN		( INPUT_WHITEN + BLOCK_SIZE/32)
#define		ROUND_SUBKEYS		(OUTPUT_WHITEN + BLOCK_SIZE/32)	/* use 2 * (# rounds) */
#define		TOTAL_SUBKEYS		(ROUND_SUBKEYS + 2*MAX_ROUNDS)

/* Typedefs:
	Typedef'ed data storage elements. Add any algorithm specific
	parameters at the bottom of the structs as appropriate.
*/

typedef ulong fullSbox[4][256];

/* The structure for key information */
typedef struct
{
	uchar direction;					/* Key used for encrypting or decrypting? */
#if ALIGN32
	uchar dummyAlign[3];				/* keep 32-bit alignment */
#endif
	int keyLen;					/* Length of the key */
	char keyMaterial[MAX_KEY_SIZE + 4];/* Raw key data in ASCII */

	/* Twofish-specific parameters: */
	ulong keySig;					/* set to VALID_SIG by makeKey() */
	int numRounds;				/* number of rounds in cipher */
	ulong key32[MAX_KEY_BITS / 32];	/* actual key bits, in ulongs */
	ulong sboxKeys[MAX_KEY_BITS / 64];/* key bits used for S-boxes */
	ulong subKeys[TOTAL_SUBKEYS];	/* round subkeys, input/output whitening bits */
#if REENTRANT
	fullSbox sBox8x32;				/* fully expanded S-box */
#if defined(COMPILE_KEY) && defined(USE_ASM)
#undef	VALID_SIG
#define	VALID_SIG	 0x504D4F43		/* 'COMP':  C is compiled with -DCOMPILE_KEY */
	ulong cSig1;					/* set after first "compile" (zero at "init") */
	void* encryptFuncPtr;			/* ptr to asm encrypt function */
	void* decryptFuncPtr;			/* ptr to asm decrypt function */
	ulong codeSize;					/* size of compiledCode */
	ulong cSig2;					/* set after first "compile" */
	uchar compiledCode[5000];		/* make room for the code itself */
#endif
#endif
} keyInstance;

/* The structure for cipher information */
typedef struct
{
	uchar mode;						/* MODE_ECB, MODE_CBC, or MODE_CFB1 */
#if ALIGN32
	uchar dummyAlign[3];				/* keep 32-bit alignment */
#endif
	uchar IV[MAX_IV_SIZE];			/* CFB1 iv uchars  (CBC uses iv32) */

	/* Twofish-specific parameters: */
	ulong cipherSig;				/* set to VALID_SIG by cipherInit() */
	ulong iv32[BLOCK_SIZE / 32];		/* CBC IV uchars arranged as ulongs */
} cipherInstance;

/* Function protoypes */
int makeKey( keyInstance* key, uchar direction, int keyLen, char* keyMaterial );

int cipherInit( cipherInstance* cipher, uchar mode, char* IV );

int blockEncrypt( cipherInstance* cipher, keyInstance* key, uchar* input, int inputLen, uchar* outBuffer );

int blockDecrypt( cipherInstance* cipher, keyInstance* key, uchar* input, int inputLen, uchar* outBuffer );

int reKey( keyInstance* key );	/* do key schedule using modified key.keyulongs */

/* API to check table usage, for use in ECB_TBL KAT */
#define		TAB_DISABLE			0
#define		TAB_ENABLE			1
#define		TAB_RESET			2
#define		TAB_QUERY			3
#define		TAB_MIN_QUERY		50
int TableOp( int op );

#ifndef CONST // warning C4005: 'CONST' : macro redefinition \microsoft visual studio\vc98\include\windef.h(138) : see previous definition of 'CONST'
#define		CONST				/* helpful C++ syntax sugar, NOP for ANSI C */
#endif

#if BLOCK_SIZE == 128			/* optimize block copies */
#define		Copy1(d,s,N)	((ulong *)(d))[N] = ((ulong *)(s))[N]
#define		BlockCopy(d,s)	{ Copy1(d,s,0);Copy1(d,s,1);Copy1(d,s,2);Copy1(d,s,3); }
#else
#define		BlockCopy(d,s)	{ memcpy(d,s,BLOCK_SIZE/8); }
#endif

/***************************************************************************
	TABLE.H	-- Tables, macros, constants for Twofish S-boxes and MDS matrix

	Submitters:
		Bruce Schneier, Counterpane Systems
		Doug Whiting,	Hi/fn
		John Kelsey,	Counterpane Systems
		Chris Hall,		Counterpane Systems
		David Wagner,	UC Berkeley

	Code Author:		Doug Whiting,	Hi/fn

	Version  1.00		April 1998

	Copyright 1998, Hi/fn and Counterpane Systems.  All rights reserved.

	Notes:
		*	Tab size is set to 4 characters in this file
		*	These definitions should be used in optimized and unoptimized
			versions to insure consistency.

***************************************************************************/

/* for computing subkeys */
#define	SK_STEP			0x02020202u
#define	SK_BUMP			0x01010101u
#define	SK_ROTL			9

/* Reed-Solomon code parameters: (12,8) reversible code
	g(x) = x**4 + (a + 1/a) x**3 + a x**2 + (a + 1/a) x + 1
   where a = primitive root of field generator 0x14D */
#define	RS_GF_FDBK		0x14D		/* field generator */
#define	RS_rem(x)		\
	{ uchar  b  = (uchar) (x >> 24);											 \
	  ulong g2 = ((b << 1) ^ ((b & 0x80) ? RS_GF_FDBK : 0 )) & 0xFF;		 \
	  ulong g3 = ((b >> 1) & 0x7F) ^ ((b & 1) ? RS_GF_FDBK >> 1 : 0 ) ^ g2 ; \
	  x = (x << 8) ^ (g3 << 24) ^ (g2 << 16) ^ (g3 << 8) ^ b;				 \
	}

/*	Macros for the MDS matrix
*	The MDS matrix is (using primitive polynomial 169):
*      01  EF  5B  5B
*      5B  EF  EF  01
*      EF  5B  01  EF
*      EF  01  EF  5B
*----------------------------------------------------------------
* More statistical properties of this matrix (from MDS.EXE output):
*
* Min Hamming weight (one uchar difference) =  8. Max=26.  Total =  1020.
* Prob[8]:  	7    23    42    20    52    95    88    94   121   128    91
*   		  102    76    41    24 	8     4 	1     3 	0     0 	0
* Runs[8]:  	2     4 	5     6 	7     8 	9    11
* MSBs[8]:  	1     4    15     8    18    38    40    43
* HW= 8: 05040705 0A080E0A 14101C14 28203828 50407050 01499101 A080E0A0
* HW= 9: 04050707 080A0E0E 10141C1C 20283838 40507070 80A0E0E0 C6432020 07070504
*   	 0E0E0A08 1C1C1410 38382820 70705040 E0E0A080 202043C6 05070407 0A0E080E
*   	 141C101C 28382038 50704070 A0E080E0 4320C620 02924B02 089A4508
* Min Hamming weight (two uchar difference) =  3. Max=28.  Total = 390150.
* Prob[3]:  	7    18    55   149   270   914  2185  5761 11363 20719 32079
*   		43492 51612 53851 52098 42015 31117 20854 11538  6223  2492  1033
* MDS OK, ROR:   6+  7+  8+  9+ 10+ 11+ 12+ 13+ 14+ 15+ 16+
*   			17+ 18+ 19+ 20+ 21+ 22+ 23+ 24+ 25+ 26+
*/
#define	MDS_GF_FDBK		0x169	/* primitive polynomial for GF(256)*/
#define	LFSR1(x) ( ((x) >> 1)  ^ (((x) & 0x01) ?   MDS_GF_FDBK/2 : 0))
#define	LFSR2(x) ( ((x) >> 2)  ^ (((x) & 0x02) ?   MDS_GF_FDBK/2 : 0)  \
							   ^ (((x) & 0x01) ?   MDS_GF_FDBK/4 : 0))

#define	Mx_1(x) ((ulong)  (x))		/* force result to ulong so << will work */
#define	Mx_X(x) ((ulong) ((x) ^ 		   LFSR2(x)))	/* 5B */
#define	Mx_Y(x) ((ulong) ((x) ^ LFSR1(x) ^ LFSR2(x)))	/* EF */

#define	M00		Mul_1
#define	M01		Mul_Y
#define	M02		Mul_X
#define	M03		Mul_X

#define	M10		Mul_X
#define	M11		Mul_Y
#define	M12		Mul_Y
#define	M13		Mul_1

#define	M20		Mul_Y
#define	M21		Mul_X
#define	M22		Mul_1
#define	M23		Mul_Y

#define	M30		Mul_Y
#define	M31		Mul_1
#define	M32		Mul_Y
#define	M33		Mul_X

#define	Mul_1	Mx_1
#define	Mul_X	Mx_X
#define	Mul_Y	Mx_Y

/*	Define the fixed p0/p1 permutations used in keyed S-box lookup.
	By changing the following constant definitions for P_ij, the S-boxes will
	automatically get changed in all the Twofish source code. Note that P_i0 is
	the "outermost" 8x8 permutation applied.  See the f32() function to see
	how these constants are to be  used.
*/
#define	P_00	1					/* "outermost" permutation */
#define	P_01	0
#define	P_02	0
#define	P_03	(P_01^1)			/* "extend" to larger key sizes */
#define	P_04	1

#define	P_10	0
#define	P_11	0
#define	P_12	1
#define	P_13	(P_11^1)
#define	P_14	0

#define	P_20	1
#define	P_21	1
#define	P_22	0
#define	P_23	(P_21^1)
#define	P_24	0

#define	P_30	0
#define	P_31	1
#define	P_32	1
#define	P_33	(P_31^1)
#define	P_34	1

#define	p8(N)	P8x8[P_##N]			/* some syntax shorthand */

}