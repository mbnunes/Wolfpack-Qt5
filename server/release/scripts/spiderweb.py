
import wolfpack
from wolfpack.consts import *
from math import ceil

#
# Tie the colliding character to the new until he breaks it.
#
def onCollide(char, item):
	# Ignore if the character already has the event
	if 'spiderweb' in char.events:
		return
		
	char.events = ['spiderweb'] + char.events
	if char.socket:
		char.socket.sysmessage('You are entangled in the spiderweb. You have to break free!')
		
#
# If we're still in a spiderweb,
# find it and try to break it.
# If it's broken, remove the event from us.
#
def onWalk(char, dir, sequence):
	# Find the spiderweb
	items = wolfpack.items(char.pos.x, char.pos.y, char.pos.map, 0)
	
	spiderweb = None
	
	for item in items:
		if 'spiderweb' in item.events:
			spiderweb = item
			break
			
	if spiderweb:
		# Damage the web until it disappears
		spiderweb.health = max(0, spiderweb.health - ceil(char.strength / 2.0))
		
		if spiderweb.health == 0:
			spiderweb.delete()
		else:	
			if char.socket:
				char.socket.sysmessage('You damage the spiderweb.')	
				packet = wolfpack.packet(0x21, 8)
				packet.setbyte(1, sequence)
				packet.setshort(2, char.pos.x)
				packet.setshort(4, char.pos.y)
				packet.setbyte(6, char.direction)
				packet.setbyte(7, char.pos.z)
				packet.send(char.socket)
				char.socket.walksequence = 0
			return 1
	
	events = char.events
	while 'spiderweb' in events:
		events.remove('spiderweb')
	char.events = events
	
	char.socket.sysmessage('You manage to break free of the spiderweb.')
	return 0

#
# Pass the damage on to the spiderweb * 4 if physical damage
# and *2 if magic damage.
#
def onDamage(char, type, amount, source):
	# Find the spiderweb
	items = wolfpack.items(char.pos.x, char.pos.y, char.pos.map, 0)
	
	spiderweb = None
	
	for item in items:
		if 'spiderweb' in item.events:
			spiderweb = item
			break

	if spiderweb:
		if type == DAMAGE_MAGICAL:
			spiderweb.health = max(0, spiderweb.health - amount * 2)
		elif type == DAMAGE_PHYSICAL:
			spiderweb.health = max(0, spiderweb.health - amount * 4)
		
		if spiderweb.health == 0:
			spiderweb.delete()
			
			targets = wolfpack.chars(char.pos.x, char.pos.y, char.pos.map, 0)
			
			for target in targets:	
				events = target.events
				while 'spiderweb' in events:
					events.remove('spiderweb')
				target.events = events

	return amount
