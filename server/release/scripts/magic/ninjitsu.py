#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Created by: Naddel                         #
#   )).-' {{ ;'`   | Revised by:                                #
#  ( (  ;._ \\ ctr | Last Modification: Created                 #
#===============================================================#
# Ninjitsu spells & moves                                       #
#===============================================================#

import math
import random
import wolfpack
import wolfpack.time
from wolfpack.consts import *
from wolfpack import properties, tr, gumps
from magic.spell import Spell
from magic.utilities import MODE_BOOK, MODE_SCROLL, MODE_CMD, MODE_WAND, fizzle, TARGET_GROUND
from wolfpack.utilities import energydamage

# Helper to send ToggleSpecialAbility packet (0xBF 0x25)
def sendToggleSpecialAbility(char, ability_id, active):
	if char and char.socket:
		packet = wolfpack.packet(0xbf, 7)
		packet.setshort(1, 7)
		packet.setshort(3, 0x25)
		packet.setshort(5, int(ability_id))
		packet.setbyte(7, 1 if active else 0)
		packet.send(char.socket)

# Base class for Ninjitsu spells/moves
class NinjaSpell(Spell):
	def __init__(self, spellid, req_skill, req_mana):
		Spell.__init__(self, 0)
		self.spellid = spellid
		self.skill = NINJITSU
		self.requiredskill = req_skill
		self.mana = req_mana
		self.mantra = ''

	def checkweapon(self, char):
		return True

	def calcdelay(self, char, mode):
		return 250

	def onBeginCast(self, char):
		char.effect(0x37C4, 10, 14)

	def checkrequirements(self, char, mode, args=[], target=None, item=None):
		if not Spell.checkrequirements(self, char, mode, args, target, item):
			return False
		if char.skill[NINJITSU] < self.requiredskill:
			if char.socket:
				char.socket.clilocmessage(1063352, str(self.requiredskill / 10.0)) # You need ~1_SKILL_REQUIREMENT~ Ninjitsu skill to perform that attack!
			return False
		scaled_mana = self.scalemana(char, mode, self.mana)
		if char.mana < scaled_mana:
			if char.socket:
				char.socket.clilocmessage(1060174, str(scaled_mana)) # You must have at least ~1_MANA_REQUIREMENT~ Mana to use this ability.
			return False
		return True

# -------------------------------------------------------------
# 501: Focus Attack
# -------------------------------------------------------------
class FocusAttack(NinjaSpell):
	def __init__(self):
		NinjaSpell.__init__(self, 501, 300, 10)

	def checkrequirements(self, char, mode, args=[], target=None, item=None):
		if not NinjaSpell.checkrequirements(self, char, mode, args, target, item):
			return False
		# Must wield melee weapon without shield
		if char.itemonlayer(LAYER_LEFTHAND) and properties.itemcheck(char.itemonlayer(LAYER_LEFTHAND), ITEM_SHIELD):
			if char.socket:
				char.socket.clilocmessage(1063096) # You cannot use this ability while holding a shield.
			return False
		weapon = char.getweapon()
		if not weapon or not properties.itemcheck(weapon, ITEM_MELEE):
			if char.socket:
				char.socket.clilocmessage(1063097) # You must be wielding a melee weapon without a shield to use this ability.
			return False
		return True

	def cast(self, char, mode, args=[], target=None, item=None):
		if char.hastag('focus_attack'):
			clearNinjaMove(char, 'focus_attack', 501)
			return True

		if not self.consumerequirements(char, mode, args, target, item):
			return False

		clearActiveNinjaMoves(char)
		char.settag('focus_attack', 1)
		sendToggleSpecialAbility(char, 501, True)
		if char.socket:
			char.socket.clilocmessage(1063095) # You prepare to focus all of your abilities into your next strike.
		return True

# -------------------------------------------------------------
# 502: Death Strike
# -------------------------------------------------------------
class DeathStrike(NinjaSpell):
	def __init__(self):
		NinjaSpell.__init__(self, 502, 850, 30)

	def cast(self, char, mode, args=[], target=None, item=None):
		if char.hastag('death_strike'):
			clearNinjaMove(char, 'death_strike', 502)
			return True

		if not self.consumerequirements(char, mode, args, target, item):
			return False

		clearActiveNinjaMoves(char)
		char.settag('death_strike', 1)
		sendToggleSpecialAbility(char, 502, True)
		if char.socket:
			char.socket.clilocmessage(1063091) # You prepare to hit your opponent with a Death Strike.
		return True

def deathstrike_process(target, args):
	if not target or not target.hastag('deathstrike_victim'):
		return
	attacker_serial = int(target.gettag('deathstrike_attacker'))
	attacker = wolfpack.findobject(attacker_serial)
	steps = int(target.gettag('deathstrike_steps')) if target.hastag('deathstrike_steps') else 0
	target.deltag('deathstrike_victim')
	target.deltag('deathstrike_attacker')
	target.deltag('deathstrike_steps')

	ninjitsu = 100.0
	if attacker:
		ninjitsu = attacker.skill[NINJITSU] / 10.0

	# ML Formula: 5+ steps = max 60 direct damage, otherwise max 30
	if steps >= 5:
		damage = min(60, int((ninjitsu / 3.0)))
	else:
		damage = min(30, int((ninjitsu / 9.0)))

	damage = max(5, damage)
	energydamage(target, attacker if attacker else target, damage, 100, 0, 0, 0, 0, 0, DAMAGE_PURE)

# -------------------------------------------------------------
# 503: Animal Form
# -------------------------------------------------------------
ANIMAL_FORMS = [
	# (name, req_skill, body, hue)
	("Rabbit", 200, 0xCD, 0),
	("Rat", 200, 0xEE, 0),
	("Dog", 400, 0xD9, 0),
	("Cat", 400, 0xC9, 0),
	("Bullfrog", 500, 0x51, 0),
	("Giant Serpent", 500, 0x15, 0),
	("Llama", 700, 0xDC, 0),
	("Ostard", 700, 0xDB, 0),
	("Wolf", 825, 0x19, 0),
	("Bake-Kitsune", 825, 0xF6, 0),
	("Kirin", 1000, 0x84, 0),
	("Unicorn", 1000, 0x7A, 0),
]

class AnimalForm(NinjaSpell):
	def __init__(self):
		NinjaSpell.__init__(self, 503, 0, 10)

	def cast(self, char, mode, args=[], target=None, item=None):
		# If already in animal form, revert
		if char.hastag('animal_form'):
			orig_body = int(char.gettag('animal_form_orig_body'))
			char.body = orig_body
			char.deltag('animal_form')
			char.deltag('animal_form_orig_body')
			char.effect(0x3728, 10, 13)
			char.update()
			sendToggleSpecialAbility(char, 503, False)
			if char.socket:
				char.socket.sysmessage(tr('You return to your normal form.'))
			return True

		if not self.consumerequirements(char, mode, args, target, item):
			return False

		# Show selection gump to choose form
		sendAnimalFormGump(char)
		return True

def sendAnimalFormGump(char):
	if not char or not char.socket:
		return
	gump = gumps.Gump()
	gump.addBackground(0, 0, 240, 360, 0x13BE)
	gump.addCheckerTrans(10, 10, 220, 340)
	gump.addText(30, 15, tr("Animal Form"), 1152)

	y = 45
	btn_id = 1
	for (name, req_skill, body, hue) in ANIMAL_FORMS:
		if char.skill[NINJITSU] >= req_skill:
			gump.addButton(20, y, 0x15E1, 0x15E5, btn_id)
			gump.addText(45, y, name, 0)
			y += 24
			btn_id += 1

	gump.send(char.socket, animal_form_gump_callback)

def animal_form_gump_callback(char, button, response):
	if button <= 0:
		return
	idx = button - 1
	valid_forms = [f for f in ANIMAL_FORMS if char.skill[NINJITSU] >= f[1]]
	if idx < len(valid_forms):
		(name, req_skill, body, hue) = valid_forms[idx]
		char.settag('animal_form', 1)
		char.settag('animal_form_orig_body', char.body)
		char.body = body
		char.effect(0x3728, 10, 13)
		char.soundeffect(0x511)
		char.update()
		sendToggleSpecialAbility(char, 503, True)

# -------------------------------------------------------------
# 504: Ki Attack
# -------------------------------------------------------------
class KiAttack(NinjaSpell):
	def __init__(self):
		NinjaSpell.__init__(self, 504, 800, 25)

	def checkrequirements(self, char, mode, args=[], target=None, item=None):
		if not NinjaSpell.checkrequirements(self, char, mode, args, target, item):
			return False
		if char.hidden:
			if char.socket:
				char.socket.clilocmessage(1063127) # You cannot use this ability while in stealth mode.
			return False
		return True

	def cast(self, char, mode, args=[], target=None, item=None):
		if char.hastag('ki_attack'):
			clearNinjaMove(char, 'ki_attack', 504)
			return True

		if not self.consumerequirements(char, mode, args, target, item):
			return False

		clearActiveNinjaMoves(char)
		char.settag('ki_attack', 1)
		char.settag('ki_start_x', char.pos.x)
		char.settag('ki_start_y', char.pos.y)
		sendToggleSpecialAbility(char, 504, True)
		if char.socket:
			char.socket.clilocmessage(1063099) # Your Ki Attack must be complete within 2 seconds for the damage bonus!

		char.dispel(char, True, 'KI_ATTACK_TIMER')
		char.addtimer(2000, ki_attack_expire, [], True, False, 'KI_ATTACK_TIMER')
		return True

def ki_attack_expire(char, args):
	if char and char.hastag('ki_attack'):
		clearNinjaMove(char, 'ki_attack', 504)
		char.deltag('ki_start_x')
		char.deltag('ki_start_y')
		if char.socket:
			char.socket.clilocmessage(1063102) # You failed to complete your Ki Attack in time.

# -------------------------------------------------------------
# 505: Surprise Attack
# -------------------------------------------------------------
class SurpriseAttack(NinjaSpell):
	def __init__(self):
		NinjaSpell.__init__(self, 505, 600, 20)

	def checkrequirements(self, char, mode, args=[], target=None, item=None):
		if not NinjaSpell.checkrequirements(self, char, mode, args, target, item):
			return False
		if not char.hidden:
			if char.socket:
				char.socket.clilocmessage(1063087) # You must be in stealth mode to use this ability.
			return False
		return True

	def cast(self, char, mode, args=[], target=None, item=None):
		if char.hastag('surprise_attack'):
			clearNinjaMove(char, 'surprise_attack', 505)
			return True

		if not self.consumerequirements(char, mode, args, target, item):
			return False

		clearActiveNinjaMoves(char)
		char.settag('surprise_attack', 1)
		sendToggleSpecialAbility(char, 505, True)
		if char.socket:
			char.socket.clilocmessage(1063128) # You prepare to surprise your prey.
		return True

def surprise_attack_expire(defender, args):
	if defender and defender.hastag('surprise_attack_malus'):
		defender.deltag('surprise_attack_malus')
		if defender.socket:
			defender.socket.clilocmessage(1063131) # Your defenses have returned to normal.

# -------------------------------------------------------------
# 506: Backstab
# -------------------------------------------------------------
class Backstab(NinjaSpell):
	def __init__(self):
		NinjaSpell.__init__(self, 506, 400, 30)

	def checkrequirements(self, char, mode, args=[], target=None, item=None):
		if not NinjaSpell.checkrequirements(self, char, mode, args, target, item):
			return False
		if not char.hidden:
			if char.socket:
				char.socket.clilocmessage(1063087) # You must be in stealth mode to use this ability.
			return False
		return True

	def cast(self, char, mode, args=[], target=None, item=None):
		if char.hastag('backstab'):
			clearNinjaMove(char, 'backstab', 506)
			return True

		if not self.consumerequirements(char, mode, args, target, item):
			return False

		clearActiveNinjaMoves(char)
		char.settag('backstab', 1)
		sendToggleSpecialAbility(char, 506, True)
		if char.socket:
			char.socket.clilocmessage(1063089) # You prepare to Backstab your opponent.
		return True

# -------------------------------------------------------------
# 507: Shadowjump
# -------------------------------------------------------------
class Shadowjump(NinjaSpell):
	def __init__(self):
		NinjaSpell.__init__(self, 507, 500, 15)
		self.validtarget = TARGET_GROUND

	def checkrequirements(self, char, mode, args=[], target=None, item=None):
		if not NinjaSpell.checkrequirements(self, char, mode, args, target, item):
			return False
		if not char.hidden:
			if char.socket:
				char.socket.clilocmessage(1063087) # You must be in stealth mode to use this ability.
			return False
		return True

	def target(self, char, mode, targettype, target, args, item):
		if not char.hidden:
			if char.socket:
				char.socket.clilocmessage(1063087)
			return False

		if not self.consumerequirements(char, mode, args, target, item):
			return False

		# Teleport while remaining hidden
		from_pos = char.pos
		char.pos.effect(0x3728, 10, 10)
		char.soundeffect(0x512)
		char.moveto(target)
		char.pos.effect(0x3728, 10, 10)
		char.update()
		return True

# -------------------------------------------------------------
# 508: Mirror Image
# -------------------------------------------------------------
class MirrorImage(NinjaSpell):
	def __init__(self):
		NinjaSpell.__init__(self, 508, 200, 10)

	def checkrequirements(self, char, mode, args=[], target=None, item=None):
		if not NinjaSpell.checkrequirements(self, char, mode, args, target, item):
			return False
		if char.itemonlayer(LAYER_MOUNT):
			if char.socket:
				char.socket.clilocmessage(1063132) # You cannot use this ability while mounted.
			return False
		return True

	def cast(self, char, mode, args=[], target=None, item=None):
		if not self.consumerequirements(char, mode, args, target, item):
			return False

		char.effect(0x376A, 1, 14)
		char.soundeffect(0x511)
		# Spawns a mirror image / illusion clone of caster
		count = int(char.gettag('mirror_images')) if char.hastag('mirror_images') else 0
		if count < 4:
			char.settag('mirror_images', count + 1)
			if char.socket:
				char.socket.sysmessage(tr('You summon a mirror image of yourself.'))
		return True

# -------------------------------------------------------------
# Global helpers for Ninjitsu moves
# -------------------------------------------------------------
def clearNinjaMove(char, tag, ability_id):
	if char:
		char.deltag(tag)
		sendToggleSpecialAbility(char, ability_id, False)

def clearActiveNinjaMoves(char):
	if char.hastag('focus_attack'):
		clearNinjaMove(char, 'focus_attack', 501)
	if char.hastag('death_strike'):
		clearNinjaMove(char, 'death_strike', 502)
	if char.hastag('ki_attack'):
		clearNinjaMove(char, 'ki_attack', 504)
	if char.hastag('surprise_attack'):
		clearNinjaMove(char, 'surprise_attack', 505)
	if char.hastag('backstab'):
		clearNinjaMove(char, 'backstab', 506)

def onLoad():
	FocusAttack().register(501)
	DeathStrike().register(502)
	AnimalForm().register(503)
	KiAttack().register(504)
	SurpriseAttack().register(505)
	Backstab().register(506)
	Shadowjump().register(507)
	MirrorImage().register(508)
