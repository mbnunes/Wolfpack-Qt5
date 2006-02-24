import wolfpack
import deeds.carpentry_deed
from taxidermykit import corpses
from wolfpack.utilities import hex2dec

def onShowTooltip(player, object, tooltip):
	if object.hastag('animalweight') and object.gettag('animalweight') >= 20:
		if object.hastag('hunter'):
			serial = int(object.gettag('hunter'))
			hunter = wolfpack.findchar(serial)
			if hunter:
				tooltip.add( 1070857, hunter.orgname ) # Caught by ~1_fisherman~
		tooltip.add( 1070858, str(object.gettag('animalweight')) ) # ~1_weight~ stones

def onUse(char, item):
	if not item.getoutmostchar() == char:
		char.socket.clilocmessage( 1042001 ) # That must be in your pack for you to use it.
		return True

	if not deeds.carpentry_deed.checkmulti(char):
		char.socket.clilocmessage( 502092 ) # You must be in your house to do this.
		return True

	northWall = wolfpack.utilities.isWall( char.pos.x, char.pos.y - 1, char.pos.z, char.pos.map )
	westWall = wolfpack.utilities.isWall( char.pos.x - 1, char.pos.y, char.pos.z, char.pos.map )

	if northWall and westWall:
		dir = char.direction
		if dir == 0: # north
			char.socket.sysmessage("n")
			northWall = True
			westWall = False
		elif dir == 6: # west
			northWall = False
			westWall = True
		else:
			char.socket.sysmessage('Turn to face the wall on which to hang this trophy.')
			return True

	corpse = item.gettag('corpse')
	itemID = 0
	if northWall:
		itemID = hex(corpses[corpse][1]).split('x')[1]
	elif westWall:
		itemID = hex(corpses[corpse][1]+0x7).split('x')[1]
	else:
		char.socket.clilocmessage( 1042626 ) # The trophy must be placed next to a wall.

	if itemID > 0:
		if setup_trophy(char, itemID):
			item.delete()
		else:
			char.socket.clilocmessage( 500269 ) # You cannot build that there.

		# house.Addons.Add( new TrophyAddon( from, itemID, m_WestID, m_NorthID, m_DeedNumber, m_AddonNumber, m_Hunter, m_AnimalWeight ) );
	return True

def setup_trophy(char, itemID):
	if check_spot(char):
		trophy = wolfpack.additem(itemID)
		trophy.pos = char.pos
		trophy.update()
		return True
	return False

def check_spot( char ):
	x = char.pos.x
	y = char.pos.y
	z = char.pos.z
	map = char.pos.map

	# We can build on floor
	# Check for dynamic items
	tile = wolfpack.items( x, y, map )
	if tile:
		dynamic_floor = wolfpack.tiledata(tile[0].id)["floor"]
		if len( tile ) and not dynamic_floor:
			return False

	# Check for dynamic items
	tile = wolfpack.statics(x, y, map, True)
	if tile:
		for i in tile:
			height = i[3] > z and i[3] < (z+20) # something is between the ground and +20?
			static_floor = wolfpack.tiledata(i[0])["floor"]
			static_roof = wolfpack.tiledata(i[0])["flag1"] & 0x10
			if len( tile ) and height: #and and not static_floor and not static_roof: # seems that the floor flag is not used for all
				return False
	return True
