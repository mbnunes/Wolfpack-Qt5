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

#if !defined(__TERRITORIES_H__)
#define __TERRITORIES_H__ 

#include "baseregion.h"

struct good_st
{
	UI32	sellable;
	UI32	buyable;
	UI32	rndmin;
	UI32	rndmax;
};

class cTerritory : public cBaseRegion
{
public:
	cTerritory( const QDomElement &Tag )
	{
		this->Init();
		this->name_ = Tag.parentNode().toElement().attribute( "id" );
		this->applyDefinition( Tag );
	}
	virtual ~cTerritory() {;}

	void	Init( void );
	bool	contains( UI16 posx, UI16 posy );

	// Getters
	QString		name( void )			{ return name_; }
	UI32		midilist( void )		{ return midilist_; }
	bool		isGuarded( void )		{ return guarded_; }
	bool		allowsMark( void )		{ return mark_; }
	bool		allowsGate( void )		{ return gate_; }
	bool		allowsRecall( void )	{ return recall_; }
	bool		canRain( void )			{ return rain_; }
	bool		canSnow( void )			{ return snow_; }
	bool		allowsMagicDamage()		{ return magicdamage_; }
	bool		isValidEscortRegion()	{ return escort_; }
	bool		allowsMagic( void )		{ return magic_; }
	QString		guardOwner( void )		{ return guardowner_; }
	UI08		snowChance( void )		{ return snowchance_; }
	UI08		rainChance( void )		{ return rainchance_; }

	QString		getGuardSect( void );
private:
	virtual void processNode( const QDomElement &Tag );

private:
	UI32					midilist_;		// midilist to play
	
	bool					guarded_;		// guarded area
	bool					mark_;			// mark allowed
	bool					gate_;			// gate allowed
	bool					recall_;		// recall
	bool					rain_;			// raining
	bool					snow_;			// snowing
	bool					magicdamage_;	// magic damage?
	bool					escort_;		// valid escort region
	bool					magic_;			// magic allowed

	QString					guardowner_;
	UI08					snowchance_;
	UI08					rainchance_;

	QStringList				guardSections_;
public:
	map< UI32, good_st >		tradesystem_;
};

class cAllTerritories : public cAllBaseRegions
{
private:
	static cAllTerritories instance;

public:
	cAllTerritories() {;}
	~cAllTerritories();

	void		Load( void );
	void		Check( P_CHAR pc );

	cTerritory*	region( QString regName );
	cTerritory* region( UI16 posx, UI16 posy );

	QString		getGuardSect( void );
	
	static cAllTerritories *getInstance( void ) { return &instance; }
};

#endif