
from magic.spell import CharEffectSpell, Spell, DelayedDamageSpell
from magic.utilities import *
import random
import wolfpack
import wolfpack.time
from wolfpack.utilities import tobackpack, energydamage
from wolfpack.consts import *

class BladeSpirits (Spell):
	def __init__(self):
		Spell.__init__(self, 5)
		self.reagents = {REAGENT_BLACKPEARL: 1, REAGENT_MANDRAKE: 1, REAGENT_NIGHTSHADE: 1}
		self.mantra = 'In Jux Hur Ylem'
		self.validtarget = TARGET_GROUND

	def cast(self, char, mode, args=[], target=None, item=None):
		if char.player and char.controlslots >= 5:
			char.socket.clilocmessage(1049645)
			return
		else:
			return Spell.cast(self, char, mode)

	def target(self, char, mode, targettype, target, args, item):
		char.turnto(target)

		if char.player and char.controlslots >= 5:
			char.socket.clilocmessage(1049645)
			return

		if not self.consumerequirements(char, mode, args, target, item):
			return

		spirits = wolfpack.addnpc('summoned_blade_spirit', target)
		spirits.controlslots = 1
		spirits.owner = char
		spirits.summontime = wolfpack.time.servertime() + 120000
		spirits.summoned = 1
		spirits.soundeffect(0x212)

class DispelField (Spell):
	def __init__(self):
		Spell.__init__(self, 5)
		self.reagents = {REAGENT_BLACKPEARL: 1, REAGENT_SPIDERSILK: 1, REAGENT_SULFURASH: 1, REAGENT_GARLIC: 1}
		self.mantra = 'An Grav'
		self.validtarget = TARGET_ITEM

	def target(self, char, mode, targettype, target, args, item):
		char.turnto(target)

		# Dispellable?
		if not target.hastag('dispellable_field'):
			if char.socket:
				char.socket.clilocmessage(1005049)
			return

		if not self.consumerequirements(char, mode, args, target, item):
			return

		wolfpack.effect(0x376a, target.pos, 9, 20)
		target.soundeffect(0x201)
		target.delete()

def incognito_expire(char, arguments):
	if not char.incognito or char.polymorph:
		return

	(oldhair, oldhaircolor, oldfacial, oldfacialcolor) = arguments

	hair = char.itemonlayer(LAYER_HAIR)
	if hair:
		hair.delete()

	if len(oldhair) != 0:
		hair = wolfpack.additem(oldhair)
		hair.color = oldhaircolor
		char.additem(LAYER_HAIR, hair)
		hair.update()

	facial = char.itemonlayer(LAYER_BEARD)
	if facial:
		facial.delete()

	if len(oldfacial) != 0:
		facial = wolfpack.additem(oldfacial)
		facial.color = oldfacialcolor
		char.additem(LAYER_BEARD, facial)
		facial.update()

	char.name = char.orgname
	char.skin = char.orgskin
	char.id = char.orgid
	char.incognito = 0

	char.resendtooltip()
	char.update()

class Incognito (Spell):
	def __init__(self):
		Spell.__init__(self, 5)
		self.reagents = {REAGENT_NIGHTSHADE: 1, REAGENT_BLOODMOSS: 1, REAGENT_GARLIC: 1}
		self.mantra = 'Kal In Ex'

	def cast(self, char, mode, args=[], target=None, item=None):
		if char.incognito or char.polymorph:
			char.socket.clilocmessage(1005559)
			return

		if not self.consumerequirements(char, mode, args, target, item):
			return

		duration = int(1 + (6 * char.skill[MAGERY]) / 50.0) * 1000

		char.orgskin = char.skin
		char.skin = random.randint(1002, 1059)

		char.orgid = char.id
		char.id = random.choice([0x190, 0x191])

		char.orgname = char.name

		newbeard = ''
		newhair = ''
		newhaircolor = random.randint(1102, 1150)

		if char.id == 0x190:
			char.name = random.choice( wolfpack.list( "NAMES_MALE" ) )

			# Create new hair
			newhair = random.choice(['2044', '2045', '2046', '203c', '203b', '203d', '2047', '2048', '2049', '204a', ''])
			newbeard = random.choice(['203e', '203f', '2040', '2041', '204b', '204c', '204d', ''])
		else:
			char.name = random.choice( wolfpack.list( "NAMES_FEMALE" ) )
			newhair = random.choice(['2044', '2045', '2046', '203c', '203b', '203d', '2047', '2048', '2049', '204a', ''])

		hair = char.itemonlayer(LAYER_HAIR)
		if hair:
			hairid = hair.baseid
			haircolor = hair.color
			hair.delete()
		else:
			hairid = ''
			haircolor = 0

		beard = char.itemonlayer(LAYER_BEARD)
		if beard:
			facialid = beard.baseid
			facialcolor = beard.color
			beard.delete()
		else:
			facialid = ''
			facialcolor = 0

		if newhair != '':
			hair = wolfpack.additem(newhair)
			hair.color = newhaircolor
			char.additem(LAYER_HAIR, hair)

		if newbeard != '':
			beard = wolfpack.additem(newbeard)
			beard.color = newhaircolor
			char.additem(LAYER_BEARD, beard)

		hair.update()
		beard.update()

		char.incognito = 1

		char.update()
		char.resendtooltip()
		char.addtimer(duration, "magic.circle5.incognito_expire", [hairid, haircolor, facialid, facialcolor], 1)

		char.effect(0x373A, 10, 15)
		char.soundeffect(0x3bd)

class MagicReflect(Spell):
	def __init__(self):
		Spell.__init__(self, 5)
		self.reagents = {REAGENT_GARLIC: 1, REAGENT_MANDRAKE: 1, REAGENT_SPIDERSILK: 1}
		self.mantra = 'In Jux Sanct'

	def cast(self, char, mode, args=[], target=None, item=None):
		if not self.consumerequirements(char, mode, args, target, item):
			return

		# Toggle Magic Reflection
		if char.propertyflags & 0x40000:
			char.propertyflags &= ~ 0x40000
			char.effect(0x375a, 9, 20)
			char.soundeffect(0x1ed)
		else:
			char.propertyflags |= 0x40000
			char.effect(0x375a, 10, 15)
			char.soundeffect(0x1e9)

		if char.socket:
			char.socket.resendstatus()

class MindBlast (DelayedDamageSpell):
	def __init__(self):
		DelayedDamageSpell.__init__(self, 5)
		self.reagents = {REAGENT_BLACKPEARL: 1, REAGENT_MANDRAKE: 1, REAGENT_NIGHTSHADE: 1, REAGENT_SULFURASH: 1}
		self.mantra = 'Por Corp Wis'
		self.reflectable = 1

	def damage(self, char, target):
		target.effect(0x374a, 10, 15)
		target.soundeffect(0x213)

		damage = min(60, int((char.skill[MAGERY] / 10.0 + char.intelligence) / 5.0))
		damage = random.randint(damage, damage + 4)
		energydamage(target, char, damage, cold=100)

def paralyze_expire(char, arguments):
	if char:
		char.frozen = 0
		char.resendtooltip()

class Paralyze (CharEffectSpell):
	def __init__(self):
		CharEffectSpell.__init__(self, 5)
		self.reagents = {REAGENT_GARLIC: 1, REAGENT_MANDRAKE: 1, REAGENT_SPIDERSILK: 1}
		self.mantra = 'An Ex Por'
		self.harmful = 1
		self.reflectable = 1

	def affectchar(self, char, mode, target, args=[]):
		if target.frozen:
			if char.socket:
				char.socket.clilocmessage(1061923)
			return 0
		return 1

	def effect(self, char, target, mode, args, item):
		target.disturb()
		target.frozen = 1
		target.resendtooltip()
		target.effect(0x376a, 6, 1)
		target.soundeffect(0x204)

		char.checkskill(self.damageskill, 0, 1200)

		# See if we can gain skill
		maxskill = self.circle * 100
		maxskill += (1 + ((self.circle - 1) / 6)) * 250

		if target.skill[MAGICRESISTANCE] < maxskill:
			target.checkskill(MAGICRESISTANCE, 0, 1200)

		duration = int((2 + max(0, (char.skill[self.damageskill] / 100.0 - target.skill[MAGICRESISTANCE] / 100.0))) * 1000)
		if not target.player:
			duration *= 3
		target.addtimer(duration, "magic.circle5.paralyze_expire", [], 1)

class PoisonField(Spell):
	def __init__(self):
		Spell.__init__(self, 5)
		self.reagents = {REAGENT_BLACKPEARL: 1, REAGENT_SPIDERSILK: 1, REAGENT_NIGHTSHADE: 1}
		self.mantra = 'In Nox Grav'
		self.validtarget = TARGET_GROUND

	def target(self, char, mode, targettype, target, args, item):
		char.turnto(target)

		if not self.consumerequirements(char, mode, args, target, item):
			return

		xdiff = abs(target.x - char.pos.x)
		ydiff = abs(target.y - char.pos.y)

		positions = []

		# North/South
		if xdiff > ydiff:
			itemid = 0x3922
			for i in range(-2, 3):
				positions.append(wolfpack.coord(target.x, target.y + i, target.z, target.map))

		# West/East
		else:
			itemid = 0x3915
			for i in range(-2, 3):
				positions.append(wolfpack.coord(target.x + i, target.y, target.z, target.map))

		serials = []
		char.soundeffect(0x20c)

		total = (char.skill[MAGERY] + char.skill[POISONING]) / 2.0
		if total >= 1000:
			level = 3
		elif total > 850:
			level = 2
		elif total > 650:
			level = 1
		else:
			level = 0

		for pos in positions:
			newitem = wolfpack.newitem(1)
			newitem.id = itemid
			#newitem.direction = 29
			newitem.moveto(pos)
			newitem.decay = 0 # Dont decay. TempEffect will take care of them
			newitem.settag('dispellable_field', 1)
			newitem.settag('level', level)
			newitem.addscript( 'magic.poisonfield' )
			newitem.update()
			serials.append(newitem.serial)

			# Affect chars who are occupying the field cells
			chars = wolfpack.chars( newitem.pos.x, newitem.pos.y, newitem.pos.map, 0 )
			for affected in chars:
				if affected.pos.z >= newitem.pos.z - 10 and affected.pos.z <= newitem.pos.z + 10:
					newitem.callevent(EVENT_COLLIDE, (affected, newitem))

		duration = int((3 + char.skill[MAGERY] / 25.0) * 1000)
		wolfpack.addtimer(duration, "magic.utilities.field_expire", serials, 1)

class SummonCreature (Spell):
	def __init__(self):
		Spell.__init__(self, 5)
		self.reagents = {REAGENT_BLOODMOSS: 1, REAGENT_MANDRAKE: 1, REAGENT_SPIDERSILK: 1}
		self.mantra = 'Kal Xen'
		self.validtarget = TARGET_GROUND

	def cast(self, char, mode, args=[], target=None, item=None):
		if char.player and char.controlslots + 1 > 5:
			if char.socket:
				char.socket.clilocmessage(1049645)
			return
		return Spell.cast(self, char, mode)

	def target(self, char, mode, targettype, target, args, item):
		char.turnto(target)

		if char.player and char.controlslots + 1 > 5:
			if char.socket:
				char.socket.clilocmessage(1049645)
			return

		if not target.validspawnspot():
			if char.socket:
				char.socket.clilocmessage(501942)
			return

		if not self.consumerequirements(char, mode, args, target, item):
			return

		npcid = random.choice( ['polar_bear', 'grizzly_bear', 'black_bear',
				'brown_bear', 'horse_1', 'walrus', 'great_hart',
				'hind', 'dog', 'boar', 'chicken', 'rabbit'] )

		creature = wolfpack.addnpc(npcid, target)
		# If the creature is out of our control, delete it.
		if creature.controlslots + char.controlslots > 5:
			creature.delete()
			char.socket.clilocmessage(1049645)
		else:
			creature.tamed = 1
			creature.addscript('speech.pets')
			creature.owner = char
			creature.summontime = wolfpack.time.servertime() + int(char.skill[MAGERY] * 400)
			creature.summoned = 1
			creature.ai = "Animal_Domestic"
			creature.soundeffect(0x215)

def onLoad():
	BladeSpirits().register(33)
	DispelField().register(34)
	Incognito().register(35)
	MagicReflect().register(36)
	MindBlast().register(37)
	Paralyze().register(38)
	PoisonField().register(39)
	SummonCreature().register(40)
