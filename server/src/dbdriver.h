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

#if !defined(__DBDRIVER_H__)
#define __DBDRIVER_H__

struct st_mysql;
struct st_mysql_res;

#include <qstring.h>

class cDBDriver
{
private:
	st_mysql_res *result;
	char **row;
public:
	cDBDriver(): result( 0 ), row( 0 ) {}
	virtual ~cDBDriver() {}

	bool execute( const QString &query ); // Executes a query
	QString error(); // Returns an error (if there is one)
	bool fetchrow(); // Fetchs a new row, returns false if there is no new row
	void free(); // Call this to free the query
	char** data(); // Get the data for the current row
	INT32 getInt( UINT32 offset ); // Get an integer with a specific offset
	QString getString( UINT32 offset ); // Get a string with a specific offset
};

#undef MYSQL_RES

#endif 
