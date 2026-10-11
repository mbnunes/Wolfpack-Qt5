import random

def onShowPaperdoll(char, player):
	ran = random.randint(0, 100)
	if ran < 20:
		char.soundeffect(112)
	elif ran < 40:
		char.soundeffect(114)
	return
