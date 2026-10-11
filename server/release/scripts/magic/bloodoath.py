
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

def dispel(char, args, source, dispelargs):
	expire(char, dispelargs)

def expire(char, args):
	victim = wolfpack.findchar(args[0])
	if char:
		if char.hasscript('magic.bloodoath'):
			char.removescript('magic.bloodoath')
		if char.hastag('bloodoath_caster'):
			char.deltag('bloodoath_caster')	
		if char.socket:
			char.socket.clilocmessage( 1061620 ) # Your Blood Oath has been broken.
	if victim:
		if victim.hasscript('magic.bloodoath'):
			victim.removescript('magic.bloodoath')
		if victim.hastag('bloodoath'):
			victim.deltag('bloodoath')
		if victim.socket:
			victim.socket.clilocmessage( 1061620 ) # Your Blood Oath has been broken.
	return
