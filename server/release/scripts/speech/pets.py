#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Created by: DarkStorm                      #
#   )).-' {{ ;'`   | Revised by:                                #
#  ( (  ;._ \\ ctr | Last Modification: Created                 #
#===============================================================#
# Speech Script for Pets                                        #
#===============================================================#

from wolfpack.consts import *
import wolfpack
import random

#check if the pet does follow the order 
#Formula from http://uo.stratics.com/content/professions/taming/taming-lore.shtml#loyalty
#return true if pet follow the order
#false if the refuses to follow the order
def checkPetControl(pet, char, text, keywords):
	
	#GMs can control everything
	if char.gm:
		return True
	
	if pet.mintaming <=291:
		return True
		
	#summoned pets can always be conrolled
	if pet.summoned:
		return True
		
	diff = pet.mintaming / 10.0
	taming = char.skill[TAMING]
	lore = char.skill[ANIMALLORE]
	#Effective Taming=(Animal Taming * 4 + Animal Lore) / 5
	effTaming = ( taming * 4 + lore) / 5
	effTaming = effTaming / 10.0
	
	if diff > effTaming:
		#Pet Control %=70 + 14 * (Effective Taming - Pet Difficulty) if efftaming < diff
		chance = 70 + 14* (effTaming - diff)
	else:
		# Pet Control %=70 + 6 * (Effective Taming - Pet Difficulty) if efftaming > diff
		chance = 70 + 6 * (effTaming - diff)
		
	if chance > 99:
		chance = 99
	if chance < 20:
		chance = 20
	
	if random.randint(1,100) <= chance:
		return True
	else:
		return False

def stopfight(pet):
	pet.attacktarget = None
	pet.war = 0
	pet.updateflags()

def startfight(pet, targetchar):
	pet.attacktarget = targetchar
	pet.war = 1
	pet.updateflags()

def come(char, pet, all=False):
	if all:
		if char.gm:
			pos = char.pos
			pets = wolfpack.chars(pos.x, pos.y, pos.map, 18, False)
			for follower in pets:
				if follower.npc and not follower.frozen:
					come(char, follower, False)
		else:
			for follower in char.followers:
				come(char, follower, False)
	elif pet and (pet.owner == char or char.gm) and pet.distanceto(char) < 18:
		stopfight(pet)
		pet.guarding = None
		pet.goto(char.pos)
		pet.sound(SND_ATTACK)

def stop(char, pet, all=False):
	if all:
		if char.gm:
			pos = char.pos
			pets = wolfpack.chars(pos.x, pos.y, pos.map, 18, False)
			for follower in pets:
				if follower.npc and not follower.frozen:
					stop(char, follower, False)
		else:
			for follower in char.followers:
				stop(char, follower, False)
	elif pet and (pet.owner == char or char.gm) and pet.distanceto(char) < 18:
		stopfight(pet)
		pet.guarding = None
		pet.wandertype = 0
		pet.sound(SND_ATTACK)

def follow_me(char, pet, all=False):
	if all:
		if char.gm:
			pos = char.pos
			pets = wolfpack.chars(pos.x, pos.y, pos.map, 18, False)
			for follower in pets:
				if follower.npc and not follower.frozen:
					follow_me(char, follower, False)
		else:
			for follower in char.followers:
				follow_me(char, follower, False)
	if pet and (pet.owner == char or char.gm) and pet.distanceto(char) < 18:
		stopfight(pet)
		pet.follow(char)
		pet.sound(SND_ATTACK)

def go_target(char, arguments, target):
	(pet, all) = arguments
	if all:
		if char.gm:
			pos = char.pos
			pets = wolfpack.chars(pos.x, pos.y, pos.map, 18, False)
			for follower in pets:
				if follower.npc and not follower.frozen:
					go_target(char, [follower.serial, False], target)
		else:
			for follower in char.followers:
				go_target(char, [follower.serial, False], target)
	else:
		pet = wolfpack.findchar(pet)
		if pet and (pet.owner == char or char.gm) and pet.distanceto(char) < 18:
			stopfight(pet)
			pet.guarding = None
			pet.goto(target.pos)
			pet.sound(SND_ATTACK)

def go(char, pet, all=False):
	char.socket.sysmessage('Please select a destination.')
	char.socket.attachtarget("speech.pets.go_target", [pet.serial, all])

def attack_target(char, arguments, target):
	if not target.char or target.char == char:
		char.socket.sysmessage('Your pets cannot attack that.')
		return

	(pet, all) = arguments
	if all:
		if char.gm:
			pos = char.pos
			pets = wolfpack.chars(pos.x, pos.y, pos.map, 18, False)
			for follower in pets:
				if follower.npc and not follower.frozen:
					attack_target(char, [follower.serial, False], target)
		else:
			for follower in char.followers:
				attack_target(char, [follower.serial, False], target)
	else:
		pet = wolfpack.findchar(pet)
		if target.char == pet:
			char.socket.sysmessage('Your pet refuses to kill itself.')
			return
		if pet and (pet.owner == char or char.gm) and pet.distanceto(char) < 18:
			startfight(pet, target.char)
			pet.fight(target.char)
			
			# right here we need to not interrupt the target, but i cant register a fight unless i do, right? not sure.
			target.char.fight(pet)
			target.char.follow(pet)
			
			go_target(char, arguments, target)
			follow_target(char, arguments, target)
			pet.sound(SND_ATTACK)

def attack(char, pet, all=False):
	char.socket.sysmessage('Please select an attack target.')
	char.socket.attachtarget("speech.pets.attack_target", [pet.serial, all])

def transfer_target(char, arguments, target):
	if not target.char or target.char == char or not target.char.player:
		char.socket.sysmessage('You can''t transfer your pet to them.')
		return

	pet = wolfpack.findchar(arguments[0])

	if pet and (pet.owner == char or char.gm):
		if (target.char.controlslots + pet.controlslots) > target.char.maxcontrolslots:
			char.socket.sysmessage('Your target already controls too many pets.')
			return
	
		pet.owner = target.char
		# Tame during transfer if gm
		if char.gm and not pet.tamed:
			pet.tamed = True
			pet.resendtooltip()
		pet.sound(SND_ATTACK)
		char.socket.sysmessage('You transfer your pet to %s.' % target.char.name)

def transfer(char, pet):
	if (not char.gm) and pet.summoned:
		return

	char.socket.sysmessage('Who do you want to transfer your pet to?')
	char.socket.attachtarget("speech.pets.transfer_target", [pet.serial])

def follow_target(char, arguments, target):
	if not target.char:
		char.socket.sysmessage('Your pets can only follow characters.')
		return

	(pet, all) = arguments
	pet = wolfpack.findchar(pet)
	if all:
		if char.gm:
			pos = char.pos
			pets = wolfpack.chars(pos.x, pos.y, pos.map, 18, False)
			for follower in pets:
				if follower.npc and not follower.frozen:
					follow_target(char, [follower.serial, False], target)
		else:
			for follower in char.followers:
				follow_target(char, [follower.serial, False], target)
	elif pet and (pet.owner == char or char.gm) and pet.distanceto(char) < 18:
		#char.socket.sysmessage('Pet following:' + str(target.char.serial))
		pet.guarding = None
		pet.follow(target.char)
		pet.sound(SND_ATTACK)

def follow(char, pet, all=False):
	if all:
		char.socket.sysmessage('Who do you want your pets to follow?')
	else:
		char.socket.sysmessage('Who do you want your pet to follow?')

	char.socket.attachtarget("speech.pets.follow_target", [pet.serial, all])

def release(char, pet):
	if pet.summoned:
		pet.delete()
		return

	pet.owner = None
	if pet.tamed:
		pet.tamed = False
	pet.removescript('speech.pets') # Remove this script if it was added dynamically
	pet.wandertype = 1
	pet.sound(SND_ATTACK)

def onSpeech(pet, char, text, keywords):
	if not char.socket:
		return False
		
	text = text.lower()

	# Test Ownership / Allow GMs to control
	if (pet.owner != char or not pet.tamed) and not char.gm:
		return False
	
	
	#check if can be controlled
	if not checkPetControl(pet,char, text, keywords):
		return True
	# Test All
	# Check for keywords

	#char.socket.sysmessage(str(keywords))
	
	all = text.startswith('all ')

	if all:
		#char.socket.sysmessage('zzz ALL')
		# begin all #
		# All Follow Me
		if 232 in keywords and 355 in keywords and 364 in keywords:
			#char.socket.sysmessage('zzz all follow me')
			follow_me(char, pet, True)
			return True
	
		# All Follow
		elif 232 in keywords and 346 in keywords and 357 in keywords:
			follow(char, pet, True)
			return True
		
		# All Kill, All Attack
		elif (360 in keywords and 349 in keywords) or (361 in keywords and 350 in keywords):
			#char.socket.sysmessage('zzz all attack')
			attack(char, pet, True)
			return True

		# All Come (356)
		elif 356 in keywords and 341 in keywords:
			#char.socket.sysmessage('zzz all come')
			come(char, pet, True)
			return True

		# All Stay, All Stop
		elif (368 in keywords and 367 in keywords) or (359 in keywords and 353 in keywords):
			#char.socket.sysmessage('zzz all stay')
			stop(char, pet, True)
			return True

		# All Go
		elif text == 'all go':
			#char.socket.sysmessage('zzz all go')
			go(char, pet, True)
			return True

		# end all #
	else:
		# begin not all #
		# Test Name if not all, return false if false
		if not text.startswith(pet.name.lower() + " "):
			return False

		# Follow Me
		elif 232 in keywords and 355 in keywords:
			#char.socket.sysmessage('zzz Follow me')
			follow_me(char, pet, False)
			return True

		# Follow
		elif 232 in keywords and 346 in keywords:
			#char.socket.sysmessage('zzz Follow')
			follow(char, pet, False)
			return True
		
		# Kill, Attack
		elif 349 in keywords or 350 in keywords:
			attack(char, pet, False)
			return True

		# Stay, Stop
		elif 353 in keywords or 367 in keywords:
			stop(char, pet, False)
			return True

		# Come (341)
		if 341 in keywords:
			come(char, pet, False)
			return True

		# Go
		elif text.endswith('go'):
			go(char, pet, False)
			return True
		
		# Transfer
		elif 366 in keywords:
			transfer(char, pet)
			return True

		# Release
		elif 365 in keywords:
			release(char, pet)
			return True
		# end some #

	return False
