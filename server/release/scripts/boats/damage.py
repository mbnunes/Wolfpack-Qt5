#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Ship Damage & Emergency Repair System      #
#   )).-' {{ ;'`   |                                            #
#  ( (  ;._ \\ ctr |                                            #
#===============================================================#
import wolfpack
from wolfpack.gumps import cGump
from wolfpack import tr
from boats import security

DEFAULT_MAX_HP = {
	"boat_small": 2500,
	"dragonboat_small": 2500,
	"boat_medium": 4000,
	"dragonboat_medium": 4000,
	"boat_large": 6000,
	"dragonboat_large": 6000,
	"rowboat": 1000,
	"tokuno_galleon": 12000,
	"gargish_galleon": 14000,
	"orcish_galleon": 15000,
	"britannian_ship": 20000
}

def init_boat_hp( boat, max_hp=None ):
	if not boat:
		return
	if max_hp is None:
		base_type = str( boat.gettag( 'deedid' ) or "" )
		max_hp = DEFAULT_MAX_HP.get( base_type, 6000 )
	boat.settag( 'boat_max_hp', max_hp )
	boat.settag( 'boat_current_hp', max_hp )

def get_boat_hp( boat ):
	if not boat:
		return 0, 0
	if not boat.hastag( 'boat_max_hp' ):
		init_boat_hp( boat )
	cur = int( boat.gettag( 'boat_current_hp' ) or 0 )
	max_hp = int( boat.gettag( 'boat_max_hp' ) or 6000 )
	return cur, max_hp

def get_damage_status( boat ):
	cur, max_hp = get_boat_hp( boat )
	if max_hp <= 0:
		return "Pristine", 100
	pct = int( ( cur * 100 ) / max_hp )
	if cur <= 0:
		return "Scuttled (Disabled)", 0
	elif pct < 50:
		return "Heavily Damaged", pct
	elif pct < 75:
		return "Damaged", pct
	else:
		return "Pristine", pct

def apply_damage( boat, amount, attacker=None ):
	if not boat:
		return
	cur, max_hp = get_boat_hp( boat )
	new_cur = max( 0, cur - amount )
	boat.settag( 'boat_current_hp', new_cur )

	# Sound of impact / wood cracking
	wolfpack.sound( boat.pos, 0x13E )

	# Tillerman / Pilot notification
	status, pct = get_damage_status( boat )
	tillerman_serial = boat.gettag( 'boat_tillerman' )
	if tillerman_serial:
		tm = wolfpack.finditem( int( tillerman_serial ) )
		if tm:
			if new_cur <= 0:
				tm.say( "Mayday! The ship is scuttled and taking on water! We cannot move!" )
			else:
				tm.say( "Hull hit! Damage report: %s (%d%% HP remaining)!" % ( status, pct ) )

	# If scuttled (0 HP), disable movement
	if new_cur <= 0:
		boat.settag( 'boat_scuttled', 1 )
		from boats import tillerman
		tillerman.StopMove( boat, None, False )

def repair_boat( boat, player ):
	if not boat or not player:
		return

	if not security.can_repair( boat, player ):
		player.socket.sysmessage( tr( "You do not have permission to repair this ship." ) )
		return

	cur, max_hp = get_boat_hp( boat )
	if cur >= max_hp:
		player.socket.sysmessage( tr( "The ship's hull and sails are already in pristine condition." ) )
		return

	backpack = player.getbackpack()
	if not backpack:
		player.socket.sysmessage( tr( "You have no backpack." ) )
		return

	# Resource checks: 10 Wood (0x1bd7 boards / 0x1bdd logs) or Cloth (0x175d)
	boards = backpack.countresource( 0x1BD7, 0x0 )
	logs = backpack.countresource( 0x1BDD, 0x0 )
	cloth = backpack.countresource( 0x175D, 0x0 )

	has_repair_kit = False
	for it in backpack.items:
		if it.baseid == "ship_repair_kit":
			has_repair_kit = True
			break

	if not has_repair_kit and (boards + logs) < 10:
		player.socket.sysmessage( tr( "You need at least 10 boards or logs (or a Ship Repair Kit) to repair the hull." ) )
		return

	# Deduct resources
	if not has_repair_kit:
		if boards >= 10:
			backpack.useresource( 10, 0x1BD7 )
		else:
			backpack.useresource( 10, 0x1BDD )
		repair_amount = 600
	else:
		repair_amount = 1500

	new_hp = min( max_hp, cur + repair_amount )
	boat.settag( 'boat_current_hp', new_hp )

	# Play hammering / carpentry sound
	wolfpack.sound( player.pos, 0x02a )

	if boat.hastag( 'boat_scuttled' ) and new_hp > 0:
		boat.deltag( 'boat_scuttled' )
		player.socket.sysmessage( tr( "Emergency repairs complete! The ship is seaworthy once more!" ) )
	else:
		player.socket.sysmessage( tr( "Repaired %d hit points. Hull integrity: %d / %d." % ( repair_amount, new_hp, max_hp ) ) )

#
# Damage & Repair Inspection Gump
#
def show_repair_gump( boat, player ):
	cur, max_hp = get_boat_hp( boat )
	status, pct = get_damage_status( boat )

	mygump = cGump()
	mygump.addRawLayout( "{page 0}" )
	mygump.addRawLayout( "{resizepic 0 0 5054 360 220}" )
	mygump.addRawLayout( "{gumppictiled 10 10 340 25 2624}" )
	mygump.addRawLayout( "{checkertrans 10 10 340 25}" )
	mygump.addRawLayout( "{croppedtext 15 12 320 20 1152 0}" ) # Title: Vessel Inspection Report

	mygump.addRawLayout( "{gumppictiled 10 40 340 125 2624}" )
	mygump.addRawLayout( "{checkertrans 10 40 340 125}" )

	# Details
	mygump.addRawLayout( "{croppedtext 15 45 150 20 1152 1}" ) # Hull Integrity:
	mygump.addRawLayout( "{croppedtext 160 45 180 20 68 2}" )  # <cur> / <max> (<pct>%)

	mygump.addRawLayout( "{croppedtext 15 70 150 20 1152 3}" ) # Condition:
	mygump.addRawLayout( "{croppedtext 160 70 180 20 38 4}" )  # <status>

	mygump.addRawLayout( "{croppedtext 15 95 320 20 1152 5}" ) # Materials needed: 10 boards or Repair Kit

	# Repair Button
	mygump.addRawLayout( "{button 15 130 4005 4007 1 0 1}" )
	mygump.addRawLayout( "{croppedtext 50 132 250 20 68 6}" ) # Conduct Emergency Repairs

	# Close Button
	mygump.addRawLayout( "{gumppictiled 10 175 340 35 2624}" )
	mygump.addRawLayout( "{checkertrans 10 175 340 35}" )
	mygump.addRawLayout( "{button 15 180 4017 4019 1 0 0}" )
	mygump.addRawLayout( "{croppedtext 50 182 100 20 1152 7}" ) # Close

	texts = [
		"Vessel Inspection & Damage Report",
		"Hull Integrity:",
		"%d / %d (%d%%)" % ( cur, max_hp, pct ),
		"Status:",
		status,
		"Required: 10 Wood Boards or Repair Kit",
		"Perform Emergency Repairs",
		"Close"
	]
	for t in texts:
		mygump.addRawText( t )

	mygump.setArgs( [ boat.serial ] )
	mygump.setCallback( repair_gump_callback )
	mygump.send( player )

def repair_gump_callback( player, args, target ):
	if target.button == 1:
		boat = wolfpack.findmulti( args[0] )
		if boat:
			repair_boat( boat, player )
			show_repair_gump( boat, player )

#
# onUse for Ship Repair Kit item
#
def onUse( player, item ):
	boat = player.multi
	if not boat:
		player.socket.sysmessage( tr( "You must be aboard a ship to use the Ship Repair Kit." ) )
		return 1
	show_repair_gump( boat, player )
	return 1
