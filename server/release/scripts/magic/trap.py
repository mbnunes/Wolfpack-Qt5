
#
# This event should be assigned to trapped containers
# As soon as they are opened, the trap sets off.
#

import wolfpack
from wolfpack.consts import *

def onUse( char, item ):
	try:
		type = item.gettag( 'trap_type' )
		damage = item.gettag( 'trap_damage' )
		owner = item.gettag( 'trap_owner' )
	except:
		item.events.remove( 'magic.trap' )
		return 0

	char.message( 502999 )
	
	pos = item.pos		
	wolfpack.effect( 0x36bd, wolfpack.coord( pos.x + 1, pos.y, pos.z, pos.map ), 15, 15 )
	wolfpack.effect( 0x36bd, wolfpack.coord( pos.x, pos.y - 1, pos.z, pos.map ), 15, 15 )
	wolfpack.effect( 0x36bd, wolfpack.coord( pos.x - 1, pos.y, pos.z, pos.map ), 15, 15 )
	wolfpack.effect( 0x36bd, wolfpack.coord( pos.x, pos.y + 1, pos.z, pos.map ), 15, 15 )
	item.soundeffect( 0x307 )
	
	events = item.events
	events.remove( 'magic.trap' )
	item.events = events
	
	# Now Damage the Character
	source = wolfpack.findchar( owner )
	char.damage( DAMAGE_MAGICAL, damage, source )
	
	return 1
