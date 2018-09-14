# Several ID based Actions for environmental objects
# like plants and oters

import wolfpack
import wolfpack.time
import random
from wolfpack.consts import COTTONPLANTS_REGROW, ANIM_ATTACK5, TINKERING, \
	LAYER_HAIR, LAYER_BEARD, LOG_MESSAGE
from wolfpack.utilities import tobackpack
from wolfpack.gumps import cGump
from math import floor
from wolfpack import tr

def cotton( char, item ):
	if item.hastag( 'lastpick' ):
		lastpick = item.gettag( 'lastpick' )

		if lastpick + COTTONPLANTS_REGROW > wolfpack.time.currenttime():
			char.message( tr("You cannot pick cotton here yet.") )
			return 1

	char.action( ANIM_ATTACK5 )
	char.soundeffect( 0x13e )

	cotton = wolfpack.additem( 'df9' )

	if cotton and not tobackpack( cotton, char ):
		cotton.update()

	char.message( tr("You reach down and pick some cotton.") )

	# Set a timer for the cotton plant
	item.settag( 'lastpick', wolfpack.time.currenttime() )
	return 1

def sextant_parts( char, item ):
	if not char.checkskill( TINKERING, 0, 500 ):
		if random.randint( 1, 100 ) <= 25:
			additional = tr(" and break the parts.")

			if item.amount > 1:
				item.amount -= 1
				item.update()
			else:
				item.delete()
		else:
			additional = "."

		char.message( "%s%s" % ( tr("You fail to create the sextant"), additional ) )
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
		char.message( tr("You put the sextant into your backpack") )

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
		char.message( 1042001 ) # That must be in your pack for you to use it.
		return 1

	gump = cGump( x=50, y=50, callback=hairdye_callback )

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
		char.message( 1042001 ) # That must be in your pack for you to use it.
		return

	# Check if it's a valid color
	if len( response.switches ) != 1:
		char.message( tr("You have to choose a hair color.") )
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

	char.message( tr("That is an invalid color.") )

# Dying Tub
def dyingtub( char, item ):
	if not char.canreach( item, 2 ):
		char.socket.clilocmessage( 1019045 ) # I can't reach that.
		return 1

	char.socket.sysmessage( tr("Please select the object you wish to dye.") )
	char.socket.attachtarget( 'environment.dyingtub_response', [ item.serial ] )
	return 1

def dyingtub_response( char, args, target ):
	dyetub = wolfpack.finditem( args[0] )

	if not dyetub or not char.canreach( dyetub, 2 ):
		char.message( 1019045 ) # I can't reach that.
		return

	if not target.item:
		char.message( tr("You must target an item.") )
		return

	# Valid Target?
	if not char.gm:
		if target.item.getoutmostchar() != char:
			char.socket.clilocmessage( 1042001 ) # That must be in your pack for you to use it.
			return

		if not target.item.dye:
			char.socket.clilocmessage( 1042083 ) # You cannot dye that.
			return

	char.log( LOG_MESSAGE, "Dying item (%x,%x) using tub (%x,%x)\n" % ( target.item.serial, target.item.color, dyetub.serial, dyetub.color ) )
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

			# Sextant Parts
			0x1059: sextant_parts,
			0x105a: sextant_parts,

			# Hair Dyes
			0xe27: hairdye,
			0xeff: hairdye,

			# Dye Tub
			0xfab: dyingtub
		}

def onUse( char, item ):
	if not item.id in actions:
		return 0

	return actions[ item.id ]( char, item )
