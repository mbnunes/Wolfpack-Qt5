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

//////////////////////////////////////////////////////////////////////
// TmpEff.cpp: implementation of temporary effects
//				cut from Wolfpack.cpp by Duke, 25.10.2000

#include "platform.h"

// Wolfpack Includes
#include "TmpEff.h"
#include "items.h"
#include "globals.h"
#include "debug.h"
#include "srvparams.h"
#include "wolfpack.h"
#include "iserialization.h"
#include "network.h"
#include "wpdefmanager.h"
#include "network/uosocket.h"
#include "skills.h"
#include "mapobjects.h"
#include "magic.h"
#include "basechar.h"
#include "player.h"
#include "npc.h"
#include "ai.h"

#include <algorithm>
#include <typeinfo>
#include <math.h>

#undef  DBGFILE
#define DBGFILE "tmpeff.cpp"

int cTempEffect::getDest()
{
	return destSer;
}

void cTempEffect::setDest(int ser)
{
	destSer=ser;
}

int cTempEffect::getSour()
{
	return sourSer;
}

void cTempEffect::setSour(int ser)
{
	sourSer=ser;
}

void cTempEffect::setExpiretime_s(int seconds)
{
	expiretime=uiCurrentTime+(seconds*MY_CLOCKS_PER_SEC);
}

void cTempEffect::setExpiretime_ms(float milliseconds)
{
	expiretime=uiCurrentTime+(int)floor(( milliseconds / 1000 )*MY_CLOCKS_PER_SEC);
}

void cTempEffect::Serialize(ISerialization &archive)
{
	if (archive.isReading())
	{
		archive.read( "expiretime",		this->expiretime ); // exptime must be subtracted from current server clock time, so it can be recalculated on next server startup
		archive.read( "dispellable",	this->dispellable );
		archive.read( "srcserial",		this->sourSer );
		archive.read( "destserial",		this->destSer );
		this->expiretime += uiCurrentTime;
	}
	else if( archive.isWritting() )
	{
		archive.write( "expiretime",	( this->expiretime - uiCurrentTime ) ); // exptime must be subtracted from current server clock time, so it can be recalculated on next server startup
		archive.write( "dispellable",	this->dispellable );
		archive.write( "srcserial",		this->getSour() );
		archive.write( "destserial",	this->getDest() );
	}
	cSerializable::Serialize( archive );
}

void cTempEffects::check()
{
	cTempEffect *tEffect = NULL;
	if( !teffects.empty() )
		tEffect = *teffects.begin();

	if( !tEffect)
		return;
	
	while( tEffect && tEffect->expiretime <= uiCurrentTime )
	{
		if( isCharSerial( tEffect->getDest() ) )
		{
			P_CHAR pChar = dynamic_cast< P_CHAR >( FindCharBySerial( tEffect->getDest() ) );
			if( pChar )
				pChar->removeEffect( tEffect );
		}

		tEffect->Expire();
		std::pop_heap( teffects.begin(), teffects.end(), cTempEffects::ComparePredicate() );
		teffects.pop_back();
		delete tEffect;

		if( !teffects.empty() )
			tEffect = *teffects.begin();
		else
			break;
	}

}

void cTempEffects::serialize(ISerialization &archive)
{
	std::vector< cTempEffect* >::iterator it = teffects.begin();
	while( it != teffects.end() )
	{
		if( (*it)->isSerializable() )
			archive.writeObject( (*it) );
		++it;
	}
}

/*!
	Dispels all TempEffects lasting on pc_dest of a given type.
	If only Dispellable is false then all effects on this character
	of the specified type are reverted.
*/
void cTempEffects::dispel( P_CHAR pc_dest, P_CHAR pSource, const QString &type, bool silent, bool onlyDispellable )
{
	std::vector< cTempEffect* >::iterator i = teffects.begin();
	for( i = teffects.begin(); i != teffects.end(); i++ )
		if( (*i) != NULL && ( !onlyDispellable || (*i)->dispellable ) && (*i)->getDest() == pc_dest->serial() && (*i)->objectID() == type )
		{
			if( isCharSerial( (*i)->getSour() ) )
			{
				P_CHAR pChar = FindCharBySerial( (*i)->getSour() );
				if( pChar )
					pChar->removeEffect( (*i) );
			}

			(*i)->Dispel( pc_dest, pSource );
			teffects.erase( i );
		}

		std::make_heap( teffects.begin(), teffects.end(), cTempEffects::ComparePredicate() );
}

void cTempEffects::dispel( P_CHAR pc_dest, P_CHAR pSource, bool silent )
{
	std::vector< cTempEffect* >::iterator i = teffects.begin();
	for( i = teffects.begin(); i != teffects.end(); i++ )
		if( (*i) != NULL && (*i)->dispellable && (*i)->getDest() == pc_dest->serial() )
		{
			if( isCharSerial( (*i)->getDest() ) )
			{
				P_CHAR pChar = FindCharBySerial( (*i)->getDest() );
				if( pChar )
					pChar->removeEffect( (*i) );
			}

			(*i)->Dispel( pSource, silent );
			teffects.erase( i );
		}

		std::make_heap( teffects.begin(), teffects.end(), cTempEffects::ComparePredicate() );
}

// cDelayedHideChar
cDelayedHideChar::cDelayedHideChar( SERIAL serial )
{
	if( !isCharSerial( serial ) || !FindCharBySerial( serial ) )
	{
		character = INVALID_SERIAL;
		return;
	}
	character = serial;
	setSerializable( true );
}

void cDelayedHideChar::Serialize( ISerialization &archive )
{
	if( archive.isReading() )
	{
		archive.read( "charserial", character );
	}
	else
	{
		archive.write( "charserial", character );
	}
	cTempEffect::Serialize( archive );
}

void cDelayedHideChar::Expire()
{
	P_PLAYER pc = dynamic_cast<P_PLAYER>(FindCharBySerial( character ));
	if( !pc || pc->socket() ) // break if the char has relogged in the meantime
		return;

	pc->setHidden( 1 );
	pc->resend( true );
}

// cTimedSpellAction
cTimedSpellAction::cTimedSpellAction( SERIAL serial, UI08 nAction )
{
	if( !isCharSerial( serial ) )
	{
		character = INVALID_SERIAL;
		return;
	}

	// Display the animation once
	P_CHAR pc = FindCharBySerial( serial );
	if( !pc )
	{
		character = INVALID_SERIAL;
		return;
	}
	pc->action( nAction );

	// Save our data
	character = serial;
	action = nAction;
	serializable = false;
	expiretime = uiCurrentTime + 750;
}

// Insert a new action if there are more than 75 ticks left
void cTimedSpellAction::Expire()
{
	if( character != INVALID_SERIAL )
		TempEffects::instance()->insert( new cTimedSpellAction( character, action ) );
}

/*
//  Fibonacci Heap implementation
//
//  18.07.2002, Joerg Stueckler (sereg)
//  email: joe.di@gmx.de
//
//  based on "Algorithms and Data Structures", T.Ottmann/P.Widmayer, 4th Edition
//  Ch. 6.1.5., page 410
cTempEffect*	cTmpEffFibHeap::accessMin() // O(1)!
{
	return head;
}

void			cTmpEffFibHeap::insert( cTempEffect* pT )	// O(1) too!
{
	cTmpEffFibHeap toMeld = cTmpEffFibHeap( pT );
	this->meld( toMeld );
}

cTempEffect*	cTmpEffFibHeap::meld( cTmpEffFibHeap &nFheap )	// O(1) !!!
{
	if( !this->head )
	{
		this->head = nFheap.head;
	}
	else if( this->head && nFheap.head )
	{
		cTempEffect* minHead = NULL;
		cTempEffect* maxHead = NULL;
		if( this->head->expiretime < nFheap.head->expiretime )
		{
			minHead = this->head;
			maxHead = nFheap.head;
		}
		else
		{
			maxHead = this->head;
			minHead = nFheap.head;
		}
		minHead->left->right = maxHead;
		cTempEffect* rightElement = maxHead->left;
		maxHead->left = minHead->left;
		minHead->left = rightElement;
		rightElement->right = minHead;

		this->head = minHead;
	}
	
	return this->head;
}

void			cTmpEffFibHeap::deleteMin()		// amortized O( lg N )
{
	if( this->head )
	{
		if( this->head->left == this->head ) // only one element in the heap
		{
			delete this->head;
			this->head = NULL;
			return;
		}
		else
		{
			if( this->head->son ) // min element has a son
			{
				// replace the head with its sons in the list
				this->head->right->left = this->head->son->left;
				this->head->son->left->right = this->head->right;
				this->head->son->left = this->head->left;
				this->head->left->right = this->head->son;

				cTempEffect* newHead = this->head->son;
				delete this->head;
				this->head = newHead;
			}
			else
			{
				// unlink the head from the list
				this->head->right->left = this->head->left;
				this->head->left->right = this->head->right;

				cTempEffect* newHead = this->head->right;
				delete this->head;
				this->head = newHead;
			}

			// now we have to iterate through the root list to find two trees with equal rank
			// we use an array to find collisions.
			cTempEffect* ranks[RANK_ARRAY_SIZE];
			memset( &ranks, 0, sizeof( ranks ) );

			cTempEffect* it = this->head;
			cTempEffect* minNode = it;
			cTempEffect* coll = NULL;

			bool collision = false;
			do
			{
				collision = false;
				it->father = NULL; // reset the fathers of the inserted sons of head (see above)
				if( it->expiretime < minNode->expiretime )
					minNode = it; // find the min node by the way :)

				if( ranks[ it->rank ] && ranks[ it->rank ] != it ) // there is already another address stored in the array!
				{
					coll = ranks[ it->rank ];
					ranks[ it->rank ] = NULL;

					if( coll->expiretime < it->expiretime )
					{
						cTempEffect* templeft = NULL;
						cTempEffect* tempright = NULL;
						if( coll->left == it )
						{
							if( coll->right != it )
							{
								templeft = it->left;
								tempright = coll->right;

								templeft->right = coll;
								tempright->left = it;
								it->left = coll;
								it->right = tempright;
								coll->left = templeft;
								coll->right = it;
							}
						}
						else if( coll->right == it )
						{
							templeft = coll->left;
							tempright = it->right;

							templeft->right = it;
							tempright->left = coll;
							it->left = templeft;
							it->right = coll;
							coll->left = it;
							coll->right = tempright;
						}
						else
						{
							// swap the min element into it position
							templeft = coll->left;
							tempright = coll->right;

							it->left->right = coll;
							it->right->left = coll;
							coll->right = it->right;
							coll->left = it->left;
						
							templeft->right = it;
							tempright->left = it;
							it->right = tempright;
							it->left = templeft;
						}
						templeft = it;
						it = coll;
						coll = templeft;
					}

					// delete max element (coll) out of the list
					// if coll was head move the head one node right
					if( coll == this->head )
						this->head = coll->right;
					coll->left->right = coll->right;
					coll->right->left = coll->left;

					// insert max into the list of sons of min
					if( it->son )
					{
						cTempEffect* rightElement = it->son->left;
						coll->left = rightElement;
						it->son->left = coll;
						rightElement->right = coll;
						coll->right = it->son;
					}
					else
					{
						it->son = coll;
						coll->left = coll;
						coll->right = coll;
					}
					coll->father = it;

					it->rank = it->rank + 1;

					it->marker = false; // reset the marker cause the node got an additional son

					it = it->left;
					collision = true;
				}
				else
				{
					ranks[ it->rank ] = it;
				}
				it = it->right;
			} while( it != this->head || collision);

			this->head = minNode;
		}
	}
}

void			cTmpEffFibHeap::decrease( cTempEffect* pT, int diffTime ) // O( 1 )
{
	if( !pT )
		return;

	pT->expiretime -= diffTime;
	if( pT->father )
	{
		// first cut pT out and insert it into the root list
		// mark the father, if the father is already marked,
		// run decrease(...) recursively with diffTime = 0
		pT->left->right = pT->right;
		pT->right->left = pT->left;

		this->insert( pT );

		if( pT->father->marker )
			this->decrease( pT->father, 0 );
		else
			pT->father->marker = true;

		pT->father = NULL;
		pT->marker = false;
	}
}

void			cTmpEffFibHeap::erase( cTempEffect *pT ) // O( lg N )
{
	this->decrease( pT, (pT->expiretime - this->head->expiretime) + 1 );
	this->deleteMin();
}

std::vector< cTempEffect* > cTempEffect::asVector()
{
	std::vector< cTempEffect* > list_ = std::vector< cTempEffect* >();
	
	cTempEffect* iterNode = this;

	do
	{
		list_.push_back( iterNode );
		if( iterNode->son )
		{
			std::vector< cTempEffect* > sons_ = iterNode->son->asVector();
			list_.insert( list_.end(), sons_.begin(), sons_.end() );
		}
		iterNode = iterNode->right;
	} while( iterNode != this );

	return list_;
}

std::vector< cTempEffect* >	cTmpEffFibHeap::asVector()
{
	if( this->head )
		return this->head->asVector();
	else
		return std::vector< cTempEffect* >();
}
*/

cRepeatAction::cRepeatAction( P_CHAR mage, UINT8 anim, UINT32 delay )
{
	_mage = mage->serial();
	_anim = anim;
	_delay = delay;
	mage->action( anim );
	expiretime = uiCurrentTime + delay;
	dispellable = false;
	serializable = false;
}

void cRepeatAction::Expire()
{
	P_CHAR pMage = FindCharBySerial( _mage );

	if( pMage )
		TempEffects::instance()->insert( new cRepeatAction( pMage, _anim, _delay ) );
}

void cTempEffects::insert( cTempEffect *pT )
{
	// If the tempeffect has a char it affects, 
	// then don't forge to add it to his effects
	if( isCharSerial( pT->getDest() ) )
	{
		P_CHAR pChar = FindCharBySerial( pT->getDest() );
		if( pChar )
			pChar->addEffect( pT );
	}

	this->teffects.push_back( pT );
	std::push_heap( teffects.begin(), teffects.end(), cTempEffects::ComparePredicate() );
}

void cTempEffects::erase( cTempEffect *pT )
{
	if( pT == (*teffects.begin()) )
	{
		std::pop_heap( teffects.begin(), teffects.end(), cTempEffects::ComparePredicate() );
		teffects.pop_back();
	}
	else
	{
		std::vector< cTempEffect* >::iterator it = std::find( teffects.begin(), teffects.end(), pT );
		if( it != teffects.end() )
		{
			teffects.erase( it );
			std::make_heap( teffects.begin(), teffects.end(), cTempEffects::ComparePredicate() );
		}
	}
}

void cDelayedHeal::Expire()
{
	P_CHAR pSource = FindCharBySerial( destSer );
	P_CHAR pTarget = FindCharBySerial( sourSer );

	if( !pSource || !pTarget )
		return;

	if( !pSource->inRange( pTarget, 5 ) )
	{
		if( pSource->objectType() == enPlayer )
		{
			P_PLAYER pp = dynamic_cast<P_PLAYER>(pSource);
			if( pp->socket() )
				pp->socket()->sysMessage( tr( "You are standing too far away to apply any bandages." ) );
		}
		return;
	}
}

cDelayedHeal::cDelayedHeal( P_CHAR pSource, P_CHAR pTarget, UINT16 _amount )
{
	// Switching them here is important because we want to 
	// keep track of our current healing targets
	destSer = pSource->serial();
	sourSer = pTarget->serial();
	amount = _amount;
	objectid = "cDelayedHeal";
	serializable = false;
	dispellable = false;
}

cAIRefreshTimer::cAIRefreshTimer( P_NPC pNPC, UINT32 time )
{
	m_npc = pNPC;
	objectid = "cAIRefreshTimer";
	serializable = false;
	dispellable = false;
	expiretime = uiCurrentTime + time * MY_CLOCKS_PER_SEC;
}

void cAIRefreshTimer::Expire()
{
	if( m_npc )
	{
		cNPC_AI* ai = m_npc->ai();
		if( ai && ai->currState() )
			ai->currState()->refresh();
	}
}
