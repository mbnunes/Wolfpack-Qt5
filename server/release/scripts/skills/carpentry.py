#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: khpae                          #
#   )).-' {{ ;'`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification: Created                 #
#################################################################

from wolfpack.consts import *
from wolfpack.utilities import *
import wolfpack

def onLoad():
	wolfpack.registerglobal( HOOK_CHAR, EVENT_SKILLUSE, "skills.carpentry" )

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
	makedeed( char )

def makesmallbede( char ):
	if not char:
		return
	rmcarpentrytags( char )
	char.settag( 'carpentry_name', "Small Bed(E)" )
	char.settag( 'carpentry_type', "small_bed_e" )
	makedeed( char )

def makelargebeds( char ):
	if not char:
		return
	rmcarpentrytags( char )
	char.settag( 'carpentry_name', "Large Bed(S)" )
	char.settag( 'carpentry_type', "large_bed_s" )
	makedeed( char )

def makelargebede( char ):
	if not char:
		return
	rmcarpentrytags( char )
	char.settag( 'carpentry_name', "Large Bed(E)" )
	char.settag( 'carpentry_type', "large_bed_e" )
	makedeed( char )

def makedartboards( char ):
	if not char:
		return
	rmcarpentrytags( char )
	char.settag( 'carpentry_name', "Dart Board(S)" )
	char.settag( 'carpentry_type', "dart_board_s" )
	makedeed( char )

def makedartboarde( char ):
	if not char:
		return
	rmcarpentrytags( char )
	char.settag( 'carpentry_name', "Dart Board(E)" )
	char.settag( 'carpentry_type', "dart_board_e" )
	makedeed( char )

def makeballotbox( char ):
	if not char:
		return
	rmcarpentrytags( char )
	char.settag( 'carpentry_name', "Ballot Box" )
	char.settag( 'carpentry_type', "ballot_box" )
	makedeed( char )

def makepentagram( char ):
	if not char:
		return
	rmcarpentrytags( char )
	char.settag( 'carpentry_name', "Pentagram" )
	char.settag( 'carpentry_type', "pentagram" )
	makedeed( char )

def makeabbatoir( char ):
	if not char:
		return
	rmcarpentrytags( char )
	char.settag( 'carpentry_name', "Abbatoir" )
	char.settag( 'carpentry_type', "abbatoir" )
	makedeed( char )

def makesmallforge( char ):
	if not char:
		return
	rmcarpentrytags( char )
	char.settag( 'carpentry_name', "Small Forge" )
	char.settag( 'carpentry_type', "small_forge" )
	makedeed( char )

def makelargeforgee( char ):
	if not char:
		return
	rmcarpentrytags( char )
	char.settag( 'carpentry_name', "Large Forge(E)" )
	char.settag( 'carpentry_type', "large_forge_e" )
	makedeed( char )

def makelargeforges( char ):
	if not char:
		return
	rmcarpentrytags( char )
	char.settag( 'carpentry_name', "Large Forge(S)" )
	char.settag( 'carpentry_type', "large_forge_s" )
	makedeed( char )

def makeanvile( char ):
	if not char:
		return
	rmcarpentrytags( char )
	char.settag( 'carpentry_name', "Anvil(E)" )
	char.settag( 'carpentry_type', "anvil_e" )
	makedeed( char )

def makeanvils( char ):
	if not char:
		return
	rmcarpentrytags( char )
	char.settag( 'carpentry_name', "Anvil(S)" )
	char.settag( 'carpentry_type', "anvil_s" )
	makedeed( char )

def maketrainingdummye( char ):
	if not char:
		return
	rmcarpentrytags( char )
	char.settag( 'carpentry_name', "Training Dummy(E)" )
	char.settag( 'carpentry_type', "training_dummy_e" )
	makedeed( char )

def maketrainingdummys( char ):
	if not char:
		return
	rmcarpentrytags( char )
	char.settag( 'carpentry_name', "Training Dummy(S)" )
	char.settag( 'carpentry_type', "training_dummy_s" )
	makedeed( char )

def makepickpocketsdipe( char ):
	if not char:
		return
	rmcarpentrytags( char )
	char.settag( 'carpentry_name', "Pickpocket Dip(E)" )
	char.settag( 'carpentry_type', "pickpocket_dip_e" )
	makedeed( char )
	
def makepickpocketsdips( char ):
	if not char:
		return
	rmcarpentrytags( char )
	char.settag( 'carpentry_name', "Pickpocket Dip(S)" )
	char.settag( 'carpentry_type', "pickpocket_dip_s" )
	makedeed( char )

def makespinningwheele( char ):
	if not char:
		return
	rmcarpentrytags( char )
	char.settag( 'carpentry_name', "Spinning Wheel(E)" )
	char.settag( 'carpentry_type', "spinning_wheel_e" )
	makedeed( char )

def makespinningwheels( char ):
	if not char:
		return
	rmcarpentrytags( char )
	char.settag( 'carpentry_name', "Spinning Wheel(S)" )
	char.settag( 'carpentry_type', "spinning_wheel_s" )
	makedeed( char )

def makeloome( char ):
	if not char:
		return
	rmcarpentrytags( char )
	char.settag( 'carpentry_name', "Loom(E)" )
	char.settag( 'carpentry_type', "loom_e" )
	makedeed( char )

def makelooms( char ):
	if not char:
		return
	rmcarpentrytags( char )
	char.settag( 'carpentry_name', "Loom(S)" )
	char.settag( 'carpentry_type', "loom_s" )
	makedeed( char )

def makestoneovene( char ):
	if not char:
		return
	rmcarpentrytags( char )
	char.settag( 'carpentry_name', "Stone Oven(E)" )
	char.settag( 'carpentry_type', "stone_oven_e" )
	makedeed( char )

def makestoneovens( char ):
	if not char:
		return
	rmcarpentrytags( char )
	char.settag( 'carpentry_name', "Stone Oven(S)" )
	char.settag( 'carpentry_type', "stone_oven_s" )
	makedeed( char )

def makeflourmille( char ):
	if not char:
		return
	rmcarpentrytags( char )
	char.settag( 'carpentry_name', "Flour Mill(E)" )
	char.settag( 'carpentry_type', "flour_mill_e" )
	makedeed( char )

def makeflourmills( char ):
	if not char:
		return
	rmcarpentrytags( char )
	char.settag( 'carpentry_name', "Flour Mill(S)" )
	char.settag( 'carpentry_type', "flour_mill_s" )
	makedeed( char )

def makewatertroughe( char ):
	if not char:
		return
	rmcarpentrytags( char )
	char.settag( 'carpentry_name', "Water Trough(E)" )
	char.settag( 'carpentry_type', "water_trough_e" )
	makedeed( char )

def makewatertroughs( char ):
	if not char:
		return
	rmcarpentrytags( char )
	char.settag( 'carpentry_name', "Water Trough(S)" )
	char.settag( 'carpentry_type', "water_trough_s" )
	makedeed( char )

def rmcarpentrytags( char ):
	if char.hastag( 'carpentry_name' ):
		char.deltag( 'carpentry_name' )
	if char.hastag( 'carpentry_type' ):
		char.deltag( 'carpentry_type' )

def makedeed( char ):
	if not char.hastag( 'carpentry_name' ) or not char.hastag( 'carpentry_type'):
		rmcarpentrytags( char )
		return
	backpack = char.getbackpack()
	if not backpack:
		return
	deed = wolfpack.additem( "14ef" )
	if not deed:
		return
	deed.name = char.gettag( 'carpentry_name' )
	deed.settag( 'carpentry_type', char.gettag( 'carpentry_type' ) )
	deed.events = "carpentry_deed"
	backpack.additem( deed )
	deed.update()
	rmcarpentrytags( char )

