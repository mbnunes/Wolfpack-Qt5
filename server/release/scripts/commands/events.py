
import wolfpack
from wolfpack.consts import LOG_MESSAGE, EVENT_USE

def addevent_response(player, arguments, target):
	event = arguments[0]
	object = None

	if target.item:
		object = target.item
	elif target.char:
		if target.char.rank > player.rank and player != target.char:
			player.socket.sysmessage("You've burnt your fingers!")
			return

		object = target.char
	else:
		player.socket.sysmessage('You have to target a character or item.')
		return

	player.log(LOG_MESSAGE, "Adds event '%s' to object 0x%x.\n" % (event, object.serial))
	object.addevent(event)
	object.resendtooltip()
	player.socket.sysmessage('You add the event to your target.')

"""
	\command addevent
	\description Attach a script to an object.
	\usage - <code>addevent [script]</code>
	Script is the id of the script you want to attach.
"""
def commandAddevent(socket, command, arguments):
	if len(arguments) == 0:
		socket.sysmessage('Usage: addevent <identifier>')
		return

	event = arguments.strip()

	try:
		wolfpack.hasevent(event, EVENT_USE)
	except:
		socket.sysmessage('No such event: %s.' % event)
		return

	socket.sysmessage("Please select the object you want to add the event '%s' to." % event)
	socket.attachtarget('commands.events.addevent_response', [event])

def removeevent_response(player, arguments, target):
	event = arguments[0]
	object = None

	if target.item:
		object = target.item
	elif target.char:
		if target.char.rank > player.rank and player != target.char:
			player.socket.sysmessage("You've burnt your fingers!")
			return

		object = target.char
	else:
		player.socket.sysmessage('You have to target a character or item.')
		return

	if object.hasevent(event):
		player.log(LOG_MESSAGE, "Removes event '%s' from object 0x%x.\n" % (event, object.serial))
		object.removeevent(event)
		object.resendtooltip()
		player.socket.sysmessage('You remove the event from your target.')
	else:
		player.socket.sysmessage('Your target does not have the given event.')

"""
	\command removeevent
	\description Remove a script from an object.
	\usage - <code>removeevent [script]</code>
	Script is the id of the script you want to remove.
"""
def commandRemoveevent(socket, command, arguments):
	if len(arguments) == 0:
		socket.sysmessage('Usage: removeevent <identifier>')
		return

	event = arguments.strip()

	try:
		wolfpack.hasevent(event, EVENT_USE)
	except:
		socket.sysmessage('No such event: %s.' % event)
		return

	socket.sysmessage("Please select the object you want to remove the event '%s' from." % event)
	socket.attachtarget('commands.events.removeevent_response', [event])

def onLoad():
	wolfpack.registercommand('addevent', commandAddevent)
	wolfpack.registercommand('removeevent', commandRemoveevent)
	#wolfpack.registercommand('addevent', commandAddevent)
