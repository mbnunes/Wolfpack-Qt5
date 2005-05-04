#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: Naddel                         #
#   )).-' {{ ;'`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification: Created                 #
#################################################################
# todo: logging, randomsteal

import wolfpack
import skills
import wolfpack.time
from wolfpack import tr
from wolfpack.consts import *
import random
import math

STEALING_DELAY = 1000

def isemptyhanded( char ):
	if char.itemonlayer(LAYER_RIGHTHAND) or char.itemonlayer(LAYER_LEFTHAND):
		return False
	return True

def isvendor( victim ):
	if victim.ai =="Human_Vendor" and victim.invulnerable:
		return True
	return False

def isplayervendor(victim):
	if victim.hasscript( "npc.playervendor" ):
		return True
	return False

def check(char, target):
	if not isemptyhanded( char ):
		char.socket.clilocmessage( 1005584 ) # Both hands must be free to steal.
		return False
	elif not char.cansee( target ):
		char.socket.clilocmessage( 500237 ) # Target can not be seen.
		return False
	else:
		return True

def stealing( char, skill ):
	if char.socket.hastag( 'skill_delay' ):
		if wolfpack.time.currenttime() < char.socket.gettag( 'skill_delay' ):
			char.socket.clilocmessage( 500118, "", 0x3b2, 3 )
			return True
		else:
			char.socket.deltag( 'skill_delay' )
	if not isemptyhanded( char ):
		char.socket.clilocmessage( 1005584 ) # Both hands must be free to steal.
		return True
	char.socket.clilocmessage( 502698 ) # Which item do you want to steal?
	char.socket.attachtarget( "skills.stealing.target", [] )

def target(char, args, target):
	char.socket.settag( 'skill_delay', int( wolfpack.time.currenttime() + STEALING_DELAY ) )
	if target.item:
		stealitem(char, target.item)
	elif target.char:
		char_steal(char, target.char)
	else:
		char.socket.clilocmessage( 502710 ) # You can't steal that!
	return True

def stealitem( char, tosteal ):
	victim = tosteal.getoutmostchar()
	# check if hands are empty and target can be seen
	if not check( char, tosteal ):
		return True
	if not char.canreach(tosteal, 1):
		char.socket.clilocmessage( 502703 ) # You must be standing next to an item to steal it.
		return True
	elif not tosteal.container and not tosteal.movable and tosteal.newbie:
		char.socket.clilocmessage( 502710 ) # You can't steal that!
		return True
	elif tosteal.content:
		char.socket.clilocmessage( 502710 ) # You can't steal that!
		return True
	# we try to steal an item thats not in a bag of a char
	elif not victim:
		char.socket.clilocmessage( 502710 ) # You can't steal that!
		return True
	elif tosteal.container == char.serial:
		char.socket.clilocmessage( 1005585 ) # You cannot steal items which are equiped.
		return True
	elif victim.serial == char.serial:
		char.socket.clilocmessage( 502704 ) # You catch yourself red-handed.
		return True
	elif victim.rank > char.rank:
		char.socket.clilocmessage( 502710 ) # You can't steal that!
		return True
	elif tosteal.corpse:
		char.socket.clilocmessage( 502710 ) # You can't steal that!
		return True

	dosteal( char, victim, tosteal )

def char_steal( char, victim ):
	# check if hands are empty and target can be seen
	if not check( char, victim ):
		return True
	# check for vendors
	elif isvendor( victim ):
		char.socket.clilocmessage( 1005598 ) # You can't steal from shopkeepers.
		return True
	elif isplayervendor( victim ):
		char.socket.clilocmessage( 502709 ) # You can't steal from vendors.
		return True
	elif char.serial == victim.serial:
		char.socket.sysmessage( tr( "Why don't you simply take it?" ))
		return True
	elif victim.gm:
		char.socket.sysmessage( tr( "You can't steal from game masters." ))
		return True
	elif not char.canreach(victim, 1):
		char.socket.sysmessage( tr( "You are too far away to steal from that person." ) )
		return True
	#randomsteal( char, victim )
	char.socket.sysmessage( tr("That skill has not been implemented yet.") )

# if target is char, random steal
def randomsteal(char, victim):
	if not victim:
		return True
	# this makes problems...
	bag = victim.getbackpack()
	if not bag:
		char.socket.sysmessage( tr( "Bad luck, your victim doesn't have a backpack." ) )
		return True
	# We can steal max. 10 Stones when we are a GM
	maxWeight = maxweight(char)
	containment = bag.content
	chance = len(containment)
	tosteal = None			
	while tosteal == None:
		for item in containment:
			if ( item.weight <= maxWeight and not item.lockeddown and not item.newbie and item.type != 9 ):
				if random.randint(1, int(chance)) == int(chance):
					tosteal = item
					break
	dosteal( char, victim, tosteal )

# We can steal max. 10 Stones when we are a GM by default
def maxweight(char):
	weight = char.skill[STEALING]/100
	if weight < 1:
		weight = 1
	return weight

def dosteal(char, victim, tosteal):
	char.socket.sysmessage( tr("You reach into %s's backpack and try to steal something...") % victim.name )
	stolen = None
	if not tosteal:
		return True
	char.socket.sysmessage(str(maxweight(char)))
	
	if tosteal.weight > maxweight(char):
		char.socket.sysmessage( "That is too heavy to steal." )
		return True
	if tosteal.amount > 1:
		maxAmount = int((char.skill[STEALING] / 10.0) / tosteal.weight)
		if maxAmount < 1:
			maxAmount = 1
		elif maxAmount > tosteal.amount:
			maxAmount = tosteal.amount
		amount = random.randint( 1, maxAmount )
		if amount >= tosteal.amount:
			pileWeight = int(math.ceil( tosteal.weight * tosteal.amount ) )
			pileWeight *= 10
			if char.checkskill( STEALING, pileWeight - 22.5, pileWeight + 27.5 ):
				stolen = tosteal
		else:
			pileWeight = int(math.ceil( tosteal.weight * amount ))
			pileWeight *= 10
			if char.checkskill( STEALING, pileWeight - 22.5, pileWeight + 27.5 ):
				stolen = tosteal.dupe()
				tosteal.amount -= amount;
	else:
		iw = int(math.ceil( tosteal.weight ))
		iw *= 10

		if char.checkskill( STEALING, iw - 22.5, iw + 27.5 ):
			stolen = tosteal
	if stolen:
		char.socket.clilocmessage( 502724 ) # You succesfully steal the item.
		char.getbackpack().additem(stolen)
		stolen.update()
	else:
		char.socket.clilocmessage( 502723 ) # You fail to steal the item.
		
	caught = char.skill[STEALING] < random.randint(0, 1000 )
	if caught:
		caught_func(char, victim, tosteal)

def caught_func( char, victim, tosteal ):
	char.socket.sysmessage( tr( "You have been caught!" ) )
	# Human being
	if victim.npc:
		victim.say(tr( "Guards! A thief is amoung us!" ), 0x09)
		# we must implement a callguards method available in python
	if victim.notoriety( char ) == 0x01:
		char.criminal()
	if victim.player and victim.player.socket:
		victim.socket.showspeech(tr( victim, "You notice %s trying to steal %s from you." ) % (char.name, tosteal.getname()))
	message = u"You notice %s trying to steal from %s." % (char.name, victim.name)
	# Everyone within 7 Tiles notices us
	chars =  wolfpack.chars(char.pos.x, char.pos.y, char.pos.map, 7)
	for guy in chars:
		if guy.player and guy.socket and not char:
			guy.socket.showspeech(victim, message)
	return True

def onLoad():
	skills.register( STEALING, stealing )
