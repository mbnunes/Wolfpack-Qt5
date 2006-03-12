
import wolfpack
from wolfpack.consts import *

# [[bodyids of corpses], id of trophy, trophy deed name, trophy name
corpses = {
"orc" : [[ "7", "17", "41", "138", "139", "140", "181", "182", "189" ], 0x1e64, 1041090, 1041104 ],
"gorilla" : [[ "29" ], 0x1e63, 1041091, 1041105 ],
"troll" : [[ "53", "54", "55" ], 0x1e66, 1041092, 1041106 ],
"brown_bear" : [[ "167" ], 0x1e60, 1041093, 1041107 ],
"polar_bear" : [[ "213" ], 0x1e65, 1041094, 1041108 ],
"great_hart" : [[ "234" ], 0x1e61, 1041095, 1041109 ],
"big_fish" : [[ "234" ], 0x1e62, 1041096, 1041110 ]
}

def VisitedByTaxidermist(item):
	if item.hastag( "VisitedByTaxidermist" ):
		return True
	return False

def onUse( char, item ):
	if not item.getoutmostchar() == char:
		char.socket.clilocmessage( 1042001 ) # That must be in your pack for you to use it.
		return False
	elif char.skill[CARPENTRY] < 900:
		char.socket.clilocmessage( 1042594 ) # You do not understand how to use this.
		return False
	else:
		char.socket.clilocmessage( 1042595 ) # Target the corpse to make a trophy out of.
		char.socket.attachtarget( "taxidermykit.response", [item.serial] )
	return True

def response( char, args, target):
	kit = wolfpack.finditem( args[0] )
	if not kit:
		return False
	if not char.canreach(target.pos, 3):
		char.socket.clilocmessage( 500295 ) # You are too far away to do that.
		return False
	if not target.item and not target.item.corpse and not baseid == "big_fish":
		char.socket.clilocmessage( 1042600 ) # That is not a corpse!
	elif target.item.corpse and VisitedByTaxidermist(target.item):
		char.socket.clilocmessage( 1042596 ) # That corpse seems to have been visited by a taxidermist already.
	else:
		if target.item.baseid == "big_fish":
			createTrophyDeed( char, 'big_fish', target.item )
			target.item.delete()
			kit.delete()

		elif getname( target.item.bodyid ) in corpses.keys():
			corpse = corpses[getname( target.item.bodyid )]
			if not wolfpack.utilities.checkresources(char.getbackpack(), '1bdd', 10):

				createTrophyDeed( char, getname( target.item.bodyid ), target.item )

				target.item.settag( 'VisitedByTaxidermist', 0 )
				kit.delete()
			else:
				char.socket.clilocmessage( 1042598 ) # You do not have enough boards.
		else:
			char.socket.clilocmessage( 1042599 ) # That does not look like something you want hanging on a wall.
		return

def getname( bodyid ):
	for corpse in corpses.keys():
		name = corpses[corpse][0]
		if str(bodyid) in name:
			return corpse
	return False

def createTrophyDeed( char, corpse, item, message = True ):
	wolfpack.utilities.consumeresources(char.getbackpack(), '1bdd', 10)
	if message:
		char.socket.clilocmessage( 1042278 ) # You review the corpse and find it worthy of a trophy.
		char.socket.clilocmessage( 1042602 ) # You use your kit up making the trophy.

	trophydeed = wolfpack.additem('trophy_deed')
	trophydeed.settag('corpse', corpse)
	trophydeed.name = '#' + str(corpses[corpse][2])
	if item.hastag('hunter'):
		hunter = item.gettag('hunter')
		trophydeed.settag('hunter', hunter)
	if item.hastag('animalweight'):
		weight = item.gettag('animalweight')
		trophydeed.settag('animalweight', weight)

	char.getbackpack().additem(trophydeed)
	trophydeed.update()
