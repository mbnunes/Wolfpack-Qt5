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
	- <code>nuke all</code>
	If you don't specify any parameters, you will
	be able to select a region to nuke. If you
	use wipe all, the whole world will be nuked.
	\notes There is also <b>WIPE</b> which is an alias for this command.
"""
"""
	\command wipe
	\description Remove items in a certain area.
	\usage - <code>wipe</code>
	- <code>wipe all</code>
	If you don't specify any parameters, you will
	be able to select a region to wipe. If you
	use wipe all, the whole world will be wiped.
	\notes There is also <b>NUKE</b> which is an alias for this command.
"""

import wolfpack
import string
import wolfpack.gumps
from wolfpack.gumps import WarningGump
#from wolfpack.utilities import *

def getBoundingBox( socket, callback, args ) :
	socket.attachtarget( "commands.wipe.getBoundingBoxResponse", [0, callback, None, args] )
	return

def getBoundingBoxResponse( char, args, target ):
	if args[0] == 0:
		char.socket.attachtarget("commands.wipe.getBoundingBoxResponse", [1, args[1], target, args[3]] )
	else:
		args[1]( char.socket, args[2], target, args[3] )
	return

def nuke( socket, command, argstring ):
	if len( argstring ) > 0:
		if argstring.lower() == "all":
			gump = WarningGump( 1060635, 30720, "Wiping <i>all</i> items in the world.<br>Do you wish to proceed?", 0xFFC000, 420, 400, wipeAllWorld, [] )
			gump.send( socket )
			return
		else:
			baseid = argstring
	else:
		baseid = None
	
	socket.sysmessage("Select the area to remove")
	getBoundingBox( socket, wipeBoundingBox, baseid )
	return 1

def wipeAllWorld( player, accept, state ):
	if not accept:
		player.socket.sysmessage( "Wipe command have been canceled" )
		return 1

	player.socket.sysmessage( "Removing all items from world, this may take a while" )
	iterator = wolfpack.itemiterator()
	item = iterator.first
	serials = []
	while item:
		if item.container == None:
			serials.append(item.serial)
		item = iterator.next
	for serial in serials:
		item = wolfpack.finditem(serial)
		if item:
			item.delete()
		
	player.socket.sysmessage( "%i items have been removed from world" % len(serials) )
	return

def wipeBoundingBox( socket, target1, target2, baseid ):
	if target1.pos.map != target2.pos.map:
		return False
	x1 = min( target1.pos.x, target2.pos.x )
	x2 = max( target1.pos.x, target2.pos.x )
	y1 = min( target1.pos.y, target2.pos.y )
	y2 = max( target1.pos.y, target2.pos.y )

	iterator = wolfpack.itemregion( x1, y1, x2, y2, target2.pos.map )
	item = iterator.first
	count = 0
	while item:
		if not baseid or item.baseid == baseid:
			item.delete()
			count += 1
		item = iterator.next
	socket.sysmessage( "%i items removed" % count )

	return 1

def onLoad():
	wolfpack.registercommand( "wipe", nuke )
	wolfpack.registercommand( "nuke", nuke )
	return
