
import wolfpack
from wolfpack.consts import LOG_ERROR
from wolfpack import console, utilities
from wolfpack.gumps import cGump

#
# Teleport someone trough
#
def onCollide(player, item):
	if item.hastag('playersonly') and player.npc:
		return 0

	if not item.hastag('target'):
		if player.socket:
			player.socket.sysmessage('This gate leads nowhere...')
		else:
			console.log(LOG_ERROR, "NPC [%x] using gate [%x] without target.\n" % (player.serial, item.serial))
		return 0

	target = item.gettag('target').split(',')

	# Convert the target of the gate.
	try:
		target = map(int, target)
	except:
		player.socket.sysmessage('This gate leads nowhere...')
		return 0

	# Move the player
	pos = player.pos
	pos.x = target[0]
	pos.y = target[1]
	pos.z = target[2]
	player.removefromview()
	player.moveto(pos)
	player.update(0)
	if player.socket:
		player.socket.resendworld()

	# show some nice effects
	if not item.hastag('silent'):
		item.soundeffect(0x1fe)
		utilities.smokepuff(player, pos)
		utilities.smokepuff(player, item.pos)

	return 1

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
		gump.addBackground( 0x24a4, 300, 200 )
		gump.addHtmlGump( 10, 30, 300, 20, '<basefont size="7" color="#336699"><center>Set target for teleportaion</center></basefont>' )
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

		gump.addButton( 83, 150, 2128, 2129, 1000 ) # Ok
		gump.addButton( 150, 150, 2119, 2120, 0 ) # Cancel

		gump.setCallback( "gate.gate_callback" )
		gump.setArgs( [ item ] )
		gump.send( player )
		return 1

	else:
		return 1

def gate_callback( char, args, response ):
	if( response.button == 0 ):
		return 1

	item = args[0]

	posx = response.text[1]
	posy = response.text[2]
	posz = response.text[3]
	posmap = response.text[4]

	target = "%s,%s,%s,%s" % (posx, posy, posz, posmap)
	item.settag( "target", target )

	return 1