
import wolfpack
from wolfpack.consts import EVENT_USE

dirs = {
	0: [0, -1],
	1: [1, -1],
	2: [1, 0],
	3: [1, 1],
	4: [0, 1],
	5: [-1, 1],
	6: [-1, 0],
	7: [-1, -1]
}

def onLoad():
	wolfpack.registerpackethook( 0x12, managePacket )

def managePacket( socket, packet ):
	# Leave this in, used for packet sniffing.
	#socket.sysmessage( "%i, %x" % ( packet.size, packet.getbyte(3) ) )

	# Open Doors
	if packet.size == 5 and packet.getbyte(3) == int( 0x58 ):
		if not socket.player:
			return False
		else:
			openDoor( socket )
			return True
	# Actions, Salute and Bow
	elif packet.size in [ 8, 11 ] and packet.getbyte(3) == int( 0xC7 ):
		if not socket.player:
			return False
		else:
			performAction( socket, packet )
			return True
	# Invoke Virtues
	elif packet.size == 6 and packet.getbyte(3) == int( 0xf4 ):
		return False

def performAction( socket, packet ):
	char = socket.player
	# Bow
	if packet.size == 8:
		char.action( 0x20 )
	# Salute
	elif packet.size == 11:
		char.action( 0x21 )
	return True

def openDoor( socket ):
	char = socket.player
	dir = char.direction
	doors = wolfpack.items(char.pos.x + dirs[dir][0], char.pos.y + dirs[dir][1], char.pos.map, 0)

	if not doors:
		return False

	opendoor = 0
	reach = 0
	for door in doors:
		if char.pos.z == door.pos.z:
			reach = 1
		elif char.pos.z < door.pos.z and char.pos.z >= ( door.pos.z - 5):
			reach = 1
		elif char.pos.z > door.pos.z and char.pos.z <= ( door.pos.z + 5):
			reach = 1
		if reach == 1:
			for event in door.scripts:
				if event == 'door':
					opendoor = 1
					break

		if opendoor == 1:
			scripts = item.scripts + item.basescripts.split(',')
			args = (char, door)
			for script in scripts:
				if wolfpack.hasevent(script, EVENT_USE):
					if wolfpack.callevent(script, EVENT_USE, args):
						break
			break
	return True
