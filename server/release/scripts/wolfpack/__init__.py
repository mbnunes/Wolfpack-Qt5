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
#statics = _wolfpack.statics

"""
	\function wolfpack.finditem
	\param serial
	\return Object or None
	\description Tries to find an object by its serial.
"""
def finditem( serial ):
	if serial <= 0x40000000:
		raise TypeError, "Invalid Serial <= 0x40000000"
	else:
		return _wolfpack.finditem( int( serial ) )

"""
	\function wolfpack.findchar
	\param serial
	\return Character of None
	\description Tries to find a character by its serial.
"""
def findchar( serial ):
	if serial >= 0x40000000:
		raise TypeError, "Invalid Serial >= 0x40000000"
	else:
		return _wolfpack.findchar( int( serial ) )

"""
	\function wolfpack.findmulti
	\param arg
	\return Multi or None.
	\description Tries to find a multi based on its position.
"""
def findmulti( arg ):
	return _wolfpack.findmulti( arg )

def region( x, y, map ):
	if not type( x ) is IntType or not type( y ) is IntType or not type( map ) is IntType:
		raise TypeError, "x, y and map need to be integer values"
	else:
		return _wolfpack.region( x, y, map )

"""
	\function wolfpack.itemregion
	\param x1
	\param y1
	\param x2
	\param y2
	\param map
	\return List of Items or None
	\description Grabs a list of items in a given area.
"""
def itemregion( x1, y1, x2, y2, map ):
	return _wolfpack.itemregion( x1, y1, x2, y2, map )

"""
	\function wolfpack.charregion
	\param x1
	\param y1
	\param x2
	\param y2
	\param map
	\return List of Characters or None
	\description Grabs a list of characters in a given area.
"""
def charregion( x1, y1, x2, y2, map ):
	return _wolfpack.charregion( x1, y1, x2, y2, map )

# Statics, Items, Map
"""
	\function wolfpack.map
	\param x
	\param y
	\param map
	\return List of Map Information
	\description Returns a dictionary with information about a given map tile.
"""
def map( x, y, map ):
	if not type( x ) is IntType or not type( y ) is IntType or not type( map ) is IntType:
		raise TypeError, "x, y and map need to be integer values"
	else:
		return _wolfpack.map( x, y, map )

"""
	\function wolfpack.hasmap
	\param map
	\return True or False
	\description Returns True/False if you have the map id.
"""
def hasmap( map ):
	if not type( map ) is IntType:
		raise TypeError, "map needs to be integer value"
	else:
		return _wolfpack.hasmap( map )

"""
	\function wolfpack.statics
	\param x
	\param y
	\param map
	\param exact
	\return List of Statics
	\description Returns a list of statics at a given point.
"""
def statics( x, y, map, exact=1 ):
	if not type( x ) is IntType or not type( y ) is IntType or not type( map ) is IntType or not type( exact ) is IntType:
		raise TypeError, "x, y and map need to be integer values"
	else:
		return _wolfpack.statics( x, y, map, exact )

"""
	\function wolfpack.items
	\param x
	\param y
	\param map
	\param range
	\return List of Items
	\description Returns a list of items at a given point.
"""
def items( x, y, map, range=1 ):
	if not type( x ) is IntType or not type( y ) is IntType or not type( map ) is IntType or not type( range ) is IntType:
		raise TypeError, "x, y and map need to be integer values"
	else:
		return _wolfpack.items( x, y, map, range )

"""
	\function wolfpack.chars
	\param x
	\param y
	\param map
	\param range
	\return List of Chars
	\description Returns a list of characters at a given point.
"""
def chars( x, y, map, range=1 ):
	if not type( x ) is IntType or not type( y ) is IntType or not type( map ) is IntType or not type( range ) is IntType:
		raise TypeError, "x, y and map need to be integer values"
	else:
		return _wolfpack.chars( x, y, map, range )

"""
	\function wolfpack.itemiterator
	\return List of Items.
	\description Returns an iterator for all items in the world.
"""
def itemiterator():
	return _wolfpack.itemiterator()

"""
	\function wolfpack.chariterator
	\return List of Chars.
	\description Returns an iterator for all chars in the world.
"""
def chariterator():
	return _wolfpack.chariterator()

"""
	\function wolfpack.allitemsserials
	\return List of Item Serials
	\description Returns a list of all item serials.
"""
def allitemsserials():
	return _wolfpack.allitemsserials()

"""
	\function wolfpack.allcharsserials
	\return List of Char Serials
	\description Returns a list of all character serials.
"""
def allcharsserials():
	return _wolfpack.allcharsserials()

"""
	\function wolfpack.addtimer
	\param expiretime
	\param function
	\param args
	\param serializable
	\return None
	\description Adds a timed effect.
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
	\function wolfpack.list
	\param id
	\return List of Definitions
	\description Returns a list defined in the definitions as a Python List.
"""
def list( id ):
	if not type( id ) is StringType:
		raise TypeError, "id needs to be a string"
	return _wolfpack.list( id )

"""
	\function wolfpack.registerglobal
	\param event
	\param script
	\return None
	\description Registers a global script hook.
"""
# Register a global hook
def registerglobal( event, script ):
	return _wolfpack.registerglobal( event, script )

"""
	\function wolfpack.registercommand
	\param command
	\param script
	\return None
	\description Registers a global command hook.
"""
# Register this script to fetch a specific command
def registercommand( command, script ):
	return _wolfpack.registercommand( command, script )

"""
	\function wolfpack.tiledata
	\param id
	\return Tile Data
	\description Returns the tiledata information for a given tile stored on the server.
"""
def tiledata( id ):
	return _wolfpack.tiledata( id )

"""
	\function wolfpack.landdata
	\param id
	\return Land Data
	\description Returns the landdata information for a given tile stored on the server.
"""
def landdata( id ):
	return _wolfpack.landdata( id )

"""
	\function wolfpack.coord
	\param x
	\param y
	\param z
	\param map
	\return None
	\description Creates a coordinate object from the given parameters (x,y,z,map).
"""
def coord( x, y, z, map ):
	return _wolfpack.coord( x, y, z, map )

"""
	\function wolfpack.effect
	\param id
	\param pos
	\param speed
	\param duration
	\return None
	\description Shows a graphical effect.
"""
def effect( id, pos, speed, duration ):
	_wolfpack.effect( id, pos, speed, duration )

"""
	\function wolfpack.serveruptime
	\return Uptime, INT
	\description Returns uptime of server in seconds.
"""
# Returns uptime of server in seconds
def serveruptime():
	return _wolfpack.serveruptime()

"""
	\function wolfpack.serverversion
	\return Version, STRING
	\description Returns the server version string.
"""
# Returns the server version
def serverversion():
	return _wolfpack.serverversion()

"""
	\function wolfpack.isstarting
	\return True or False
	\description Returns if the server is in starting state.
"""
def isstarting():
	return _wolfpack.isstarting()

"""
	\function wolfpack.isrunning
	\return True or False
	\description Returns if the server is in running state.
"""
def isrunning():
	return _wolfpack.isrunning()

"""
	\function wolfpack.isreloading
	\return True or False
	\description Returns if the server is in reload state.
"""
def isreloading():
	return _wolfpack.isreloading()

"""
	\function wolfpack.isclosing
	\return True or False
	\description Returns if the server is in closing state.
"""
def isclosing():
	return _wolfpack.isclosing()

#def spell( id ):
#	return _wolfpack.spell( id )

"""
	\function wolfpack.addmulti
	\param def The definition id.
	\return Item or None
	\description Creates a multi from a given definition id.
"""
addmulti = _wolfpack.addmulti

"""
	\function wolfpack.newnpc
	\param createserial
	\return NPC
	\description Creates an entirely new npc.
"""
def newnpc( createserial = 0 ):
	return _wolfpack.newnpc( createserial )

"""
	\function wolfpack.newitem
	\param createserial
	\return Item
	\description Creates an entirely new item.
"""
def newitem( createserial = 0 ):
	return _wolfpack.newitem( createserial )

"""
	\function wolfpack.newplayer
	\param createserial
	\return Player
	\description Creates an entirely new player.
"""
def newplayer( createserial = 0 ):
	return _wolfpack.newplayer( createserial )

"""
	\function wolfpack.tickcount
	\return Tickcount
	\description Returns the current Tickcount on Windows.
"""
def tickcount():
	return _wolfpack.tickcount()

"""
	\function wolfpack.packet
	\param id
	\param size
	\return None
	\description Creates a packet.
"""
def packet( id, size ):
	return _wolfpack.packet( id, size )
