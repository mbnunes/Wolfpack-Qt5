
import wolfpack
from wolfpack import tr
from wolfpack.consts import *
from combat.utilities import weaponskill
from wolfpack.utilities import energydamage, tobackpack
import random
from math import floor
import system.poison
import combat.aos
from magic.circle5 import paralyze_expire

# Ability registry
ABILITIES = {}

# Skills used for the skillsum (scaling mana)
SKILLS = [SWORDSMANSHIP, MACEFIGHTING, FENCING, WRESTLING, ARCHERY, PARRYING, LUMBERJACKING, STEALTH, POISONING]

# Cache the clear packet
capacket = wolfpack.packet(0xbf, 5)
capacket.setshort(1, 5)
capacket.setshort(3, 33)

#
# Get the current ability instance
#
def getability(player):
	if not player.socket or not player.socket.hastag('weaponability'):
		return None

	ability = player.socket.gettag('weaponability')

	if not ability in ABILITIES:
		return None

	instance = ABILITIES[ability]

	if not instance.checkuse(player):
		clearability(player)
		return None
	else:
		return instance

#
# Clear the current ability instance
#
def clearability(player):
	if player.socket:
		player.socket.deltag('weaponability')
		capacket.send(player.socket)

#
# Ability baseclass
#
class BaseAbility:
	def __init__(self, id):
		self.id = id
		ABILITIES[id] = self
		self.hitchance = 1.0 # Factor for the hitchance of the attack made using this ability
		self.damage = 1.0 # Factor for the damage of the attack made using this ability
		self.mana = 20 # Required mana to use
		self.primary = [] # List of baseids this is the primary ability for
		self.secondary = [] # List of baseids this is the secondary ability for
		self.ignorephysical = False # ignore the defenders physical resistance if this ability is used
		self.movingshot = False # Weapon can be fired on the move
		# Note: not an intproperty because the client has a hardcoded list of abilities for 
		# weapons. SPECIAL MAGIC: None is the baseid for fists (!)

	#
	# Scale the damage of the attack made using this ability
	#
	def scaledamage(self, attacker, defender, damage):
		return damage * self.damage

	#
	# Scale the accuracy of this attack
	#
	def scalehitchance(self, attacker, defender, chance):
		return chance * self.hitchance

	#
	# This event is called if the player hits an opponent.
	#
	def hit(self, attacker, defender, damage):
		pass

	#
	# This event is called if the player misses an opponent.
	#
	def miss(self, attacker, defender):
		pass		

	#
	# Get the required mana for this move
	#
	def getmana(self, player, weapon):
		mana = self.mana
		skillsum = 0

		# Scale according to skillsum
		for skill in SKILLS:
			skillsum += player.skill[skill]

		if skillsum >= 3000:
			mana = max(0, mana - 10) # - 10 mana for 300% secondary skills
		elif skillsum >= 2000:
			mana = max(0, mana - 5) # - 5 mana for 200% secondary skills

		return mana

	#
	# Check if the weapon is a primary ability or not
	#
	def checkprimary(self, player, weapon):
		if not weapon and None in self.primary:
			return True # primary unarmed
		elif weapon and weapon.baseid in self.primary:
			return True # primary normal
		else:
			return False # secondary or invalid

	#
	# Get the required amount of skill to use this ability
	#
	def getskill(self, player, weapon):
		if self.checkprimary(player, weapon):
			return 700
		else:
			return 900

	#
	# Checks if the player meets the skill requirements to use
	# this weapon ability
	#
	def checkskill(self, player, weapon):
		skill = self.getskill(player, weapon) # Get the required skill amount
		skillid = weaponskill(player, weapon, True) # Get the weaponskill used by the player

		result = player.skill[skillid] >= skill

		if not result and player.socket:
			player.socket.clilocmessage(1060182, '%0.f%%' % (skill / 10.0))

		return result

	#
	# Checks if this player meets the mana requriements for this
	# weapon ability.
	#
	def checkmana(self, player, weapon, use = False):
		mana = self.getmana(player, weapon)

		if mana == 0:
			return True # This ability does not require mana

		if player.socket and player.socket.hastag('last_special_ability'):
			if player.socket.gettag('last_special_ability') + 3000 > wolfpack.time.currenttime():
				mana *= 2
			else:
				player.socket.deltag('last_special_ability')

		# Check if there is enough mana
		if player.mana < mana:
			if player.socket:
				player.socket.clilocmessage(1060181, str(mana)) # You need xxx mana.
			return False

		# Consume the mana
		if use:
			player.mana -= mana
			player.updatemana()

		return True

	#
	# Checks if this ability can be used with the weapon.
	#
	def checkweapon(self, player, weapon):
		if not weapon and (None in self.primary or None in self.secondary):
			return True
		elif weapon:
			return weapon.baseid in self.primary or weapon.baseid in self.secondary
		else:
			if player.socket:
				player.socket.sysmessage(tr('You cannot use this ability with that weapon.'))
			return False

	#
	# See if the player can use this ability. 
	# If use is true, really consume mana.
	#
	def checkuse(self, player):
		weapon = player.getweapon() # Get the used weapon	
		return self.checkweapon(player, weapon) and self.checkmana(player, weapon) and self.checkskill(player, weapon)

	#
	# Actively use the ability. It clears the current ability and uses the mana.
	#
	def use(self, player):
		weapon = player.getweapon() # Get the used weapon
		self.checkmana(player, weapon, True) # Consume the mana
		clearability(player) # Clear the ability
		if player.socket:
			player.socket.settag('last_special_ability', wolfpack.time.currenttime())		

#
# An ability was selected
#
def onSelectAbility(player, ability):
	if ability == 0:
		clearability(player)
	elif ability in ABILITIES:
		instance = ABILITIES[ability]
		if not instance.checkuse(player):
			clearability(player)
		else:
			player.socket.settag('weaponability', ability)
	else:
		player.socket.sysmessage(tr('This ability has not been implemented yet.'))
		clearability(player)

	return True

#
# Register the global handler for selecting a weapon ability
#
def onLoad():
	wolfpack.registerglobal(EVENT_SELECTABILITY, "combat.specialmoves")

#
# Armor Ignore
#
class ArmorIgnore(BaseAbility):
	def __init__(self):
		BaseAbility.__init__(self, 1)
		self.mana = 30
		self.damage = 0.9 # Only 90% damage
		self.primary = ['f43', 'f44', '13af', '13b0', '143c', '143d', '26cc', '26c2', '26bd', '26c7', 'f63', 'f62', '1400', '1401', '13b7', '13b8', 'f60', 'f61']
		self.secondary = ['f5e', 'f5f', '13fe', '13ff', ]
		self.ignorephysical = True

	def hit(self, attacker, defender, damage):
		weapon = attacker.getweapon()
		if not self.checkuse(attacker):
			return

		self.use(attacker)

		if defender.dead or defender.pos.map == 0xFF:
			return # Out of reach		

		if attacker.socket:
			attacker.socket.clilocmessage(1060076)
		if defender.socket:
			defender.socket.clilocmessage(1060077)

		defender.soundeffect(0x56)
		defender.effect(0x3728, 200, 25)
ArmorIgnore()

#
# Bleed Attack
#
import system.bleeding

class BleedAttack(BaseAbility):
	def __init__(self):
		BaseAbility.__init__(self, 2)
		self.mana = 30
		self.primary = ['f47', 'f48', 'f45', 'f46', 'ec2', 'ec3', '26ba', '26c4', 'e87', 'e88', '1404', '1405', '1440', '1441']
		self.secondary = ['13fa', '13fb', '13af', '13b0', '1406', '1407']

	def hit(self, attacker, defender, damage):
		if not self.checkuse(attacker):
			return

		self.use(attacker)

		if defender.dead or defender.pos.map == 0xFF:
			return # Out of reach		

		if attacker.socket:
			attacker.socket.clilocmessage(1060159) # Your target is bleeding
		if defender.socket:
			defender.socket.clilocmessage(1060160) # You are bleeding

		defender.soundeffect(0x133)
		defender.effect(0x377a, 244, 25)

		system.bleeding.start(defender, attacker)
BleedAttack()

#
# Concussion Blow
#
class ConcussionBlow(BaseAbility):
	def __init__(self):
		BaseAbility.__init__(self, 3)
		self.mana = 25
		self.primary = ['f5c', 'f5d', 'f4f', 'f50', '13f9', '13f8']
		self.secondary = ['f47', 'f48', '143a', '143b', '143e', '143f', 'e8a', 'e89', '26c0', '26ca', '13b7', '13b8', 'f60', 'f61']

	def hit(self, attacker, defender, damage):
		if not self.checkuse(attacker):
			return

		self.use(attacker)

		if defender.dead or defender.pos.map == 0xFF:
			return # Out of reach

		if attacker.socket:
			attacker.socket.clilocmessage(1060165) # You have delivered a concussion!
		if defender.socket:
			defender.socket.clilocmessage(1060166) # You feel disoriented!

		defender.soundeffect(0x213)
		defender.effect(0x377a, 1, 32)

		source = defender.pos
		target = defender.pos
		source.z += 10
		target.z += 20
		sendmovingeffect(source, target, model = 0x36fe, hue = 1133)

		#energydamage(defender, attacker, random.randint(10, 40), physical=100, damagetype=DAMAGE_PHYSICAL)
		defender.damage(DAMAGE_GODLY, random.randint(10, 40), attacker)
ConcussionBlow()

def sendmovingeffect(source, target, model, hue=0, speed=1, rendermode=0):
	# Build the packet
	packet = wolfpack.packet(0xC0, 36)
	packet.setbyte(1, 0) # Moving
	packet.setint(2, 0) # Source
	packet.setint(6, 0) # Target
	packet.setshort(10, model) # Effect Model
	packet.setshort(12, source.x) # Source X
	packet.setshort(14, source.y) # Source Y
	packet.setbyte(16, source.z) # Source Z
	packet.setshort(17, target.x) # Target X
	packet.setshort(19, target.y) # Target Y
	packet.setbyte(21, target.z) # Target Z	
	packet.setbyte(22, speed) # Speed
	packet.setbyte(23, 0) # Duration
	packet.setshort(24, 0) # Unknown
	packet.setbyte(26, 1) # Fixed Direction
	packet.setbyte(27, 0) # Explode on Impact
	packet.setint(28, hue) # Hue
	packet.setint(32, rendermode) # Rendermode

	# Search for sockets at the source location
	chars = wolfpack.chars(source.x, source.y, source.map, 18)
	for char in chars:
		if char.socket:	
			packet.send(char.socket)

#
# CrushingBlow
#
class CrushingBlow(BaseAbility):
	def __init__(self):
		BaseAbility.__init__(self, 4)
		self.mana = 25
		self.primary = ['f49', 'f4a', '143a', '143b', '26bc', '26c6', '1406', '1407', '13f4', '13f5', 'f5e', 'f5f', '13b9', '13ba']
		self.secondary = ['1438', '1439']

	def hit(self, attacker, defender, damage):
		if not self.checkuse(attacker):
			return

		self.use(attacker)

		if defender.dead or defender.pos.map == 0xFF:
			return # Out of reach		

		if attacker.socket:
			attacker.socket.clilocmessage(1060090) # You have delivered a crushing blow!
		if defender.socket:
			defender.socket.clilocmessage(1060091) # You take extra damage from the crushing attack!

		defender.soundeffect(0x1e1)
		#defender.effect(0x377a, 1, 32)

		source = defender.pos
		target = defender.pos
		source.z += 50
		target.z += 20
		sendmovingeffect(source, target, model = 0xfb4)

		amount = floor(0.5 * damage)
		if amount != 0:
			energydamage(defender, attacker, amount, physical=100, damagetype=DAMAGE_PHYSICAL)

CrushingBlow()

#
# Disarm
#
class Disarm (BaseAbility):
	def __init__(self):
		BaseAbility.__init__(self, 5)
		self.primary = [None] # Unarmed
		self.secondary = ['f43', 'f44', 'e85', 'e86', '13f6', '13f7', 'ec4', 'ec5', 'f5c', 'f5d', '1404', '1405', '13f4', '13f5']

	def hit(self, attacker, defender, damage):
		if not self.checkuse(attacker):
			return

		# Clear Ability, then check if it really can be used
		clearability(attacker)

		if defender.dead or defender.pos.map == 0xFF:
			return # Out of reach		

		weapon = defender.getweapon()

		# Doesnt have a weapon to disarm
		if not weapon:
			if attacker.socket:
				attacker.socket.clilocmessage(1060849) # Already unarmed ...

		# The weapon is immovable
		elif weapon.lockeddown or weapon.movable > 1:
			if attacker.socket:
				attacker.socket.clilocmessage(1004001) # You cannot disarm ...

		else:
			self.use(attacker)
			if attacker.socket:
				attacker.socket.clilocmessage(1060092) # You disarm their weapon!
			if defender.socket:
				defender.socket.clilocmessage(1060093) # Your weapon has been disarmed!

			defender.soundeffect(0x3b9)
			defender.effect(0x37be, 232, 25)

			layer = weapon.layer # Save the weapon layer

			if not tobackpack(weapon, defender): # Unequip the weapon
				weapon.update()

			# Players cannot equip another weapon for 5000 ms
			if defender.socket:
				defender.socket.settag('block_equip', wolfpack.time.currenttime() + 5000)
			elif defender.npc:
				defender.addtimer(random.randint(5000, 7500), reequip_weapon, [weapon.serial, layer], True)

def reequip_weapon(npc, args):
	weapon = wolfpack.finditem(args[0])
	layer = args[1]
	backpack = npc.getbackpack()

	if weapon and npc and weapon.container == backpack:
		if npc.getweapon() == None and not npc.itemonlayer(layer):
			npc.additem(layer, weapon)
			weapon.update()
Disarm()

#
# Dismount
#
class Dismount (BaseAbility):
	def __init__(self):
		BaseAbility.__init__(self, 6)
		self.primary = ['26c0', '26ca']
		self.secondary = ['f49', 'f4a', '13b3', '13b4', 'f4d', 'f4e', '13fc', '13fd', '26bd', '26c7', 'e87', 'e88']

	def checkuse(self, attacker):
		result = BaseAbility.checkuse(self, attacker)
		if result:
			if attacker.ismounted():
				weapon = attacker.getweapon()
				if not weapon or not weapon.baseid in ['26c0', '26ca']:
					if attacker.socket:
						attacker.socket.clilocmessage(1061283)
					return False
		return result

	# NOTE: Only usable against mounted players
	def hit(self, attacker, defender, damage):
		if not self.checkuse(attacker):
			return

		# Clear Ability, then check if it really can be used
		clearability(attacker)

		if defender.dead or defender.pos.map == 0xFF:
			return # Out of reach		

		if not defender.socket or not defender.ismounted():
			if attacker.socket:
				attacker.socket.clilocmessage(1060848)
			return

		self.use(attacker) # Use Mana
		
		if attacker.socket:
			attacker.socket.clilocmessage(1060082)

		mounted = attacker.ismounted()

		if mounted:
			defender.socket.clilocmessage(1062315)
		else:
			defender.socket.clilocmessage(1060083)

		defender.soundeffect(0x140)
		defender.effect(0x3728, 10, 15)

		defender.unmount()		

		# Make it impossible to mount a horse for 10 seconds
		defender.socket.settag('block_mount', wolfpack.time.currenttime() + 10000)

		if not mounted:
			energydamage(defender, attacker, random.randint(15, 25), physical=100, damagetype=DAMAGE_PHYSICAL)

Dismount()

#
# DoubleStrike
#
class DoubleStrike(BaseAbility):
	def __init__(self):
		BaseAbility.__init__(self, 7)
		self.mana = 30
		self.primary = ['f4c', 'f4b', '13ff', '13fe', 'e85', 'e86', '1442', '1443', '26c3', '26cd', '26bf', '26c9', 'e89', 'e8a', '26c1', '26cb', '13b6', '13b5']
		self.secondary = []
		self.damage = 0.9

	def hit(self, attacker, defender, damage):
		if not self.checkuse(attacker):
			return

		self.use(attacker)

		if defender.dead or defender.pos.map == 0xFF:
			return # Out of reach		

		if attacker.socket:
			attacker.socket.clilocmessage(1060084) # You attack with lightning speed!
		if defender.socket:
			defender.socket.clilocmessage(1060085) # Your attacker strikes with lightning speed!

		if defender.dead or attacker.dead:
			return

		defender.soundeffect(0x3bb)
		defender.effect(0x37b9, 244, 25)

		# Make sure that the next swing has only 90% of the damage
		if attacker.socket:
			attacker.socket.settag('weaponability', 1000)
		attacker.callevent(EVENT_SWING, (attacker, defender, wolfpack.time.currenttime())) # Swing again

class DoubleStrikeInternal(BaseAbility):
	def __init__(self):
		BaseAbility.__init__(self, 1000)
		self.mana = 0 # Don't require mana
		self.damage = 0.9

	def checkuse(self, attacker):
		return True

	def hit(self, attacker, defender, damage):
		clearability(attacker)

	def miss(self, attacker, defender):
		clearability(attacker)

DoubleStrike()
DoubleStrikeInternal()

#
# InfectiousStrike
#
class InfectiousStrike(BaseAbility):
	def __init__(self):
		BaseAbility.__init__(self, 8)
		self.mana = 15
		self.primary = ['f51', 'f52', '13f6', '13f7']
		self.secondary = ['ec2', 'ec3', '26bf', '26c9', '26c8', '26be', '1400', '1401']

	def hit(self, attacker, defender, damage):
		if not self.checkuse(attacker):
			return

		# The weapon has to have poison charges in order to use this ability
		poisoning_uses = 0
		weapon = attacker.getweapon()
		if weapon and weapon.hastag('poisoning_uses'):
			poisoning_uses = int(weapon.gettag('poisoning_uses'))

		if poisoning_uses <= 0:
			if attacker.socket:
				attacker.socket.clilocmessage(1061141) # Your weapon has no charges...
			return

		self.use(attacker)

		if defender.dead or defender.pos.map == 0xFF:
			return # Out of reach		

		poisoning_uses -= 1
		if poisoning_uses <= 0:
			weapon.deltag('poisoning_uses')
			weapon.resendtooltip()
		else:
			weapon.settag('poisoning_uses', poisoning_uses)

		poisoning_strength = 0 # Assume lesser unless the tag tells so otherwise
		if weapon.hastag('poisoning_strength'):
			poisoning_strength = int(weapon.gettag('poisoning_strength'))

		# High poisoning yields a higher chance of increasing the poison level
		if poisoning_strength < 4 and attacker.skill[POISONING] / 1000.0 > random.random():
			poisoning_strength += 1
			if attacker.socket:
				attacker.socket.clilocmessage(1060080) # Your precise strike has increased the level of the poison by 1
			if defender.socket:
				defender.socket.clilocmessage(1060081) # The poison seems extra effective!

		defender.soundeffect(0xdd)
		defender.effect(0x3728, 244, 25)

		# Apply poison
		if system.poison.poison(defender, poisoning_strength):
			if attacker.socket:
				attacker.socket.clilocmessage(1008096, "", 0x3b2, 3, None, defender.name, False, False)
			if defender.socket:
				attacker.socket.clilocmessage(1008097, "", 0x3b2, 3, None, attacker.name, False, True)

InfectiousStrike()

#
# MortalStrike
#
class MortalStrike(BaseAbility):
	def __init__(self):
		BaseAbility.__init__(self, 9)
		self.mana = 30
		self.primary = []
		self.secondary = ['26c1', '26cb', 'f46', 'f45', '143c', '143d', '26bc', '26c6', '13b1', '13b2', 'f4f', 'f50', '1402', '1403', '26bb', '26c5', '26c1', '26cb']

	def hit(self, attacker, defender, damage):
		if not self.checkuse(attacker):
			return
		self.use(attacker)

		if defender.dead or defender.pos.map == 0xFF:
			return # Out of reach		

		if attacker.socket:
			attacker.socket.clilocmessage(1060086) # You deliver a mortal wound!
		if defender.socket:
			defender.socket.clilocmessage(1060087) # You have been mortally wounded!

		defender.soundeffect(0x1e1)
		defender.effect(0x37b9, 244, 25)
		defender.dispel(None, True, 'MORTALSTRIKE_CHECK')

		if defender.npc:
			defender.settag('mortalstrike', wolfpack.time.currenttime() + 12000)
			defender.addtimer(12000, ismortallywounded, [True], False, False, 'MORTALSTRIKE_CHECK')
		else:
			defender.settag('mortalstrike', wolfpack.time.currenttime() + 6000)
			defender.addtimer(6000, ismortallywounded, [True], False, False, 'MORTALSTRIKE_CHECK')

MortalStrike()

#
# Checks the current mortal strike status of the victim and returns if its affected
#
def ismortallywounded(char, args = None):
	if args and args[0]:
		char.deltag('mortalstrike') # Delete the mortal strike tag
		if char.socket:
			char.socket.clilocmessage(1060208)
		return False

	if not char.hastag('mortalstrike'):
		return False
		
	expire = int(char.gettag('mortalstrike'))
	if expire < wolfpack.time.currenttime():
		char.deltag('mortalstrike')
		if char.socket:
			char.socket.clilocmessage(1060208)
		return False

	# Wont it expire within 20 seconds? > Server Restart occured
	if expire + 20000 < wolfpack.time.currenttime():
		char.deltag('mortalstrike')
		if char.socket:
			char.socket.clilocmessage(1060208)
		return False

	return True # Character is affected

#
# MovingShot
#
class MovingShot(BaseAbility):
	def __init__(self):
		BaseAbility.__init__(self, 10)
		self.mana = 15
		self.hitchance = 0.75 # Only 75% chance to hit
		self.primary = ['13fc', '13fd']
		self.secondary = ['26c2', '26cc', '26c3', '26cd']
		self.movingshot = True

	def miss(self, attacker, defender):
		self.use(attacker)

		if attacker.socket:
			attacker.socket.clilocmessage(1060089)

	def hit(self, attacker, defender, damage):
		self.use(attacker)	

		if attacker.socket:
			attacker.socket.clilocmessage(1060216)
MovingShot()

#
# ParalyzeBlow
#
class ParalyzeBlow(BaseAbility):
	def __init__(self):
		BaseAbility.__init__(self, 11)
		self.mana = 30
		self.primary = [None, 'f4d', 'f4e', '13b1', '13b2', '26c8', '26be', '26bb', '26c5']
		self.secondary = ['26ba', '26c4', 'f62', 'f63', 'df0', 'df1', '13f9', '13f8', '13b5', '13b6', '13ba', '13b9']

	def hit(self, attacker, defender, damage):
		if not self.checkuse(attacker):
			return

		self.use(attacker)

		if defender.dead or defender.pos.map == 0xFF:
			return # Out of reach		

		if attacker.socket:
			attacker.socket.clilocmessage(1060163) # You deliver a paralyzing blow!
		if defender.socket:
			defender.socket.clilocmessage(1060164) # The attack has temporarily paralyzed you!

		defender.soundeffect(0x204)
		defender.effect(0x376a, 9, 32)

		if defender.player:
			duration = 3000
		else:
			duration = 6000

		defender.addscript("combat.paralyze_blow")

		# Use the normal paralyze code from here
		defender.disturb()
		defender.frozen = True
		defender.resendtooltip()
		defender.dispel(None, True, "PARALYZE_EXPIRE")
		defender.addtimer(duration, paralyze_expire, [], True, False, "PARALYZE_EXPIRE")

ParalyzeBlow()

#
# ShadowStrike
#
class ShadowStrike(BaseAbility):
	def __init__(self):
		BaseAbility.__init__(self, 12)
		self.mana = 30
		self.primary = ['ec4', 'ec5', '13b3', '13b4', '1402', '1403']
		self.secondary = ['1442', '1443', 'f51', 'f52', '1440', '1441']

	#
	# Check if the player has the required stealth skill
	#
	def checkuse(self, attacker):
		result = BaseAbility.checkuse(self, attacker)
		if result and attacker.skill[STEALTH] < 800:
			if attacker.socket:
				attacker.socket.clilocmessage(1060183) # You lack the required lalala...
			return False
		return result

	def hit(self, attacker, defender, damage):
		if not self.checkuse(attacker):
			return

		self.use(attacker)

		if defender.dead or defender.pos.map == 0xFF:
			return # Out of reach		

		if attacker.socket:
			attacker.socket.clilocmessage(1060078) # You strike and hide in the shadows!
		if defender.socket:
			defender.socket.clilocmessage(1060079) # You are dazed by the attack and your attacker vanishes!

		attacker.soundeffect(0x482)
		attacker.pos.effect(0x376A, 8, 12)
		defender.effect(0x37be, 20, 25)

		if attacker.war:
			attacker.war = False
			if attacker.socket:
				attacker.socket.resendplayer()
		attacker.attacktarget = None		
		attacker.removefromview()
		attacker.hidden = True
		attacker.update()

ShadowStrike()

#
# WhirlwindAttack
#
class WhirlwindAttack(BaseAbility):
	def __init__(self):
		BaseAbility.__init__(self, 13)
		self.mana = 15
		self.primary = ['13fa', '13fb', '1438', '1439', '143f', '143e', 'df0', 'df1']
		self.secondary = ['f4c', 'f4b']

	def hit(self, attacker, defender, damage):
		if not self.checkuse(attacker):
			return

		self.use(attacker)

		if defender.dead or defender.pos.map == 0xFF:
			return # Out of reach		

		attacker.effect(0x3728, 10, 15)
		attacker.soundeffect(0x2a1)

		# Compile a list of targets
		targets = []
		center = attacker.pos
		chariter = wolfpack.charregion(center.x - 1, center.y - 1, center.x + 1, center.y + 1, center.map)
		char = chariter.first
		while char:
			if char != attacker and char != defender:
				targets.append(char)
			char = chariter.next

		# Our own party
		party = attacker.party
		guild = attacker.guild
		weapon = attacker.getweapon()
		
		for target in targets:
			if not attacker.canreach(target, 1):
				continue

			if not target.dead and not target.invulnerable and not target.region.safe and (not party or party != target.party) and (not guild or target.guild != guild):
				if attacker.socket:
					attacker.socket.clilocmessage(1060161) # The whirling attack strikes a target!
				if defender.socket:
					defender.socket.clilocmessage(1060162) # You are struck by the whirling attack and take damage!
				combat.aos.hit(attacker, target, weapon, wolfpack.time.currenttime())

WhirlwindAttack()

#
# 14: RidingSwipe
#
class RidingSwipe(BaseAbility):
	def __init__(self):
		BaseAbility.__init__(self, 14)
		self.mana = 25
		self.primary = []
		self.secondary = []

	def hit(self, attacker, defender, damage):
		if not self.checkuse(attacker):
			return
		self.use(attacker)
		if defender.dead or defender.pos.map == 0xFF:
			return
		# Dismount mounted opponent
		if defender.itemonlayer(LAYER_MOUNT):
			mount = defender.itemonlayer(LAYER_MOUNT)
			wolfpack.utilities.tobackpack(mount, defender)
			if defender.socket:
				defender.socket.clilocmessage(1040023) # You have been unhorsed!
			if attacker.socket:
				attacker.socket.clilocmessage(1040024) # You unhorse your target!
		attacker.soundeffect(0x524)
		defender.effect(0x3728, 10, 15)

RidingSwipe()

#
# 15: FrenziedWhirlwind
#
class FrenziedWhirlwind(BaseAbility):
	def __init__(self):
		BaseAbility.__init__(self, 15)
		self.mana = 20
		self.primary = []
		self.secondary = []

	def hit(self, attacker, defender, damage):
		if not self.checkuse(attacker):
			return
		self.use(attacker)
		if defender.dead or defender.pos.map == 0xFF:
			return
		attacker.effect(0x3728, 10, 15)
		attacker.soundeffect(0x2a1)
		# Hit area and apply bleed/whirlwind
		center = attacker.pos
		chariter = wolfpack.charregion(center.x - 2, center.y - 2, center.x + 2, center.y + 2, center.map)
		char = chariter.first
		weapon = attacker.getweapon()
		while char:
			if char != attacker and not char.dead and not char.invulnerable and not char.region.safe:
				if attacker.canreach(char, 2):
					combat.aos.hit(attacker, char, weapon, wolfpack.time.currenttime())
			char = chariter.next

FrenziedWhirlwind()

#
# 16: Block
#
class Block(BaseAbility):
	def __init__(self):
		BaseAbility.__init__(self, 16)
		self.mana = 20
		self.primary = []
		self.secondary = []

	def hit(self, attacker, defender, damage):
		if not self.checkuse(attacker):
			return
		self.use(attacker)
		attacker.settag('block_ability', 1)
		attacker.dispel(attacker, True, 'BLOCK_ABILITY')
		attacker.addtimer(6000, block_ability_expire, [], True, False, 'BLOCK_ABILITY')
		if attacker.socket:
			attacker.socket.clilocmessage(1063345) # You block an attack!
		if defender.socket:
			defender.socket.clilocmessage(1063346) # Your attack was blocked!
		attacker.effect(0x37C4, 1, 16)

def block_ability_expire(char, args):
	if char:
		char.deltag('block_ability')
		if char.socket:
			char.socket.clilocmessage(1150286) # You no longer try to block the next attack.

Block()

#
# 17: DefenseMastery
#
class DefenseMastery(BaseAbility):
	def __init__(self):
		BaseAbility.__init__(self, 17)
		self.mana = 20
		self.primary = []
		self.secondary = []

	def hit(self, attacker, defender, damage):
		if not self.checkuse(attacker):
			return
		self.use(attacker)
		attacker.settag('defense_mastery', 1)
		attacker.dispel(attacker, True, 'DEFENSE_MASTERY')
		attacker.addtimer(3000, defense_mastery_expire, [], True, False, 'DEFENSE_MASTERY')
		if attacker.socket:
			attacker.socket.clilocmessage(1063353) # You perform a masterful defense!
		attacker.effect(0x375A, 1, 17)

def defense_mastery_expire(char, args):
	if char:
		char.deltag('defense_mastery')

DefenseMastery()

#
# 18: NerveStrike
#
class NerveStrike(BaseAbility):
	def __init__(self):
		BaseAbility.__init__(self, 18)
		self.mana = 30
		self.primary = []
		self.secondary = []

	def hit(self, attacker, defender, damage):
		if not self.checkuse(attacker):
			return
		self.use(attacker)
		if defender.dead or defender.pos.map == 0xFF:
			return
		attacker.soundeffect(0x204)
		defender.effect(0x376A, 9, 32)
		# Paralyze for 2 seconds
		defender.disturb()
		defender.frozen = True
		defender.dispel(None, True, 'PARALYZE_EXPIRE')
		defender.addtimer(2000, paralyze_expire, [], True, False, 'PARALYZE_EXPIRE')
		if attacker.socket:
			attacker.socket.clilocmessage(1063356) # You cripple your target with a nerve strike!
		if defender.socket:
			defender.socket.clilocmessage(1063357) # Your attacker dealt a crippling nerve strike!

NerveStrike()

#
# 19: TalonStrike
#
class TalonStrike(BaseAbility):
	def __init__(self):
		BaseAbility.__init__(self, 19)
		self.mana = 20
		self.damage = 1.2
		self.primary = []
		self.secondary = []

	def hit(self, attacker, defender, damage):
		if not self.checkuse(attacker):
			return
		self.use(attacker)
		if defender.dead or defender.pos.map == 0xFF:
			return
		if attacker.socket:
			attacker.socket.clilocmessage(1063358) # You deliver a talon strike!
		if defender.socket:
			defender.socket.clilocmessage(1063359) # Your attacker delivers a talon strike!
		defender.effect(0x373A, 1, 17)
		# Bleed DoT ticks
		defender.settag('talon_ticks', 3)
		defender.dispel(defender, True, 'TALON_STRIKE')
		defender.addtimer(1000, talon_strike_tick, [attacker.serial], True, False, 'TALON_STRIKE')

def talon_strike_tick(defender, args):
	if not defender or not defender.hastag('talon_ticks'):
		return
	ticks = int(defender.gettag('talon_ticks')) - 1
	if defender.hitpoints > 5:
		defender.hitpoints = max(1, defender.hitpoints - 4)
		defender.updatehealth()
	if ticks > 0:
		defender.settag('talon_ticks', ticks)
		defender.addtimer(1000, talon_strike_tick, args, True, False, 'TALON_STRIKE')
	else:
		defender.deltag('talon_ticks')

TalonStrike()

#
# 20: Feint
#
class Feint(BaseAbility):
	def __init__(self):
		BaseAbility.__init__(self, 20)
		self.mana = 30
		self.primary = []
		self.secondary = []

	def hit(self, attacker, defender, damage):
		if not self.checkuse(attacker):
			return
		self.use(attacker)
		if defender.dead or defender.pos.map == 0xFF:
			return
		if attacker.socket:
			attacker.socket.clilocmessage(1063360) # You baffle your target with a feint!
		if defender.socket:
			defender.socket.clilocmessage(1063361) # You were deceived by an attacker's feint!
		attacker.effect(0x3728, 1, 13)
		attacker.soundeffect(0x525)
		attacker.settag('feint_active', 1)
		attacker.dispel(attacker, True, 'FEINT_ABILITY')
		attacker.addtimer(6000, feint_expire, [], True, False, 'FEINT_ABILITY')

def feint_expire(char, args):
	if char:
		char.deltag('feint_active')

Feint()

#
# 21: DualWield
#
class DualWield(BaseAbility):
	def __init__(self):
		BaseAbility.__init__(self, 21)
		self.mana = 20
		self.primary = []
		self.secondary = []

	def hit(self, attacker, defender, damage):
		if not self.checkuse(attacker):
			return
		self.use(attacker)
		if defender.dead or defender.pos.map == 0xFF:
			return
		if attacker.socket:
			attacker.socket.clilocmessage(1150281) # You begin trying to strike with both your weapons at once.
		attacker.effect(0x3779, 1, 15)
		attacker.soundeffect(0x524)
		weapon = attacker.getweapon()
		# Second rapid attack
		combat.aos.hit(attacker, defender, weapon, wolfpack.time.currenttime())

DualWield()

#
# 22: DoubleShot
#
class DoubleShot(BaseAbility):
	def __init__(self):
		BaseAbility.__init__(self, 22)
		self.mana = 30
		self.primary = []
		self.secondary = []

	def hit(self, attacker, defender, damage):
		if not self.checkuse(attacker):
			return
		self.use(attacker)
		if defender.dead or defender.pos.map == 0xFF:
			return
		defender.effect(0x37B9, 1, 19)
		weapon = attacker.getweapon()
		# Rapid second projectile
		combat.aos.hit(attacker, defender, weapon, wolfpack.time.currenttime())

DoubleShot()

#
# 23: ArmorPierce
#
class ArmorPierce(BaseAbility):
	def __init__(self):
		BaseAbility.__init__(self, 23)
		self.mana = 30
		self.damage = 1.5
		self.ignorephysical = True
		self.primary = []
		self.secondary = []

	def hit(self, attacker, defender, damage):
		if not self.checkuse(attacker):
			return
		self.use(attacker)
		if defender.dead or defender.pos.map == 0xFF:
			return
		if attacker.socket:
			attacker.socket.clilocmessage(1063350) # You pierce your opponent's armor!
		if defender.socket:
			defender.socket.clilocmessage(1063351) # Your attacker pierced your armor!
		defender.soundeffect(0x28E)
		defender.effect(0x3728, 1, 26)

ArmorPierce()

#
# 24: Bladeweave
#
class Bladeweave(BaseAbility):
	def __init__(self):
		BaseAbility.__init__(self, 24)
		self.mana = 15
		self.primary = []
		self.secondary = []

	def hit(self, attacker, defender, damage):
		if not self.checkuse(attacker):
			return
		self.use(attacker)
		if defender.dead or defender.pos.map == 0xFF:
			return
		attacker.soundeffect(0x510)
		defender.effect(0x37B9, 1, 5)

Bladeweave()

#
# 25: ForceArrow
#
class ForceArrow(BaseAbility):
	def __init__(self):
		BaseAbility.__init__(self, 25)
		self.mana = 20
		self.primary = []
		self.secondary = []

	def hit(self, attacker, defender, damage):
		if not self.checkuse(attacker):
			return
		self.use(attacker)
		if defender.dead or defender.pos.map == 0xFF:
			return
		defender.effect(0x376A, 1, 15)
		defender.soundeffect(0x213)

ForceArrow()

#
# 26: LightningArrow
#
class LightningArrow(BaseAbility):
	def __init__(self):
		BaseAbility.__init__(self, 26)
		self.mana = 20
		self.primary = []
		self.secondary = []

	def hit(self, attacker, defender, damage):
		if not self.checkuse(attacker):
			return
		self.use(attacker)
		if defender.dead or defender.pos.map == 0xFF:
			return
		defender.effect(0x3818, 1, 11)
		defender.soundeffect(0x51D)

LightningArrow()

#
# 27: PsychicAttack
#
class PsychicAttack(BaseAbility):
	def __init__(self):
		BaseAbility.__init__(self, 27)
		self.mana = 25
		self.primary = []
		self.secondary = []

	def hit(self, attacker, defender, damage):
		if not self.checkuse(attacker):
			return
		self.use(attacker)
		if defender.dead or defender.pos.map == 0xFF:
			return
		defender.effect(0x374A, 1, 15)
		defender.soundeffect(0x1F8)

PsychicAttack()

#
# 28: SerpentArrow
#
class SerpentArrow(BaseAbility):
	def __init__(self):
		BaseAbility.__init__(self, 28)
		self.mana = 25
		self.primary = []
		self.secondary = []

	def hit(self, attacker, defender, damage):
		if not self.checkuse(attacker):
			return
		self.use(attacker)
		if defender.dead or defender.pos.map == 0xFF:
			return
		defender.effect(0x374A, 1, 15)
		system.poison.poison(defender, 1)

SerpentArrow()

#
# 29: ForceOfNature
#
class ForceOfNature(BaseAbility):
	def __init__(self):
		BaseAbility.__init__(self, 29)
		self.mana = 35
		self.damage = 1.35
		self.primary = []
		self.secondary = []

	def hit(self, attacker, defender, damage):
		if not self.checkuse(attacker):
			return
		self.use(attacker)
		if defender.dead or defender.pos.map == 0xFF:
			return
		attacker.effect(0x3709, 1, 20)
		defender.soundeffect(0x208)

ForceOfNature()

#
# 30: InfusedThrow
#
class InfusedThrow(BaseAbility):
	def __init__(self):
		BaseAbility.__init__(self, 30)
		self.mana = 25
		self.primary = []
		self.secondary = []

	def hit(self, attacker, defender, damage):
		if not self.checkuse(attacker):
			return
		self.use(attacker)
		if defender.dead or defender.pos.map == 0xFF:
			return
		defender.effect(0x36B0, 1, 15)
		defender.soundeffect(0x11D)

InfusedThrow()

#
# 31: MysticArc
#
class MysticArc(BaseAbility):
	def __init__(self):
		BaseAbility.__init__(self, 31)
		self.mana = 20
		self.primary = []
		self.secondary = []

	def hit(self, attacker, defender, damage):
		if not self.checkuse(attacker):
			return
		self.use(attacker)
		if defender.dead or defender.pos.map == 0xFF:
			return
		defender.effect(0x3709, 1, 15)
		defender.soundeffect(0x208)

MysticArc()

#
# 32: Disrobe
#
class Disrobe(BaseAbility):
	def __init__(self):
		BaseAbility.__init__(self, 32)
		self.mana = 20
		self.primary = []
		self.secondary = []

	def hit(self, attacker, defender, damage):
		if not self.checkuse(attacker):
			return
		self.use(attacker)
		if defender.dead or defender.pos.map == 0xFF:
			return
		# Unequip defender's chest/tunic to backpack
		chest = defender.itemonlayer(LAYER_CHEST)
		if chest:
			wolfpack.utilities.tobackpack(chest, defender)
		defender.soundeffect(0x248)

Disrobe()

#
# 33: ColdWind
#
class ColdWind(BaseAbility):
	def __init__(self):
		BaseAbility.__init__(self, 33)
		self.mana = 20
		self.primary = []
		self.secondary = []

	def hit(self, attacker, defender, damage):
		if not self.checkuse(attacker):
			return
		self.use(attacker)
		if defender.dead or defender.pos.map == 0xFF:
			return
		defender.effect(0x375A, 1, 15)
		defender.soundeffect(0x10B)

ColdWind()

