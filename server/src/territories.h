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

#include <map>

#include <qobject.h>

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
	cTerritory( const QDomElement &Tag, cBaseRegion *parent )
	{
		this->init();
		this->name_ = Tag.attribute( "id" );
		this->applyDefinition( Tag );
		this->parent_ = parent;
	}

	void	init( void );

	// Getters
	QString		name( void )			{ return name_; }
	QString		midilist( void )		{ return midilist_; }
	bool		isGuarded( void )			{ return flags_ & 0x0001; }
	bool		isNoMark( void )			{ return flags_ & 0x0002; }
	bool		isNoGate( void )			{ return flags_ & 0x0004; }
	bool		isNoRecallOut( void )		{ return flags_ & 0x0008; }
	bool		isNoRecallIn( void )		{ return flags_ & 0x0010; }
	bool		isRecallShield( void ) { return flags_ & 0x0020; }
	bool		isNoAgressiveMagic( void )	{ return flags_ & 0x0040; }
	bool		isAntiMagic( void )			{ return flags_ & 0x0080; }
	bool		isValidEscortRegion( void )	{ return flags_ & 0x0100; }
	bool		isCave( void )				{ return flags_ & 0x0200; }
	bool		isNoMusic( void )			{ return flags_ & 0x0400; }
	QString		guardOwner( void )		{ return guardowner_; }
	UI08		snowChance( void )		{ return snowchance_; }
	UI08		rainChance( void )		{ return rainchance_; }

	QString		getGuardSect( void );
private:
	// Setters to ease up the flag meanings
	void		setGuarded( bool data )				{ if( data ) flags_ |= 0x0001; else flags_ &= ~0x0001; }
	void		setNoMark( bool data )				{ if( data ) flags_ |= 0x0002; else flags_ &= ~0x0002; }
	void		setNoGate( bool data )				{ if( data ) flags_ |= 0x0004; else flags_ &= ~0x0004; }
	void		setNoRecallOut( bool data )			{ if( data ) flags_ |= 0x0008; else flags_ &= ~0x0008; }
	void		setNoRecallIn( bool data )			{ if( data ) flags_ |= 0x0010; else flags_ &= ~0x0010; }
	void		setRecallShield( bool data )		{ if( data ) flags_ |= 0x0020; else flags_ &= ~0x0020; }
	void		setNoAgressiveMagic( bool data )	{ if( data ) flags_ |= 0x0040; else flags_ &= ~0x0040; }
	void		setAntiMagic( bool data )			{ if( data ) flags_ |= 0x0080; else flags_ &= ~0x0080; }
	void		setValidEscortRegion( bool data )	{ if( data ) flags_ |= 0x0100; else flags_ &= ~0x0100; }
	void		setCave( bool data )				{ if( data ) flags_ |= 0x0200; else flags_ &= ~0x0200; }
	void		setNoMusic( bool data )				{ if( data ) flags_ |= 0x0400; else flags_ &= ~0x0400; }

	virtual void processNode( const QDomElement &Tag );

private:
	QString					midilist_;		// midilist to play

	UINT16					flags_;			// flags like guarded, mark allowed, etc. (see getters)
	
	QString					guardowner_;
	UI08					snowchance_;
	UI08					rainchance_;

	QStringList				guardSections_;
public:
	std::map< UI32, good_st >		tradesystem_;
};

class cAllTerritories : public QObject, public cAllBaseRegions
{
	Q_OBJECT
private:
	static cAllTerritories instance;
	cAllTerritories() {}

public:
	~cAllTerritories();

	void		load( void );
	void		check( P_CHAR pc );

	cTerritory* region( QString regName )
	{
		return dynamic_cast< cTerritory* >(this->topregion_->region( regName ));
	}

	cTerritory* region( UI16 posx, UI16 posy )
	{
		return dynamic_cast< cTerritory* >(this->topregion_->region( posx, posy ));
	}

	QString		getGuardSect( void );
	
	static cAllTerritories *getInstance( void ) { return &instance; }
};

#endif

