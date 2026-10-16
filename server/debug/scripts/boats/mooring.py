#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Mooring Line & Boarding System             #
#   )).-' {{ ;'`   |                                            #
#  ( (  ;._ \\ ctr |                                            #
#===============================================================#
import wolfpack
from wolfpack import tr
from boats import security

def find_nearest_land( player, start_pos, max_dist=8 ):
	# Check tiles in outward spiral for walkable land/dock
	map_id = start_pos.map
	best_pos = None
	min_dist = 999

	for dx in range( -max_dist, max_dist + 1 ):
		for dy in range( -max_dist, max_dist + 1 ):
			if dx == 0 and dy == 0:
				continue
			dist = abs( dx ) + abs( dy )
			if dist > max_dist:
				continue
			x = start_pos.x + dx
			y = start_pos.y + dy
			z = start_pos.z

			# Check if tile is not water
			land_id = wolfpack.map( x, y, map_id )
			# Water tiles typically 0x00A8 to 0x00AB, 0x0136, etc.
			is_water = False
			if (land_id >= 0x00A8 and land_id <= 0x00AB) or (land_id >= 0x0136 and land_id <= 0x0137):
				is_water = True

			if not is_water:
				test_coord = wolfpack.coord( x, y, z, map_id )
				# Check if walkable and not on a boat
				if player.canreach( test_coord, max_dist ):
					if dist < min_dist:
						min_dist = dist
						best_pos = test_coord

	return best_pos

def onUse( player, item ):
	boat_serial = item.gettag( 'boat_serial' )
	boat = wolfpack.findmulti( int( boat_serial ) ) if boat_serial else None
	if not boat:
		player.socket.sysmessage( tr( "This mooring line is not attached to any ship." ) )
		return 1

	if not player.canreach( item.pos, 8 ):
		player.socket.sysmessage( tr( "You are too far away from the mooring line." ) )
		return 1

	if boat.hastag( 'boat_moving' ):
		player.socket.sysmessage( tr( "You cannot use the mooring line while the ship is underway!" ) )
		return 1

	# Check whether player is currently on this boat or boarding from outside
	is_aboard = ( player.multi and player.multi.serial == boat.serial )

	if not is_aboard:
		# Player is attempting to board the ship
		if not security.can_board( boat, player ):
			player.socket.sysmessage( tr( "You do not have permission to board this vessel." ) )
			return 1

		# Boarding: move player to the mooring line location on deck
		dest = wolfpack.coord( item.pos.x, item.pos.y, item.pos.z, item.pos.map )
		player.moveto( dest )
		player.update()
		player.socket.sysmessage( tr( "You climb aboard the vessel via the mooring line." ) )
		wolfpack.sound( player.pos, 0x028 ) # Footstep/rope sound
	else:
		# Player is on board attempting to step ashore to a dock or land
		dest = find_nearest_land( player, item.pos, 8 )
		if not dest:
			player.socket.sysmessage( tr( "You cannot dock this far out to sea. You must be near land or a pier." ) )
			return 1

		player.moveto( dest )
		player.update()
		player.socket.sysmessage( tr( "You step ashore via the mooring line." ) )
		wolfpack.sound( player.pos, 0x028 )

	return 1
