#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Created by: DarkStorm                      #
#   )).-' {{ ;'`   | Revised by:                                #
#  ( (  ;._ \\ ctr | Last Modification: Created                 #
#===============================================================#
# Utility functions used in Python scripts                      #
#===============================================================#

# Calculates the Magic Damage (Base Damage + Base + Source)

import wolfpack
from wolfpack.consts import *
from math import floor
import whrandom

def agression( source, target, spell ):
	spellinfo = wolfpack.spell( spell )
	
	# Spell is agressive
	if spellinfo[ 'flags' ] & 0x02:
		# Murderers or Criminals may be attacked freely
		if target.iscriminal() or target.ismurderer():
			return
		
		# If Enemy Guild then return
		if source.guildcompare( target ) == 2:
			return

		return

	# If it's not agressive, it's beneficial
	else:
		return		

def magicdamage( basedamage, spell, mage, defender ):
	#DIRECT DAMAGE SPELLS:	
	# Information found on uo.stratics.com
	# (Including: magic arrow, harm, fireball, lightning, energy bolt, explosion, chain lightning, meteor swarm, flamestrike, and Earthquake)	
	# Actual_Damage_done = (Base_Spell_Damage - 50% [if Resisted]) + Eval_Intelligence_Damage_Modifier
	# Figuring your chance to resist a spell is done as follows:
	# Your chance to resist a spell is the greater of either:
	# Your_Chance_to_Resist = Your_Resistance / 5
	# Your resistance skill minus the sum of (your attackers magery skill (less 20 points) divided by five) and (the circle of the spell multiplied by 5)
	# Your_Chance_to_Resist = Your_Resistance - (( Attackers_Magery - 20 )/5 + Spell_Circle*5))

	circle = 1 + floor( spell / 8 )

	# Calculate the resistances
	chance1 = floor( defender.baseskill[ MAGICRESISTANCE ] / 5 )
	chance2 = floor( defender.baseskill[ MAGICRESISTANCE ] - ( mage.baseskill[ MAGERY ] / 5 + circle * 5 ) )

	chance = max( [ chance1, chance2 ] ) # The higher chance is used

	whrandom.seed()

	damage = basedamage

	# Check succeeded
	if whrandom.randrange( 0, 99 ) < chance:
		damage /= 2

	#The remaining damage is multiplied against a small equation:
	# If your resistance is higher than your opponents evaluate intelligence, then the equation is:
	#(1 + (Attackers_EI - Your_resistance) / 200 )
	# If your resistance is lower than your opponents evaluate intelligence, then the equation is:
	#(1 + (Attackers_EI - Your_resistance) / 500 )
	if( defender.baseskill[ MAGICRESISTANCE ] > mage.baseskill[ EVALUATINGINTEL ] ):
		damage = damage * ( 1 + ( mage.baseskill[ EVALUATINGINTEL ] - defender.baseskill[ MAGICRESISTANCE ] ) / 200 ) )
	else:
		damage = damage * ( 1 + ( mage.baseskill[ EVALUATINGINTEL ] - defender.baseskill[ MAGICRESISTANCE ] ) / 500 ) )

	return damage

# Shows Fizzle Animation + Sound
def fizzle( char ):
	char.effect( 0x3735, 1, 30 )
	char.soundeffect( 0x5c )

# Stat Modifier (Bless, Curse, etc.)

def statmodifier_dispel( char, args, source, dispelargs ):
	# Normally reduce "if not 'silent' in dispelargs"
	char.str -= args[0]
	char.dex -= args[1]
	char.int -= args[2]

	if not "silent" in dispelargs:
		# If Str changed, resend to others
		if args[0] != 0:
			char.updatehealth()
		char.updatestats()

	return

def statmodifier_expire( char, args ):
	# The stat modifier normally expired, so just reduce the stats and resend
	char.str -= args[0]
	char.dex -= args[1]
	char.int -= args[2]

	# If Str changed, resend to others
	if args[0] != 0:
		char.updatehealth()
	char.updatestats()

	return

def statmodifier( char, time, mStr, mDex, mInt ):
	# Dispel any old stat modifiers on this character
	# And readd a new one (remove the old ones silently)
	char.dispel( char, 0, "statmodifier", [ "silent" ] )

	char.str += mStr
	char.dex += mDex
	char.int += mInt

	if mStr != 0:
		char.updatehealth()

	char.updatestats()

	# Save the values in a tempeffect to remove them later
	char.addtimer( time, "wolfpack.utilities.statmodifier_expire", [ mStr, mDex, mInt ], 1, 1, "statmodifier", "wolfpack.utilities.statmodifier_dispel" )
