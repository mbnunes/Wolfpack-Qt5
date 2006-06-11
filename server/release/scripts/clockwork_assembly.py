import wolfpack
from wolfpack import tr
import wolfpack.utilities
from wolfpack.consts import *
import random
from math import floor

def onUse(char, item):
	backpack = char.getbackpack()
	if not backpack:
		return False
	if not item.getoutmostitem() == backpack:
		char.socket.clilocmessage( 1042001 ) # That must be in your pack for you to use it.
		return True
	TinkerSkill = char.skill[TINKERING] / 10.0
	if TinkerSkill < 60.0:
		char.socket.sysmessage( tr("You must have at least 60.0 skill in tinkering to construct a golem.") )
		return True
	elif len(char.followers) + 4 > char.maxcontrolslots:
		char.socket.clilocmessage( 1049607 ) # You have too many followers to control that creature.
		return True

	scalar = 0
	if TinkerSkill >= 100.0:
		scalar = 1.0
	elif TinkerSkill >= 90.0:
		scalar = 0.9
	elif TinkerSkill >= 80.0:
		scalar = 0.8
	elif TinkerSkill >= 70.0:
		scalar = 0.7
	else:
		scalar = 0.6

	if wolfpack.utilities.checkresources(backpack, 'power_crystal', 1): # Power Crystal
		char.socket.sysmessage( "You must have a power crystal to construct the golem." )
		return True
	elif wolfpack.utilities.checkresources(backpack, 'iron_ingot', 50): # Iron Ingot
		char.socket.sysmessage( "You must have 50 iron ingots to construct the golem." )
		return True
	elif wolfpack.utilities.checkresources(backpack, 'bronze_ingot', 50): # Bronze Ingot
		char.socket.sysmessage( "You must have 50 bronze ingots to construct the golem." )
		return True
	elif wolfpack.utilities.checkresources(backpack, '1053', 5): # Gears
		char.socket.sysmessage( "You must have 5 gears to construct the golem." )
		return True

	wolfpack.utilities.consumeresources(backpack, 'power_crystal', 1)
	wolfpack.utilities.consumeresources(backpack, 'iron_ingot', 50)
	wolfpack.utilities.consumeresources(backpack, 'bronze_ingot', 50)
	wolfpack.utilities.consumeresources(backpack, '1053', 5)

	golem = wolfpack.addnpc('tinkered_golem', char.pos)
	golem.owner = char
	golem.tamed = True
	golem.follow(char)
	golem.update()

	# Stats
	golem.strength = random.randint(251, 350) * scalar
	golem.dexterity = random.randint(76, 100) * scalar
	golem.intelligence = random.randint(101, 150) * scalar
	golem.hitpoints = random.randint(151, 210) * scalar

	# Damage
	golem.settag('mindamage', 13 * scalar)
	golem.settag('maxdamage', 24 * scalar)

	# Resistances
	golem.settag('res_physical', random.randint(35, 55) * scalar)
	golem.settag('res_fire', 100 * scalar)
	golem.settag('res_cold', random.randint(10, 30) * scalar)
	golem.settag('res_poison', random.randint(10, 25) * scalar)
	golem.settag('res_energy', random.randint(30, 40) * scalar)

	# Skills
	magicres = int(random.randint(1501, 1900) * scalar)
	tactic = int(random.randint(601, 1000) * scalar)
	wrest = int(random.randint(601, 1000) * scalar)
	
	golem.skill[MAGICRESISTANCE] = magicres
	golem.skill[TACTICS] = tactic
	golem.skill[WRESTLING] = wrest

	char.soundeffect( 0x241 )
	return True
