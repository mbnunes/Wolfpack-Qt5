#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Created by: DarkStorm                      #
#   )).-' {{ ;'`   | Revised by:                                #
#  ( (  ;._ \\ ctr | Last Modification: Created                 #
#===============================================================#

"""
	\library wolfpack
	\description Contains several server functions.
"""
import _wolfpack
from wolfpack.consts import *
from types import *

additem = _wolfpack.additem
addnpc = _wolfpack.addnpc
guilds = _wolfpack.guilds
newguild = _wolfpack.newguild
findguild = _wolfpack.findguild
queueaction = _wolfpack.queueaction
charbase = _wolfpack.charbase
registerpackethook = _wolfpack.registerpackethook
currenttime = _wolfpack.currenttime
charcount = _wolfpack.charcount
itemcount = _wolfpack.itemcount
getdefinition = _wolfpack.getdefinition
getdefinitions = _wolfpack.getdefinitions
hasevent = _wolfpack.hasevent
hasnamedevent = _wolfpack.hasnamedevent
callevent = _wolfpack.callevent
callnamedevent = _wolfpack.callnamedevent

"""
	\function finditem
	\param serial
	\return Serial of the item, or None if failed.
"""
def finditem( serial ):
	if serial <= 0x40000000:
		raise TypeError, "Invalid Serial <= 0x40000000"
	else:
		return _wolfpack.finditem( int( serial ) )

"""
	\function findchar
	\param serial
	\return Serial of the char, or None if failed.
"""
def findchar( serial ):
	if serial >= 0x40000000:
		raise TypeError, "Invalid Serial >= 0x40000000"
	else:
		return _wolfpack.findchar( int( serial ) )

"""
	\function findmulti
	\param arg
	\return Serial of the multi or None.
"""
def findmulti( arg ):
	return _wolfpack.findmulti( arg )

"""
	\function region
	\param x
	\param y
	\param map
	\return The region id for the coordinates given.
"""
def region( x, y, map ):
	if not type( x ) is IntType or not type( y ) is IntType or not type( map ) is IntType:
		raise TypeError, "x, y and map need to be integer values"
	else:
		return _wolfpack.region( x, y, map )

"""
	\function itemregion
	\param x1
	\param y1
	\param x2
	\param y2
	\param map
	\return Array of items in the given region.
"""
def itemregion( x1, y1, x2, y2, map ):
	return _wolfpack.itemregion( x1, y1, x2, y2, map )

"""
	\function charregion
	\param x1
	\param y1
	\param x2
	\param y2
	\param map
	\return Array of characters in the given region.
"""
def charregion( x1, y1, x2, y2, map ):
	return _wolfpack.charregion( x1, y1, x2, y2, map )

# Statics, Items, Map
"""
	\function map
	\param x
	\param y
	\param map
	\return Unknown?
"""
def map( x, y, map ):
	if not type( x ) is IntType or not type( y ) is IntType or not type( map ) is IntType:
		raise TypeError, "x, y and map need to be integer values"
	else:
		return _wolfpack.map( x, y, map )

"""
	\function hasmap
	\param map
	\return True or False, if you have the map id passed.
"""
def hasmap( map ):
	if not type( map ) is IntType:
		raise TypeError, "map needs to be integer value"
	else:
		return _wolfpack.hasmap( map )

"""
	\function statics
	\param x
	\param y
	\param map
	\param exact
	\return Unknown?
"""
def statics( x, y, map, exact=1 ):
	if not type( x ) is IntType or not type( y ) is IntType or not type( map ) is IntType or not type( exact ) is IntType:
		raise TypeError, "x, y and map need to be integer values"
	else:
		return _wolfpack.statics( x, y, map, exact )
#statics = _wolfpack.statics

"""
	\function items
	\param x
	\param y
	\param map
	\param range
	\return Array of items at a given position.
"""
def items( x, y, map, range=1 ):
	if not type( x ) is IntType or not type( y ) is IntType or not type( map ) is IntType or not type( range ) is IntType:
		raise TypeError, "x, y and map need to be integer values"
	else:
		return _wolfpack.items( x, y, map, range )

"""
	\function chars
	\param x
	\param y
	\param map
	\param range
	\return Array of characters at a given position.
"""
def chars( x, y, map, range=1 ):
	if not type( x ) is IntType or not type( y ) is IntType or not type( map ) is IntType or not type( range ) is IntType:
		raise TypeError, "x, y and map need to be integer values"
	else:
		return _wolfpack.chars( x, y, map, range )

"""
	\function itemiterator
	\return Array of items.
"""
def itemiterator():
	return _wolfpack.itemiterator()

"""
	\function chariterator
	\return Array of characters.
"""
def chariterator():
	return _wolfpack.chariterator()

"""
	\function allitemsserials
	\return Array of all existing item serials
"""
def allitemsserials():
	return _wolfpack.allitemsserials()

"""
	\function allcharsserials
	\return Array of all existing character serials
"""
def allcharsserials():
	return _wolfpack.allcharsserials()

"""
	\function addtimer
	\param expiretime
	\param function
	\param args
	\param serializable
	\return None
"""
def addtimer( expiretime, function, args, serializable=0 ):

	if not type( expiretime ) is IntType:
		raise TypeError, "expiretime needs to be a number"
	elif not type( function ) is StringType:
		raise TypeError, "function needs to be a string"
	elif not type( args ) is ListType:
		raise TypeError, "args need to be a list"
	elif not type( serializable ) is IntType:
		raise TypeError, "serializable needs to be a number"

	if serializable != 0:
		serializable = 1

	return _wolfpack.addtimer( expiretime, function, args, serializable )

"""
	\function list
	\param id
	\return List of the given id.
"""
def list( id ):
	if not type( id ) is StringType:
		raise TypeError, "id needs to be a string"
	return _wolfpack.list( id )

"""
	\function registerglobal
	\param event
	\param script
	\return None
"""
# Register a global hook
def registerglobal( event, script ):
	return _wolfpack.registerglobal( event, script )

"""
	\function registercommand
	\param command
	\param script
	\return None
"""
# Register this script to fetch a specific command
def registercommand( command, script ):
	return _wolfpack.registercommand( command, script )

"""
	\function tiledata
	\param id
	\return Data of the given tile id.
"""
def tiledata( id ):
	return _wolfpack.tiledata( id )

"""
	\function landdata
	\param id
	\return Data of the given land id.
"""
def landdata( id ):
	return _wolfpack.landdata( id )

"""
	\function coord
	\param x
	\param y
	\param z
	\param map
	\return None
"""
def coord( x, y, z, map ):
	return _wolfpack.coord( x, y, z, map )

"""
	\function effect
	\param id
	\param pos
	\param speed
	\param duration
	\return None
"""
def effect( id, pos, speed, duration ):
	_wolfpack.effect( id, pos, speed, duration )

"""
	\function serveruptime
	\return The server's uptime in seconds.
"""
# Returns uptime of server in seconds
def serveruptime():
	return _wolfpack.serveruptime()

"""
	\function serverversion
	\return The server's version string.
"""
# Returns the server version
def serverversion():
	return _wolfpack.serverversion()

"""
	\function isstarting
	\return True or False, if the server is starting.
"""
def isstarting():
	return _wolfpack.isstarting()

"""
	\function isrunning
	\return True or False, if the server is running.
"""
def isrunning():
	return _wolfpack.isrunning()

"""
	\function isreloading
	\return True or False, if the server is reloading.
"""
def isreloading():
	return _wolfpack.isreloading()

"""
	\function isclosing
	\return True or False, if the server is closing.
"""
def isclosing():
	return _wolfpack.isclosing()

"""
	\function spell
	\param id
	\return None
"""
def spell( id ):
	return _wolfpack.spell( id )

"""
	\function multi
	\param id
	\return None
"""
def multi( id ):
	return _wolfpack.multi( id )

"""
	\function newnpc
	\param createserial
	\return The npc created
"""
def newnpc( createserial = 0 ):
	return _wolfpack.newnpc( createserial )

"""
	\function newitem
	\param createserial
	\return The item created.
"""
def newitem( createserial = 0 ):
	return _wolfpack.newitem( createserial )

"""
	\function newplayer
	\param createserial
	\return The player created
"""
def newplayer( createserial = 0 ):
	return _wolfpack.newplayer( createserial )

"""
	\function tickcount
	\return None
"""
def tickcount():
	return _wolfpack.tickcount()

"""
	\function packet
	\param id
	\param size
	\return None
"""
def packet( id, size ):
	return _wolfpack.packet( id, size )

