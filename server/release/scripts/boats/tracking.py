#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Ship Tracking & Sea Radar System           #
#   )).-' {{ ;'`   |                                            #
#  ( (  ;._ \\ ctr |                                            #
#===============================================================#
import wolfpack
from wolfpack.gumps import cGump
from wolfpack import tr

def get_bearing( from_pos, to_pos ):
	dx = to_pos.x - from_pos.x
	dy = to_pos.y - from_pos.y

	if abs( dx ) < 5 and dy < 0: return "North"
	if abs( dx ) < 5 and dy > 0: return "South"
	if dx > 0 and abs( dy ) < 5: return "East"
	if dx < 0 and abs( dy ) < 5: return "West"
	if dx > 0 and dy < 0: return "North-East"
	if dx > 0 and dy > 0: return "South-East"
	if dx < 0 and dy < 0: return "North-West"
	return "South-West"

def scan_ships( player, max_range=200 ):
	ships = []
	p_map = player.pos.map
	p_boat_serial = player.multi.serial if player.multi else 0

	# Iterate multis or nearby objects
	# In Wolfpack, multis can be scanned or found in world
	# We search items/multis around player pos up to max_range
	for item in wolfpack.items( player.pos, max_range, p_map ):
		if item.ismulti():
			# Verify if it's a boat
			if item.hastag( 'boat_facing' ) or item.hastag( 'boat_id_north' ) or item.hastag( 'boat_tillerman' ):
				if item.serial != p_boat_serial:
					dist = int( player.pos.distance( item.pos ) )
					bearing = get_bearing( player.pos, item.pos )
					name = item.name or "Unknown Vessel"
					ships.append( ( item.serial, name, dist, bearing, item.pos ) )

	ships.sort( key=lambda s: s[2] ) # Sort by closest
	return ships

def show_tracking_gump( player, ships=None ):
	if ships is None:
		ships = scan_ships( player, 200 )

	mygump = cGump()
	mygump.addRawLayout( "{page 0}" )
	mygump.addRawLayout( "{resizepic 0 0 5054 440 320}" )
	mygump.addRawLayout( "{gumppictiled 10 10 420 25 2624}" )
	mygump.addRawLayout( "{checkertrans 10 10 420 25}" )
	mygump.addRawLayout( "{croppedtext 15 12 300 20 1152 0}" ) # Title: Maritime Sea Radar

	# Table Headers
	mygump.addRawLayout( "{gumppictiled 10 40 420 22 2624}" )
	mygump.addRawLayout( "{checkertrans 10 40 420 22}" )
	mygump.addRawLayout( "{croppedtext 50 42 160 20 1152 1}" ) # Vessel Name
	mygump.addRawLayout( "{croppedtext 220 42 80 20 1152 2}" ) # Distance
	mygump.addRawLayout( "{croppedtext 310 42 100 20 1152 3}" ) # Direction

	# Content area
	mygump.addRawLayout( "{gumppictiled 10 65 420 200 2624}" )
	mygump.addRawLayout( "{checkertrans 10 65 420 200}" )

	# Close Button
	mygump.addRawLayout( "{gumppictiled 10 270 420 35 2624}" )
	mygump.addRawLayout( "{checkertrans 10 270 420 35}" )
	mygump.addRawLayout( "{button 15 275 4017 4019 1 0 0}" )
	mygump.addRawLayout( "{croppedtext 50 277 100 20 1152 4}" )

	texts = [
		"Maritime Sea Radar & Tracking",
		"Vessel Name",
		"Distance",
		"Bearing",
		"Close"
	]

	if not ships:
		none_idx = len( texts ); texts.append( "No other vessels detected in scanning range (200 tiles)." )
		mygump.addRawLayout( f"{{croppedtext 20 80 380 20 38 {none_idx}}}" )
	else:
		y = 70
		for i, (serial, name, dist, bearing, pos) in enumerate( ships[:8] ):
			btn_idx = i + 1
			mygump.addRawLayout( f"{{button 15 {y + 2} 4005 4007 1 0 {btn_idx}}}" )

			name_idx = len( texts ); texts.append( name )
			dist_idx = len( texts ); texts.append( "%d tiles" % dist )
			bear_idx = len( texts ); texts.append( bearing )

			mygump.addRawLayout( f"{{croppedtext 50 {y} 160 20 1152 {name_idx}}}" )
			mygump.addRawLayout( f"{{croppedtext 220 {y} 80 20 68 {dist_idx}}}" )
			mygump.addRawLayout( f"{{croppedtext 310 {y} 100 20 68 {bear_idx}}}" )
			y += 24

	for t in texts:
		mygump.addRawText( t )

	mygump.setArgs( [ ships ] )
	mygump.setCallback( tracking_gump_callback )
	mygump.send( player )

def tracking_gump_callback( player, args, target ):
	btn = target.button
	if btn == 0:
		return

	ships = args[0]
	idx = btn - 1
	if idx < len( ships ):
		serial, name, dist, bearing, pos = ships[idx]
		# Set player tracking arrow if supported by socket or notify coordinates
		player.socket.sysmessage( tr( "Tracking locked onto: %s (bearing %s, distance %d tiles at %d, %d)." % ( name, bearing, dist, pos.x, pos.y ) ) )

#
# onUse for Spyglass when scanning
#
def onUse( player, item ):
	player.socket.sysmessage( tr( "Scanning the open seas for vessels..." ) )
	wolfpack.sound( player.pos, 0x02d ) # Spyglass sound
	show_tracking_gump( player )
	return 1
