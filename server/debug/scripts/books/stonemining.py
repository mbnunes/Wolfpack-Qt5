
from wolfpack.consts import MINING
from wolfpack import tr

def onUse( char, item ):
	# Has to be in our posession
	if item.getoutmostchar() != char:
		char.socket.clilocmessage(500364) # The book must be in your pack to use it.
		return False
	if char.skill[MINING] < 1000:
		char.socket.sysmessage( tr('Only a Grandmaster Miner can learn from this book.') )
		return True
	if char.hastag( 'stonemining' ):
		char.socket.sysmessage( tr('You have already learned this knowledge.') )
		return True
	char.settag( 'stonemining', 0 )
	char.socket.sysmessage( tr('You have learned to mine for stones. Target mountains when mining to find stones.') )
	item.delete()
	return True
