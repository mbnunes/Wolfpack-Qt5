
import wolfpack
from math import sqrt
import random

def boom_timer(object, args):
	(x, y, map) = args
	
	z = wolfpack.map(x, y, map)['z']
	
	pos = wolfpack.coord(x, y, z, map)
		
	if 0.5 >= random.random():
		t = random.randint(0, 2)
		
		# Fire Column
		if t == 0:
			pos.effect(0x3709, 10, 30)
			pos.soundeffect(0x208)
		
		# Explosion
		elif t == 1:
			pos.effect(0x36bd, 20, 10)
			pos.soundeffect(0x307)
		
		# Ball of fire
		elif t == 2:
			pos.effect(0x36fe, 10, 10)

# Go boom
def boom(socket, command, arguments):
	pos = socket.player.pos
	
	# Go boom
	for x in range(-12, 13):
		for y in range(-12, 13):
			dist = sqrt(x*x+y*y)
			
			if dist <= 12:
				delay = int(random.random() * 10000)
				wolfpack.addtimer(delay, 'commands.boom.boom_timer', [pos.x + x, pos.y + y, pos.map])

def onLoad():
	wolfpack.registercommand('boom', boom)
