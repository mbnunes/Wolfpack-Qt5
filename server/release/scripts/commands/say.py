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
		socket.sysmessage( "Usage '.say <blabla>'" )
		return True
	socket.sysmessage( "What should say that?" )
	socket.attachtarget( "commands.say.saytarget", [ arguments ] )

def saytarget( char, args, target ):
	if target.char:
		if target.char.socket:
			target.char.say( "%s" % unicode(args[0]) )
			return True
	if target.item:
		char.socket.showspeech( target.item, "%s" % unicode(args[0]) )
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