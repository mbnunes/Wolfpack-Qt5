import time
import wolfpack
import whrandom
import wolfpack.settings
from wolfpack.consts import *

STRGAIN = 0
DEXGAIN = 1
INTGAIN = 2
GAINFACTOR = 3
SKILLCAP = 4
ANTIMACRO = 5

#SKILLID, STRGAIN, DEXGAIN, INTGAIN, GAINFACTOR, SKILLCAP, ANTIMACRO
skilltable = \
{

ALCHEMY:					[0, 0.5, 0.5, 1, 1000, FALSE],
ANATOMY:					[0.15, 0.15, 0.7, 1, 1000, FALSE ],
ANIMALLORE:				[0, 0, 1, 1, 1000, FALSE ], 
ITEMID:						[0, 0, 1, 1, 1000, FALSE ],
ARMSLORE:				[0.75, 0.15, 0.1, 1, 1000, FALSE ],
PARRYING:				[0.75, 0.25, 0, 1, 1000, FALSE],
BEGGING:					[0, 0, 0, 1, 1000, FALSE ],
BLACKSMITHING:		[1, 0, 0, 1, 1000, FALSE],
BOWCRAFT:				[0.6, 1.6, 0, 1, 1000, FALSE],
PEACEMAKING:			[0, 0, 0, 1, 1000, FALSE ],
CAMPING:					[2, 1.5, 1.5, 1, 1000, FALSE ],
CARPENTRY:				[2, 0.5, 0, 1, 1000, FALSE],
CARTOGRAPHY:			[0, 0.75, 0.75, 1, 1000, FALSE],
COOKING:					[0, 2, 3, 1, 1000, FALSE],
DETECTINGHIDDEN:	[0, 0.4, 0.6, 1, 1000, FALSE ],
ENTICEMENT:				[0, 0.25, 0.25, 1, 1000, FALSE ],
EVALUATINGINTEL:	[0, 0, 1, 1, 1000, FALSE ],
HEALING:					[0.6, 0.6, 0.8, 1, 1000, FALSE ],
FISHING:					[0.5, 0.5, 0, 1, 1000, FALSE ],
FORENSICS:				[0, 0.2, 0.8, 1, 1000, FALSE ],
HERDING:					[1.625, 0.625, 0.25, 1, 1000, FALSE ],
HIDING:					[0, 0.8, 0.2, 1, 1000, FALSE ],
PROVOCATION:			[0, 0.45, 0.05, 1, 1000, FALSE ],
INSCRIPTION:			[0, 0.2, 0.8, 1, 1000, FALSE],
LOCKPICKING:			[0, 2, 0, 1, 1000, FALSE ],
MAGERY:		 			[0, 0, 1.5, 1, 1000, FALSE ],
MAGICRESISTANCE:	[0.25, 0.25, 0.5, 1, 1000, FALSE ],
TACTICS:	 				[0, 0, 0, 1, 1000, FALSE],
SNOOPING:				[0, 2.5, 0, 1, 1000, FALSE ],
MUSICIANSHIP:			[0, 0.8, 0.2, 1, 1000, FALSE ],
POISONING:	 			[0, 0.4, 1.6, 1, 1000, FALSE ],
ARCHERY:					[0.25, 0.75, 0, 1, 1000, FALSE],
SPIRITSPEAK:			[0, 0, 1, 1, 1000, FALSE ],
STEALING:					[0, 1, 0, 1, 1000, FALSE ],
TAILORING:				[0.38, 1.63, 0.5, 1, 1000, FALSE],
TAMING:	 	 			[1.4, 0.2, 0.4, 1, 1000, FALSE ],
TASTEID:					[0.2, 0, 0.8, 1, 1000, FALSE ],
TINKERING:				[0.5, 0.2, 0.3, 1, 1000, FALSE],
TRACKING:				[0, 1.25, 1.25, 1, 1000, FALSE ],
VETERINARY:				[0.8, 0.4, 0,8,	1, 1000, FALSE ],
SWORDSMANSHIP:		[0.75, 0.25, 0, 1, 1000, FALSE],
MACEFIGHTING:			[0.9, 0.1, 0, 1, 1000, FALSE],
FENCING:	 				[0.45, 0.55, 0, 1, 1000, FALSE],
WRESTLING:	 			[0.9, 0.1, 0, 1, 1000, FALSE],
LUMBERJACKING:		[2, 0, 0, 1, 1000, FALSE ],
MINING:	 				[2, 0, 0, 1, 1000, FALSE ],
MEDITATION: 			[0, 0, 0, 1, 1000, FALSE ],
STEALTH:					[0, 0, 0, 1, 1000, FALSE ],
REMOVETRAPS:			[0, 0, 0, 1, 1000, FALSE ],
NECROMANCY:			[0, 0, 0, 1, 1000, FALSE ],
FOCUS:	 					[0, 0, 0, 1, 1000, FALSE],
CHIVALRY:				[0, 0, 0, 1, 1000, FALSE ]                
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
		return 1
		
	return 0

# table structure - look mining.py
def successharvest( char, gem, table, resname, amount ):
	socket = char.socket
	message = table[ resname ][ SUCCESSCLILOC ]
	backpack = char.getbackpack()
	if not backpack:
		return OOPS
	resourceitem = wolfpack.newitem( 1 ) # Get new serial
	resourceitem.decay = 1
	resourceitem.color = table[ resname ][ COLORID ]
	resourceitem.id = table[ resname ][ RESOURCEID ]
	resourceitem.amount = amount
        gem.settag( 'resourcecount', str( int( gem.gettag( 'resourcecount' ) ) - amount ) )

	backpack.additem( resourceitem )

	socket.clilocmessage( table[ resname ][ SUCCESSCLILOC ], "", GREEN, NORMAL )
	return OK


def checkskill( char, targetobject, skillid, chance ):
	if char.dead:
		return OOPS
	skillvalue = char.skill[ skillid ]
	skillscap = wolfpack.settings.getNumber( "General", "SkillCap", 700 ) * 10
	charskillcap = char.skillcap[ skillid ]
	chartotalskills = totalskills( char )

	if chance >= whrandom.random():
		mult = 0.5
	else:
		mult = 0.2

	gainchance = float( skillscap - chartotalskills ) / skillscap
	gainchance += float( charskillcap - skillvalue ) / charskillcap / 2
	gainchance = ( ( gainchance + ( 1.0 - chance ) * mult ) / 2 ) * skilltable[ skillid ][ GAINFACTOR ]

	if gainchance < 0.01:
		gainchance = 0.01
	
	#If you lucky and antimacro is agree so let's gain this skill
	if ( gainchance >= whrandom.random() ) or skillvalue < 100:
		skillgain( char, skillid )

	return OK


def skillgain( char, skillid ):
	skillvalue = char.skill[ skillid ]
	charskillcap = char.skillcap[ skillid ]
	skillscap = wolfpack.settings.getNumber( "General", "SkillCap", 700 ) * 10
	charskilllock = char.skilllock[ skillid ]
	chartotalskills = totalskills( char )

	#Skill growing
	if skillvalue < charskillcap and charskilllock == GROWUP:
		#Base skill value < 10.0 ?
		toGain = 1
		if skillvalue <= 100:
			toGain = whrandom.randint( 1, 5 )

		#Pull down skill which is marked as GROWDOWN
		if ( chartotalskills / skillscap ) >= whrandom.random():
			for i in range( 0, ALLSKILLS ):
				if char.skilllock[ i ] == GROWDOWN and skillvalue > toGain:
					char.skill[ i ] = char.skill[ i ] - toGain
					break
	
		if chartotalskills + toGain <= skillscap:
			char.skill[ skillid ] = char.skill[ skillid ] + toGain

	#Stats growing
	if charskilllock == GROWUP:
		if float( skilltable[ skillid ][ STRGAIN ] ) / 33.3 > whrandom.random():
			statgain( char, STRENGTH )
		elif float( skilltable[ skillid ][ DEXGAIN ] ) / 33.3 > whrandom.random():
			statgain( char, DEXTERITY )
		elif float( skilltable[ skillid ][ INTGAIN ] ) / 33.3 > whrandom.random():
			statgain( char, INTELLIGENCE )


	return OK


def statgain( char, stat ):
	#STATGAINDELAY is not over ?
	if char.hastag( 'laststatgain' ) and ( time.time() - int(char.gettag( 'laststatgain' ) ) ) < STATGAINDELAY:
		return OOPS
	
	char.settag( 'laststatgain', str( time.time() ) )
	if float( totalstats( char ) ) / wolfpack.settings.getNumber( "General", "StatCap", 225 ) >= whrandom.random():
		if stat == STRENGTH:
			char.strength = char.strength + 1
		elif stat == DEXTERITY:
			char.dexterity = char.dexterity + 1
		elif stat == INTELLIGENCE:
			char.intelligence = char.intelligence + 1
			
	return OK

def totalskills( char ):
	total = 0
	for i in range( 0, ALLSKILLS ):
		total += char.skill[ i ]
	return total

def totalstats( char ):
	return char.strength + char.dexterity + char.intelligence

def antimacrocheck( char, skillid, object ):
	#Get or set antimacro tag: "AM" + SERIAL = COUNT
	tagname = "AMC_" + str( char.serial ) + " " + str( skillid )
	if object.hastag( tagname ):
		count = object.gettag( tagname )
		object.settag( tagname, str( int( count + 1 ) ) )
		if count <= ANTIMACROALLOWANCE:
			return OK
		elif count > ANTIMACROALLOWANCE + 1:
			return OOPS
		else:
			object.addtimer( ANTIMACRODELAY, "wolfpack.utilities.cleartag", [char, tagname] )
			return OOPS
	else:
		object.settag( tagname, "1" )
		return OK


