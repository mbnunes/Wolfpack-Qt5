
# POTIONS SCRIPT
# This script in general has all functions for potions
# more1 defines the type of the potion
# more2 the strength

# Nightsight potion
def nightsightPotion( char, potion ):
	char.message( "This potion will help you to see in the dark" )

potions = {
	1: nightsightPotion
}

def onUse( char, item ):
	# Potions need to be on your body to use them.
	if item.getoutmostchar() != char:
		char.message( "This potion needs to be in your belongings to use it." )
		return 1

	pType = item.more1

	if not potions.has_key( pType ):
		char.socket.sysmessage( "This potion has an unknown type: %u" % pType )
		return 1
	
	potions[ pType ]( char, item )
	return 1
