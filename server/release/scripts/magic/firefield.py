
import wolfpack
from wolfpack.consts import *

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
		char.damage(DAMAGE_MAGICAL, 5, source)
		char.addtimer(750, 'magic.firefield.expire', [])
	return 0

#
# See if we're still in a firefield.
#
def expire(char, arguments):	
	items = wolfpack.items(char.pos.x, char.pos.y, char.pos.map, 0)

	for item in items:	
		if 'magic.firefield' in item.events:
			source = None
			if item.hastag('source'):
				source = wolfpack.findchar(int(item.gettag('source')))
		
			char.damage(DAMAGE_MAGICAL, 5, source)
			char.addtimer(750, 'magic.firefield.expire', [])
			return

	char.deltag('in_firefield')
