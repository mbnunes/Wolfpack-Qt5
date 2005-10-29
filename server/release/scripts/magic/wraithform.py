
from wolfpack.consts import SPIRITSPEAK

# Wraith form gives an additional 5-20% mana leech	
def onDoDamage(char, type, amount, victim):
	manaleech = (5 + int((15 * attacker.skill[SPIRITSPEAK]) / 10))
	char.mana = min(char.maxmana, char.mana + manaleech)
	attacker.updatemana()
	return amount
