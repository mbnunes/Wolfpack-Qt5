#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts
#  ((    _/{  "-;  | Created by: Magnus
#   )).-' {{ ;'`   | Revised by:
#  ( (  ;._ \\ ctr | Last Modification: Created
#===============================================================#
"""
	\command restock
	\description Restock NPCs in all world.
	\usage - <code>restock</code>
	- <code>restock all</code>
	- <code>restock baseid</code>
	If you use restock all, you will restock all spawned
	npcs in the world (just the spawned).
	If you specify a baseid, all the npcs in the world
	with that baseid, will restock.
	If you specify nothing, command wont work.
	\notes This will only act on spawned NPCs
"""

import wolfpack
from wolfpack.consts import LOG_MESSAGE

def commandRestock(socket, cmd, args):
	if len(args) == 0:
		socket.sysmessage('Usage: restock <baseid> OR restock all')
		return

	baseid = args.lower()

	counter = 0

	chars = wolfpack.chariterator()
	char = chars.first

	# Now lets check the args. I think is better to check here than check in NPC Loop
	if baseid == 'all':

		while char:
			if char.hastag( "spawner" ):
				char.delete()
				counter += 1

			char = chars.next

	else:

		while char:
			if char.baseid.lower() == baseid:
				if char.hastag( "spawner" ):
					char.delete()
					counter += 1

			char = chars.next

	socket.sysmessage( str(counter) + " NPCs restocked in the World" )

def onLoad():
	wolfpack.registercommand( "restock", commandRestock )
	return