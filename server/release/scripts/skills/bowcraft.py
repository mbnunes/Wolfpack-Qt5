#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: khpae									 #
#   )).-' {{ ;'`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification: Created                 #
#################################################################

import wolfpack
import wolfpack.utilities
from wolfpack.consts import *
from math import *

# tool : "1022", "1023"
# log : "1bdd - 1be2"
# board : "1bd7 - 1bdc"
# shaft : ("1024", "1025",) "1bd4 - 1bd6"
# feather : ("dfa", "dfb",) "1bd1 - 1bd3"
# arrow : "f3f - f41", 0x0f40, 0x0f41
# bolt : "1bfb - 1bfd"

def onLoad():
	wolfpack.registerglobal( HOOK_CHAR, EVENT_SKILLUSE, "skills.bowcraft" )

def onUse( char, item ):
	# Needs to be on ourself
	if item.getoutmostchar() != char:
		char.socket.clilocmessage( 0x7A258 )
		return 1
	# is it in use already ?
	#if item.gettag( "use" ):
		#char.socket.clilocmessage( )
	#	return 1

	# bowcraft menu gump
	char.sendmakemenu( "CRAFTMENU_BOWCRAFT" )

	# set response function

	return 1

# these are moved to python script b/c it will make all the logs into the items
def makeshaft( char ):
	if not char:
		return
	backpack = char.getbackpack()
	if not backpack:
		return

	# count logs and boards
	num_logs = char.countresource( 0x1bdd )
	num_boards = char.countresource( 0x1bd7 )
	num_shafts = num_logs + num_boards
	if not num_shafts:
		return

	# use up resources
	if num_logs:
		char.useresource( num_logs, 0x1bdd )
	if num_boards:
		char.useresource( num_boards, 0x1bd7 )

	char.checkskill( BOWCRAFT, 0, 400 )
	#chance = skills.bowcraft.shaft_chance( char ) / 100
	#num_shafts = floor( chance * num_shafts )
	#if not num_shafts:
	#	return
	# make and insert item into backpack
	if num_shafts == 1:
		item = "1bd4"
	elif num_shafts < 6:
		item = "1bd5"
	else:
		item = "1bd6"
	shafts = wolfpack.additem( item )
	if not shafts:
		return
	shafts.amount = num_shafts
	backpack.additem( shafts )
	shafts.update()

def makearrow( char ):
	skills.bowcraft.makearrows( char, "arrow" )

def makebolt( char ):
	skills.bowcraft.makearrows( char, "bolt" )

def makearrows( char, str ):
	if not char:
		return
	
	# count resources and set the number to make
	num_shafts = char.countresource( 0x1bdd )
	num_feathers = char.countresource( 0x1bd1 )
	num_arrows = min( num_feathers, num_shafts )
	if not num_arrows:
		return

	# use up resources
	char.useresource( num_arrows, 0x1bdd )
	char.useresource( num_arrows, 0x1bd1 )

	char.checkskill( BOWCRAFT, 0, 400 )
	# make and insert shafts into char's backpack
	#chance = skills.bowcraft.shaft_chance( char ) / 100
	#num_arrows = floor( chance * num_arrows )
	#if not num_arrows:
	#	return
	if str == "arrow":
		if num_arrows == 1:
			item = "f3f"
		elif num_arrows < 6:
			item = "f40"
		else:
			item = "f41"
	else:
		if num_arrows == 1:
			item = "1bfb"
		elif num_arrows < 6:
			item = "1bfc"
		else:
			item = "1bfd"
	arrows = wolfpack.additem( item )
	backpack = char.getbackpack()
	if not arrows or not backpack:
		return
	arrows.amount = num_arrows
	backpack.additem( arrows )
	arrows.update()

# % success chance on making shaft/arrow/bolt
def shaft_chance( char ):
	skill = char.skill[ BOWCRAFT ]
	chance = 50 + floor( 0.125 * skill )
	chance = min( chance, 100 )
	return chance
