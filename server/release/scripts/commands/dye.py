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

def dye( socket, command, arguments ):
	try:
		color = int( hex2dec( str(arguments) ) )
		socket.attachtarget( "commands.dye.response", [color] )
	except:
		socket.sysmessage( 'Usage: dye <color-id>' )

def response( char, args, target ):
	if target.item:
		color = args[0]
		target.item.color = hex(color)
		target.item.update()
	elif target.char:
		skin = args[0]
		if skin == 0:
			skin = target.char.orgskin
		target.char.skin = hex(skin)
		target.char.update()
	else:
		char.socket.sysmessage( 'That was not a valid object.', GRAY )
	return

def onLoad():
	wolfpack.registercommand( "dye", dye )
