#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Created by: codex                          #
#   )).-' {{ ;'`   | Revised by:                                #
#  ( (  ;._ \\ ctr | Last Modification: Created                 #
#===============================================================#
# stat Command  						#
#===============================================================#

"""
	\command stat
	\description Change the strength, dexterity or intelligence of a character.
	\usage - <code>stat str value</code>
	- <code>stat dex value</code>
	- <code>stat int value</code>
	Changes either the strength (str), dexterity (dex) or 
	intelligence(int) of the character to the desired value.
"""

import wolfpack
import string
from wolfpack.consts import *

usage =   'Usage: stat <short-stat-name> <value>'
example = 'Example: stat str 100'

def stat( socket, command, args ):
	args = args.strip()   # Remove trailing and leading whitespaces
	if len( args ) > 0:
		try:
			( stat, value ) = args.split( ' ' )
			stat = str( stat )
			value = int( value )
		except:
			socket.sysmessage( usage )
			socket.sysmessage( example )
			return OK

	else:
		socket.sysmessage( usage )
		socket.sysmessage( example )
		return OK

	# Uknown stat name ?
	if not stat in statnames:
		socket.clilocmessage( 3000380, "", YELLOW, NORMAL ) # I Accept
		socket.sysmessage( str( statnames ) )
		return OK

	# Accept 10 >= value <= 125 only.
	elif value < 0 or value > 125:
		socket.clilocmessage( 1005628, "", YELLOW, NORMAL ) # Stats range between 10 and 125
		return OK

	socket.clilocmessage( 503403, "", YELLOW, NORMAL ) # Select the body.
	socket.attachtarget( "commands.stat.callback", [ stat, value ] )

	return OK

def callback( char, args, target ):
	socket = char.socket

	# What are you targeted ?
	if not target.char:
		socket.clilocmessage( 500931, "", YELLOW, NORMAL ) # Invalid mobile
		return OK

	# Is target not your own char ?
	if not char == target.char:
		socket.clilocmessage( 1005213, "", YELLOW, NORMAL ) # You can't do that
		return OK

	( stat, value ) = args

	if stat == 'str':
		char.strength = value
	elif stat == 'int': 
		char.intelligence = value
	elif stat == 'dex': 
		char.dexterity = value
	else:
		socket.clilocmessage( 3000380, "", YELLOW, NORMAL ) # I Accept
		socket.sysmessage( str( statnames ) )
		return OK

	socket.clilocmessage( 1005630, "", YELLOW, NORMAL ) # Your stats have been adjusted.
	
	char.updatestats()
		
	return OK		

def onLoad():
	wolfpack.registercommand( "stat", stat )

