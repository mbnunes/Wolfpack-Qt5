
from whrandom import randrange

def onUse(char, item):
	# Out of reach?
	if not char.canreach(item, 2):
		char.socket.clilocmessage(500295) 
		return True

	char.turnto(item)
	zahl1 = randrange(1, 7)
	zahl2 = randrange(1, 7)

	char.emote( "rolls a " + unicode(zahl1) + " and a " + unicode(zahl2) )	
	return True
