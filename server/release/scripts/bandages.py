
import wolfpack
import random
import system.poison
import system.bleeding
import wolfpack.utilities
from wolfpack.consts import *
from combat.specialmoves import ismortallywounded

def onUse( char, item ):
	if not char.canreach(item, 2):
		char.socket.clilocmessage(500295)
		return True
		
	# already healing?
	if char.socket.hastag( 'bandage_slipped' ):
		char.socket.sysmessage( 'You are already healing somebody.' )
		return True
	
	char.reveal() # Reveal

	# Display Target
	if item.id == 0xe21 or item.id == 0xee9:
		char.socket.clilocmessage(500948) #who will you use on
		char.socket.attachtarget('bandages.response', [item.serial])

	return 1
	
def response(char, arguments, target):
	bandage = wolfpack.finditem(arguments[0])
	
	# Out of reach?
	if not bandage or not char.canreach(bandage, 2):
		char.socket.clilocmessage(500295)
		return

	# Targetcheck
	targetChar = target.char
	
	if not targetChar:
		if target.item and target.item.corpse:
			targetChar = target.item.owner
			
		if not targetChar:
			char.socket.clilocmessage(500970)
			return
		
	# Range check
	if not char.canreach(targetChar, 2):
		char.socket.clilocmessage(500295)
		return
		
	# Remove a bandage if we can start healing
	if startheal(char, targetChar):
		if bandage.amount > 1:
			bandage.amount -= 1
			bandage.update()
		else:
			bandage.delete()

def getskills(target):
	if not target.player and target.id not in [0x190, 0x191, 0x192, 0x193]:
		return (VETERINARY, ANIMALLORE)
	else:
		return (HEALING, ANATOMY)

# Start healing the target
def startheal(char, target):
	socket = char.socket
	
	if target.baseid == 'golem':
		socket.clilocmessage(500970) # Golems cannot be healed
		return False
		
	elif target.getstrproperty('slayer_group', '') == 'undeads':
		socket.clilocmessage(500951) # Undeads cannot be healed
		return False
		
	elif ismortallywounded(target):
		if char == target:
			socket.clilocmessage(1005000)
		else:
			socket.clilocmessage(1010398)
		return False		
		
	elif target.poison == -1 and target.hitpoints >= target.maxhitpoints and not system.bleeding.isbleeding(target):
		socket.clilocmessage(500955) # Already at full health
		return False

	elif target.dead and not target.pos.validspawnspot():
		socket.clilocmessage(501042) # Not a valid spawnspot for living players
		return False

	(primary, secondary) = getskills(target) # Get the skills used to heal the target
	
	# For resurrecting someone there is a 5 second delay
	if target.dead:
		resurrection = 5000
	else:
		resurrection = 0
	
	# We are bandaging ourself
	if target == char:
		delay = int(5000 + (500 * ((120 - char.dexterity) / 10.0)))
	# We are bandaging someone else
	else:
		# We are targetting an animal
		if primary == VETERINARY:
			if char.dexterity >= 40:
				delay = 2000
			else:
				delay = 3000
		# We are bandaging another player or a human
		else:
			if char.dexterity >= 100:
				delay = 3000 + resurrection
			elif char.dexterity >= 40:
				delay = 4000 + resurrection
			else:
				delay = 5000 + resurrection

	# what's the use of this dispel??
	#char.dispel(None, 1, 'bandage_timer') # Display pending bandage timers
	socket.settag('bandage_slipped', 0) # Clear the "slipping" property
	socket.clilocmessage(500956) # Begin applying bandages
	char.addtimer(delay, 'bandages.endheal', [primary, secondary, target.serial]) # Add a bandage timer
	# Show an emote that he is using bandages ?

	return True
	
# Stop healing
def endheal(char, arguments):
	socket = char.socket
	
	if not socket:
		return # Cancel healing if the player disconnected
		
	if char.dead:
		socket.clilocmessage(500962) # Died before finishing his work
		return
	
	if socket.hastag('bandage_slipped'):
		slipped = int(socket.gettag('bandage_slipped')) # How many times the fingers slipped
	else:
		slipped = 0
	socket.deltag('bandage_slipped')

	# Retrieve arguments
	(primary, secondary, target) = arguments
	target = wolfpack.findchar(target)
	
	# Sanity checks again
	if not target or not char.canreach(target, 2):
		socket.clilocmessage(500963) # You did not stay close enough
		return
		
	if target.dead:
		resurrectTarget(char, target, primary, secondary, slipped) # Resurrection attempt
	elif target.poison != -1:
		cureTarget(char, target, primary, secondary, slipped) # Cure attempt
	elif system.bleeding.isbleeding(target):
		system.bleeding.end(target)
	else:
		healTarget(char, target, primary, secondary, slipped) # Heal attempt

# Resurrecting the target		
def resurrectTarget(char, target, primary, secondary, slipped):
	socket = char.socket
	primarySkill = char.skill[primary]
	secondarySkill = char.skill[secondary]

	char.soundeffect(0x57) # Play a soundeffect for the applied bandages
	
	chance = ((primarySkill - 680) / 500.0) - (slipped * 0.02)

	if primarySkill >= 800 and secondarySkill >= 800: # Minimum skill
		# Can the guy be resurrected at that position
		if not target.pos.validspawnspot():
			socket.clilocmessage(501042)
			if target.socket:
				target.socket.clilocmessage(502391)
			return # Cancel

		char.checkskill(primary, 0, 1200) # Check skills
		char.checkskill(secondary, 0, 1200) # Check skills
		
		if chance > random.random(): # We succeed in resurrecting the target
			socket.clilocmessage(500965) # You succeed
			target.resurrect() # Resurrect the target
			return # Cancel or else the other message will appear
	
	if not target.player:
		socket.clilocmessage(503256) # You fail to resurrect the creature
	else:
		socket.clilocmessage(500966) # You are unable to resurrect your patient
	
# Curing the target
def cureTarget(char, target, primary, secondary, slipped):
	socket = char.socket
	primarySkill = char.skill[primary]
	secondarySkill = char.skill[secondary]

	socket.clilocmessage(500969) # Finished applying bandages
	
	chance = ((primarySkill - 300) / 500.0) - (target.poison * 0.1) - (slipped * 0.02) # Calculate chance
	
	if primarySkill >= 600 and secondarySkill >= 600: # Unable to cure the poison
		char.checkskill(primary, 0, 1200) # Check primary skill
		char.checkskill(secondary, 0, 1200) # Check secondary skill
		
		if chance > random.random():
			if char != target:
				socket.clilocmessage(1010058) # You cured all poisons
				if target.socket:
					target.socket.clilocmessage(1010059) # You have been cured of all poisons.
			elif target.socket:
				target.socket.clilocmessage(1010059) # You have been cured of all poisons
			
			system.poison.cure(target) # Cure the poison
		else:
			socket.clilocmessage(1010060) # You failed to cure your target
	else:
		socket.clilocmessage(1010060) # You failed to cure your target
	
	char.soundeffect(0x57) # Play a soundeffect for the applied bandages
	
# Healing the target
def healTarget(char, target, primary, secondary, slipped):
	socket = char.socket
	primarySkill = char.skill[primary]
	secondarySkill = char.skill[secondary]
	char.soundeffect(0x57)
	
	if target.hitpoints >= target.maxhitpoints:
		socket.clilocmessage(500967) # Already at full health		
		return
				
	char.checkskill(primary, 0, 1200) # Check primary skill
	char.checkskill(secondary, 0, 1200) # Check secondary skill
	
	chance = ((primarySkill + 100) / 1000.0) - slipped * 0.02
	
	if chance <= random.random():
		socket.clilocmessage(500968) # Your bandages barely helped
		return
		
	minValue = int((secondarySkill / 80.0) + (primarySkill / 50.0) + 4.0) # Min value to heal
	maxValue = int((secondarySkill / 60.0) + (primarySkill / 25.0) + 4.0) # Max value to heal
		
	value = random.randint(minValue, maxValue) # Calculate a value between min/max
	value = max(1, value - (value * slipped * 0.35)) # Take the count of slipped fingers into account
	
	if value == 1:
		socket.clilocmessage(500968) # Your bandages barley help
	else:
		socket.clilocmessage(500969) # You finish applying the bandages
		
	target.hitpoints = min(target.maxhitpoints, target.hitpoints + value)
	target.updatehealth()
