
from wolfpack.consts import *
from wolfpack.utilities import energydamage
import random
from wolfpack import properties
import wolfpack

#
# This file contains several hiteffects used by the combat code
#
def scaledamage(attacker, mindamage, maxdamage, evintscale):
	damage = random.randint(mindamage, maxdamage)
	
	# This is using a different algorithm than usual
	damage += int(attacker.skill[EVALUATINGINTEL] * evintscale)
	
	bonus = attacker.skill[INSCRIPTION] / 100.0
	if attacker.player:
		bonus += attacker.intelligence / 10.0
		bonus += properties.fromchar(attacker, SPELLDAMAGEBONUS)

	damage += int((damage * bonus) / 100)

	return damage

def hitmagicarrow(attacker, defender):
	if defender.invulnerable or defender.dead or defender.invisible or defender.hidden:
		return

	attacker.soundeffect(0x1e5)
	attacker.movingeffect(0x36e4, defender, False, True, 5)

	damage = scaledamage(attacker, 3, 4, 0.01)
	defender.addtimer(1000, 'combat.hiteffects.damage_callback', [attacker.serial, damage])

def hitharm(attacker, defender):
	if defender.invulnerable or defender.dead or defender.invisible or defender.hidden:
		return

	damage = scaledamage(attacker, 6, 9, 0.015)
	distance = defender.distanceto(attacker)
	
	if distance > 2:
		damage /= 4
	elif distance > 1:
		damage /= 2

	defender.effect(0x374a, 10, 30)
	defender.soundeffect(0xfc)

	energydamage(defender, attacker, damage, 0, 0, 100, 0, 0, 0, DAMAGE_MAGICAL)

def hitfireball(attacker, defender):
	if defender.invulnerable or defender.dead or defender.invisible or defender.hidden:
		return

	attacker.soundeffect(0x15e)
	attacker.movingeffect(0x36d4, defender, False, True, 7)

	damage = scaledamage(attacker, 6, 9, 0.018)
	defender.addtimer(1000, 'combat.hiteffects.damage_callback', [attacker.serial, damage])

def damage_callback(defender, args):
	(attacker, damage) = (wolfpack.findchar(args[0]), args[1])
	if attacker and defender and not defender.invulnerable and not defender.dead and not defender.invisible and not defender.hidden:
		energydamage(defender, attacker, damage, 0, 100, 0, 0, 0, 0, DAMAGE_MAGICAL)

def hitlightning(attacker, defender):
	if defender.invulnerable or defender.dead or defender.invisible or defender.hidden:
		return

	defender.lightning()
	energydamage(defender, attacker, scaledamage(attacker, 6, 9, 0.02), 0, 0, 0, 0, 100, 0, DAMAGE_MAGICAL)

def hitdispel(attacker, defender):
	if not defender.npc or not defender.summoned:
		return
	
	dispelChance = 50.0 + (100 * ((attacker.magery / 10.0 - attacker.getintproperty('dispeldifficulty', 0)) / (attacker.getintproperty('dispelfocus', 1) * 2))) / 100.0
	
	if dispelChance > random.randint(0, 99):
		defender.pos.effect(0x3728, 8, 20)
		defender.soundeffect(0x201)
		defender.delete()
	else:
		defender.effect(0x3779, 10, 20)
		if attacker.socket:
			attacker.socket.clilocmessage(1010084) # The creature resisted the attempt to dispel it!

EFFECTS = {
	HITMAGICARROW: hitmagicarrow, 
	HITHARM: hitharm, 
	HITFIREBALL: hitfireball, 
	HITLIGHTNING: hitlightning, 
	HITDISPEL: hitdispel
}
