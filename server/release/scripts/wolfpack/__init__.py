#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Created by: DarkStorm                      #
#   )).-' {{ ;'`   | Revised by:                                #
#  ( (  ;._ \\ ctr | Last Modification: Created                 #
#===============================================================#

import _wolfpack
import types

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

def addnpc( definition ):
	if not type( definition ) is StringType:
		raise TypeError, "You need to provide a valid definition string"
	else:
		return _wolfpack.addnpc( definition )
