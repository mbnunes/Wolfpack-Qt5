# Several ID based Actions for environmental objects
# like plants and oters

from wolfpack.consts import *
from wolfpack.utilities import tobackpack
from wolfpack.gumps import cGump
from math import floor
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
			item.delete()

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
	if char.checkskill(MUSICIANSHIP, 0, 1000):
		char.soundeffect(0x38)
	else:
		char.soundeffect(0x39)
	return 1

def tambourine( char, item ):
	if char.checkskill(MUSICIANSHIP, 0, 1000):
		char.soundeffect(0x52)
	else:
		char.soundeffect(0x53)
	return 1

def harp(char, item):
	if char.checkskill(MUSICIANSHIP, 0, 1000):
		char.soundeffect(0x45)
	else:
		char.soundeffect(0x46)
	return 1

def lute(char, item):
	if char.checkskill(MUSICIANSHIP, 0, 1000):
		char.soundeffect(0x4c)
	else:
		char.soundeffect(0x4d)
	return 1

hairdye_groups = 	[
						#  Starting Hue, Count
						[ 1601, 26 ],
						[ 1627, 27 ],
						[ 1501, 32 ],
						[ 1301, 32 ],
						[ 1401, 32 ],
						[ 1201, 24 ],
						[ 2401, 29 ],
						[ 2212, 6 ],
						[ 1101, 8 ],
						[ 1109, 8 ],
						[ 1117, 16 ],
						[ 1133, 16 ]
					]

def hairdye( char, item ):
	if item.container != char.getbackpack():
		char.message( 'This item has to be in your backpack to use it.' )
		return 1

	gump = cGump( x=50, y=50, callback="environment.hairdye_callback" )

	# First Page (visible everywhere)
	gump.startPage( 0 )

	gump.addBackground( 0xa28, 350, 355 )
	gump.addResizeGump( 10, 44, 0x13EC, 110, 270 )

	gump.addXmfHtmlGump( 0, 16, 350, 35, 0xF6D45 ) # Gump Title (Hair Color Selection...)

	# Ok Button
	gump.addButton( 50, 318, 0xFA5, 0xFA7, 1 )
	gump.addXmfHtmlGump( 95, 319, 250, 35, 0xF6D46 )

	# Add Hairdye Groups (visible on every page)
	i = 0

	for dye_group in hairdye_groups:
		gump.addPageButton( 87, 50 + ( i * 22 ), 0x1468, 0x1468, i + 1 )
		gump.addText( 20, 49 + ( i * 22 ), '*****', dye_group[0] )
		i += 1

	i = 0

	for dye_group in hairdye_groups:
		gump.startPage( i + 1 )

		for j in range( 0, dye_group[1] ):
			gump.addText( 178 + ( floor( j / 16 ) * 80 ), 42 + ( ( j % 16 ) * 17 ), '*****', dye_group[0] + j )
			gump.addRadioButton( 156 + ( floor( j / 16 ) * 80 ), 42 + ( ( j % 16 ) * 17 ), 0xD2, 0xD3, dye_group[0] + j )

		i += 1

	gump.setArgs( [ item.serial ] )

	gump.send( char )

	return 1

def hairdye_callback( char, args, response ):
	if response.button != 1:
		return

	# Check the item first
	item = wolfpack.finditem( args[0] )

	if not item or item.container != char.getbackpack():
		char.message( 'The item has to be in your backpack to use it.' )
		return

	# Check if it's a valid color
	if len( response.switches ) != 1:
		char.message( 'You have to choose a hair color.' )
		return

	color = response.switches[0]

	# Find the color
	for dye_group in hairdye_groups:
		if color >= dye_group[0] and color < dye_group[0] + dye_group[1]:
			item.delete()

			hair = char.itemonlayer( LAYER_HAIR )
			beard = char.itemonlayer( LAYER_BEARD )

			if beard:
				beard.color = color
				beard.update()

			if hair:
				hair.color = color
				hair.update()

			return

	char.message( 'Thats an invalid color' )

# Dying Tub
def dyingtub( char, item ):
	if not char.canreach( item, 2 ):
		char.socket.sysmessage( "You can't reach this." )
		return 1

	char.socket.sysmessage( 'What do you want to use this on?' )
	char.socket.attachtarget( 'environment.dyingtub_response', [ item.serial ] )
	return 1

def dyingtub_response( char, args, target ):
	dyetub = wolfpack.finditem( args[0] )

	if not dyetub or not char.canreach( dyetub, 2 ):
		char.message( "You can't reach the dyetub from here." )
		return

	if not target.item:
		char.message( 'You need to target an item.' )
		return

	# Valid Target?
	if not char.gm:
		if target.item.getoutmostchar() != char:
			char.socket.sysmessage( "You have to have this in your belongings." )
			return

		if not target.item.dye:
			char.socket.sysmessage( "You cannot dye this." )
			return

	char.socket.log( LOG_TRACE, "Dying item (%x,%x) using tub (%x,%x)\n" % ( target.item.serial, target.item.color, dyetub.serial, dyetub.color ) )
	target.item.color = dyetub.color
	target.item.update()
	char.soundeffect(0x023e)

# Table of IDs mapped to handler functions
actions =	{
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
			0xeb4: lute,

			# Hair Dyes
			0xe27: hairdye,
			0xeff: hairdye,

			# Dye Tub
			0xfab: dyingtub,

		}

def onUse( char, item ):
	if not actions.has_key( item.id ):
		return 0

	return actions[ item.id ]( char, item )
