#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: khpae                          #
#   )).-' {{ ;'`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification: Created                 #
#################################################################

from wolfpack.consts import *
from wolfpack.utilities import *
import wolfpack

# name:[ { skill:[ min, max ], .. }, { resource1:num1, ... } ]
skills = { 'small_bed':[ { CARPENTRY:[ 947, 1000], TAILORING:[ 750, 900 ] }, { 0x1bd7:100, 0x175d:100 } ],
	'large_bed':[ { CARPENTRY:[ 947, 1000], TAILORING:[ 750, 900 ] }, { 0x1bd7:150, 0x175d:150 } ],
	'dart_board':[ { CARPENTRY:[ 157, 300 ] }, { 0x1bd7:5 } ],
	'ballot_box':[ { CARPENTRY:[ 473, 600 ] }, { 0x1bd7:5 } ],
	'pentagram':[ { CARPENTRY:[ 1000, 1200 ], MAGERY:[ 750, 850 ] }, { 0x1bd7:100, 0x1bf2:40 } ],
	'abbatoir':[ { CARPENTRY:[ 1000, 1200 ], MAGERY:[ 500, 600 ] }, { 0x1bd7:100, 0x1bf2:40 } ],
	'small_forge':[ { CARPENTRY:[ 736, 850 ], BLACKSMITHING:[ 750, 850 ] }, { 0x1bd7:5, 0x1bf2:75 } ],
	'large_forge':[ { CARPENTRY:[ 789, 900 ], BLACKSMITHING:[ 800, 900 ] }, { 0x1bd7:5, 0x1bf2:100 } ],
	'anvil':[ { CARPENTRY:[ 736, 850 ], BLACKSMITHING:[ 750, 850 ] }, { 0x1bd7:5, 0x1bf2:150 } ],
	'trainingdummy':[ { CARPENTRY:[ 684, 800 ], TAILORING:[ 500, 600 ] }, { 0x1bd7:55, 0x175d:60 } ],
	'pickpocketsdip':[ { CARPENTRY:[ 736, 850 ], TAILORING:[ 500, 600 ] }, { 0x1bd7:55, 0x175d:60 } ],
	'spinningwheel':[ { CARPENTRY:[ 736, 850 ], TAILORING:[ 650, 750 ] }, { 0x1bd7:25, 0x175d:25 } ],
	'loom':[ { CARPENTRY:[ 842, 950 ], TAILORING:[ 650, 750 ] }, { 0x1bd7:85, 0x175d:25 } ],
	'stoneoven':[ { CARPENTRY:[ 684, 800 ], TINKERING:[ 500, 600 ] }, { 0x1bd7:85, 0x1bf2:125 } ],
	'flourmill':[ { CARPENTRY:[ 947, 1200 ], TINKERING:[ 500, 600 ] }, { 0x1bd7:100, 0x1bf2:50 } ],
	'watertrough':[ { CARPENTRY:[ 736, 850 ] }, { 0x1bd7:150 } ] }

#def onLoad():
#	wolfpack.registerglobal( HOOK_CHAR, EVENT_SKILLUSE, "skills.carpentry" )

# skill is used via a tool
def onUse( char, item ):
	if item.getoutmostchar() != char:
		return 1
	# send makemenu
	char.sendmakemenu( "CRAFTMENU_CARPENTRY" )
	return 1

def makesmallbeds( char ):
	if not char:
		return
	rmcarpentrytags( char )
	char.settag( 'carpentry_name', "Small Bed(S)" )
	char.settag( 'carpentry_type', "small_bed_s" )
	char.settag( 'skills', "small_bed" )
	makedeed( char )

def makesmallbede( char ):
	if not char:
		return
	rmcarpentrytags( char )
	char.settag( 'carpentry_name', "Small Bed(E)" )
	char.settag( 'carpentry_type', "small_bed_e" )
	char.settag( 'skills', "small_bed" )
	makedeed( char )

def makelargebeds( char ):
	if not char:
		return
	rmcarpentrytags( char )
	char.settag( 'carpentry_name', "Large Bed(S)" )
	char.settag( 'carpentry_type', "large_bed_s" )
	char.settag( 'skills', "large_bed" )
	makedeed( char )

def makelargebede( char ):
	if not char:
		return
	rmcarpentrytags( char )
	char.settag( 'carpentry_name', "Large Bed(E)" )
	char.settag( 'carpentry_type', "large_bed_e" )
	char.settag( 'skills', "large_bed" )
	makedeed( char )

def makedartboards( char ):
	if not char:
		return
	rmcarpentrytags( char )
	char.settag( 'carpentry_name', "Dart Board(S)" )
	char.settag( 'carpentry_type', "dart_board_s" )
	char.settag( 'skills', "dart_board" )
	makedeed( char )

def makedartboarde( char ):
	if not char:
		return
	rmcarpentrytags( char )
	char.settag( 'carpentry_name', "Dart Board(E)" )
	char.settag( 'carpentry_type', "dart_board_e" )
	char.settag( 'skills', "dart_board" )
	makedeed( char )

def makeballotbox( char ):
	if not char:
		return
	rmcarpentrytags( char )
	char.settag( 'carpentry_name', "Ballot Box" )
	char.settag( 'carpentry_type', "ballot_box" )
	char.settag( 'skills', "ballot_box" )
	makedeed( char )

def makepentagram( char ):
	if not char:
		return
	rmcarpentrytags( char )
	char.settag( 'carpentry_name', "Pentagram" )
	char.settag( 'carpentry_type', "pentagram" )
	char.settag( 'skills', "pentagram" )
	makedeed( char )

def makeabbatoir( char ):
	if not char:
		return
	rmcarpentrytags( char )
	char.settag( 'carpentry_name', "Abbatoir" )
	char.settag( 'carpentry_type', "abbatoir" )
	char.settag( 'skills', "abbatoir" )
	makedeed( char )

def makesmallforge( char ):
	if not char:
		return
	rmcarpentrytags( char )
	char.settag( 'carpentry_name', "Small Forge" )
	char.settag( 'carpentry_type', "small_forge" )
	char.settag( 'skills', "small_forge" )
	makedeed( char )

def makelargeforgee( char ):
	if not char:
		return
	rmcarpentrytags( char )
	char.settag( 'carpentry_name', "Large Forge(E)" )
	char.settag( 'carpentry_type', "large_forge_e" )
	char.settag( 'skills', "large_forge" )
	makedeed( char )

def makelargeforges( char ):
	if not char:
		return
	rmcarpentrytags( char )
	char.settag( 'carpentry_name', "Large Forge(S)" )
	char.settag( 'carpentry_type', "large_forge_s" )
	char.settag( 'skills', "large_forge" )
	makedeed( char )

def makeanvile( char ):
	if not char:
		return
	rmcarpentrytags( char )
	char.settag( 'carpentry_name', "Anvil(E)" )
	char.settag( 'carpentry_type', "anvil_e" )
	char.settag( 'skills', "anvil" )
	makedeed( char )

def makeanvils( char ):
	if not char:
		return
	rmcarpentrytags( char )
	char.settag( 'carpentry_name', "Anvil(S)" )
	char.settag( 'carpentry_type', "anvil_s" )
	char.settag( 'skills', "anvil" )
	makedeed( char )

def maketrainingdummye( char ):
	if not char:
		return
	rmcarpentrytags( char )
	char.settag( 'carpentry_name', "Training Dummy(E)" )
	char.settag( 'carpentry_type', "training_dummy_e" )
	char.settag( 'skills', "training_dummy" )
	makedeed( char )

def maketrainingdummys( char ):
	if not char:
		return
	rmcarpentrytags( char )
	char.settag( 'carpentry_name', "Training Dummy(S)" )
	char.settag( 'carpentry_type', "training_dummy_s" )
	char.settag( 'skills', "training_dummy" )
	makedeed( char )

def makepickpocketsdipe( char ):
	if not char:
		return
	rmcarpentrytags( char )
	char.settag( 'carpentry_name', "Pickpocket Dip(E)" )
	char.settag( 'carpentry_type', "pickpocket_dip_e" )
	char.settag( 'skills', "pickpocket_dip" )
	makedeed( char )
	
def makepickpocketsdips( char ):
	if not char:
		return
	rmcarpentrytags( char )
	char.settag( 'carpentry_name', "Pickpocket Dip(S)" )
	char.settag( 'carpentry_type', "pickpocket_dip_s" )
	char.settag( 'skills', "pickpocket_dip" )
	makedeed( char )

def makespinningwheele( char ):
	if not char:
		return
	rmcarpentrytags( char )
	char.settag( 'carpentry_name', "Spinning Wheel(E)" )
	char.settag( 'carpentry_type', "spinning_wheel_e" )
	char.settag( 'skills', "spinning_wheel" )
	makedeed( char )

def makespinningwheels( char ):
	if not char:
		return
	rmcarpentrytags( char )
	char.settag( 'carpentry_name', "Spinning Wheel(S)" )
	char.settag( 'carpentry_type', "spinning_wheel_s" )
	char.settag( 'skills', "spinning_wheel" )
	makedeed( char )

def makeloome( char ):
	if not char:
		return
	rmcarpentrytags( char )
	char.settag( 'carpentry_name', "Loom(E)" )
	char.settag( 'carpentry_type', "loom_e" )
	char.settag( 'skills', "loom" )
	makedeed( char )

def makelooms( char ):
	if not char:
		return
	rmcarpentrytags( char )
	char.settag( 'carpentry_name', "Loom(S)" )
	char.settag( 'carpentry_type', "loom_s" )
	char.settag( 'skills', "loom" )
	makedeed( char )

def makestoneovene( char ):
	if not char:
		return
	rmcarpentrytags( char )
	char.settag( 'carpentry_name', "Stone Oven(E)" )
	char.settag( 'carpentry_type', "stone_oven_e" )
	char.settag( 'skills', "stone_oven" )
	makedeed( char )

def makestoneovens( char ):
	if not char:
		return
	rmcarpentrytags( char )
	char.settag( 'carpentry_name', "Stone Oven(S)" )
	char.settag( 'carpentry_type', "stone_oven_s" )
	char.settag( 'skills', "stone_oven" )
	makedeed( char )

def makeflourmille( char ):
	if not char:
		return
	rmcarpentrytags( char )
	char.settag( 'carpentry_name', "Flour Mill(E)" )
	char.settag( 'carpentry_type', "flour_mill_e" )
	char.settag( 'skills', "flour_mill" )
	makedeed( char )

def makeflourmills( char ):
	if not char:
		return
	rmcarpentrytags( char )
	char.settag( 'carpentry_name', "Flour Mill(S)" )
	char.settag( 'carpentry_type', "flour_mill_s" )
	char.settag( 'skills', "flour_mill" )
	makedeed( char )

def makewatertroughe( char ):
	if not char:
		return
	rmcarpentrytags( char )
	char.settag( 'carpentry_name', "Water Trough(E)" )
	char.settag( 'carpentry_type', "water_trough_e" )
	char.settag( 'skills', "water_trough" )
	makedeed( char )

def makewatertroughs( char ):
	if not char:
		return
	rmcarpentrytags( char )
	char.settag( 'carpentry_name', "Water Trough(S)" )
	char.settag( 'carpentry_type', "water_trough_s" )
	char.settag( 'skills', "water_trough" )
	makedeed( char )

def rmcarpentrytags( char ):
	if char.hastag( 'carpentry_name' ):
		char.deltag( 'carpentry_name' )
	if char.hastag( 'carpentry_type' ):
		char.deltag( 'carpentry_type' )
	if char.hastag( 'skills' ):
		char.deltag( 'skills' )

def makedeed( char ):
	if not char.hastag( 'carpentry_name' ) or not char.hastag( 'carpentry_type') or not char.hastag( 'skills' ):
		rmcarpentrytags( char )
		return
	backpack = char.getbackpack()
	if not backpack:
		return

	# interpret the skill/resource data
	item_name = char.gettag( 'skills' )
	if not skills.has_key( item_name ):
		return
	use_skills = skills[ item_name ][ 0 ].keys()
	use_items = skills[ item_name ][ 1 ].keys()
	num_items = [ 0 ] * len( use_items )

	# check if we have sufficient resources
	success = 1
	for i in range( 0, len( use_items ) ):
		num_items[ i ] = skills[ item_name ][ 1 ][ use_items[ i ] ]
		if use_items[ i ] == 0x1bf2:
			color = 0x0961
		else:
			color = 0x0
		res_count = char.countresource( use_items[ i ], color )
		if res_count < num_res[ i ]:
			success = 0
	if not success:
		return

	# check if we have sufficient skills and luck ;)
	success = 1
	for i in range( 0, len( use_skills ) ):
		skill_min = skills[ item_name ][ 0 ][ use_skills[ i ] ][ 0 ]
		skill_max = skills[ item_name ][ 0 ][ use_skills[ i ] ][ 1 ]
		success &= char.checkskill( use_skills[ i ], skill_min, skill_max )
	if not success:
		return

	# useup resources
	for i in range( 0, len( use_items ) ):
		if use_items[ i ] == 0x1bf2:
			color = 0x0961
		else:
			color = 0x0
		char.useresource( num_items[ i ], use_items[ i ], color )

	# now, create the deed
	deed = wolfpack.additem( "14ef" )
	if not deed:
		return
	deed.name = char.gettag( 'carpentry_name' )
	deed.settag( 'carpentry_type', char.gettag( 'carpentry_type' ) )
	deed.events = "carpentry_deed"
	backpack.additem( deed )
	deed.update()
	rmcarpentrytags( char )

