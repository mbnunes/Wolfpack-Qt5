#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Created by: Incanus                        #
#   )).-' {{ ;'`   | Revised by:                                #
#  ( (  ;._ \\ ctr | Last Modification: Created                 #
#===============================================================#
# .exportstatics Command					#
#===============================================================#

"""
	\command exportstatics
	\description Export static items.
	\notes Only the Text format is supported:

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

def exportcmd( socket, command, arguments ):
	socket.sysmessage( "Target the upper left corner of the area you want to export." )
	socket.attachtarget( "commands.exportstatics.callback", [] )
	return

def callback( char, args, target ):
	socket = char.socket

	# Upper left Corner
	if len( args ) == 0:
		if target.item and target.item.container:
			socket.sysmessage( "This is an invalid target." )
			return

		socket.sysmessage( "Target the lower right corner of the area you want to export." )
		socket.attachtarget( "commands.exportstatics.callback", [ target.pos.x, target.pos.y ] )

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

		gump = cGump( x=100, y=100, callback=export, args=[ x1, y1, x2, y2 ] )

		gump.addBackground( id=0x2436, width=350, height=300 )

		text = '<basefont color="#FEFEFE"><h3>Export</h3><br><basefont color="#FEFEFE"><u>X1</u>,<u>Y1</u>: %u,%u<br><u>X2</u>,<u>Y2</u>: %u,%u' % ( x1, y1, x2, y2 )
		gump.addHtmlGump( x=20, y=20, width=310, height=200, html=text )

		# InputField
		gump.addResizeGump( x=20, y=210, id=0xBB8, width=310, height=25 )
		gump.addInputField( x=25, y=212, width=295, height=20, hue=0x834, id=1, starttext="export.txt" )

		gump.addText( x=265, y=250, text='Export', hue=0x835 )
		gump.addButton( x=310, y=250, up=0x26af, down=0x26b1, returncode=1 )

		gump.send( char )

def export( char, args, choice ):
	if choice.button != 1 or len( args ) != 4:
		return False

	filename = choice.text[1]

	if len( filename ) == 0:
		char.socket.sysmessage( "Error: You need to provide a valid filename." )
		return False

	# Open the output file
	output = open( filename, "wb" )	# Note that we *force* the output to have lines terminated with \n\r

	if os.name == 'posix':
		newline = "\n"
	else:
		newline = "\r\n"
	warnings = ''

	i = 0

	# There is no iterator for static items, so we have to check each coord
	for x in range(args[0], args[2]+1):
		for y in range(args[1], args[3]+1):
			# Check for static items
			statics = wolfpack.statics(x, y, char.pos.map, True)
			for item in statics:
				output.write( "0x%x %i %i %i 0x%x%s" % (item[0], item[1], item[2], item[3], item[4], newline ) )
				i = i + 1

	output.close()

	# Show a Report gump
	gump = cGump( x=100, y=100 )

	gump.addBackground( id=0x2436, width=350, height=300 )

	text = '<basefont color="#FEFEFE"><h3>Export</h3><br><basefont color="#FEFEFE">%d items have been exported to "%s".<br><br><basefont color="#ff0000"><u>Warnings:</u><br><basefont color="#FEFEFE">%s' % ( i, filename, warnings )
	gump.addHtmlGump( x=20, y=20, width=310, height=200, html=text, canScroll=1 )

	gump.addText( x=265, y=250, text='Close', hue=0x835 )
	gump.addButton( x=310, y=250, up=0x26af, down=0x26b1, returncode=0 )

	gump.send( char )
	return True

def onLoad():
	wolfpack.registercommand( "exportstatics", exportcmd )
	return
