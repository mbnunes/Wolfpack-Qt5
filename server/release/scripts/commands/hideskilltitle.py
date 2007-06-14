#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Created by: Naddel                         #
#   )).-' {{ ;'`   | Revised by:                                #
#  ( (  ;._ \\ ctr | Last Modification: Created                 #
#===============================================================#
# .hideskilltitle Command                                       #
#===============================================================#

import wolfpack
from wolfpack.settings import getbool

def hideskilltitle( socket, command, arguments ):
	if getbool( "General", "ShowSkillTitles", True ):
		pre = "disabled"
		if socket.player.hastag( "hideskilltitle" ):
			socket.player.deltag( "hideskilltitle" )
		else:
			socket.player.settag( "hideskilltitle", 1 )
			pre = "enabled"
		socket.sysmessage( "HideSkillTitle is now " + pre + "!" )

def onLoad():
	wolfpack.registercommand( "hideskilltitle", hideskilltitle )

"""
	\command hideskilltitle
	\description Indicates if other players can see the skill title in the paperdoll
"""
