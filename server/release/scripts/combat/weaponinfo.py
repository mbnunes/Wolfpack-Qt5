
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
  # HITSOUND: [], # A random list of soundeffects played for this weapon if it hits
  # MISSSOUND: [], # A list of soundeffects played for this weapon if it misses
  #}

  # Katana
  '13ff': {
    MINDAMAGE: 11,
    MAXDAMAGE: 13,
    SPEED: 46,
    HITSOUND: [0x23b, 0x23c],
  }
}
