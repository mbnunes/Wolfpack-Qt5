#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: khpae                          #
#   )).-' {{ ;'`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification: Created                 #
#################################################################

from wolfpack.consts import *
from wolfpack.utilities import *
import wolfpack

# from UO Stratics:
# 1. Initially, your skill affects ONLY your success in begging. Meaning, it determines whether you will be given some money or not. It does NOT determine the amount of gold you receive. If you manage to get beyond 90 skill, shopkeepers will begin to give you gold and things change a bit. 
# 2. If an NPC has 100 gold or more in its pack, it will give you 10 gold after each successful beg. Note, even if that NPC has more than 200 gold in its pack, it will still give only 10 gold pieces. 
# 3. If an NPC has 90-99 gold in its pack, it will give 9 gold pieces. 80-89 will yield 8 gold pieces, and so on. 
# 4. You can continue to beg from the same NPC until he or she runs out of gold. (editors note: or until he or she has 9 or less gold) 
# 5. If your karma gets too low, there is a chance the NPC will refuse to give you any gold at all. Where "low" means negative karma. All karma my GM Beggar had was the little he could acquire from wrestling rats in town.

GOLD_COIN = "eed"
GOLD_COIN1 = "eee"
GOLD_COIN2 = "eef"
BEGGING_RANGE = 3

def onLoad():
	wolfpack.registerglobal( HOOK_CHAR, EVENT_SKILLUSE, "skills.begging" )

def onSkillUse( char, skill ):
	if skill != BEGGING:
		return 0

	char.socket.clilocmessage( 0x7A277, "", 0x3b2, 3 )
	char.socket.attachtarget( "skills.begging.response" )
	return 1

def response( char, args, target ):
	if not char:
		return

	# you can only try to npc
	if not target.char or not target.char.npc:
		return

	npc = target.char
	# check if this npc is a 'human' ?

	# You are too far away...
	if char.canreach( npc.pos, BEGGING_RANGE ):
		# male npc
		if not npc.sex:
			char.socket.clilocmessage( 500401, "", 0x3b2, 3 )
		# female npc
		else:
			char.socket.clilocmessage( 500402, "", 0x3b2, 3 )
		return

	# town cryer : I feel sorry for thee... Thou dost not look trustworthy... no gold for thee today! : 500405 + 500406
	gold = npc.countresource( GOLD_COIN )
	if not gold or gold < 10:
		# Thou dost not look trustworthy... no gold for thee today!
		char.socket.clilocmessage( 500406, "", 0x3b2, 3, npc )
		return

	success = char.checkskill( BEGGING, 0, 1200 )

	# npc who has more than 100gp will give you 10gp
	if gold < 100:
		gold_beg = gold / 10
	else:
		gold_beg = 10

	if gold_beg == 1:
		gold_coins = GOLD_COIN
	elif gold_beg < 6:
		gold_coins = GOLD_COIN1
	else:
		gold_coins = GOLD_COIN2

	# success msg : I feel sorry for thee... here have a gold coin : 500405 + 1010012
	# fail msg : They seem unwilling to give you any money : 500404

	if success:
		char.socket.clilocmessage( 500405, "", 0x3b2, 3, npc )
		char.socket.clilocmessage( 1010012, "", 0x3b2, 3, npc )
		npc.useresource( gold_beg, 0x0eed )
		backpack = char.getbackpack()
		if not backpack:
			return
		coins = wolfpack.additem( gold_coins )
		if not coins:
			return
		coins.amount = gold_beg
		backpack.additem( coins )
		coins.update()
	else:
		char.socket.clilocmessage( 500404, "", 0x3b2, 3, npc )

