
# Check for Strength Requirements
def onWearItem( player, wearer, item, layer ):
	if item.hastag( 'req_str' ) and wearer.strength < int( item.gettag( 'req_str' ) ):
		if player != wearer:
			player.message( 'This person seems too weak to wear that.' )
			return 1
		else:
			player.message( 'You are too weak to wear that.' )
			return 1
			
	if item.hastag( 'req_dex' ) and wearer.dexterity < int( item.gettag( 'req_dex' ) ):
		if player != wearer:
			player.message( 'This person seems too clumsy to wear that.' )
			return 1
		else:
			player.message( 'You are too clumsy to wear that.' )
			return 1			

	if item.hastag( 'req_int' ) and wearer.intelligence < int( item.gettag( 'req_int' ) ):
		if player != wearer:
			player.message( 'This person seems too dumb to wear that.' )
			return 1
		else:
			player.message( 'You are too dumb to wear that.' )
			return 1			
			
	return 0

def onEquip( char, item, layer ):
	# Boni?
	changed = 0
	
	if item.hastag( 'boni_str' ):
		char.strength = char.strength + int( item.gettag( 'boni_str' ) )
		changed = 1
		
	if item.hastag( 'boni_dex' ):
		char.dexterity = char.dexterity + int( item.gettag( 'boni_dex' ) )
		changed = 1
		
	if item.hastag( 'boni_int' ):
		char.intelligence = char.intelligence + int( item.gettag( 'boni_int' ) )
		changed = 1
		
	if changed and char.socket:
		char.socket.resendstatus()
	
def onUnequip( char, item, layer ):
	changed = 0
	
	if item.hastag( 'boni_str' ):
		char.strength = char.strength - int( item.gettag( 'boni_str' ) )
		changed = 1
		
	if item.hastag( 'boni_dex' ):
		char.dexterity = char.dexterity - int( item.gettag( 'boni_dex' ) )
		changed = 1
		
	if item.hastag( 'boni_int' ):
		char.intelligence = char.intelligence - int( item.gettag( 'boni_int' ) )
		changed = 1
		
	if changed and char.socket:
		char.socket.resendstatus()
