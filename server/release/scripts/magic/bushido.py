#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Created by: Naddel                         #
#   )).-' {{ ;'`   | Revised by:                                #
#  ( (  ;._ \\ ctr | Last Modification: Created                 #
#===============================================================#
# Bushido spells & moves                                        #
#===============================================================#

import random
import wolfpack
import wolfpack.time
from wolfpack.consts import *
from wolfpack import properties, tr
from magic.spell import Spell
from magic.utilities import MODE_BOOK, MODE_SCROLL, MODE_CMD, MODE_WAND, fizzle

# Helper to send ToggleSpecialAbility packet (0xBF 0x25)
def sendToggleSpecialAbility(char, ability_id, active):
	if char and char.socket:
		packet = wolfpack.packet(0xbf, 7)
		packet.setshort(1, 7)
		packet.setshort(3, 0x25)
		packet.setshort(5, int(ability_id))
		packet.setbyte(7, 1 if active else 0)
		packet.send(char.socket)

# Checks if mobile has weapon or shield equipped
def hasWeaponOrShield(char):
	if char.itemonlayer(LAYER_LEFTHAND) or char.itemonlayer(LAYER_RIGHTHAND):
		return True
	return False

# Base class for Bushido abilities
class BushidoSpell(Spell):
	def __init__(self, spellid, req_skill, req_mana):
		Spell.__init__(self, 0)
		self.spellid = spellid
		self.skill = BUSHIDO
		self.requiredskill = req_skill
		self.mana = req_mana
		self.mantra = ''

	def checkweapon(self, char):
		# Bushido doesn't require free hands
		return True

	def calcdelay(self, char, mode):
		# Bushido moves are instant cast (0.25s / instant)
		return 250

	def onBeginCast(self, char):
		# Purple cast sparkle
		char.effect(0x37C4, 10, 7)

	def checkrequirements(self, char, mode, args=[], target=None, item=None):
		if not Spell.checkrequirements(self, char, mode, args, target, item):
			return False
		if char.skill[BUSHIDO] < self.requiredskill:
			if char.socket:
				char.socket.clilocmessage(1070768, str(self.requiredskill / 10.0)) # You need ~1_SKILL_REQUIREMENT~ Bushido skill to perform that attack!
			return False
		scaled_mana = self.scalemana(char, mode, self.mana)
		if char.mana < scaled_mana:
			if char.socket:
				char.socket.clilocmessage(1060174, str(scaled_mana)) # You must have at least ~1_MANA_REQUIREMENT~ Mana to use this ability.
			return False
		return True

	def checkcast(self, char):
		return self.checkrequirements(char, MODE_BOOK)

# -------------------------------------------------------------
# 401: Honorable Execution
# -------------------------------------------------------------
class HonorableExecution(BushidoSpell):
	def __init__(self):
		BushidoSpell.__init__(self, 401, 250, 0)

	def cast(self, char, mode, args=[], target=None, item=None):
		# Toggle move
		if char.hastag('honorable_execution'):
			clearMove(char, 'honorable_execution', 401)
			return True

		if not self.consumerequirements(char, mode, args, target, item):
			return False

		clearActiveMoves(char)
		char.settag('honorable_execution', 1)
		sendToggleSpecialAbility(char, 401, True)
		if char.socket:
			char.socket.clilocmessage(1063122) # You better kill your enemy with your next hit or you'll be rather sorry...
		return True

def honorable_penalty_expire(char, args):
	if char:
		char.deltag('honorable_penalty')
		if char.socket:
			char.socket.clilocmessage(1063121) # You return to your normal stance / effects end

def honorable_swing_bonus_expire(char, args):
	if char:
		char.deltag('honorable_swing_bonus')

# -------------------------------------------------------------
# 402: Confidence
# -------------------------------------------------------------
class Confidence(BushidoSpell):
	def __init__(self):
		BushidoSpell.__init__(self, 402, 250, 10)

	def cast(self, char, mode, args=[], target=None, item=None):
		if not self.consumerequirements(char, mode, args, target, item):
			return False

		# Interrupt other Bushido stances
		endConfidence(char)
		endEvasion(char)
		endCounterAttack(char)

		char.settag('confidence_active', 1)
		char.soundeffect(0x51A)
		char.effect(0x375A, 1, 17)
		if char.socket:
			char.socket.clilocmessage(1063115) # You exude confidence.

		sendToggleSpecialAbility(char, 402, True)

		# 15s overall stance timer for parry heal bonus
		char.dispel(char, True, 'CONFIDENCE_STANCE')
		char.addtimer(15000, confidence_stance_expire, [], True, False, 'CONFIDENCE_STANCE')

		# Regen HP over 4-5 ticks
		bushido = char.skill[BUSHIDO]
		total_hits = 15 + int((bushido * bushido) / 57600.0)
		char.settag('confidence_regen_left', total_hits)
		char.settag('confidence_regen_tick', 0)
		char.dispel(char, True, 'CONFIDENCE_REGEN')
		char.addtimer(1000, confidence_regen_tick, [], True, False, 'CONFIDENCE_REGEN')
		return True

def confidence_regen_tick(char, args):
	if not char or not char.hastag('confidence_regen_left'):
		return
	left = int(char.gettag('confidence_regen_left'))
	tick = int(char.gettag('confidence_regen_tick')) + 1
	char.settag('confidence_regen_tick', tick)

	heal_amount = max(1, left // 5)
	if tick >= 5:
		heal_amount = left
		char.deltag('confidence_regen_left')
		char.deltag('confidence_regen_tick')
	else:
		char.settag('confidence_regen_left', left - heal_amount)
		char.addtimer(1000, confidence_regen_tick, [], True, False, 'CONFIDENCE_REGEN')

	if char.hitpoints < char.maxhitpoints:
		char.hitpoints = min(char.maxhitpoints, char.hitpoints + heal_amount)
		char.updatehealth()

def confidence_stance_expire(char, args):
	if char and char.hastag('confidence_active'):
		endConfidence(char)
		if char.socket:
			char.socket.clilocmessage(1063116) # Your confidence wanes.

def endConfidence(char):
	if char:
		char.deltag('confidence_active')
		char.deltag('confidence_regen_left')
		char.deltag('confidence_regen_tick')
		char.dispel(char, True, 'CONFIDENCE_STANCE')
		char.dispel(char, True, 'CONFIDENCE_REGEN')
		sendToggleSpecialAbility(char, 402, False)

# -------------------------------------------------------------
# 403: Evasion
# -------------------------------------------------------------
class Evasion(BushidoSpell):
	def __init__(self):
		BushidoSpell.__init__(self, 403, 600, 10)

	def checkrequirements(self, char, mode, args=[], target=None, item=None):
		if not BushidoSpell.checkrequirements(self, char, mode, args, target, item):
			return False
		if not hasWeaponOrShield(char):
			if char.socket:
				char.socket.clilocmessage(1062944) # You must have a weapon or a shield equipped to use this ability!
			return False
		if char.hastag('evasion_cooldown'):
			if char.socket:
				char.socket.clilocmessage(501789) # You must wait before trying again.
			return False
		return True

	def cast(self, char, mode, args=[], target=None, item=None):
		if not self.consumerequirements(char, mode, args, target, item):
			return False

		endConfidence(char)
		endEvasion(char)
		endCounterAttack(char)

		char.soundeffect(0x51B)
		char.effect(0x376A, 1, 20)
		if char.socket:
			char.socket.clilocmessage(1063120) # You feel that you might be able to deflect any attack!

		sendToggleSpecialAbility(char, 403, True)
		char.settag('evasion_active', 1)

		# Duration scales with Bushido
		bushido = char.skill[BUSHIDO] / 10.0
		duration = 3.0
		if bushido > 60.0:
			duration += (bushido - 60.0) / 20.0
		if char.skill[ANATOMY] >= 1000 and char.skill[TACTICS] >= 1000 and char.skill[BUSHIDO] > 1000:
			duration += 1.0

		char.dispel(char, True, 'EVASION_DURATION')
		char.addtimer(int(duration * 1000), evasion_expire, [], True, False, 'EVASION_DURATION')

		# Cooldown: 20 seconds
		char.settag('evasion_cooldown', 1)
		char.dispel(char, True, 'EVASION_COOLDOWN')
		char.addtimer(20000, evasion_cooldown_expire, [], True, False, 'EVASION_COOLDOWN')
		return True

def evasion_expire(char, args):
	if char and char.hastag('evasion_active'):
		endEvasion(char)
		if char.socket:
			char.socket.clilocmessage(1063121) # You no longer feel that you could deflect any attack.

def evasion_cooldown_expire(char, args):
	if char:
		char.deltag('evasion_cooldown')

def endEvasion(char):
	if char:
		char.deltag('evasion_active')
		char.dispel(char, True, 'EVASION_DURATION')
		sendToggleSpecialAbility(char, 403, False)

# -------------------------------------------------------------
# 404: Counter Attack
# -------------------------------------------------------------
class CounterAttack(BushidoSpell):
	def __init__(self):
		BushidoSpell.__init__(self, 404, 400, 5)

	def checkrequirements(self, char, mode, args=[], target=None, item=None):
		if not BushidoSpell.checkrequirements(self, char, mode, args, target, item):
			return False
		if not hasWeaponOrShield(char):
			if char.socket:
				char.socket.clilocmessage(1062944) # You must have a weapon or a shield equipped to use this ability!
			return False
		return True

	def cast(self, char, mode, args=[], target=None, item=None):
		if not self.consumerequirements(char, mode, args, target, item):
			return False

		endConfidence(char)
		endEvasion(char)
		endCounterAttack(char)

		char.settag('counterattack_active', 1)
		sendToggleSpecialAbility(char, 404, True)
		if char.socket:
			char.socket.clilocmessage(1063118) # You prepare to respond immediately to the next blocked blow.

		char.dispel(char, True, 'COUNTERATTACK_DURATION')
		char.addtimer(30000, counterattack_expire, [], True, False, 'COUNTERATTACK_DURATION')
		return True

def counterattack_expire(char, args):
	if char and char.hastag('counterattack_active'):
		endCounterAttack(char)
		if char.socket:
			char.socket.clilocmessage(1063119) # You return to your normal stance.

def endCounterAttack(char):
	if char:
		char.deltag('counterattack_active')
		char.dispel(char, True, 'COUNTERATTACK_DURATION')
		sendToggleSpecialAbility(char, 404, False)

# -------------------------------------------------------------
# 405: Lightning Strike
# -------------------------------------------------------------
class LightningStrike(BushidoSpell):
	def __init__(self):
		BushidoSpell.__init__(self, 405, 500, 5)

	def cast(self, char, mode, args=[], target=None, item=None):
		if char.hastag('lightning_strike'):
			clearMove(char, 'lightning_strike', 405)
			return True

		if not self.consumerequirements(char, mode, args, target, item):
			return False

		clearActiveMoves(char)
		char.settag('lightning_strike', 1)
		sendToggleSpecialAbility(char, 405, True)
		if char.socket:
			char.socket.clilocmessage(1063167) # You prepare to strike quickly.
		return True

# -------------------------------------------------------------
# 406: Momentum Strike
# -------------------------------------------------------------
class MomentumStrike(BushidoSpell):
	def __init__(self):
		BushidoSpell.__init__(self, 406, 700, 10)

	def cast(self, char, mode, args=[], target=None, item=None):
		if char.hastag('momentum_strike'):
			clearMove(char, 'momentum_strike', 406)
			return True

		if not self.consumerequirements(char, mode, args, target, item):
			return False

		clearActiveMoves(char)
		char.settag('momentum_strike', 1)
		sendToggleSpecialAbility(char, 406, True)
		if char.socket:
			char.socket.clilocmessage(1070757) # You prepare to strike two enemies with one blow.
		return True

# -------------------------------------------------------------
# Global helpers for moves
# -------------------------------------------------------------
def clearMove(char, tag, ability_id):
	if char:
		char.deltag(tag)
		sendToggleSpecialAbility(char, ability_id, False)

def clearActiveMoves(char):
	if char.hastag('honorable_execution'):
		clearMove(char, 'honorable_execution', 401)
	if char.hastag('lightning_strike'):
		clearMove(char, 'lightning_strike', 405)
	if char.hastag('momentum_strike'):
		clearMove(char, 'momentum_strike', 406)

def onLoad():
	HonorableExecution().register(401)
	Confidence().register(402)
	Evasion().register(403)
	CounterAttack().register(404)
	LightningStrike().register(405)
	MomentumStrike().register(406)
