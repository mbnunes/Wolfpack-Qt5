
#
# Event for Locking Items
#
def onUse( char, item ):
	events = item.events

	if not item.hastag( 'lock' ):
		return 0

	lock = item.gettag( 'lock' )
	
	if lock != 'magic':
		lock = int( lock )
	
		if item.hastag( 'locked' ):
			locked = int( item.gettag( 'locked' ) )
		else:
			locked = 0
			
		if locked == 0:
			return 0
	
		# Search for a key in the users backpack
		backpack = char.getbackpack()
				
		if searchkey( backpack, lock ):
			if 'door' in events:
				char.message( 501282 ) # quickly open / relock
			return 0
	
	char.message( 502503 ) # Thats locked
	return 1
