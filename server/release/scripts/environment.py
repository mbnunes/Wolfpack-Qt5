# Several ID based Actions for environmental objects
# like plants and oters

from wolfpack.consts import *
from wolfpack.utilities import tobackpack
import wolfpack
import time
import random

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
	
def sextant_parts( char, item ):
	if not char.checkskill( TINKERING, 0, 500 ):
		if random.randint( 1, 100 ) <= 25:
			additional = ' and break the parts'
			
			if item.amount > 1:
				item.amount -= 1
				item.update()
			else:
				item.delete()			
		else:
			additional = ''
	
		char.message( "You don't manage to create the sextant%s" % additional )
	else:
		if item.amount > 1:
			item.amount -= 1
			item.update()
		else:
			item.remove()	
	
		if item.id == 0x1059:
			item = wolfpack.additem( '1057' )
		else:
			item = wolfpack.additem( '1058' )

		char.getbackpack().additem( item, 1, 1, 0 )
		item.update()
		char.message( "You put the sextant into your backpack" )
		
	return 1		

def sextant( char, item ):
	char.message( 'Sorry but this feature is not implemented yet' )
	return 1

def drum( char, item ):
	if char.checkskill( MUSICIANSHIP, 0, 1000 ):
		char.soundeffect( 0x38 )
	else:
		char.soundeffect( 0x39 )

def tambourine( char, item ):
	if char.checkskill( MUSICIANSHIP, 0, 1000 ):
		char.soundeffect( 0x52 )
	else:
		char.soundeffect( 0x53 )

def harp( char, item ):
	if char.checkskill( MUSICIANSHIP, 0, 1000 ):
		char.soundeffect( 0x45 )
	else:
		char.soundeffect( 0x46 )
		
def lute( char, item ):
	if char.checkskill( MUSICIANSHIP, 0, 1000 ):
		char.soundeffect( 0x4c )
	else:
		char.soundeffect( 0x4d )
	

# Table of IDs mapped to handler functions
actions =  {
			# Cotton Plants
			0x0c4f: cotton,
			0x0c50: cotton,
			0x0c51: cotton,
			0x0c52: cotton,
			0x0c53: cotton,
			0x0c54: cotton,
			
			# Sextant
			0x1057: sextant,
			0x1058: sextant,
			
			# Sextant Parts
			0x1059: sextant_parts,
			0x105a: sextant_parts,
			
			# Instruments
			0xe9c: drum,
			0xe9d: tambourine,
			0xe9e: tambourine,
			0xeb1: harp,
			0xeb2: harp,
			0xeb3: lute,
			0xeb4: lute
		   }

def onUse( char, item ):
	if not actions.has_key( item.id ):
		return 0
		
	return actions[ item.id ]( char, item )
