//==================================================================================
//
//      Wolfpack Emu (WP)
//	UO Server Emulation Program
//
//	Copyright 1997, 98 by Marcus Rating (Cironian)
//  Copyright 2001-2003 by holders identified in authors.txt
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
//	Foundation, Inc., 59 Temple Palace - Suite 330, Boston, MA 02111-1307, USA.
//
//	* In addition to that license, if you are running this program or modified
//	* versions of it on a public system you HAVE TO make the complete source of
//	* the version used by you available or provide people with a location to
//	* download it.
//
//
//
//	Wolfpack Homepage: http://wpdev.sf.net/
//==================================================================================

#if !defined(__BOATS_H__)
#define __BOATS_H__

// Platform specifics
#include "platform.h"

#include "multis.h"

// Forward class Declaration
class QString;

#define X 0
#define Y 1

//itemoffsets
#define PORT_PLANK 0
#define STARB_PLANK 1
#define HOLD 2
#define TILLER 3

//itemids
#define PORT_P_C 0//Port Plank Closed
#define PORT_P_O 1//Port Planl Opened
#define STAR_P_C 2//Starboard Plank Closed
#define STAR_P_O 3//Starb Plank Open
#define HOLD_ID 4//Hold
#define TILLER_ID 5//Tiller


// this class had been more likely a namespace than a class. i decided to make it derival of cItem
// like the house multis. so it needs the processNode and the objectID methods!
// be aware when changing things in here!
// - sereg

class QStringList;

class cBoat : public cMulti
{
private:
	static void buildSqlString( QStringList &fields, QStringList &tables, QStringList &conditions );

public:
	const char *objectID() const
	{
        return "cBoat";
	}

	cBoat();

	// PersistentObject
	static void registerInFactory();
	void load( char **, UINT16& );
	void save();
	bool del();

	void handlePlankClick( cUOSocket* socket, P_ITEM pplank );
	void switchPlankState( P_ITEM pplank );

	void build( const cElement *Tag, UI16 posx, UI16 posy, SI08 posz, SERIAL senderserial, SERIAL deedserial );
	bool move( void );
	void turn( SI08 turn );

	char speechInput( cUOSocket* socket, const QString &input );

	virtual void toDeed( cUOSocket* socket );

	//void setAutoSail( UOXSOCKET s, P_ITEM pMap );
	bool doesAutoSail( void ) { return autosail_ > 0; }

	QString deedSection( void ) { return deedsection_; }
	UI08	boatDir( void ) { return boatdir; }

	bool	ishouse() { return false; }
	bool	isboat() { return true; }

protected:
	virtual void processNode( const cElement *Tag );
	void	processSpecialItemNode( const cElement *Tag, UI08 item );

	bool leave( cUOSocket* socket, P_ITEM pplank );

	bool isValidPlace( UI16 posx, UI16 posy, SI08 posz, UI08 boatdir );

protected:
	SI08	moves_; // 1 - forward, 0 - stop, -1 - backward
	SI08	shift_; // 1 - right, 0 - none, -1 - left
	UI08	boatdir;
	UI08	autosail_;
	std::vector< UI16 > multiids_;
	signed short itemoffsets[4][4][2];
	unsigned short itemids[4][6];
	SERIAL itemserials[4];


/*
autosail_ :		a boolean which defines if the boat is autosailing (using a map) or not

multiids_ :		the multi's ids where the index in the vector marks the boatdirection
				DEFINABLE in the xml definition of the boat multi

itemoffsets :	3 dimensional array which holds the item's x/y offsets for each boatdirection
				[4]=boatdirection of ship
				[4]=Which Item (PT Plank, SB Plank, Hatch, TMan)
				[2]=Coord (x,y) offsets
				DEFINABLE in the xml definition of the boat multi

itemids :		2 dimension array which holds the item's ids for each boatdirection
				[4] = boatdirection
				[6] = Which Item (PT Plank Up,PT Plank Down, SB Plank Up, SB Plank Down, Hatch, TMan)
				DEFINABLE in the xml definition of the boat multi
*/
};


#endif

