#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Created by: Naddel                         #
#   )).-' {{ ;'`   | Revised by:                                #
#  ( (  ;._ \\ ctr | Last Modification: Created                 #
#===============================================================#
# .showskilltitles Command                                      #
#===============================================================#

import wolfpack
from wolfpack.settings import getbool

def showskilltitles( socket, command, arguments ):
	if getbool( "General", "ShowSkillTitles", True ):
		pre = "disabled"
		if socket.player.hastag( "showskilltitles" ):
			socket.player.deltag( "showskilltitles" )
		else:
			socket.player.settag( "showskilltitles", 1 )
			pre = "enabled"
		socket.sysmessage( "ShowSkillTitles is now " + pre + "!" )

def onLoad():
	wolfpack.registercommand( "showskilltitles", showskilltitles )

"""
	\command showskilltitles
	\description Indicates if the title of other chars is shown in their paperdolls.
	\notes The title is not shown if the other char has staff enabled or this setting is disabled in wolfpack.xml.
"""
