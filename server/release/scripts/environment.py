# Several ID based Actions for environmental objects
# like plants and oters

from wolfpack.consts import *
from wolfpack.utilities import tobackpack
import wolfpack
import time

def cotton( char, item ):
	currenttime = int( time.time() )
	
	if item.hastag( 'lastpick' ):
		lastpick = item.gettag( 'lastpick' )
		
		if lastpick + COTTONPLANTS_REGROW > currenttime:
			char.message( "You can't pick cotton here yet." )
			return 1

	char.action( ANIM_ATTACK5 )
	char.soundeffect( 0x13e )
	
	cotton = wolfpack.additem( 'df9' )
	
	if cotton and not tobackpack( cotton, char ):
		cotton.update()
						
	char.message( "You reach down and pick some cotton." )
	
	# Set a timer for the cotton plant
	item.settag( 'lastpick', currenttime )
	return 1

# Table of IDs mapped to handler functions
actions =  {
			0x0c4f: cotton,
			0x0c50: cotton,
			0x0c51: cotton,
			0x0c52: cotton,
			0x0c53: cotton,
			0x0c54: cotton
		   }

def onUse( char, item ):
	if not actions.has_key( item.id ):
		return 0
		
	return actions[ item.id ]( char, item )
