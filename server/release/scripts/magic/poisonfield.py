
import wolfpack
from wolfpack.utilities import energydamage
from system import poison

#
# Attach a tag to the character to note that he entered a poisonfield.
#
def onCollide(char, item):
	if char.dead or char.invulnerable:
		return

	if not char.hastag('in_poisonfield'):
		level = 0
		if item.hastag('level'):
			level = int(item.gettag('level'))

		char.settag('in_poisonfield', 1)
		if char.poison < level:
			char.soundeffect(0x474)
			poison.poison(char, level)
		char.addtimer(350, 'magic.poisonfield.expire', [])
	return 0

#
# See if we're still in a poisonfield.
#
def expire(char, arguments):
	if char.dead:
		char.deltag('in_poisonfield')
		return

	items = wolfpack.items(char.pos.x, char.pos.y, char.pos.map, 0)

	for item in items:
		if item.hasscript( 'magic.poisonfield' ):
			level = 0
			if item.hastag('level'):
				level = int(item.gettag('level'))

			if char.poison < level:
				char.soundeffect(0x474)
				poison.poison(char, level)

			char.addtimer(350, 'magic.poisonfield.expire', [])
			return

	char.deltag('in_poisonfield')
