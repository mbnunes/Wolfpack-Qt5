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

#include "wolfpack.h"
#include "wptargetrequests.h"
#include "items.h"

class cAddItemTarget: public cTargetRequest
{
	Q_OBJECT
	QString item_;
	bool nodecay;
public:
	cAddItemTarget( const QString &item, bool _nodecay = false ) : nodecay( _nodecay ), item_(item) {}
	bool responsed( cUOSocket *socket, cUORxTarget *target );
};

class cAddNpcTarget: public cTargetRequest
{
	Q_OBJECT
	QString npc_;
public:
	cAddNpcTarget( const QString &npc ) : npc_(npc) {}
	bool responsed( cUOSocket *socket, cUORxTarget *target );
};

class cBuildMultiTarget: public cTargetRequest
{
	Q_OBJECT
	QString multisection_;
	SERIAL	deedserial_;
	SERIAL	senderserial_;
public:
	cBuildMultiTarget( const QString &multisection, SERIAL senderserial, SERIAL deedserial ) { multisection_ = multisection; senderserial_ = senderserial; deedserial_ = deedserial; };
	bool responsed( cUOSocket *socket, cUORxTarget *target );
};

class cSetMultiOwnerTarget: public cTargetRequest
{
	Q_OBJECT
	SERIAL multi_;
	bool coowner_;
public:
	cSetMultiOwnerTarget( SERIAL multiserial, bool coowner = false ) : multi_(multiserial), coowner_(coowner) {}

	bool responsed( cUOSocket *socket, cUORxTarget *target );
};

class cMultiAddToListTarget: public cTargetRequest
{
	Q_OBJECT
	SERIAL	multi_;
	bool	banlist_;
public:
	cMultiAddToListTarget( SERIAL multiserial, bool banlist = false ) : multi_(multiserial), banlist_(banlist) {}

	bool responsed( cUOSocket *socket, cUORxTarget *target );
};

class cMultiChangeLockTarget: public cTargetRequest
{
	Q_OBJECT
private:
	SERIAL multi_;
public:
	cMultiChangeLockTarget( SERIAL multiserial ) { multi_ = multiserial; }

	bool responsed( cUOSocket *socket, cUORxTarget *target );
};

class cSkAnatomy: public cTargetRequest
{
	Q_OBJECT
public:
	bool responsed( cUOSocket *socket, cUORxTarget *target );
};

// This is the target-request for ArmsLore
class cSkArmsLore: public cTargetRequest
{
	Q_OBJECT
public:
	bool responsed( cUOSocket *socket, cUORxTarget *target );
};

// Detecting hidden
class cSkDetectHidden: public cTargetRequest
{
	Q_OBJECT
public:
	virtual bool responsed( cUOSocket *socket, cUORxTarget *target );
};

// Item Identification
class cSkItemID: public cTargetRequest
{
	Q_OBJECT
public:
	virtual bool cSkItemID::responsed( cUOSocket *socket, cUORxTarget *target );
};


// Evaluate Inteligence
class cSkIntEval: public cTargetRequest
{
	Q_OBJECT
public:
	bool responsed( cUOSocket *socket, cUORxTarget *target );
};


// Tame
class cSkTame: public cTargetRequest
{
	Q_OBJECT
public:
	bool responsed( cUOSocket *socket, cUORxTarget *target );
};


// Begging
class cSkBegging: public cTargetRequest
{
	Q_OBJECT
public:
	bool responsed( cUOSocket *socket, cUORxTarget *target );
};


// Animal Lore
class cSkAnimalLore: public cTargetRequest
{
	Q_OBJECT
public:
	virtual bool responsed( cUOSocket *socket, cUORxTarget *target )
	{
		P_CHAR pc = FindCharBySerial( target->serial() );
		if( !pc ) return true;
		P_CHAR pc_currchar = socket->player();
		
		// blackwind distance fix 
		if( pc->dist(pc_currchar ) >= 10 ) 
		{ 
			socket->sysMessage( tr("You need to be closer to find out more about them" ) ); 
			return true; 
		} 
		
		if (pc->isGMorCounselor())
		{
			socket->sysMessage( tr("Little is known of these robed gods.") );
			return true;
		}
		if (pc->isHuman()) // Used on human
		{
			socket->sysMessage( tr("The human race should use conversation!") );
			return true;
		}
		else // Lore used on a non-human
		{
			UI16 skill = pc_currchar->skill( ANIMALLORE );
			if( pc->tamed() || ( skill >= 1000 && skill <= 1100 && (pc->taming()<=1200||pc->taming()>0) ) ||
				skill >= 1100 )
			{
				if( pc_currchar->checkSkill( ANIMALLORE, 0, 1000 ) )
				{
					QString message = tr("Attack[%1-%2] Defense [%3] Hit Points [%5]. ").arg(pc->lodamage()).arg(pc->hidamage()).arg(pc->def()).arg(pc->hp());
					message += tr("It accepts");
					bool anyfood = false;
					register int bit;
					for( bit = 0; bit < enNumberOfFood; ++bit )
						if( pc->food() & ( 1 << bit ) )
						{
							anyfood = true;
							message += QString( " %1," ).arg( foodname[ bit+1 ] );
						}
					
					if( anyfood )
						message = message.left( message.length()-1 ); // cut the last ","
					else
						message += tr(" nothing");
					message += tr(" for food." );
					socket->showSpeech( pc, message );

					if( pc->owner() == pc_currchar )
						socket->showSpeech( pc, tr("It is loyal to you!") );
					else
					{						
						QString ownername = "nobody";
						if( pc->owner() )
							ownername = pc->owner()->name();
						socket->showSpeech( pc, tr("It is loyal to %1!").arg( ownername ) );
					}
				}
				return true;
			}
			socket->sysMessage( tr("You can not think of anything relevant at this time.") );
		}
		return true;
	}
};

// Stealing
class cSkStealing: public cTargetRequest
{
	Q_OBJECT
public:
	bool cSkStealing::responsed( cUOSocket *socket, cUORxTarget *target );
};


// Forensics Evaluation
class cSkForensics: public cTargetRequest
{
	Q_OBJECT
public:
	virtual bool responsed( cUOSocket *socket, cUORxTarget *target )
	{
		int curtim=uiCurrentTime;
		const PC_ITEM pi = FindItemBySerial(target->serial());
		P_CHAR pc_currchar = socket->player();
		
		if( !pi || !pi->corpse() )
		{
			socket->sysMessage( tr( "That does not appear to be a corpse." ) );
			return true;
		}
		
		if(pc_currchar->isGM())
		{
			socket->sysMessage( tr("The %1 is %2 seconds old and the killer was %3.").arg(pi->name()).arg((curtim-pi->murdertime())/MY_CLOCKS_PER_SEC).arg( pi->murderer() ) );
		}
		else
		{
			if (!pc_currchar->checkSkill( FORENSICS, 0, 500)) 
				socket->sysMessage( tr("You are not certain about the corpse.")); 
			else
			{
				if(((curtim-pi->murdertime())/MY_CLOCKS_PER_SEC)>180) socket->sysMessage( tr("The %1 is many many seconds old.").arg(pi->name()));
				else if(((curtim-pi->murdertime())/MY_CLOCKS_PER_SEC)>60) socket->sysMessage( tr("The %1 is many seconds old.").arg(pi->name()) );
				else if(((curtim-pi->murdertime())/MY_CLOCKS_PER_SEC)<=60) socket->sysMessage( tr("The %1 is few seconds old.").arg(pi->name()) );
								
				if ( !pc_currchar->checkSkill( FORENSICS, 500, 1000, false ) || pi->murderer() == "" ) 
					socket->sysMessage( tr("You can't say who was the killer.") ); 
				else
				{
					socket->sysMessage( tr("The killer was %1.").arg( pi->murderer() ) );
				}
			}
		}
		return true;
	}
};


// Poisoning
class cSkPoisoning: public cTargetRequest
{
	Q_OBJECT
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
		
		P_ITEM pi = FindItemBySerial( target->serial() );
		if ( !pi )
			return true;
		
		P_CHAR pc = socket->player();
		int success=0;
		switch(pPoison->morez())
		{
		case 1:	success=pc->checkSkill( POISONING, 0, 500);		break;//lesser poison
		case 2:	success=pc->checkSkill( POISONING, 151, 651);		break;//poison
		case 3:	success=pc->checkSkill( POISONING, 551, 1051);		break;//greater poison
		case 4:	success=pc->checkSkill( POISONING, 901, 1401);		break;//deadly poison
		default:
			LogError("cSkPoisoning::poisonItem(..): switch reached default\n");
			return true;
		}

		pc->soundEffect( 0x0247 ); //poisoning effect

		if(success)
		{
			
			if(pi->poisoned()<pPoison->morez()) pi->setPoisoned( pPoison->morez() );
			socket->sysMessage( tr("You successfully poison that item.") );
		} 
		else
			socket->sysMessage( tr("You fail to apply the poison.") );
		
		//empty bottle after poisoning
		Items->DeleItem( pPoison );
		pPoison = new cItem;
		pPoison->Init(true);
		pPoison->setId(0x0F0E);
		pPoison->moveTo(pc->pos());
		pPoison->startDecay();
		pPoison->update();
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


// Taste Identification
class cSkTasteID: public cTargetRequest
{
	Q_OBJECT
public:
	virtual bool responsed( cUOSocket *socket, cUORxTarget *target )
	{
		const P_ITEM pi = FindItemBySerial( target->serial() );
		P_CHAR pc_currchar = socket->player();
		if ( pi && !pi->isLockedDown() ) // Ripper
		{
			if( !( pi->type() == 19 || pi->type() == 14) )
			{
				socket->sysMessage( tr("You cant taste that!") );
				return true;
			}
			if (!pc_currchar->checkSkill( TASTEID, 0, 250))
			{
				socket->sysMessage( tr("You can't quite tell what this item is...") );
			}
			else
			{
				if( pi->corpse() )
				{
					socket->sysMessage( tr("You have to use your forensics evalutation skill to know more on this corpse.") );
					return true;
				}
				
				// Identify Item by Antichrist // Changed by MagiusCHE)
				if (pc_currchar->checkSkill( TASTEID, 250, 500, false ))
					if (pi->name2() != "#")
						pi->setName( pi->name2() ); // Item identified! -- by Magius(CHE)
					
				socket->sysMessage( tr("You found that this item appears to be called: %1").arg(pi->name())  );
					
				if (pc_currchar->checkSkill( TASTEID, 250, 500, false ))
				{
					if((pi->poisoned()>0) || (pi->morex()==4 && pi->morey()==6 && pi->morez()==1))
						socket->sysMessage( tr("This item is poisoned!") );
					else
						socket->sysMessage( tr("This item shows no poison.") );
						
					// Show Creator by Magius(CHE)
					if (pc_currchar->checkSkill( TASTEID, 250, 500, false ))
					{
						if (pi->creator().length()>0)
						{
							if (pi->madewith()>0) 
								socket->sysMessage( tr("It is %1 by %2").arg(skill[pi->madewith()-1].madeword).arg(pi->creator()) ); // Magius(CHE)
							else if (pi->madewith()<0) 
								socket->sysMessage( tr("It is %1 by %2").arg(skill[0-pi->madewith()-1].madeword).arg(pi->creator()) ); // Magius(CHE)
							else 
								socket->sysMessage( tr("It is made by %1").arg(pi->creator()) ); // Magius(CHE)
						} else 
							socket->sysMessage( tr("You don't know its creator!") );
					} else 
						socket->sysMessage( tr("You can't know its creator!") );
				}
			}
		}
		return true;
	}
};


// Provocation
class cSkProvocation: public cTargetRequest
{
	Q_OBJECT
	bool firstTarget;
	P_CHAR attacker;
public:
	
	cSkProvocation() : firstTarget(false) {}
	
	bool acquireFirst( cUOSocket* socket, cUORxTarget* target );

	bool cSkProvocation::selectVictim( cUOSocket* socket, cUORxTarget* target );

	virtual bool responsed( cUOSocket *socket, cUORxTarget *target )
	{
		if ( !firstTarget )
			return acquireFirst( socket, target );
		else
			return selectVictim( socket, target);
	}
	
};

class cSkRepairItem : public cTargetRequest
{
	Q_OBJECT
private:
	cMakeSection* makesection_;
public:
	cSkRepairItem( cMakeSection* makesection = NULL ) : cTargetRequest(), makesection_( makesection ) {}
	virtual bool responsed( cUOSocket *socket, cUORxTarget *target );
};

class cResurectTarget: public cTargetRequest
{
	Q_OBJECT
public:
	virtual bool responsed( cUOSocket *socket, cUORxTarget *target )
	{
		P_CHAR pChar = FindCharBySerial( target->serial() );

		if( !pChar )
		{
			socket->sysMessage( tr( "This is not a living being." ) );
			return true;
		}

		Targ->NpcResurrectTarget( pChar );
		return true;
	}
};


class cKillTarget: public cTargetRequest
{
	Q_OBJECT
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

		pChar->kill();
		return true;
	}
};

class cSetTarget: public cTargetRequest
{
	Q_OBJECT
	QString key,value;
public:
	cSetTarget( const QString& nKey, const QString& nValue ) : key(nKey), value(nValue) {}
	bool responsed( cUOSocket *socket, cUORxTarget *target );
};

class cRemoveTarget: public cTargetRequest
{
	Q_OBJECT
public:
	bool responsed( cUOSocket *socket, cUORxTarget *target );
};

class cTeleTarget: public cTargetRequest
{
	Q_OBJECT
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

class cInfoTarget: public cTargetRequest
{
	Q_OBJECT
public:
	bool responsed( cUOSocket *socket, cUORxTarget *target );
};

class cShowTarget: public cTargetRequest
{
	Q_OBJECT
private:
	QString key;
public:
	cShowTarget( const QString _key ) { key = _key; }
	bool responsed( cUOSocket *socket, cUORxTarget *target );
};

class cBankTarget: public cTargetRequest
{
	Q_OBJECT
private:
	UINT8 layer;
public:
	cBankTarget( UINT8 data ) { layer = data; }

	virtual bool responsed( cUOSocket *socket, cUORxTarget *target )
	{
		P_CHAR pChar = FindCharBySerial( target->serial() );
		if( !pChar )
		{
			socket->sysMessage( tr( "This does not appear to be a living being." ) );
			return true;
		}

		P_ITEM pItem = pChar->GetItemOnLayer( layer );

		if( !pItem )
		{
			socket->sysMessage( tr( "This being does not have a container on layer 0x%1" ).arg( layer, 2, 16 ) );
			return true;
		}

		socket->sendContainer( pItem );
		return true;
	}
};

class cSetTagTarget: public cTargetRequest
{
	Q_OBJECT
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
		if( isCharSerial( target->serial() ) )
		{
			P_CHAR pChar = FindCharBySerial( target->serial() );
			if( pChar )
			{
				if( type_ )
					pChar->tags().set( key_, cVariant( value_.toInt() ) );
				else
					pChar->tags().set( key_, cVariant( value_ ) );
			}
			return true;
		}
		else if( isItemSerial( target->serial() ) )
		{
			P_ITEM pItem = FindItemBySerial( target->serial() );
			if( pItem )
			{
				if( type_ )
					pItem->tags().set( key_, cVariant( value_.toInt() ) );
				else
					pItem->tags().set( key_, cVariant( value_ ) );
			}
			return true;
		}
		return false;
	}
};

class cGetTagTarget: public cTargetRequest
{
	Q_OBJECT
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
				socket->sysMessage( tr("Tag \"%1\" has value \"%2\".").arg( key_ ).arg( pChar->tags().get( key_ ).asString() ) );
			}
			return true;
		}
		else if( isItemSerial( target->serial() ) )
		{
			P_ITEM pItem = FindItemBySerial( target->serial() );
			if( pItem )
			{
				socket->sysMessage( tr("Tag \"%1\" has value \"%2\".").arg( key_ ).arg( pItem->tags().get( key_ ).asString() ) );
			}
			return true;
		}
		return false;
	}
};

class cRemoveTagTarget: public cTargetRequest
{
	Q_OBJECT
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
					QStringList keys = pChar->tags().getKeys();
					QStringList::const_iterator it = keys.begin();
					while( it != keys.end() )
					{
						pChar->tags().remove( (*it) );
						it++;
					}
					socket->sysMessage( tr("All tags removed.") );
				}
				else
				{
					pChar->tags().remove( key_ );
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
					QStringList keys = pItem->tags().getKeys();
					QStringList::const_iterator it = keys.begin();
					while( it != keys.end() )
					{
						pItem->tags().remove( (*it) );
						it++;
					}
					socket->sysMessage( tr("All tags removed.") );
				}
				else
				{
					pItem->tags().remove( key_ );
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
	Q_OBJECT
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

class cModifySpellbook: public cTargetRequest
{
	Q_OBJECT
private:
	UINT8 spell;
	bool deleteMode;
public:
	cModifySpellbook( UINT8 _spell, bool _deleteMode = false ) : spell(_spell), deleteMode(_deleteMode) {}

	bool responsed( cUOSocket *socket, cUORxTarget *target );
	virtual void canceled( cUOSocket *socket ) { Q_UNUSED(socket); }
};

class cAddEventTarget: public cTargetRequest
{
	Q_OBJECT
private:
	QString _event;
public:
	cAddEventTarget( const QString &event ): _event( event ) {}
	
	bool responsed( cUOSocket *socket, cUORxTarget *target );
};

class cRemoveEventTarget: public cTargetRequest
{
	Q_OBJECT
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
	Q_OBJECT
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

class cNukeTarget: public cTargetRequest
{
	Q_OBJECT
private:
	INT16 x1, y1;
public:
	cNukeTarget( INT16 _x1 = -1, INT16 _y1 = -1 ): x1( _x1 ), y1( _y1 ) {}

	virtual bool responsed( cUOSocket *socket, cUORxTarget *target )
	{
		// Set the first corner and readd ourself
		if( x1 == -1 || y1 == -1 )
		{
			x1 = target->x();
			y1 = target->y();
			socket->sysMessage( tr( "Please select the second corner of the box you want to nuke." ) );
			return false;
		}
		else
		{
			INT16 x2;
			if( target->x() < x1 )
			{
				x2 = x1;
				x1 = target->x();
			}
			else
				x2 = target->x();

			INT16 y2;
			if( target->y() < y1 )
			{
				y2 = y1;
				y1 = target->y();
			}
			else
				y2 = target->y();

			socket->sysMessage( tr( "Nuking from %1,%2 to %3,%4" ).arg( x1 ).arg( y1 ).arg( x2 ).arg( y2 ) );
			UINT16 dCount = 0;
			vector< P_ITEM > toDelete;

			// This could eventually be optimized
			AllItemsIterator iter;
			for( iter.Begin(); !iter.atEnd(); ++iter )
			{
				P_ITEM pItem = iter.GetData();

				if( pItem && pItem->isInWorld() && pItem->pos().x >= x1 && pItem->pos().x <= x2 && pItem->pos().y >= y1 && pItem->pos().y <= y2 )
				{
					// Delete the item
					toDelete.push_back( pItem );
					dCount++;
				}
			}

			for( vector< P_ITEM >::iterator sIter = toDelete.begin(); sIter != toDelete.end(); ++sIter )
				Items->DeleItem( (*sIter) );

			socket->sysMessage( tr( "Deleted %1 items." ).arg( dCount ) );
			return true;
		}		
	}
};

class cTileTarget: public cTargetRequest
{
	Q_OBJECT
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
	Q_OBJECT
public:
	virtual bool responsed( cUOSocket *socket, cUORxTarget *target )
	{
		P_CHAR pChar = FindCharBySerial( target->serial() );

		if( pChar )
		{
			pChar->restock();
			socket->sysMessage( tr( "This vendor's inventar has been restocked." ) );
		}
		else
		{
			socket->sysMessage( tr( "Please target a valid vendor." ) );
		}

		return true;
	}
};

class cSpellTarget : public cTargetRequest
{
	Q_OBJECT
	UINT8 spell;
	UINT8 type;
public:
	cSpellTarget( P_CHAR pMage, UINT8 _spell, UINT8 _type );
	virtual bool responsed( cUOSocket *socket, cUORxTarget *target );
	virtual void timedout( cUOSocket *socket );
	virtual void canceled( cUOSocket *socket );
};

/*!
	Class for handling a dye request (for dying dye-tubs).
*/
class cDyeTubDyeTarget: public cTargetRequest
{
	Q_OBJECT
	UINT16 _color;
public:
	cDyeTubDyeTarget( UINT16 color ) : _color(color) {}
	bool responsed( cUOSocket *socket, cUORxTarget *target );
};


#endif

