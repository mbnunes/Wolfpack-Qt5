//==================================================================================
//
//      Wolfpack Emu (WP)
//	UO Server Emulation Program
//
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

/*==============================================================================
This class builds a sample state machine for NPC AI
==============================================================================*/

#ifndef AI_H_HEADER_INCLUDED
#define AI_H_HEADER_INCLUDED

// platform includes
#include "platform.h"

// library includes
#include <qstring.h>

// wolfpack includes
#include "typedefs.h"
#include "factory.h"

class AbstractState;
class Coord_cl;
class cUORxBuy;
class cUORxSell;
class cUORxTarget;
class cUOSocket;

class cNPC_AI
{
public:
	cNPC_AI() { currentState = NULL; }
	cNPC_AI( P_NPC npc ) : m_npc( npc ) , currentState( NULL ) {}
	virtual ~cNPC_AI();
		
	virtual void eventHandler() = 0;
	virtual void updateState();
	virtual void updateState( AbstractState* newState );

	virtual QString AIType() = 0;

	AbstractState*	currState() const { return currentState; }

	P_NPC	npc() const { return m_npc; };
	void	setNPC( P_NPC npc );
protected:
	AbstractState*	currentState;
	P_NPC	m_npc;
};

class AIFactory : public Factory<cNPC_AI, QString>
{
public:
	static AIFactory* instance()
	{
		static AIFactory factory;
		return &factory;
	}
};

class StateFactory : public Factory<AbstractState, QString>
{
public:
	static StateFactory* instance()
	{
		static StateFactory factory;
		return &factory;
	}
};

class Actions
{
public:
	Actions() { waitForPathCalculation = 0; npc = NULL; }
	virtual ~Actions() {}

	P_NPC npc;

	virtual void attack();
	virtual void reattack();
	virtual void reset() {}
	virtual void flee() {}
	virtual void greet() {}
	virtual void showTargetCursor() {}
	virtual void decline() {}
	virtual void callGuards();
	virtual void wanderFreely();
	virtual void moveTo( const Coord_cl &pos );
	virtual void movePath();
	virtual void movePath( const Coord_cl &pos );
	virtual void ceaseFlee() {}

	UINT8 waitForPathCalculation;
};

class Events
{
public:
	Events() {}
	virtual ~Events() {}

	virtual void attacked() {}
	virtual void won() {}
	virtual void combatCancelled() {}
	virtual void hitpointsCritical() {}
	virtual void hitpointsRestored() {}
	virtual void speechInput( P_PLAYER pTalker, const QString &message ) {}
	virtual void targetCursorInput( cUOSocket *socket, cUORxTarget *target ) {}
//	virtual void pay() {}
	virtual void foundVictim( P_CHAR pVictim ) {}
	virtual void handleSelection( P_PLAYER pPlayer, cUORxBuy* packet ) {}
	virtual void handleSelection( P_PLAYER pPlayer, cUORxSell* packet ) {}
//	virtual void selectionCancelled() {}
//	virtual void selectionTimeOut() {}
	virtual void refresh() {}
	virtual void tameAttempt() {}
};

class AbstractState : public Actions, public Events
{
public:
	AbstractState() : nextState( NULL ), m_interface( NULL ) {}
	AbstractState( cNPC_AI* interface_ ) : nextState( NULL ), m_interface( interface_ ) {}
	~AbstractState() {}

	virtual QString stateType() = 0;

	AbstractState* nextState;

	virtual void execute() {}
	void setInterface( cNPC_AI* newInterface ) { m_interface = newInterface; }

protected:
	cNPC_AI*	m_interface;
};

class AbstractState_Wander : public AbstractState
{
public:
	AbstractState_Wander() { m_interface = NULL; npc = NULL; }
	AbstractState_Wander( cNPC_AI* interface_, P_NPC npc_ ) { m_interface = interface_; npc = npc_; }
	~AbstractState_Wander() {}

	virtual void execute();
};

class AbstractState_Combat : public AbstractState
{
public:
	AbstractState_Combat() { m_interface = NULL; npc = NULL; }
	AbstractState_Combat( cNPC_AI* interface_, P_NPC npc_ ) { m_interface = interface_; npc = npc_; }
	~AbstractState_Combat() {}

	virtual void execute();
};

class AbstractState_Flee : public AbstractState
{
public:
	AbstractState_Flee() { m_interface = NULL; npc = NULL; }
	AbstractState_Flee( cNPC_AI* interface_, P_NPC npc_ ) { m_interface = interface_; npc = npc_; }
	~AbstractState_Flee() {}

	virtual void execute();
};

/*
Basic Monster AI
includes:
- Aggressive Monster Level 0: 
		attacks anything attackable nearby until victory or defeat
- Aggressive Monster Level 1:
		attacks any target nearby, can flee and finds new target if not reachable
*/

class Monster_Aggressive_L0 : public cNPC_AI
{
public:
	Monster_Aggressive_L0();
	Monster_Aggressive_L0( P_NPC currnpc );

	virtual void eventHandler();

	virtual QString AIType() { return "Monster_Aggressive_L0"; }
	static void registerInFactory();
};

class Monster_Aggr_L0_Wander : public AbstractState_Wander
{
public:
	Monster_Aggr_L0_Wander() { m_interface = NULL; npc = NULL; }
	Monster_Aggr_L0_Wander( cNPC_AI* interface_, P_NPC npc_ ) { m_interface = interface_; npc = npc_; }
	~Monster_Aggr_L0_Wander() {}

	virtual QString stateType() { return "Monster_Aggr_L0_Wander"; }
	static void registerInFactory();

	// events handled
	virtual void attacked();
	virtual void foundVictim( P_CHAR pVictim );
};

class Monster_Aggr_L0_Combat : public AbstractState
{
public:
	Monster_Aggr_L0_Combat() { m_interface = NULL; }
	Monster_Aggr_L0_Combat( cNPC_AI* interface_, P_NPC npc_ ) { m_interface = interface_; npc = npc_; }
	~Monster_Aggr_L0_Combat() {}

	virtual QString stateType() { return "Monster_Aggr_L0_Combat"; }
	virtual void execute();
	static void registerInFactory();

	// events handled
	virtual void won();
	virtual void combatCancelled();
};

class Monster_Aggressive_L1 : public cNPC_AI
{
public:
	Monster_Aggressive_L1();
	Monster_Aggressive_L1( P_NPC currnpc );

	virtual void eventHandler();

	virtual QString AIType() { return "Monster_Aggressive_L1"; }
	static void registerInFactory();
};

class Monster_Aggr_L1_Wander : public AbstractState_Wander
{
public:
	Monster_Aggr_L1_Wander() { m_interface = NULL; }
	Monster_Aggr_L1_Wander( cNPC_AI* interface_, P_NPC npc_ ) { m_interface = interface_; npc = npc_; }
	~Monster_Aggr_L1_Wander() {}

	virtual QString stateType() { return "Monster_Aggr_L1_Wander"; }
	static void registerInFactory();

	// events handled
	virtual void attacked();
	virtual void foundVictim( P_CHAR pVictim );
};

class Monster_Aggr_L1_Combat : public AbstractState
{
public:
	Monster_Aggr_L1_Combat() { m_interface = NULL; }
	Monster_Aggr_L1_Combat( cNPC_AI* interface_, P_NPC npc_ ) { m_interface = interface_; npc = npc_; }
	~Monster_Aggr_L1_Combat() {}

	virtual QString stateType() { return "Monster_Aggr_L1_Combat"; }
	virtual void execute();
	static void registerInFactory();

	// events handled
	virtual void won();
	virtual void combatCancelled();
	virtual void hitpointsCritical();
};

class Monster_Aggr_L1_Flee : public AbstractState
{
public:
	Monster_Aggr_L1_Flee() { m_interface = NULL; }
	Monster_Aggr_L1_Flee( cNPC_AI* interface_, P_NPC npc_ ) { m_interface = interface_; npc = npc_; }
	~Monster_Aggr_L1_Flee() {}

	virtual QString stateType() { return "Monster_Aggr_L1_Flee"; }
	virtual void execute();
	static void registerInFactory();

	// events handled
	virtual void won();
	virtual void combatCancelled();
	virtual void hitpointsRestored();
};

class Human_Vendor : public cNPC_AI
{
public:
	Human_Vendor();
	Human_Vendor( P_NPC currnpc );

	virtual void eventHandler();

	virtual QString AIType() { return "Human_Vendor"; }
	static void registerInFactory();
};

class Human_Vendor_Wander : public AbstractState_Wander
{
public:
	Human_Vendor_Wander() { m_interface = NULL; }
	Human_Vendor_Wander( cNPC_AI* interface_, P_NPC npc_ ) { m_interface = interface_; npc = npc_; }
	~Human_Vendor_Wander() {}

	virtual QString stateType() { return "Human_Vendor_Wander"; }
	static void registerInFactory();

	// events handled
	virtual void attacked();
	virtual void speechInput( P_PLAYER pTalker, const QString &message );
	virtual void handleSelection( P_PLAYER pPlayer, cUORxBuy* packet );
	virtual void handleSelection( P_PLAYER pPlayer, cUORxSell* packet );
};

class Human_Vendor_Combat : public AbstractState_Combat
{
public:
	Human_Vendor_Combat() { m_interface = NULL; }
	Human_Vendor_Combat( cNPC_AI* interface_, P_NPC npc_ ) { m_interface = interface_; npc = npc_; }
	~Human_Vendor_Combat() {}

	virtual QString stateType() { return "Human_Vendor_Combat"; }
//	virtual void execute();
	static void registerInFactory();

	// events handled
	virtual void won();
	virtual void combatCancelled();
	virtual void hitpointsCritical();
};

class Human_Vendor_Flee : public AbstractState_Flee
{
public:
	Human_Vendor_Flee() { m_interface = NULL; }
	Human_Vendor_Flee( cNPC_AI* interface_, P_NPC npc_ ) { m_interface = interface_; npc = npc_; }
	~Human_Vendor_Flee() {}

	virtual QString stateType() { return "Human_Vendor_Flee"; }
//	virtual void execute();
	static void registerInFactory();

	// events handled
	virtual void won();
	virtual void combatCancelled();
	virtual void hitpointsRestored();
};

/*
class Human_Vendor_BuyQuery : public AbstractState
{
public:
	Human_Vendor_BuyQuery() { m_interface = NULL; }
	Human_Vendor_BuyQuery( cNPC_AI* interface_, P_NPC npc_ ) { m_interface = interface_; npc = npc_; }
	~Human_Vendor_BuyQuery() {}

	virtual QString stateType() { return "Human_Vendor_BuyQuery"; }
//	virtual void execute();
	static void registerInFactory();

	// events handled
	virtual void handleSelection( P_PLAYER pPlayer, cUORxBuy* packet );
	virtual void selectionCancelled();
	virtual void selectionTimeOut();
};

class Human_Vendor_SellQuery : public AbstractState
{
public:
	Human_Vendor_SellQuery() { m_interface = NULL; }
	Human_Vendor_SellQuery( cNPC_AI* interface_, P_NPC npc_ ) { m_interface = interface_; npc = npc_; }
	~Human_Vendor_SellQuery() {}

	virtual QString stateType() { return "Human_Vendor_SellQuery"; }
//	virtual void execute();
	static void registerInFactory();

	// events handled
	virtual void handleSelection( P_PLAYER pPlayer, cUORxBuy* packet );
	virtual void selectionCancelled();
	virtual void selectionTimeOut();
};
*/

class Human_Stablemaster : public cNPC_AI
{
public:
	Human_Stablemaster();
	Human_Stablemaster( P_NPC currnpc );

	virtual void eventHandler();

	virtual QString AIType() { return "Human_Stablemaster"; }
	static void registerInFactory();
};

class Human_Stablemaster_Wander : public AbstractState_Wander
{
public:
	Human_Stablemaster_Wander() { m_interface = NULL; m_hasRefreshTimer = false; }
	Human_Stablemaster_Wander( cNPC_AI* interface_, P_NPC npc_ ) { m_interface = interface_; npc = npc_; m_hasRefreshTimer = false; }
	~Human_Stablemaster_Wander() {}

	virtual QString stateType() { return "Human_Stablemaster_Wander"; }
	static void registerInFactory();
	virtual void execute();

	// events handled
	virtual void attacked();
	virtual void speechInput( P_PLAYER pTalker, const QString &message );
	virtual void targetCursorInput( cUOSocket *socket, cUORxTarget *target );
	virtual void refresh();
//	virtual void handleSelection( P_PLAYER pPlayer, cUORxBuy* packet );
//	virtual void handleSelection( P_PLAYER pPlayer, cUORxSell* packet );

protected:
	bool m_hasRefreshTimer;
};

class Human_Stablemaster_Combat : public AbstractState_Combat
{
public:
	Human_Stablemaster_Combat() { m_interface = NULL; }
	Human_Stablemaster_Combat( cNPC_AI* interface_, P_NPC npc_ ) { m_interface = interface_; npc = npc_; }
	~Human_Stablemaster_Combat() {}

	virtual QString stateType() { return "Human_Stablemaster_Combat"; }
//	virtual void execute();
	static void registerInFactory();

	// events handled
	virtual void won();
	virtual void combatCancelled();
	virtual void hitpointsCritical();
};

class Human_Stablemaster_Flee : public AbstractState_Flee
{
public:
	Human_Stablemaster_Flee() { m_interface = NULL; }
	Human_Stablemaster_Flee( cNPC_AI* interface_, P_NPC npc_ ) { m_interface = interface_; npc = npc_; }
	~Human_Stablemaster_Flee() {}

	virtual QString stateType() { return "Human_Stablemaster_Flee"; }
//	virtual void execute();
	static void registerInFactory();

	// events handled
	virtual void won();
	virtual void combatCancelled();
	virtual void hitpointsRestored();
};


/* Basic animal AI 
- Animal_Wild : wild animals, shy, will flee on taming
- Animal_Domestic : domestic animals, just good pets :) (not implemented yet)
*/

class Animal_Wild : public cNPC_AI
{
public:
	Animal_Wild();
	Animal_Wild( P_NPC currnpc );

	virtual void eventHandler();

	virtual QString AIType() { return "Animal_Wild"; }
	static void registerInFactory();
};

class Animal_Wild_Wander : public AbstractState_Wander
{
public:
	Animal_Wild_Wander() { m_interface = NULL; }
	Animal_Wild_Wander( cNPC_AI* interface_, P_NPC npc_ ) { m_interface = interface_; npc = npc_; }
	~Animal_Wild_Wander() {}

	virtual QString stateType() { return "Animal_Wild_Wander"; }
	static void registerInFactory();

	virtual void execute();

	// events handled
	virtual void attacked();
	virtual void speechInput( P_PLAYER pTalker, const QString &message );
	virtual void tameAttempt();
};

class Animal_Wild_Combat : public AbstractState_Combat
{
public:
	Animal_Wild_Combat() { m_interface = NULL; }
	Animal_Wild_Combat( cNPC_AI* interface_, P_NPC npc_ ) { m_interface = interface_; npc = npc_; }
	~Animal_Wild_Combat() {}

	virtual QString stateType() { return "Human_Vendor_Combat"; }
//	virtual void execute();
	static void registerInFactory();

	// events handled
	virtual void won();
	virtual void combatCancelled();
	virtual void hitpointsCritical();
};

class Animal_Wild_Flee : public AbstractState_Flee
{
public:
	Animal_Wild_Flee() { m_interface = NULL; m_fleeingDueTame = false; }
	Animal_Wild_Flee( cNPC_AI* interface_, P_NPC npc_ ) { m_interface = interface_; npc = npc_; m_fleeingDueTame = false; }
	Animal_Wild_Flee( cNPC_AI* interface_, P_NPC npc_, bool fleeduetame ) { m_interface = interface_; npc = npc_; m_fleeingDueTame = fleeduetame; }
	~Animal_Wild_Flee() {}

	virtual QString stateType() { return "Animal_Wild_Flee"; }
//	virtual void execute();
	static void registerInFactory();

	// events handled
	virtual void won();
	virtual void combatCancelled();
	virtual void hitpointsRestored();

	// special actions
	virtual void ceaseFlee();
protected:
	bool m_fleeingDueTame;
};


#endif /* AI_H_HEADER_INCLUDED */