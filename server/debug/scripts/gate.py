import wolfpack
from wolfpack.consts import LOG_ERROR
from wolfpack import console, utilities, tr
from wolfpack.gumps import cGump

#
# Teleport someone trough
#
def onCollide(player, item):
	if item.hastag('playersonly') and player.npc:
		return False

	if not item.hastag('target'):
		if player.socket:
			player.socket.sysmessage( tr('This gate leads nowhere...') )
		else:
			console.log(LOG_ERROR, tr("NPC [%x] using gate [%x] without target.\n") % (player.serial, item.serial))
		return False

	target = item.gettag('target').split(',')

	# Convert the target of the gate.
	try:
		target = map(int, target)
	except:
		player.socket.sysmessage( tr('This gate leads nowhere...') )
		return False

	# Validate the coord
	try:
		m = target[3]
	except:
		m = player.pos.map
	pos = wolfpack.coord( target[0], target[1], target[2], m )

	if not utilities.isValidPosition( pos ):
		player.socket.sysmessage( tr('This gate leads nowhere...') )
		return False

	if not utilities.isMapAvailableTo( player, pos.map ):
		return False

	# Move his pets if he has any
	if player.player:
		for follower in player.followers:
			if follower.wandertype == 4 and follower.distanceto(player) < 5:
				follower.removefromview()
				follower.moveto(pos)
				follower.update()

	player.removefromview()
	player.moveto(pos)
	player.update()
	if player.socket:
		player.socket.resendworld()

	# show some nice effects
	if not item.hastag('silent'):
		item.soundeffect(0x1fe)
		utilities.smokepuff(player, pos)
		utilities.smokepuff(player, item.pos)

	return True

#
# Here a gump which allows customization.
#
def onUse(player, item):
	if player.gm:
		if not item.hastag( 'target' ):
			item.settag( 'target', "0,0,0,0" )

		if item.hastag( 'target' ):
			target = item.gettag('target').split(',')

			pos = player.pos
			x = target[0]
			y = target[1]
			z = target[2]
			map = target[3]

		# create gump
		gump = cGump( 0, 0, 0, 50, 50 )
		# Header
		gump.addBackground( 0x24a4, 300, 280 )
		gump.addHtmlGump( 10, 30, 300, 20, tr('<basefont size="7" color="#336699"><center>Set target for teleportaion</center></basefont>') )
		# Input Fields
		gump.addHtmlGump( 40, 60, 300, 20, 'x-Pos:' )
		gump.addResizeGump( 90, 60, 0x2486, 156, 26 )
		gump.addInputField( 93, 63, 150, 20, 0x539, 1, x )

		gump.addHtmlGump( 40, 80, 300, 20, 'y-Pos:' )
		gump.addResizeGump( 90, 80, 0x2486, 156, 26 )
		gump.addInputField( 93, 83, 150, 20, 0x539, 2, y )

		gump.addHtmlGump( 40, 100, 300, 20, 'z-Pos:' )
		gump.addResizeGump( 90, 100, 0x2486, 156, 26 )
		gump.addInputField( 93, 103, 150, 20, 0x539, 3, z )

		gump.addHtmlGump( 40, 120, 300, 20, 'Map:' )
		gump.addResizeGump( 90, 120, 0x2486, 156, 26 )
		gump.addInputField( 93, 123, 150, 20, 0x539, 4, map )

		playersonly = item.hastag('playersonly')
		gump.addCheckbox(40, 160, 0xd2, 0xd3, 1, playersonly )
		gump.addHtmlGump( 70, 160, 300, 20, 'Players only' )
		
		silent = item.hastag('silent')
		gump.addCheckbox(40, 190, 0xd2, 0xd3, 2, silent )
		gump.addHtmlGump( 70, 190, 300, 20, 'Silent' )


		gump.addButton( 83, 225, 2128, 2129, 1000 ) # Ok
		gump.addButton( 150, 225, 2119, 2120, 0 ) # Cancel

		gump.setCallback( gate_callback )
		gump.setArgs( [ item ] )
		gump.send( player )
		return True

	else:
		return True

def gate_callback( char, args, response ):
	if( response.button == 0 ):
		return True

	item = args[0]

	if 1 in response.switches:
		item.settag('playersonly', 1)
	elif item.hastag('playersonly'):
		item.deltag('playersonly')
		
	if 2 in response.switches:
		item.settag('silent', 1)
	elif item.hastag('silent'):
		item.deltag('silent')
	
	posx = response.text[1]
	posy = response.text[2]
	posz = response.text[3]
	posmap = response.text[4]

	target = "%s,%s,%s,%s" % (posx, posy, posz, posmap)
	item.settag( "target", target )

	return True
