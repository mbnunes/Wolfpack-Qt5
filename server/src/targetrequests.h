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

#if !defined(__TARGETREQUESTS_H__)
#define __TARGETREQUESTS_H__

#include "inlines.h"
#include "network/uosocket.h"
#include "network/uorxpackets.h"
#include "targetrequest.h"
#include "items.h"
#include "corpse.h"
#include "gumps.h"
#include "world.h"
#include "player.h"
#include "muls/tilecache.h"
#include "npc.h"

#include "ai/ai.h"

class cKillTarget : public cTargetRequest
{
public:
	virtual bool responsed( cUOSocket* socket, cUORxTarget* target )
	{
		if ( !socket->player() )
			return true;

		P_CHAR pChar = FindCharBySerial( target->serial() );

		// check for rank
		if ( pChar && pChar->objectType() == enPlayer )
		{
			P_PLAYER pp = dynamic_cast<P_PLAYER>( pChar );
			if ( pp->account()->rank() >= socket->player()->account()->rank() && pp != socket->player() )
			{
				socket->sysMessage( tr( "You want to suicide?" ) );
				return true;
			}
		}

		if ( !pChar )
		{
			socket->sysMessage( tr( "You need to target a living being" ) );
			return true;
		}

		pChar->kill( socket->player() );
		//pChar->damage( DAMAGE_GODLY, pChar->hitpoints(), socket->player() );
		return true;
	}
};

class cSetTarget : public cTargetRequest
{
	QString key, value;
public:
	cSetTarget( const QString& nKey, const QString& nValue ) : key( nKey ), value( nValue )
	{
	}
	bool responsed( cUOSocket* socket, cUORxTarget* target );
};

class cRemoveTarget : public cTargetRequest
{
public:
	bool responsed( cUOSocket* socket, cUORxTarget* target );
};

class cTeleTarget : public cTargetRequest
{
public:
	virtual bool responsed( cUOSocket* socket, cUORxTarget* target )
	{
		// This is a GM command so we do not check anything but send the
		// char where he wants to move
		if ( !socket->player() )
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

class cShowTarget : public cTargetRequest
{
private:
	QString key;
public:
	cShowTarget( const QString _key )
	{
		key = _key;
	}
	bool responsed( cUOSocket* socket, cUORxTarget* target );
};

class cSetTagTarget : public cTargetRequest
{
private:
	UINT8 type_;
	QString key_;
	QString value_;
public:
	cSetTagTarget( QString key, QString value, UINT8 type )
	{
		type_ = type;
		key_ = key;
		value_ = value;
	}

	virtual bool responsed( cUOSocket* socket, cUORxTarget* target )
	{
		Q_UNUSED( socket );
		if ( isCharSerial( target->serial() ) )
		{
			P_CHAR pChar = FindCharBySerial( target->serial() );

			if ( pChar )
			{
				// check for rank
				if ( pChar->objectType() == enPlayer )
				{
					P_PLAYER pp = dynamic_cast<P_PLAYER>( pChar );
					if ( pp->account()->rank() >= socket->player()->account()->rank() && pp != socket->player() )
					{
						socket->sysMessage( tr( "Better do not try that!" ) );
						return true;
					}
				}

				if ( type_ )
					pChar->setTag( key_, cVariant( value_.toInt() ) );
				else
					pChar->setTag( key_, cVariant( value_ ) );
			}
			return true;
		}
		else if ( isItemSerial( target->serial() ) )
		{
			P_ITEM pItem = FindItemBySerial( target->serial() );
			if ( pItem )
			{
				if ( type_ )
					pItem->setTag( key_, cVariant( value_.toInt() ) );
				else
					pItem->setTag( key_, cVariant( value_ ) );
			}
			return true;
		}
		return false;
	}
};

class cGetTagTarget : public cTargetRequest
{
private:
	QString key_;
public:
	cGetTagTarget( QString key )
	{
		key_ = key;
	}

	virtual bool responsed( cUOSocket* socket, cUORxTarget* target )
	{
		if ( isCharSerial( target->serial() ) )
		{
			P_CHAR pChar = FindCharBySerial( target->serial() );
			if ( pChar )
			{
				socket->sysMessage( tr( "Tag \"%1\" has value \"%2\"." ).arg( key_ ).arg( pChar->getTag( key_ ).toString() ) );
			}
			return true;
		}
		else if ( isItemSerial( target->serial() ) )
		{
			P_ITEM pItem = FindItemBySerial( target->serial() );
			if ( pItem )
			{
				socket->sysMessage( tr( "Tag \"%1\" has value \"%2\"." ).arg( key_ ).arg( pItem->getTag( key_ ).toString() ) );
			}
			return true;
		}
		return false;
	}
};

class cRemoveTagTarget : public cTargetRequest
{
private:
	QString key_;
public:
	cRemoveTagTarget( QString key )
	{
		key_ = key;
	}

	virtual bool responsed( cUOSocket* socket, cUORxTarget* target )
	{
		if ( isCharSerial( target->serial() ) )
		{
			P_CHAR pChar = FindCharBySerial( target->serial() );
			if ( pChar )
			{
				// check for rank
				if ( pChar->objectType() == enPlayer )
				{
					P_PLAYER pp = dynamic_cast<P_PLAYER>( pChar );
					if ( pp->account()->rank() >= socket->player()->account()->rank() && pp != socket->player() )
					{
						socket->sysMessage( tr( "Better do not try that!" ) );
						return true;
					}
				}

				if ( key_.lower() == "all" )
				{
					QStringList keys = pChar->getTags();
					QStringList::const_iterator it = keys.begin();
					while ( it != keys.end() )
					{
						pChar->removeTag( ( *it ) );
						it++;
					}
					socket->sysMessage( tr( "All tags removed." ) );
				}
				else
				{
					pChar->removeTag( key_ );
					socket->sysMessage( tr( "Tag \"%1\" removed." ).arg( key_ ) );
				}
			}
			return true;
		}
		else if ( isItemSerial( target->serial() ) )
		{
			P_ITEM pItem = FindItemBySerial( target->serial() );
			if ( pItem )
			{
				if ( key_.lower() == "all" )
				{
					QStringList keys = pItem->getTags();
					QStringList::const_iterator it = keys.begin();
					while ( it != keys.end() )
					{
						pItem->removeTag( ( *it ) );
						it++;
					}
					socket->sysMessage( tr( "All tags removed." ) );
				}
				else
				{
					pItem->removeTag( key_ );
					socket->sysMessage( tr( "Tag \"%1\" removed." ).arg( key_ ) );
				}
			}
			return true;
		}
		return false;
	}
};

class cMoveTarget : public cTargetRequest
{
private:
	Q_INT16 x, y, z;
public:
	cMoveTarget( Q_INT16 _x, Q_INT16 _y, INT8 _z ) : x( _x ), y( _y ), z( _z )
	{
	}

	virtual bool responsed( cUOSocket* socket, cUORxTarget* target )
	{
		cUObject* pObject = 0;

		if ( isCharSerial( target->serial() ) )
			pObject = FindCharBySerial( target->serial() );
		else if ( isItemSerial( target->serial() ) )
			pObject = FindItemBySerial( target->serial() );

		// We have to have a valid target
		if ( !pObject )
		{
			socket->sysMessage( tr( "You have to target a character or an item." ) );
			return true;
		}

		// check for rank
		P_CHAR pChar = dynamic_cast<P_CHAR>( pObject );
		if ( pChar && pChar->objectType() == enPlayer )
		{
			P_PLAYER pp = dynamic_cast<P_PLAYER>( pChar );
			if ( pp->account()->rank() >= socket->player()->account()->rank() && pp != socket->player() )
			{
				socket->sysMessage( tr( "Better do not try that!" ) );
				return true;
			}
		}

		// Move the object relatively
		Coord_cl newPos = pObject->pos() + Coord_cl( x, y, z );
		pObject->moveTo( newPos );

		if ( pObject->isChar() )
		{
			P_CHAR pChar = dynamic_cast<P_CHAR>( pObject );

			if ( pChar )
				pChar->resend();
		}
		else if ( pObject->isItem() )
		{
			P_ITEM pItem = dynamic_cast<P_ITEM>( pObject );
			if ( pItem )
				pItem->update();
		}

		return true;
	}
};

class cRestockTarget : public cTargetRequest
{
public:
	virtual bool responsed( cUOSocket* socket, cUORxTarget* target )
	{
		P_CHAR pChar = FindCharBySerial( target->serial() );

		if ( pChar )
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

class cStableTarget : public cTargetRequest
{
private:
	P_NPC m_npc;
public:
	cStableTarget( P_NPC npc ) : m_npc( npc )
	{
	}

	virtual bool responsed( cUOSocket* socket, cUORxTarget* target )
	{
		if ( m_npc )
		{
			Human_Stablemaster* ai = dynamic_cast<Human_Stablemaster*>( m_npc->ai() );
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
	cFollowTarget( P_NPC npc ) : m_npc( npc )
	{
	}

	virtual bool responsed( cUOSocket*, cUORxTarget* target )
	{
		if ( m_npc )
		{
			P_CHAR pTarget = World::instance()->findChar( target->serial() );
			if ( pTarget )
			{
				m_npc->setWanderFollowTarget( pTarget );
				m_npc->setWanderType( enFollowTarget );
			}
		}
		return true;
	}
};

#endif
