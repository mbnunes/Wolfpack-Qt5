
from wolfpack.consts import *

# ReactiveArmor EventSet
# This is basically a helper to override onDamage 
# probably throw back damage
def onDamage( char, type, damage, source ):
	# No source ?
	# Resistance is futile
	if not char.hastag( 'damage_absorb' ):
		events = char.events
		events.remove( 'magic.reactivearmor' )
		char.events = events
	
	if type == DAMAGE_PHYSICAL and source and source.distanceto( char ) <= 1:
		absorb = int( char.gettag( 'damage_absorb' ) )

		reflect = max( 1, int( damage / 5 ) )
		
		source.damage( DAMAGE_MAGICAL, reflect, char )
		source.soundeffect( 0x1f1 )
		source.effect( 0x374a, 10, 16 )
	
		if damage < absorb:
			char.settag( 'damage_absorb', absorb - damage )
			damage = 0			
		else:
			damage -= absorb

			char.message( 1005556 )
			char.deltag( 'damage_absorb' )
			
			events = char.events
			events.remove( 'magic.reactivearmor' )
			char.events = events
	
	return damage
