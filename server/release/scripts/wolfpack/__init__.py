#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Created by: DarkStorm                      #
#   )).-' {{ ;'`   | Revised by:                                #
#  ( (  ;._ \\ ctr | Last Modification: Created                 #
#===============================================================#

import _wolfpack
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

# Statics, Items, Map
def map( x, y, map ):
	if not type( x ) is IntType or not type( y ) is IntType or not type( map ) is IntType:
		raise TypeError, "x, y and map need to be integer values"
	else:
		return _wolfpack.map( x, y, map )
	
def statics( x, y, map, exact=1 ):
	if not type( x ) is IntType or not type( y ) is IntType or not type( map ) is IntType or not type( exact ) is IntType:
		raise TypeError, "x, y and map need to be integer values"
	else:
		return _wolfpack.statics( x, y, map, exact )

def items( x, y, map, exact=1 ):
	if not type( x ) is IntType or not type( y ) is IntType or not type( map ) is IntType or not type( exact ) is IntType:
		raise TypeError, "x, y and map need to be integer values"
	else:
		return _wolfpack.items( x, y, map, exact )

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
