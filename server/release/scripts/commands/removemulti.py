"""
	\command removemulti
	\description Delete a multi.
"""

import wolfpack
import housing
from wolfpack.consts import LOG_MESSAGE
from wolfpack.utilities import hex2dec

def commandRemovemulti(socket, cmd, args):
	
	socket.sysmessage( "Please select the multi for removal." )
	
	socket.attachtarget( "commands.removemulti.doRemovemulti", [] )
	return True

def doRemovemulti( char, args, target ):

	# finding the multi
	multi = wolfpack.findmulti(target.pos)

	# Now we have the multi... or not	
	if not multi:
		char.socket.sysmessage( "No Multi here for removal!" )
		return True
	else:
		# Lets make similar to demolish:
		# Looking for the list of items in the Multi
		listitems = multi.objects

		# Now... the Loop to remove all items
		contador = 0
		for multiitem in listitems:
			multiitem.delete()
			contador += 1
	
		# Message about how many items are deleted in the house
		char.socket.sysmessage( "Deleted %i items in house!" % contador )

		# Unregistering the House
		housing.unregisterHouse(multi)

		# Erasing Multi
		multi.delete()

		return True

def onLoad():
	wolfpack.registercommand( "removemulti", commandRemovemulti )
	return
