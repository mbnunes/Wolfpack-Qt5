
import wolfpack
from wolfpack.utilities import *

# Check for Strength Requirements
def onWearItem( player, wearer, item, layer ):
	if item.hastag( 'req_str' ) and wearer.strength < int( item.gettag( 'req_str' ) ):
		if player != wearer:
			player.message( 'This person can\'t wear that item, seems not strong enough.' )
			return 1
		else:
			player.message( 'You cannot wear that item, you seem not strong enough.' )
			return 1
			
	if item.hastag( 'req_dex' ) and wearer.dexterity < int( item.gettag( 'req_dex' ) ):
		if player != wearer:
			player.message( 'This person can\'t wear that item, seems not agile enough.' )
			return 1
		else:
			player.message( 'You cannot wear that item, you seem not agile enough.' )
			return 1

	if item.hastag( 'req_int' ) and wearer.intelligence < int( item.gettag( 'req_int' ) ):
		if player != wearer:
			player.message( 'This person can\'t wear that item, seems not smart enough.' )
			return 1
		else:
			player.message( 'You cannot wear that item, you seem not smart enough.' )
			return 1
			
	return 0

def onEquip( char, item, layer ):
	# Bonus Strength
	if item.hastag( 'boni_str' ):
		char.strength = char.strength + int( item.gettag( 'boni_str' ) )
	# Bonus Dex
	if item.hastag( 'boni_dex' ):
		char.dexterity = char.dexterity + int( item.gettag( 'boni_dex' ) )
	# Bonus Int
	if item.hastag( 'boni_int' ):
		char.intelligence = char.intelligence + int( item.gettag( 'boni_int' ) )
	# Update Stats
	char.updatestats()

def onUnequip( char, item, layer ):
	# Bonus Str
	if item.hastag( 'boni_str' ):
		char.strength = char.strength - int( item.gettag( 'boni_str' ) )
	# Bonus Dex
	if item.hastag( 'boni_dex' ):
		char.dexterity = char.dexterity - int( item.gettag( 'boni_dex' ) )
	# Bonus Int
	if item.hastag( 'boni_int' ):
		char.intelligence = char.intelligence - int( item.gettag( 'boni_int' ) )
	# Update Stats
	char.updatestats()
