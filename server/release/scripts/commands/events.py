
import wolfpack
from wolfpack.consts import LOG_MESSAGE, EVENT_USE

def addscript_response( player, arguments, target ):
	script = arguments[0]
	object = None

	if target.item:
		object = target.item
	elif target.char:
		if target.char.rank > player.rank and player != target.char:
			player.socket.sysmessage( "You've burnt your fingers!" )
			return False
		object = target.char
	else:
		player.socket.sysmessage( 'You have to target a character or item.' )
		return False

	player.log( LOG_MESSAGE, "Adds script '%s' to object 0x%x.\n" % ( script, object.serial ) )
	object.addscript( str( script ) )
	object.resendtooltip()
	player.socket.sysmessage( 'You add the script to your target.' )
	return True

"""
	\command addscript
	\description Attach a script to an object.
	\usage - <code>addscript [script]</code>
	Script is the id of the script you want to attach.
"""
def commandAddscript( socket, command, arguments ):
	if len(arguments) == 0:
		socket.sysmessage( 'Usage: addscript <identifier>' )
		return False

	script = arguments.strip()

	try:
		wolfpack.hasevent( script, EVENT_USE )
	except:
		socket.sysmessage( 'No such script: %s.' % script )
		return False

	socket.sysmessage( "Please select the object you want to add the script '%s' to." % script)
	socket.attachtarget( 'commands.events.addscript_response', [ script ] )

def removescript_response( player, arguments, target ):
	script = arguments[0]
	object = None

	if target.item:
		object = target.item
	elif target.char:
		if target.char.rank > player.rank and player != target.char:
			player.socket.sysmessage( "You've burnt your fingers!" )
			return False

		object = target.char
	else:
		player.socket.sysmessage( 'You have to target a character or item.' )
		return False

	if object.hasscript( str( script ) ):
		player.log( LOG_MESSAGE, "Removes script '%s' from object 0x%x.\n" % ( script, object.serial ) )
		object.removescript( str( script ) )
		object.resendtooltip()
		player.socket.sysmessage( 'You remove the script from your target.' )
		return True
	else:
		player.socket.sysmessage( 'Your target does not have the given script.' )
		return False

"""
	\command removescript
	\description Remove a script from an object.
	\usage - <code>removescript [script]</code>
	Script is the id of the script you want to remove.
"""
def commandRemovescript(socket, command, arguments):
	if len(arguments) == 0:
		socket.sysmessage('Usage: removescript <identifier>')
		return False

	script = arguments.strip()

	try:
		wolfpack.hasevent( script, EVENT_USE )
	except:
		socket.sysmessage( 'No such script: %s.' % script )
		return False

	socket.sysmessage( "Please select the object you want to remove the script '%s' from." % script)
	socket.attachtarget( 'commands.events.removescript_response', [ script ] )
	return True

def onLoad():
	wolfpack.registercommand( 'addscript', commandAddscript )
	wolfpack.registercommand( 'removescript', commandRemovescript )
