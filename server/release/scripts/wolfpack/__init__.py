#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Created by: DarkStorm                      #
#   )).-' {{ ;'`   | Revised by:                                #
#  ( (  ;._ \\ ctr | Last Modification: Created                 #
#===============================================================#

import _wolfpack
from wolfpack.consts import *
from types import *

def finditem( serial ):
	if serial <= 0x40000000:
		raise TypeError, "Invalid Serial <= 0x40000000"
	else:
		return _wolfpack.finditem( int( serial ) )

def findchar( serial ):
	if serial >= 0x40000000:
		raise TypeError, "Invalid Serial >= 0x40000000"
	else:
		return _wolfpack.findchar( int( serial ) )

def findmulti( arg ):
	return _wolfpack.findmulti( arg )

def additem( definition ):
	if not type( definition ) is StringType:
		raise TypeError, "You need to provide a valid definition string"
	else:
		return _wolfpack.additem( definition )

def addnpc( definition, pos ):
	if not type( definition ) is StringType:
		raise TypeError, "You need to provide a valid definition string"
	else:
		return _wolfpack.addnpc( definition, pos )

def region( x, y, map ):
	if not type( x ) is IntType or not type( y ) is IntType or not type( map ) is IntType:
		raise TypeError, "x, y and map need to be integer values"
	else:
		return _wolfpack.region( x, y, map )

def itemregion( x1, y1, x2, y2, map ):
	return _wolfpack.itemregion( x1, y1, x2, y2, map )
	
def charregion( x1, y1, x2, y2, map ):
	return _wolfpack.charregion( x1, y1, x2, y2, map )	

# Statics, Items, Map
def map( x, y, map ):
	if not type( x ) is IntType or not type( y ) is IntType or not type( map ) is IntType:
		raise TypeError, "x, y and map need to be integer values"
	else:
		return _wolfpack.map( x, y, map )

def hasmap( map ):
	if not type( map ) is IntType:
		raise TypeError, "map needs to be integer value"
	else:
		return _wolfpack.hasmap( map )
	
def statics( x, y, map, exact=1 ):
	if not type( x ) is IntType or not type( y ) is IntType or not type( map ) is IntType or not type( exact ) is IntType:
		raise TypeError, "x, y and map need to be integer values"
	else:
		return _wolfpack.statics( x, y, map, exact )
#statics = _wolfpack.statics

def items( x, y, map, range=1 ):
	if not type( x ) is IntType or not type( y ) is IntType or not type( map ) is IntType or not type( range ) is IntType:
		raise TypeError, "x, y and map need to be integer values"
	else:
		return _wolfpack.items( x, y, map, range )

def chars( x, y, map, range=1 ):
	if not type( x ) is IntType or not type( y ) is IntType or not type( map ) is IntType or not type( range ) is IntType:
		raise TypeError, "x, y and map need to be integer values"
	else:
		return _wolfpack.chars( x, y, map, range )

def itemiterator():
	return _wolfpack.itemiterator()

def chariterator():
	return _wolfpack.chariterator()

def allitemsserials():
	return _wolfpack.allitemsserials()

def allcharsserials():
	return _wolfpack.allcharsserials()
	
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
	
def list( id ):
	if not type( id ) is StringType:
		raise TypeError, "id needs to be a string"

	return _wolfpack.list( id )

# Register a global hook
def registerglobal( event, script ):
	return _wolfpack.registerglobal( event, script )

# Register this script to fetch a specific command
def registercommand( command, script ):
	return _wolfpack.registercommand( command, script )

def tiledata( id ):
	return _wolfpack.tiledata( id )

def landdata( id ):
	return _wolfpack.landdata( id )

def coord( x, y, z, map ):
	return _wolfpack.coord( x, y, z, map )

def effect( id, pos, speed, duration ):
	_wolfpack.effect( id, pos, speed, duration )

# Returns uptime of server in seconds
def serveruptime():
	return _wolfpack.serveruptime()
	
# Returns the server version
def serverversion():
	return _wolfpack.serverversion()

def isstarting():
	return _wolfpack.isstarting()
	
def isrunning():
	return _wolfpack.isrunning()
	
def isreloading():
	return _wolfpack.isreloading()
	
def isclosing():
	return _wolfpack.isclosing()
	
def spell( id ):
	return _wolfpack.spell( id )

def multi( id ):
	return _wolfpack.multi( id )
	
def tiledata( id ):
	return _wolfpack.tiledata( id )
	
def newnpc( createserial = 0 ):
	return _wolfpack.newnpc( createserial )

def newitem( createserial = 0 ):
	return _wolfpack.newitem( createserial )

def newplayer( createserial = 0 ):
	return _wolfpack.newplayer( createserial )

def tickcount():
	return _wolfpack.tickcount()

def packet( id, size ):
	return _wolfpack.packet( id, size )
	
queueaction = _wolfpack.queueaction
