
#
# This scripts controls bleeding invoked by the bleeding attack
#

import random
import wolfpack
from wolfpack.consts import *

TIMERID = 'BLEEDING_WOUNDS'
TIMERINTERVAL = 2000
BLOOD = ['122a', '122b', '122c', '122d', '122e'] # List of blood ids

#
# Start the bleeding process
#
def start(player, source):	
	player.dispel(None, True, TIMERID) # Dispel a previous bleeding wound
	player.settag('bleeding', 1)
	player.addtimer(TIMERINTERVAL, "system.bleeding.stroke", [source.serial, 5], False, False, TIMERID)

#
# Checks if the given character is bleeding.
#
def isbleeding(player):
	return player.hastag('bleeding')

#
# End the bleeding process
#
def end(player, message = True):
	player.dispel(None, True, TIMERID)
	player.deltag('bleeding')
	if message and player.socket:
		player.socket.clilocmessage(1060167) # Your wounds have healed...

#
# A stroke of the bleeding process
#
def stroke(player, arguments):
	(source, count) = arguments

	if player.dead:
		end(player, False) # End bleeding if dead, but dont message
	else:
		amount = random.randint(count, count * 2)
		if player.npc:
			amount *= 2

		player.soundeffect(0x133)
		player.damage(DAMAGE_GODLY, amount, wolfpack.findchar(source)) 

		blood = wolfpack.additem(random.choice(BLOOD))
		blood.movable = 3 # Not movable
		blood.decay = True # Make it decay
		blood.moveto(player.pos)
		blood.update()

	# There are still strokes left	
	if count > 1:
		player.addtimer(TIMERINTERVAL, "system.bleeding.stroke", [source, count - 1], False, False, TIMERID)
	else:
		end(player)
