#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Created by: Radiant                        #
#   )).-' {{ ;'`   | Revised by:                                #
#  ( (  ;._ \\ ctr | Last Modification: Created                 #
#===============================================================#
# Necromancy Spells                                             #
#===============================================================#

import wolfpack
from wolfpack.consts import *
from magic.utilities import *
from magic.necromancy import NecroSpell

def onLoad():
    AnimateDead().register(101)

class AnimateDead(NecroSpell):
    def __init__(self):
        NecroSpell.__init__(self)
        #self.reagents = {REAGENT_DAEMONBLOOD: 1, REAGENT_GRAVEDUST: 1}
        self.mantra = 'Uus Corp'
        self.validtarget = TARGET_ITEM
        self.circle = 4

    def target(self, char, mode, targettype, target, args, item):
        char.turnto(target)

        if target.id != 0x2006:
            char.socket.clilocmessage(1042600)
            return

        if char.player and char.controlslots >= 5:
			char.socket.clilocmessage(1049645)
			return

        if target.owner:
            char.socket.sysmessage('You may not use this on players!')
            return

        if not self.consumerequirements(char, mode, args, target, item):
            return

        char.socket.sysmessage('Not yet implented')

        # Need baseid prop on corpses for this!
        #animate = wolfpack.addnpc(target.bodyid, target.pos)
        #animate.controlslots = 1
        #animate.owner = char
        #animate.summontime = wolfpack.time.currenttime() + 120000
        #animate.summoned = 1
        #item.remove()