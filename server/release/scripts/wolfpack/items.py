#######################################################
#
# Wrapper for wolfpack.items
#
#######################################################

from _wolfpack import *

def findBySerial( Serial ):
	return items_findbyserial( Serial )

def add( itemid, a=None, b=None, c=None ):
	if( a is None ):
		return items_add( itemid )
	
	elif( a != None ) and ( ( b == None ) or ( c == None ) ):
		return items_add( itemid, a )
		
	else:
		return items_add( itemid, a, b, c )