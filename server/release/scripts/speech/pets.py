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

def stopfight(pet):
	if pet.attacktarget:
		pet.attacktarget = None

	if pet.war:
		pet.war = 0
		pet.updateflags()

def come(char, pet, all=0):
	if all:
		for follower in char.followers:
			come(char, follower, 0)
	elif pet and pet.owner == char and pet.distanceto(char) < 18:
		stopfight(pet)
		pet.guarding = None
		pet.goto(char.pos)
		pet.sound(SND_ATTACK)

def stop(char, pet, all=0):
	if all:
		for follower in char.followers:
			stop(char, follower, 0)
	elif pet and pet.owner == char and pet.distanceto(char) < 18:
		stopfight(pet)
		pet.guarding = None
		pet.wandertype = 0
		pet.sound(SND_ATTACK)

def follow_me(char, pet, all=0):
	if all:
		for follower in char.followers:
			follow_me(char, follower, 0)
	elif pet and pet.owner == char and pet.distanceto(char) < 18:
		stopfight(pet)
		pet.follow(char)
		pet.sound(SND_ATTACK)

def go_target(char, arguments, target):
	(pet, all) = arguments
	if all:
		for follower in char.followers:
			if follower.distanceto(char) < 18:
				stopfight(follower)
				follower.guarding = None
				follower.goto(target.pos)
				follower.sound(SND_ATTACK)
	else:
		pet = wolfpack.findchar(pet)
		if pet and pet.owner == char and pet.distanceto(char) < 18:
			stopfight(pet)
			pet.guarding = None
			pet.goto(target.pos)
			pet.sound(SND_ATTACK)

def go(char, pet, all=0):
	char.socket.sysmessage('Please select a destination?')
	char.socket.attachtarget("speech.pets.go_target", [pet.serial, all])

def attack_target(char, arguments, target):
	if not target.char or target.char == char:
		char.socket.sysmessage('Your pets can''t attack that.')
		return

	(pet, all) = arguments
	if all:
		for follower in char.followers:
			follower.fight(target.char)
			follower.sound(SND_ATTACK)
	else:
		pet = wolfpack.findchar(pet)
		if pet and pet.owner == char and pet.distanceto(char) < 18:
			pet.fight(target.char)
			pet.sound(SND_ATTACK)

def attack(char, pet, all=0):
	char.socket.sysmessage('Please select an attack target.')
	char.socket.attachtarget("speech.pets.attack_target", [pet.serial, all])

def transfer_target(char, arguments, target):
	if not target.char or target.char == char or not target.char.player:
		char.socket.sysmessage('You can''t transfer your pet to them.')
		return

	pet = wolfpack.findchar(arguments[0])

	if not pet:
		return

	if len(target.char.followers) + pet.controlslots > 5:
		char.socket.sysmessage('Your target already controls too many pets.')
		return

	if pet.owner == char:
		pet.owner = target.char
		pet.sound(SND_ATTACK)
		char.socket.sysmessage('You transfer your pet to %s.' % target.char.name)

def transfer(char, pet):
	if pet.summoned:
		return

	char.socket.sysmessage('Who do you want to transfer your pet to?')
	char.socket.attachtarget("speech.pets.transfer_target", [pet.serial])

def follow_target(char, arguments, target):
	if not target.char:
		char.socket.sysmessage('Your pets can only follow characters.')
		return

	(pet, all) = arguments
	if all:
		for follower in char.followers:
			if follower.distanceto(char) < 18:
				stopfight(follower)
				follower.guarding = None
				follower.follow(target.char)
				follower.sound(SND_ATTACK)
	else:
		pet = wolfpack.findchar(pet)
		if pet and pet.owner == char and pet.distanceto(char) < 18:
			stopfight(pet)
			pet.guarding = None
			pet.follow(target.char)
			pet.sound(SND_ATTACK)

def follow(char, pet, all=0):
	char.socket.sysmessage('Who do you want your pet to follow?')
	char.socket.attachtarget("speech.pets.follow_target", [pet.serial, all])

def release(char, pet):
	if pet.summoned:
		pet.delete()
		return

	pet.owner = None
	if pet.tamed:
		pet.tamed = 0
	pet.sound(SND_ATTACK)

	pet.removeevent('speech.pets')

def onSpeech(pet, char, text, keywords):
	if not char.socket:
		return 0

	# Check if the pet is owned by char
	if ( not pet.owner or pet.owner != char ) and not char.gm:
		return 0

	# Check for keywords
	checkname = not text.startswith('all ')
	text = text.lower()

	# for all* commands we dont need to check for the name
	if not checkname:
		for keyword in keywords:
			if (keyword >= 356 and keyword <= 364) or keyword == 368:
				checkname = 0

	# Check if we are mean't by this command
	if checkname:
		if not text.startswith(pet.name.lower() + " "):
			return 0

	# Come (341)
	if 341 in keywords:
		come(char, pet, 0)
		return 1

	# All Come (356)
	elif 356 in keywords:
		come(char, pet, 1)
		return 1

	# All Go
	elif text == 'all go':
		go(char, pet, 1)
		return 1

	# Go
	elif text.endswith('go'):
		go(char, pet, 0)
		return 1

	# All Kill, All Attack
	elif 360 in keywords or 361 in keywords:
		attack(char, pet, 1)
		return 1

	# Kill, Attack
	elif 349 in keywords or 350 in keywords:
		attack(char, pet, 0)
		return 1

	# Transfer
	elif 366 in keywords:
		transfer(char, pet)
		return 1

	# Release
	elif 365 in keywords:
		release(char, pet)
		return 1

	# All Stay, All Stop
	elif 368 in keywords or 359 in keywords:
		stop(char, pet, 1)
		return 1

	# Stay, Stop
	elif 353 in keywords or 367 in keywords:
		stop(char, pet, 0)
		return 1

	# Follow Me
	elif 355 in keywords:
		follow_me(char, pet, 0)
		return 1

	# All Follow Me
	elif 364 in keywords:
		follow_me(char, pet, 1)
		return 1

	# All Follow
	elif 357 in keywords:
		follow(char, pet, 1)
		return 1

	# Follow
	elif 346 in keywords:
		follow(char, pet, 1)
		return 1

	return 0
