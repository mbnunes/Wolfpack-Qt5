#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: khpae                          #
#   )).-' {{ ;'`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification: Created                 #
#################################################################

from wolfpack.consts import *
from wolfpack.utilities import tobackpack
import skills
import wolfpack
import wolfpack.time
import random
from system import poison
import potions

POISONING_DELAY = 1000

# Min/Max-Skills for the various poison levels
MINSKILLS = [0, 300, 600, 950]
MAXSKILLS = [600, 700, 1000, 1000]

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
	if ( not potion or not potion.hasintproperty( 'potiontype' ) ) or ( potion.getintproperty( 'potiontype' ) < 14  and potion.getintproperty( 'potiontype' ) > 17 ):
		char.socket.clilocmessage( 502139 )
		return

	# assign target request
	char.socket.clilocmessage( 502142 )
	char.socket.attachtarget( "skills.poisoning.selecttarget", [ potion.serial ] )
	return 1

# Check for the following weapons: Butchers Knife, Cleaver, Dagger, Double Bladed Staff, Pike, Kryss
ALLOWED = [ 0x13f6, 0x13f7, 0xec2, 0xec3, 0xf51, 0xf52, 0x26bf, 0x26c9, 0x26be, 0x26c8, 0x1400, 0x1401 ]

def selecttarget( char, args, target ):
	# you cannot use skill while dead
	if char.dead:
		socket.clilocmessage( 502796 )
		return

	potion = wolfpack.finditem(args[ 0 ])
	if not potion:
		return

	if not target.item:
		char.socket.clilocmessage( 502145 )
		return

	global ALLOWED
	if not target.item.id in ALLOWED and not target.item.hasscript('food'):
		char.socket.clilocmessage(1060204)
		return

	if skills.skilltable[ POISONING ][ skills.UNHIDE ] and char.hidden:
		char.removefromview()
		char.hidden = False
		char.update()

	# sound / effect
	char.soundeffect( 0x4F )
	# apply poison to the item
	char.addtimer( POISONING_DELAY, "skills.poisoning.poisonit", [ potion.serial, target.item.serial ] )
	return 1

def poisonit( char, args ):
	potion = wolfpack.finditem(args[ 0 ])
	item = wolfpack.finditem(args[ 1 ])
	if not potion or not item:
		return

	skill = char.skill[ POISONING ]

	# poison strength : lesser(0), normal, greater, deadly(3)
	strength = int( potion.getintproperty( 'potiontype' ) ) - 14

	# consume the potion / add a blank bottle
	potions.consumePotion(char, potion, True)

	if not char.checkskill( POISONING, MINSKILLS[strength], MAXSKILLS[strength] ):
		char.socket.clilocmessage( 1010518 )
		return 1

	char.socket.clilocmessage(1010517)

	# decrease karma / fame

	# set poisoning infos as tags
	item.settag( 'poisoning_char', char.serial )
	item.settag( 'poisoning_strength', strength )
	item.settag( 'poisoning_skill', skill )
	# weapon : poison chance when hit % = char.skill[ POISONING ] / 4
	# 	number of uses before the poison wears off
	if item.hasscript( 'blades' ):
		item.settag( 'poisoning_uses', 18 - strength * 2 )
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
	chance = int( skill / 40 )
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

