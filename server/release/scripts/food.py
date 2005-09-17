
import wolfpack
import random
from wolfpack.consts import ANIM_FIDGET3
from wolfpack.utilities import tobackpack
from system import poison
import skills
from wolfpack import tr
from math import ceil

farm_food = [  'c7c', 'c70', 'c7b', 'c78', 'c71', 'c64', 'c65' ]
farm_eaters = [ 'rabbit', 'goat', 'hind', 'pack_horse', 'pack_llama', 'cow', 'bull',
	'sheep_unsheered', 'sheep_sheered', 'llama', 'horse', 'great_hart',
	'ostard_desert', 'ostard_forest', 'ostard_frinzied' ]

# List of baseids of items only animals can eat
animalsonly = [ "f36", "100c", "100d" ]

#
# TODO: favorite food of animals:
# FruitsAndVegies | GrainsAndHay | Meat | Eggs | Gold | Fish
#

#
# Feed the food
#
def onDropOnChar(char, item):
	# See if it's not hostile
	if char.npc and char.id < 0x190:
		player = item.container

		if not char.tamed:
			return False

		if not player.canreach(char, 2):
			player.socket.clilocmessage(500312) # You cannot reach that.
			if not tobackpack( item, player ):
				item.update()
			return True

		if char.hunger >= 20:
			player.message( tr('It doesn''t seem to be hungry.') )
			if not tobackpack( item, player ):
				item.update()
			return True

		fillfactor = item.getintproperty('fillfactor', 1)

		complete_fillfactor = fillfactor * item.amount

		if complete_fillfactor > 20 - char.hunger:
			requireditems = int(ceil((20 - char.hunger) / fillfactor))
			item.amount -= requireditems
			char.hunger = 20
			if not tobackpack(item, player):
				item.update()
				item.resendtooltip()
		else:
			char.hunger += complete_fillfactor
			item.delete()

		# Fidget animation and munch munch sound
		char.soundeffect( random.choice([0x03a, 0x03b, 0x03c]), 1 )
		if not char.ismounted():
			char.action(ANIM_FIDGET3)
		return True

	return False

def caneat(char, item):
	if item.baseid in animalsonly:
		char.socket.sysmessage(tr('You don''t want to eat that.'))
		return False
	return True

#
# Eat the food
#
def onUse(player, item):
	# Has to belong to us.
	if item.getoutmostchar() != player:
		player.socket.clilocmessage(500866) # You can't eat that, it belongs to someone else.
		return True
	elif not caneat(player, item):
		return True
	Eat( player, item )
	return True

def Eat(player, item):
	if FillHunger( player, item ):
		# Fidget animation and munch munch sound
		player.soundeffect( random.choice([0x03a, 0x03b, 0x03c]), 1 )
		if not player.ismounted():
			player.action(ANIM_FIDGET3)

		# poisoned food
		if item.hastag( 'poisoning_char' ):
			poison.poison( player, item.gettag( 'poisoning_strength' ) )
			player.socket.clilocmessage( 1010512 ) # You have been poisoned!
			skills.poisoning.wearoff( item )

		if item.amount > 1:
			item.amount -= 1
			item.update()
		else:
			item.delete()

		return True
	return False

def FillHunger(player, item):
	# Can we eat anymore?
	if player.hunger >= 20:
		player.socket.clilocmessage(500867) # You are simply too full to eat any more!
		return False

	fillfactor = item.getintproperty('fillfactor', 1)
	iHunger = player.hunger + fillfactor
	if player.stamina < player.maxstamina:
		player.stamina += random.randint( 6, 9 ) + fillfactor/5 # restore some stamina
	if iHunger >= 20:
		player.hunger = 20
		player.socket.clilocmessage( 500872 ) # You manage to eat the food, but you are stuffed!
	else:
		player.hunger = iHunger

		if iHunger < 5:
			player.socket.clilocmessage( 500868 ) # You eat the food, but are still extremely hungry.
		elif iHunger < 10:
			player.socket.clilocmessage( 500869 ) # You eat the food, and begin to feel more satiated.
		elif iHunger < 15:
			player.socket.clilocmessage( 500870 ) # After eating the food, you feel much less hungry.
		else:
			player.socket.clilocmessage( 500871 ) # You feel quite full after consuming the food.
	return True

def onCollide( char, item ):
	if char.npc and item.baseid in farm_food and char.baseid in farm_eaters:
		if char.hasscript( 'food' ):
			return True
		char.addscript( 'food' )
		char.say( tr("*nibbles*") )
		item.movable = 3
		item.update()
		return True
	else:
		return False

def onWalk(char, dir, sequence):
	if char.baseid in farm_eaters:
		items = wolfpack.items(char.pos.x, char.pos.y, char.pos.map, 0)

		food = None
		for item in items:
			if item.hasscript( 'food' ) and item.baseid in farm_food:
				food = item
				break

		if food:
			food.delete()
			char.soundeffect( random.choice( [ 0x03a, 0x03b, 0x03c ] ), 1 )
			char.say( tr("*munch*") )
			if char.hitpoints < char.maxhitpoints:
				char.hitpoints += 1
				char.update()

	char.removescript('food')
	return True
