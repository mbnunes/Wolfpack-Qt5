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

#if !defined(__TARGETREQUEST_H__)
#define __TARGETREQUEST_H__

#include "inlines.h"
#include "network/uosocket.h"
#include "network/uorxpackets.h"
#include "wptargetrequests.h"
#include "items.h"
#include "corpse.h"
#include "gumps.h"
#include "world.h"
#include "player.h"
#include "tilecache.h"
#include "npc.h"
#include "ai.h"


class cAddItemTarget: public cTargetRequest
{
	QString item_;
	bool nodecay;
public:
	cAddItemTarget( const QString &item, bool _nodecay = false ) : nodecay( _nodecay ), item_(item) {}
	bool responsed( cUOSocket *socket, cUORxTarget *target );
};

class cAddNpcTarget: public cTargetRequest
{
	QString npc_;
public:
	cAddNpcTarget( const QString &npc ) : npc_(npc) {}
	bool responsed( cUOSocket *socket, cUORxTarget *target );
};

class cBuildMultiTarget: public cTargetRequest
{
	QString multisection_;
	SERIAL	deedserial_;
	SERIAL	senderserial_;
public:
	cBuildMultiTarget( const QString &multisection, SERIAL senderserial, SERIAL deedserial ) { multisection_ = multisection; senderserial_ = senderserial; deedserial_ = deedserial; };
	bool responsed( cUOSocket *socket, cUORxTarget *target );
};

class cSetMultiOwnerTarget: public cTargetRequest
{
	
	SERIAL multi_;
	bool coowner_;
public:
	cSetMultiOwnerTarget( SERIAL multiserial, bool coowner = false ) : multi_(multiserial), coowner_(coowner) {}

	bool responsed( cUOSocket *socket, cUORxTarget *target );
};

class cMultiAddToListTarget: public cTargetRequest
{
	
	SERIAL	multi_;
	bool	banlist_;
public:
	cMultiAddToListTarget( SERIAL multiserial, bool banlist = false ) : multi_(multiserial), banlist_(banlist) {}

	bool responsed( cUOSocket *socket, cUORxTarget *target );
};

class cMultiChangeLockTarget: public cTargetRequest
{
	
private:
	SERIAL multi_;
public:
	cMultiChangeLockTarget( SERIAL multiserial ) { multi_ = multiserial; }

	bool responsed( cUOSocket *socket, cUORxTarget *target );
};

// Stealing
class cSkStealing: public cTargetRequest
{
	
public:
	bool cSkStealing::responsed( cUOSocket *socket, cUORxTarget *target );
};


// Forensics Evaluation
class cSkForensics: public cTargetRequest
{
	
public:
	virtual bool responsed( cUOSocket *socket, cUORxTarget *target )
	{
		int curtim=uiCurrentTime;
		P_ITEM pi = FindItemBySerial(target->serial());
		P_PLAYER pc_currchar = socket->player();
		
		if( !pi || !pi->corpse() )
		{
			socket->sysMessage( tr( "That does not appear to be a corpse." ) );
			return true;
		}
		
		cCorpse* corpse = dynamic_cast<cCorpse*>(pi);
		if ( !corpse )
			return true;

		if(pc_currchar->isGM())
		{
			socket->sysMessage( tr("The %1 is %2 seconds old and the killer was %3.").arg(corpse->name()).arg((curtim-corpse->murdertime())/MY_CLOCKS_PER_SEC).arg( corpse->murderer() ) );
		}
		else
		{
			if (!pc_currchar->checkSkill( FORENSICS, 0, 500)) 
				socket->sysMessage( tr("You are not certain about the corpse.")); 
			else
			{
				if(((curtim-corpse->murdertime())/MY_CLOCKS_PER_SEC)>180) socket->sysMessage( tr("The %1 is many many seconds old.").arg(corpse->name()));
				else if(((curtim-corpse->murdertime())/MY_CLOCKS_PER_SEC)>60) socket->sysMessage( tr("The %1 is many seconds old.").arg(corpse->name()) );
				else if(((curtim-corpse->murdertime())/MY_CLOCKS_PER_SEC)<=60) socket->sysMessage( tr("The %1 is few seconds old.").arg(corpse->name()) );
								
				if ( !pc_currchar->checkSkill( FORENSICS, 500, 1000, false ) || corpse->murderer().isNull() ) 
					socket->sysMessage( tr("You can't say who was the killer.") ); 
				else
				{
					socket->sysMessage( tr("The killer was %1.").arg( corpse->murderer() ) );
				}
			}
		}
		return true;
	}
};


// Poisoning
class cSkPoisoning: public cTargetRequest
{
	
	bool poisonSelected;
	P_ITEM pPoison;
public:
	cSkPoisoning() : poisonSelected(false), pPoison(0) {}

	bool selectPoison( cUOSocket* socket, cUORxTarget* target )
	{
		pPoison = FindItemBySerial( target->serial() );
		if ( !pPoison || pPoison->type() != 19 || pPoison->type() != 6 )
		{
			socket->sysMessage( tr("That is not a valid poison") );
			return true;
		}
		poisonSelected = true;
		return false; // Resend the target request
	}

	bool poisonItem( cUOSocket* socket, cUORxTarget* target )
	{
		return true;
	}

	virtual bool responsed( cUOSocket *socket, cUORxTarget *target )
	{
		if ( poisonSelected )
			return poisonItem( socket, target );
		else
			return selectPoison( socket, target );
	}
};

class cSkRepairItem : public cTargetRequest
{
	
private:
	cDoCodeAction* makesection_;
public:
	cSkRepairItem( cDoCodeAction* makesection = NULL ) : cTargetRequest(), makesection_( makesection ) {}
	virtual bool responsed( cUOSocket *socket, cUORxTarget *target );
};

class cResurectTarget: public cTargetRequest
{
	
public:
	virtual bool responsed( cUOSocket *socket, cUORxTarget *target )
	{
		P_CHAR pChar = FindCharBySerial( target->serial() );

		if( !pChar )
		{
			socket->sysMessage( tr( "This is not a living being." ) );
			return true;
		}

		pChar->resurrect();
		return true;
	}
};


class cKillTarget: public cTargetRequest
{
	
public:
	virtual bool responsed( cUOSocket *socket, cUORxTarget *target )
	{
		if( !socket->player() )
			return true;

		P_CHAR pChar = FindCharBySerial( target->serial() );

		if( !pChar )
		{
			socket->sysMessage( tr( "You need to target a living being" ) );
			return true;
		}

		pChar->damage( DAMAGE_GODLY, pChar->hitpoints(), socket->player() );
		return true;
	}
};

class cSetTarget: public cTargetRequest
{
	
	QString key,value;
public:
	cSetTarget( const QString& nKey, const QString& nValue ) : key(nKey), value(nValue) {}
	bool responsed( cUOSocket *socket, cUORxTarget *target );
};

class cRemoveTarget: public cTargetRequest
{
	
public:
	bool responsed( cUOSocket *socket, cUORxTarget *target );
};

class cTeleTarget: public cTargetRequest
{
	
public:
	virtual bool responsed( cUOSocket *socket, cUORxTarget *target )
	{
		// This is a GM command so we do not check anything but send the 
		// char where he wants to move
		if( !socket->player() )
			return true;
	
		socket->player()->removeFromView( false );

		Coord_cl newPos = socket->player()->pos();
		newPos.x = target->x();
		newPos.y = target->y();
		newPos.z = target->z();
		socket->player()->moveTo( newPos );

		socket->player()->resend( false );
		socket->resendWorld();
		return true;
	}
};

class cShowTarget: public cTargetRequest
{
	
private:
	QString key;
public:
	cShowTarget( const QString _key ) { key = _key; }
	bool responsed( cUOSocket *socket, cUORxTarget *target );
};

class cSetTagTarget: public cTargetRequest
{
	
private:
	UINT8 type_;
	QString key_;
	QString value_;
public:
	cSetTagTarget( QString key, QString value, UINT8 type ) 
	{ 
		type_	= type; 
		key_	= key;
		value_	= value;
	}

	virtual bool responsed( cUOSocket *socket, cUORxTarget *target )
	{
		Q_UNUSED(socket);
		if( isCharSerial( target->serial() ) )
		{
			P_CHAR pChar = FindCharBySerial( target->serial() );
			if( pChar )
			{
				if( type_ )
					pChar->setTag( key_, cVariant( value_.toInt() ) );
				else
					pChar->setTag( key_, cVariant( value_ ) );
			}
			return true;
		}
		else if( isItemSerial( target->serial() ) )
		{
			P_ITEM pItem = FindItemBySerial( target->serial() );
			if( pItem )
			{
				if( type_ )
					pItem->setTag( key_, cVariant( value_.toInt() ) );
				else
					pItem->setTag( key_, cVariant( value_ ) );
			}
			return true;
		}
		return false;
	}
};

class cGetTagTarget: public cTargetRequest
{
	
private:
	QString key_;
public:
	cGetTagTarget( QString key ) 
	{ 
		key_	= key;
	}

	virtual bool responsed( cUOSocket *socket, cUORxTarget *target )
	{
		if( isCharSerial( target->serial() ) )
		{
			P_CHAR pChar = FindCharBySerial( target->serial() );
			if( pChar )
			{
				socket->sysMessage( tr("Tag \"%1\" has value \"%2\".").arg( key_ ).arg( pChar->getTag( key_ ).toString() ) );
			}
			return true;
		}
		else if( isItemSerial( target->serial() ) )
		{
			P_ITEM pItem = FindItemBySerial( target->serial() );
			if( pItem )
			{
				socket->sysMessage( tr("Tag \"%1\" has value \"%2\".").arg( key_ ).arg( pItem->getTag( key_ ).toString() ) );
			}
			return true;
		}
		return false;
	}
};

class cRemoveTagTarget: public cTargetRequest
{
	
private:
	QString key_;
public:
	cRemoveTagTarget( QString key ) 
	{ 
		key_	= key;
	}

	virtual bool responsed( cUOSocket *socket, cUORxTarget *target )
	{
		if( isCharSerial( target->serial() ) )
		{
			P_CHAR pChar = FindCharBySerial( target->serial() );
			if( pChar )
			{
				if( key_.lower() == "all" )
				{
					QStringList keys = pChar->getTags();
					QStringList::const_iterator it = keys.begin();
					while( it != keys.end() )
					{
						pChar->removeTag( (*it) );
						it++;
					}
					socket->sysMessage( tr("All tags removed.") );
				}
				else
				{
					pChar->removeTag( key_ );
					socket->sysMessage( tr("Tag \"%1\" removed.").arg( key_ ) );
				}
			}
			return true;
		}
		else if( isItemSerial( target->serial() ) )
		{
			P_ITEM pItem = FindItemBySerial( target->serial() );
			if( pItem )
			{
				if( key_.lower() == "all" )
				{
					QStringList keys = pItem->getTags();
					QStringList::const_iterator it = keys.begin();
					while( it != keys.end() )
					{
						pItem->removeTag( (*it) );
						it++;
					}
					socket->sysMessage( tr("All tags removed.") );
				}
				else
				{
					pItem->removeTag( key_ );
					socket->sysMessage( tr("Tag \"%1\" removed.").arg( key_ ) );
				}
			}
			return true;
		}
		return false;
	}
};

class cTagsInfoTarget: public cTargetRequest
{
	
public:
	cTagsInfoTarget() {}

	virtual bool responsed( cUOSocket *socket, cUORxTarget *target )
	{
		if( isCharSerial( target->serial() ) )
		{
			P_CHAR pChar = FindCharBySerial( target->serial() );
			if( pChar )
			{
				cTagsInfoGump* pGump = new cTagsInfoGump( pChar );
				socket->send( pGump );
			}
			return true;
		}
		else if( isItemSerial( target->serial() ) )
		{
			P_ITEM pItem = FindItemBySerial( target->serial() );
			if( pItem )
			{
				cTagsInfoGump* pGump = new cTagsInfoGump( pItem );
				socket->send( pGump );
			}
			return true;
		}
		return false;
	}
};

class cAddEventTarget: public cTargetRequest
{
	
private:
	QString _event;
public:
	cAddEventTarget( const QString &event ): _event( event ) {}
	
	bool responsed( cUOSocket *socket, cUORxTarget *target );
};

class cRemoveEventTarget: public cTargetRequest
{
	
private:
	QString _event;
public:
	cRemoveEventTarget( const QString &event ): _event( event ) {}
	
	virtual bool responsed( cUOSocket *socket, cUORxTarget *target )
	{
		cUObject *pObject = 0;
		
		if( isCharSerial( target->serial() ) )
			pObject = FindCharBySerial( target->serial() );
		else if( isItemSerial( target->serial() ) )
			pObject = FindItemBySerial( target->serial() );

		// We have to have a valid target
		if( !pObject )
		{
			socket->sysMessage( tr( "You have to target a character or an item." ) );
			return true;
		}

		// Check if we already have the event
		if( !pObject->hasEvent( _event ) )
		{
			socket->sysMessage( tr( "This object doesn't have the event '%1'" ).arg( _event ) );
			return true;
		}

		pObject->removeEvent( _event );
		return true;
	}
};

class cMoveTarget: public cTargetRequest
{
	
private:
	INT16 x,y,z;
public:
	cMoveTarget( INT16 _x, INT16 _y, INT8 _z ): x( _x ), y( _y ), z( _z ) {}
	
	virtual bool responsed( cUOSocket *socket, cUORxTarget *target )
	{
		cUObject *pObject = 0;
		
		if( isCharSerial( target->serial() ) )
			pObject = FindCharBySerial( target->serial() );
		else if( isItemSerial( target->serial() ) )
			pObject = FindItemBySerial( target->serial() );

		// We have to have a valid target
		if( !pObject )
		{
			socket->sysMessage( tr( "You have to target a character or an item." ) );
			return true;
		}

		// Move the object relatively
		pObject->removeFromView();
		Coord_cl newPos = pObject->pos() + Coord_cl( x, y, z );
		pObject->moveTo( newPos );
		
		if( pObject->isChar() )
		{
			P_CHAR pChar = dynamic_cast< P_CHAR >( pObject );
			if( pChar )
				pChar->resend();
		}
		else if( pObject->isItem() )
		{
			P_ITEM pItem = dynamic_cast< P_ITEM >( pObject );
			if( pItem )
				pItem->update();
		}

		return true;
	}
};

class cTileTarget: public cTargetRequest
{
	
private:
	INT16 x1, y1;
	INT8 z;
	QStringList ids;
public:
	cTileTarget( INT8 _z, const QStringList &_ids ): x1( -1 ), y1( -1 ), z( _z ), ids( _ids ) {}

	bool responsed( cUOSocket *socket, cUORxTarget *target );
};

class cRestockTarget: public cTargetRequest
{
	
public:
	virtual bool responsed( cUOSocket *socket, cUORxTarget *target )
	{
		P_CHAR pChar = FindCharBySerial( target->serial() );

		if( pChar )
		{
//			pChar->restock();
			socket->sysMessage( tr( "This vendor's inventar has been restocked." ) );
		}
		else
		{
			socket->sysMessage( tr( "Please target a valid vendor." ) );
		}

		return true;
	}
};

/*!
	Class for handling a dye request (for dying dye-tubs).
*/
class cDyeTubDyeTarget: public cTargetRequest
{
	
	UINT16 _color;
public:
	cDyeTubDyeTarget( UINT16 color ) : _color(color) {}
	bool responsed( cUOSocket *socket, cUORxTarget *target );
};

class cStableTarget: public cTargetRequest
{
	
private:
	P_NPC m_npc;
public:
	cStableTarget( P_NPC npc ) : m_npc( npc ) {}

	virtual bool responsed( cUOSocket *socket, cUORxTarget *target )
	{
		if( m_npc )
		{
			Human_Stablemaster* ai = dynamic_cast< Human_Stablemaster* >(m_npc->ai());
			ai->handleTargetInput( socket->player(), target );
		}
		return true;
	}
};

class cFollowTarget : public cTargetRequest
{
	
private:
	P_NPC m_npc;
public:
	cFollowTarget( P_NPC npc ) : m_npc( npc ) {}

	virtual bool responsed( cUOSocket* socket, cUORxTarget* target )
	{
		if( m_npc )
		{
			P_CHAR pTarget = World::instance()->findChar( target->serial() );
			if( pTarget )
			{
				m_npc->setWanderFollowTarget( pTarget );
				m_npc->setWanderType( enFollowTarget );
			}
		}
		return true;
	}
};


#endif

