#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Created by: Naddel                         #
#   )).-' {{ ;'`   | Revised by:                                #
#  ( (  ;._ \\ ctr | Last Modification: Created                 #
#===============================================================#
# .showskilltitles Command                                      #
#===============================================================#

import wolfpack
from wolfpack.utilities import booleantoggle
from wolfpack.settings import getbool

def showskilltitles( socket, command, arguments ):
	if getbool( "General", "ShowSkillTitles", True ):
		booleantoggle( socket.player.showskilltitles )
		socket.sysmessage( "ShowSkillTitles is now '%i'." % char.showskilltitles )

def onLoad():
	wolfpack.registercommand( "showskilltitles", showskilltitles )

"""
	\command bank
	\description Shows the content of a container on another character.
	\usage - <code>bank [layer]</code>
	If layer is omitted, the bank box of the targetted character is shown. Otherwise
	the server sends the contents of the container on the given layer.
	\notes The container on the given layer is not automatically created if it doesn't exist.
"""
