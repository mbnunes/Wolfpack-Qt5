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
		this->name_ = Tag.attribute( "id" );
		this->applyDefinition( Tag );
	}
	virtual ~cTerritory() {;}

	void	Init( void );
	bool	contains( UI16 posx, UI16 posy );

	// Getters
	QString		name( void )			{ return name_; }
	UI32		midilist( void )		{ return midilist_; }
	bool		isGuarded( void )		{ return flags_ & 0x01; }
	bool		allowsMark( void )		{ return flags_ & 0x02; }
	bool		allowsGate( void )		{ return flags_ & 0x04; }
	bool		allowsRecall( void )	{ return flags_ & 0x08; }
	bool		allowsMagicDamage()		{ return flags_ & 0x10; }
	bool		isValidEscortRegion()	{ return flags_ & 0x20; }
	bool		allowsMagic( void )		{ return flags_ & 0x40; }
	QString		guardOwner( void )		{ return guardowner_; }
	UI08		snowChance( void )		{ return snowchance_; }
	UI08		rainChance( void )		{ return rainchance_; }

	QString		getGuardSect( void );
private:
	// Setters to ease up the flag meanings
	void		setGuarded( bool data )		{ (data) ? flags_ &= 0xFE : flags_ |= 0x01; }
	void		setMark( bool data )		{ (data) ? flags_ &= 0xFD : flags_ |= 0x02; }
	void		setGate( bool data )		{ (data) ? flags_ &= 0xFB : flags_ |= 0x04; }
	void		setRecall( bool data )		{ (data) ? flags_ &= 0xF7 : flags_ |= 0x08; }
	void		setMagicDamage( bool data ) { (data) ? flags_ &= 0xEF : flags_ |= 0x10; }
	void		setEscortRegion( bool data) { (data) ? flags_ &= 0xDF : flags_ |= 0x20; }
	void		setMagic( bool data )		{ (data) ? flags_ &= 0xBF : flags_ |= 0x40; }

	virtual void processNode( const QDomElement &Tag );

private:
	UI32					midilist_;		// midilist to play

	UI08					flags_;			// flags like guarded, mark allowed, etc. (see getters)
	
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
	cTerritory* region( UI16 posx, UI16 posy )
	{
		iterator it = this->begin();
		cTerritory* Region = NULL;
		while( it != this->end() && Region == NULL )
		{
			Region = dynamic_cast< cTerritory* >(it->second);
			if( Region != NULL && !Region->contains( posx, posy ) )
				Region = NULL;
			it++;
		}

		return Region;
	}

	QString		getGuardSect( void );
	
	static cAllTerritories *getInstance( void ) { return &instance; }
};

#endif