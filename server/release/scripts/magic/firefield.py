
import wolfpack
from wolfpack.utilities import energydamage

#
# Attach a tag to the character to note that he entered a firefield.
#
def onCollide(char, item):
	if char.dead:
		return

	if not char.hastag('in_firefield'):
		source = None
		if item.hastag('source'):
			source = wolfpack.findchar(int(item.gettag('source')))

		char.settag('in_firefield', 1)
		energydamage(char, source, 2, fire=100)
		char.addtimer(350, 'magic.firefield.expire', [])
	return 0

#
# See if we're still in a firefield.
#
def expire(char, arguments):
	items = wolfpack.items(char.pos.x, char.pos.y, char.pos.map, 0)

	for item in items:
		if item.hasevent( 'magic.firefield' ):
			source = None
			if item.hastag('source'):
				source = wolfpack.findchar(int(item.gettag('source')))

			energydamage(char, source, 2, fire=100)
			char.addtimer(350, 'magic.firefield.expire', [])
			char.soundeffect(0x208)
			return

	char.deltag('in_firefield')
