#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Ship's Wheel & Mouse Steering (Helm)       #
#   )).-' {{ ;'`   |                                            #
#  ( (  ;._ \\ ctr |                                            #
#===============================================================#
import wolfpack
from wolfpack.gumps import cGump
from wolfpack import tr
from boats import security, damage, tillerman

def find_boat_at( player ):
	if player.multi:
		return player.multi
	# Search items in 1 tile around player for boat fixture
	for it in wolfpack.items( player.pos, 3, player.pos.map ):
		if it.hastag( 'boat_serial' ):
			b = wolfpack.findmulti( int( it.gettag( 'boat_serial' ) ) )
			if b:
				return b
	return None

def onUse( player, item ):
	boat_serial = item.gettag( 'boat_serial' )
	if not boat_serial:
		player.socket.sysmessage( tr( "This wheel is not connected to any vessel." ) )
		return 1

	boat = wolfpack.findmulti( int( boat_serial ) )
	if not boat:
		player.socket.sysmessage( tr( "The vessel could not be found." ) )
		return 1

	if not player.canreach( item.pos, 3 ):
		player.socket.sysmessage( tr( "You are too far from the wheel to steer." ) )
		return 1

	if not security.can_pilot( boat, player ):
		player.socket.sysmessage( tr( "You do not have permission to pilot this vessel." ) )
		return 1

	if boat.hastag( 'boat_scuttled' ):
		player.socket.sysmessage( tr( "This ship is scuttled and taking on water! You must repair the hull before setting sail!" ) )
		return 1

	# Set current helmsman
	boat.settag( 'boat_pilot', player.serial )
	show_helm_gump( boat, player )
	return 1

def show_helm_gump( boat, player ):
	mygump = cGump()
	mygump.addRawLayout( "{page 0}" )
	mygump.addRawLayout( "{resizepic 0 0 5054 340 330}" )
	mygump.addRawLayout( "{gumppictiled 10 10 320 25 2624}" )
	mygump.addRawLayout( "{checkertrans 10 10 320 25}" )
	mygump.addRawLayout( "{croppedtext 15 12 300 20 1152 0}" ) # Helm & Steering

	# Info bar
	is_anchored = boat.hastag( 'boat_anchored' )
	is_moving = boat.hastag( 'boat_moving' )
	fast_speed = int( boat.gettag( 'helm_fast_speed' ) or 1 ) == 1

	status_str = "ANCHORED" if is_anchored else ("UNDERWAY" if is_moving else "DRIFTING")
	speed_str = "FAST (Full Sail)" if fast_speed else "SLOW (Half Sail)"

	mygump.addRawLayout( "{gumppictiled 10 40 320 40 2624}" )
	mygump.addRawLayout( "{checkertrans 10 40 320 40}" )
	mygump.addRawLayout( "{croppedtext 15 42 150 20 1152 1}" ) # Status: <status>
	mygump.addRawLayout( "{croppedtext 170 42 150 20 68 2}" )  # Speed: <speed>
	mygump.addRawLayout( "{croppedtext 15 60 300 20 1152 3}" ) # Vessel: <name>

	# Steering Grid
	# Forward-Left (btn 7), Forward (btn 1), Forward-Right (btn 8)
	mygump.addRawLayout( "{button 75 90 4500 4500 1 0 7}" )   # NW
	mygump.addRawLayout( "{button 145 85 4501 4501 1 0 1}" )  # N (Forward)
	mygump.addRawLayout( "{button 215 90 4502 4502 1 0 8}" )  # NE

	# Turn Port (btn 5), STOP (btn 0 / 10), Turn Starboard (btn 6)
	mygump.addRawLayout( "{button 65 140 4507 4507 1 0 5}" )  # W (Turn Port)
	mygump.addRawLayout( "{button 135 135 4017 4019 1 0 10}" ) # Center STOP button
	mygump.addRawLayout( "{croppedtext 138 155 40 20 1152 4}" ) # STOP text
	mygump.addRawLayout( "{button 225 140 4503 4503 1 0 6}" )  # E (Turn Starboard)

	# Back-Left (btn 9), Backward (btn 2), Back-Right (btn 11)
	mygump.addRawLayout( "{button 75 190 4506 4506 1 0 9}" )  # SW
	mygump.addRawLayout( "{button 145 195 4505 4505 1 0 2}" ) # S (Backward)
	mygump.addRawLayout( "{button 215 190 4504 4504 1 0 11}" ) # SE

	# Additional Actions bar
	mygump.addRawLayout( "{gumppictiled 10 240 320 45 2624}" )
	mygump.addRawLayout( "{checkertrans 10 240 320 45}" )

	# Anchor button (btn 20)
	anchor_btn_txt = "Raise Anchor" if is_anchored else "Drop Anchor"
	mygump.addRawLayout( "{button 15 244 4005 4007 1 0 20}" )
	mygump.addRawLayout( "{croppedtext 48 245 100 20 1152 5}" )

	# Speed button (btn 21)
	mygump.addRawLayout( "{button 170 244 4005 4007 1 0 21}" )
	mygump.addRawLayout( "{croppedtext 203 245 120 20 1152 6}" )

	# Come About (180 deg) button (btn 22)
	mygump.addRawLayout( "{button 15 264 4005 4007 1 0 22}" )
	mygump.addRawLayout( "{croppedtext 48 265 100 20 1152 7}" ) # Come About

	# Inspection button (btn 23)
	mygump.addRawLayout( "{button 170 264 4005 4007 1 0 23}" )
	mygump.addRawLayout( "{croppedtext 203 265 120 20 1152 8}" ) # Ship Status

	# Close button
	mygump.addRawLayout( "{gumppictiled 10 290 320 30 2624}" )
	mygump.addRawLayout( "{checkertrans 10 290 320 30}" )
	mygump.addRawLayout( "{button 15 294 4017 4019 1 0 0}" )
	mygump.addRawLayout( "{croppedtext 50 295 100 20 1152 9}" )

	ship_name = boat.name or "Sailing Ship"
	texts = [
		"Ship Helm & Steering Controls",
		"Status: " + status_str,
		speed_str,
		"Vessel: " + ship_name,
		"STOP",
		anchor_btn_txt,
		"Toggle Speed",
		"Come About",
		"Inspect Damage",
		"Leave Wheel"
	]
	for t in texts:
		mygump.addRawText( t )

	mygump.setArgs( [ boat.serial ] )
	mygump.setCallback( helm_gump_callback )
	mygump.send( player )

def helm_gump_callback( player, args, target ):
	button = target.button
	if button == 0:
		return

	boat = wolfpack.findmulti( args[0] )
	if not boat:
		return

	if not security.can_pilot( boat, player ):
		player.socket.sysmessage( tr( "You do not have permission to pilot this vessel." ) )
		return

	fast_speed = int( boat.gettag( 'helm_fast_speed' ) or 1 ) == 1
	tillerman_serial = boat.gettag( 'boat_tillerman' )
	tm = wolfpack.finditem( int( tillerman_serial ) ) if tillerman_serial else None

	if button == 1: # Forward
		tillerman.StartMove( boat, tm, tillerman.Forward, fast_speed )
	elif button == 2: # Backward
		tillerman.StartMove( boat, tm, tillerman.Backward, fast_speed )
	elif button == 5: # Turn Left (Port)
		tillerman.StartTurn( boat, tm, -2, True )
	elif button == 6: # Turn Right (Starboard)
		tillerman.StartTurn( boat, tm, 2, True )
	elif button == 7: # Forward Left
		tillerman.StartMove( boat, tm, tillerman.ForwardLeft, fast_speed )
	elif button == 8: # Forward Right
		tillerman.StartMove( boat, tm, tillerman.ForwardRight, fast_speed )
	elif button == 9: # Backward Left
		tillerman.StartMove( boat, tm, tillerman.BackwardLeft, fast_speed )
	elif button == 11: # Backward Right
		tillerman.StartMove( boat, tm, tillerman.BackwardRight, fast_speed )
	elif button == 10: # STOP
		tillerman.StopMove( boat, tm, True )
	elif button == 20: # Anchor
		if boat.hastag( 'boat_anchored' ):
			tillerman.RaiseAnchor( boat, tm, True )
		else:
			tillerman.LowerAnchor( boat, tm, True )
	elif button == 21: # Speed toggle
		new_speed = 0 if fast_speed else 1
		boat.settag( 'helm_fast_speed', new_speed )
		player.socket.sysmessage( tr( "Sail set to: %s." % ( "FULL SAIL (Fast)" if new_speed == 1 else "HALF SAIL (Slow)" ) ) )
	elif button == 22: # Come About (180 deg)
		tillerman.StartTurn( boat, tm, -4, True )
	elif button == 23: # Inspect Damage
		damage.show_repair_gump( boat, player )
		return

	# Reopen Helm Gump for continuous steering
	show_helm_gump( boat, player )
