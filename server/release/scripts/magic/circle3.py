
from magic import registerspell
from wolfpack.consts import *
from magic.spell import CharEffectSpell, Spell, DelayedDamageSpell
from magic.utilities import *
import random
import wolfpack
from wolfpack.utilities import tobackpack, energydamage, smokepuff
from system import poison

class Bless (CharEffectSpell):
	def __init__(self):
		CharEffectSpell.__init__(self, 3)
		self.reagents = {REAGENT_MANDRAKE: 1, REAGENT_GARLIC: 1}
		self.mantra = 'Rel Sanct'

	def effect(self, char, target, mode, args, item):
		statmodifier(char, target, 3, 0)

		target.effect(0x373a, 10, 15)
		target.soundeffect(0x1ea)

class Fireball (DelayedDamageSpell):
	def __init__(self):
		DelayedDamageSpell.__init__(self, 3)
		self.reagents = {REAGENT_BLACKPEARL: 1}
		self.mantra = 'Vas Flam'
		self.sound = 0x44b
		self.missile = [ 0x36D4, 0, 1, 7, 0 ]
		self.reflectable = 1

	def damage(self, char, target):
		damage = self.scaledamage(char, target, 19, 1, 5)
		energydamage(target, char, damage, fire=100)

class WallOfStone(Spell):
	def __init__(self):
		Spell.__init__(self, 3)
		self.reagents = {REAGENT_BLOODMOSS: 1, REAGENT_GARLIC: 1}
		self.mantra = 'In Sanct Ylem'
		self.validtarget = TARGET_GROUND

	def target(self, char, mode, targettype, target, args, item):
		char.turnto(target)

		if not self.consumerequirements(char, mode, args, target, item):
			return

		xdiff = abs(target.x - char.pos.x)
		ydiff = abs(target.y - char.pos.y)

		if xdiff > ydiff:
			positions = [ wolfpack.coord(target.x, target.y - 1, target.z, target.map), wolfpack.coord(target.x, target.y, target.z, target.map), wolfpack.coord(target.x, target.y + 1, target.z, target.map) ]
		else:
			positions = [ wolfpack.coord(target.x - 1, target.y, target.z, target.map), wolfpack.coord(target.x, target.y, target.z, target.map), wolfpack.coord(target.x + 1, target.y, target.z, target.map) ]

		# At least one spell should look fancy
		# Calculate the Angle here
		serials = []

		char.soundeffect(0x1f6)

		for pos in positions:
			newitem = wolfpack.newitem(1)
			newitem.id = 0x80
			newitem.moveto(pos)
			newitem.decay = 0 # Dont decay. TempEffect will take care of them
			newitem.update()
			newitem.settag('dispellable_field', 1)
			serials.append(newitem.serial)
			wolfpack.effect(0x376a, pos, 9, 10)

		wolfpack.addtimer(10000, "magic.utilities.field_expire", serials, 1)

class Teleport(Spell):
	def __init__(self):
		Spell.__init__(self, 3)
		self.reagents = {REAGENT_BLOODMOSS: 1, REAGENT_MANDRAKE: 1}
		self.mantra = 'Rel Por'
		self.validtarget = TARGET_GROUND
		self.range = 12

	def target(self, char, mode, targettype, target, args, item):
		char.turnto(target)

		# Check if the target tile is blocked or in a multi
		# target.validspawnspot() will automatically set the z
		# of the coord to the nearest top
		if (not target.validspawnspot() or wolfpack.findmulti(target)) and not char.gm:		
			if char.socket:
				char.socket.clilocmessage(501942)
			return

		# Line of Sight (touch!! or else we can teleport trough windows)
		if not char.canreach(target, 12):
			if char.socket:
				char.socket.clilocmessage(500237)
			return

		if not self.consumerequirements(char, mode, args, target, item):
			return

		source = char.pos

		char.removefromview()
		char.moveto(target)
		char.update()
		if char.socket:
			char.socket.resendworld()

		smokepuff(char, source)
		smokepuff(char, target)

class MagicLock(Spell):
	def __init__(self):
		Spell.__init__(self, 3)
		self.reagents = {REAGENT_BLOODMOSS: 1, REAGENT_SULFURASH: 1, REAGENT_GARLIC: 1}
		self.mantra = 'An Por'
		self.validtarget = TARGET_ITEM

	def target(self, char, mode, targettype, target, args, item):
		char.turnto(target)

		# We can only lock unlocked chests
		if target.type != 1 or target.hasscript( 'lock' ) or target.container:
			char.message(501762)
			return

		# We cannot use this on locked down chests
		if target.secured:
			char.message(501761)
			return

		if not self.consumerequirements(char, mode, args, target, item):
			return

		target.addscript( 'lock' )
		target.settag('lock', 'magic')

		char.message(501763)
		wolfpack.effect(0x376a, target.pos, 9, 32)
		target.soundeffect(0x1fa)

class Unlock(Spell):
	def __init__(self):
		Spell.__init__(self, 3)
		self.reagents = {REAGENT_BLOODMOSS: 1, REAGENT_SULFURASH: 1}
		self.mantra = 'Ex Por'
		self.validtarget = TARGET_ITEM

	def target(self, char, mode, targettype, target, args, item):
		char.turnto(target)

		# We can only lock unlocked chests
		if not target.hasscript( 'lock' ):
			char.message(503101)
			return

		if target.gettag('lock') != 'magic':
			char.message(503099)
			return

		if not self.consumerequirements(char, mode, args, target, item):
			return

		target.removescript( 'lock' )
		target.deltag('lock', 'magic')

		wolfpack.effect(0x376a, target.pos, 9, 32)
		target.soundeffect(0x1ff)

class Telekinesis(Spell):
	def __init__(self):
		Spell.__init__(self, 3)
		self.reagents = {REAGENT_BLOODMOSS: 1, REAGENT_MANDRAKE: 1}
		self.mantra = 'Ort Por Ylem'
		self.validtarget = TARGET_ITEM

	def target(self, char, mode, targettype, target, args, item):
		char.turnto(target)

		hasevent = 0

		# Check if there is an event handling onUse but not onTelekinesis
		scripts = item.scripts + item.basescripts.split(',')
		for event in scripts:
			if len(event) == 0:
				continue
	
			if wolfpack.hasevent(event, EVENT_TELEKINESIS):
				hasevent = 1 # The object has at least one telekinesis handler
				continue

			if wolfpack.hasevent(event, EVENT_USE):
				if char.socket:
					char.socket.clilocmessage(501857)
				return

		# If the object has no handlers
		# And it's not a container, exit
		if not hasevent and target.type != 1:
			return

		if not self.consumerequirements(char, mode, args, target, item):
			return

		wolfpack.effect(0x376a, target.pos, 9, 32)
		target.soundeffect(0x1f5)

		result = 0

		for event in scripts:
			# If the event can handle onTelekinesis, call it
			if wolfpack.hasevent(event, EVENT_TELEKINESIS):
				if wolfpack.callevent(event, EVENT_TELEKINESIS, (char, target)):
					return

		if target.type == 1:
			if char.socket:
				char.socket.sendcontainer(target)
			return

		char.socket.clilocmessage(501857)
		fizzle(char)

class Poison (CharEffectSpell):
	def __init__(self):
		CharEffectSpell.__init__(self, 3)
		self.reagents = {REAGENT_NIGHTSHADE: 1}
		self.mantra = 'In Nox'
		self.harmful = 1
		self.reflectable = 1

	def effect(self, char, target, mode, args, item):
		target.disturb()

		if self.checkresist(char, target):
			if target.socket:
				target.socket.clilocmessage(501783)
		else:
			total = char.skill[MAGERY] + char.skill[POISONING]
			distance = char.distanceto(target)
			if distance >= 3:
				total -= (distance - 3) * 10.0

			level = 0

			if total >= 2000:
				level = 3
			elif total > 1701:
				level = 2
			elif total > 1301:
				level = 1

			poison.poison(target, level)

		target.effect(0x374A, 10, 15)
		target.soundeffect(0x474)

def onLoad():
	Bless().register(17)
	Fireball().register(18)
	MagicLock().register(19)
	Poison().register(20)
	Telekinesis().register(21)
	Teleport().register(22)
	Unlock().register(23)
	WallOfStone().register(24)
