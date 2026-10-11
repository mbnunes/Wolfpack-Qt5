
def onDeath(char, source, corpse):
	expire(char, [])

def dispel(char, args, source, dispelargs):
	expire(char, args)

def expire(char, args):
	char.removescript('magic.mindrot')
	if char.socket:
		char.socket.clilocmessage( 1060872 ) # Your mind feels normal again.
