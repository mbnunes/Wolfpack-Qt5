#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Created by: Naddel                         #
#   )).-' {{ ;'`   | Revised by:                                #
#  ( (  ;._ \\ ctr | Last Modification: Created                 #
#===============================================================#
# .say  Command                                                 #
#===============================================================#

import wolfpack

def say( socket, command, arguments ):
	if len(arguments) == 0:
		socket.sysmessage( "Usage '.say <text>'" )
		return True
	socket.sysmessage( "" )
	socket.attachtarget( "commands.say.saytarget", [ arguments ] )

def saytarget( char, args, target ):
	if target.char:
		target.char.say( unicode(args[0]) )
	elif target.item:
		target.item.say( unicode(args[0]) )
	else:
		char.socket.sysmessage( "That's not a valid object." )

def onLoad():
	wolfpack.registercommand( "say", say )
	
"""
	\command say
	\description Let a char say the text or show the text over an item.
	\usage - <code>say text</code>
	The text is shown over the chosen char or item. If the target is a char,
	the speechcolor of that char will be used.
"""