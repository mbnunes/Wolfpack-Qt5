
import wolfpack
import random
from wolfpack.consts import ANIM_FIDGET3
from wolfpack.utilities import tobackpack
from system import poison

farm_food = [  'c7c', 'c70', 'c7b', 'c78', 'c71', 'c64', 'c65' ]
farm_eaters = [ 'rabbit', 'goat', 'hind', 'pack_horse', 'pack_llama', 'cow', 'bull',
	'sheep_unsheered', 'sheep_sheered', 'llama', 'horse', 'great_hart',
	'ostard_desert', 'ostard_forest', 'ostard_frinzied' ]
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

	# Fidget animation and munch munch sound
	player.soundeffect( random.choice([0x03a, 0x03b, 0x03c]), 1 )
	player.action(ANIM_FIDGET3)

	player.hunger += 1

	# poisoned food
	if item.hastag( 'poisoning_char' ):
		poison.poison( player, item.gettag( 'poisoning_strength' ) )
		player.socket.clilocmessage( 1010512 )
		skills.poisoning.wearoff( item )

	if item.amount > 1:
		item.amount -= 1
		item.update()
	else:
		item.delete()

	return 1

def onCollide( char, item ):
	if char.npc and item.baseid in farm_food and char.baseid in farm_eaters:
		if 'food' in char.events:
			return True
		if char.baseid in farm_eaters:
			char.events = ['food'] + char.events
			char.say( "*nibbles*" )
			item.magic = 3
			item.update()
			return True
		else:
			return False
	else:
		return False

def onWalk(char, dir, sequence):
	if char.baseid in farm_eaters:
		items = wolfpack.items(char.pos.x, char.pos.y, char.pos.map, 0)
		for item in items:
			if 'food' in item.events and item.baseid in farm_food:
				food = item
				break
		if food:
			food.delete()
			char.soundeffect( random.choice( [ 0x03a, 0x03b, 0x03c ] ), 1 )
			char.say( "*munch*" )
			if char.hitpoints < char.maxhitpoints:
				char.hitpoints += 1
				char.update()
		events = char.events
		while 'food' in events:
			events.remove('food')
		char.events = events
		return True
	else:
		events = char.events
		while 'food' in events:
			events.remove('food')
		char.events = events
		return True
