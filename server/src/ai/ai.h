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
 * Wolfpack Homepage: http://developer.berlios.de/projects/wolfpack/
 */

#ifndef AI_H_HEADER_INCLUDED
#define AI_H_HEADER_INCLUDED

// platform includes
#include "../platform.h"

// wolfpack includes
#include "../basics.h"
#include "../factory.h"
#include "../definable.h"
#include "../singleton.h"
#include "../typedefs.h"
#include "../world.h"

// library includes
#include <qptrlist.h>
#include <qstring.h>
#include <qstringlist.h>

// forward declarations
class Coord;
class AbstractAI;

// Uncomment if you want AI debugging messages
// #define AIDEBUG

// Export this for other AI functions
bool invalidTarget( P_NPC npc, P_CHAR victim, int dist = -1 );
bool validTarget( P_NPC npc, P_CHAR victim, int dist = -1 );

class AbstractAction
{
protected:
	AbstractAction() : m_npc( NULL ), m_ai( NULL )
	{
	}
public:
	AbstractAction( P_NPC npc, AbstractAI* ai ) : m_npc( npc ), m_ai( ai )
	{
	}
	virtual ~AbstractAction()
	{
	}

	// is this action passive (wandering, etc.)?
	virtual bool isPassive() = 0;

	// executes the action
	virtual void execute() = 0;

	/* preCondition
	 *
	 * @return A value between 0 and 1 which indicates the
	 * propability of the action to be executed successfully.
	 * (So the method will return 0 if the action cannot be executed)
	 * This gives us the possibility of fuzzy logic.
	 */
	virtual float preCondition() = 0;

	/* postCondition
	 *
	 * @return A value between 0 and 1 which indicates the
	 * propability that this action will lead to a result
	 * in proper time.
	 * (So the method will return 1 if the action is finished)
	 * This gives us the possibility of fuzzy logic.
	 */
	virtual float postCondition() = 0;

	P_NPC npc() const
	{
		return m_npc;
	}
	void setNPC( P_NPC npc )
	{
		m_npc = npc;
	}
	virtual const char* name() = 0;

protected:
	P_NPC m_npc;
	AbstractAI* m_ai;
};

class AbstractAI
{
protected:
	AbstractAI() : m_npc( NULL ), m_currentAction( NULL ), notorietyOverride_( 0 )
	{
		m_actions.setAutoDelete( true );
	}

public:
	AbstractAI( P_NPC npc ) : m_npc( npc ), m_currentAction( NULL ), notorietyOverride_( 0 )
	{
		m_actions.setAutoDelete( true );
	}
	virtual ~AbstractAI()
	{
	} // virtual destructor.

	// some events that can be triggered from outside
	virtual void onSpeechInput( P_PLAYER pTalker, const QString& comm );

	// this method is called, when the npc ai should be checked
	virtual void check();

	virtual QString name() = 0;

	AbstractAction* currentAction()
	{
		return m_currentAction;
	}

	P_NPC npc() const
	{
		return m_npc;
	}
	void setNPC( P_NPC npc )
	{
		m_npc = npc;
	}

	unsigned char notorietyOverride() const
	{
		return notorietyOverride_;
	}
	void setnotorietyOverride( unsigned char value )
	{
		notorietyOverride_ = value;
	}

	// This is for creating AI interfaces through the AIFactory
	virtual void init( P_NPC npc )
	{
		m_npc = npc;
		AbstractAction* action = NULL;
		for ( action = m_actions.first(); action ; action = m_actions.next() )
		{
			action->setNPC( npc );
		}
	}

protected:
	P_NPC m_npc;
	AbstractAction* m_currentAction;
	QPtrList<AbstractAction> m_actions;
	unsigned char notorietyOverride_;
};

class cAIFactory : public Factory<AbstractAI, QString>
{
public:
	void checkScriptAI( const QStringList& oldSections, const QStringList& newSections );
};

typedef Singleton<cAIFactory> AIFactory;

class Action_Wander : public AbstractAction
{
protected:
	Action_Wander() : AbstractAction(), waitForPathCalculation( 0 )
	{
	}
public:
	virtual bool isPassive();
	Action_Wander( P_NPC npc, AbstractAI* ai ) : AbstractAction( npc, ai ), waitForPathCalculation( 0 )
	{
	}
	virtual void execute();
	virtual float preCondition();
	virtual float postCondition();
	virtual const char* name()
	{
		return "Action_Wander";
	}

protected:
	bool moveTo( const Coord& pos, bool run = false );
	bool movePath( const Coord& pos, bool run = false );
	int waitForPathCalculation;
};

class Action_Flee : public Action_Wander
{
protected:
	Action_Flee() : Action_Wander(), pFleeFromSer( INVALID_SERIAL )
	{
	}
public:
	Action_Flee( P_NPC npc, AbstractAI* ai ) : Action_Wander( npc, ai ), pFleeFromSer( INVALID_SERIAL )
	{
	}
	virtual void execute();

	virtual const char* name()
	{
		return "Action_Flee";
	}
protected:
	SERIAL pFleeFromSer;
};

class Action_FleeAttacker : public Action_Flee
{
protected:
	Action_FleeAttacker() : Action_Flee()
	{
	}

public:
	Action_FleeAttacker( P_NPC npc, AbstractAI* ai ) : Action_Flee( npc, ai )
	{
	}
	virtual float preCondition();
	virtual float postCondition();

	virtual const char* name()
	{
		return "Action_FleeAttacker";
	}
};

class Action_Defend : public AbstractAction
{
protected:
	Action_Defend() : AbstractAction()
	{
	}
public:
	Action_Defend( P_NPC npc, AbstractAI* ai ) : AbstractAction( npc, ai )
	{
	}
	virtual void execute();
	virtual float preCondition();
	virtual float postCondition();
	virtual bool isPassive()
	{
		return false;
	}
	virtual const char* name()
	{
		return "Action_Defend";
	}
};

class Monster_Aggr_Wander : public Action_Wander
{
protected:
	Monster_Aggr_Wander() : Action_Wander()
	{
	}
public:
	Monster_Aggr_Wander( P_NPC npc, AbstractAI* ai ) : Action_Wander( npc, ai )
	{
	}
	virtual float preCondition();

	virtual const char* name()
	{
		return "Monster_Aggr_Wander";
	}
};

class Monster_Aggr_MoveToTarget : public Action_Wander
{
protected:
	unsigned int nextTry;

	Monster_Aggr_MoveToTarget() : Action_Wander()
	{
		nextTry = 0;
	}
public:
	Monster_Aggr_MoveToTarget( P_NPC npc, AbstractAI* ai ) : Action_Wander( npc, ai )
	{
		nextTry = 0;
	}

	virtual bool isPassive()
	{
		return false;
	}

	virtual void execute();
	virtual float preCondition();
	virtual float postCondition();

	virtual const char* name()
	{
		return "Action_MoveToTarget";
	}
};

class Monster_Aggr_Fight : public AbstractAction
{
protected:
	Monster_Aggr_Fight() : AbstractAction()
	{
	}
public:
	virtual bool isPassive()
	{
		return false;
	}

	Monster_Aggr_Fight( P_NPC npc, AbstractAI* ai ) : AbstractAction( npc, ai )
	{
	}
	virtual void execute();
	virtual float preCondition();
	virtual float postCondition();

	virtual const char* name()
	{
		return "Monster_Aggr_Fight";
	}
};

class Monster_Aggressive : public AbstractAI
{
protected:
	Monster_Aggressive() : AbstractAI(), m_currentVictimSer( INVALID_SERIAL )
	{
		notorietyOverride_ = 3;
		nextVictimCheck = 0;
	}

public:
	Monster_Aggressive( P_NPC npc ) : AbstractAI( npc ), m_currentVictimSer( INVALID_SERIAL )
	{
		notorietyOverride_ = 3;
		nextVictimCheck = 0;
	}

	virtual void check();

	P_CHAR currentVictim() const
	{
		return World::instance()->findChar( m_currentVictimSer );
	}
protected:
	virtual void selectVictim() = 0;
	unsigned int nextVictimCheck;
	SERIAL m_currentVictimSer;
};

class Monster_Aggressive_L0 : public Monster_Aggressive
{
protected:
	Monster_Aggressive_L0() : Monster_Aggressive()
	{
	}

public:
	Monster_Aggressive_L0( P_NPC npc ) : Monster_Aggressive( npc )
	{
		m_actions.append( new Monster_Aggr_Wander( npc, this ) );
		m_actions.append( new Monster_Aggr_MoveToTarget( npc, this ) );
		m_actions.append( new Monster_Aggr_Fight( npc, this ) );
	}

	static void registerInFactory();
	virtual QString name()
	{
		return "Monster_Aggressive_L0";
	}

protected:
	virtual void selectVictim();
};

class Monster_Aggressive_L1 : public Monster_Aggressive
{
protected:
	Monster_Aggressive_L1() : Monster_Aggressive()
	{
	}

public:
	Monster_Aggressive_L1( P_NPC npc ) : Monster_Aggressive( npc )
	{
		m_actions.append( new Monster_Aggr_Wander( npc, this ) );
		m_actions.append( new Action_FleeAttacker( npc, this ) );
		m_actions.append( new Monster_Aggr_MoveToTarget( npc, this ) );
		m_actions.append( new Monster_Aggr_Fight( npc, this ) );
	}

	static void registerInFactory();
	virtual QString name()
	{
		return "Monster_Aggressive_L1";
	}

protected:
	virtual void selectVictim();
};

class Monster_Berserk : public Monster_Aggressive_L0
{
protected:
	Monster_Berserk() : Monster_Aggressive_L0()
	{
	}
	unsigned int lastVictimChange;

public:
	Monster_Berserk( P_NPC npc ) : Monster_Aggressive_L0( npc )
	{
		lastVictimChange = 0;
	}

	static void registerInFactory();
	virtual QString name()
	{
		return "Monster_Berserk";
	}

protected:
	virtual void selectVictim();
};

class Normal_Base : public AbstractAI
{
protected:
	Normal_Base() : AbstractAI()
	{
	}

public:
	Normal_Base( P_NPC npc ) : AbstractAI( npc )
	{
		m_actions.append( new Action_Wander( npc, this ) );
		m_actions.append( new Action_FleeAttacker( npc, this ) );
	}

	static void registerInFactory();
	virtual QString name()
	{
		return "Normal_Base";
	}
};

class Human_Vendor : public AbstractAI
{
	OBJECTDEF( Human_Vendor )
protected:
	Human_Vendor() : AbstractAI()
	{
		notorietyOverride_ = 1;
	}

public:
	Human_Vendor( P_NPC npc ) : AbstractAI( npc )
	{
		notorietyOverride_ = 1;
		m_actions.append( new Action_Wander( npc, this ) );
		m_actions.append( new Action_FleeAttacker( npc, this ) );
	}

	virtual void onSpeechInput( P_PLAYER pTalker, const QString& comm );

	static void registerInFactory();
	virtual QString name()
	{
		return "Human_Vendor";
	}
};

class cUORxTarget;

class Human_Stablemaster : public Human_Vendor
{
	OBJECTDEF( Human_Stablemaster )
protected:
	Human_Stablemaster()
	{
		notorietyOverride_ = 1;
	}

public:
	Human_Stablemaster( P_NPC npc );
	virtual void init( P_NPC npc );

	virtual void onSpeechInput( P_PLAYER pTalker, const QString& comm );

	static void registerInFactory();
	virtual QString name()
	{
		return "Human_Stablemaster";
	}

	void refreshStock();
	void handleTargetInput( P_PLAYER player, cUORxTarget* target );
};

class Animal_Wild_Flee : public Action_Flee
{
protected:
	Animal_Wild_Flee() : Action_Flee()
	{
	}
public:
	Animal_Wild_Flee( P_NPC npc, AbstractAI* ai ) : Action_Flee( npc, ai )
	{
	}
	virtual float preCondition();
	virtual float postCondition();

	virtual const char* name()
	{
		return "Animal_Wild_Flee";
	}
};

class AnimalAI : public AbstractAI
{
protected:
	AnimalAI() : AbstractAI()
	{
	}

public:
	AnimalAI( P_NPC npc ) : AbstractAI( npc )
	{
	}

	virtual void onSpeechInput( P_PLAYER pTalker, const QString& comm );
};

class Animal_Wild : public AnimalAI
{
protected:
	Animal_Wild() : AnimalAI()
	{
	}

public:
	Animal_Wild( P_NPC npc ) : AnimalAI( npc )
	{
		m_actions.append( new Action_Wander( npc, this ) );
		m_actions.append( new Action_FleeAttacker( npc, this ) );
		m_actions.append( new Action_Defend( npc, this ) );
		m_actions.append( new Animal_Wild_Flee( npc, this ) );
		m_actions.append( new Monster_Aggr_MoveToTarget( npc, this ) );
	}

	static void registerInFactory();
	virtual QString name()
	{
		return "Animal_Wild";
	}
};

class Animal_Domestic : public AnimalAI
{
protected:
	Animal_Domestic() : AnimalAI()
	{
	}

public:
	Animal_Domestic( P_NPC npc ) : AnimalAI( npc )
	{
		m_actions.append( new Action_Wander( npc, this ) );
		m_actions.append( new Action_FleeAttacker( npc, this ) );
		m_actions.append( new Action_Defend( npc, this ) );
	}

	static void registerInFactory();
	virtual QString name()
	{
		return "Animal_Domestic";
	}
};

class ScriptAction : public AbstractAction
{
protected:
	ScriptAction() : AbstractAction(), exec( ( char* ) 0 ), precond( ( char* ) 0 ), postcond( ( char* ) 0 )
	{
	}
public:
	ScriptAction( P_NPC npc, AbstractAI* ai ) : AbstractAction( npc, ai ), exec( ( char* ) 0 ), precond( ( char* ) 0 ), postcond( ( char* ) 0 )
	{
	}
	virtual bool isPassive();
	virtual void execute();
	virtual float preCondition();
	virtual float postCondition();

	void setExecuteFunction( const QString& data )
	{
		exec = data;
	}
	void setPreCondFunction( const QString& data )
	{
		precond = data;
	}
	void setPostCondFunction( const QString& data )
	{
		postcond = data;
	}

	virtual const char* name()
	{
		return "ScriptAction:" + exec;
	}

protected:
	QString exec;
	QString precond;
	QString postcond;
};

class ScriptAI : public AbstractAI, public cDefinable
{
protected:
	ScriptAI() : AbstractAI(), onspeech( ( char* ) 0 )
	{
	}

	virtual void processNode( const cElement* Tag );

public:
	ScriptAI( P_NPC npc ) : AbstractAI( npc ), onspeech( ( char* ) 0 )
	{
	}

	static void registerInFactory( const QString& name );
	virtual QString name()
	{
		return m_name;
	}
	void setName( const QString& d )
	{
		m_name = d;
	}
	virtual void init( P_NPC npc );

	virtual void onSpeechInput( P_PLAYER pTalker, const QString& comm );
	void setOnSpeechFunction( const QString& data )
	{
		onspeech = data;
	}

protected:
	QString m_name;
	QString onspeech;
};

class Human_Guard_Called_Fight : public AbstractAction
{
	OBJECTDEF( Human_Guard_Called_Fight )
protected:
	Human_Guard_Called_Fight() : AbstractAction()
	{
	}
public:
	Human_Guard_Called_Fight( P_NPC npc, AbstractAI* ai ) : AbstractAction( npc, ai )
	{
	}
	virtual void execute();
	virtual float preCondition();
	virtual float postCondition();
	virtual bool isPassive()
	{
		return false;
	}

	virtual const char* name()
	{
		return "Human_Guard_Called_Fight";
	}
};

class Human_Guard_Called_TeleToTarget : public AbstractAction
{
protected:
	Human_Guard_Called_TeleToTarget() : AbstractAction()
	{
	}
public:
	Human_Guard_Called_TeleToTarget( P_NPC npc, AbstractAI* ai ) : AbstractAction( npc, ai )
	{
	}
	virtual bool isPassive()
	{
		return false;
	}
	virtual void execute();
	virtual float preCondition();
	virtual float postCondition();

	virtual const char* name()
	{
		return "Human_Guard_Called_TeleToTarget";
	}
};

class Human_Guard_Called_Disappear : public AbstractAction
{
protected:
	Human_Guard_Called_Disappear() : AbstractAction()
	{
	}
public:
	Human_Guard_Called_Disappear( P_NPC npc, AbstractAI* ai ) : AbstractAction( npc, ai )
	{
	}
	virtual bool isPassive()
	{
		return false;
	}
	virtual void execute();
	virtual float preCondition();
	virtual float postCondition();

	virtual const char* name()
	{
		return "Human_Guard_Called_Disappear";
	}
};

class cTerritory;

class Human_Guard_Called : public AbstractAI
{
protected:
	Human_Guard_Called() : AbstractAI()
	{
		notorietyOverride_ = 1;
	}

	cTerritory* region_;

public:
	Human_Guard_Called( P_NPC npc );
	virtual void init( P_NPC npc );

	static void registerInFactory();
	virtual QString name()
	{
		return "Human_Guard_Called";
	}
};

class Human_Guard_Wander : public Action_Wander
{
protected:
	Human_Guard_Wander() : Action_Wander()
	{
	}
public:
	Human_Guard_Wander( P_NPC npc, AbstractAI* ai ) : Action_Wander( npc, ai )
	{
	}
	virtual float preCondition();
	virtual float postCondition();

	virtual const char* name()
	{
		return "Human_Guard_Wander";
	}
};

class Human_Guard_MoveToTarget : public Action_Wander
{
protected:
	Human_Guard_MoveToTarget() : Action_Wander()
	{
	}
public:
	Human_Guard_MoveToTarget( P_NPC npc, AbstractAI* ai ) : Action_Wander( npc, ai )
	{
	}
	virtual void execute();
	virtual float preCondition();
	virtual float postCondition();

	virtual const char* name()
	{
		return "Human_Guard_MoveToTarget";
	}
};

class Human_Guard_Fight : public AbstractAction
{
	OBJECTDEF( Human_Guard_Fight )
protected:
	Human_Guard_Fight() : AbstractAction()
	{
	}
public:
	Human_Guard_Fight( P_NPC npc, AbstractAI* ai ) : AbstractAction( npc, ai )
	{
	}
	virtual bool isPassive()
	{
		return false;
	}
	virtual void execute();
	virtual float preCondition();
	virtual float postCondition();

	virtual const char* name()
	{
		return "Human_Guard_Fight";
	}
};

class Human_Guard : public AbstractAI
{
protected:
	Human_Guard() : AbstractAI(), m_currentVictimSer( INVALID_SERIAL )
	{
		notorietyOverride_ = 1;
	}

public:
	Human_Guard( P_NPC npc );

	static void registerInFactory();
	virtual QString name()
	{
		return "Human_Guard";
	}

	virtual void check();

	P_CHAR currentVictim() const
	{
		return World::instance()->findChar( m_currentVictimSer );
	}
protected:
	virtual void selectVictim();

	SERIAL m_currentVictimSer;
};

#ifndef __VC6
template <typename T>
AbstractAI* productCreatorFunctor()
{
	return new T( 0 );
}
#else
AbstractAI* productCreatorFunctor_Animal_Domestic();
AbstractAI* productCreatorFunctor_Animal_Wild();
AbstractAI* productCreatorFunctor_Human_Guard();
AbstractAI* productCreatorFunctor_Human_Guard_Called();
AbstractAI* productCreatorFunctor_Human_Vendor();
AbstractAI* productCreatorFunctor_Human_Stablemaster();
AbstractAI* productCreatorFunctor_Monster_Aggressive_L0();
AbstractAI* productCreatorFunctor_Monster_Berserk();
AbstractAI* productCreatorFunctor_Monster_Aggressive_L1();
#endif
#endif /* AI_H_HEADER_INCLUDED */
