#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Naval Cannons & Ship-to-Ship Combat        #
#   )).-' {{ ;'`   |                                            #
#  ( (  ;._ \\ ctr |                                            #
#===============================================================#
import wolfpack
from wolfpack.gumps import cGump
from wolfpack import tr
from boats import security, damage

STATE_DIRTY = 0
STATE_CLEAN = 1
STATE_CHARGED = 2
STATE_LOADED = 3
STATE_READY = 4

STATE_NAMES = {
	STATE_DIRTY: "Dirty (Needs Swab)",
	STATE_CLEAN: "Cleaned (Needs Powder)",
	STATE_CHARGED: "Charged (Needs Ammo)",
	STATE_LOADED: "Loaded (Needs Fuse)",
	STATE_READY: "PRIMED & READY TO FIRE!"
}

def onUse( player, item ):
	# Handle Deeds
	if item.baseid in [ "ship_cannon_heavy_deed", "ship_cannon_light_deed" ]:
		boat = player.multi
		if not boat:
			player.socket.sysmessage( tr( "You must be aboard a ship to deploy a naval cannon." ) )
			return 1
		if not security.can_fire_cannons( boat, player ):
			player.socket.sysmessage( tr( "You do not have permission to deploy cannons on this vessel." ) )
			return 1
		player.socket.sysmessage( tr( "Select a deck location to deploy the cannon:" ) )
		player.socket.attachtarget( "boats.cannons.deploy_target", [ item.serial, item.baseid ] )
		return 1

	# Handle Cannon
	boat_serial = item.gettag( 'boat_serial' )
	boat = wolfpack.findmulti( int( boat_serial ) ) if boat_serial else player.multi
	if not boat:
		player.socket.sysmessage( tr( "This cannon is not mounted on a ship." ) )
		return 1

	if not player.canreach( item.pos, 3 ):
		player.socket.sysmessage( tr( "You are too far from the cannon." ) )
		return 1

	if not security.can_fire_cannons( boat, player ):
		player.socket.sysmessage( tr( "You do not have crew permission to operate ship cannons." ) )
		return 1

	show_cannon_gump( item, player )
	return 1

def deploy_target( player, args, target ):
	deed = wolfpack.finditem( args[0] )
	if not deed:
		return

	boat = player.multi
	if not boat:
		player.socket.sysmessage( tr( "You must be aboard a ship to deploy a cannon." ) )
		return

	# Target pos must be within reach and on the boat
	if not player.canreach( target.pos, 3 ):
		player.socket.sysmessage( tr( "You cannot deploy the cannon that far away." ) )
		return

	is_heavy = ( args[1] == "ship_cannon_heavy_deed" )
	cannon_id = "ship_cannon_heavy" if is_heavy else "ship_cannon_light"

	cannon = wolfpack.additem( cannon_id )
	if not cannon:
		player.socket.sysmessage( tr( "Failed to deploy cannon." ) )
		return

	cannon.moveto( target.pos )
	cannon.update()
	cannon.decay = 0
	cannon.settag( 'boat_serial', boat.serial )
	cannon.settag( 'cannon_state', STATE_CLEAN )
	cannon.settag( 'cannon_heavy', 1 if is_heavy else 0 )

	# Track cannon as a boat subpart
	part_count = int( boat.gettag( 'boat_part_count' ) or 0 ) + 1
	boat.settag( 'boat_part_count', part_count )
	boat.settag( 'boat_part%i' % part_count, cannon.serial )

	deed.delete()
	wolfpack.sound( cannon.pos, 0x02b ) # Heavy drop sound
	player.socket.sysmessage( tr( "You have mounted the cannon upon the deck." ) )

def show_cannon_gump( cannon, player ):
	state = int( cannon.gettag( 'cannon_state' ) or STATE_DIRTY )
	ammo_type = str( cannon.gettag( 'cannon_ammo' ) or "None" )
	is_heavy = int( cannon.gettag( 'cannon_heavy' ) or 0 ) == 1

	mygump = cGump()
	mygump.addRawLayout( "{page 0}" )
	mygump.addRawLayout( "{resizepic 0 0 5054 360 280}" )
	mygump.addRawLayout( "{gumppictiled 10 10 340 25 2624}" )
	mygump.addRawLayout( "{checkertrans 10 10 340 25}" )
	mygump.addRawLayout( "{croppedtext 15 12 320 20 1152 0}" ) # Title: Naval Cannon Station

	# Status area
	mygump.addRawLayout( "{gumppictiled 10 40 340 60 2624}" )
	mygump.addRawLayout( "{checkertrans 10 40 340 60}" )
	mygump.addRawLayout( "{croppedtext 15 45 150 20 1152 1}" ) # Cannon:
	mygump.addRawLayout( "{croppedtext 160 45 170 20 68 2}" )  # Heavy/Light
	mygump.addRawLayout( "{croppedtext 15 70 150 20 1152 3}" ) # Status:
	mygump.addRawLayout( "{croppedtext 160 70 170 20 38 4}" )  # <State Name>

	# Actions Area
	mygump.addRawLayout( "{gumppictiled 10 105 340 125 2624}" )
	mygump.addRawLayout( "{checkertrans 10 105 340 125}" )

	# 1: Clean Cannon
	mygump.addRawLayout( "{button 15 110 4005 4007 1 0 1}" )
	mygump.addRawLayout( "{croppedtext 50 112 200 20 1152 5}" ) # 1. Swab & Clean Barrel

	# 2: Powder Charge
	mygump.addRawLayout( "{button 15 135 4005 4007 1 0 2}" )
	mygump.addRawLayout( "{croppedtext 50 137 200 20 1152 6}" ) # 2. Load Powder Charge

	# 3: Load Ammo
	mygump.addRawLayout( "{button 15 160 4005 4007 1 0 3}" )
	mygump.addRawLayout( "{croppedtext 50 162 200 20 1152 7}" ) # 3. Ram Ammo (Ball / Grapeshot)

	# 4: Prime Fuse
	mygump.addRawLayout( "{button 15 185 4005 4007 1 0 4}" )
	mygump.addRawLayout( "{croppedtext 50 187 200 20 1152 8}" ) # 4. Prime with Fuse Match

	# FIRE Button (Highlighted)
	if state == STATE_READY:
		mygump.addRawLayout( "{button 250 135 4005 4007 1 0 5}" )
		mygump.addRawLayout( "{croppedtext 285 137 60 20 68 9}" ) # FIRE!
	else:
		mygump.addRawLayout( "{croppedtext 255 137 80 20 90 10}" ) # Not Ready

	# Dismantle Button
	mygump.addRawLayout( "{button 15 210 4005 4007 1 0 6}" )
	mygump.addRawLayout( "{croppedtext 50 212 200 20 38 11}" ) # Dismantle to Deed

	# Close Button
	mygump.addRawLayout( "{gumppictiled 10 235 340 35 2624}" )
	mygump.addRawLayout( "{checkertrans 10 235 340 35}" )
	mygump.addRawLayout( "{button 15 240 4017 4019 1 0 0}" )
	mygump.addRawLayout( "{croppedtext 50 242 100 20 1152 12}" )

	cannon_type_str = "Heavy Naval Cannon" if is_heavy else "Light Naval Cannon"
	texts = [
		"Naval Artillery Station",
		"Artillery Type:",
		cannon_type_str,
		"Current State:",
		STATE_NAMES.get( state, "Unknown" ),
		"1. Swab & Clean Barrel",
		"2. Load Powder Charge",
		"3. Ram Ammunition",
		"4. Prime Fuse Cord",
		"FIRE!",
		"Unready",
		"Dismantle Cannon",
		"Close"
	]
	for t in texts:
		mygump.addRawText( t )

	mygump.setArgs( [ cannon.serial ] )
	mygump.setCallback( cannon_gump_callback )
	mygump.send( player )

def cannon_gump_callback( player, args, target ):
	btn = target.button
	if btn == 0:
		return

	cannon = wolfpack.finditem( args[0] )
	if not cannon:
		return

	state = int( cannon.gettag( 'cannon_state' ) or STATE_DIRTY )
	backpack = player.getbackpack()
	if not backpack:
		player.socket.sysmessage( tr( "You have no backpack." ) )
		return

	if btn == 1: # Clean
		if state != STATE_DIRTY:
			player.socket.sysmessage( tr( "The cannon bore is already clean." ) )
		else:
			# Check swab
			cannon.settag( 'cannon_state', STATE_CLEAN )
			wolfpack.sound( cannon.pos, 0x04f ) # Swab sound
			player.socket.sysmessage( tr( "You swab out the barrel and clean away powder residue." ) )
		show_cannon_gump( cannon, player )

	elif btn == 2: # Charge Powder
		if state < STATE_CLEAN:
			player.socket.sysmessage( tr( "You must clean the barrel before loading powder." ) )
		elif state >= STATE_CHARGED:
			player.socket.sysmessage( tr( "The cannon is already charged with powder." ) )
		else:
			# Check powder charge in backpack
			charges = [ it for it in backpack.items if it.baseid == "powder_charge" ]
			if not charges:
				player.socket.sysmessage( tr( "You need a Powder Charge in your backpack." ) )
			else:
				charges[0].delete()
				cannon.settag( 'cannon_state', STATE_CHARGED )
				wolfpack.sound( cannon.pos, 0x247 )
				player.socket.sysmessage( tr( "You pour the powder charge into the barrel and pack it tightly." ) )
		show_cannon_gump( cannon, player )

	elif btn == 3: # Load Ammo
		if state < STATE_CHARGED:
			player.socket.sysmessage( tr( "You must charge the cannon with powder before loading ammo." ) )
		elif state >= STATE_LOADED:
			player.socket.sysmessage( tr( "A projectile is already loaded in the cannon." ) )
		else:
			balls = [ it for it in backpack.items if it.baseid in [ "cannonball", "grapeshot" ] ]
			if not balls:
				player.socket.sysmessage( tr( "You need a Cannonball or Grapeshot in your backpack." ) )
			else:
				ball = balls[0]
				ammo_name = ball.name or ball.baseid
				ball.delete()
				cannon.settag( 'cannon_state', STATE_LOADED )
				cannon.settag( 'cannon_ammo', ammo_name )
				wolfpack.sound( cannon.pos, 0x247 )
				player.socket.sysmessage( tr( "You ram a %s down into the breach." % ammo_name ) )
		show_cannon_gump( cannon, player )

	elif btn == 4: # Prime Fuse
		if state < STATE_LOADED:
			player.socket.sysmessage( tr( "You must load the powder and projectile before priming the fuse." ) )
		elif state >= STATE_READY:
			player.socket.sysmessage( tr( "The cannon is already primed and ready to fire!" ) )
		else:
			matches = [ it for it in backpack.items if it.baseid == "fuse_match" ]
			if not matches:
				player.socket.sysmessage( tr( "You need Fuse Matches in your backpack to prime the touchhole." ) )
			else:
				matches[0].delete()
				cannon.settag( 'cannon_state', STATE_READY )
				wolfpack.sound( cannon.pos, 0x054 )
				player.socket.sysmessage( tr( "You prime the touchhole with fuse powder. CANNON READY TO FIRE!" ) )
		show_cannon_gump( cannon, player )

	elif btn == 5: # FIRE!
		if state != STATE_READY:
			player.socket.sysmessage( tr( "The cannon is not ready to fire." ) )
			show_cannon_gump( cannon, player )
			return
		player.socket.sysmessage( tr( "Select target to aim cannon at:" ) )
		player.socket.attachtarget( "boats.cannons.fire_target", [ cannon.serial ] )

	elif btn == 6: # Dismantle
		deed_id = "ship_cannon_heavy_deed" if int( cannon.gettag( 'cannon_heavy' ) or 0 ) == 1 else "ship_cannon_light_deed"
		deed = wolfpack.additem( deed_id )
		if deed:
			backpack.additem( deed )
			deed.update()
			player.socket.sysmessage( tr( "You dismantle the cannon and place the deed in your pack." ) )
			cannon.delete()

def fire_target( player, args, target ):
	cannon = wolfpack.finditem( args[0] )
	if not cannon:
		return

	is_heavy = int( cannon.gettag( 'cannon_heavy' ) or 0 ) == 1
	max_range = 16 if is_heavy else 10

	if player.pos.distance( target.pos ) > max_range:
		player.socket.sysmessage( tr( "Target is out of cannon range (max %d tiles)." % max_range ) )
		show_cannon_gump( cannon, player )
		return

	# Fire sequence!
	# 1. Cannon blast sound and smoke
	wolfpack.sound( cannon.pos, 0x11C ) # Cannon explosion sound
	wolfpack.effect( 0x36b0, cannon.pos, 10, 16 ) # Explosion smoke at cannon

	# 2. Moving projectile animation from cannon to target
	# Moving effect: id 0x0e73 (cannonball)
	wolfpack.movingeffect( 0x0E73, cannon.pos, target.pos, 10, 0, 0, 0 )

	# Reset cannon state to dirty
	cannon.settag( 'cannon_state', STATE_DIRTY )
	cannon.deltag( 'cannon_ammo' )

	# 3. Impact effect at target
	wolfpack.sound( target.pos, 0x207 ) # Heavy explosion impact sound
	wolfpack.effect( 0x36b0, target.pos, 10, 16 )

	# 4. Damage resolution
	base_dmg = 90 if is_heavy else 50

	# Check if target is a creature/player
	if target.char:
		target_char = target.char
		target_char.damage( base_dmg, player, 0 ) # physical damage
		player.socket.sysmessage( tr( "Direct hit on %s for %d damage!" % ( target_char.name, base_dmg ) ) )

	# Check if target is on a boat
	target_boat = None
	if target.multi:
		target_boat = target.multi
	else:
		# Check nearby multi
		for it in wolfpack.items( target.pos, 2, target.pos.map ):
			if it.hastag( 'boat_serial' ):
				target_boat = wolfpack.findmulti( int( it.gettag( 'boat_serial' ) ) )
				break

	if target_boat and ( not player.multi or target_boat.serial != player.multi.serial ):
		ship_dmg = base_dmg * 10 # Multi damage multiplier
		damage.apply_damage( target_boat, ship_dmg, player )
		player.socket.sysmessage( tr( "Direct hit on %s! Dealt %d hull damage!" % ( target_boat.name or "enemy ship", ship_dmg ) ) )

	player.socket.sysmessage( tr( "The cannon recoils with a thunderous roar!" ) )
