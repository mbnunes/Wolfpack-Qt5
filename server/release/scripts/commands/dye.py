#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts
#  ((    _/{  "-;  | Created by: Dreoth
#   )).-' {{ ;'`   | Revised by:
#  ( (  ;._ \\ ctr | Last Modification: Created
#===============================================================#
# .dye Command
#===============================================================#

"""
	\command dye
	\description Changes the color of the targetted item or skin of the targetted character.
	\usage - <code>dye color</code>
	The color can either be passed as a decimal number or in the standard hexadecimal notation.
"""

import wolfpack
from wolfpack.utilities import hex2dec
from wolfpack.consts import *

def dye( socket, command, arguments ):
	try:
		color = int( hex2dec( str(arguments) ) )
		socket.attachtarget( "commands.dye.response", [color] )
	except:
		socket.sysmessage( 'Usage: dye <color-id>' )

def response( char, args, target ):
	if target.item:
		color = args[0]
		oldcolor = target.item.color
		target.item.color = hex(color)
		target.item.update()
		
		char.log(LOG_MESSAGE, "Redyes item 0x%x from color 0x%x to 0x%x.\n" % (target.item.serial, oldcolor, target.item.color))
	elif target.char:
		skin = args[0]
		if skin == 0:
			skin = target.char.orgskin
		oldskin = target.char.skin
		target.char.skin = hex(skin)
		target.char.update()
		
		char.log(LOG_MESSAGE, "Redyes char 0x%x from skin 0x%x to 0x%x.\n" % (target.char.serial, oldskin, target.char.skin))
	else:
		char.socket.sysmessage( 'That was not a valid object.', GRAY )
	return

def onLoad():
	wolfpack.registercommand( "dye", dye )
