#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: khpae                          #
#   )).-' {{ ;'`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification: Created                 #
#################################################################

from wolfpack.consts import *
from wolfpack.utilities import *
import wolfpack

gems = { 0xf0f:'star sapphire', 0xf10:'emerald', 0xf11:'sapphire',
	0xf12:'sapphire', 0xf13:'ruby', 0xf14:'ruby', 0xf15:'citrine',
	0xf16:'amethyst', 0xf17:'amethyst', 0xf18:'tourmaline', 0xf19:'sapphire',
	0xf1a:'ruby', 0xf1b:'star sapphire', 0xf1c:'rubby', 0xf1d:'ruby',
	0xf1e:'tourmaline', 0xf1f:'sapphire', 0xf20:'tourmaline',
	0xf21:'star sapphire', 0xf22:'amethyst', 0xf23:'citrine',
	0xf24:'citrine', 0xf25:'amber', 0xf26:'diamond', 0xf27:'diamond',
	0xf28:'diamond', 0xf29:'diamond', 0xf2a:'ruby', 0xf2b:'ruby',
	0xf2c:'citrine', 0xf2d:'tourmaline', 0xf2e:'amethyst', 0xf2f:'emerald',
	0xf30:'diamond' }

#def onLoad():
#	wolfpack.registerglobal( HOOK_CHAR, EVENT_SKILLUSE, "skills.tinkering" )

# skill is used via a tool
def onUse( char, item ):
	if item.getoutmostchar() != char:
		return 1
	# send makemenu
	char.sendmakemenu( "CRAFTMENU_TINKERING" )
	return 1

def makering( char ):
	if not char:
		return
	cleartinkertags( char )
	char.settag( 'tinkering', "ring" )
	char.settag( 'tinkeritem', "108a" )
	sendgemtarget( char.socket )

#def makesilvernecklace( char ):

#def makegoldennecklace( char ):

def makenecklace( char ):
	if not char:
		return
	cleartinkertags( char )
	char.settag( 'tinkering', "necklace" )
	char.settag( 'tinkeritem', "1089" )
	sendgemtarget( char.socket )

def makeearring( char ):
	if not char:
		return
	cleartinkertags( char )
	char.settag( 'tinkering', "ear ring" )
	char.settag( 'tinkeritem', "1087" )
	sendgemtarget( char.socket )

def makebracelet( char ):
	if not char:
		return
	cleartinkertags( char )
	char.settag( 'tinkering', "bracelet" )
	char.settag( 'tinkeritem', "1086" )
	sendgemtarget( char.socket )

def sendgemtarget( socket ):
	socket.clilocmessage( 502934, "", 0x3b2, 3 )
	socket.attachtarget( 'skills.tinkering.response' )

def cleartinkertags( char ):
	if char.hastag( 'tinkering' ):
		char.deltag( 'tinkering' )
	if char.hastag( 'tinkeritem' ):
		char.deltag( 'tinkeritem' )

def response( char, args, target ):
	if not char.hastag( 'tinkering' ) or not char.hastag( 'tinkeritem' ):
		return
	backpack = char.getbackpack()
	if not backpack:
		return
	if not target.item:
		return
	gem = target.item
	if not gem.getoutmostchar() == char:
		char.socket.clilocmessage( 1042265, "", 0x3b2, 3 )
		return
	# is it a valid gem ?
	if not gems.haskey( gem.id ):
		return
	# check ingot
	if backpack.countresource( 0x1bf2, 0x0961 ) < 2:
		return
	# use ingot
	backpack.useresource( 2, 0x1bf2, 0x0961 )
	# use the gem
	backpack.useresource( gem.id )
	# skill check
	success = char.checkskill( TINKERING, 40, 50 )
	if not success:
		return
	item = char.gettag( 'tinkeritem' )
	# make item, name it, add in the backpack
	jewelry = wolfpack.additem( item )
	jewelry.name = "a %s %s" % ( gems[ gem.id], char.gettag( 'tinkering' ) )
	backpack.additem( jewelry )
	jewelry.update()
	cleartinkertags( char )
