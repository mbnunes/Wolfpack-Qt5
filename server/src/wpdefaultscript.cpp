//==================================================================================
//
//      Wolfpack Emu (WP)
//	UO Server Emulation Program
//
//	Copyright 1997, 98 by Marcus Rating (Cironian)
//  Copyright 2001 by holders identified in authors.txt
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
//	* In addition to that license, if you are running this program or modified
//	* versions of it on a public system you HAVE TO make the complete source of
//	* the version used by you available or provide people with a location to
//	* download it.
//
//
//
//	Wolfpack Homepage: http://wpdev.sf.net/
//========================================================================================

#include "wpdefaultscript.h"

// A method for WPDefaultScript
bool WPDefaultScript::canHandleSpeech( const QString &text, std::vector< UINT16 > keywords )
{
	// Check keywords first.
	for( std::vector< UINT16 >::const_iterator iter1 = keywords.begin(); iter1 != keywords.end(); ++iter1 )
		for( std::vector< UINT16 >::const_iterator iter2 = speechKeywords_.begin(); iter2 != speechKeywords_.end(); ++iter2 )
		{
			if( *iter1 == *iter2 )
				return true;
		}

	for( std::vector< QString >::const_iterator iter3 = speechWords_.begin(); iter3 != speechWords_.end(); ++iter3 )
		if( text.contains( *iter3 ) )
			return true;

	for( std::vector< QRegExp >::const_iterator iter4 = speechRegexp_.begin(); iter4 != speechRegexp_.end(); ++iter4 )
		if( text.contains( *iter4 ) )
			return true;

	return false;
}
