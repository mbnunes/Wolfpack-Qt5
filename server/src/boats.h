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

#if !defined(__BOATS_H__)
#define __BOATS_H__

// Platform specifics
#include "platform.h"

#include "uobject.h"
#include "items.h"

// Forward class Declaration
class cBoat;
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

class cBoat : public cItem
{
public:
	cBoat();
	virtual ~cBoat() {;}

	virtual void Serialize( ISerialization &archive )
	{
		if( archive.isReading() )
		{
			archive.read( "boat.autosail", autosail_ );
			archive.read( "boat.dir", boatdir );
			int i,j,k;
			for( i = 0; i < 8; i++ )
			{
				int currid = 0;
				archive.read( "boat.multiid", currid );
				this->multiids_.push_back( currid );
			}
			for( i = 0; i < 4; i++ )
			{
				for( j = 0; j < 4; j++ )
				{
					for( k = 0; k < 2; k++ )
					{
						signed short curroffset = 0;
						archive.read( (char*)QString("boat.itemoffset.%1.%2.%3").arg(i).arg(j).arg(k).latin1(), curroffset );
						this->itemoffsets[i][j][k] = curroffset;
					}
				}
			}
			for( i = 0; i < 4; i ++ )
			{
				for( j = 0; j < 6; j++ )
				{
					unsigned short currid = 0;
					archive.read( (char*)QString("boat.itemid.%1.%2").arg(i).arg(j).latin1(), currid );
					this->itemids[i][j] = currid;
				}
			}
		}
		else
		{
			archive.write( "boat.autosail", autosail_ );
			archive.write( "boat.dir", boatdir );
			int i, j, k;
			for( i = 0; i < 8; i++ )
			{
				int currid = this->multiids_[i];
				archive.write( "boat.multiid", currid );
			}
			for( i = 0; i < 3; i++ )
			{
				for( j = 0; j < 3; j++ )
				{
					for( k = 0; k < 1; k++ )
					{
						archive.write( (char*)QString("boat.itemoffset.%1.%2.%3").arg(i).arg(j).arg(k).latin1(), this->itemoffsets[i][j][k] );
					}
				}
			}
			for( i = 0; i < 4; i ++ )
			{
				for( j = 0; j < 6; j++ )
				{
					archive.write( (char*)QString("boat.itemid.%1.%2").arg(i).arg(j).latin1(), this->itemids[i][j] );
				}
			}
		}
		cItem::Serialize( archive );
	}
	virtual QString objectID( void ) const { return "BOAT"; }

	void handlePlankClick( UOXSOCKET s, P_ITEM pplank );
	void switchPlankState( P_ITEM pplank );

	void build( const QDomElement &Tag, UI16 posx, UI16 posy, SI08 posz, SERIAL senderserial, SERIAL deedserial );
	bool move( void );
	void turn( SI08 turn );

	char speechInput( UOXSOCKET s, const QString &input );

	void toDeed( UOXSOCKET s );

	//void setAutoSail( UOXSOCKET s, P_ITEM pMap );
	bool doesAutoSail( void ) { return autosail_ > 0; }

	QString deedSection( void ) { return deedsection_; }
	UI08	boatDir( void ) { return boatdir; }

protected:
	virtual void processNode( const QDomElement &Tag );
	void	processSpecialItemNode( const QDomElement &Tag, UI08 item );

	bool leave( UOXSOCKET s, P_ITEM pplank );

	bool isValidPlace( UI16 posx, UI16 posy, SI08 posz, UI08 boatdir );

protected:
	bool	moves_;
	UI08	boatdir;
	UI08	autosail_;
	std::vector< UI16 > multiids_;
	signed short itemoffsets[4][4][2];
	unsigned short itemids[4][6];
	SERIAL itemserials[4];
	QString deedsection_;


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

deedsection_ :	the script section in one of the xml definition files for the deed
				which will be created when turning the boat into a deed (toDeed)
				DEFINABLE in the xml definition of the boat multi
*/
};
#endif

