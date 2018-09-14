#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: khpae                          #
#   )).-' {{ ;'`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification: Created                 #
#################################################################

from wolfpack.consts import *
from wolfpack.utilities import *
import wolfpack
import wolfpack.time
import skills
# from UO Stratics:
# 1. Initially, your skill affects ONLY your success in begging. Meaning, it determines whether you will be given some money or not. It does NOT determine the amount of gold you receive. If you manage to get beyond 90 skill, shopkeepers will begin to give you gold and things change a bit.
# 2. If an NPC has 100 gold or more in its pack, it will give you 10 gold after each successful beg. Note, even if that NPC has more than 200 gold in its pack, it will still give only 10 gold pieces.
# 3. If an NPC has 90-99 gold in its pack, it will give 9 gold pieces. 80-89 will yield 8 gold pieces, and so on.
# 4. You can continue to beg from the same NPC until he or she runs out of gold. (editors note: or until he or she has 9 or less gold)
# 5. If your karma gets too low, there is a chance the NPC will refuse to give you any gold at all. Where "low" means negative karma. All karma my GM Beggar had was the little he could acquire from wrestling rats in town.

GOLD_COIN = "eed"
GOLD_COIN1 = "eed"
BEGGING_RANGE = 3

BEGGING_DELAY = 5000

def begging( char, skill ):
	if skill != BEGGING:
		return False

	if char.socket.hastag( 'skill_delay' ):
		if wolfpack.time.currenttime() < char.socket.gettag( 'skill_delay' ):
			char.socket.clilocmessage( 500118, "", 0x3b2, 3 )
			return True
		else:
			char.socket.deltag( 'skill_delay' )

	char.socket.clilocmessage( 500397, "", 0x3b2, 3 )

	char.socket.attachtarget( "skills.begging.response" )
	return True

def response( char, args, target ):
	if not char:
		return

	if skills.skilltable[ BEGGING ][ skills.UNHIDE ] and char.hidden:
		char.reveal()

	message = 0
	if target.char:
		# can't beg from players
		if target.char.player:
			message = 500398 # Perhaps just asking would work better.
		# Make sure the npc is human
		elif not target.char.bodytype == 4:
			message = 500399 # There is little chance of getting money from that!
		# You are too far away...
		elif not char.canreach( target.char, BEGGING_RANGE ):
			# male npc
			if not target.char.gender:
				char.socket.clilocmessage( 500401, "", 0x3b2, 3 ) # You are too far away to beg from him.
			# female npc
			else:
				char.socket.clilocmessage( 500402, "", 0x3b2, 3 ) # You are too far away to beg from her.
		# If we're on a mount, who would give us money?
		elif char.itemonlayer( 25 ):
			message = 500404 # They seem unwilling to give you any money.
		else:
			# Face each other
			char.turnto( target.char )
			target.char.turnto( char )
			char.action( 32 ) # Bow
			char.addtimer( 2000, getmoney, [target.char.serial] )
	else: # not a char
		message = 500399 # There is little chance of getting money from that!

	if message:
		char.socket.clilocmessage( message )

	return True

def getmoney( char, args ):
	npc = wolfpack.findchar( args[0] )
	if not npc:
		return False

	char.socket.settag( 'skill_delay', int( wolfpack.time.currenttime() + BEGGING_DELAY ) )

	theirPack = npc.getbackpack()
	badKarmaChance = 0.5 - float(char.karma / 8570)

	if not theirPack:
		char.socket.clilocmessage( 500404 ) # They seem unwilling to give you any money.
	elif char.karma < 0 and badKarmaChance > random.random():
		npc.say( 500406, "" ) # Thou dost not look trustworthy... no gold for thee today!
	elif char.checkskill( BEGGING, 0, 1000 ):
		toConsume = npc.countresource( wolfpack.utilities.hex2dec( 0xeed ) ) / 10
		max = 10 + (char.fame / 2500)
		if max > 14:
			max = 14
		elif max < 10:
			max = 10

		if toConsume > max:
			toConsume = max
		if toConsume > 0:
			consumed = npc.useresource( toConsume, 0x0eed )
			if consumed > 0:
				npc.say( 500405 ) # I feel sorry for thee...
				coins = wolfpack.additem( GOLD_COIN1 )
				coins.amount = toConsume
				if not tobackpack(coins, char):
					coins.update()
				coins.update()
				sound = 0
				if toConsume <= 1:
					sound = 0x2e4
				elif toConsume <= 5:
					sound = 0x2e5
				else:
					sound = 0x2e6
				char.soundeffect(sound)
				if char.karma > -3000:
					toLose = char.karma + 3000

					if toLose > 40:
						toLose = 40
					char.awardkarma( -toLose )
			else:
				npc.say( 500407 ) # I have not enough money to give thee any!
		else: 
			npc.say( 500407 ) # I have not enough money to give thee any!
	else:
		npc.say( 500404 ) # They seem unwilling to give you any money.

	return True

def onLoad():
	skills.register( BEGGING, begging )
