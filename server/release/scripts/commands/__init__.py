
import wolfpack
from wolfpack import time

def season( socket, command, arguments ):
	packet = [ 0xbc, int(arguments), 0x01 ]
	socket.sendpacket(packet)

#def enablegod( socket, command, arguments ):
#	packet = [ 0x2b, 0x01 ]
#	socket.sendpacket( packet )

def updateplayer(socket, command, arguments):
	socket.updateplayer()

def uotime(socket, command, arguments):
	socket.sysmessage('Current Time: %02u:%02u' % (time.hour(), time.minute()))
	socket.sysmessage('Current Lightlevel: %u' % (time.currentlightlevel()))

def onLoad():
	wolfpack.registercommand("season", season)
	wolfpack.registercommand("updateplayer", updateplayer)
	wolfpack.registercommand("time", uotime)

"""
	\command time
	\description Show the current ingame time and the current lightlevel.
"""

"""
	\command updateplayer
	\description Resends your character to your socket.
"""
	
"""
	\command season
	\description Sends a season change to the current client.
	\usage - <code>season id</code>	
	Id is the season you want to change to. 
	Possible values are:
	0: Spring
	1: Summer
	2: Fall
	3: Winter
	4: Desolation
	\notes This command is mainly for testing purposes. It doesn't permanently change the season for your client.
"""
