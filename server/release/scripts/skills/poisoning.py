#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: khpae                          #
#   )).-' {{ ;'`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification: Created                 #
#################################################################

from wolfpack.consts import *
from wolfpack.time import *
from wolfpack.utilities import tobackpack
import skills
import wolfpack
import random
from system import poison

POISONING_DELAY = 1000

# Button for AnimalTaming pressed on skill gump
def poisoning( char, skill ):
	# you cannot use skill while dead
	if char.dead:
		char.socket.clilocmessage( 502796 )
		return

	if skill != POISONING:
		return

	# assign target request
	char.socket.clilocmessage( 502137 )
	char.socket.attachtarget("skills.poisoning.selectpotion")
	return 1

def selectpotion(char, args, target):
	# check if the targetted item is a poison potion
	potion = target.item
	if ( not potion.hastag( 'potiontype' ) ) or ( potion.gettag( 'potiontype' ) < 14  and potion.gettag( 'potiontype' ) > 17 ):
		char.socket.clilocmessage( 502139 )
		return

	# assign target request
	char.socket.clilocmessage( 502142 )
	char.socket.attachtarget( "skills.poisoning.selecttarget", [ potion ] )
	return 1

def selecttarget( char, args, target ):
	# you cannot use skill while dead
	if char.dead:
		socket.clilocmessage( 502796 )
		return

	potion = args[ 0 ]
	if not potion:
		return

	# target item must exists in backpack
	#if not target.item.getoutmostitem() == char.backpack:
	#	char.socket.clilocmessage( 500295 )
	#	return

	# check target type : food / blades
	if not target.item:
		char.socket.clilocmessage( 502145 )
		return
	if not ( 'food' in target.item.events or 'blades' in target.item.events ):
		char.socket.clilocmessage( 502145 )
		return

	# already poisoned
	#if target.hastag( 'poisoning' ):
	#	return

	# sound / effect
	char.soundeffect( 0x4F )
	# apply poison to the item
	char.addtimer( POISONING_DELAY, "skills.poisoning.poisonit", [ potion, target.item ] )
	return 1

def poisonit( char, args ):
	potion = args[ 0 ]
	item = args[ 1 ]
	if not potion or not item:
		return

	skill = char.skill[ POISONING ]

	# poison strength : lesser(1), normal, greater, deadly(4)
	strength = int( potion.gettag( 'potiontype' ) ) - 13

	# consume the potion / add a blank bottle
	potion.delete()
	bottle = wolfpack.additem( 'f0e' )
	if not tobackpack( bottle, char ):
		bottle.moveto( char.pos.x, char.pos.y, char.pos.z, char.pos.map )
	bottle.update()

	# FIXME : success / fail chance
	if skill < ( strength + 0.8 ) * 20:
		# failed to poison item
		char.socket.clilocmessage( 1010518 )
		return 1

	# check skill advance

	# FIXME : less / strong chance
	if random.randint( 0, 20 ) > ( skill - strength * 20 ):
		strength = strength - 1
		if strength < 1:
			# failed to poison
			char.socket.clilocmessage( 502148 )
			return 1
		char.socket.clilocmessage( 1010518 )
	else:
		char.socket.clilocmessage( 1010517 )

	# decrease karma / fame

	# set poisoning infos as tags
	item.settag( 'poisoning_char', char.serial )
	item.settag( 'poisoning_strength', strength )
	item.settag( 'poisoning_skill', skill )
	# weapon : poison chance when hit % = char.skill[ POISONING ] / 4
	# 	number of uses before the poison wears off
	if 'blades' in item.events:
		item.settag( 'poisoning_uses', 20 - strength * 2 )
	return 1

def hitEffect( char, weapon ):
	if not char or not weapon:
		return
	if not weapon.hastag( 'poisoning_uses' ):
		return
	uses = weapon.gettag( 'poisoning_uses' )
	if uses < 1:
		skills.poisoning.wearoff( weapon )
		return
	strength = weapon.gettag( 'poisoning_strength' )
	if not strength or not ( strength > 0 and strength < 5 ):
		skills.poisoning.wearoff( weapon )
		return
	skill = weapon.gettag( 'poisoning_skill' )
	chance = int( skill / 4 )
	if chance > random.randint( 0, 100 ):
		poison.poison( char, strength )
		uses = uses - 1
		if uses > 0:
			weapon.deltag( 'poisoning_uses' )
			weapon.settag( 'poisoning_uses', uses )
		else:
			skills.poisoning.wearoff( weapon )

def wearoff( item ):
	if item.hastag( 'poisoning_uses' ):
		item.deltag( 'poisoning_uses' )
	if item.hastag( 'poisoning_strength' ):
		item.deltag( 'poisoning_strength' )
	if item.hastag( 'poisoning_skill' ):
		item.deltag( 'poisoning_skill' )
	if item.hastag( 'poisoning_char' ):
		item.deltag( 'poisoning_char' )
		
def onLoad():
	skills.register( POISONING, poisoning )
