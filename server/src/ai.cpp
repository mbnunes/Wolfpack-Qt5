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

#include "python/utilities.h"

#include "ai.h"
#include "npc.h"
#include "sectors.h"
#include "player.h"
#include "srvparams.h"
#include "globals.h"
#include "basics.h"
#include "walking.h"
#include "itemid.h"
#include "items.h"
#include "console.h"
#include "world.h"

// library includes
#include <math.h>
#include <vector>

void cAIFactory::checkScriptAI( const QStringList &oldSections, const QStringList &newSections )
{
	QStringList::const_iterator aiit = oldSections.begin();
	while( aiit != oldSections.end() )
	{
		// We must reset all existing and scripted AI objects, so changes can take effect.
		if( !newSections.contains( *aiit ) )
		{
			cCharIterator iter;
			for( P_CHAR pChar = iter.first(); pChar; pChar = iter.next() )
			{
				P_NPC pNPC = dynamic_cast< P_NPC >(pChar);
				if( pNPC )
				{
					ScriptAI* ai = dynamic_cast< ScriptAI* >(pNPC->ai());
					if( ai && ai->name() == (*aiit) )
					{
						pNPC->setAI( *aiit );
					}
				}
			}
		}
		else
		{
			cCharIterator iter;
			for( P_CHAR pChar = iter.first(); pChar; pChar = iter.next() )
			{
				P_NPC pNPC = dynamic_cast< P_NPC >(pChar);
				if( pNPC )
				{
					ScriptAI* ai = dynamic_cast< ScriptAI* >(pNPC->ai());
					if( ai && ai->name() == (*aiit) )
					{
						delete ai;
						pNPC->setAI( NULL );
					}
				}
			}
			unregisterType( *aiit );
		}
		++aiit;
	}
	aiit = newSections.begin();
	while( aiit != newSections.end() )
	{
		if( !oldSections.contains( *aiit ) )
		{
			ScriptAI::registerInFactory( *aiit );
		}
		++aiit;
	}
}

struct stActionNode
{
	stActionNode( float fz, AbstractAction* ac ) : fuzzy( fz ), action( ac ) {}

	float			fuzzy;
	AbstractAction*	action;
};

struct ActionNodeComparePredicate : public std::binary_function<stActionNode, stActionNode, bool>
{
	bool operator()(const stActionNode &a, const stActionNode &b)
	{
		return a.fuzzy > b.fuzzy;
	}
};

void AbstractAI::check()
{
	// If we have no current action or our action cant be executed, we must get a new one
	if( !m_currentAction || ( m_currentAction && m_currentAction->preCondition() <= 0.0f ) )
	{
		std::vector< stActionNode > actions;
		std::vector< stActionNode >::iterator it;

		AbstractAction* action = NULL;
		for( action = m_actions.first(); action; action = m_actions.next() )
		{
			actions.push_back( stActionNode( action->preCondition(), action ) );
		}
		std::sort( actions.begin(), actions.end(), ActionNodeComparePredicate() );

		it = actions.begin();
		while( it != actions.end() && !m_currentAction )
		{
			if( (*it).fuzzy > 0.0f )
				m_currentAction = (*it).action;

			++it;
		}
	}

	// Now we should have a current action set, else do nothing!
	if( m_currentAction )
	{
		m_currentAction->execute();

		// We must check the postcondition now and set the current action to NULL
		// if the action is finished (when it returns >= 1.0f)!
		float rnd = RandomNum( 0, 1000 ) / 1000.0f;
		if( m_currentAction->postCondition() >= rnd )
			m_currentAction = NULL;
	}
}

static AbstractAI* productCreator_SCP()
{
	return new ScriptAI( NULL );
}

void ScriptAI::registerInFactory( const QString &name )
{
	AIFactory::instance()->registerType(name, productCreator_SCP);
}

void ScriptAI::processNode( const cElement *Tag )
{
	QString TagName = Tag->name();
	// <action precondition="scriptfunction" postcondition="scriptfunction" execute="scriptfunction" onspeech="scriptfunction" />
	if( TagName == "action" )
	{
		if( Tag->hasAttribute( "precondition" ) &&
			Tag->hasAttribute( "postcondition" ) &&
			Tag->hasAttribute( "execute" ) )
		{
			ScriptAction* action = new ScriptAction( m_npc, this );
			action->setPreCondFunction( Tag->getAttribute( "precondition" ) );
			action->setPostCondFunction( Tag->getAttribute( "postcondition" ) );
			action->setExecuteFunction( Tag->getAttribute( "execute" ) );

			m_actions.append( action );
		}
		else
			Console::instance()->send( "Action tag in ai definition must contain attributes for pre-,postcondition and execute at least\n" );
	}
	else if( TagName == "onspeech" )
	{
		setOnSpeechFunction( Tag->getValue() );
	}
}

void ScriptAI::init( P_NPC npc )
{
	const cElement* node = DefManager->getDefinition( WPDT_AI, m_name );
	if( node )
		applyDefinition( node );
	AbstractAI::init( npc );
}

void ScriptAI::onSpeechInput( P_PLAYER pTalker, const QString &comm )
{
	if( !onspeech.isNull() )
	{
		// Try to call the python function
		// Get everything before the last dot
		if( onspeech.contains( "." ) )
		{
			// Find the last dot
			INT32 position = onspeech.findRev( "." );
			QString sModule = onspeech.left( position );
			QString sFunction = onspeech.right( onspeech.length() - (position+1) );

			PyObject *pModule = PyImport_ImportModule( const_cast< char* >( sModule.latin1() ) );

			if( pModule )
			{
				PyObject *pFunc = PyObject_GetAttrString( pModule, const_cast< char* >( sFunction.latin1() ) );
				if( pFunc && PyCallable_Check( pFunc ) )
				{
					// Create our Argument list
					PyObject *p_args = PyTuple_New( 3 );
					PyTuple_SetItem( p_args, 0, PyGetCharObject( m_npc ) );
					PyTuple_SetItem( p_args, 1, PyGetCharObject( pTalker ) );
					PyTuple_SetItem( p_args, 2, PyString_FromString( comm.latin1() ) );

					Py_XDECREF( PyEval_CallObject( pFunc, p_args ) );

					Py_XDECREF( p_args );

					reportPythonError( sModule );
				}
				Py_XDECREF( pFunc );
			}

			Py_XDECREF( pModule );
		}
	}
}

float ScriptAction::preCondition()
{
	if( !precond.isNull() )
	{
		// Try to call the python function
		// Get everything before the last dot
		if( precond.contains( "." ) )
		{
			// Find the last dot
			INT32 position = precond.findRev( "." );
			QString sModule = precond.left( position );
			QString sFunction = precond.right( precond.length() - (position+1) );

			PyObject *pModule = PyImport_ImportModule( const_cast< char* >( sModule.latin1() ) );

			if( pModule )
			{
				PyObject *pFunc = PyObject_GetAttrString( pModule, const_cast< char* >( sFunction.latin1() ) );
				if( pFunc && PyCallable_Check( pFunc ) )
				{
					// Create our Argument list
					PyObject *p_args = PyTuple_New( 3 );
					PyTuple_SetItem( p_args, 0, PyGetCharObject( m_npc ) );
					PyTuple_SetItem( p_args, 1, Py_None );
					PyTuple_SetItem( p_args, 2, Py_None );

					PyObject *returnValue = PyObject_CallObject( pFunc, p_args ); 

					Py_XDECREF( p_args );

					reportPythonError( sModule );
					
					if( returnValue == NULL || !PyFloat_Check( returnValue ) ) 
					{
						Py_XDECREF( returnValue );
						return 1.0f;
					}
					else
					{
						double result = PyFloat_AsDouble( returnValue );
						Py_XDECREF( returnValue );
						return result;
					}
				}
				Py_XDECREF( pFunc );
			}

			Py_XDECREF( pModule );
		}
	}
	return 0.0f;
}

float ScriptAction::postCondition()
{
	if( !postcond.isNull() )
	{
		// Try to call the python function
		// Get everything before the last dot
		if( postcond.contains( "." ) )
		{
			// Find the last dot
			INT32 position = postcond.findRev( "." );
			QString sModule = postcond.left( position );
			QString sFunction = postcond.right( postcond.length() - (position+1) );

			PyObject *pModule = PyImport_ImportModule( const_cast< char* >( sModule.latin1() ) );

			if( pModule )
			{
				PyObject *pFunc = PyObject_GetAttrString( pModule, const_cast< char* >( sFunction.latin1() ) );
				if( pFunc && PyCallable_Check( pFunc ) )
				{
					// Create our Argument list
					PyObject *p_args = PyTuple_New( 3 );
					PyTuple_SetItem( p_args, 0, PyGetCharObject( m_npc ) );
					PyTuple_SetItem( p_args, 1, Py_None );
					PyTuple_SetItem( p_args, 2, Py_None );

					PyObject *returnValue = PyObject_CallObject( pFunc, p_args ); 

					Py_XDECREF( p_args );

					reportPythonError( sModule );
					
					if( returnValue == NULL || !PyFloat_Check( returnValue ) ) 
					{
						Py_XDECREF( returnValue );
						return 1.0f;
					}
					else
					{
						double result = PyFloat_AsDouble( returnValue );
						Py_XDECREF( returnValue );
						return result;
					}
				}
				Py_XDECREF( pFunc );
			}

			Py_XDECREF( pModule );
		}
	}
	return 1.0f;
}

void ScriptAction::execute()
{
	if( !exec.isNull() )
	{
		// Try to call the python function
		// Get everything before the last dot
		if( exec.contains( "." ) )
		{
			// Find the last dot
			INT32 position = exec.findRev( "." );
			QString sModule = exec.left( position );
			QString sFunction = exec.right( exec.length() - (position+1) );

			PyObject *pModule = PyImport_ImportModule( const_cast< char* >( sModule.latin1() ) );

			if( pModule )
			{
				PyObject *pFunc = PyObject_GetAttrString( pModule, const_cast< char* >( sFunction.latin1() ) );
				if( pFunc && PyCallable_Check( pFunc ) )
				{
					// Create our Argument list
					PyObject *p_args = PyTuple_New( 3 );
					PyTuple_SetItem( p_args, 0, PyGetCharObject( m_npc ) );
					PyTuple_SetItem( p_args, 1, Py_None );
					PyTuple_SetItem( p_args, 2, Py_None );

					Py_XDECREF( PyEval_CallObject( pFunc, p_args ) );

					Py_XDECREF( p_args );

					reportPythonError( sModule );
				}
				Py_XDECREF( pFunc );
			}

			Py_XDECREF( pModule );
		}
	}
}

float Action_Wander::preCondition()
{
	/*
	 * Wandering has the following preconditions:
	 * - There is no character attacking us.
	 * - The wander type is not enHalt.
	 * - We are wandering towards a destination and aren't there yet.
	 * - We are following a char and aren't in follow range yet.
	 */

	if( m_npc->attackerSerial() != INVALID_SERIAL )
		return 0.0f;

	if( m_npc->wanderType() == enHalt )
		return 0.0f;

	if( m_npc->wanderType() == enDestination && m_npc->wanderDestination() == m_npc->pos() )
		return 0.0f;

	if( m_npc->wanderType() == enFollowTarget && !m_npc->inRange( m_npc->wanderFollowTarget(), SrvParams->pathfindFollowRadius() ) )
		return 0.0f;

	return 1.0f;
}

float Action_Wander::postCondition()
{
	/*
	 * Wandering has the following postconditions:
	 * - The NPC is wandering freely/in a circle/in a rectangle, 
	 *   then we can abort after each step!
	 * - The NPC has reached its destination.
	 * - The NPC is within follow range.
	 */

	switch( m_npc->wanderType() )
	{
	case enFreely:
	case enCircle:
	case enRectangle:
		return 1.0f;

	case enDestination:
		if( m_npc->wanderDestination() == m_npc->pos() )
			return 1.0f;
		break;

	case enFollowTarget:
		if( m_npc->inRange( m_npc->wanderFollowTarget(), SrvParams->pathfindFollowRadius() ) )
			return 1.0f;
		break;

	};
	return 0.0f;
}

void Action_Wander::execute()
{
	switch( m_npc->wanderType() )
	{
	case enFreely:
	{
		UINT8 dir = m_npc->direction();
		if( RandomNum(0, 100) < 20 )
			dir = RandomNum( 0, 7 );

		m_npc->setDirection( dir );
		Movement::instance()->Walking( m_npc, dir, 0xFF );
		break;
	}
	case enRectangle:
	{
		// get any point out of the rectangle and calculate the direction to it
		UINT16 rndx = RandomNum( m_npc->wanderX1(), m_npc->wanderX2() );
		UINT16 rndy = RandomNum( m_npc->wanderY1(), m_npc->wanderY2() );
		
		UINT8 dir = m_npc->pos().direction( Coord_cl( rndx, rndy ) );
		m_npc->setDirection( dir );
		Movement::instance()->Walking( m_npc, dir, 0xFF );
		break;
	}
	case enCircle:
	{
		Coord_cl pos = m_npc->pos();
		pos.x = m_npc->wanderX1();
		pos.y = m_npc->wanderY1();
		// get any point within the circle and calculate the direction to it
		// first a random distance which can be max. the length of the radius
		float rnddist = (float)RandomNum( 1, m_npc->wanderRadius() );
		// now get a point on this circle around the m_npc
		float rndphi = (float)RandomNum( 0, 100 ) / 100.0f * 2.0f * 3.14159265358979323846f;
		pos.x = pos.x + (INT16)floor( cos( rndphi ) * rnddist );
		pos.y = pos.y + (INT16)floor( sin( rndphi ) * rnddist );

		UINT8 dir = m_npc->pos().direction( pos );
		m_npc->setDirection( dir );
		Movement::instance()->Walking( m_npc, dir, 0xFF );
		break;	
	}
	case enFollowTarget:
	{
		if( SrvParams->pathfind4Follow() )
		{
			P_CHAR pTarget = m_npc->wanderFollowTarget();
			if( pTarget )
			{
				movePath( pTarget->pos() );
			}
			if( pTarget->dist( m_npc ) > 3 )
				m_npc->setAICheckTime( uiCurrentTime + (float)m_npc->aiCheckInterval() * 0.0005f * MY_CLOCKS_PER_SEC );
		}
		else
		{
			P_CHAR pTarget = m_npc->wanderFollowTarget();
			if( pTarget )
			{
				moveTo( pTarget->pos() );
			}
		}
		break;
	}
	case enDestination:
	{
		movePath( m_npc->wanderDestination() );
		break;
	}
	}
}

void Action_Wander::moveTo( const Coord_cl &pos )
{
	// simply move towards the target
	UINT8 dir = m_npc->pos().direction( pos );
	Coord_cl newPos = Movement::instance()->calcCoordFromDir( dir, m_npc->pos() );
	if( !mayWalk( m_npc, newPos ) )
	{
		if( dir == 7 )
			dir = 0;
		else
			dir++;

		newPos = Movement::instance()->calcCoordFromDir( dir, m_npc->pos() );
		if( !mayWalk( m_npc, newPos ) )
		{
			if( dir == 0 )
				dir = 6;
			else if( dir == 1 )
				dir = 7;
			else
				dir = dir - 2;

			newPos = Movement::instance()->calcCoordFromDir( dir, m_npc->pos() );
			if( !mayWalk( m_npc, newPos ) )
			{
				return;
			}
		}
	}

	m_npc->setDirection( dir );
	Movement::instance()->Walking( m_npc, dir, 0xFF );
}

void Action_Wander::movePath( const Coord_cl &pos )
{
	if( ( waitForPathCalculation <= 0 && !m_npc->hasPath() ) || pos != m_npc->pathDestination() )
	{
		UINT8 range = 1;
		if( m_npc->rightHandItem() && IsBowType( m_npc->rightHandItem()->id() ) )
			range = ARCHERY_RANGE;

		m_npc->findPath( pos, range == 1 ? 1.5f : (float)range );
		// dont return here!
	}
	else if( !m_npc->hasPath() )
	{
		waitForPathCalculation--;
		moveTo( pos );
		return;
	}

	if( m_npc->hasPath() )
	{
		waitForPathCalculation = 0;
		Coord_cl nextmove = m_npc->nextMove();
		UINT8 dir = m_npc->pos().direction( nextmove );
		m_npc->setDirection( dir );
		Movement::instance()->Walking( m_npc, dir, 0xFF );
		m_npc->popMove();
		return;
	}
	else
	{
		waitForPathCalculation = 3;
		moveTo( pos );
		return;
	}
}

void Action_Flee::execute()
{
	if( !m_npc->hasPath() )
	{
		Coord_cl newPos = m_npc->pos();
		Coord_cl fleePos = pFleeFrom->pos();

		// find a valid spot in a circle of flee_radius fields to move to
		float rnddist = (float)RandomNum( 1, SrvParams->pathfindFleeRadius() );
		if( newPos != fleePos )
		{
			int v1 = newPos.x - fleePos.x;
			int v2 = newPos.y - fleePos.y;
			float v_norm = sqrt( v1 * v1 + v2 * v2 );
			newPos.x = newPos.x + (INT16)floor( rnddist * v1 / v_norm );
			newPos.y = newPos.y + (INT16)floor( rnddist * v2 / v_norm );
		}
		else
		{
			float rndphi = (float)RandomNum( 0, 100 ) / 100.0f * 2 * 3.14;
			newPos.x = newPos.x + (INT16)floor( sin( rndphi ) * rnddist );
			newPos.y = newPos.y + (INT16)floor( cos( rndphi) * rnddist );
		}

		// we use pathfinding for fleeing
		movePath( newPos );
	}
	else
		movePath( m_npc->pathDestination() );
}

float Action_FleeAttacker::preCondition()
{
	/*
	 * Fleeing from an attacker has the following preconditions:
	 * - There is a character attacking us.
	 * - The attacking character has not died.
	 * - The attacking character is within flee range.
	 * - The hitpoints are below the critical value.
	 *
	 * Fuzzy: The nearer we get to the critical health, the chance
	 *	      increases to flee.
	 */

	P_CHAR pAttacker = World::instance()->findChar( m_npc->attackerSerial() );
	if( !pAttacker || pAttacker->isDead() || !m_npc->inRange( pAttacker, SrvParams->pathfindFleeRadius() ) )
		return 0.0f;

	if( m_npc->hitpoints() < m_npc->criticalHealth() )
		return 0.0f;

	pFleeFrom = pAttacker;
	float healthmod = (float)(m_npc->maxHitpoints() - m_npc->hitpoints()) /
						(float)(m_npc->maxHitpoints() - m_npc->criticalHealth());
	return healthmod;
}

float Action_FleeAttacker::postCondition()
{
	/*
	 * Fleeing from an attacker has the following postconditions:
	 * - The character isn't attacking us anymore.
	 * - The attacker has died.
	 * - The attacker is not within flee range.
	 * - The hitpoints are restored.
	 *
	 * Fuzzy: The farer we are from the critical health line,
	 *        the higher is the chance to end the flee action.
	 */

	P_CHAR pAttacker = World::instance()->findChar( m_npc->attackerSerial() );
	if( !pAttacker || pAttacker->isDead() || !m_npc->inRange( pAttacker, SrvParams->pathfindFleeRadius() ) )
		return 1.0f;

	float healthmod = (float)(m_npc->hitpoints() - m_npc->criticalHealth()) /
						(float)(m_npc->maxHitpoints() - m_npc->criticalHealth());
	return healthmod;
}

float Action_Defend::preCondition()
{
	/*
	 * Defending has the following preconditions:
	 * - There is a character attacking us.
	 * - The attacking character has not died.
	 * - The attacking character is within combat range.
	 * - The hitpoints are above the critical value.
	 *
	 * Fuzzy: The nearer we get to the critical health, the chance
	 *	      increases to flee.
	 */

	P_CHAR pAttacker = World::instance()->findChar( m_npc->attackerSerial() );
	if( !pAttacker || pAttacker->isDead() )
		return 0.0f;

	if( m_npc->hitpoints() < m_npc->criticalHealth() )
		return 0.0f;

	UINT8 range = 1;
	if( m_npc->rightHandItem() && IsBowType( m_npc->rightHandItem()->id() ) )
		range = ARCHERY_RANGE;

	if( !m_npc->inRange( pAttacker, range ) )
		return 0.0f;

	float healthmod = (float)m_npc->hitpoints() / ((float)m_npc->criticalHealth()/100.0f * (float)m_npc->maxHitpoints());
	return healthmod;
}

float Action_Defend::postCondition()
{
	/*
	 * Defending has the following postconditions:
	 * - The character isn't attacking us anymore.
	 * - The attacker has died.
	 * - The attacker is not within combat range.
	 * - Health is critical.
	 *
	 * Fuzzy: The nearer we get to the critical health line,
	 *        the higher is the chance to end the defend action.
	 */

	P_CHAR pAttacker = World::instance()->findChar( m_npc->attackerSerial() );
	if( !pAttacker || pAttacker->isDead() )
		return 1.0f;

	UINT8 range = 1;
	if( m_npc->rightHandItem() && IsBowType( m_npc->rightHandItem()->id() ) )
		range = ARCHERY_RANGE;

	if( !m_npc->inRange( pAttacker, range ) )
		return 1.0f;

	float healthmod = (float)(m_npc->maxHitpoints() - m_npc->hitpoints()) /
						(float)(m_npc->maxHitpoints() - ((float)m_npc->criticalHealth()/100.0f * (float)m_npc->maxHitpoints()));
	return healthmod;
}

void Action_Defend::execute()
{
	// combat is handled somewhere else
}

