
#include "targetactions.h"
#include "srvparams.h"
//#include "classes.h"
#include "skills.h"

bool cSkHealing::responsed( cUOSocket *socket, cUORxTarget *target )
{
	
	signed short tempshort;

	P_ITEM pib = FindItemBySerial( bandageSerial );	// item index of bandage
	
	P_CHAR pp = FindCharBySerial( target->serial() ); // pointer to patient
	if (pp != NULL)
	{
		P_CHAR ph = socket->player();	// points to the healer
		if (!SrvParams->bandageInCombat() && (pp->war() || ph->war()))
		{
			P_CHAR pc_attacker = FindCharBySerial(ph->attacker); // Ripper...cant heal while in a fight
			if ( (pc_attacker != NULL) && pc_attacker->war())
			{
				socket->sysMessage( tr("You can`t heal while in a fight!") );
				return true;
			}
		}
		if(ph->pos.distance(pp->pos)>5)
		{
			socket->sysMessage( tr("You are not close enough to apply the bandages.") );
			return true;
		}
		if ((ph->isInnocent()) &&(ph->serial != pp->serial))
		{
			if ((pp->crimflag()>0) ||(pp->isMurderer()))
			{
				criminal(ph);
			}
		}
		
		if (pp->dead())
		{
			if (ph->skill(HEALING) < 800 || ph->skill(ANATOMY) < 800)
				socket->sysMessage( tr("You are not skilled enough to resurrect") );
			else
			{
				int reschance = static_cast<int>((ph->baseSkill(HEALING)+ph->baseSkill(ANATOMY))*0.17);
				int rescheck=RandomNum(1,100);
				if (ph->checkSkill(HEALING,800,1000) && ph->checkSkill(ANATOMY,800,1000) && reschance<=rescheck)
					socket->sysMessage( tr("You failed to resurrect the ghost") );
				else
				{
					Targ->NpcResurrectTarget(ph);
					socket->sysMessage( tr("Because of your skill, you were able to resurrect the ghost.") );
				}
			}
			return true;
		}
		
		if (pp->poisoned()>0)
		{
			if ( pp->isHuman() )
			{
				if (ph->skill(HEALING)<600 || ph->skill(ANATOMY)<600)
				{
					socket->sysMessage( tr("You are not skilled enough to cure poison.") );
					socket->sysMessage( tr("The poison in your target's system counters the bandage's effect.") );
				}
				else
				{
					int curechance = static_cast<int>((ph->baseSkill(HEALING)+ph->baseSkill(ANATOMY))*0.67);
					int curecheck=RandomNum(1,100);
					ph->checkSkill(HEALING,600,1000);
					ph->checkSkill(ANATOMY,600,1000);
					if(curechance<=curecheck)
					{
						pp->setPoisoned(0);
						socket->sysMessage( tr("Because of your skill, you were able to counter the poison.") );
					}
					else
						socket->sysMessage( tr("You fail to counter the poison") );
					pib->ReduceAmount(1);
				}
				return true;
			}
			else
			{
				if (ph->baseSkill(VETERINARY)<=600 || ph->baseSkill(ANIMALLORE)<=600)
				{
					socket->sysMessage( tr("You are not skilled enough to cure poison."));
					socket->sysMessage( tr("The poison in your target's system counters the bandage's effect."));
				}
				else
				{
					if (ph->checkSkill(VETERINARY,600,1000) &&
						ph->checkSkill(ANIMALLORE,600,1000))
					{
						pp->setPoisoned(0);
						socket->sysMessage( tr("Because of your skill, you were able to counter the poison."));
					}
					else
					{
						socket->sysMessage( tr("You fail to counter the poison"));
						pib->ReduceAmount(1);
					}
				}
			}
			return true;
		}
		
		if(pp->hp() == pp->st() )
		{
			socket->sysMessage( tr("That being is not damaged") );
			return true;
		}
		
		if(pp->isHuman()) //Used on human
		{
			if (!ph->checkSkill(HEALING,0,1000))
			{
				socket->sysMessage( tr("You apply the bandages, but they barely help!") );
//				pp->hp++;
				tempshort = pp->hp();
				pp->setHp(++tempshort);
			}
			else
			{
				int healmin = (((ph->skill(HEALING)/5)+(ph->skill(ANATOMY)/5))+3); //OSI's formula for min amount healed (Skyfire)
				int healmax = (((ph->skill(HEALING)/5)+(ph->skill(ANATOMY)/2))+10); //OSI's formula for max amount healed (Skyfire)
				int j=RandomNum(healmin,healmax);
				//int iMore1 = min(pp->st, j+pp->hp)-pp->hp;
				if(j>(pp->st() -pp->hp()))
					j=(pp->st() -pp->hp());
				if(pp->serial==ph->serial)
					tempeffect(ph, ph, 35, j, 0, 15, 0);//allow a delay
				else 
					tempeffect(ph, ph, 35, j, 0, 5, 0);// added suggestion by Ramases //-Fraz- must be checked
			}
		}
		else //Bandages used on a non-human
		{
			if (!ph->checkSkill(VETERINARY,0,1000))
				socket->sysMessage( tr("You are not skilled enough to heal that creature.") );
			else
			{
				int healmin = (((ph->skill(HEALING)/5)+(ph->skill(VETERINARY)/5))+3); //OSI's formula for min amount healed (Skyfire)
				int healmax = (((ph->skill(HEALING)/5)+(ph->skill(VETERINARY)/2))+10); //OSI's formula for max amount healed (Skyfire)
				int j = RandomNum(healmin, healmax);
				// khpae
				pp->setHp( (pp->st() > (pp->hp() + j)) ? (pp->hp() + j) : pp->st() );
				updatestats(pp, 0);
				socket->sysMessage( tr("You apply the bandages and the creature looks a bit healthier.") );
			}
		}
		SetTimerSec(&ph->objectdelay,SrvParams->objectDelay() + SrvParams->bandageDelay());
		pib->ReduceAmount(1);
	}
	return true;
}

bool cSkLockpicking::responsed( cUOSocket *socket, cUORxTarget *target )
{
 	const P_ITEM pi = FindItemBySerial( target->serial() );
	P_CHAR pc_currchar = socket->player();
	if (pi && !pi->isLockedDown()) // Ripper
	{
		P_ITEM piPick = FindItemBySerial(lockPick);
		if (piPick == NULL)
			return true;
		if(pi->type()==1 || pi->type()==12 || pi->type()==63) 
		{
			socket->sysMessage( tr("That is not locked.") );
			return true;
		}
		
		if(pi->type()==8 || pi->type()==13 || pi->type()==64)
		{
			if(pi->more1==0 && pi->more2==0 && pi->more3==0 && pi->more4==0)
			{ //Make sure it isn't an item that has a key (i.e. player house, chest..etc)
				if(pc_currchar->checkSkill( LOCKPICKING, 0, 1000))
				{
					switch(pi->type())
					{
					case 8:  pi->setType( 1 );  break;
					case 13: pi->setType( 12 ); break;
					case 64: pi->setType( 63 ); break;
					default:
						LogError("switch reached default");
						return true;
					}
					pi->soundEffect( 0x241 );
				} 
				else
				{
					if((rand()%100)>50) 
					{
						socket->sysMessage( tr("You broke your lockpick!") );
						piPick->ReduceAmount( 1 );
					} 
					else
						socket->sysMessage( tr("You fail to open the lock.") );
				}
			} else
			{
				socket->sysMessage( tr("That cannot be unlocked without a key.") );
			}
		}
	}
	return true;
}