
# You have to have one hand free for using a potion
# This is not valid for explosion potions
def canUsePotion( char, item ):
	firsthand = char.itemonlayer( 1 )
	secondhand = char.itemonlayer( 2 )

	if not firsthand and not secondhand:
		return True

	if firsthand and not secondhand and not firsthand.twohanded:
		return True

	if not firsthand and secondhand and not secondhand.twohanded:
		return True

	# You must have a free hand to drink a potion.
	char.socket.clilocmessage( 0x7A99C )
	return False

# Consume the potion
def consumePotion( char, potion, givebottle=True ):

	if potion.amount == 1:
		potion.delete()
	else:
		potion.amount -= 1
		potion.update()

	# Lets add an empty bottle!
	if givebottle:
		# Empty Bottle Definition
		bottle = wolfpack.additem( 'f0e' )
		if not wolfpack.utilities.tocontainer( bottle, char.getbackpack() ):
			bottle.update()
