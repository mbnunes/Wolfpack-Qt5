
#include "ai.h"
#include "ai_mage.h"
#include "../npc.h"
#include "../combat.h"
#include "../inlines.h"
#include "../walking.h"
#include "../serverconfig.h"

/*
	The additional mage code does:
	- Meditate while wandering to get more mana back.
*/
class Monster_Mage_Wander : public Monster_Aggr_Wander {
protected:
	Monster_Mage_Wander() : Monster_Aggr_Wander() {}

public:
	Monster_Mage_Wander(P_NPC npc, AbstractAI* ai) : Monster_Aggr_Wander(npc, ai) {}

	/*
		This is a slightly changed version of the combined super class methods.
		We still execute the wander action even if we dont really move.
	*/
	virtual float preCondition() {
		Monster_Aggressive* pAI = dynamic_cast<Monster_Aggressive*>( m_ai );
		if ( pAI && pAI->currentVictim() )
			return 0.0f;

		if ( m_npc->attackTarget() )
			return 0.0f;

		return 1.0f;
	}

	virtual void execute() {
		// Since we removed this from the precondition method,
		// we need check for it here.
		if ( m_npc->wanderType() != enHalt
			&& ( m_npc->wanderType() != enDestination || m_npc->wanderDestination() != m_npc->pos() )
			&& ( m_npc->wanderType() != enFollowTarget || m_npc->inRange( m_npc->wanderFollowTarget(), Config::instance()->pathfindFollowRadius() ) )
			)
		{
			Monster_Aggr_Wander::execute();
		}	

		// Should we heal ourself?
		if (m_npc->hitpoints() < m_npc->maxHitpoints()  // Only try to heal if we're not at full health
			&& !m_npc->summoned() // Summoned creatures dont heal
			&& (m_npc->skillValue(MAGERY) / 100) > RandomNum(1, 100) // We dont heal with every step
			) {
            int toheal = m_npc->maxHitpoints() - m_npc->hitpoints(); // How many hitpoints did we loose?
			if (toheal >= 50 && m_npc->mana() >= 11) { // Is it worth the effort? Do we have enough mana?
				// Try a greater heal
				if (m_npc->canHandleEvent(EVENT_CASTSPELL)) {
					PyObject* args = Py_BuildValue( "(Nii[]N)", m_npc->getPyObject(), 29, 0, m_npc->getPyObject() );
					bool result = m_npc->callEventHandler( EVENT_CASTSPELL, args );
					Py_DECREF( args );
				}
			} else if (m_npc->mana() >= 4) { // Only bother if we have enough mana
				// Try a normal heal instead
				if (m_npc->canHandleEvent(EVENT_CASTSPELL)) {
					PyObject* args = Py_BuildValue( "(Nii[]N)", m_npc->getPyObject(), 4, 0, m_npc->getPyObject() );
					bool result = m_npc->callEventHandler( EVENT_CASTSPELL, args );
					Py_DECREF( args );
				}
			}
		}
	}

	virtual const char* name() {
		return "Monster_Mage_Wander";
	}
};

/*
	The additional mage code does:
*/
class Monster_Mage_Fight : public Monster_Aggr_Fight {
protected:
	Monster_Mage_Fight() : Monster_Aggr_Fight() {}

public:
	Monster_Mage_Fight(P_NPC npc, AbstractAI* ai) : Monster_Aggr_Fight(npc, ai) {}

	virtual float preCondition() {
		return Monster_Aggr_Fight::preCondition(); // Casting has precende if possible
	}

	virtual const char* name() {
		return "Monster_Mage_Fight";
	}
};

class Monster_Mage_Cast : public AbstractAction {
protected:
	Monster_Mage_Cast() : AbstractAction() {}
	unsigned int nextSpellTime;
	int spell;
	cUObject *objTarget;
	Coord posTarget;

public:
	Monster_Mage_Cast(P_NPC npc, AbstractAI *ai) : AbstractAction(npc, ai) {
		nextSpellTime = 0;
		spell = -1; // Current Spell
		objTarget = 0;
		posTarget = Coord::null;
	}

	// Is the target dispellable?
	bool canDispel(P_CHAR target) {
		P_NPC npc = dynamic_cast<P_NPC>(target);
		return npc && npc->summoned() && m_npc->inRange(npc, 12);
	}

	/*
		Find something to dispel we are fighting.
	*/
	P_NPC findDispelOpponent() {
		if (m_npc->intelligence() < 95 || m_npc->summoned()) {
			return 0; // No dispelling below 95 int or if the NPC is summoned itself.
		}

		QPtrList<cFightInfo> &fights = m_npc->fights();
		Monster_Aggressive *ai = static_cast<Monster_Aggressive*>(m_ai);

		P_NPC currentTarget = 0;
		unsigned int currentPriority = 0;

		/*
			Check our current attack target. It has the highest priority of all since
			we are fighting it anyway.
		*/
		if (!invalidTarget(m_npc, ai->currentVictim()) && canDispel(ai->currentVictim())) {
			currentTarget = dynamic_cast<P_NPC>(ai->currentVictim());
			currentPriority = m_npc->dist(ai->currentVictim());
			if (currentPriority <= 2) {
				return currentTarget; // We found a threat in our range, so dispel it now.
			}
		}

		/*
			Now check everyone who is fighting us
		*/
		for (cFightInfo *info = fights.first(); info; info = fights.next()) {
			P_NPC checkTarget;
			if (info->victim() == m_npc) {
				checkTarget = dynamic_cast<P_NPC>(info->attacker());
			} else {
				checkTarget = dynamic_cast<P_NPC>(info->victim());
			}

			// They have to be fighting us or they are uninteresting for this check
			if (!checkTarget || checkTarget->attackTarget() != m_npc) {
				continue;
			}

			if (!invalidTarget(m_npc, checkTarget) && canDispel(checkTarget)) {
				unsigned int newPriority = m_npc->dist(checkTarget);
				if (!currentTarget || currentPriority > newPriority) {
					currentTarget = dynamic_cast<P_NPC>(ai->currentVictim());
					currentPriority = m_npc->dist(ai->currentVictim());
					if (currentPriority <= 2) {
						return currentTarget; // We found a threat in our range, so dispel it now.
					}
				}
			}
		}

		return currentTarget;
	}

	/*
		Get the id of a random damage spell
	*/
	int getRandomHarmfulSpell()
	{
		static const ushort mageryPerCircle = ( 1000 / 7 );
		int maxCircle = wpMin<int>( 8, wpMax<int>( 1, ( ( m_npc->skillValue( MAGERY ) + 200 ) / mageryPerCircle ) ) );
		int selected = RandomNum( 1, maxCircle * 2 ) - 1; // Select a random spell

		// 5: Magic Arrow
		// 12: Harm
		// 18: Fireball
		// 30: Lightning
		// 37: Mind Blast
		// 42: Energy Bolt
		// 43: Explosion
		// 51: FlameStrike (Default)
		static int const spells[16] = { 5, 5, 12, 12, 18, 18, 30, 30, 37, 37, 42, 43, 51, };

		return spells[selected];
	}

	/*
		Chose a random spell
	*/
	void chooseSpell(int &spell, cUObject *&objTarget, Coord &posTarget, P_CHAR currentVictim) {
		// If we are not summoned, try healing
		if (m_npc->hitpoints() < m_npc->maxHitpoints()  // Only try to heal if we're not at full health
			&& !m_npc->summoned() // Summoned creatures dont heal
			&& (m_npc->skillValue(MAGERY) / 100) > RandomNum(1, 100) // We dont heal with every step
			) {
            int toheal = m_npc->maxHitpoints() - m_npc->hitpoints(); // How many hitpoints did we loose?
			if (toheal >= 50 && m_npc->mana() >= 11) { // Is it worth the effort? Do we have enough mana?
				spell = 29; // Greater Heal
				objTarget = m_npc;
				return;
			} else if (toheal >= 10 && m_npc->mana() >= 4) { // Only bother if we have enough mana
				spell = 4; // Heal
				objTarget = m_npc;
				return;
			}
		}

		spell = getRandomHarmfulSpell();
		objTarget = currentVictim;
	}

	virtual float preCondition() {
		// Can we cast a spell?
		if (nextSpellTime > Server::instance()->time()) {
			return 0.0f;
		}

		Monster_Aggressive *ai = static_cast<Monster_Aggressive*>(m_ai);

		// If we are already casting a spell, cancel this
		if (m_npc->hasScript("magic")) {
			return 0.0f;
		}

		// Reinitialize to "zero"
		spell = -1;
		objTarget = 0;
		posTarget = Coord::null;

		// We dont have a spell ready, are ready to cast.
		P_NPC dispelTarget = findDispelOpponent();
		if (m_npc->poison() != -1) { // Always try to cure if poisoned
			spell = 11; // Cure
			objTarget = m_npc;
		} else if (dispelTarget && m_npc->skillValue(MAGERY) * 0.01 * 75 > RandomNum(1, 100)) { // We have something to dispel that is attacking us. Easily dispatch threat.
			spell = 41; // Dispel
			objTarget = dispelTarget;
		} else {
			P_CHAR currentVictim = ai->currentVictim();

			if (currentVictim && m_npc->inRange(currentVictim, 12)) {
				chooseSpell(spell, objTarget, posTarget, currentVictim); // Choose a spell
			}
		}

		if (spell == -1) {
			return 0.0f; // We couldn't find a spell to cast
		}
		
		return 2.0f; // Give us priority in the fuzzy logic
	}

	virtual float postCondition() {
		return 1.0f; // One time action
	}

	virtual void execute() {
		if (spell == -1) {
			return; // Shouldn't happen
		}

		if (m_npc->canHandleEvent(EVENT_CASTSPELL)) {
			PyObject *target = PyGetObjectObject(objTarget);
			PyObject* args = Py_BuildValue( "(Nii[]N)", m_npc->getPyObject(), spell, 0, target );
			bool result = m_npc->callEventHandler( EVENT_CASTSPELL, args );
			Py_DECREF( args );
		}

		// Calculate the next spell delay
		if (spell == 41) {
			nextSpellTime = Server::instance()->time() + m_npc->actionSpeed(); // Dispel gets special handling
		} else {
			float scale = m_npc->skillValue(MAGERY) * 0.003;
			unsigned int minDelay = 6000 - (scale * 750);
			unsigned int maxDelay = 6000 - (scale * 1250);
			nextSpellTime = Server::instance()->time() + RandomNum(minDelay, maxDelay);
		}
	}

	virtual const char* name() {
		return "Monster_Mage_Cast";
	}

	virtual bool isPassive() {
		return false;
	}
};

/*
	The additional mage code does:
	- Run away from the current target if we are casting a spell.
*/
class Monster_Mage_MoveToTarget : public Monster_Aggr_Wander {
protected:
	Monster_Mage_MoveToTarget() : Monster_Aggr_Wander() {}

public:
	Monster_Mage_MoveToTarget(P_NPC npc, AbstractAI* ai) : Monster_Aggr_Wander(npc, ai) {}

	virtual float preCondition() {
		if (m_npc->nextMoveTime() > Server::instance()->time()) {
			return 0.0f; // We can't move yet.
		}

		Monster_Aggressive *ai = static_cast<Monster_Aggressive*>(m_ai);

		if (ai->currentVictim()) {
			return 1.0f; // Always keep a distance
		} else {
			return 0.0f;
		}
	}

	virtual void execute() {
		P_CHAR currentVictim = static_cast<Monster_Aggressive*>(m_ai)->currentVictim();

		if (!currentVictim || m_npc->isFrozen())
			return;

		m_npc->setNextMoveTime();

		// If we're not casting a spell and waiting for a new one, move toward the target
		if (!m_npc->hasScript("magic")) {
			movePath(currentVictim->pos(), true);
		} else {
			unsigned int distance = m_npc->dist(currentVictim);

			if (distance >= 10 && distance <= 12) {
				return; // Right distance
			} else if (distance > 10) {
				movePath(currentVictim->pos(), true);
			} else if (distance < 10) {
				// Calculate the opposing direction and get away (at least try it)
				unsigned char direction = (m_npc->pos().direction(currentVictim->pos()) + 4) % 8;

				if (direction != m_npc->direction()) {
					if (!Movement::instance()->Walking(m_npc, direction|0x80, 0xFF)) {
						return;
					}
				}

				if (!Movement::instance()->Walking(m_npc, direction|0x80, 0xFF)) {
					direction = (direction + 1) % 8;

					if (direction != m_npc->direction()) {
						if (!Movement::instance()->Walking(m_npc, direction|0x80, 0xFF)) {
							return;
						}
					}

					if (!Movement::instance()->Walking(m_npc, direction|0x80, 0xFF)) {
						direction = (direction == 1) ? 7 : (direction - 2);

						if (direction != m_npc->direction()) {
							if (!Movement::instance()->Walking(m_npc, direction|0x80, 0xFF)) {
								return;
							}
						}

						if (!Movement::instance()->Walking(m_npc, direction|0x80, 0xFF)) {
							return; // We tried everything but failed
						}
					}
				}
			}
		}
	}

	virtual float postCondition() {
		return 1.0f; // This action doesn't last.
	}

	virtual const char* name() {
		return "Monster_Mage_MoveToTarget";
	}
};

Monster_Mage::Monster_Mage( P_NPC npc ) : Monster_Aggressive( npc ) {
	m_actions.append( new Monster_Mage_Wander( npc, this ) );
	m_actions.append( new Action_FleeAttacker( npc, this ) );
	m_actions.append( new Monster_Mage_MoveToTarget( npc, this ) );
	m_actions.append( new Monster_Mage_Fight( npc, this ) );
	m_actions.append( new Monster_Mage_Cast( npc, this ) );
}

void Monster_Mage::selectVictim() {
}
