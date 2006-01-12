
from wolfpack.utilities import changeResistance

def dispel(char, args, source, dispelargs):
	expire(char, args)

def expire(char, args):
	if char.socket and not args:
		char.socket.clilocmessage( 1061688 ) # Your skin returns to normal.
	char.removescript('magic.corpseskin')
	char.updatestats()
	return
