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

#ifndef AI_H_HEADER_INCLUDED
#define AI_H_HEADER_INCLUDED

#include "typedefs.h"

// platform includes
#include "platform.h"

// wolfpack includes
#include "factory.h"

// library includes
#include "qptrlist.h"

// forward declarations
class Coord_cl;
class AbstractAI;

class AbstractAction
{
protected:
	AbstractAction() : m_npc( NULL ),m_ai( NULL ) {}
public:
	AbstractAction( P_NPC npc, AbstractAI* ai ) : m_npc( npc ), m_ai( ai ) {}

	// executes the action
	virtual void	execute() = 0;

	/* preCondition
	 *
	 * @return A value between 0 and 1 which indicates the
	 * propability of the action to be executed successfully.
	 * (So the method will return 0 if the action cannot be executed)
	 * This gives us the possibility of fuzzy logic.
	 */
	virtual float	preCondition() = 0;

	/* postCondition
	 * 
	 * @return A value between 0 and 1 which indicates the
	 * propability that this action will lead to a result 
	 * in proper time.
	 * (So the method will return 1 if the action is finished)
	 * This gives us the possibility of fuzzy logic.
	 */
	virtual float	postCondition() = 0;

	P_NPC			npc() const { return m_npc; }
	void			setNPC( P_NPC npc ) { m_npc = npc; }

protected:
	P_NPC			m_npc;
	AbstractAI*		m_ai;
};

class AbstractAI
{
protected:
	AbstractAI() : m_npc( NULL ), m_currentAction( NULL ) { m_actions.setAutoDelete( true ); }

public:
	AbstractAI( P_NPC npc ) : m_npc( npc ), m_currentAction( NULL ) { m_actions.setAutoDelete( true ); }
	virtual ~AbstractAI() {} // virtual destructor.

	// some events that can be triggered from outside
	virtual void onSpeechInput( P_PLAYER pTalker, const QString &comm ) {}

	// this method is called, when the npc ai should be checked
	virtual void check();

	virtual QString name() = 0;

	P_NPC			npc() const { return m_npc; }
	void			setNPC( P_NPC npc ) { m_npc = npc; }

	// This is for creating AI interfaces through the AIFactory
	virtual void	init( P_NPC npc )
	{
		m_npc = npc;
		AbstractAction* action = NULL;
		for( action = m_actions.first(); action ; action = m_actions.next() )
		{
			action->setNPC( npc );
		}
	}

protected:
	P_NPC			m_npc;
	AbstractAction*	m_currentAction;
	QPtrList< AbstractAction >	m_actions;
};

class AIFactory : public Factory< AbstractAI, QString >
{
public:
	static AIFactory* instance()
	{
		static AIFactory factory;
		return &factory;
	}
};

class Action_Wander : public AbstractAction
{
protected:
	Action_Wander() : AbstractAction() {}
public:
	Action_Wander( P_NPC npc, AbstractAI* ai ) : AbstractAction( npc, ai ) {}
	virtual void execute();
	virtual float preCondition();
	virtual float postCondition();

protected:
	void moveTo( const Coord_cl& pos );
	void movePath( const Coord_cl& pos );
	int waitForPathCalculation;
};

class Action_FleeAttacker : public Action_Wander
{
protected:
	Action_FleeAttacker() : Action_Wander() {}
public:
	Action_FleeAttacker( P_NPC npc, AbstractAI* ai ) : Action_Wander( npc, ai ) {}
	virtual void execute();
	virtual float preCondition();
	virtual float postCondition();
};

class Action_Defend : public AbstractAction
{
protected:
	Action_Defend() : AbstractAction() {}
public:
	Action_Defend( P_NPC npc, AbstractAI* ai ) : AbstractAction( npc, ai ) {}
	virtual void execute();
	virtual float preCondition();
	virtual float postCondition();
};

class Monster_Aggr_Wander : public Action_Wander
{
protected:
	Monster_Aggr_Wander() : Action_Wander() {}
public:
	Monster_Aggr_Wander( P_NPC npc, AbstractAI* ai ) : Action_Wander( npc, ai ) {}
	virtual float preCondition();
};

class Monster_Aggr_MoveToTarget : public Action_Wander
{
protected:
	Monster_Aggr_MoveToTarget() : Action_Wander() {}
public:
	Monster_Aggr_MoveToTarget( P_NPC npc, AbstractAI* ai ) : Action_Wander( npc, ai ) {}
	virtual void execute();
	virtual float preCondition();
	virtual float postCondition();
};

class Monster_Aggr_Fight : public AbstractAction
{
protected:
	Monster_Aggr_Fight() : AbstractAction() {}
public:
	Monster_Aggr_Fight( P_NPC npc, AbstractAI* ai ) : AbstractAction( npc, ai ) {}
	virtual void execute();
	virtual float preCondition();
	virtual float postCondition();
};

class Monster_Aggressive : public AbstractAI
{
protected:
	Monster_Aggressive() : AbstractAI(), m_currentVictim( NULL ) {}

public:
	Monster_Aggressive( P_NPC npc ) : AbstractAI( npc ), m_currentVictim( NULL ) {} 

	virtual void check();

	P_CHAR currentVictim() const { return m_currentVictim; }
protected:
	virtual void selectVictim() = 0;

	P_CHAR	m_currentVictim;
};

class Monster_Aggressive_L0 : public Monster_Aggressive
{
protected:
	Monster_Aggressive_L0() : Monster_Aggressive() {}

public:
	Monster_Aggressive_L0( P_NPC npc ) : Monster_Aggressive( npc ) 
	{
		m_actions.append( new Monster_Aggr_Wander( npc, this ) );
		m_actions.append( new Monster_Aggr_MoveToTarget( npc, this ) );
		m_actions.append( new Monster_Aggr_Fight( npc, this ) );
	}

	static void registerInFactory();
	virtual QString name() { return "Monster_Aggressive_L0"; }

protected:
	virtual void selectVictim();
};

class Monster_Aggressive_L1 : public Monster_Aggressive
{
protected:
	Monster_Aggressive_L1() : Monster_Aggressive() {}

public:
	Monster_Aggressive_L1( P_NPC npc ) : Monster_Aggressive( npc ) 
	{
		m_actions.append( new Monster_Aggr_Wander( npc, this ) );
		m_actions.append( new Action_FleeAttacker( npc, this ) );
		m_actions.append( new Monster_Aggr_MoveToTarget( npc, this ) );
		m_actions.append( new Monster_Aggr_Fight( npc, this ) );
	}

	static void registerInFactory();
	virtual QString name() { return "Monster_Aggressive_L1"; }

protected:
	virtual void selectVictim();
};

class Human_Vendor : public AbstractAI
{
protected:
	Human_Vendor() : AbstractAI() {}

public:
	Human_Vendor( P_NPC npc ) : AbstractAI( npc ) 
	{
		m_actions.append( new Action_Wander( npc, this ) );
		m_actions.append( new Action_FleeAttacker( npc, this ) );
	} 

	virtual void onSpeechInput( P_PLAYER pTalker, const QString &comm );

	static void registerInFactory();
	virtual QString name() { return "Human_Vendor"; }
};

class cUORxTarget;

class Human_Stablemaster : public AbstractAI
{
protected:
	Human_Stablemaster() : AbstractAI() {}

public:
	Human_Stablemaster( P_NPC npc );
	virtual void init( P_NPC npc );

	virtual void onSpeechInput( P_PLAYER pTalker, const QString &comm );

	static void registerInFactory();
	virtual QString name() { return "Human_Stablemaster"; }

	void refreshStock();
	void handleTargetInput( P_PLAYER player, cUORxTarget* target );
};

class Animal_Wild_Flee : public Action_Wander
{
protected:
	Animal_Wild_Flee() : Action_Wander() {}
public:
	Animal_Wild_Flee( P_NPC npc, AbstractAI* ai ) : Action_Wander( npc, ai ) {}
	virtual void execute();
	virtual float preCondition();
	virtual float postCondition();
};

class AnimalAI : public AbstractAI
{
protected:
	AnimalAI() : AbstractAI() {}

public:
	virtual void onSpeechInput( P_PLAYER pTalker, const QString &comm );
};

class Animal_Wild : public AnimalAI
{
protected:
	Animal_Wild() : AnimalAI() {}

public:
	Animal_Wild( P_NPC npc )	
	{
		m_actions.append( new Action_Wander( npc, this ) );
		m_actions.append( new Action_FleeAttacker( npc, this ) );
		m_actions.append( new Action_Defend( npc, this ) );
		m_actions.append( new Animal_Wild_Flee( npc, this ) );
	}

	static void registerInFactory();
	virtual QString name() { return "Animal_Wild"; }
};

class Animal_Domestic : public AnimalAI
{
protected:
	Animal_Domestic() : AnimalAI() {}

public:
	Animal_Domestic( P_NPC npc )	
	{
		m_actions.append( new Action_Wander( npc, this ) );
		m_actions.append( new Action_FleeAttacker( npc, this ) );
		m_actions.append( new Action_Defend( npc, this ) );
	}

	static void registerInFactory();
	virtual QString name() { return "Animal_Domestic"; }
};


#endif /* AI_H_HEADER_INCLUDED */