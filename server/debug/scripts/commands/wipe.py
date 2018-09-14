#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Created by: Correa                         #
#   )).-' {{ ;'`   | Revised by:                                #
#  ( (  ;._ \\ ctr | Last Modification: Created                 #
#===============================================================#
# .wipe/nuke/nukez Commands					#
#===============================================================#

"""
	\command nuke
	\description Remove items in a certain area.
	\usage - <code>nuke</code>
	- <code>nuke all</code>
	- <code>nuke nomulti</code>
	- <code>nuke onlymulti</code>
	- <code>nuke map</code>
	If you don't specify any parameters, you will
	be able to select a region to nuke. If you
	use wipe all, the whole world will be nuked.
	Using nomulti, you will nuke all items in selected 
	region,	except items that are in some multi.
	Using onlymulti, you will nuke all items in the
	selected region that are in any multi, and just
	these items.
	Using map, you'll nuke the map you're on (Map 0, map 1, etc)
	\notes There is also <b>WIPE</b> which is an alias for this command.
"""
"""
	\command wipe
	\description Remove items in a certain area.
	\usage - <code>wipe</code>
	- <code>wipe all</code>
	- <code>wipe nomulti</code>
	- <code>wipe onlymulti</code>
	- <code>wipe map</code>
	If you don't specify any parameters, you will
	be able to select a region to nuke. If you
	use wipe all, the whole world will be nuked.
	Using nomulti, you will nuke all items in selected 
	region,	except items that are in some multi.
	Using onlymulti, you will nuke all items in the
	selected region that are in any multi, and just
	these items.
	Using map, you'll nuke the map you're on (Map 0, map 1, etc)
	\notes There is also <b>NUKE</b> which is an alias for this command.
"""
"""
	\command nukez
	\description Remove items in a certain area at a given height.
	\usage - <code>nukez z</code>
	- <code>nuke z baseid</code>
	Z is the height where the items should be removed.
	\notes If baseid is given, only items with that baseid will get removed.
"""

import wolfpack
import string
import wolfpack.gumps
from gumps.warning_gump import WarningGump
import wolfpack.consts
from wolfpack import tr

def getBoundingBox( socket, callback, args ):
	socket.attachtarget( "commands.wipe.getBoundingBoxResponse", [0, callback, None, args] )
	return

def getBoundingBoxResponse( char, args, target ):
	if target.item and target.item.layer != 0:
		char.socket.sysmessage( tr("Wiping equipped items is not allowed!") )
		return
	if args[0] == 0:
		char.socket.attachtarget("commands.wipe.getBoundingBoxResponse", [1, args[1], target, args[3]] )
	else:
		args[1]( char.socket, args[2], target, args[3] )
	return

def nuke( socket, command, argstring ):
	argstring = argstring.strip() # Remove trailing and leading whitespaces
	if len( argstring ) > 0:
		if argstring.lower() == "all":
			gump = WarningGump( 1060635, 30720, tr("Wiping <i>all</i> items in the world.<br>Do you wish to proceed?"), 0xFFC000, 420, 400, wipeAllWorld, [] )
			gump.send( socket )
			return
		elif argstring.lower() == "map":
			gump = WarningGump( 1060635, 30720, tr("Wiping <i>all</i> items in this map.<br>Do you wish to proceed?"), 0xFFC000, 420, 400, wipeMap, [] )
			gump.send( socket )
			return
		else:
			if argstring.lower() == "nomulti":
				socket.sysmessage(tr("You choose to nuke anything except items in multis"))
				baseid = argstring
			else:
				if argstring.lower() == "onlymulti":
					socket.sysmessage(tr("You choose to nuke just items in multis"))
					baseid = argstring
				else:
					baseid = argstring
	else:
		baseid = None

	socket.sysmessage(tr("Select the area to remove"))
	getBoundingBox( socket, wipeBoundingBox, baseid )
	return True

def nukez( socket, command, arguments ):
	arguments = arguments.strip() # Remove trailing and leading whitespaces
	if( len(arguments) == 0 or arguments.count(' ') > 1 ):
		socket.sysmessage(tr('Usage: .nukez z [ baseid ]'))
		return

	if( arguments.count(' ') == 1 ):
		(z, baseid) = arguments.split(' ')
	else:
		z = arguments
		baseid = None
	try:
		z = int(z)
	except:
		socket.sysmessage(tr('Invalid z value.'))
		return

	socket.sysmessage(tr("Select the area to remove"))
	getBoundingBox( socket, wipeBoundingBox, [z, baseid] )
	return True

def wipeAllWorld( player, accept, state ):
	if not accept:
		player.socket.sysmessage( tr("Wipe command have been canceled") )
		return 1

	player.socket.sysmessage( tr("Removing all items from world, this may take a while") )
	iterator = wolfpack.itemiterator()
	item = iterator.first
	counter = 0
	while item:
		if item.container == None:
			item.delete()
			counter += 1
		item = iterator.next

	player.socket.sysmessage( tr("%i items have been removed from world") % counter )
	return

def wipeMap( player, accept, state ):
	if not accept:
		player.socket.sysmessage( tr("Wipe command have been canceled") )
		return 1

	player.socket.sysmessage( tr("Removing all items from this map, this may take a while") )
	iterator = wolfpack.itemiterator()
	item = iterator.first
	counter = 0
	while item:
		if item.pos.map == player.pos.map:
			if item.container == None:
				item.delete()
				counter += 1
		item = iterator.next

	player.socket.sysmessage( tr("%i items have been removed from this map") % counter )
	return

def wipeBoundingBox( socket, target1, target2, argstring ):
	if target1.pos.map != target2.pos.map:
		return False
	x1 = min( target1.pos.x, target2.pos.x )
	x2 = max( target1.pos.x, target2.pos.x )
	y1 = min( target1.pos.y, target2.pos.y )
	y2 = max( target1.pos.y, target2.pos.y )

	iterator = wolfpack.itemregion( x1, y1, x2, y2, target2.pos.map )
	item = iterator.first
	count = 0

	if( argstring and (type(argstring) == list or type(argstring) == tuple) ):
		(z, baseid) = argstring
	else:
		baseid = argstring
		z = None

	while item:
		if (type(z) != int or z == item.pos.z) and (not baseid or item.baseid == baseid):
			item.delete()
			count += 1
		else:
			if baseid and baseid.lower() == "nomulti":
				if not item.multi:
					item.delete()
					count += 1
			else:
				if baseid and baseid.lower() == "onlymulti":
					if item.multi:
						item.delete()
						count += 1

		item = iterator.next
	socket.sysmessage( tr("%i items removed") % count )

	if socket.player:
		socket.player.log(wolfpack.consts.LOG_MESSAGE, tr("Nuking from (%d,%d) to (%d,%d). %d items deleted. Arguments given: %s \n") % (x1,y1,x2,y2,count,argstring) )
	else:
		wolfpack.log(wolfpack.consts.LOG_MESSAGE, tr("Nuking from (%d,%d) to (%d,%d). %d items deleted. Arguments given: %s (Socket-ID: %s)\n") % (x1,y1,x2,y2,count,argstring,socket.id) )
	return True

def onLoad():
	wolfpack.registercommand( "wipe", nuke )
	wolfpack.registercommand( "nuke", nuke )
	wolfpack.registercommand( "nukez", nukez )
	return
