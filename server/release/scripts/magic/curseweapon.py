
# Necromancer: Curse Weapon

def onEquip(char, item, layer):
	char.addscript("magic.curseweapon")

def onUnequip(char, item, layer):
	char.removescript("magic.curseweapon")

def expire(item, args):
	if item.hastag('cursed'):
		item.deltag('cursed')
	item.removescript("magic.curseweapon")
	item.getoutmostchar().removescript("magic.curseweapon")

def onDoDamage(char, type, amount, victim):
	char.hitpoints += amount/2
	if char.hitpoints > char.maxhitpoints:
		char.hitpoints = char.maxhitpoints
	char.updatehealth()
	return amount
