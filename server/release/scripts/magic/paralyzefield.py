
import wolfpack
from wolfpack.consts import *

#
# Attach a tag to the character to note that he entered a poisonfield.
#
def onCollide(char, item):
	if char.dead or char.invulnerable:
		return

	strength = 0
	if item.hastag('strength'):
		strength = int(item.gettag('strength'))

	duration = int((strength / 10.0 - char.skill[MAGICRESISTANCE] / 10.0) * 300)

	char.disturb()
	char.frozen = 1
	char.resendtooltip()
	char.effect(0x376a, 6, 1)
	char.soundeffect(0x204)

	if not char.player:
		duration *= 3

	char.addtimer(duration, "magic.circle5.paralyze_expire", [], 1)
	return 0
