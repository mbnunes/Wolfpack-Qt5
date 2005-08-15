
from random import randrange
from wolfpack import tr

def onUse(char, item):
	# Out of reach?
	if not char.canreach(item, 2):
		char.socket.clilocmessage(500295) 
		return True

	char.turnto(item)
	number1 = randrange(1, 7)
	number2 = randrange(1, 7)

	char.emote( tr("rolls a ") + unicode(number1) + tr(" and a ") + unicode(number2) )	
	return True
