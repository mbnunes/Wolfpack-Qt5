
import random
from wolfpack.consts import *
from wolfpack.utilities import tobackpack

#
# Feed the food
#
def onDropOnChar(char, item):
	# See if it's not hostile
	if char.npc and char.id < 0x190:
		player = item.container
		
		if not char.tamed:
			return 0
		
		if not player.canreach(char, 2):
			player.socket.clilocmessage(500312)
			if not tobackpack(item, player):
				item.update()
			return 1
			
		if char.hunger >= 6:
			player.message('They don''t seem to be hungry.')
			if not tobackpack(item, player):
				item.update()
			return 1
			
		if item.amount > 6 - char.hunger:
			item.amount -= 6 - char.hunger			
			char.hunger = 6
			if not tobackpack(item, player):
				item.update()
				item.resendtooltip()			
		else:
			char.hunger += item.amount
			item.delete()
			
		# Fidget animation and munch munch sound
		char.soundeffect(random.choice([0x03a, 0x03b, 0x03c]))
		char.action(ANIM_FIDGET3)
		return 1

	char.say('Dropped item on char')
	return 0

#
# Eat the food
#
def onUse(player, item):
	# Has to belong to us.
	if item.getoutmostchar() != player:
		player.socket.clilocmessage(500866)
		return 1

	# Can we eat anymore?
	if player.hunger >= 6:
		player.socket.clilocmessage(500867)
		return 1
		
	player.socket.clilocmessage(min(500872, 500868 + player.hunger))
	
	if item.amount > 1:
		item.amount -= 1
		item.update()
	else:
		item.delete()
		
	# Fidget animation and munch munch sound
	player.soundeffect(random.choice([0x03a, 0x03b, 0x03c]))
	player.action(ANIM_FIDGET3)
		
	player.hunger += 1
	return 1
