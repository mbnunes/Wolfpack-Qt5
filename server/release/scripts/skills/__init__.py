import wolfpack
import random
import wolfpack.settings
from wolfpack.consts import *

STRGAIN = 0
DEXGAIN = 1
INTGAIN = 2
GAINFACTOR = 3
SKILLCAP = 4
ANTIMACRO = 5

#SKILLID, STRGAIN, DEXGAIN, INTGAIN, GAINFACTOR, SKILLCAP, ANTIMACRO
skilltable = {
	ALCHEMY:					[ 0, 0.5, 0.5, 1, 1000, FALSE ],
	ANATOMY:					[ 0.1, 0.1, 0.8, 1, 1000, FALSE ],
	ANIMALLORE:				[ 0, 0, 1, 1, 1000, FALSE ],
	ITEMID:						[ 0, 0, 1, 1, 1000, FALSE ],
	ARMSLORE:				[ 0.8, 0.1, 0.1, 1, 1000, FALSE ],
	PARRYING:				[ 0.75, 0.25, 0, 1, 1000, FALSE],
	BEGGING:					[ 0, 0, 0, 1, 1000, FALSE ],
	BLACKSMITHING:		[ 1, 0, 0, 1, 1000, FALSE],
	BOWCRAFT:				[ 0.6, 1.6, 0, 1, 1000, FALSE],
	PEACEMAKING:			[ 0, 0, 0, 1, 1000, FALSE ],
	CAMPING:					[ 2, 1.5, 1.5, 1, 1000, FALSE ],
	CARPENTRY:				[ 2, 0.5, 0, 1, 1000, FALSE],
	CARTOGRAPHY:			[ 0, 0.75, 0.75, 1, 1000, FALSE],
	COOKING:					[ 0, 2, 3, 1, 1000, FALSE],
	DETECTINGHIDDEN:	[ 0, 0.4, 0.6, 1, 1000, FALSE ],
	ENTICEMENT:				[ 0, 0.25, 0.25, 1, 1000, FALSE ],
	EVALUATINGINTEL:	[ 0, 0, 1, 1, 1000, FALSE ],
	HEALING:					[ 0.6, 0.6, 0.8, 1, 1000, FALSE ],
	FISHING:					[ 0.5, 0.5, 0, 1, 1000, FALSE ],
	FORENSICS:				[ 0, 0.2, 0.8, 1, 1000, FALSE ],
	HERDING:					[ 1.625, 0.625, 0.25, 1, 1000, FALSE ],
	HIDING:					[ 0, 0.8, 0.2, 1, 1000, FALSE ],
	PROVOCATION:			[ 0, 0.45, 0.05, 1, 1000, FALSE ],
	INSCRIPTION:			[ 0, 0.2, 0.8, 1, 1000, FALSE],
	LOCKPICKING:			[ 0, 2, 0, 1, 1000, FALSE ],
	MAGERY:		 			[ 0, 0, 1.5, 1, 1000, FALSE ],
	MAGICRESISTANCE:	[ 0.25, 0.25, 0.5, 1, 1000, FALSE ],
	TACTICS:	 				[ 0, 0, 0, 1, 1000, FALSE],
	SNOOPING:				[ 0, 2.5, 0, 1, 1000, FALSE ],
	MUSICIANSHIP:			[ 0, 0.8, 0.2, 1, 1000, FALSE ],
	POISONING:	 			[ 0, 0.4, 1.6, 1, 1000, FALSE ],
	ARCHERY:					[ 0.25, 0.75, 0, 1, 1000, FALSE],
	SPIRITSPEAK:			[ 0, 0, 1, 1, 1000, FALSE ],
	STEALING:					[ 0, 1, 0, 1, 1000, FALSE ],
	TAILORING:				[ 0.38, 1.63, 0.5, 1, 1000, FALSE ],
	TAMING:	 	 			[ 1.4, 0.2, 0.4, 1, 1000, FALSE ],
	TASTEID:					[ 0.2, 0, 0.8, 1, 1000, FALSE ],
	TINKERING:				[ 0.5, 0.2, 0.3, 1, 1000, FALSE ],
	TRACKING:				[ 0, 1.25, 1.25, 1, 1000, FALSE ],
	VETERINARY:				[ 0.8, 0.4, 0,8,	1, 1000, FALSE ],
	SWORDSMANSHIP:		[ 0.75, 0.25, 0, 1, 1000, FALSE ],
	MACEFIGHTING:			[ 0.9, 0.1, 0, 1, 1000, FALSE ],
	FENCING:	 				[ 0.45, 0.55, 0, 1, 1000, FALSE ],
	WRESTLING:	 			[ 0.9, 0.1, 0, 1, 1000, FALSE ],
	LUMBERJACKING:		[ 2, 0, 0, 1, 1000, FALSE ],
	MINING:	 				[ 2, 0, 0, 1, 1000, FALSE ],
	MEDITATION: 			[ 0, 0, 0, 1, 1000, FALSE ],
	STEALTH:					[ 0, 0, 0, 1, 1000, FALSE ],
	REMOVETRAPS:			[ 0, 0, 0, 1, 1000, FALSE ],
	NECROMANCY:			[ 0, 0, 0, 1, 1000, FALSE ],
	FOCUS:	 					[ 0, 0, 0, 1, 1000, FALSE],
	CHIVALRY:				[ 0, 0, 0, 1, 1000, FALSE ]
}

skills = {}

def register( id, handler ):
	if skills.has_key( id ):
		raise Exception, "Duplicate Skill: " + str( id )

	skills[ id ] = handler

def onLoad():
	wolfpack.registerglobal( EVENT_SKILLUSE, "skills" )

def onSkillUse( char, skill ):
	if skills.has_key( skill ):
		skills[ skill ]( char, skill )
		return True

	return False

#
# Check if a certain chance can be met using the skill
#
def checkskill(char, skill, chance):
  # Normalize
  chance = min(1.0, max(0.02, chance))
  
  minskill = (1.0 - chance) * 1200
  maxskill = 1200
  char.checkskill(skill, minskill, maxskill)
  result = chance >= random.random()
  return result

def totalstats( char ):
	return char.strength + char.dexterity + char.intelligence

def cleartag( self, args ):
	char = wolfpack.findchar(args[0])
	tagname = args[1]
	self.deltag( tagname )
	return True

def antimacrocheck( char, skillid, object ):
	#Get or set antimacro tag: "AM" + SERIAL = COUNT
	tagname = "AMC_%i_%i" % ( char.serial, skillid )
	if object.hastag( tagname ):
		count = object.gettag( tagname )
		object.settag( tagname, int( count + 1 ) )
		if count <= ANTIMACROALLOWANCE:
			return True
		elif count > ANTIMACROALLOWANCE + 1:
			return False
		else:
			object.addtimer( ANTIMACRODELAY, "skills.cleartag", [char.serial, tagname] )
			return False
	else:
		object.settag( tagname, "1" )
		return True
