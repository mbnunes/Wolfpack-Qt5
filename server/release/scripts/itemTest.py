
def onUse( char, item ):
	char.x = item.morex
	char.y = item.morey
	char.z = item.morez
	char.update() # Important!
	char.message( "You have been teleported" )
	
	return 1 # Interrupt the chain