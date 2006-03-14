
import wolfpack
import random
from wolfpack.consts import ANIM_FIDGET3
from wolfpack.utilities import tobackpack
from system import poison
import skills
from wolfpack import tr
from math import ceil
from speech.pets import isPetFriend

farm_food = [  'c7c', 'c70', 'c7b', 'c78', 'c71', 'c64', 'c65' ]
farm_eaters = [ 'rabbit', 'goat', 'hind', 'pack_horse', 'pack_llama', 'cow', 'bull',
	'sheep_unsheered', 'sheep_sheered', 'llama', 'horse', 'great_hart',
	'ostard_desert', 'ostard_forest', 'ostard_frinzied' ]

# List of baseids of items only animals can eat (not complete, because not all foods have the script right now)
animalsonly = [ 'f36', '100c', '100d', # Hay
		'9f1', '1609', '1607', # Raw Meat
		'97a', # Raw Fish
		'eed' # Gold
		]

#
# TODO: favorite food of animals:
# FruitsAndVegies | GrainsAndHay | Meat | Eggs | Gold | Fish
# intproperties: 1=meat; 2=fish; 3=FruitAndVegies; 4=Eggs; 5=GrainsAndHay; 6=Gold; 7=Metal
#
types = {
	'1' : ['979', '9b7', '9c0', '9c9', '9f2', '160a', '1608', '9f1', '1609', '1607', '9b9', ], # missing: body parts
	'2' : ['97b', '97a'],
	'3' : ['c74', 'c75', 'c64', 'c65', 'c66', 'c67', '171f', '1720', '1721', '1722', '1728', '172a', '1727', \
	'9d1', '9d2', '994', '9d0', 'c5c', 'c5d', 'c72', 'c73', 'c79', 'c7a', 'c77', 'c78', 'c7b', 'c7c', 'c6d', \
	'c6e', 'c70', 'c71', 'c6a', 'c6b' ],
	'4' : ['9b6', '9b5'],
	'5' : ['103b', '98c', "f36", "100c", "100d"],
	'6' : ['eed']
	}

def checkfoodpreference(char, item):
	favfoodtypes = char.getstrproperty("food", "0")
	favfoodtypes = favfoodtypes.split(",")
	#char.say(str(favfoodtypes))
	if len(favfoodtypes) == 1 and '0' in favfoodtypes:
		return False
	for i in favfoodtypes:
		allowed = types.get(i, [])
		if item.baseid in allowed:
			return True
	return False

#
# Feed the food
#
def onDropOnChar(char, item):
	# See if it's not hostile
	if char.npc and char.id < 0x190:
		player = item.container
		if char.tamed and (char.owner == player) or isPetFriend(player, char) :
			if checkfoodpreference(char, item):
				ischecked(player, char, item)
		else:
			return False
		return True

def ischecked(player, char, item):
	amount = item.amount
	if amount > 0:
		happier = False
		stamGain = 0
		if item.baseid == 'eed':
			stamGain = amount - 50
		else:
			stamGain = (amount * 15) - 50
		if stamGain > 0:
			char.stamina += stamGain
		for i in range(0, amount):
			loyalty = 0
			if char.hastag('loyalty'):
				loyalty = char.gettag('loyalty')
			if not loyalty:
				char.settag('loyalty', 11)
			# if loyalty < Wonderfully happy
			elif loyalty < 11 and 0.5 >= random.random():
				char.settag('loyalty', loyalty + 1)
				happier = True
		if happier:
			char.say(502060, '', '', 0, char.saycolor, player.socket) # Your pet looks happier.

		if char.bodytype == 3: # is animal
			char.action( 3 )
		elif char.bodytype == 1: # is monster
			char.action( 17 )

		#if ( IsBondable && !IsBonded )
		#{
		#	Mobile master = m_ControlMaster;

		#	if ( master != null )
		#	{
		#		if ( m_dMinTameSkill <= 29.1 || master.Skills[SkillName.AnimalTaming].Value >= m_dMinTameSkill || this is SwampDragon || this is Ridgeback || this is SavageRidgeback )
		#		{
		#			if ( BondingBegin == DateTime.MinValue )
		#			{
		#				BondingBegin = DateTime.Now;
		#			}
		#			else if ( (BondingBegin + BondingDelay) <= DateTime.Now )
		#			{
		#				IsBonded = true;
		#				BondingBegin = DateTime.MinValue;
		#				from.SendLocalizedMessage( 1049666 ); // Your pet has bonded with you!
		#			}
		#		}
		#	}
		#}

		item.delete()


		#if not char.tamed:
		#	return False
#
		#if not player.canreach(char, 2):
		#	player.socket.clilocmessage(500312) # You cannot reach that.
		#	if not tobackpack( item, player ):
		#		item.update()
		#	return True
#
		#if char.hunger >= 20:
		#	player.message( tr('It doesn''t seem to be hungry.') )
		#	if not tobackpack( item, player ):
		#		item.update()
		#	return True
#
		#fillfactor = item.getintproperty('fillfactor', 1)
#
		#complete_fillfactor = fillfactor * item.amount
#
		#if complete_fillfactor > 20 - char.hunger:
		#	requireditems = int(ceil((20 - char.hunger) / fillfactor))
		#	item.amount -= requireditems
		#	char.hunger = 20
		#	if not tobackpack(item, player):
		#		item.update()
		#		item.resendtooltip()
		#else:
		#	char.hunger += complete_fillfactor
		#	item.delete()
#
		## Fidget animation and munch munch sound
		#char.soundeffect( random.choice([0x03a, 0x03b, 0x03c]), 1 )
		#if not char.ismounted():
		#	char.action(ANIM_FIDGET3)
		#return True
#
#	return False

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
