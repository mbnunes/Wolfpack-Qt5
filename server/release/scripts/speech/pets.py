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
	pet.attacktarget = None
	pet.war = 0
	pet.updateflags()

def startfight(pet, targetchar):
	pet.attacktarget = targetchar
	pet.war = 1
	pet.updateflags()

def come(char, pet, all=0):
	if all:
		for follower in char.followers:
			come(char, follower, 0)
	elif pet and (pet.owner == char or char.gm) and pet.distanceto(char) < 18:
		stopfight(pet)
		pet.guarding = None
		pet.goto(char.pos)
		pet.sound(SND_ATTACK)

def stop(char, pet, all=0):
	if all:
		for follower in char.followers:
			stop(char, follower, 0)
	elif pet and (pet.owner == char or char.gm) and pet.distanceto(char) < 18:
		stopfight(pet)
		pet.guarding = None
		pet.wandertype = 0
		pet.sound(SND_ATTACK)

def follow_me(char, pet, all=0):
	if all:
		for follower in char.followers:
			follow_me(char, follower, 0)
	if pet and (pet.owner == char or char.gm) and pet.distanceto(char) < 18:
		stopfight(pet)
		pet.follow(char)
		pet.sound(SND_ATTACK)

def go_target(char, arguments, target):
	(pet, all) = arguments
	if all:
		for follower in char.followers:
			go_target(char, [follower.serial, 0], target)
	else:
		pet = wolfpack.findchar(pet)
		if pet and (pet.owner == char or char.gm) and pet.distanceto(char) < 18:
			stopfight(pet)
			pet.guarding = None
			pet.goto(target.pos)
			pet.sound(SND_ATTACK)

def go(char, pet, all=0):
	char.socket.sysmessage('Please select a destination.')
	char.socket.attachtarget("speech.pets.go_target", [pet.serial, all])

def attack_target(char, arguments, target):
	if not target.char or target.char == char:
		char.socket.sysmessage('Your pets cannot attack that.')
		return

	(pet, all) = arguments
	if all:
		for follower in char.followers:
			attack_target(char, [follower.serial, 0], target)
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

def attack(char, pet, all=0):
	char.socket.sysmessage('Please select an attack target.')
	char.socket.attachtarget("speech.pets.attack_target", [pet.serial, all])

def transfer_target(char, arguments, target):
	if not target.char or target.char == char or not target.char.player:
		char.socket.sysmessage('You can''t transfer your pet to them.')
		return

	pet = wolfpack.findchar(arguments[0])

	if pet and (pet.owner == char or char.gm):
		if len(target.char.followers) + pet.controlslots > target.char.maxcontrolslots:
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
	if pet.summoned:
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
		for follower in char.followers:
			go_target(char, [follower.serial, 0], target)
	elif pet and (pet.owner == char or char.gm) and pet.distanceto(char) < 18:
		#char.socket.sysmessage('Pet following:' + str(target.char.serial))
		pet.guarding = None
		pet.follow(target.char)
		pet.sound(SND_ATTACK)

def follow(char, pet, all=0):
	if all == 1:
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
	pet.sound(SND_ATTACK)

def onSpeech(pet, char, text, keywords):
	if not char.socket:
		return 0

	# Test Ownership / Allow GMs to control
	if ( not pet.owner or pet.owner != char ) and not char.gm:
		return 0

	# Test All
	# Check for keywords
	
	all = text.startswith('all ')
	
	if all:
		text = text.lower()
		#char.socket.sysmessage('zzz ALL')
		# begin all #
		# All Follow Me
		if 232 in keywords and 355 in keywords and 364 in keywords:
			#char.socket.sysmessage('zzz all follow me')
			follow_me(char, pet, 1)
			return 1
	
		# All Follow
		elif 232 in keywords and 346 in keywords and 357 in keywords:
			#char.socket.sysmessage('zzz all follow')
			follow(char, pet, 1)
			return 1
		
		# All Kill, All Attack
		elif (360 in keywords and 349 in keywords) or (361 in keywords and 350 in keywords):
			#char.socket.sysmessage('zzz all attack')
			attack(char, pet, 1)
			return 1

		# All Come (356)
		elif 356 in keywords and 341 in keywords:
			#char.socket.sysmessage('zzz all come')
			come(char, pet, 1)
			return 1

		# All Stay, All Stop
		elif (368 in keywords and 367 in keywords) or (359 in keywords and 353 in keywords):
			#char.socket.sysmessage('zzz all stay')
			stop(char, pet, 1)
			return 1

		# All Go
		elif text == 'all go':
			#char.socket.sysmessage('zzz all go')
			go(char, pet, 1)
			return 1

		# end all #
	else:
		# begin not all #
		# Test Name if not all, return false if false
		if not text.startswith(pet.name.lower() + " "):
			return 0

		# Follow Me
		elif 232 in keywords and 355 in keywords:
			#char.socket.sysmessage('zzz Follow me')
			follow_me(char, pet, 0)
			return 1

		# Follow
		elif 232 in keywords and 346 in keywords:
			#char.socket.sysmessage('zzz Follow')
			follow(char, pet, 0)
			return 1
		
		# Kill, Attack
		elif 349 in keywords or 350 in keywords:
			attack(char, pet, 0)
			return 1

		# Stay, Stop
		elif 353 in keywords or 367 in keywords:
			stop(char, pet, 0)
			return 1

		# Come (341)
		if 341 in keywords:
			come(char, pet, 0)
			return 1

		# Go
		elif text.endswith('go'):
			go(char, pet, 0)
			return 1
		
		# Transfer
		elif 366 in keywords:
			transfer(char, pet)
			return 1

		# Release
		elif 365 in keywords:
			release(char, pet)
			return 1
		# end some #

	return 0