
import wolfpack
from wolfpack.consts import *
import combat.properties
import combat.aos

DEBUG = 0

#
# This function is used to initialize the basic combat
# logic and register some events with the server.
#
def onLoad():
	wolfpack.registerglobal(EVENT_SWING, "combat")
	wolfpack.registerglobal(EVENT_SHOWSTATUS, "combat")

#
# This is called when the character swings at his combat target.
# We do all the combat stuff here and set a new timeout until
# the next hit can be done.
#
def onSwing(attacker, defender, time):
	# We won't allow any swings from or against players
	# who are offline
	if defender.player and not defender.socket and not defender.logouttime:
		attacker.attacktarget = 0
		return

	if attacker.player and not attacker.socket and not attacker.logouttime:
		attacker.attacktarget = 0
		return

	# Let the defender strike back if he doesnt fight anyone right now
	if not defender.attacktarget:
		defender.fight(attacker)

	if DEBUG:
		attacker.log(LOG_PYTHON, "Swing from 0x%x at 0x%x\n" % (defender.serial, attacker.serial))

	if AGEOFSHADOWS:
		weapon = attacker.getweapon()

		try:
			# Ranged weapons need shooting first
			if weapon and (weapon.type == 1007 or weapon.type == 1006):
				# We have to be standing for >= 1000 ms, otherwise try again later
				if attacker.lastmovement + 1000 > wolfpack.currenttime():
					attacker.nextswing = attacker.lastmovement + 1000
					return

				# See if we can fire the weapon.
				# if not, wait until the next normal swing.
				if combat.aos.fireweapon(attacker, defender, weapon):
					if combat.aos.checkhit(attacker, defender, time):
						combat.aos.hit(attacker, defender, weapon, time)
					else:
						combat.aos.miss(attacker, defender, weapon, time)
			elif combat.aos.checkhit(attacker, defender, time):
					combat.aos.hit(attacker, defender, weapon, time)
			else:
					combat.aos.miss(attacker, defender, weapon, time)

		except:
			# Try again in 10 seconds
			attacker.nextswing = time + 10000
			raise

		attacker.nextswing = time + combat.properties.getdelay(attacker, weapon)

#
# Callback for showing the status gump to yourself.
# We need to insert the AOS resistances here.
#
def onShowStatus(char, packet):
	damagebonus = combat.properties.fromchar(char, DAMAGEBONUS)

	# Get weapon properties if applicable
	(mindamage, maxdamage) = combat.properties.getdamage(char)

	# Scale damage
	mindamage = int(combat.aos.scaledamage(char, mindamage, 0))
	maxdamage = int(combat.aos.scaledamage(char, maxdamage, 0))

	packet.setshort(62, combat.properties.fromchar(char, RESISTANCE_PHYSICAL)) # Physical resistance
	packet.setshort(70, combat.properties.fromchar(char, RESISTANCE_FIRE)) # Fire Resistance
	packet.setshort(72, combat.properties.fromchar(char, RESISTANCE_COLD)) # Cold Resistance
	packet.setshort(74, combat.properties.fromchar(char, RESISTANCE_POISON)) # Poison Resistance
	packet.setshort(76, combat.properties.fromchar(char, RESISTANCE_ENERGY)) # Energy Resistance
	packet.setshort(78, 0) # Luck
	packet.setshort(80, mindamage) # Min. Damage
	packet.setshort(82, maxdamage) # Max. Damage
