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
UNHIDE = 6

#SKILLID, STRGAIN, DEXGAIN, INTGAIN, GAINFACTOR, SKILLCAP, ANTIMACRO, UNHIDE
skilltable = {
	ALCHEMY:					[ 0, 0.5, 0.5, 1, 1000, False, False ],
	ANATOMY:					[ 0.1, 0.1, 0.8, 1, 1000, False, False ],
	ANIMALLORE:				[ 0, 0, 1, 1, 1000, False, False ],
	ITEMID:						[ 0, 0, 1, 1, 1000, False, False ],
	ARMSLORE:					[ 0.8, 0.1, 0.1, 1, 1000, False, False ],
	PARRYING:					[ 0.75, 0.25, 0, 1, 1000, False, True ],
	BEGGING:					[ 0, 0, 0, 1, 1000, False, True ],
	BLACKSMITHING:		[ 1, 0, 0, 1, 1000, False, False ],
	BOWCRAFT:					[ 0.6, 1.6, 0, 1, 1000, False, False ],
	PEACEMAKING:			[ 0, 0, 0, 1, 1000, False, True ],
	CAMPING:					[ 2, 1.5, 1.5, 1, 1000, False, False ],
	CARPENTRY:				[ 2, 0.5, 0, 1, 1000, False, False ],
	CARTOGRAPHY:			[ 0, 0.75, 0.75, 1, 1000, False, False ],
	COOKING:					[ 0, 2, 3, 1, 1000, False, False ],
	DETECTINGHIDDEN:	[ 0, 0.4, 0.6, 1, 1000, False, False ],
	ENTICEMENT:				[ 0, 0.25, 0.25, 1, 1000, False, True ],
	EVALUATINGINTEL:	[ 0, 0, 1, 1, 1000, False, False ],
	HEALING:					[ 0.6, 0.6, 0.8, 1, 1000, False, True ],
	FISHING:					[ 0.5, 0.5, 0, 1, 1000, False, False ],
	FORENSICS:				[ 0, 0.2, 0.8, 1, 1000, False, False ],
	HERDING:					[ 1.625, 0.625, 0.25, 1, 1000, False, True ],
	HIDING:						[ 0, 0.8, 0.2, 1, 1000, False, True ],
	PROVOCATION:			[ 0, 0.45, 0.05, 1, 1000, False, True ],
	INSCRIPTION:			[ 0, 0.2, 0.8, 1, 1000, False, False ],
	LOCKPICKING:			[ 0, 2, 0, 1, 1000, False, True ],
	MAGERY:		 				[ 0, 0, 1.5, 1, 1000, False, True ],
	MAGICRESISTANCE:	[ 0.25, 0.25, 0.5, 1, 1000, False, False ],
	TACTICS:					[ 0, 0, 0, 1, 1000, False, True ],
	SNOOPING:					[ 0, 2.5, 0, 1, 1000, False, False ],
	MUSICIANSHIP:			[ 0, 0.8, 0.2, 1, 1000, False, False ],
	POISONING:				[ 0, 0.4, 1.6, 1, 1000, False, False ],
	ARCHERY:					[ 0.25, 0.75, 0, 1, 1000, False, True ],
	SPIRITSPEAK:			[ 0, 0, 1, 1, 1000, False, True ],
	STEALING:					[ 0, 1, 0, 1, 1000, False, True ],
	TAILORING:				[ 0.38, 1.63, 0.5, 1, 1000, False, False ],
	TAMING:	 	 				[ 1.4, 0.2, 0.4, 1, 1000, False, True ],
	TASTEID:					[ 0.2, 0, 0.8, 1, 1000, False, False ],
	TINKERING:				[ 0.5, 0.2, 0.3, 1, 1000, False, False ],
	TRACKING:					[ 0, 1.25, 1.25, 1, 1000, False, False ],
	VETERINARY:				[ 0.8, 0.4, 0,8,	1, 1000, False, True ],
	SWORDSMANSHIP:		[ 0.75, 0.25, 0, 1, 1000, False, True ],
	MACEFIGHTING:			[ 0.9, 0.1, 0, 1, 1000, False, True ],
	FENCING:					[ 0.45, 0.55, 0, 1, 1000, False, True ],
	WRESTLING:				[ 0.9, 0.1, 0, 1, 1000, False, True ],
	LUMBERJACKING:		[ 2, 0, 0, 1, 1000, False, False ],
	MINING:	 					[ 2, 0, 0, 1, 1000, False, False ],
	MEDITATION: 			[ 0, 0, 0, 1, 1000, False, True ],
	STEALTH:					[ 0, 0, 0, 1, 1000, False, False ],
	REMOVETRAPS:			[ 0, 0, 0, 1, 1000, False, True ],
	NECROMANCY:				[ 0, 0, 0, 1, 1000, False, True ],
	FOCUS:						[ 0, 0, 0, 1, 1000, False, True ],
	CHIVALRY:					[ 0, 0, 0, 1, 1000, False, True ],
	NINJITSU:	 				[ 0, 0, 0, 1, 1000, False, True],
	BUSHIDO:					[ 0, 0, 0, 1, 1000, False, True ],
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
