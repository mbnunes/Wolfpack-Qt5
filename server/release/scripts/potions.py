import wolfpack
from random import randint, random
from wolfpack import time
from wolfpack.consts import *
from wolfpack.utilities import hex2dec, throwobject, energydamage
import math
from system import poison

# potion [ return_bottle, aggressive, target, name ]
potions = \
{
	0:		[ 1, 0, 0, '#1044542', '#1041620', 'potion_nightsight' ], # nightsight
	1:		[ 1, 0, 0, '#1044543', '#1041634', 'potion_lesserheal' ], # lesser heal
	2:		[ 1, 0, 0, '#1044544', '#1041635', 'potion_heal' ], # heal
	3:		[ 1, 0, 0, '#1044545', '#1041636', 'potion_greaterheal' ], # greater heal
	4:		[ 1, 0, 0, '#1044552', '#1041621', 'potion_lessercure' ], # lesser cure
	5:		[ 1, 0, 0, '#1044553', '#1041622', 'potion_cure' ], # cure
	6:		[ 1, 0, 0, '#1044554', '#1041623', 'potion_greatercure' ], # greater cure
	7:		[ 1, 0, 0, '#1044540', '#1041624', 'potion_agility' ], # agility
	8:		[ 1, 0, 0, '#1044541', '#1041625', 'potion_greateragility' ], # greater agility
	9:		[ 1, 0, 0, '#1044546', '#1041626', 'potion_strength' ], # strength
	10:	[ 1, 0, 0, '#1044547', '#1041627', 'potion_greaterstrength' ], # greater strength
	11:	[ 0, 1, 1, '#1044555', '#1041637', 'potion_lesserexplosion' ], # lesser explosion
	12:	[ 0, 1, 1, '#1044556', '#1041638', 'potion_explosion' ], # explosion
	13:	[ 0, 1, 1, '#1044557', '#1041639', 'potion_greaterexplosion' ], # greater explosion
	14:	[ 1, 0, 0, '#1044548', '#1041628', 'potion_lesserpoison' ], # lesser poison
	15:	[ 1, 0, 0, '#1044549', '#1041629', 'potion_poison' ], # poison
	16:	[ 1, 0, 0, '#1044550', '#1041630', 'potion_greaterpoison' ], # greater poison
	17:	[ 1, 0, 0, '#1044551', '#1041631', 'potion_deadlypoison' ], # deadly poison
	18:	[ 1, 0, 0, '#1044538', '#1041632', 'potion_refresh' ], # refresh
	19:	[ 1, 0, 0, '#1044539', '#1041633', 'potion_totalrefresh' ], # total refresh
	20:	[ 1, 0, 0, 'Intellegence', 'A keg of Intellegence potions', 'potion_intelligence' ], # intelligence
	21:	[ 1, 0, 0, 'Greater Intellegence', 'A keg of Greater Intellegence potions', 'potion_greaterintelligence' ], # greater intelligence
	22:	[ 1, 0, 0, 'Lesser Mana', 'A keg of Lesser Mana potions', 'potion_lessermana' ], # lesser mana
	23:	[ 1, 0, 0, 'Mana', 'A keg of Mana potions', 'potion_mana' ], # mana
	24:	[ 1, 0, 0, 'Greater Mana', 'A keg of Greater Mana potions', 'potion_greatermana' ] # greater mana
}

POT_RETURN_BOTTLE = 0
POT_AGGRESSIVE = 1
POT_TARGET = 2
POT_NAME = 3
KEG_NAME = 4
POT_DEF = 5

# Use the potion
def onUse( char, item ):
	socket = char.socket
	# Potions need to be on your body to use them, or in arms reach.
	if item.getoutmostchar() != char:
		char.message( "This potion is out of your reach..." )
		return OOPS

	# Lets make sure the tag exists.
	if not item.hastag( 'potiontype' ):
		return OOPS
	else:
		potiontype = item.gettag( 'potiontype' )
		# Make sure it's in the index
		if not potiontype in potions:
			return OOPS

		# Do we throw this thing?
		if potions[ potiontype ][ POT_TARGET ] == TRUE:
			# Explosion Potion
			if potiontype in [ 11, 12, 13 ]:
				# char, potion, counter value
				if not item.hastag('exploding'):
					potionexplosion( [ char.serial, item.serial, 4, item.amount ] )
					item.settag('exploding', 'true')
				socket.sysmessage( 'Please select a target...', RED )
				socket.attachtarget( "potions.targetexplosionpotion", [ item ] )

		# We just drink this potion...
		else:
			# If it's a drinkable potion we check if there is a free hand.
			if not canUsePotion( char, item ):
				return OK

			# Nightsight Potions
			if potiontype == 0:
				nightsightPotion( char, item )
			# Heal Potions
			elif potiontype in [1,2,3]:
				healPotion(char, item, potiontype)
			# Cure Potions
			elif potiontype in [4,5,6]:
				curePotion(char, item, potiontype)
			# Agility Potions
			elif potiontype in [7,8]:
				agilityPotion( char, item, potiontype )
			# Strength Potions
			elif potiontype in [ 9, 10 ]:
				strengthPotion( char, item, potiontype )
			# Poison Potions
			elif potiontype in [ 14, 15, 16, 17 ]:
				poisonPotion( char, item, potiontype )
			# Refresh Potions
			elif potiontype in [ 18, 19 ]:
				refreshPotion( char, item, potiontype )

		return OK


# Explosion Potion Function
def targetexplosionpotion(char, args, target):
	socket = char.socket
	potion = args[0]
	check = 10
	if not potion:
		return 0
	if target.char:
		if target.char.invulnerable:
			return 0
		if target.char.dead:
			return 0
		if not char.cansee(target.char):
			return 0
		if char.distanceto(target.char) > check:
			return 0
		if not char.canreach(target.char, check):
			return 0
		pos = target.char.pos
	elif target.item:
		if not char.cansee(target.item):
			return 0
		if char.distanceto(target.item) > check:
			return 0
		if not char.canreach(target.item, check):
			return 0
		item = target.item.getoutmostitem()
		if item.container:
			pos = item.container.pos
		else:
			pos = item.pos
	else:
		if not char.canreach(target.pos, check):
			return 0
		pos = target.pos

	if char.distanceto(pos) > 15:
		socket.clilocmessage(1005539)
		return

	throwobject(char, potion, pos, 1, 3, 5)
	# char, potion, counter value
	#potionexplosion( [ char.serial, potion.serial, 4, potion.amount ] )
	return

# Explosion Potion Function
def potionexplosion(args):
	char = wolfpack.findchar(args[0])
	potion = wolfpack.finditem(args[1])
	counter = args[2]
	bonus = args[3]
	if not bonus:
		bonus = 1
	if counter > 0:
		wolfpack.addtimer(1000, "potions.potioncountdown", [char.serial, potion.serial, counter, bonus] )
		return
	else:
		potion.soundeffect(0x307) # Boom!
		potion.effect(0x36BD, 20, 10)
		potionregion( [char, potion, bonus] )
		potion.delete()
		return

# Explosion Potion Function
def potioncountdown( time, args ):
	char = wolfpack.findchar(args[0])
	potion = wolfpack.finditem(args[1])
	counter = args[2]
	bonus = args[3]
	if potion:
		if not bonus:
			bonus = 1
		if counter >= 0:
			if counter > 0:
				potion.say("%u" % (counter - 1))
				counter -= 1
			potionexplosion([char.serial, potion.serial, counter, bonus])
		return

# Checks line of sight for characters with blasts.
def charpotionloscheck( args ):
	char = wolfpack.findchar(args[0])
	potion = wolfpack.finditem(args[1])
	check = args[2]
	if char.invulnerable:
		return 0
	if char.dead:
		return 0
	if not char.cansee(potion):
		return 0
	if char.distanceto(potion) > check:
		return 0
	if not char.canreach(potion, check):
		return 0
	return 1

def itempotionloscheck( args ):
	potion1 = wolfpack.finditem(args[0])
	potion2 = wolfpack.finditem(args[1])
	check = args[2]
	if potion1.distanceto(potion2) > check:
		return 0
	return 1

def charcharloscheck( args ):
	char1 = wolfpack.findchar(args[0])
	char2 = wolfpack.findchar(args[1])
	check = args[2]
	if char2.invulnerable:
		return 0
	if char2.dead:
		return 0
	if not char1.cansee(char2):
		return 0
	if char1.distanceto(char2) > check:
		return 0
	if not char1.canreach(char2, check):
		return 0
	return 1

# Explosion Potion Function
def potionregion( args ):
	char = args[0]
	potion = args[1]
	bonus = args[2]
	if potion.gettag('potiontype') == 11:
		outradius = 1
	elif potion.gettag('potiontype') == 12:
		outradius = 2
	elif potion.gettag('potiontype') == 13:
		outradius = randint(2,3)
	else:
		outradius = 1
	# Potion Keg Radius Override!
	if potion.hastag('kegfill') and potion.hastag('potiontype'):
		if potion.gettag('potiontype') in [11, 12, 13] and potion.gettag('kegfill') >= 1:
			kegfill = potion.gettag('kegfill')
			if kegfill == 100:
				outradius = 13
			elif kegfill >= 90:
				outradius = 12
			elif kegfill >= 80:
				outradius = 11
			elif kegfill >= 70:
				outradius = 10
			elif kegfill >= 60:
				outradius = 9
			elif kegfill >= 50:
				outradius = 8
			elif kegfill >= 40:
				outradius = 7
			elif kegfill >= 30:
				outradius = 6
			else:
				outradius = 5
	# Potion thrown on the ground
	if not potion.container:
		x1 = int(potion.pos.x - outradius)
		y1 = int(potion.pos.y - outradius)
		x2 = int(potion.pos.x + outradius)
		y2 = int(potion.pos.y + outradius)
		damageregion = wolfpack.charregion( x1, y1, x2, y2, potion.pos.map )
		# Character Bombing
		target = damageregion.first
		while target:
			if charpotionloscheck( [ target.serial, potion.serial, outradius ] ):
				target.effect(0x36BD, 20, 10)
				potiondamage(char, target, potion, bonus)
				target = damageregion.next
			else:
				target = damageregion.next

		# Chain Reaction Bombing
		chainregion = wolfpack.itemregion( x1, y1, x2, y2, potion.pos.map )
		chainbomb= chainregion.first
		while chainbomb:
			if chainbomb.baseid in [ 'potion_greaterexplosion', 'potion_explosion', 'potion_lesserexplosion', 'f0d' ]:
				if itempotionloscheck( [potion.serial, chainbomb.serial, outradius] ):
					if not chainbomb.hastag('exploding'):
						chainbomb.settag('exploding', 'true')
						wolfpack.addtimer(randint(1000, 2250), "potions.potioncountdown", [char.serial, chainbomb.serial, 0, chainbomb.amount] )
						chainbomb = chainregion.next
					else:
						chainbomb = chainregion.next
				else:
					chainbomb = chainregion.next
			# Potion Kegs
			elif chainbomb.hastag('kegfill') and chainbomb.hastag('potiontype'):
				if itempotionloscheck( [potion.serial, chainbomb.serial, outradius] ):
					if chainbomb.gettag('potiontype') in [11, 12, 13] and chainbomb.gettag('kegfill') >= 1:
						wolfpack.addtimer(randint(1000, 2250), "potions.potioncountdown", [char.serial, chainbomb.serial, 11, chainbomb.gettag('kegfill') ] )
						chainbomb = chainregion.next
					else:
						chainbomb = chainregion.next
				else:
					chainbomb = chainregion.next
			else:
				chainbomb = chainregion.next
		return
	# Potion is in a container
	else:
		x1 = int(char.pos.x - outradius)
		y1 = int(char.pos.y - outradius)
		x2 = int(char.pos.x + outradius)
		y2 = int(char.pos.y + outradius)
		damageregion = wolfpack.charregion( x1, y1, x2, y2, char.pos.map )
		# Area Bombing
		target = damageregion.first
		while target:
			if charcharloscheck( [ char.serial, target.serial, outradius ] ):
				target.effect(0x36BD, 20, 10)
				potiondamage(char, target, potion, bonus)
				target = damageregion.next
			else:
				target = damageregion.next

		# Chain Reaction Bombing
		chainregion = wolfpack.itemregion( x1, y1, x2, y2, char.pos.map )
		chainbomb = chainregion.first
		while chainbomb:
			if chainbomb.baseid in [ 'potion_greaterexplosion', 'potion_explosion', 'potion_lesserexplosion', 'f0d' ]:
				if itempotionloscheck( [potion.serial, chainbomb.serial, outradius] ):
					if not chainbomb.hastag('exploding'):
						chainbomb.settag('exploding', 'true')
						wolfpack.addtimer(randint(1000, 2250), "potions.potioncountdown", [char.serial, chainbomb.serial, 0] )
						chainbomb = chainregion.next
					elif chainbomb.hastag('kegfill') and chanbomb.hastag('potiontype'):
						if chainbomb.gettag('potiontype') in [11, 12, 13] and chainbomb.gettag('kegfill') >= 1:
							wolfpack.addtimer(randint(1000, 2250), "potions.potioncountdown", [char.serial, chainbomb.serial, 11, chainbomb.gettag('kegfill') ] )
							chainbomb = chainregion.next
					else:
						chainbomb = chainregion.next
				else:
					chainbomb = chainregion.next
			# Potion Kegs
			elif chainbomb.hastag('kegfill') and chainbomb.hastag('potiontype'):
				if itempotionloscheck( [potion.serial, chainbomb.serial, outradius] ):
					if chainbomb.gettag('potiontype') in [11, 12, 13] and chainbomb.gettag('kegfill') >= 1:
						wolfpack.addtimer(randint(1000, 2250), "potions.potioncountdown", [char.serial, chainbomb.serial, 11, chainbomb.gettag('kegfill') ] )
						chainbomb = chainregion.next
					else:
						chainbomb = chainregion.next
				else:
					chainbomb = chainregion.next
			else:
				chainbomb = chainregion.next
		return

# Explosion Potion Function
def potiondamage( char, target, potion, dmgbonus ):
	if potion.gettag('potiontype') == 11:
		damage = randint(POTION_LESSEREXPLOSION_RANGE[0], POTION_LESSEREXPLOSION_RANGE[1])
	elif potion.gettag('potiontype') == 12:
		damage = randint(POTION_EXPLOSION_RANGE[0], POTION_EXPLOSION_RANGE[1])
	elif potion.gettag('potiontype') == 13:
		damage = randint(POTION_GREATEREXPLOSION_RANGE[0], POTION_GREATEREXPLOSION_RANGE[1])
	else:
		damage = randint(POTION_LESSEREXPLOSION_RANGE[0], POTION_GREATEREXPLOSION_RANGE[1])
	if char.skill[ALCHEMY] == 1200:
		bonus = 10
	elif char.skill[ALCHEMY] >= 1100:
		bonus = randint(8,9)
	elif char.skill[ALCHEMY] >= 1000:
		bonus = randint(6,7)
	else:
		bonus = randint(0,5)
	if potion.amount > 1:
		damage = damage * potion.amount
	if dmgbonus > 1:
		damage = damage * dmgbonus
	damage += bonus
	if not potion.container:
		if char.distanceto(potion) > 1:
			damage = damage / char.distanceto(potion)
	else:
		damage = damage / char.distanceto(char)
	energydamage(target, char, damage, fire=100)
	return

# You have to have one hand free for using a potion
# This is not valid for explosion potions
def canUsePotion( char, item ):
	firsthand = char.itemonlayer( 1 )
	secondhand = char.itemonlayer( 2 )

	if not firsthand and not secondhand:
		return OK

	if firsthand and not secondhand and not firsthand.twohanded:
		return OK

	if not firsthand and secondhand and not secondhand.twohanded:
		return OK

	char.socket.clilocmessage( 0x7A99C ) # You must have a free hand to drink a potion.
	return OOPS

# Consume the potion
def consumePotion( char, potion, givebottle ):

	if potion.amount == 1:
		potion.delete()
	else:
		potion.amount -= 1
		potion.update()

	if givebottle == TRUE: # Lets add an empty bottle!
		bottle = wolfpack.additem( 'f0e' ) # Empty Bottle Definition
		if not wolfpack.utilities.tocontainer( bottle, char.getbackpack() ):
			bottle.update()

"""
-----------------------------
-- POTION FUNCTIONS --
-----------------------------
"""

# Nightsight potion
def nightsightPotion( char, potion ):
	socket = char.socket
	# Remove an old bonus
	if char.hastag('nightsight'):
		bonus = char.gettag('nightsight')
		char.lightbonus = max(0, char.lightbonus - bonus)

	# With 100% magery you gain a 18 light level bonus
	bonus = min(18, math.floor(18 * (char.skill[MAGERY] / 1000.0)))

	char.events = ['magic.nightsight'] + char.events
	char.settag("nightsight", bonus)
	char.settag("nightsight_start", time.minutes())
	char.lightbonus += bonus

	if char.socket:
		socket.updatelightlevel()

	char.soundeffect(0x1e3)
	char.effect(0x376a, 9, 32)
	consumePotion( char, potion, potions[ potion.gettag('potiontype') ][ POT_RETURN_BOTTLE ] )
	return OK

# Heal Potions
def healPotion( char, potion, healtype ):
	socket = char.socket
	if not canUsePotion( char, potion ):
		return OOPS

	if char.poison > -1:
		# You can not heal yourself in your current state.
		socket.clilocmessage(1005000)
		return OOPS
	if char.hitpoints >= char.maxhitpoints:
		socket.clilocmessage(1049547)
		if char.hitpoints > char.maxhitpoints:
			char.hitpoints = char.maxhitpoints
			char.updatehealth()
		return OOPS

	if not char.hastag( "heal_pot_timer" ):
		char.settag( "heal_pot_timer", (time.servertime() + HEAL_POT_DELAY) )

	# Compare
	elapsed = int( char.gettag( "heal_pot_timer" ) )
	if elapsed > time.servertime():
		socket.clilocmessage( 500235, '', GRAY ) # You must wait 10 seconds before using another healing potion.
		return OOPS
	else:
		char.settag( "heal_pot_timer", (time.servertime() + HEAL_POT_DELAY) )

	amount = 0

	# Lesser Heal
	if healtype == 1:
		amount = randint( POTION_LESSERHEAL_RANGE[0], POTION_LESSERHEAL_RANGE[1] )
	# Heal
	elif healtype == 2:
		amount = randint( POTION_HEAL_RANGE[0], POTION_HEAL_RANGE[1] )
	# Greater Heal
	elif healtype == 3:
		amount = randint( POTION_GREATERHEAL_RANGE[0], POTION_GREATERHEAL_RANGE[1] )

	char.hitpoints = min( char.hitpoints + amount, char.maxhitpoints ) # We don't heal over our maximum health

	# Resend Health
	char.updatehealth()
	#char.socket.clilocmessage( 1060203, str(amount) , GRAY, NORMAL ) # broken
	socket.sysmessage( 'You have had ' + str( amount ) + ' hit points of damage healed.', GRAY )

	char.action( ANIM_FIDGET3 )
	char.soundeffect( SOUND_DRINK1 )
	consumePotion( char, potion, potions[ healtype ][ POT_RETURN_BOTTLE ] )

	return OK

# Cure Potions
def curePotion( char, potion, curetype ):
	socket = char.socket
	if char.poison == -1:
		socket.clilocmessage(1042000) # You are not poisoned.
		return OOPS

	if curetype == 4:
		curelevel = 0
	elif curetype == 5:
		curelevel = 1
	elif curetype == 6:
		curelevel = 2

	if curelevel >= char.poison:
		poison.cure(char)
		char.effect(0x373a, 10, 15)
		char.soundeffect(0x1e0)
	# curelevel now must be lower than char.poison
	else:
		if (char.poison - curelevel) == 1:
			chance = 0.5
		elif (char.poison - curelevel) == 2:
			chance = 0.25
		elif (char.poison - curelevel) == 3:
			chance = 0.1

		if chance > random():
			poison.cure(char)
			char.effect(0x373a, 10, 15)
			char.soundeffect(0x1e0)

	# Drinking and consume
	char.action( ANIM_FIDGET3 )
	char.soundeffect( SOUND_DRINK1 )
	consumePotion( char, potion, potions[ curetype ][ POT_RETURN_BOTTLE ] )
	# If we succeeded, special effects
	if char.poison == -1:
		char.effect(0x373a, 10, 15)
		char.soundeffect(0x1e0)
		socket.clilocmessage(500231) # You feel cured of poison!
	else:
		socket.clilocmessage(500232) # That potion was not strong enough to cure your ailment!
	return OK

# Agility Potion
def agilityPotion( char, potion, agilitytype ):
	socket = char.socket
	bonus = 0

	# Agility
	if agilitytype == 7:
		bonus = 10
	# Greater Agility
	elif agilitytype == 8:
		bonus = 20
	# Oops!
	else:
		return OOPS

	if not char.hastag( "dex_pot_timer" ):
		char.settag( "dex_pot_timer", (time.servertime() + AGILITY_TIME) )

	# Compare
	elapsed = int( char.gettag( "dex_pot_timer" ) )
	if elapsed > time.servertime():
		socket.clilocmessage(502173) # You are already under a similar effect.
		return OOPS
	else:
		char.settag( 'dex_pot_timer', (time.servertime() + AGILITY_TIME) )

	if char.dexterity + bonus < 1:
		bonus = -(char.strength - 1)
	char.dexterity2 += bonus
	char.dexterity += bonus
	char.stamina = min(char.stamina, char.maxstamina)
	char.updatestamina()
	char.updatestats()

	char.addtimer(AGILITY_TIME, "magic.utilities.statmodifier_expire", [1, bonus], 1, 1, "magic_statmodifier_1", "magic.utilities.statmodifier_dispel")

	char.action( ANIM_FIDGET3 )
	char.soundeffect( SOUND_DRINK1 )
	char.effect( 0x375a, 10, 15 )
	char.soundeffect( SOUND_AGILITY_UP )
	consumePotion( char, potion, potions[ agilitytype ][ POT_RETURN_BOTTLE ] )

	return OK

# Strength Potion
def strengthPotion( char, potion, strengthtype ):
	socket = char.socket
	if not canUsePotion( char, potion ):
		return OOPS

	bonus = 0

	# Agility
	if strengthtype == 9:
		bonus = 10
	# Greater Agility
	elif strengthtype == 10:
		bonus = 20
	# Oops!
	else:
		return OOPS

	if not char.hastag( "str_pot_timer" ):
		char.settag( "str_pot_timer", (time.servertime() + STRENGTH_TIME) )

	# Compare
	elapsed = int( char.gettag( "str_pot_timer" ) )
	if elapsed > time.servertime():
		socket.clilocmessage(502173) # You are already  under a similar effect
		return OOPS
	else:
		char.settag( 'str_pot_timer', (time.servertime() + STRENGTH_TIME) )

	if char.strength + bonus < 1:
		bonus = -(char.strength - 1)
	char.strength2 += bonus
	char.strength += bonus
	char.hitpoints = min(char.hitpoints, char.maxhitpoints)
	char.updatehealth()
	char.updatestats()

	char.addtimer(STRENGTH_TIME, "magic.utilities.statmodifier_expire", [0, bonus], 1, 1, "magic_statmodifier_0", "magic.utilities.statmodifier_dispel")

	char.action( ANIM_FIDGET3 )
	char.soundeffect( SOUND_DRINK1 )
	char.effect( 0x375a, 10, 15 )
	char.soundeffect( SOUND_STRENGTH_UP )
	consumePotion( char, potion, potions[ strengthtype ][ POT_RETURN_BOTTLE ] )

	return OK

# Poison Potions
def poisonPotion( char, potion, poisontype ):
	socket = char.socket
	levels = [14, 15, 16, 17]
	if poisontype == levels[0]:
		poison.poison(char, 0)
	elif poisontype == levels[1]:
		poison.poison(char, 1)
	elif poisontype == levels[2]:
		poison.poison(char, 2)
	elif poisontype == levels[3]:
		poison.poison(char, 3)
	else:
		return OOPS

	char.action( ANIM_FIDGET3 )
	char.soundeffect( SOUND_DRINK1 )
	consumePotion( char, potion, potions[ poisontype ][ POT_RETURN_BOTTLE ] )
	return OK

def refreshPotion( char, potion, refreshtype ):
	socket = char.socket
	# refresh potion
	if refreshtype == 18:
		char.stamina += (char.maxstamina / 4)
		char.updatestamina()
	# total refresh potion
	elif refreshtype == 19:
		char.stamina = char.maxstamina
		char.updatestamina()
	else:
		return OOPS

	if char.stamina > char.maxstamina:
		char.stamina = char.maxstamina
		char.updatestamina()
		return OOPS

	char.action( ANIM_FIDGET3 )
	char.soundeffect( SOUND_DRINK1 )
	consumePotion( char, potion, potions[ refreshtype ][ POT_RETURN_BOTTLE ] )
	return OK

# INVIS POTION
# 502179	Your skin becomes extremely sensitive to light, changing to mirror the colors of things around you.
