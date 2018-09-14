
def dispel(char, args, source, dispelargs):
	expire(char, args)

def expire(char, args):
	char.soundeffect( 0xf8 )
	char.removescript('magic.divinefury')
