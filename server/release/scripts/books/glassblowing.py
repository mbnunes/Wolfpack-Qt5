
from wolfpack.consts import ALCHEMY

def onUse( char, item ):
	# Has to be in our posession
	if item.getoutmostchar() != char:
		char.socket.clilocmessage(500364) # The book must be in your pack to use it.
		return False
	if char.skill[ALCHEMY] < 1000:
		char.socket.sysmessage( "Only a Grandmaster Alchemist can learn from this book." )
		return True
	if char.hastag( 'glassblowing' ):
		char.socket.sysmessage( 'You have already learned this information.' )
		return True
	char.settag( 'glassblowing', 0 )
	char.socket.sysmessage( 'You have learned to make items from glass. You will need to find miners to mine find sand for you to make these items.' )
	item.delete()
	return True
