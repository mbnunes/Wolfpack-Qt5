#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Created by: Correa                         #
#   )).-' {{ ;'`   | Revised by:                                #
#  ( (  ;._ \\ ctr | Last Modification: Created                 #
#===============================================================#
# .wipe/nuke Commands											#
#===============================================================#

"""
	\command nuke
	\description Remove items in a certain area.
	\usage - <code>nuke</code>
	\usage - <code>nuke all</code>
	If you don't specify any parameters, you will
	be able to select a region to nuke. If you
	use wipe all, the whole world will be nuked.
	\notes There is also <b>WIPE</b> which is an alias for this command.
"""
"""
	\command wipe
	\description Remove items in a certain area.
	\usage - <code>wipe</code>
	\usage - <code>wipe all</code>
	If you don't specify any parameters, you will
	be able to select a region to wipe. If you
	use wipe all, the whole world will be wiped.
	\notes There is also <b>NUKE</b> which is an alias for this command.
"""

import wolfpack
import string
import wolfpack.gumps
from wolfpack.gumps import *
from wolfpack.utilities import *
from wolfpack import *

def getBoundingBox( socket, callback ) :
	socket.attachtarget( "commands.wipe.getBoundingBoxResponse", [0, callback] )

def getBoundingBoxResponse( char, args, target ):
	if args[0] == 0:
		char.socket.attachtarget("commands.wipe.getBoundingBoxResponse", [1, args[1], target] )
	else:
		args[1]( char.socket, args[2], target )


def nuke( socket, command, argstring ):
	if len( argstring ) > 0:
		if argstring.lower() == "all":
			gump = WarningGump( 1060635, 30720, "Wipping <i>all</i> items in the world.<br>Do you wish to proceed?", 0xFFC000, 420, 400, wipeAllWorld, [] )
			gump.send( socket )
	else:
		socket.sysmessage("Select the area to remove")
		getBoundingBox( socket, wipeBoundingBox )

	return 1

def wipeAllWorld( player, accept, state ):
	if not accept:
		player.socket.sysmessage( "Wipe command have been canceled" )
		return 1

	player.socket.sysmessage( "Removing all items from world, this may take a while" )
	iterator = wolfpack.itemiterator()
	item = iterator.first
	count = 0;
	while item:
		if item.container == None:
			item.delete()
			count += 1
		item = iterator.next
	player.socket.sysmessage( "%i items have been removed from world" % count )

def wipeBoundingBox( socket, target1, target2 ):
	count = 0
	iterator = wolfpack.itemregion( target1.pos.x, target1.pos.y, target2.pos.x, target2.pos.y, target2.pos.map )
	item = iterator.first
	while item:
		item.delete()
		item = iterator.next
		count += 1;

	socket.sysmessage( "%i items removed" % count )
	return 1

def onLoad():
	wolfpack.registercommand( "wipe", nuke )
	wolfpack.registercommand( "nuke", nuke )
