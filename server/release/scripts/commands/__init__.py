
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
	socket.sysmessage('Elapsed Minutes: %u. Elapsed Days: %u.' % (time.minutes(), time.days()))
	socket.sysmessage('Current Lightlevel: %u' % (time.currentlightlevel()))

def nudgetarget(player, arguments, target):
	if target.item:
		pos = target.item.pos
		pos.z += arguments[0]
		target.item.moveto(pos)
		target.item.update()

	elif target.char:
		pos = target.char.pos
		pos.z += arguments[0]
		target.char.moveto(pos)
		target.char.update()

	else:
		player.socket.sysmessage('You have to target an item or character.')	

"""
	\command nudgedown
	\usage - <code>nudgedown</code>
	- <code>nudgedown amount</code>
	\description Decreases the targetted objects z level. If no amount is given, 
	an amount of 1 is assumed.
"""

def nudgedown(socket, command, arguments):
	amount = 1
	try:
		amount = int(arguments)
	except:
		pass
		
	socket.attachtarget('commands.nudgetarget', [- amount])
	
"""
	\command nudgeup
	\usage - <code>nudgeup</code>
	- <code>nudgeup amount</code>
	\description Increases the targetted objects z level. If no amount is given, 
	an amount of 1 is assumed.
"""
	
def nudgeup(socket, command, arguments):
	amount = 1
	try:
		amount = int(arguments)
	except:
		pass
		
	socket.attachtarget('commands.nudgetarget', [amount])

def onLoad():
	wolfpack.registercommand("season", season)
	wolfpack.registercommand("updateplayer", updateplayer)
	wolfpack.registercommand("time", uotime)
	wolfpack.registercommand("nudgeup", nudgeup)
	wolfpack.registercommand("nudgedown", nudgedown)

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
