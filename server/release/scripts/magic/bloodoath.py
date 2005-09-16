
from wolfpack.consts import *
import wolfpack

def onDoDamage(char, type, amount, victim):
	# if victim is caster, increase damage
	if char.hastag('bloodoath') and char.gettag('bloodoath') == victim.serial:
		amount = (int(amount * 1.1))
		char.damage( DAMAGE_PHYSICAL, amount, victim )
	return amount

# disbound the "dark pact" on death
def onDeath(char, source, corpse):
	if char.hastag('bloodoath_caster'):
		char.dispel(None, True, 'BLOODOATH', [char.gettag('bloodoath_caster')])
	else:
		char = wolfpack.findchar(char.gettag('bloodoath'))
		char.dispel(None, True, 'BLOODOATH', [char.gettag('bloodoath_caster')])
