
from wolfpack.consts import MINING

def onUse( char, item ):
	# Has to be in our posession
	if item.getoutmostchar() != char:
		char.socket.clilocmessage(500364) # The book must be in your pack to use it.
		return False
	if char.skill[MINING] < 1000:
		char.socket.sysmessage( "Only a Grandmaster Miner can learn from this book." )
		return True
	if char.hastag( 'sandmining' ):
		char.socket.sysmessage( 'You have already learned this information.' )
		return True
	char.settag( 'sandmining', 0 )
	char.socket.sysmessage( 'You have learned how to mine fine sand. Target sand areas when mining to look for fine sand.' )
	item.delete()
	return True
