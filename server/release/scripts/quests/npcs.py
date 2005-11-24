#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts
#  ((    _/{  "-;  | Created by: MagnusBr
#   )).-' {{ ;'`   | Revised by:
#  ( (  ;._ \\ ctr | Last Modification: Created
#===============================================================#
# Quest System - NPC and NPC menu Things
#===============================================================#

from wolfpack import tr
import wolfpack
from wolfpack.gumps import cGump
from wolfpack.consts import *
from quests.utils import *
from quests.functions import *

#######################################################################################
##############   Open the Choice Menu for Quest Action   ##############################
#######################################################################################

def openchoicequestmenu(char, player):

	socket = player.socket

	dialog = cGump( nomove=1, x=100, y=30 )

	dialog.addResizeGump(7, 6, 9400, 253, 122)
	dialog.addText(81, 17, "Quest System", 1149)
	dialog.addButton(30, 55, 1209, 1210, 1)
	dialog.addButton(30, 90, 1209, 1210, 2)
	dialog.addText(56, 50, "List Quests", 1149)
	dialog.addText(56, 85, "Report Completed Quest", 1149)

	dialog.setArgs( [char] )
	dialog.setCallback( questchoiceresponse )

	dialog.send( player.socket )

#######################################################################################
# Response for Quest List from a NPC
#######################################################################################

def questchoiceresponse( char, args, target ):

	button = target.button

	npc = args[0]

	if button == 1:

		npcquestmain(npc, char)

	elif button == 2:

		npcquestreport(npc, char)

#######################################################################################
##############   Main Function for Context Menu Option   ##############################
#######################################################################################

def npcquestmain(npc, player):

	# Amount of Quests
	questamount = returnamountquests(npc)

	if not questamount:
		npc.say("Sorry, I have no Quests for you")
	else:
		if not questamount > 1:
			id = npc.gettag('quests')

			if checkhaverequiredquests(player, id):
				if not checkifquestcompleted(player, id):
					showmenuquest(player, id, npc)
				else:
					npc.say("You already completed all my tasks. Thanks!")
			else:
				npc.say("Sorry, I have no Quests that you can handle yet")
		else:
			npcquestmenu(npc, player, questamount)

#######################################################################################
##############   Function for Quest Report   ##########################################
#######################################################################################

def npcquestreport(npc, player):

	# Amount of Quests of this NPC
	questamount = 0

	# Loop for Quests
	for i in range(1, 11):

		if player.hastag('Quest.'+ str(i) +'.NPCDest'):

			serialdest = player.gettag('Quest.'+ str(i) +'.NPCDest')

			if str(npc.serial) == str(serialdest):

				questamount += 1

	# Message
	if not questamount:
		player.socket.sysmessage("You have no assigned quests from this NPC")

	else:
		npcreportlist(npc, player)

#######################################################################################
##############   Quest list from a NPC   ##############################################
#######################################################################################

def npcquestmenu(npc, player, questamount):
	
	socket = player.socket

	dialog = cGump( nomove=1, x=100, y=30 )

	dialog.addResizeGump(37, 14, 9260, 445, 422)
	dialog.addTiledGump(81, 35, 358, 386, 5124, 0)
	dialog.addTiledGump(55, 32, 30, 391, 10460, 0)
	dialog.addTiledGump(439, 32, 30, 391, 10460, 0)
	dialog.addGump(-12, 290, 10402, 0)
	dialog.addGump(53, 137, 10411, 0)
	dialog.addGump(95, 39, 9005, 0)
	dialog.addTiledGump(40, 421, 440, 17, 10100, 0)
	dialog.addTiledGump(39, 15, 440, 17, 10100, 0)
	dialog.addGump(4, 16, 10421, 0)
	dialog.addGump(21, 4, 10420, 0)
	dialog.addGump(449, 162, 10411, 0)
	dialog.addGump(449, 323, 10412, 0)
	dialog.addGump(394, 52, 1417, 0)
	dialog.addGump(449, 3, 10410, 0)
	dialog.addGump(402, 61, 9012, 0)
	dialog.addText(135, 49, "Quest List", 1149)
	dialog.addText(136, 79, npc.name + ", from: " + npc.region.name, 1149)
	dialog.addTiledGump(139, 69, 161, 2, 2432, 0)
	dialog.addButton(338, 394, 12012, 12013, 0)

	# Temporary storage
	tempy = 142

	# Loop for Quests
	quests = returnquestlist(npc)

	for i in range( 0, questamount ):

		id = str( quests[i] )

		if checkhaverequiredquests(player, id):

			if not checkifquestcompleted(player, id):
				dialog.addText(106, int(tempy), givequestname( id ), 1149)
				dialog.addButton(391, int(tempy), 9904, 9905, i + 1)
			else:
				dialog.addText(106, int(tempy), givequestname( id ), 908)
				dialog.addGump(391, int(tempy), 9903, 0)

			tempy += 21

	dialog.setArgs( [quests, npc] )
	dialog.setCallback( questlistresponse )

	dialog.send( player.socket )

#######################################################################################
# Response for Quest List from a NPC
#######################################################################################

def questlistresponse( char, args, target ):

	button = target.button

	npc = args[1]
	quests = args[0]

	if not button == 0:

		showmenuquest(char, int(quests[button - 1]), npc)

#######################################################################################
##############   Showing Quest Description from a NPC   ###############################
#######################################################################################

def showmenuquest(player, id, npc):

	socket = player.socket

	dialog = cGump( nomove=1, x=100, y=30 )

	dialog.addResizeGump(37, 14, 9260, 445, 422)
	dialog.addTiledGump(81, 35, 358, 386, 5124, 0)
	dialog.addTiledGump(55, 32, 30, 391, 10460, 0)
	dialog.addTiledGump(439, 32, 30, 391, 10460, 0)
	dialog.addGump(-12, 290, 10402, 0)
	dialog.addGump(53, 137, 10411, 0)
	dialog.addGump(95, 39, 9005, 0)
	dialog.addTiledGump(40, 421, 440, 17, 10100, 0)
	dialog.addTiledGump(39, 15, 440, 17, 10100, 0)
	dialog.addGump(4, 16, 10421, 0)
	dialog.addGump(21, 4, 10420, 0)
	dialog.addGump(449, 162, 10411, 0)
	dialog.addGump(449, 323, 10412, 0)
	dialog.addGump(394, 52, 1417, 0)
	dialog.addGump(449, 3, 10410, 0)
	dialog.addGump(402, 61, 9012, 0)
	dialog.addText(135, 49, "Quest Offer", 1149)
	dialog.addTiledGump(139, 69, 161, 2, 2432, 0)

	dialog.addText(167, 105, givequestname(id), 175)
	dialog.addText(103, 139, "Description (quest chain)", 175)

	dialog.addButton(340, 395, 12018, 12019, 0)		# Refuse
	dialog.addButton(105, 395, 12000, 12001, 1)		# Accept
	dialog.addButton(302, 365, 12009, 12010, 2)		# Continue

	dialog.addHtmlGump(104, 165, 329, 173, '<basefont color="#C7D32C">' + givequestdescription(id), 0, 1)

	dialog.setArgs( [npc, id] )
	dialog.setCallback( questshowresponse )

	dialog.send( player.socket )

#######################################################################################
# Response for Quest Description from a NPC
#######################################################################################

def questshowresponse( char, args, target ):

	button = target.button

	npc = args[0]
	id = args[1]

	if button == 1:

		givequesttoplayer(char, id, npc)

	if button == 2:

		showquestdetailsnpc(char, id, npc, 0)

	return

#######################################################################################
##############   Showing Quest Details from NPC   #####################################
#######################################################################################

def showquestdetailsnpc(player, id, npc, page):

	socket = player.socket

	dialog = cGump( nomove=1, x=100, y=30 )

	dialog.addResizeGump(37, 14, 9260, 445, 422)
	dialog.addTiledGump(81, 35, 358, 386, 5124, 0)
	dialog.addTiledGump(55, 32, 30, 391, 10460, 0)
	dialog.addTiledGump(439, 32, 30, 391, 10460, 0)
	dialog.addGump(-12, 290, 10402, 0)
	dialog.addGump(53, 137, 10411, 0)
	dialog.addGump(95, 39, 9005, 0)
	dialog.addTiledGump(40, 421, 440, 17, 10100, 0)
	dialog.addTiledGump(39, 15, 440, 17, 10100, 0)
	dialog.addGump(4, 16, 10421, 0)
	dialog.addGump(21, 4, 10420, 0)
	dialog.addGump(449, 162, 10411, 0)
	dialog.addGump(449, 323, 10412, 0)
	dialog.addGump(394, 52, 1417, 0)
	dialog.addGump(449, 3, 10410, 0)
	dialog.addGump(402, 61, 9012, 0)
	dialog.addTiledGump(139, 69, 161, 2, 2432, 0)

	dialog.addText(135, 49, "Quest Offer", 1149)
	dialog.addText(167, 105, givequestname(id), 175)
	dialog.addText(103, 140, "Objective:", 175)
	dialog.addText(103, 160, "All of the following", 175)

	# Data
	npcamount = givequestnpcamounts(id)						# Amount of NPCs to be killed (Different types)
	npclist = givequestnpctargets(id)						# Required NPCs
	eachnpcamount = givequestnpceachamount(id)					# Amount of each Required NPC
	
	itemamount = givequestitemamounts(id)						# Amount of Items to be killed (Different types)
	itemlist = givequestitemtargets(id)						# Required Items
	eachitemamount = givequestitemeachamount(id)					# Amount of each Required Item
	
	# Construct dialog
	if page < npcamount:
		dialog.addText(103, 180, "Slay", 55)
		dialog.addText(145, 180, eachnpcamount[page], 90)
		dialog.addText(180, 180, givenpcname(npclist[page]), 1149)

		dialog.addText(145, 200, "Location", 55)
		dialog.addText(260, 200, " --- ", 1149)	
	
		dialog.addText(145, 240, "Return To", 55)
		dialog.addText(260, 240, npc.name + " (" + npc.region.name + ")", 90)

		if (page + 1) < npcamount:
			dialog.addButton(302, 365, 12009, 12010, 3)		# Continue
		elif itemamount:
			dialog.addButton(302, 365, 12009, 12010, 3)		# Continue
	
	elif (page - npcamount) < itemamount:
		dialog.addText(103, 180, "Get", 55)
		dialog.addText(145, 180, eachitemamount[page - npcamount], 90)
		dialog.addText(180, 180, giveitemname(itemlist[page - npcamount]), 1149)
	
		dialog.addText(145, 240, "Return To", 55)
		dialog.addText(260, 240, npc.name + " (" + npc.region.name + ")", 90)

		if ((page - npcamount) + 1) < itemamount:
			dialog.addButton(302, 365, 12009, 12010, 3)		# Continue
		
	dialog.addButton(340, 395, 12018, 12019, 0)		# Refuse
	dialog.addButton(105, 395, 12000, 12001, 1)		# Accept
	dialog.addButton(145, 365, 12015, 12016, 2)		# Previous

	dialog.setArgs( [id, npc, page] )
	dialog.setCallback( questdetailsresponsenpc )

	dialog.send( player.socket )

#######################################################################################
# Response for Menu with Quest Details from NPC
#######################################################################################

def questdetailsresponsenpc( char, args, target ):

	button = target.button

	id = args[0]
	npc = args[1]
	page = args[2]

	if button == 1:
		givequesttoplayer(char, id, npc)

	elif button == 2:
		if page == 0:
			showmenuquest(char, id, npc)
		else:
			showquestdetailsnpc(char, id, npc, page - 1)

	elif button == 3:
		showquestdetailsnpc(char, id, npc, page + 1)

	return

#######################################################################################
##############   Quest list from a NPC   ##############################################
#######################################################################################

def npcreportlist(npc, player):
	
	socket = player.socket

	dialog = cGump( nomove=1, x=100, y=30 )

	dialog.addResizeGump(37, 14, 9260, 445, 422)
	dialog.addTiledGump(81, 35, 358, 386, 5124, 0)
	dialog.addTiledGump(55, 32, 30, 391, 10460, 0)
	dialog.addTiledGump(439, 32, 30, 391, 10460, 0)
	dialog.addGump(-12, 290, 10402, 0)
	dialog.addGump(53, 137, 10411, 0)
	dialog.addGump(95, 39, 9005, 0)
	dialog.addTiledGump(40, 421, 440, 17, 10100, 0)
	dialog.addTiledGump(39, 15, 440, 17, 10100, 0)
	dialog.addGump(4, 16, 10421, 0)
	dialog.addGump(21, 4, 10420, 0)
	dialog.addGump(449, 162, 10411, 0)
	dialog.addGump(449, 323, 10412, 0)
	dialog.addGump(394, 52, 1417, 0)
	dialog.addGump(449, 3, 10410, 0)
	dialog.addGump(402, 61, 9012, 0)
	dialog.addText(135, 49, "Quest Report", 1149)
	dialog.addText(136, 79, npc.name + ", from: " + npc.region.name, 1149)
	dialog.addTiledGump(139, 69, 161, 2, 2432, 0)
	dialog.addButton(338, 394, 12012, 12013, 0)

	# Temporary storage
	tempy = 142

	# Loop for Quests
	for i in range(1, 11):

		if player.hastag('Quest.'+ str(i) +'.NPCDest'):

			serialdest = player.gettag('Quest.'+ str(i) +'.NPCDest')

			if str(npc.serial) == str(serialdest):

				id = player.gettag('Quest.'+ str(i) +'.ID')
	
				dialog.addButton(391, int(tempy), 9904, 9905, i)
				dialog.addText(106, int(tempy), givequestname( id ), 1149)

		tempy += 21

	dialog.setArgs( [npc] )
	dialog.setCallback( npcreportresponse )

	dialog.send( player.socket )

#######################################################################################
# Response for Quest List from a NPC
#######################################################################################

def npcreportresponse( char, args, target ):

	button = target.button

	npc = args[0]

	if not button == 0:

		reportquestnpc(char, npc, button)

#######################################################################################
##############   Return a List with quests for this NPC   #############################
#######################################################################################

def returnquestlist(npc):
	
	if not npc.hastag('quests'):
		return 0
	else:
		# Picking Quests and making a list
		quests = npc.gettag('quests')
		questlist = quests.split(',')

		# Returning the amount
		return questlist

#######################################################################################
##############   Return How many Quests a NPC Have   ##################################
#######################################################################################

def returnamountquests(npc):
	
	if not npc.hastag('quests'):
		return 0
	else:
		# Picking Quests and making a list
		quests = npc.gettag('quests')
		questlist = quests.split(',')

		# Returning the amount
		return len( questlist )