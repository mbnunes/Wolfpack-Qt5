#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: Viper                          #
#   )).-' {{ ;'`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification: Created                 #
#################################################################

from wolfpack.consts import *
from wolfpack.utilities import *
from wolfpack.time import *
from wolfpack.properties import *
import wolfpack
import skills
from wolfpack import weaponinfo
from wolfpack import armorinfo

ARMSLORE_DELAY = 1000

def armslore( char, skill ):
	# only handle armslore
	if skill != ARMSLORE:
		return False

	if char.socket.hastag( 'skill_delay' ):
		cur_time = servertime()
		if cur_time < char.socket.gettag( 'skill_delay' ):
			char.socket.clilocmessage( 500118, "", 0x3b2, 3 )
			return True
		else:
			char.socket.deltag( 'skill_delay' )

	char.socket.clilocmessage( 0x7A27D, "", 0x3b2, 3 )
	char.socket.attachtarget( "skills.armslore.response" )

	return True

def response( char, args, target ):

	backpack = char.getbackpack()
	item = target.item

	if item:
		if not item.getoutmostchar() == char:
			if not char.canreach( item, 4 ):
				char.socket.clilocmessage( 0x7A27F, "", 0x3b2, 3 )
				return False

			if not char.distanceto( item ) < 5:
				char.socket.clilocmessage( 0x7A27E, "", 0x3b2, 3 )
				return False

		if isweapon( item ) or isarmor( item ) or isshield( item ):
			cur_time = servertime()
			char.socket.settag( 'skill_delay', cur_time + ARMSLORE_DELAY )
			if not char.checkskill( ARMSLORE, 0, 1000 ):
				char.socket.clilocmessage( 0x7A281, "", 0x3b2, 3 )
				return False

			char.socket.clilocmessage( 0x103319, "", 0x3b2, 3 )
			condi = 10 * item.health / item.maxhealth
			if condi < 0:
				condi = 0
			elif condi > 9:
				condi = 9
			char.socket.clilocmessage( 1038285 + condi )
			if isweapon( item ):
				layer_id = 0
				if item.twohanded:
					layer_id = 1
				mindamage = fromitem( item, MINDAMAGE )
				maxdamage = fromitem( item, MAXDAMAGE )
				avdamage = ( mindamage + maxdamage ) / 2
				dmg_id = 9 * max( 0, min( int( avdamage / 5 ), 6 ) )
				if itemcheck( item, ITEM_PIERCING ):
					char.socket.clilocmessage( 1038218 + layer_id + dmg_id )
				elif itemcheck( item, ITEM_SLASHING ):
					char.socket.clilocmessage( 1038220 + layer_id + dmg_id )
				elif itemcheck( item, ITEM_BASHING ):
					char.socket.clilocmessage( 1038222 + layer_id + dmg_id )
				elif itemcheck( item, ITEM_RANGED ):
					char.socket.clilocmessage( 1038224 + dmg_id )
				else:
					char.socket.clilocmessage( 1038216 + layer_id + dmg_id )
				# check if poisoned
				if item.hastag( 'poisoning_uses' ):
					char.socket.clilocmessage( 1038284 )
			else:
				armor = fromitem( item, RESISTANCE_PHYSICAL )
				armor = armor + fromitem( item, RESISTANCE_FIRE )
				armor = armor + fromitem( item, RESISTANCE_COLD )
				armor = armor + fromitem( item, RESISTANCE_POISON )
				armor = armor + fromitem( item, RESISTANCE_ENERGY )
				arm_id = max( 0, min( int( armor / 5 ), 7 ) )
				char.socket.clilocmessage( 1038295 + arm_id )
			return True
		else:
			char.socket.clilocmessage( 0x7A280, "", 0x3b2, 3 )
			return False

	else:
		if not char.canreach( target.char, 4 ):
			char.socket.clilocmessage( 0x7A27F, "", 0x3b2, 3 )
			return False

		if not char.distanceto ( target.char ) < 5:
			char.socket.clilocmessage( 0x7A27E, "", 0x3b2, 3 )
			return False

		char.socket.clilocmessage( 0x7A280, "", 0x3b2, 3 )
		return False

def onLoad():
	skills.register( ARMSLORE, armslore )
