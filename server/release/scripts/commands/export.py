#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Created by: Correa                         #
#   )).-' {{ ;'`   | Revised by:                                #
#  ( (  ;._ \\ ctr | Last Modification: Created                 #
#===============================================================#
# .export Command						#
#===============================================================#

"""
	\command export
	\description Export worlditems into three different formats.
	\notes The following two formats are supported:

	<b>Sphere</b>
	The sphere format consists of a number of item blocks
	which look like the following one:

	<code>[WORLDITEM 0efa]
	SERIAL=040000001
	NAME=a custom item name
	ID=0efa
	COLOR=0
	P=1250,1200,0</code>

	<b>WSC</b>
	This format is roughly equivalent to the WSC format
	exported by sphere and the WSC format used by UOX, NOX and
	derivates.

	<b>Text</b>
	Each line in this format represents one item. The line
	has the following format:
	id x y z color

	Id is the (decimal or hexadecimal) item display id.
	X, y and z are the coordinates of the item.
	Color is the item color.

	Example: 0xEED 1351 1200 12 0
"""

import wolfpack
from wolfpack.gumps import cGump
import os

# BaseID's not to save.
nonsaves = ['gem','ore_gem','wood_gem']

def exportcmd( socket, command, arguments ):

	socket.sysmessage( "Target the upper left corner of the area you want to export." )
	socket.attachtarget( "commands.export.callback", [] )

def onLoad():
	wolfpack.registercommand( "export", exportcmd )

def callback( char, args, target ):
	socket = char.socket

	# Upper left Corner
	if len( args ) == 0:
		if target.item and target.item.container:
			socket.sysmessage( "This is an invalid target." )
			return

		socket.sysmessage( "Target the lower right corner of the area you want to export." )
		socket.attachtarget( "commands.export.callback", [ target.pos.x, target.pos.y ] )

	# Lower Right Corner
	elif len( args ) == 2:
		if target.item and target.item.container:
			socket.sysmessage( "This is an invalid target." )
			return

		# Create a gump to enter the output filename
		x1 = min( args[0], target.pos.x )
		x2 = max( args[0], target.pos.x )
		y1 = min( args[1], target.pos.y )
		y2 = max( args[1], target.pos.y )

		gump = cGump( x=100, y=100, callback="commands.export.export", args=[ x1, y1, x2, y2 ] )

		gump.addBackground( id=0x2436, width=350, height=300 )
		#gump.addCheckerTrans( 15, 15, 320, 270 )

		text = '<basefont color="#FEFEFE"><h3>Export</h3><br><basefont color="#FEFEFE"><u>X1</u>,<u>Y1</u>: %u,%u<br><u>X2</u>,<u>Y2</u>: %u,%u<br><br>Enter the filename for your exported items below. Use the radio buttons to choose the export format.' % ( x1, y1, x2, y2 )
		gump.addHtmlGump( x=20, y=20, width=310, height=200, html=text )

		# Radiobuttons
		gump.startGroup( 0 )
		gump.addRadioButton( x=20, y=160, off=0x25f8, on=0x25fb, id=1 )
		gump.addText( x=55, y=165, text='Sphere 51a', hue=0x835 )

		gump.addRadioButton( x=150, y=160, off=0x25f8, on=0x25fb, id=2, selected=1 )
		gump.addText( x=185, y=165, text='WSC', hue=0x835 )

		gump.addRadioButton( x=250, y=160, off=0x25f8, on=0x25fb, id=3 )
		gump.addText( x=285, y=165, text='Text', hue=0x835 )

		# InputField
		gump.addResizeGump( x=20, y=210, id=0xBB8, width=310, height=25 )
		gump.addInputField( x=25, y=212, width=295, height=20, hue=0x834, id=1, starttext="export.wsc" )

		gump.addText( x=265, y=250, text='Export', hue=0x835 )
		gump.addButton( x=310, y=250, up=0x26af, down=0x26b1, returncode=1 )

		gump.send( char )

def export( char, args, choice ):
	if choice.button != 1 or len( args ) != 4:
		return 1

	filename = choice.text[1]

	if len( choice.switches ) != 1:
		char.socket.sysmessage( "Error: len(choice.switches) != 1." )
		return 1

	format = choice.switches[0]

	if len( filename ) == 0:
		char.socket.sysmessage( "Error: You need to provide a valid filename." )
		return 1

	# Open the output file
	output = open( filename, "wb" )	# Note that we *force* the output to have lines terminated with \n\r

	iterator = wolfpack.itemregion( args[0], args[1], args[2], args[3], char.pos.map )
	warnings = ''

	if os.name == 'posix':
		newline = "\n"
	else:
		newline = "\r\n"

	item = iterator.first
	i = 0
	while item:
		if not item.baseid in nonsaves and len( item.spawnregion ) == 0
			# Build our string
			if format == 1: # Sphere 51a
				output.write( "[WORLDITEM 0%x]%s" % ( item.id, newline ) )
				output.write( "SERIAL=0%x%s" % ( item.serial, newline ) )
				if item.name != '#':
					output.write( "NAME=%s%s" % ( item.name, newline ) )
				output.write( "ID=0%x%s" % ( item.id, newline ) )
				output.write( "COLOR=0%x%s" % ( item.color, newline ) )
				output.write( "P=%i,%i,%i%s%s" % ( item.pos.x, item.pos.y, item.pos.z, newline ) )

			elif format == 2: # WSC, Lonewolf Style, Compatible with Linux Worldforge
				output.write( "SECTION WORLDITEM%s" % newline )
				output.write( "{%s" % newline )
				output.write( "SERIAL %i%s" % ( item.serial, newline ) )
				output.write( "ID %i%s" % ( item.id, newline ) )
				if item.baseid != '':
					output.write( "BASEID %s%s" % ( item.baseid, newline ) )
				if item.name != '#' or item.name != '':
					output.write( "NAME %s%s" % ( item.name, newline ) )

				output.write( "X %i%s" % ( item.pos.x, newline ) )
				output.write( "Y %i%s" % ( item.pos.y, newline ) )
				output.write( "Z %i%s" % ( item.pos.z, newline ) )
				output.write( "MAP %i%s" % ( item.pos.map, newline ) )
				if item.type:
					output.write( "TYPE %i%s" % ( item.type, newline ) ) # World Freeze Requirement
				else:
					output.write( "TYPE 255%s" % newline ) # World Freeze Requirement
				output.write( "COLOR %i%s" % ( item.color, newline ) )
				output.write( "CONT -1%s" % newline )
				output.write( "}%s%s" ( newline, newline ) )

			else: # Text
				output.write( "%s 0x%x %i %i %i %i 0x%x%s" % ( item.baseid, item.id, item.pos.x, item.pos.y, item.pos.z, item.pos.map, item.color, newline ) )
				# Older Format, no baseid/map saved
				#output.write( "0x%x %i %i %i 0x%x%s" % ( item.id, item.pos.x, item.pos.y, item.pos.z, item.color, newline ) )

			if item.amount > 1:
				warnings += 'Item %i has an amount of %i. This information will be lost when made static.<br><br>' % ( hex( item.serial ), item.amount )

			eventlist = item.eventlist
			if len( eventlist ) > 0:
				warnings += 'Item %i has events (%s) assigned to it. It wont be usable when made static.<br><br>' % ( hex( item.serial ), eventlist )

			if item.type != 0:
			warnings += 'Item %i is of type %i. It wont be usable when made static.<br><br>' % ( hex( item.serial ), item.type )

			i += 1

		item = iterator.next

	output.close()

	# Show a Report gump
	gump = cGump( x=100, y=100 )

	gump.addBackground( id=0x2436, width=350, height=300 )
	#gump.addCheckerTrans( 15, 15, 320, 270 )

	text = '<basefont color="#FEFEFE"><h3>Export</h3><br><basefont color="#FEFEFE">%d items have been exported to "%s".<br><br><basefont color="#ff0000"><u>Warnings:</u><br><basefont color="#FEFEFE">%s' % ( i, filename, warnings )
	gump.addHtmlGump( x=20, y=20, width=310, height=200, html=text, canScroll=1 )

	gump.addText( x=265, y=250, text='Close', hue=0x835 )
	gump.addButton( x=310, y=250, up=0x26af, down=0x26b1, returncode=0 )

	gump.send( char )
