#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Created by: Radiant                        #
#   )).-' {{ ;'`   | Revised by:                                #
#  ( (  ;._ \\ ctr | Last Modification: Created                 #
#===============================================================#
# Necromancy basic stuff                                        #
#===============================================================#

from magic.spell import Spell
from wolfpack.consts import *
from magic.utilities import *

class NecroSpell(Spell):
    def __init__(self):
        Spell.__init__(self, 0)
        self.skill = NECROMANCY
        self.damageskill = SPIRITSPEAK
        self.mana = 0
        