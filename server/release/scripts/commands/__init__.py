
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
	\command goname
	\usage - <code>goname name</code>
	\description Go to the characters found with the given name.
	\notes Use the command twice to go to the next char with the same name etc.
"""
def goname(socket, command, arguments):
	if len(arguments) == 0:
		socket.sysmessage('Usage: goname <name>')
		return

	chars = wolfpack.chariterator()

	char = chars.first
	name = hash(arguments.lower())

	found = []
	while char:
		if hash(char.name.lower()) == name:
			if not char.rank > socket.player.rank:
				found.append( char )

		char = chars.next

	if socket.hastag( "goname" ):
		i = socket.gettag( "goname" ) + 1
		if i >= len(found):
			# No more chars with the same name, start from the beginning
			i = 0
		socket.settag( "goname", i )
	else:
		socket.settag( "goname", 0 )
		i = 0

	if len(found) == 0:
		socket.sysmessage('A character with the given name was not found.')
	else:		
		pos = found[i].pos
		if pos.map == 0xFF:
			if found[i].npc:
				stablemaster = wolfpack.findobject(found[i].stablemaster)
			else:
				stablemaster = None
				
			if not stablemaster:
				socket.sysmessage("Not going to character '%s' [Serial: 0x%x]. They are on the internal map." % (found[i].name, found[i].serial))
			else:
				socket.sysmessage("Character '%s' [Serial: 0x%x] is stabled in object 0x%x." % (found[i].name, found[i].serial, stablemaster.serial))
		else:
			socket.sysmessage("Going to character '%s' [Serial: 0x%x]." % (found[i].name, found[i].serial))
			socket.player.removefromview()
			socket.player.moveto(pos)
			socket.player.update()
			socket.resendworld()

"""
	\command goitem
	\usage - <code>goitem name</code>
	\description Go to the item found with the given name.
	\notes Use the command twice to go to the next item with the same name etc.
"""
def goitem(socket, command, arguments):
	if len(arguments) == 0:
		socket.sysmessage('Usage: goitem <name>')
		return

	items = wolfpack.itemiterator()

	item = items.first
	name = hash(arguments.lower())

	found = []
	while item:
		if hash(item.name.lower()) == name:
			found.append( item )

		item = items.next

	if socket.hastag( "goitem" ):
		i = socket.gettag( "goitem" ) + 1
		if i >= len(found):
			# No more items with the same name, start from the beginning
			i = 0
		socket.settag( "goitem", i )
	else:
		socket.settag( "goitem", 0 )
		i = 0

	if len(found) == 0:
		socket.sysmessage('A item with the given name was not found.')
	else:
		container = found[i].getoutmostitem()
		
		if container.container:
			container = container.container
			
		socket.sysmessage("Going to item '%s' [Serial: 0x%x; Top: 0x%x]." % (found[i].name, found[i].serial, container.serial))
		pos = found[i].pos
		socket.player.removefromview()
		socket.player.moveto(pos)
		socket.player.update()
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
			
			if item.container.isitem():
				socket.sendobject(item.container)
				socket.sendcontainer(item.container)
		else:
			socket.sysmessage('No item with the serial 0x%x could be found.' % uid)
		return
	elif uid > 0:
		char = wolfpack.findchar(uid)
		if char and char.rank <= socket.player.rank:
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
	pass

def newlostarget(char, arguments, target):
	targpos = target.pos
	
	if target.item:
		targpos = target.item
	elif target.char:
		targpos = target.char
		
	srcpos = char.pos
	srcpos.z += 15
	
	result = char.canreach(targpos, 20, True)
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
	wolfpack.registercommand("goname", goname)
	wolfpack.registercommand("newlos", newlos)
	wolfpack.registercommand("goitem", goitem)

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
