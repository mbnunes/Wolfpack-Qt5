#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Multi-Level Ship Security System           #
#   )).-' {{ ;'`   |                                            #
#  ( (  ;._ \\ ctr |                                            #
#===============================================================#
import wolfpack
from wolfpack.gumps import cGump
from wolfpack import tr

# Security Levels
DENIED = 0
PASSENGER = 1
PUBLIC = 1
CREW = 2
OFFICER = 3
CAPTAIN = 4
OWNER = 4

LEVEL_NAMES = {
	DENIED: "Denied",
	PASSENGER: "Passenger",
	CREW: "Crewman",
	OFFICER: "Officer",
	CAPTAIN: "Captain / Owner"
}

def get_serial_list( boat, tag_name ):
	val = str( boat.gettag( tag_name ) or "" )
	if not val:
		return []
	parts = [ int(s) for s in val.split(',') if s.strip().isdigit() ]
	return parts

def set_serial_list( boat, tag_name, serials ):
	if not serials:
		if boat.hastag( tag_name ):
			boat.deltag( tag_name )
	else:
		boat.settag( tag_name, ",".join( str(s) for s in serials ) )

def get_security_level( boat, char ):
	if not boat or not char:
		return DENIED

	if char.gm:
		return CAPTAIN

	# Check Owner
	if boat.owner and boat.owner.serial == char.serial:
		return CAPTAIN

	if boat.hastag( 'boat_owner' ) and int( boat.gettag( 'boat_owner' ) ) == char.serial:
		return CAPTAIN

	# Officers
	if char.serial in get_serial_list( boat, 'boat_officers' ):
		return OFFICER

	# Crew
	if char.serial in get_serial_list( boat, 'boat_crew' ):
		return CREW

	# Passengers
	if char.serial in get_serial_list( boat, 'boat_passengers' ):
		return PASSENGER

	# Guild access
	if boat.owner and hasattr(boat.owner, 'guild') and hasattr(char, 'guild'):
		if boat.owner.guild and char.guild and boat.owner.guild == char.guild:
			if int( boat.gettag( 'boat_guild_access' ) or 1 ) == 1:
				return CREW

	# Public access
	if int( boat.gettag( 'boat_public_access' ) or 0 ) == 1:
		return PASSENGER

	return DENIED

def can_pilot( boat, char ):
	return get_security_level( boat, char ) >= OFFICER

def can_fire_cannons( boat, char ):
	return get_security_level( boat, char ) >= CREW

def can_access_hold( boat, char ):
	return get_security_level( boat, char ) >= OFFICER

def can_board( boat, char ):
	return get_security_level( boat, char ) >= PASSENGER

def can_manage_security( boat, char ):
	return get_security_level( boat, char ) >= CAPTAIN

def can_repair( boat, char ):
	return get_security_level( boat, char ) >= CREW

#
# Security Manifest Gump
#
def show_security_gump( boat, player ):
	if not can_manage_security( boat, player ):
		player.socket.sysmessage( tr( "Only the ship's Captain may manage vessel security." ) )
		return

	officers = get_serial_list( boat, 'boat_officers' )
	crew = get_serial_list( boat, 'boat_crew' )
	passengers = get_serial_list( boat, 'boat_passengers' )
	is_public = int( boat.gettag( 'boat_public_access' ) or 0 ) == 1

	mygump = cGump()
	mygump.addRawLayout( "{page 0}" )
	mygump.addRawLayout( "{resizepic 0 0 5054 480 430}" )
	mygump.addRawLayout( "{gumppictiled 10 10 460 25 2624}" )
	mygump.addRawLayout( "{checkertrans 10 10 460 25}" )
	mygump.addRawLayout( "{croppedtext 15 12 300 20 1152 0}" ) # Title: Vessel Security Manifest

	# Ship info bar
	mygump.addRawLayout( "{gumppictiled 10 40 460 30 2624}" )
	mygump.addRawLayout( "{checkertrans 10 40 460 30}" )
	mygump.addRawLayout( "{croppedtext 15 45 200 20 1152 1}" ) # Vessel: <Name>
	mygump.addRawLayout( "{croppedtext 220 45 150 20 1152 2}" ) # Access: Public/Private
	# Toggle Public button
	mygump.addRawLayout( "{button 380 44 4005 4007 1 0 1}" ) # Button 1: Toggle Public
	mygump.addRawLayout( "{croppedtext 415 45 50 20 68 3}" ) # Toggle

	# Add Member Button
	mygump.addRawLayout( "{gumppictiled 10 75 460 25 2624}" )
	mygump.addRawLayout( "{checkertrans 10 75 460 25}" )
	mygump.addRawLayout( "{button 15 77 4005 4007 1 0 2}" ) # Button 2: Add Member
	mygump.addRawLayout( "{croppedtext 50 78 200 20 68 4}" ) # Grant Access / Add Crew

	# Roster Headers
	mygump.addRawLayout( "{gumppictiled 10 105 460 22 2624}" )
	mygump.addRawLayout( "{checkertrans 10 105 460 22}" )
	mygump.addRawLayout( "{croppedtext 50 107 180 20 1152 5}" ) # Name
	mygump.addRawLayout( "{croppedtext 240 107 120 20 1152 6}" ) # Rank
	mygump.addRawLayout( "{croppedtext 380 107 80 20 1152 7}" ) # Action

	# List area
	mygump.addRawLayout( "{gumppictiled 10 130 460 240 2624}" )
	mygump.addRawLayout( "{checkertrans 10 130 460 240}" )

	# Close button
	mygump.addRawLayout( "{gumppictiled 10 380 460 35 2624}" )
	mygump.addRawLayout( "{checkertrans 10 380 460 35}" )
	mygump.addRawLayout( "{button 15 385 4017 4019 1 0 0}" ) # Close
	mygump.addRawLayout( "{croppedtext 50 387 100 20 1152 8}" )

	ship_name = boat.name or "Sailing Ship"
	pub_text = "Status: PUBLIC" if is_public else "Status: PRIVATE"

	texts = [
		"Vessel Security Manifest",
		"Ship: " + ship_name,
		pub_text,
		"Toggle",
		"Grant Access to Player",
		"Crew Member",
		"Rank",
		"Remove",
		"Close"
	]

	# Build roster
	roster = []
	for s in officers:
		ch = wolfpack.findchar( s )
		name = ch.name if ch else ("Officer (%s)" % s)
		roster.append( ( s, name, "Officer", OFFICER ) )
	for s in crew:
		ch = wolfpack.findchar( s )
		name = ch.name if ch else ("Crew (%s)" % s)
		roster.append( ( s, name, "Crewman", CREW ) )
	for s in passengers:
		ch = wolfpack.findchar( s )
		name = ch.name if ch else ("Passenger (%s)" % s)
		roster.append( ( s, name, "Passenger", PASSENGER ) )

	mygump.addRawLayout( "{page 1}" )
	y = 135
	btn_offset = 10
	for i, (serial, name, rank_str, lvl) in enumerate( roster[:9] ):
		remove_btn = btn_offset + i
		mygump.addRawLayout( f"{{button 15 {y + 2} 4005 4007 1 0 {remove_btn}}}" )

		name_idx = len( texts ); texts.append( name )
		rank_idx = len( texts ); texts.append( rank_str )
		act_idx = len( texts ); texts.append( "Revoke" )

		mygump.addRawLayout( f"{{croppedtext 50 {y} 180 20 1152 {name_idx}}}" )
		mygump.addRawLayout( f"{{croppedtext 240 {y} 120 20 68 {rank_idx}}}" )
		mygump.addRawLayout( f"{{croppedtext 380 {y} 80 20 38 {act_idx}}}" )
		y += 24

	for t in texts:
		mygump.addRawText( t )

	mygump.setArgs( [ boat.serial, roster ] )
	mygump.setCallback( security_gump_callback )
	mygump.send( player )

def security_gump_callback( player, args, target ):
	button = target.button
	if button == 0:
		return

	boat = wolfpack.findmulti( args[0] )
	if not boat:
		return

	# Toggle Public
	if button == 1:
		curr = int( boat.gettag( 'boat_public_access' ) or 0 )
		new_val = 0 if curr == 1 else 1
		boat.settag( 'boat_public_access', new_val )
		if new_val == 1:
			player.socket.sysmessage( tr( "The ship is now OPEN to the public." ) )
		else:
			player.socket.sysmessage( tr( "The ship is now PRIVATE (crew/passengers only)." ) )
		show_security_gump( boat, player )
		return

	# Add Member
	if button == 2:
		player.socket.sysmessage( tr( "Target a player to grant ship access to:" ) )
		player.socket.attachtarget( "boats.security.target_add_member", [ boat.serial ] )
		return

	# Revoke button
	if button >= 10:
		idx = button - 10
		roster = args[1]
		if idx < len( roster ):
			target_serial, target_name, _, lvl = roster[idx]
			if lvl == OFFICER:
				officers = get_serial_list( boat, 'boat_officers' )
				if target_serial in officers: officers.remove( target_serial )
				set_serial_list( boat, 'boat_officers', officers )
			elif lvl == CREW:
				crew = get_serial_list( boat, 'boat_crew' )
				if target_serial in crew: crew.remove( target_serial )
				set_serial_list( boat, 'boat_crew', crew )
			elif lvl == PASSENGER:
				passengers = get_serial_list( boat, 'boat_passengers' )
				if target_serial in passengers: passengers.remove( target_serial )
				set_serial_list( boat, 'boat_passengers', passengers )

			player.socket.sysmessage( tr( "Revoked ship access for %s." % target_name ) )
			show_security_gump( boat, player )

def target_add_member( player, args, target ):
	if not target.char or not target.char.isplayer():
		player.socket.sysmessage( tr( "You must target a player." ) )
		return

	boat = wolfpack.findmulti( args[0] )
	if not boat:
		return

	target_char = target.char
	if target_char.serial == player.serial:
		player.socket.sysmessage( tr( "You are already the Captain!" ) )
		return

	# Show Rank Selection Gump
	show_rank_selection_gump( player, boat, target_char )

def show_rank_selection_gump( player, boat, target_char ):
	mygump = cGump()
	mygump.addRawLayout( "{page 0}" )
	mygump.addRawLayout( "{resizepic 0 0 5054 300 200}" )
	mygump.addRawLayout( "{gumppictiled 10 10 280 25 2624}" )
	mygump.addRawLayout( "{checkertrans 10 10 280 25}" )
	mygump.addRawLayout( "{croppedtext 15 12 260 20 1152 0}" )

	# Options
	# Officer
	mygump.addRawLayout( "{button 15 45 4005 4007 1 0 1}" )
	mygump.addRawLayout( "{croppedtext 50 46 200 20 68 1}" )
	# Crewman
	mygump.addRawLayout( "{button 15 75 4005 4007 1 0 2}" )
	mygump.addRawLayout( "{croppedtext 50 76 200 20 68 2}" )
	# Passenger
	mygump.addRawLayout( "{button 15 105 4005 4007 1 0 3}" )
	mygump.addRawLayout( "{croppedtext 50 106 200 20 68 3}" )

	# Cancel
	mygump.addRawLayout( "{button 15 150 4017 4019 1 0 0}" )
	mygump.addRawLayout( "{croppedtext 50 152 100 20 1152 4}" )

	texts = [
		"Grant Rank to " + target_char.name,
		"Officer (Pilot, Cannons, Cargo)",
		"Crewman (Cannons, Repair, Board)",
		"Passenger (Boarding Access)",
		"Cancel"
	]
	for t in texts:
		mygump.addRawText( t )

	mygump.setArgs( [ boat.serial, target_char.serial, target_char.name ] )
	mygump.setCallback( rank_selection_callback )
	mygump.send( player )

def rank_selection_callback( player, args, target ):
	btn = target.button
	if btn == 0:
		return

	boat = wolfpack.findmulti( args[0] )
	target_serial = args[1]
	target_name = args[2]

	if not boat:
		return

	# Remove from any existing lists
	for tag in [ 'boat_officers', 'boat_crew', 'boat_passengers' ]:
		l = get_serial_list( boat, tag )
		if target_serial in l:
			l.remove( target_serial )
			set_serial_list( boat, tag, l )

	if btn == 1: # Officer
		officers = get_serial_list( boat, 'boat_officers' )
		officers.append( target_serial )
		set_serial_list( boat, 'boat_officers', officers )
		player.socket.sysmessage( tr( "%s is now an Officer on this ship." % target_name ) )
	elif btn == 2: # Crewman
		crew = get_serial_list( boat, 'boat_crew' )
		crew.append( target_serial )
		set_serial_list( boat, 'boat_crew', crew )
		player.socket.sysmessage( tr( "%s is now a Crewman on this ship." % target_name ) )
	elif btn == 3: # Passenger
		passengers = get_serial_list( boat, 'boat_passengers' )
		passengers.append( target_serial )
		set_serial_list( boat, 'boat_passengers', passengers )
		player.socket.sysmessage( tr( "%s is now a Passenger on this ship." % target_name ) )

	show_security_gump( boat, player )
