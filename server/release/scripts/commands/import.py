#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Created by: Correa                         #
#   )).-' {{ ;'`   | Revised by:                                #
#  ( (  ;._ \\ ctr | Last Modification: Created                 #
#===============================================================#
# .export Command						#
#===============================================================#

"""
	\command import
	\description Import items from external worldfiles.
	\notes The following formats are supported:

	<b>Multi</b>
	Each line in this format represents one item. The line
	has the following format:
	id x y z show

	Id is the (decimal or hexadecimal) item display id.
	X and y are the relative coordinates of the item.
	Z is the relative z position of the item.
	Show is ignored.

	Your current position is used as the point of origin.

	Example: 0xEED -1 -1 12 0

	<b>WSC</b>
	This format is roughly equivalent to the WSC format
	exported by sphere and the WSC format used by UOX, NOX and
	derivates.

	<b>Text</b>
	This format is similar to the Multi format above.
	The order of fields per line is:

	id x y z color

	X, y and z are absolute coordinates.
	Color is the desired item color.
"""

import wolfpack
from wolfpack.consts import *
from wolfpack.gumps import cGump
from wolfpack.utilities import hex2dec
from wolfpack import console
from string import lstrip
import os

def import_command( socket, command, arguments ):
	char = socket.player

	gump = cGump( x=100, y=100, callback="commands.import.callback" )

	gump.addBackground( id=0x2436, width=350, height=300 )

	text = '<basefont color="#FEFEFE"><h3>Import - <basefont color="#ff0000">Items Only</basefont></h3><br />Enter the name of the worldfile you want to import below. But before clicking Import, choose the correct file format from the list.</basefont>'
	gump.addHtmlGump( x=20, y=20, width=310, height=200, html=text )

	# Radiobuttons
	gump.startGroup( 0 )
	gump.addRadioButton( x=20, y=160, off=0x25f8, on=0x25fb, id=1 )
	gump.addText( x=55, y=165, text='Multi Txt', hue=0x835 )

	gump.addRadioButton( x=150, y=160, off=0x25f8, on=0x25fb, id=2, selected=1 )
	gump.addText( x=185, y=165, text='WSC', hue=0x835 )

	gump.addRadioButton( x=250, y=160, off=0x25f8, on=0x25fb, id=3 )
	gump.addText( x=285, y=165, text='Text', hue=0x835 )

	gump.addRadioButton( x=20, y=120, off=0x25f8, on=0x25fb, id=4 )
	gump.addText( x=55, y=125, text='Sphere 51a', hue=0x835 )

	#gump.addRadioButton( x=150, y=120, off=0x25f8, on=0x25fb, id=5 )
	#gump.addText( x=185, y=125, text='Sphere 55i', hue=0x835 )

	# InputField
	gump.addResizeGump( x=20, y=210, id=0xBB8, width=310, height=25 )
	gump.addInputField( x=25, y=212, width=295, height=20, hue=0x834, id=1, starttext="export.wsc" )

	gump.addText( x=265, y=250, text='Import', hue=0x835 )
	gump.addButton( x=310, y=250, up=0x26af, down=0x26b1, returncode=1 )

	gump.send( char )

def onLoad():
	wolfpack.registercommand( "import", import_command )

"""
	Parse multi txt file
"""
def parseMulti( file, pos ):
  warnings = ''
  count = 0

  for line in file:
    # Replace \r and \n's
    line = line.replace( "\r", "" )
    line = line.replace( "\n", "" )

    if len(line.split(' ')) != 5:
      continue

    ( id, x, y, z, show ) = line.split(' ')

    if not int(show):
      continue

    id = hex2dec( id )
    x = int( x )
    y = int( y )
    z = int( z )
    newitem = wolfpack.newitem(1) # Generate a new serial for us

    newitem.decay = 0
    newitem.id = id

    newitem.moveto( pos.x + x, pos.y + y, pos.z + z, pos.map, 1 )
    newitem.update()
    count += 1

  return ( count, warnings )

"""
	Parse .txt file
"""
def parseTxt( file, map ):
	warnings = ''
	count = 0

	parseTickCount = 0
	createTickCount = 0
	propTickCount = 0
	moveTickCount = 0

	for line in file:
		step1 = wolfpack.tickcount()

		# Replace \r and \n's
		line = line.replace( "\r", "" )
		line = line.replace( "\n", "" )

		( id, x, y, z, color ) = line.split( ' ' )

		id = hex2dec( id )
		baseid = '%x' % id
		color = hex2dec( color )
		x = int( x )
		y = int( y )
		z = int( z )

		step2 = wolfpack.tickcount()
		newitem = wolfpack.additem( '%s' % baseid ) # Generate a new serial for us
		step3 = wolfpack.tickcount()

		newitem.decay = 0
		newitem.color = color
		newitem.id = id

		step4 = wolfpack.tickcount()

		newitem.moveto( x, y, z, map, 1 )

		step5 = wolfpack.tickcount()

		parseTickCount += step2 - step1
		createTickCount += step3 - step2
		propTickCount += step4 - step3
		moveTickCount += step5 - step4

		newitem.update()

		count += 1

	print "Parsing: %i ticks" % parseTickCount
	print "Creating: %i ticks" % createTickCount
	print "Prop: %i ticks" % propTickCount
	print "Move: %i ticks" % moveTickCount

	return ( count, warnings )

def parseSphere51a(file, map):
	itemid = -1
	props = {}
	count = 0
	warnings = ''

	while 1:
		line = file.readline()
		if not line:
			break
		line = line.strip()

		if line.startswith('[WORLDITEM ') and line.endswith(']'):
			itemid = int(line[11:len(line)-1], 16)
		elif itemid != -1 and "=" in line:
			(key, value) = line.split('=', 1)
			props[key.lower()] = value
		elif itemid != -1 and len(line) == 0:
			if props.has_key('p'):
				(x, y, z) = props['p'].split(',')
				x = int(x)
				y = int(y)
				z = int(z)
				if props.has_key('color'):
			 		color = int(props['color'], 16)
			 	else:
			 		color = 0

				item = wolfpack.newitem(1)
				item.decay = 0
				item.movable = 2
				item.id = itemid
				item.color = color
				item.moveto( x, y, z, map )
				item.update()
				count += 1

			itemid = -1
			props = {}

	return (count, warnings)

def parseSphere55i(file, map):
	"""
	itemid = -1
	props = {}
	count = 0
	warnings = ''

	while 1:
		line = file.readline()
		if not line:
			break
		line = line.strip()

		if line.startswith('[WORLDITEM ') and line.endswith(']'):
			itemid = int(line[11:len(line)-1], 16)
		elif itemid != -1 and "=" in line:
			(key, value) = line.split('=', 1)
			props[key.lower()] = value
		elif itemid != -1 and len(line) == 0:
			if props.has_key('p'):
				(x, y, z) = props['p'].split(',')
				x = int(x)
				y = int(y)
				z = int(z)
				if props.has_key('color'):
			 		color = int(props['color'], 16)
			 	else:
			 		color = 0

				item = wolfpack.newitem(1)
				item.decay = 0
				item.movable = 2
				item.id = itemid
				item.color = color
				item.moveto( x, y, z, map )
				item.update()
				count += 1

			itemid = -1
			props = {}

	return (count, warnings)
	"""
	pass

"""
	Parses a .wsc file and imports the items into our world.
"""
def parseWsc( file, map ):
	warnings = ''
	count = 0

	while 1:
		line = file.readline()

		if not line:
			break

		# Replace \r and \n's
		line = line.replace( "\r", "" )
		line = line.replace( "\n", "" )

		# SECTION WORLDITEM : 17 byte
		if line[:17] == 'SECTION WORLDITEM':
			item = {}

			while 1:
				line = file.readline()

				if not line or len( line ) == 0 or line[0] == '}':
					break

				if line[0] == '{':
					continue

				line = line.replace( "\r", "" )
				line = line.replace( "\n", "" )

				if line.find( ' ' ) == -1:
					continue

				# Space has to be in there
				( key, value ) = line.split( ' ', 1 )

				item[ key ] = value

			# Check if we can import this item
			serial = 'Unset'
			if item.has_key( 'SERIAL' ):
				serial = hex( int( item[ 'SERIAL' ] ) )

			if item.has_key( 'BASEID' ):
				baseid = item[ 'BASEID' ]
			else:
				baseid = ''

			if not item.has_key( 'ID' ):
				warnings += 'Item (%s) has no ID property. Skipping.<br>' % ( serial )
				continue

			id = int( item['ID'] )

			if not item.has_key( 'X' ) or not item.has_key( 'Y' ):
				warnings += 'Item (Serial: %s, ID: %x) has no X or Y property. Skipping.<br>' % ( serial, id )
				continue

			if item.has_key('X'):
				x = int( item['X'] )
			else:
				x = 0
			if item.has_key('Y'):
				y = int( item['Y'] )
			else:
				y = 0
			if item.has_key('Z'):
				z = int( item['Z'] )
			else:
				z = 0
			if item.has_key('MAP'):
				map = int( item['MAP'] )

			color = 0
			if item.has_key( 'COLOR' ):
				color = int( item[ 'COLOR' ] )

			if item.has_key( 'CONT' ) and item[ 'CONT' ] != '-1':
				# warnings += 'Item (Serial: %s, ID: %x) is contained in container %s. Skipping' % ( serial, id, item[ 'CONT' ] )
				continue

			amount = 1
			if item.has_key( 'AMOUNT' ):
				amount = int( item[ 'AMOUNT' ] )

			name = '#'
			if item.has_key( 'NAME' ):
				name = item[ 'NAME' ]

			if item.has_key( 'TYPE' ):
				type = item[ 'TYPE' ]

			#print 'Item %x, Color %x, Pos %i,%i,%i<br>' % ( id, color, x, y, z )

			# REMEMBER: Set them to nodecay!!!
			if baseid != '':
				newitem = wolfpack.additem( "%s" % baseid ) # Generate a new serial for us
			else:
				baseid = lstrip( str( hex( id ) ), "0x" )
				
				# Multi ?
				if id >= 0x4000:
					newitem = wolfpack.addmulti( "%s" % baseid ) # Generate a new serial for us
				else:
					newitem = wolfpack.additem( "%s" % baseid ) # Generate a new serial for us

			if not newitem:
				warnings += "Found an invalid item id '%s' at %s<br>" % (baseid, str(newposition))
				continue

			newitem.decay = 0
			newitem.movable = 3 # Make everything gm movable by default
			newitem.color = color
			newitem.id = id
			newitem.amount = amount
			if name != "#":
				newitem.name = name
			newposition = "%i,%i,%i,%i" % (x, y, z, map)
			newitem.pos = newposition
			#newitem.moveto( x, y, z, map )
			newitem.update()

			count += 1

		elif line[:17] == 'SECTION WORLDCHAR':
			warning += "Found character in import file. Importing characters is not allowed.<br>"

	return ( count, warnings )

def callback( char, args, choice ):
	if choice.button == 0:
		return

	filename = choice.text[1]

	if len( filename ) == 0:
		char.socket.sysmessage( "You have to specify a filename." )

	file = open( filename, 'rtu' ) # We know it's \r\n

	format = choice.switches[0]

	if format == 1: #Multi
		( count, warnings ) = parseMulti( file, char.pos )
	elif format == 2: #WSC
		( count, warnings ) = parseWsc( file, char.pos.map )
	elif format == 3: # Text
		( count, warnings ) = parseTxt( file, char.pos.map )
	elif format == 4: # Sphere 51a
		(count, warnings) = parseSphere51a(file, char.pos.map)
	elif format == 5: # Sphere 55i
		(count, warnings) = parseSphere55i(file, char.pos.map)

	file.close()

	# Show a Report gump
	gump = cGump( x=100, y=100 )

	gump.addBackground( id=0x2436, width=350, height=300 )

	text = '<basefont color="#FEFEFE"><h3>Import</h3><br><basefont color="#FEFEFE">%d items have been imported from "%s".<br><br><basefont color="#ff0000"><u>Warnings:</u><br><basefont color="#FEFEFE">%s' % ( count, filename, warnings )
	gump.addHtmlGump( x=20, y=20, width=310, height=200, html=text, canScroll=1 )

	gump.addText( x=265, y=250, text='Close', hue=0x835 )
	gump.addButton( x=310, y=250, up=0x26af, down=0x26b1, returncode=0 )

	gump.send( char )
