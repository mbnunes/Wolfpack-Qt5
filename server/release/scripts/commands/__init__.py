
import wolfpack
from wolfpack import time
from wolfpack.utilities import hex2dec
from wolfpack.gumps import cGump

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

def resendtooltiptarget(player, arguments, target):
	if target.item:
		target.item.resendtooltip()
	elif target.char:
		target.char.resendtooltip()
	else:
		player.socket.sysmessage('You have to target either a character or item.')

"""
	\command resendtooltip
	\description Resend the tooltip of an object. This is mainly for debugging purposes.
"""

def resendtooltip(socket, command, arguments):
	socket.sysmessage('Which objects tooltip do you want to resend?')
	socket.attachtarget('commands.resendtooltiptarget', [])

def nightsight(socket, command, arguments):
	player = socket.player
	player.removescript('magic.nightsight')

	if player.hastag('nightsight'):
		bonus = player.gettag('nightsight')
		player.lightbonus = max(0, player.lightbonus - bonus)
		player.deltag('nightsight')
	else:
		player.settag('nightsight', 255)
		player.lightbonus = 255
	socket.updatelightlevel()

"""
	\command multigems
	\description Toggle the multigems flag for the current account.
	If this flag is on, multis will be sent as worldgems instead
	of the real multi object.
"""
def multigems(socket, command, arguments):
	socket.account.multigems = not socket.account.multigems
	socket.resendworld(1)

	if socket.account.multigems:
		socket.sysmessage("'multigems' is now on.")
	else:
		socket.sysmessage("'multigems' is now off.")

	socket.resendworld()

"""
	\command gouid
	\usage - <code>gouid serial</code>
	\description Find an object with the given serial and move to it.
"""
def gouid(socket, command, arguments):
	try:
		uid = hex2dec(arguments)
	except:
		socket.symsessage('Usage: gouid <serial>')
		return

	if uid > 0x40000000:
		item = wolfpack.finditem(uid)
		if item:
			container = item.getoutmostitem()
			if container.container:
				container = container.container

			# Going to container
			socket.sysmessage('Going to item 0x%x [Top: 0x%x].' % (uid, container.serial))
			pos = container.pos
			socket.player.removefromview()
			socket.player.moveto(pos)
			socket.player.update()
			socket.resendworld()
		else:
			socket.sysmessage('No item with the serial 0x%x could be found.' % uid)
		return
	elif uid > 0:
		char = wolfpack.findchar(uid)
		if char:
			socket.sysmessage('Going to char 0x%x.' % (uid))
			pos = char.pos
			socket.player.removefromview()
			socket.player.moveto(pos)
			socket.player.update()
			socket.resendworld()
		else:
			socket.sysmessage('No char with the serial 0x%x could be found.' % uid)
		return

	socket.sysmessage('You specified an invalid serial: 0x%x.' % uid)

"""
	\command followers
	\description Show the names and serials of the followers of the
	targetted character.
"""
def followers_target(player, arguments, target):
	if not target.char or not target.char.player:
		player.socket.sysmessage('You have to target a player.')
		return
	followers = target.char.followers
	for follower in followers:
		player.socket.sysmessage('Follower: %s [0x%x, %u slot(s)]' % (follower.name, follower.serial, follower.controlslots))

def followers(socket, command, arguments):
	socket.sysmessage('Whose followers do you want to see?')
	socket.attachtarget("commands.followers_target", [])

def test(socket, command, arguments):
	gump = cGump()
	gump.addGump(0, 0, 0x67)
	gump.addText(0, 0, "਎Here is your bank box,".decode('utf-8'), 2)
	gump.send(socket)	

def newlostarget(char, arguments, target):
	targpos = target.pos
	
	if target.item:
		targpos = target.item
	elif target.char:
		targpos = target.char
		
	srcpos = char.pos
	srcpos.z += 15
	
	result = srcpos.lineofsightnew(targpos)
	char.socket.sysmessage('RESULT: ' + str(result))

def newlos(socket, command, arguments):
	socket.attachtarget('commands.newlostarget', [])
	socket.sysmessage('Select NEWLOS target.')
	
def onLoad():
	wolfpack.registercommand("test", test)
	wolfpack.registercommand("resendtooltip", resendtooltip)
	wolfpack.registercommand("season", season)
	wolfpack.registercommand("updateplayer", updateplayer)
	wolfpack.registercommand("time", uotime)
	wolfpack.registercommand("nudgeup", nudgeup)
	wolfpack.registercommand("nudgedown", nudgedown)
	wolfpack.registercommand("nightsight", nightsight)
	wolfpack.registercommand("multigems", multigems)
	wolfpack.registercommand("followers", followers)
	wolfpack.registercommand("gouid", gouid)
	wolfpack.registercommand("newlos", newlos)

"""
	\command nightsight
	\description Toggle gamemaster nightsight.
"""

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
