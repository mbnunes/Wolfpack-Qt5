/*
*     Wolfpack Emu (WP)
* UO Server Emulation Program
*
* Copyright 2001-2004 by holders identified in AUTHORS.txt
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Palace - Suite 330, Boston, MA 02111-1307, USA.
*
* In addition to that license, if you are running this program or modified
* versions of it on a public system you HAVE TO make the complete source of
* the version used by you available or provide people with a location to
* download it.
*
* Wolfpack Homepage: http://developer.berlios.de/projects/wolfpack/
*/

#if !defined( __PLATFORM_H__ )
#define __PLATFORM_H__

#include <qglobal.h>

#if defined(Q_CC_MSVC) | defined(Q_CC_INTEL) | defined(Q_CC_BOR)
#	pragma pack(8)
#	pragma warning(disable: 4514)
#	pragma warning(disable: 4786)
#	pragma warning(disable: 4710)
#	pragma warning(disable: 4702)
// The intel compiler doesn't like this one
#	pragma warning(disable: 985)
#	define PACK
#endif

#if defined(Q_CC_GNU)
#	define PACK __attribute__((packed))
#endif

#if defined (Q_OS_FREEBSD)
#	include <math.h>
inline float ceilf( float _X )
{
	return ( ( float ) ceil( ( double ) _X ) );
}
#endif

/*!
* The WP_ISLIKELY macro tags a boolean expression as likely to evaluate to
* 'true'. When used in an if ( ) statement, it gives a hint to the compiler
* that the following code block is likely to get executed. Providing this
* information helps the compiler to optimize the code for better performance.
* Using the macro has an insignificant code size or runtime memory footprint impact.
* The code semantics is not affected.
*
* \note
* Providing wrong information ( like marking a condition that almost never
* passes as 'likely' ) will cause a significant runtime slowdown. Therefore only
* use it for cases where you can be sure about the odds of the expression to pass
* in all cases ( independent from e.g. user configuration ).
*
* \par
* The WP_ISUNLIKELY macro tags an expression as unlikely evaluating to 'true'.
*
* \note
* Do NOT use ( !WP_ISLIKELY(foo) ) as an replacement for WP_ISUNLIKELY !
*
* \code
* if ( WP_ISUNLIKELY( testsomething() ) )
*     abort();     // assume its unlikely that the application aborts
* \endcode
*/
#if __GNUC__ - 0 >= 3
#	define WP_ISLIKELY( x )    __builtin_expect(!!(x),1)
#	define WP_ISUNLIKELY( x )  __builtin_expect(!!(x),0)
#else
#	define WP_ISLIKELY( x )   ( x )
#	define WP_ISUNLIKELY( x )  ( x )
#endif

typedef Q_UINT64	UI64;
typedef Q_UINT32	UI32;
typedef Q_UINT16	UI16;
typedef Q_UINT8		UI08;
typedef Q_INT64		SI64;
typedef Q_INT32		SI32;
typedef Q_INT16		SI16;
typedef Q_INT8		SI08;
typedef float		RF32;
typedef double		RF64;

#if defined(Q_CC_MSVC)
#	if defined(WPIMPORT)
#		define WPEXPORT __declspec(dllimport)
#	else
#		define WPEXPORT __declspec(dllexport)
#	endif
#else
#	undef WPEXPORT
#	define WPEXPORT
#endif


/************************************************************************/
/* CPU Endian Abstraction   											*/
/************************************************************************/

/*!
A handy little method to swap the bytes of any int-style datatype around 
Based on routine from the MUSCLE framework (http://freshmeat.net/projects/muscle/)
*/
template <typename T>
inline T wpSwapBytes( T swapMe )
{
	T retVal;
	const Q_UINT8 * readFrom = ( const Q_UINT8* ) &swapMe;
	Q_UINT8 * writeTo = ( ( Q_UINT8* ) &retVal ) + sizeof( retVal );
	for ( Q_UINT16 i = 0; i < sizeof( swapMe ); ++i )
	{
		*( --writeTo ) = *( readFrom++ );
	}
	return retVal;
}

#if defined( Q_CC_GNU )
#	if ( defined( __PPC__ ) || defined(__APPLE__) )
#		define WP_USE_POWERPC_INLINE_ASSEMBLY 1
#	elif defined( __i386__ )
#		define WP_USE_X86_INLINE_ASSEMBLY 1
#	endif
#endif

#if defined(WP_USE_POWERPC_INLINE_ASSEMBLY)
static inline Q_UINT16 wpPowerPCSwapInt16( Q_UINT16 val )
{
	Q_UINT16 a;
	volatile Q_UINT16 * addr = &a;
	__asm__ __volatile__("sthbrx %1,0,%2" : "=m" (*addr) : "r" (val), "r" (addr));
	return a;
}
static inline Q_UINT32 wpPowerPCSwapInt32( Q_UINT32 val )
{
	Q_UINT32 a;
	volatile Q_UINT32 * addr = &a;
	__asm__ __volatile__("stwbrx %1,0,%2" : "=m" (*addr) : "r" (val), "r" (addr));
	return a;
}
static inline float wpPowerPCSwapFloat( float val )
{
	float a;
	volatile float * addr = &a;
	__asm__ __volatile__("stwbrx %1,0,%2" : "=m" (*addr) : "r" (val), "r" (addr));
	return a;
}
static inline Q_UINT64 wpPowerPCSwapInt64( Q_UINT64 val )
{
	return ( ( Q_UINT64 ) ( wpPowerPCSwapInt32( ( Q_UINT32 ) ( ( val >> 32 ) & 0xFFFFFFFF ) ) ) ) | ( ( ( Q_UINT64 ) ( wpPowerPCSwapInt32( ( Q_UINT32 ) ( val & 0xFFFFFFFF ) ) ) ) << 32 );
}
static inline double wpPowerPCSwapDouble( double val )
{
	Q_UINT64 v64 = wpPowerPCSwapInt64( *( ( Q_UINT64* ) &val ) );
	return *( ( double * ) &v64 );
}
#  define B_SWAP_DOUBLE(arg)   wpPowerPCSwapDouble((double)(arg))
#  define B_SWAP_FLOAT(arg)    wpPowerPCSwapFloat((float)(arg))
#  define B_SWAP_INT64(arg)    wpPowerPCSwapInt64((Q_UINT64)(arg))
#  define B_SWAP_INT32(arg)    wpPowerPCSwapInt32((Q_UINT32)(arg))
#  define B_SWAP_INT16(arg)    wpPowerPCSwapInt16((Q_UINT16)(arg))
#elif defined(WP_USE_X86_INLINE_ASSEMBLY)
static inline Q_UINT16 wpX86SwapInt16( Q_UINT16 val )
{
	__asm__ __volatile__ ("xchgb %b0,%h0" : "=q" (val) : "0" (val));
	return val;
}
static inline Q_UINT32 wpX86SwapInt32( Q_UINT32 val )
{
	__asm__ __volatile__ ("bswap %0" : "+r" (val));
	return val;
}
static inline float wpX86SwapFloat( float val )
{
	__asm__ __volatile__ ("bswap %0" : "+r" (val));
	return val;
}
static inline Q_UINT64 wpX86SwapInt64( Q_UINT64 val )
{
	return ( ( Q_UINT64 ) ( wpX86SwapInt32( ( Q_UINT32 ) ( ( val >> 32 ) & 0xFFFFFFFF ) ) ) ) | ( ( ( Q_UINT64 ) ( wpX86SwapInt32( ( Q_UINT64 ) ( val & 0xFFFFFFFF ) ) ) ) << 32 );
}
static inline double wpX86SwapDouble( double val )
{
	Q_UINT64 v64 = wpX86SwapInt64( *( ( Q_UINT64* ) &val ) );
	return *( ( double * ) &v64 );
}
#  define B_SWAP_DOUBLE(arg)   wpX86SwapDouble((double)(arg))
#  define B_SWAP_FLOAT(arg)    wpX86SwapFloat((float)(arg))
#  define B_SWAP_INT64(arg)    wpX86SwapInt64((Q_UINT64)(arg))
#  define B_SWAP_INT32(arg)    wpX86SwapInt32((Q_UINT32)(arg))
#  define B_SWAP_INT16(arg)    wpX86SwapInt16((Q_UINT16)(arg))
# else
#  define B_SWAP_DOUBLE(arg)   wpSwapBytes((double)(arg))
#  define B_SWAP_FLOAT(arg)    wpSwapBytes((float)(arg))
#  define B_SWAP_INT64(arg)    wpSwapBytes((Q_UINT64)(arg))
#  define B_SWAP_INT32(arg)    wpSwapBytes((Q_UINT32)(arg))
#  define B_SWAP_INT16(arg)    wpSwapBytes((Q_UINT16)(arg))
# endif

#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
#  define B_HOST_TO_LENDIAN_DOUBLE(arg) ((double)(arg))
#  define B_HOST_TO_LENDIAN_FLOAT(arg)  ((float)(arg))
#  define B_HOST_TO_LENDIAN_INT64(arg)  ((Q_UINT64)(arg))
#  define B_HOST_TO_LENDIAN_INT32(arg)  ((Q_UINT32)(arg))
#  define B_HOST_TO_LENDIAN_INT16(arg)  ((Q_UINT16)(arg))
#  define B_HOST_TO_BENDIAN_DOUBLE(arg) B_SWAP_DOUBLE(arg)
#  define B_HOST_TO_BENDIAN_FLOAT(arg)  B_SWAP_FLOAT(arg)
#  define B_HOST_TO_BENDIAN_INT64(arg)  B_SWAP_INT64(arg)
#  define B_HOST_TO_BENDIAN_INT32(arg)  B_SWAP_INT32(arg)
#  define B_HOST_TO_BENDIAN_INT16(arg)  B_SWAP_INT16(arg)
#  define B_LENDIAN_TO_HOST_DOUBLE(arg) ((double)(arg))
#  define B_LENDIAN_TO_HOST_FLOAT(arg)  ((float)(arg))
#  define B_LENDIAN_TO_HOST_INT64(arg)  ((Q_UINT64)(arg))
#  define B_LENDIAN_TO_HOST_INT32(arg)  ((Q_UINT32)(arg))
#  define B_LENDIAN_TO_HOST_INT16(arg)  ((Q_UINT16)(arg))
#  define B_BENDIAN_TO_HOST_DOUBLE(arg) B_SWAP_DOUBLE(arg)
#  define B_BENDIAN_TO_HOST_FLOAT(arg)  B_SWAP_FLOAT(arg)
#  define B_BENDIAN_TO_HOST_INT64(arg)  B_SWAP_INT64(arg)
#  define B_BENDIAN_TO_HOST_INT32(arg)  B_SWAP_INT32(arg)
#  define B_BENDIAN_TO_HOST_INT16(arg)  B_SWAP_INT16(arg)
#elif Q_BYTE_ORDER == Q_BIG_ENDIAN
#  define B_HOST_IS_LENDIAN 0
#  define B_HOST_IS_BENDIAN 1
#  define B_HOST_TO_LENDIAN_DOUBLE(arg) B_SWAP_DOUBLE(arg)
#  define B_HOST_TO_LENDIAN_FLOAT(arg)  B_SWAP_FLOAT(arg)
#  define B_HOST_TO_LENDIAN_INT64(arg)  B_SWAP_INT64(arg)
#  define B_HOST_TO_LENDIAN_INT32(arg)  B_SWAP_INT32(arg)
#  define B_HOST_TO_LENDIAN_INT16(arg)  B_SWAP_INT16(arg)
#  define B_HOST_TO_BENDIAN_DOUBLE(arg) ((double)(arg))
#  define B_HOST_TO_BENDIAN_FLOAT(arg)  ((float)(arg))
#  define B_HOST_TO_BENDIAN_INT64(arg)  ((Q_UINT64)(arg))
#  define B_HOST_TO_BENDIAN_INT32(arg)  ((Q_UINT32)(arg))
#  define B_HOST_TO_BENDIAN_INT16(arg)  ((Q_UINT16)(arg))
#  define B_LENDIAN_TO_HOST_DOUBLE(arg) B_SWAP_DOUBLE(arg)
#  define B_LENDIAN_TO_HOST_FLOAT(arg)  B_SWAP_FLOAT(arg)
#  define B_LENDIAN_TO_HOST_INT64(arg)  B_SWAP_INT64(arg)
#  define B_LENDIAN_TO_HOST_INT32(arg)  B_SWAP_INT32(arg)
#  define B_LENDIAN_TO_HOST_INT16(arg)  B_SWAP_INT16(arg)
#  define B_BENDIAN_TO_HOST_DOUBLE(arg) ((double)(arg))
#  define B_BENDIAN_TO_HOST_FLOAT(arg)  ((float)(arg))
#  define B_BENDIAN_TO_HOST_INT64(arg)  ((Q_UINT64)(arg))
#  define B_BENDIAN_TO_HOST_INT32(arg)  ((Q_UINT32)(arg))
#  define B_BENDIAN_TO_HOST_INT16(arg)  ((Q_UINT16)(arg))
#else
#	error "Undefined Q_BYTE_ORDER????"
#endif

#endif // __PLATFORM_H__
