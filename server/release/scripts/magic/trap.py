
#
# This event should be assigned to trapped containers
# As soon as they are opened, the trap sets off.
#

import wolfpack
from wolfpack.utilities import energydamage

def onUse(char, item):
	try:
		type = item.gettag( 'trap_type' )
		damage = item.gettag( 'trap_damage' )
		owner = wolfpack.findchar(int(item.gettag('trap_owner')))
	except:
		item.removescript( 'magic.trap' )
		item.deltag('trap_type')
		item.deltag('trap_damage')
		item.deltag('trap_owner')
		return 0

	item.removescript( 'magic.trap' )
	item.deltag('trap_type')
	item.deltag('trap_damage')
	item.deltag('trap_owner')

	char.message(502999)

	if not item.container:
		pos = item.pos
	else:
		item = item.getoutmostitem()
		if item.container:
			item = item.container
	pos = item.pos

	wolfpack.effect( 0x36bd, wolfpack.coord( pos.x + 1, pos.y, pos.z, pos.map ), 15, 15 )
	wolfpack.effect( 0x36bd, wolfpack.coord( pos.x, pos.y - 1, pos.z, pos.map ), 15, 15 )
	wolfpack.effect( 0x36bd, wolfpack.coord( pos.x - 1, pos.y, pos.z, pos.map ), 15, 15 )
	wolfpack.effect( 0x36bd, wolfpack.coord( pos.x, pos.y + 1, pos.z, pos.map ), 15, 15 )
	char.soundeffect(0x307)

	# Now Damage the Character
	if char.distanceto(item) <= 2:
		energydamage(char, owner, damage, fire=100)

	return 1

def onTelekinesis(char, item):
	return onUse(char, item)
