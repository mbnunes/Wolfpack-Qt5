"""
	\command remove
	\description Delete an item or character.
"""

import wolfpack
from wolfpack.consts import LOG_MESSAGE
from wolfpack.utilities import hex2dec

def onLoad():
	wolfpack.registercommand( "remove", commandRemove )

def commandRemove(socket, cmd, args):
	if len(args) > 0:
		serial = hex2dec(args)
		doRemoveSerial( socket, serial )
		return True
	socket.sysmessage( "Please select the object for removal." )
	if( socket.account.authorized('Misc', 'May Remove Players') ):
		socket.sysmessage( "Caution: This can remove players!" )
	socket.attachtarget( "commands.remove.doRemove", [] )
	return True

def doRemoveSerial( socket, serial ):
	object = wolfpack.findobject( serial )
	if object and object.isitem():
		socket.sysmessage("Found item object 0x%x for deletion." % serial )
		socket.player.log( LOG_MESSAGE, "Removed item 0x%x.\n" % serial )
		object.delete()
		return True
	else:
		socket.sysmessage("Object 0x%x could not be found for deletion." % serial )
		return False
	return False

def doRemove( char, args, target ):
	if target.item:
		char.log( LOG_MESSAGE, "Removed item 0x%x.\n" % target.item.serial )
		target.item.delete()
		return True
	elif target.char:
		if target.char.npc:
			char.log( LOG_MESSAGE, "Removed npc 0x%x.\n" % target.char.serial )
			target.char.delete()
			return True
		elif not target.char.npc:
			player = target.char

			if player.socket:
				socket = player.socket
			else:
				socket = None

			if player.rank >= char.rank or not char.account.authorized('Misc', 'May Remove Players'):
				char.socket.sysmessage("You cannot remove this player.")
				return False
			else:
				if player.account and player.account.acl != 'player':
					char.socket.sysmessage( "Players with special accounts can not be removed." )
					return False
				if socket:
					socket.disconnect()
					char.log( LOG_MESSAGE, "Removed player 0x%x.\n" % target.char.serial )
					target.char.delete()
					return True
				else:
					char.log( LOG_MESSAGE, "Removed player 0x%x.\n" % target.char.serial )
					target.char.delete()
					return True
	else:
		char.socket.sysmessage( "This was not a valid object for deletion!" )
		return False
