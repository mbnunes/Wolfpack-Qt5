
from wolfpack.consts import *

#
# Weapon Information
# This file includes all known standard weapon information.
# The baseid of the item is used as the key for WEAPONINFO.
# For unspecified entries the default value will be used.
#
WEAPONINFO = {
  # Demonstration entry
  #'baseid': {
  # MINDAMAGE: The minimum damage this weapon deals
  # MAXDAMAGE: The maximum damage this weapon deals
  # HITSOUND: A list of soundeffects played for this weapon if it hits
  # MISSSOUND: A list of soundeffects played for this weapon if it misses
  # SPEED: The weapon speed
  # AMMUNITION: The BASE ID of the ammunition to use for this weapon.
  # PROJECTILE: The item id of the projectile to throw at the defender if shot.
  # PROJECTILEHUE: The hue of the projectile this weapon shoots
  #}

  # Katana
  '13ff': {
    MINDAMAGE: 11,
    MAXDAMAGE: 13,
    SPEED: 46,
    HITSOUND: [0x23b, 0x23c]
  },

  # Bow
  '13b1': {
    MINDAMAGE: 16,
    MAXDAMAGE: 18,
    SPEED: 25,
    PROJECTILE: 0xF42,
    AMMUNITION: 'f3f',
  },
}
