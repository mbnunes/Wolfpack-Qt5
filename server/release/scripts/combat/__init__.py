
import wolfpack
from wolfpack.consts import *
import combat.properties
import combat.aos

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
  attacker.log(LOG_PYTHON, "Swing at " + str(time) + "\n")

  if AGEOFSHADOWS:
    weapon = attacker.getweapon()

    try:      
      combat.utilities.playswinganimation(attacker, defender, weapon)
  
      if combat.aos.checkhit(attacker, defender, time):
        combat.aos.hit(attacker, defender, weapon, time)
      else:
        combat.aos.miss(attacker, defender, weapon, time)
    except:
      # Try again in 10 seconds
      attacker.nextswing = time + 10000
      raise

    attacker.nextswing = time + combat.properties.getdelay(attacker, weapon)
    attacker.log(LOG_PYTHON, "Next swing in " + str(attacker.nextswing - time) + "ms\n")

#
# Callback for showing the status gump to yourself.
# We need to insert the AOS resistances here.
#
def onShowStatus(char, packet):
  properties = combat.properties.fromchar(char)

  # Get weapon properties if applicable
  weapon_properties = combat.properties.fromitem(char.getweapon())

  # Increase the damage by the bonus
  weapon_properties[MINDAMAGE] *= properties[DAMAGEBONUS] / 100
  weapon_properties[MAXDAMAGE] *= properties[DAMAGEBONUS] / 100

  packet.setshort(62, properties[RESISTANCE_PHYSICAL]) # Physical resistance
  packet.setshort(70, properties[RESISTANCE_FIRE]) # Fire Resistance
  packet.setshort(72, properties[RESISTANCE_COLD]) # Cold Resistance
  packet.setshort(74, properties[RESISTANCE_POISON]) # Poison Resistance
  packet.setshort(76, properties[RESISTANCE_ENERGY]) # Energy Resistance
  packet.setshort(78, 0) # Luck
  packet.setshort(80, weapon_properties[MINDAMAGE]) # Min. Damage
  packet.setshort(82, weapon_properties[MAXDAMAGE]) # Max. Damage
