import wolfpack
from wolfpack.consts import *

#SKILLID, STRGAIN, DEXGAIN, INTGAIN, ANTIMACRO
skilltable = \
{
ALCHEMY: 	 [0,	0.5,	0.5,	FALSE],
ANATOMY: 	 [0.15,	0.5,	0.5,	TRUE ],
ANIMALLORE: 	 [0,	0.5,	0.5,	TRUE ], 
ITEMID: 	 [0,	0.5,	0.5,	TRUE ],
ARMSLORE: 	 [0.75,	0.5,	0.5,	TRUE ],
PARRYING: 	 [0.75,	0.5,	0.5,	FALSE],
BEGGING: 	 [0,	0.5,	0.5,	TRUE ],
BLACKSMITHING:	 [1,	0.5,	0.5,	FALSE],
BOWCRAFT: 	 [0.6,	0.5,	0.5,	FALSE],
PEACEMAKING: 	 [0,	0.5,	0.5,	TRUE ],
CAMPING: 	 [2,	0.5,	0.5,	TRUE ],
CARPENTRY: 	 [2,	0.5,	0.5,	FALSE],
CARTOGRAPHY: 	 [0, 	0.5,	0.5,	FALSE],
COOKING: 	 [0,	0.5,	0.5,	FALSE],
DETECTINGHIDDEN: [0,	0.5,	0.5,	TRUE ],
ENTICEMENT: 	 [0,	0.5,	0.5,	TRUE ],
EVALUATINGINTEL: [0,	0.5,	0.5,	TRUE ],
HEALING: 	 [0.6,	0.5,	0.5,	TRUE ],
FISHING: 	 [0.5,	0.5,	0.5,	TRUE ],
FORENSICS: 	 [0,	0.5,	0.5,	TRUE ],
HERDING: 	 [1.625,0.5,	0.5,	TRUE ],
HIDING:	 	 [0,	0.5,	0.5,	TRUE ],
PROVOCATION: 	 [0,	0.5,	0.5,	TRUE ],
INSCRIPTION: 	 [0,	0.5,	0.5,	FALSE],
LOCKPICKING: 	 [0,	0.5,	0.5,	TRUE ],
MAGERY:		 [0,	0.5,	0.5,	TRUE ],
MAGICRESISTANCE: [0.25,	0.5,	0.5,	TRUE ],
TACTICS: 	 [0,	0.5,	0.5,	FALSE],
SNOOPING: 	 [0,	0.5,	0.5,	TRUE ],
MUSICIANSHIP: 	 [0,	0.5,	0.5,	TRUE ],
POISONING: 	 [0,	0.5,	0.5,	TRUE ],
ARCHERY: 	 [0.25,	0.5,	0.5,	FALSE],
SPIRITSPEAK: 	 [0,	0.5,	0.5,	TRUE ],
STEALING: 	 [0,	0.5,	0.5,	TRUE ],
TAILORING: 	 [0.38,	0.5,	0.5,	FALSE],
TAMING:	 	 [1.4,	0.5,	0.5,	TRUE ],
TASTEID: 	 [0.2,	0.5,	0.5,	TRUE ],
TINKERING: 	 [0.5,	0.5,	0.5,	FALSE],
TRACKING: 	 [0,	0.5,	0.5,	TRUE ],
VETERINARY: 	 [0.8,	0.5,	0.5,	TRUE ],
SWORDSMANSHIP:   [0.75,	0.5,	0.5,	FALSE],
MACEFIGHTING: 	 [0.9,	0.5,	0.5,	FALSE],
FENCING: 	 [0.45,	0.5,	0.5,	FALSE],
WRESTLING: 	 [0.9,	0.5,	0.5,	FALSE],
LUMBERJACKING:   [2,	0.5,	0.5,	TRUE ],
MINING: 	 [2,	0.5,	0.5,	TRUE ],
MEDITATION: 	 [0,	0.5,	0.5,	TRUE ],
STEALTH: 	 [0,	0.5,	0.5,	TRUE ],
REMOVETRAPS: 	 [0,	0.5,	0.5,	TRUE ],
NECROMANCY: 	 [0,	0.5,	0.5,	TRUE ],
FOCUS: 	 	 [0,	0.5,	0.5,	FALSE],
CHIVALRY: 	 [0,	0.5,	0.5,	TRUE ]                
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
        gem.settag( 'resourcecount', gem.gettag( 'resourcecount' ) - amount )

	backpack.additem( resourceitem )

	socket.clilocmessage( table[ resname ][ SUCCESSCLILOC ], "", YELLOW, NORMAL )
	return OK
