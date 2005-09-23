from wolfpack.consts import MAGICRESISTANCE

def onDamage(char, type, amount, source):
	amount = int(amount * 1.25)
	char.dispel(char, True, 'EVILOMEN')
	return amount

def dispel(char, args, source, dispelargs):
	expire(char, dispelargs)

def expire(char, args):
	char.removescript('magic.evilomen')
	if char.hastag('magicresistance'):
		char.skill[MAGICRESISTANCE] = char.gettag('magicresistance')
		char.deltag('magicresistance')
	return
