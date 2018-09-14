import magic
from magic.utilities import REAGENT_GARLIC
from wolfpack.consts import DAMAGE_MAGICAL

def onDoDamage(char, type, amount, victim):
	damagedone = amount / 5 # Leech 20% Health
	if damagedone > 0:
		char.hitpoints = min(char.maxhitpoints, char.hitpoints + damagedone)
		char.updatehealth()
	return amount

def onCastSpell(char, spell):
	if REAGENT_GARLIC in magic.spells[spell].reagents:
		char.socket.clilocmessage( 1061651 ) # The garlic burns you!
		damage = random.randint(17, 23)
		char.damage( DAMAGE_MAGICAL, damage, char)

def onRegenMana(char, points):
	points += 3
	return points

def onRegenStamina(char, points):
	points += 15
	return points
