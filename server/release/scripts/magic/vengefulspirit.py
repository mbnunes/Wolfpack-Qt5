
import wolfpack
from wolfpack.utilities import isValidPosition
import random

# The target is killed, so remove the NPC
def onDoDamage(char, type, amount, victim):
	if amount >= victim.hitpoints:
		char.kill()
	return amount

def effects(char):
	char.pos.soundeffect( 0x10B )
	char.pos.effect( 0x37CC, 1, 50, 2101, 7 )

# check our target every 2 seconds
def checktarget(char, args):
	if check(char):
		char.addtimer( 2 * 1000, checktarget, [])

def onDeath(char, source, corpse):
	effects(char)

def onDelete(char):
	effects(char)

def check(char):
	target_serial = char.gettag('target')
	target = wolfpack.findchar(target_serial)
	if not target or target.dead:
		char.kill()
		return False
	elif (char.pos.map != target.pos.map) or char.distanceto(target) > 15:
		for i in range(0, 5):
			loc = wolfpack.coord(target.pos.x - 4 + random.randint(0, 9), target.pos.y - 4 + random.randint(0, 9), target.pos.z, target.pos.map)
			if isValidPosition( loc ):
				break

		char.pos.effect( 0x3728, 1, 13, 37, 7 )
		char.moveto( loc )
		char.removefromview()
		char.update()
		char.effect( 0x3728, 1, 13, 37, 7 )
		char.soundeffect( 0x37D )
	return True
