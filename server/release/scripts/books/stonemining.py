
from wolfpack.consts import MINING

def onUse( char, item ):
	# Has to be in our posession
	if item.getoutmostchar() != char:
		char.socket.clilocmessage(500364) # The book must be in your pack to use it.
		return False
	if char.skill[MINING] < 1000:
		char.socket.sysmessage( "Only a Grandmaster Miner can learn from this book." )
		return True
	if char.hastag( 'stonemining' ):
		char.socket.sysmessage( 'You have already learned this knowledge.' )
		return True
	char.settag( 'stonemining', 0 )
	char.socket.sysmessage( 'You have learned to mine for stones. Target mountains when mining to find stones.' )
	item.delete()
	return True
