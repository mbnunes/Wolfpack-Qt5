#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Ship Recall Runes (Maritime Teleportation) #
#   )).-' {{ ;'`   |                                            #
#  ( (  ;._ \\ ctr |                                            #
#===============================================================#
import wolfpack
from wolfpack import tr
from boats import security

def onUse( player, item ):
	backpack = player.getbackpack()
	if not backpack or not player.canreach( item, -1 ):
		player.socket.sysmessage( tr( "The ship rune must be in your pack to use it." ) )
		return 1

	ship_serial = item.gettag( 'ship_serial' )

	# 1. If not yet bound, bind to current vessel
	if not ship_serial:
		boat = player.multi
		if not boat:
			player.socket.sysmessage( tr( "You must be aboard a ship to bind this rune to its deck." ) )
			return 1

		if not security.can_pilot( boat, player ):
			player.socket.sysmessage( tr( "Only Officers and the Captain may bind magical runes to this ship." ) )
			return 1

		ship_name = boat.name or "Sailing Vessel"
		item.settag( 'ship_serial', boat.serial )
		item.name = "Ship Rune: %s" % ship_name
		wolfpack.sound( player.pos, 0x1F2 )
		wolfpack.effect( 0x376A, player.pos, 9, 32 )
		player.socket.sysmessage( tr( "You attune the maritime rune to the vessel '%s'." % ship_name ) )
		item.update()
		return 1

	# 2. Already bound: attempt Recall to ship
	boat = wolfpack.findmulti( int( ship_serial ) )
	if not boat:
		player.socket.sysmessage( tr( "The magical resonance fades... that vessel no longer exists in this realm." ) )
		return 1

	if not security.can_board( boat, player ):
		player.socket.sysmessage( tr( "You do not have permission to board that vessel." ) )
		return 1

	if boat.hastag( 'boat_moving' ):
		player.socket.sysmessage( tr( "The vessel is surging through rough waters! You cannot safely recall while it is underway." ) )
		return 1

	# Find boarding position (near tillerman, wheel, or center of boat)
	dest = None
	# Check tillerman pos
	tm_serial = boat.gettag( 'boat_tillerman' )
	if tm_serial:
		tm = wolfpack.finditem( int( tm_serial ) )
		if tm:
			dest = wolfpack.coord( tm.pos.x, tm.pos.y, tm.pos.z, tm.pos.map )

	if not dest:
		dest = wolfpack.coord( boat.pos.x, boat.pos.y, boat.pos.z, boat.pos.map )

	# Teleport with effects
	wolfpack.sound( player.pos, 0x1FE )
	wolfpack.effect( 0x3709, player.pos, 10, 30 )

	player.moveto( dest )
	player.update()

	wolfpack.sound( player.pos, 0x1FE )
	wolfpack.effect( 0x3709, player.pos, 10, 30 )
	player.socket.sysmessage( tr( "You recall aboard '%s'!" % ( boat.name or "the vessel" ) ) )
	return 1
