import wolfpack
from wolfpack.consts import *

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
        gem.amount = gem.amount - amount

	backpack.additem( resourceitem )

	socket.clilocmessage( table[ resname ][ SUCCESSCLILOC ], "", YELLOW, NORMAL )
	return OK
