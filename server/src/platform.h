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
 * Wolfpack Homepage: http://wpdev.sf.net/
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
	inline float ceilf(float _X)
	{return ((float)ceil((double)_X)); }
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

typedef unsigned long UI32;
typedef unsigned short UI16;
typedef unsigned char UI08;
typedef signed long SI32;
typedef signed short SI16;
typedef signed char SI08;
typedef float RF32;
typedef double RF64;

#if defined(Q_CC_MSVC)

#if defined WPIMPORT
#define WPEXPORT __declspec(dllimport)
#else
#define WPEXPORT __declspec(dllexport)
#endif

#else

#undef WPEXPORT
#define WPEXPORT

#endif

#endif // __PLATFORM_H__
