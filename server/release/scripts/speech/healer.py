#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts			#
#  ((    _/{  "-;  # Created by: Naddel				#
#   )).-' {{ ;'`   # Revised by:				#
#  ( (  ;._ \\ ctr # Last Modification:				#
#################################################################
import wolfpack.time
from wolfpack.gumps import cGump

RESURRECT_DELAY = 2000 # 2 sec.
YOUNG_HEAL_TIME = 60 * 5000 # 5 min.
priced_healer = ['priced_healer_male', 'priced_healer_female']

def onSeeChar(npc, char):
	if not char.socket or char.socket.hastag('healerresurrect_gump'):
		return False
	if char.socket.hastag( 'resurrect_delay' ):
		if wolfpack.time.currenttime() < char.socket.gettag( 'resurrect_delay' ):
			return False
		else:
			char.socket.deltag( 'skill_delay' )

	if not char.frozen and npc.distanceto(char) <= 4:# and npc.cansee(char):
		if char.dead:
			char.socket.settag( 'resurrect_delay', int( wolfpack.time.currenttime() + RESURRECT_DELAY ) )
			if not char.pos.validspawnspot():
				char.socket.clilocmessage(502391) # Thou can not be resurrected there!
			elif checkResurrect(npc, char):
				offerResurrection(npc, char)
				char.socket.settag('healerresurrect_gump', 0)

			#
			# We need a .young property before we can enable this
			#
			#elif HealsYoungPlayers(npc) and char.hitpoints < char.maxhitpoints and char.player: # and char.young:
			#	offerHeal( npc, char )
	return False

def HealsYoungPlayers(healer):
	if healer.getintproperty('price', 0):
		return False
	return True

# Just for overriding
def checkResurrect(healer, char):
	return True

def offerResurrection(healer, char):
	char.turnto(healer)
	if not healer.baseid in priced_healer: # Priced healers don't say anything
		healer.say( 501224 ) # Thou hast strayed from the path of virtue, but thou still deservest a second chance.
	char.soundeffect(0x214)
	char.effect(0x376A, 10, 16)
	resurrectGump(healer, char, 2)

def offerHeal(healer, char):
	char.turnto(healer)

	if CheckYoungHealTime(char):
		char.socket.settag( 'young_heal_delay', int( wolfpack.time.currenttime() + YOUNG_HEAL_TIME ) )
		healer.say( 501229 ) # You look like you need some healing my child.

		char.soundeffect(0x1F2)
		char.effect(0x376A, 9, 32)

		char.hitpoints = char.maxhitpoints
	else:
		healer.say(501228) # I can do no more for you at this time.
	return True

def CheckYoungHealTime(char):
	if char.socket.hastag( 'young_heal_delay' ):
		if wolfpack.time.currenttime() < char.socket.gettag( 'young_heal_delay' ):
			return False
		else:
			char.socket.deltag( 'young_heal_delay' )
	return True

"""
	msg:
	ChaosShrine = 0,
	VirtueShrine = 1,
	Healer = 2,
	Generic = 3,
"""
def resurrectGump(healer, char, msg):
	dialog = cGump( 0, 0, 0, 0, 40 )
	dialog.setCallback(response1)
	dialog.setType(0x3f1dcc97)
	dialog.startPage(0)

	dialog.addBackground(2600, 400, 350)

	dialog.addXmfHtmlGump(0, 20, 400, 35, 1011022, 0,0 ) # <center>Resurrection</center>
	dialog.addXmfHtmlGump(50, 55, 300, 140, 1011023 + int(msg), 1, 1 ) # It is possible for you to be resurrected here by this healer. Do you wish to try?<br>
										# * CONTINUE - You chose to try to come back to life now.<br>
										# * CANCEL - You prefer to remain a ghost for now.

	dialog.addButton( 200, 227, 4005, 4007, 0 )
	dialog.addXmfHtmlGump(235, 230, 110, 35, 1011012, 0,0 ) # CANCEL

	dialog.addButton(65, 227, 4005, 4007, 1) # Okay
	dialog.addXmfHtmlGump(100, 230, 110, 35, 1011011, 0,0 ) # CONTINUE

	dialog.send(char.socket)

def response1( player, args, choice ):
	price = 0
	if args and args[0]:
		price = args[0]
	if choice.button == 0:
		return False
	if not player.dead:
		return False
	player.socket.sysmessage(str(choice.button))
	if choice.button in [1, 2]:
		if not player.pos.validspawnspot():
			player.socket.clilocmessage(502391) # Thou can not be resurrected there!
			return False
		if price > 0:
			if 1 in response.switches:
				bank = player.getbankbox()
				amount = bank.countresource(0xeed, 0x0)

 				if bank.countresource(0xeed, 0x0):
					bank.useresource(price, 0xeed, 0x0)
					player.socket.clilocmessage( 1060398, str(price) ) # ~1_AMOUNT~ gold has been withdrawn from your bank box.
					player.socket.clilocmessage( 1060022, str(amount) ) # You have ~1_AMOUNT~ gold in cash remaining in your bank box.
				else:
					player.socket.clilocmessage( 1060020 ) # Unfortunately, you do not have enough cash in your bank to cover the cost of the healing.
					return
			else:
				player.socket.clilocmessage( 1060019 ) # You decide against paying the healer, and thus remain dead.
				return

		player.soundeffect( 0x214 )
		player.effect( 0x376A, 10, 16 )

		player.resurrect()
	return True
