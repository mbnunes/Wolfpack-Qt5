
from magic.spell import CharEffectSpell, Spell, DelayedDamageSpell
from magic.utilities import *
import random
import wolfpack
from wolfpack.utilities import tobackpack, energydamage
from system import poison

class Curse (CharEffectSpell):
	def __init__(self):
		CharEffectSpell.__init__(self, 4)
		self.reagents = {REAGENT_NIGHTSHADE: 1, REAGENT_GARLIC: 1, REAGENT_SULFURASH: 1}
		self.mantra = 'Des Sanct'
		self.harmful = 1
		self.reflectable = 1

	def effect(self, char, target, mode, args, item):
		statmodifier(char, target, 3, 1)
		target.effect(0x373a, 10, 15)
		target.soundeffect(0x1ea)

class GreaterHeal (CharEffectSpell):
	def __init__(self):
		CharEffectSpell.__init__(self, 4)
		self.reagents = {REAGENT_GARLIC: 1, REAGENT_GINSENG: 1, REAGENT_SPIDERSILK: 1}
		self.mantra = 'In Vas Mani'

	def affectchar(self, char, mode, target, args=[]):
		if target.poison != -1:
			if target == char:
				char.message(1005000)
			else:
				char.message(1010398)
			return 0
		return 1

	def effect(self, char, target, mode, args, item):
		# 40% of Magery + 1-10
		amount = int(0.04 * char.skill[ MAGERY ]) + random.randint(1, 10)
		target.hitpoints = min(target.maxhitpoints, target.hitpoints + amount)
		target.updatehealth()

		target.effect(0x376A, 9, 32)
		target.soundeffect(0x202)

class Lightning (DelayedDamageSpell):
	def __init__(self):
		DelayedDamageSpell.__init__(self, 4)
		self.reagents = {REAGENT_SULFURASH: 1, REAGENT_MANDRAKE: 1}
		self.mantra = 'Por Ort Grav'
		self.delay = None
		self.reflectable = 1

	def damage(self, char, target):
		target.lightning()
		target.soundeffect(0x29)

		damage = self.scaledamage(char, target, 22, 1, 4)
		energydamage(target, char, damage, energy=100)

def manadrain_restore(char, arguments):
	if not char.dead:
		char.mana = min(char.maxmana, arguments[0])
		char.propertyflags &= ~ 0x100000
		char.updatemana()
		char.effect(0x3779, 10, 25)
		char.soundeffect(0x28e)

class ManaDrain (DelayedDamageSpell):
	def __init__(self):
		DelayedDamageSpell.__init__(self, 4)
		self.reagents = {REAGENT_SULFURASH: 1, REAGENT_MANDRAKE: 1}
		self.mantra = 'Ort Rel'
		self.delay = None
		self.reflectable = 1

	def damage(self, char, target):
		char.checkskill(self.damageskill, 0, 1200)

		# See if we can gain skill
		maxskill = self.circle * 100
		maxskill += (1 + ((self.circle - 1) / 6)) * 250

		if target.skill[MAGICRESISTANCE] < maxskill:
			target.checkskill(MAGICRESISTANCE, 0, 1200)

		# If the target is already under effect of the spell, do nothing.
		if target.propertyflags & 0x100000:
			amount = 0
		else:
			amount = min(target.mana, int(40 + (char.skill[self.damageskill] - target.skill[MAGICRESISTANCE]) / 10))

		target.effect(0x3789, 10, 25)
		target.soundeffect(0x1F8)

		# Drain and establish an effect to restore the drain
		if amount > 0:
			target.mana -= amount
			target.updatemana()
			target.propertyflags |= 0x100000
			target.addtimer(6000, "magic.circle4.manadrain_restore", [amount], 1)

class Recall (Spell):
	def __init__(self):
		Spell.__init__(self, 4)
		self.reagents = {REAGENT_BLOODMOSS: 1, REAGENT_SULFURASH: 1, REAGENT_GARLIC: 1}
		self.mantra = 'Kal Ort Por'
		self.validtarget = TARGET_ITEM
		self.casttime = 2250

	def cast(self, char, mode, args=[], target=None, item=None):
		if not char.gm:
			if char.iscriminal():
				if char.socket:
					char.socket.clilocmessage(1005561)
				return

			if char.attacktarget:
				if char.socket:
					char.socket.clilocmessage(1005564)
				return

		return Spell.cast(self, char, mode)

	def target(self, char, mode, targettype, target, args, item):
		char.turnto(target)

		# We can only recall from recall runes
		if not 'magic.rune' in target.events:
			char.message(502357)
			return

		if not self.consumerequirements(char, mode, args, target, item):
			return

		if not target.hastag('marked') or target.gettag('marked') != 1:
			char.message(502354)
			fizzle(char)
			return

		region = None
		region = wolfpack.region(char.pos.x, char.pos.y, char.pos.map)

		if region and region.norecallout:
			char.message(501802)
			fizzle(char)
			return

		location = target.gettag('location')
		location = location.split(",")
		location = wolfpack.coord(int(location[0]), int(location[1]), int(location[2]), int(location[3]))

		region = None
		region = wolfpack.region(location.x, location.y, location.map)

		if not location.validspawnspot():
			char.message(501942)
			fizzle(char)
			return

		if region and region.norecallin:
			char.message(1019004)
			fizzle(char)
			return

		char.soundeffect(0x1fc)
		char.removefromview()
		char.moveto(location)
		char.update()
		char.socket.resendworld()
		char.soundeffect(0x1fc)

class FireField(Spell):
	def __init__(self):
		Spell.__init__(self, 4)
		self.reagents = {REAGENT_BLACKPEARL: 1, REAGENT_SPIDERSILK: 1, REAGENT_SULFURASH: 1}
		self.mantra = 'In Flam Grav'
		self.validtarget = TARGET_GROUND
		self.harmful = 1
		self.resistable = 1

	def target(self, char, mode, targettype, target, args, item):
		char.turnto(target)

		if not char.canreach(target, 10):
			char.message(500237)
			return

		if not self.consumerequirements(char, mode, args, target, item):
			return

		xdiff = abs(target.x - char.pos.x)
		ydiff = abs(target.y - char.pos.y)

		positions = []

		# West / East
		if xdiff > ydiff:
			itemid = 0x3996
			for i in range(-2, 3):
				positions.append(wolfpack.coord(target.x, target.y + i, target.z, target.map))

		# North South
		else:
			itemid = 0x398c
			for i in range(-2, 3):
				positions.append(wolfpack.coord(target.x + i, target.y, target.z, target.map))

		serials = []

		char.soundeffect(0x20c)

		for pos in positions:
			newitem = wolfpack.newitem(1)
			newitem.id = itemid
			#newitem.direction = 29
			newitem.moveto(pos)
			newitem.decay = 0 # Dont decay. TempEffect will take care of them
			newitem.settag('dispellable_field', 1)
			newitem.settag('source', char.serial)
			newitem.events = ['magic.firefield']
			newitem.update()
			serials.append(newitem.serial)

			# Affect chars who are occupying the field cells
			chars = wolfpack.chars(newitem.pos.x, newitem.pos.y, newitem.pos.map)
			if len(chars) > 0 :
				for affected in chars:
					if affected.pos.z >= newitem.pos.z - 10 and affected.pos.z <= newitem.pos.z + 10:
						newitem.callevent(EVENT_COLLIDE, (affected, newitem))

		duration = int((4 + char.skill[ MAGERY ] * 0.05) * 1000)
		wolfpack.addtimer(duration, "magic.utilities.field_expire", serials, 1)

class ArchCure (Spell):
	def __init__(self):
		Spell.__init__(self, 4)
		self.reagents = {REAGENT_GARLIC: 1, REAGENT_GINSENG: 1, REAGENT_MANDRAKE: 1}
		self.mantra = 'Vas An Nox'
		self.validtarget = TARGET_GROUND

	def target(self, char, mode, targettype, target, args, item):
		if not self.consumerequirements(char, mode, args, target, item):
			return

		char.turnto(target)
		char.soundeffect(0x299)
		cured = 0

		# Enumerate chars
		chars = wolfpack.chars(target.x, target.y, char.pos.map, 3)
		for target in chars:
			target.effect(0x373a, 10, 15)
			target.soundeffect(0x1e0)

			if target.poison != -1:
				chance = (10000 + int(char.skill[MAGERY] * 7.5) - ((target.poison + 1) * 1750)) / 10000.0
				if chance >= random.random():
					poison.cure(target)
					cured += 1
					if target.socket:
						target.socket.clilocmessage(1010059)

		if cured > 0:
			if char.socket:
				char.socket.clilocmessage(1010058)

class ArchProtection (Spell):
	def __init__(self):
		Spell.__init__(self, 4)
		self.reagents = {REAGENT_GARLIC: 1, REAGENT_GINSENG: 1, REAGENT_MANDRAKE: 1, REAGENT_SULFURASH: 1}
		self.mantra = 'Vas Uus Sanct'
		self.validtarget = TARGET_GROUND

	def target(self, char, mode, targettype, target, args, item):
		if not self.consumerequirements(char, mode, args, target, item):
			return

		char.turnto(target)
		char.soundeffect(0x299)
		cured = 0

		if char.player:
			party = char.party
			guild = char.guild
		else:
			party = None
			guild = None

		# Enumerate chars
		chars = wolfpack.chars(target.x, target.y, char.pos.map, 3)
		for target in chars:
			if not target.player:
				continue

			if target == char or (guild and target.guild == guild) or (party and target.party == party):
				# Toggle Protection
				if target.propertyflags & 0x20000:
					target.propertyflags &= ~ 0x20000
					target.effect(0x375a, 9, 20)
					target.soundeffect(0x1ed)

					try:
						if target.hastag('protection_skill'):
							skill = int(target.gettag('protection_skill'))
							if target.skill[MAGICRESISTANCE] + skill > target.skillcap[MAGICRESISTANCE]:
								target.skill[MAGICRESISTANCE] = target.skillcap[MAGICRESISTANCE]
							else:
								target.skill[MAGICRESISTANCE] += skill
					except:
						pass
					target.deltag('protection_skill')
				else:
					target.propertyflags |= 0x20000
					target.effect(0x375a, 9, 20)
					target.soundeffect(0x1e9)

					# Remove Magic Resistance Skill
					amount = 35 - int(target.skill[INSCRIPTION] / 200)
					if target.skill[MAGICRESISTANCE] - amount < 0:
						amount = target.skill[MAGICRESISTANCE]

					target.settag('protection_skill', amount)
					target.skill[MAGICRESISTANCE] -= amount

				if target.socket:
					target.socket.resendstatus()

def onLoad():
	ArchCure().register(25)
	ArchProtection().register(26)
	Curse().register(27)
	FireField().register(28)
	GreaterHeal().register(29)
	Lightning().register(30)
	ManaDrain().register(31)
	Recall().register(32)
