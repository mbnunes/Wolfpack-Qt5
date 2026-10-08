#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: Naddel                         #
#   )).-' {{ ;'`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification: Created                 #
#################################################################

import wolfpack
from wolfpack.utilities import hasHandFree
import random
from wolfpack.consts import NINJITSU
from system.poison import POISONS, poison

maxUses = 10


def onUse(char, item):
	if not item.layer > 0:
		return
	if item.hastag('remaining_uses') and item.gettag('remaining_uses') < 1:
		char.socket.clilocmessage( 1063297 ) # You have no shuriken in your ninja belt!
	elif item.hastag('using'):
		char.socket.clilocmessage( 1063298 ) # You cannot throw another shuriken yet.
	elif not hasHandFree(char, item):
		char.socket.clilocmessage( 1063299 ) # You must have a free hand to throw shuriken.
	else:
		char.socket.attachtarget( 'ninja_belt.callback', [item.serial] )
	return True

def callback(player, arguments, target):
	belt = wolfpack.finditem(arguments[0])
	if not belt or belt.layer == 0:
		return
	
	if target.char:
		Shoot(player, belt, target.char)
	elif target.item and target.item.hasscript('shuriken'):
		Reload( player, belt, target.item )
	else:
		player.socket.clilocmessage( 1063301 ) # You can only place shuriken in a ninja belt.

# Shoot on a target char
def Shoot(player, belt, target):
	if player == target:
		return
	if target.invulnerable or target.gm:
		return
	elif belt.hastag('remaining_uses') and belt.gettag('remaining_uses') < 1:
		player.socket.clilocmessage( 1063297 ) # You have no shuriken in your ninja belt!
	elif belt.hastag('using'):
		player.socket.clilocmessage( 1063298 ) # You cannot throw another shuriken yet.
	elif not hasHandFree(player, belt):
		player.socket.clilocmessage( 1063299 ) # You must have a free hand to throw shuriken.
	elif not player.distanceto(target) > 2:
		player.socket.clilocmessage( 1063303 ) # Your target is too close!
	else:
		belt.settag('using', 1)
		player.turnto(target)
		player.reveal()
		player.action(9) # 26 if mounted!
		player.soundeffect(0x23A)

		player.movingeffect(0x27AC, target, False, False, 1)

		if player.checkskill(NINJITSU, -100, 650):
			target.addtimer(1000, onShurikenHit, [player.serial, belt.serial])
		else:
			ConsumeUse(belt)

		belt.addtimer(2500, ResetUsing, [])

# Handle hit
def onShurikenHit(target, args):
	player = wolfpack.findchar(args[0])
	belt = wolfpack.finditem(args[1])
	if not player or not belt:
		return

	target.damage(0, random.randint(3, 5), player)

	poison_lvl = -1
	poisoning_uses = 0
	if belt.hastag('poisoning_strength'):
		poison_lvl = belt.gettag('poisoning_strength')
	if belt.hastag('poisoning_uses'):
		poisoning_uses = belt.gettag('poisoning_uses')

	if poison_lvl != -1 and poisoning_uses > 0:
		poison(target, poison_lvl)

	ConsumeUse(belt)

# Belt can be used again
def ResetUsing(belt, args):
	belt.deltag('using')

# Consume a shuriken from the belt
def ConsumeUse(belt):
	remaining_uses = 0
	if belt.hastag('remaining_uses'):
		remaining_uses = belt.gettag('remaining_uses')
	if remaining_uses < 1:
		return

	belt.settag('remaining_uses', remaining_uses - 1)

	# Consume poison charges
	if belt.hastag('poisoning_uses') and belt.gettag('poisoning_uses') > 0:
		belt.settag('poisoning_uses', belt.gettag('poisoning_uses') - 1)
		if belt.gettag('poisoning_uses') == 0:
			belt.settag('poisoning_strength', -1)

	belt.resendtooltip()

# Unload the belt
def Unload(player, belt):
	belt_rem_uses = 0
	belt_poison = -1
	belt_pC = 0

	if belt.hastag('poisoning_strength'):
		belt_poison = belt.gettag('poisoning_strength')
	if belt.hastag('poisoning_uses'):
		belt_pC = belt.gettag('poisoning_uses')

	if belt.hastag('remaining_uses'):
		belt_rem_uses = belt.gettag('remaining_uses')

	if belt_rem_uses < 1:
		return

	backpack = player.getbackpack()
	shuriken = wolfpack.additem('27ac')
	shuriken.settag('poisoning_strength', belt_poison )
	shuriken.settag('poisoning_uses', belt_pC )
	shuriken.amount = belt_rem_uses
	shuriken.settag('remaining_uses', belt_rem_uses)
	backpack.additem( shuriken )
	shuriken.update()

	belt.settag('remaining_uses', 0)
	belt.settag('poisoning_uses', 0)
	belt.settag('poisoning_strength', -1)

	belt.resendtooltip()


# Reload over ContextMenu
def TargetReload(player, arguments, target):
	belt = wolfpack.finditem(arguments[0])
	if not belt:
		return

	if target.item and target.item.hasscript('shuriken'):
		Reload( player, belt, target.item )

# Reload the belt
def Reload(player, belt, shuriken):
	belt_rem_uses = shuriken_rem_uses = 0

	shuriken_poison = belt_poison = -1
	shuriken_poisonUses = belt_poisonUses = 0

	if belt.hastag('remaining_uses'):
		belt_rem_uses = belt.gettag('remaining_uses')

	if shuriken.hastag('remaining_uses'):
		shuriken_rem_uses = shuriken.gettag('remaining_uses')

	need = maxUses - belt_rem_uses

	if need <= 0:
		player.socket.clilocmessage( 1063302 ) # You cannot add any more shuriken.
	elif shuriken_rem_uses > 0:
		if need > shuriken_rem_uses:
			need = shuriken_rem_uses

		if shuriken.hastag('poisoning_strength'):
			shuriken_poison = shuriken.gettag('poisoning_strength')
		if shuriken.hastag('poisoning_uses'):
			shuriken_poisonUses = shuriken.gettag('poisoning_uses')
		if belt.hastag('poisoning_uses'):
			belt_poisonUses = belt.gettag('poisoning_uses')

		if shuriken_poison != -1 and shuriken_poisonUses > 0:
			if belt_poisonUses <= 0 or belt_poison == shuriken_poison:
				if belt_poisonUses != 0 and belt_poison < shuriken_poison:
					Unload( player, belt )
				if need > shuriken_poisonUses:
					need = shuriken_poisonUses

				if belt_poison == -1 or belt_poisonUses <= 0:
					belt_poisonUses = need
				else:
					belt_poisonUses += need

				belt_poison = shuriken_poison

				shuriken_poisonUses -= need;

				if shuriken_poisonUses <= 0:
					shuriken_poison = -1
				
				belt.settag('poisoning_strength', belt_poison)
				belt.settag('poisoning_uses', belt_poisonUses)
				
				shuriken.settag('poisoning_strength', shuriken_poison)
				shuriken.settag('poisoning_uses', shuriken_poisonUses)

				belt.settag('remaining_uses', belt_rem_uses + need)
				shuriken.settag('remaining_uses', shuriken_rem_uses - need)

			else:
				player.socket.clilocmessage( 1070767 ) # Loaded projectile is stronger, unload it first
		else:
			belt.settag('remaining_uses', belt_rem_uses + need)
			shuriken.settag('remaining_uses', shuriken_rem_uses - need)

		if shuriken.gettag('remaining_uses') <= 0:
			shuriken.delete()

		shuriken.resendtooltip()
		belt.resendtooltip()

######################################################################################
#############   Tool Tip   ###########################################################
######################################################################################

def onShowTooltip(viewer, object, tooltip):
	tooltip.add(1060584, str(object.gettag('remaining_uses')))

	belt_poison = -1
	belt_poisonUses = 0
	if object.hastag('poisoning_strength'):
		belt_poison = object.gettag('poisoning_strength')
	if object.hastag('poisoning_uses'):
		belt_poisonUses = object.gettag('poisoning_uses')
		
	if belt_poison != -1 and belt_poisonUses > 0:
		tooltip.add( 1062412 + belt_poison, str(belt_poisonUses) )

######################################################################################
#############   Context Menu   #######################################################
######################################################################################

def onContextCheckEnabled(player, object, tag):
	return object.gettag('remaining_uses') > 0

def onContextEntry(player, object, entry):
	if entry == 1:
		player.socket.attachtarget( 'ninja_belt.TargetReload', [object.serial] )
	elif entry == 2:
		Unload( player, object )

def onEquip(char, item, layer):
	if char.socket:
		char.socket.clilocmessage( 1070785 ) # Double click this item each time you wish to throw a shuriken.
