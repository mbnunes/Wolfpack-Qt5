#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Galleon Pilot NPC Script                   #
#   )).-' {{ ;'`   |                                            #
#  ( (  ;._ \\ ctr |                                            #
#===============================================================#
import wolfpack
from wolfpack.gumps import cGump
from wolfpack import tr
from boats import security, wheel, damage, tillerman

def find_boat_for_pilot( pilot ):
	boat_serial = pilot.gettag( 'boat_serial' )
	if boat_serial:
		b = wolfpack.findmulti( int( boat_serial ) )
		if b: return b
	if pilot.multi:
		return pilot.multi
	return None

def onUse( player, pilot ):
	boat = find_boat_for_pilot( pilot )
	if not boat:
		pilot.say( "Arrr, I have lost my ship!" )
		return 1

	if not security.can_board( boat, player ):
		pilot.say( "Avast ye! Step off this vessel, landlubber!" )
		return 1

	pilot.say( "Greetings, %s! How may this humble pilot serve the ship?" % player.name )
	show_pilot_menu( pilot, boat, player )
	return 1

def show_pilot_menu( pilot, boat, player ):
	mygump = cGump()
	mygump.addRawLayout( "{page 0}" )
	mygump.addRawLayout( "{resizepic 0 0 5054 360 260}" )
	mygump.addRawLayout( "{gumppictiled 10 10 340 25 2624}" )
	mygump.addRawLayout( "{checkertrans 10 10 340 25}" )
	mygump.addRawLayout( "{croppedtext 15 12 320 20 1152 0}" ) # Title: Galleon Command Post

	# Info header
	ship_name = boat.name or "Galleon"
	status_str, pct = damage.get_damage_status( boat )
	mygump.addRawLayout( "{gumppictiled 10 40 340 40 2624}" )
	mygump.addRawLayout( "{checkertrans 10 40 340 40}" )
	mygump.addRawLayout( "{croppedtext 15 42 160 20 1152 1}" ) # Vessel: <name>
	mygump.addRawLayout( "{croppedtext 180 42 150 20 68 2}" )  # Condition: <pct>%

	# Menu Options
	mygump.addRawLayout( "{gumppictiled 10 85 340 125 2624}" )
	mygump.addRawLayout( "{checkertrans 10 85 340 125}" )

	# 1: Helm Steering
	mygump.addRawLayout( "{button 15 90 4005 4007 1 0 1}" )
	mygump.addRawLayout( "{croppedtext 50 92 250 20 1152 3}" ) # 1. Take the Wheel (Mouse Steering)

	# 2: Security Manifest
	mygump.addRawLayout( "{button 15 115 4005 4007 1 0 2}" )
	mygump.addRawLayout( "{croppedtext 50 117 250 20 1152 4}" ) # 2. Crew Manifest & Security

	# 3: Damage & Repairs
	mygump.addRawLayout( "{button 15 140 4005 4007 1 0 3}" )
	mygump.addRawLayout( "{croppedtext 50 142 250 20 1152 5}" ) # 3. Hull Inspection & Repairs

	# 4: Toggle Anchor
	mygump.addRawLayout( "{button 15 165 4005 4007 1 0 4}" )
	anchor_txt = "Raise Anchor" if boat.hastag( 'boat_anchored' ) else "Drop Anchor"
	mygump.addRawLayout( "{croppedtext 50 167 250 20 1152 6}" ) # 4. Anchor Toggle

	# Close
	mygump.addRawLayout( "{gumppictiled 10 215 340 35 2624}" )
	mygump.addRawLayout( "{checkertrans 10 215 340 35}" )
	mygump.addRawLayout( "{button 15 220 4017 4019 1 0 0}" )
	mygump.addRawLayout( "{croppedtext 50 222 100 20 1152 7}" )

	texts = [
		"Galleon Navigation & Orders",
		"Ship: " + ship_name,
		"Hull: %s (%d%%)" % ( status_str, pct ),
		"Take the Wheel (Helm Navigation)",
		"Security Manifest & Crew Ranks",
		"Hull Damage Inspection & Repair",
		anchor_txt,
		"Dismiss"
	]
	for t in texts:
		mygump.addRawText( t )

	mygump.setArgs( [ pilot.serial, boat.serial ] )
	mygump.setCallback( pilot_menu_callback )
	mygump.send( player )

def pilot_menu_callback( player, args, target ):
	btn = target.button
	if btn == 0:
		return

	pilot = wolfpack.findchar( args[0] )
	boat = wolfpack.findmulti( args[1] )
	if not boat:
		return

	if btn == 1: # Helm
		wheel.show_helm_gump( boat, player )
	elif btn == 2: # Security
		security.show_security_gump( boat, player )
	elif btn == 3: # Damage
		damage.show_repair_gump( boat, player )
	elif btn == 4: # Anchor
		tm_serial = boat.gettag( 'boat_tillerman' )
		tm = wolfpack.finditem( int( tm_serial ) ) if tm_serial else None
		if boat.hastag( 'boat_anchored' ):
			tillerman.RaiseAnchor( boat, tm, True )
			if pilot: pilot.say( "Anchor raised, sir! We are adrift!" )
		else:
			tillerman.LowerAnchor( boat, tm, True )
			if pilot: pilot.say( "Anchor dropped, sir! Ship held firmly!" )

def onSpeech( pilot, player, text, keywords ):
	boat = find_boat_for_pilot( pilot )
	if not boat:
		return False

	# If player is aboard and speaks navigation commands
	if player.multi and player.multi.serial == boat.serial:
		tm_serial = boat.gettag( 'boat_tillerman' )
		tm = wolfpack.finditem( int( tm_serial ) ) if tm_serial else None
		# Delegate to tillerman onSpeech
		return tillerman.onSpeech( tm or pilot, player, text, keywords )
	return False
