#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Created by: DarkStorm                      #
#   )).-' {{ ;'`   | Revised by:                                #
#  ( (  ;._ \\ ctr | Last Modification: Created                 #
#===============================================================#
# Utility functions used in Python scripts                      #
#===============================================================#

# Calculates the Magic Damage (Base Damage + Base + Source)

import wolfpack
from wolfpack.consts import *
from math import floor
import whrandom
from types import *

def magicdamage( basedamage, spell, mage, defender ):
	#DIRECT DAMAGE SPELLS:	
	# Information found on uo.stratics.com
	# (Including: magic arrow, harm, fireball, lightning, energy bolt, explosion, chain lightning, meteor swarm, flamestrike, and Earthquake)	
	# Actual_Damage_done = (Base_Spell_Damage - 50% [if Resisted]) + Eval_Intelligence_Damage_Modifier
	# Figuring your chance to resist a spell is done as follows:
	# Your chance to resist a spell is the greater of either:
	# Your_Chance_to_Resist = Your_Resistance / 5
	# Your resistance skill minus the sum of (your attackers magery skill (less 20 points) divided by five) and (the circle of the spell multiplied by 5)
	# Your_Chance_to_Resist = Your_Resistance - (( Attackers_Magery - 20 )/5 + Spell_Circle*5))

	circle = 1 + floor( spell / 8 )

	# Calculate the resistances
	chance1 = floor( defender.skill[ MAGICRESISTANCE ] / 5 )
	chance2 = floor( defender.skill[ MAGICRESISTANCE ] - ( mage.skill[ MAGERY ] / 5 + circle * 5 ) )

	chance = max( [ chance1, chance2 ] ) # The higher chance is used

	whrandom.seed()

	damage = basedamage

	# Check succeeded
	if whrandom.randrange( 0, 99 ) < chance:
		damage /= 2

	#The remaining damage is multiplied against a small equation:
	# If your resistance is higher than your opponents evaluate intelligence, then the equation is:
	#(1 + (Attackers_EI - Your_resistance) / 200 )
	# If your resistance is lower than your opponents evaluate intelligence, then the equation is:
	#(1 + (Attackers_EI - Your_resistance) / 500 )
	if( defender.skill[ MAGICRESISTANCE ] > mage.skill[ EVALUATINGINTEL ] ):
		damage = damage * ( 1 + ( mage.skill[ EVALUATINGINTEL ] - defender.skill[ MAGICRESISTANCE ] ) / 200 )
	else:
		damage = damage * ( 1 + ( mage.skill[ EVALUATINGINTEL ] - defender.skill[ MAGICRESISTANCE ] ) / 500 )

	return damage

# Shows Fizzle Animation + Sound
def fizzle( char ):
	char.effect( 0x3735, 1, 30 )
	char.soundeffect( 0x5c )

# Stat Modifier (Bless, Curse, etc.)

def statmodifier_dispel( char, args, source, dispelargs ):
	# Normally reduce "if not 'silent' in dispelargs"
	char.strength -= args[0]
	char.dexterity -= args[1]
	char.intelligence -= args[2]

	if not "silent" in dispelargs:
		# If Str changed, resend to others
		if args[0] != 0:
			char.updatehealth()
		char.updatestats()

	return

def statmodifier_expire( char, args ):
	# The stat modifier normally expired, so just reduce the stats and resend
	char.strength -= args[0]
	char.dexterity -= args[1]
	char.intelligence -= args[2]

	# If Str changed, resend to others
	if args[0] != 0:
		char.updatehealth()
	char.updatestats()

	return

def statmodifier( char, time, mStr = 0, mDex = 0, mInt = 0 ):
	# Dispel any old stat modifiers on this character
	# And readd a new one (remove the old ones silently)
	char.dispel( char, 0, "statmodifier", [ "silent" ] )

	char.strength += mStr
	char.dexterity += mDex
	char.intelligence += mInt

	if mStr != 0:
		char.updatehealth()

	char.updatestats()

	# Save the values in a tempeffect to remove them later
	char.addtimer( time, "wolfpack.utilities.statmodifier_expire", [ mStr, mDex, mInt ], 1, 1, "statmodifier", "wolfpack.utilities.statmodifier_dispel" )

def hex2dec( value ):
	if type( value ) is IntType:
		return value

	if not type( value ) is StringType:
		raise TypeError, "Can only Convert Integers and Strings"

	value = value.lower()

	if not value.startswith( "0x" ):
		return int( value )
	else:
		return int( value.replace( "0x", "" ), 16 )

def evenorodd( value ):
	if value % 2 == 0:
		return "even"
	else:
		return "odd"

def itemsmatch( a, b ):
	return a.type == b.type and a.id == b.id and a.color == b.color and a.events == b.events

def tobackpack( item, char ):
	backpack = char.getbackpack()
	return tocontainer( item, backpack )

def tocontainer( item, container ):
	for content in container.content:
		# Found an item to stack with
		if itemsmatch( content, item ):
			if content.amount + item.amount <= 65535:
				content.amount = content.amount + item.amount
				content.update()
				item.delete()
				return 1 # Stacked

	# We couldn't stack
	container.additem( item, 1, 1, 0 )
	return 0 # Not stacked

def cont2cont( container1, container2 ):
	for item in container1.content:
		tocontainer( item, container2 )
		item.update()
		

def isclothing( item ):

	clothes = [ 0x152e, 0x152f, 0x1530, 0x1531, 0x152e, 0x1537, 0x1538, 0x1539, \
			0x153a, 0x153b, 0x153c, 0x153d, 0x153e, 0x1541, 0x1542, 0x1712, \
			0x170b, 0x170c, 0x170d, 0x170e, 0x170f, 0x1710, 0x1711, 0x1712, \
			0x1f00, 0x1f01, 0x1f02, 0x1f03, 0x1f04, 0x1f7b, 0x1f7c, 0x1f9f, \
			0x1fa0, 0x1fa1, 0x1fa2, 0x1ffd, 0x1ffe, 0x2649, 0x264a, 0x265f, \
			0x2660, 0x2661, 0x2662, 0x2663, 0x2664, 0x2665, 0x2666, 0x267b, \
			0x267c, 0x267d, 0x267e, 0x267f, 0x2680, 0x2681, 0x2682, 0x2683, \
			0x2684, 0x2685, 0x2686, 0x268d, 0x268e, 0x268f, 0x26b0, 0x1515, \
			0x1516, 0x1517, 0x1518, 0x1efe, 0x1eff ]

	if item.id in clothes:
		return 1
	else:
		return 0

def ishat ( item ):

	hats = [ 0x153f, 0x1540, 0x1543, 0x1544, 0x1545, 0x1546, 0x1547, 0x1548, \
		0x1713, 0x1714, 0x1715, 0x1716, 0x1717, 0x1718, 0x1719, 0x171a, \
		0x171b, 0x171c ]

	if item.id in hats:
		return 1
	else:
		return 0

def isarmor( item ):

	armors = [ 0x13be, 0x13bf, 0x13c0, 0x13c3, 0x13c4, 0x13c5, 0x13c6, 0x13c7, 0x13cb, \
			0x13cc, 0x13cd, 0x13ce, 0x13d2, 0x13d3, 0x13d4, 0x13d5, 0x13d6, \
			0x13da, 0x13db, 0x13dc, 0x13dd, 0x13e1, 0x13e2, 0x13d2, 0x13eb, \
			0x13ec, 0x13ed, 0x13ee, 0x13ef, 0x13f0, 0x13f1, 0x13f2, 0x1408, \
			0x1409, 0x140a, 0x140b, 0x140c, 0x140d, 0x140e, 0x140f, 0x1410, \
			0x1411, 0x1412, 0x1413, 0x1414, 0x1415, 0x1416, 0x1417, 0x1418, \
			0x1419, 0x141a, 0x144e, 0x144f, 0x1450, 0x1451, 0x1452, 0x1453, \
			0x1454, 0x1455, 0x1456, 0x1457, 0x2642, 0x2643, 0x2644, 0x2645, \
			0x2658, 0x2642, 0x2641, 0x2648, 0x2657, 0x1c00, 0x1c01, 0x1c02, \
			0x1c03, 0x1c04, 0x1c05, 0x1c06, 0x1c07, 0x1c08, 0x1c09, 0x1c0a, \
			0x1c0b, 0x1c0c, 0x1c0d, 0x1f0b, 0x1f0c, 0x1549, 0x154a, 0x154b, \
			0x154c, 0x2646, 0x2647, 0x13bb ] 

	if item.id in armors:
		return 1
	else:
		return 0

def isweapon( item ):

	weapontypes = [ 1001, 1002, 1003, 1004, 1005, 1006, 1007 ]

	if item.type in weapontypes:
		return 1
	else:
		return 0

def isshield( item ):
	shields = [ 0x1b72, 0x1b73, 0x1b74, 0x1b75, 0x1b76, 0x1b77, 0x1b78, \
		 0x1b79, 0x1b7a, 0x1b7b, 0x1bc3, 0x1bc4, 0x1bc5 ]

	if item.id in shields:
		return 1
	else: 
		return 0

def isspellbook( item ):
	sbtypes = [ 9 ]
	sbids = [ 0xe3b, 0xefa, 0x2252, 0x2253 ]
	if item.id in sbids and item.type in sbtypes:
		return 1
	else:
		return 0
	        
def isinstrument( item ):
	return item.id in [ 0xE9C, 0xE9D, 0xE9E, 0xEB1, 0xEB2, 0xEB3, 0xEB4 ]


def isminingtool( item ):
	return item.id in [ 0xf39, 0xf3a, 0xe85, 0xe86 ]

def isoregem( item ):
	return item.id == 0x1ea7 and item.hastag( 'resource' ) and item.gettag( 'resource' ) == 'ore'

def ismountainorcave( tile ):
	return tile in range( 220, 231 ) + \
			range( 236, 247 ) + \
			range( 252, 263 ) + \
			range( 268, 279 ) + \
			range( 286, 297 ) + \
			range( 321, 324 ) + \
			range( 467, 474 ) + \
			range( 476, 487 ) + \
			range( 492, 495 ) + \
			range( 543, 579 ) + \
			range( 581, 621 ) + \
			range( 700, 715 ) + \
			range( 1339, 1359 ) + \
			range( 1361, 1363 ) + \
			range( 1386, 1386 ) + \
			range( 1741, 1757 ) + \
			range( 1771, 1790 ) + \
			range( 1801, 1824 ) + \
			range( 1831, 1854 ) + \
			range( 1861, 1884 ) + \
			range( 1981, 2004 ) + \
			range( 2028, 2033 ) + \
			range( 2100, 2105 ) + \
			range( 16233, 16248 ) + \
			range( 17419, 17438 ) + \
			range( 17723, 17747 ) + \
			range( 23335, 23358 ) + \
			range( 29505, 29521 ) + \
			range( 29745, 29753 ) + \
			range( 30009, 30012 )

def issand( tile ):
	return tile in range(   22,   62 ) + \
			range(   68,   75 ) + \
			range(  286,  301 ) + \
			range(  402,  402 ) + \
			range(  424,  427 ) + \
			range(  441,  465 ) + \
			range(  642,  645 ) + \
			range(  650,  657 ) + \
			range(  821,  828 ) + \
			range(  833,  836 ) + \
			range(  845,  852 ) + \
			range(  857,  860 ) + \
			range(  951,  958 ) + \
			range(  967,  970 ) + \
			range( 1447, 1458 ) + \
			range( 1611, 1618 ) + \
			range( 1623, 1626 ) + \
			range( 1635, 1642 ) + \
			range( 1647, 1650 )

def iscrystal( tile ):
	return tile in range( 25904, 25132 )

def cleartag( self, args ):
	char = args[0]
	tagname = args[1]
	self.deltag( tagname )
	return OK

def rusmsg( player, locmsg ):
	player.socket.sysmessage( unicode( locmsg, 'cp1251') )
	return OK
