
import wolfpack
from wolfpack.utilities import *

# Check for Strength Requirements
def onWearItem( player, wearer, item, layer ):
	if item.hastag( 'req_str' ) and wearer.strength < int( item.gettag( 'req_str' ) ):
		if player != wearer:
			player.socket.sysmessage( 'This person can\'t wear that item, seems not strong enough.' )
			return 1
		else:
			# You are not strong enough to equip that.
			player.socket.clilocmessage( 500213, '', GRAY )
			return 1
			
	if item.hastag( 'req_dex' ) and wearer.dexterity < int( item.gettag( 'req_dex' ) ):
		if player != wearer:
			player.socket.sysmessage( 'This person can\'t wear that item, seems not agile enough.' )
			return 1
		else:
			# You do not have enough dexterity to equip this item.
			player.socket.clilocmessage( 502077, '', GRAY )
			return 1

	if item.hastag( 'req_int' ) and wearer.intelligence < int( item.gettag( 'req_int' ) ):
		if player != wearer:
			player.socket.sysmessage( 'This person can\'t wear that item, seems not smart enough.' )
			return 1
		else:
			player.socket.sysmessage( 'You are not ingellgent enough to equip this item.' )
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
