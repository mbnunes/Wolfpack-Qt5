import wolfpack
from wolfpack import tr

"""
	\command findlock
	\usage - <code>findlock lock</code>
	\description Go to the item found with the lock id.
	\notes Command for finding the matching lock to keys.
"""

def findlock(socket, command, arguments):
	if len(arguments) == 0:
		socket.sysmessage( tr('Usage: findlock <lock>') )
		return

	lock = arguments.strip().lower()
	items = wolfpack.itemiterator()
	item = items.first

	while item:
		if item.hasscript( "lock" ):
			if item.hastag( "lock" ) and item.gettag( "lock" ).lower() == lock:
				container = item.getoutmostitem()

				if container.container:
					container = container.container

				socket.sysmessage( tr("Going to item '%s' [Serial: 0x%x; Top: 0x%x]." % (item.getname(), item.serial, container.serial)) )
				pos = container.pos
				socket.player.removefromview()
				socket.player.moveto(pos)
				socket.player.update()
				socket.resendworld()

				if item.container:
					socket.sendobject(item.container)
					if item.container.isitem():
						socket.sendcontainer(item.container)
				return True

		item = items.next

	socket.sysmessage( "A matching lock with the id '%s' was not found." % lock )

def onLoad():
	wolfpack.registercommand('findlock', findlock)
