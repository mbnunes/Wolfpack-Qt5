#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: Correa                         #
#   )).-' {{ ;'`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification:                         #
#################################################################

import wolfpack.settings
import skills
from wolfpack.consts import *

MAX_TEACHING = 300
MIN_TEACHING = 20

#	  onContextCheckVisible
def onContextCheckVisible( char, target, tag ):
	if (target.skill[tag - 1] >= 300):
		return 1 # visible

	return 0 # not visible

def onContextCheckEnabled( char, target, tag ):
	skill = tag - 1
	if ( skill == STEALTH and char.skill[ HIDING ] < 800 ):
		return 0 # disabled
	if ( skill == REMOVETRAPS and ( char.skill[ LOCKPICKING ] < 500 or char.skill[ DETECTINGHIDDEN ] < 500 ) ):
		return 0 # disabled
	if ( char.skill[ skill ] >= MAX_TEACHING ):
		return 0 # disabled
	else:
		return 1 #enabled

def onContextEntry( char, target, tag  ):

	skill = tag - 1
	if ( char.dead or skill < 0 ):
		return 1

	baseToSet = target.skill[ skill ] / ( 1000 / MAX_TEACHING )
	if ( baseToSet > MAX_TEACHING ):
		baseToSet = MAX_TEACHING
	elif ( baseToSet < MIN_TEACHING ):
		target.say( str( baseToSet ) )
		return 1
	pointsToLearn = baseToSet - char.skill[ skill ]
	if ( pointsToLearn < 0 ): # Player knows more than me
		return 1

	target.say( 1019077, args = "", affix = " " + str( pointsToLearn*10 ), prepend = 0, socket = char.socket ) # I will teach thee all I know, if paid the amount in full.  The price is:
	target.say( 1043108, socket = char.socket ) #For less I shall teach thee less.
	char.settag("npctrainer", str( target.serial ) )
	char.settag("trainningskill", str( skill ) )
	return 1

def onDropOnChar( char, item ):

	if ( item.id != 0xeed ):
		return 0 # not what we expected :(

	dropper = item.container
	if ( dropper.gettag("npctrainer") == str( char.serial ) and dropper.hastag("trainningskill") ):
		skill = int( dropper.gettag("trainningskill") )
		dropper.deltag("npctrainer")
		dropper.deltag("trainningskill")
		amount = item.amount / 10

		if ( amount > MAX_TEACHING ):
			amount = MAX_TEACHING

		# check for skill cap
		cap = wolfpack.settings.getNumber("General", "SkillCap", 700) * 10

		sum = skills.totalskills( dropper )

		if ( sum >= cap ):
			return 0

		baseToSet = char.skill[ skill ] / ( 1000 / MAX_TEACHING )
		if ( baseToSet > MAX_TEACHING ):
			baseToSet = MAX_TEACHING
		elif ( baseToSet < MIN_TEACHING ):
			char.say( str( baseToSet ) )
			return 0

		if ( baseToSet + sum > cap ):
			baseToSet = cap - sum

		pointsToLearn = baseToSet - dropper.skill[ skill ]
		if ( pointsToLearn < 0 ): # Player knows more than me
			return 0

		if ( amount > pointsToLearn ):
			amount = pointsToLearn
			item.amount = item.amount - amount * 10
		else:
			item.delete()

		dropper.skill[skill] = dropper.skill[skill] + amount

		char.say( 501539 ) # Let me show thee something of how this is done.
		return 1
	return 0


