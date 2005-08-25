
from wolfpack.consts import CARPENTRY
from wolfpack import tr

def onUse( char, item ):
	# Has to be in our posession
	if item.getoutmostchar() != char:
		char.socket.clilocmessage(500364) # The book must be in your pack to use it.
		return False
	if char.skill[CARPENTRY] < 1000:
		char.socket.sysmessage( tr('Only a Grandmaster Carpenter can learn from this book.') )
		return True
	if char.hastag( 'masonry' ):
		char.socket.sysmessage( tr('You have already learned this information.') )
		return True
	char.settag( 'masonry', 0 )
	char.socket.sysmessage( tr('You have learned to make items from stone. You will need miners to gather stones for you to make these items.') )
	item.delete()
	return True
