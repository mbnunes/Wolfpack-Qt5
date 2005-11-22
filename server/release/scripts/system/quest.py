#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts
#  ((    _/{  "-;  | Created by: MagnusBr
#   )).-' {{ ;'`   | Revised by:
#  ( (  ;._ \\ ctr | Last Modification: Created
#===============================================================#
# Quest System
#===============================================================#
# TAGS (Planned):
# - A Tag for the destination for a quest (UID of NPC)		- Tag.Quest.X.NPCDest
# - A Tag for the ID of the Tag					- Tag.Quest.X.ID
# - A Tag for the Qnt of NPCs required				- Tag.Quest.X.ReqNPC
#
# I have a different way for organize scripts. Expect nobody have problems with this :)

from wolfpack import tr
import wolfpack
from wolfpack.gumps import cGump
from wolfpack.consts import *

#######################################################################################
##############   Open the Quest Menu for Players   ####################################
#######################################################################################

def openquestplayer(player):
	
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
	dialog.addText(135, 49, "Quest Log", 1149)
	dialog.addTiledGump(139, 69, 161, 2, 2432, 0)
	dialog.addButton(338, 394, 12012, 12013, 0)

	# Temporary Y
	tempy = 142
	
	# Loop for Quests
	for i in range(1, 11):

		# Player has this tag?
		if player.hastag('Quest.'+ str(i) +'.ID'):
			
			questid = player.gettag('Quest.'+ str(i) +'.ID')

			# The Dialog
			dialog.addButton(391, tempy, 9904, 9905, i)
			dialog.addText(106, tempy, givequestname( questid ), 1149)

			# Increase y
			tempy += 21

	dialog.setArgs( [] )
	dialog.setCallback( questplayerresponse )

	dialog.send( player.socket )

#######################################################################################
# Response for Quest Menu
#######################################################################################

def questplayerresponse( player, args, target ):

	button = target.button

	if not button == 0:

		# We have this Quest?
		if player.hastag('Quest.'+ str(button) +'.ID'):
			questid = player.gettag('Quest.'+ str(button) +'.ID')
			showquestdescription(player, questid, button)


	return True

#######################################################################################
##############   Showing the Menu with Quest for Player   #############################
#######################################################################################

def showquestdescription(player, id, slot):

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
	dialog.addText(135, 49, "Quest Log", 1149)
	dialog.addTiledGump(139, 69, 161, 2, 2432, 0)
	dialog.addText(167, 105, givequestname(id), 175)
	dialog.addText(103, 139, "Description (quest chain)", 175)
	dialog.addButton(340, 395, 12012, 12013, 0)		# Okay
	dialog.addButton(105, 395, 12021, 12022, 1)		# Resign
	dialog.addButton(302, 365, 12009, 12010, 2)		# Continue
	dialog.addButton(145, 365, 12015, 12016, 3)		# Previous

	dialog.addHtmlGump(104, 165, 329, 173, '<basefont color="#C7D32C">' + givequestdescription(id), 0, 1)

	dialog.setArgs( [id, slot] )
	dialog.setCallback( questdescriptionresponse )

	dialog.send( player.socket )

#######################################################################################
# Response for Menu with Quest
#######################################################################################

def questdescriptionresponse( char, args, target ):

	button = target.button

	id = args[0]
	slot = args[1]

	if button == 1:
		resignquest(char, id, slot)

	elif button == 2:
		showquestdetails(char, id, slot, 0)

	elif button == 3:
		openquestplayer(char)

	return

#######################################################################################
##############   Showing Quest Details for Player   ###################################
#######################################################################################

def showquestdetails(player, id, slot, page):

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

	dialog.addText(135, 49, "Quest Log", 1149)
	dialog.addText(167, 105, givequestname(id), 175)
	dialog.addText(103, 140, "Objective:", 175)
	dialog.addText(103, 160, "All of the following", 175)

	# Data
	npcdest = wolfpack.findchar( int( player.gettag('Quest.'+ str(slot) +'.NPCDest') ) )	# NPC to report Quest Back
	npcamount = givequestnpcamounts(id)						# Amount of NPCs to be killed (Different types)
	npclist = givequestnpctargets(id)						# Required NPCs
	eachnpcamount = givequestnpceachamount(id)					# Amount of each Required NPC
	killedlist = player.gettag('Quest.'+ str(slot) +'.ReqNPC').split(',')

	# Construct dialog
	dialog.addText(103, 180, "Slay", 55)
	dialog.addText(145, 180, eachnpcamount[page], 90)
	dialog.addText(180, 180, givenpcname(npclist[page]), 1149)

	dialog.addText(145, 200, "Location", 55)
	dialog.addText(260, 200, " --- ", 1149)

	dialog.addText(145, 220, "Total", 55)
	dialog.addText(260, 220, killedlist[page], 90)	
	
	dialog.addText(145, 240, "Return To", 55)
	dialog.addText(260, 240, npcdest.name + " (" + npcdest.region.name + ")", 90)
	
	dialog.addButton(340, 395, 12012, 12013, 0)		# Okay
	dialog.addButton(105, 395, 12021, 12022, 1)		# Resign
	dialog.addButton(145, 365, 12015, 12016, 2)		# Previous

	if (page + 1) < npcamount:
		dialog.addButton(302, 365, 12009, 12010, 3)		# Continue

	dialog.setArgs( [id, slot, page] )
	dialog.setCallback( questdetailsresponse )

	dialog.send( player.socket )

#######################################################################################
# Response for Menu with Quest
#######################################################################################

def questdetailsresponse( char, args, target ):

	button = target.button

	id = args[0]
	slot = args[1]
	page = args[2]

	if button == 1:
		resignquest(char, id, slot)

	elif button == 2:
		if page == 0:
			showquestdescription(char, id, slot)
		else:
			showquestdetails(char, id, slot, page - 1)

	elif button == 3:
		showquestdetails(char, id, slot, page + 1)

	return

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
			player.sysmessage("Quest system not yet implemented")
		else:
			npcquestmenu(npc, player, questamount)

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
	
		dialog.addButton(391, int(tempy), 9904, 9905, i + 1)
		dialog.addText(106, int(tempy), givequestname( id ), 1149)

		tempy += 21

	dialog.setArgs( [quests, npc] )
	dialog.setCallback( questlistresponse )

	dialog.send( player.socket )

#######################################################################################
# Response for Quest List
#######################################################################################

def questlistresponse( char, args, target ):

	button = target.button

	npc = args[1]
	quests = args[0]

	if not button == 0:

		showmenuquest(char, int(quests[button - 1]), npc)

#######################################################################################
##############   Showing the Menu with Quest   ########################################
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
# Response for Menu with Quest
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
	
	# Construct dialog
	dialog.addText(103, 180, "Slay", 55)
	dialog.addText(145, 180, eachnpcamount[page], 90)
	dialog.addText(180, 180, givenpcname(npclist[page]), 1149)

	dialog.addText(145, 200, "Location", 55)
	dialog.addText(260, 200, " --- ", 1149)	
	
	dialog.addText(145, 240, "Return To", 55)
	dialog.addText(260, 240, npc.name + " (" + npc.region.name + ")", 90)
	
	dialog.addButton(340, 395, 12018, 12019, 0)		# Refuse
	dialog.addButton(105, 395, 12000, 12001, 1)		# Accept
	dialog.addButton(145, 365, 12015, 12016, 2)		# Previous

	if (page + 1) < npcamount:
		dialog.addButton(302, 365, 12009, 12010, 3)		# Continue

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

#######################################################################################
##############   Return a Quest Name   ################################################
#######################################################################################

def givequestname(id):
	quest = 'NULL'

	node = wolfpack.getdefinition(WPDT_QUEST, str(id))

	if node:
		count = node.childcount
		for i in range(0, count):
			subnode = node.getchild(i)
			if subnode.name == 'name':
				quest = subnode.text

		return quest

	else:

		return "Error"

#######################################################################################
##############   Give a Quest Description   ###########################################
#######################################################################################

def givequestdescription(id):
	quest = 'NULL'
	node = wolfpack.getdefinition(WPDT_QUEST, str(id))

	count = node.childcount
	for i in range(0, count):
		subnode = node.getchild(i)
		if subnode.name == 'description':
			quest = subnode.text

	return quest

#######################################################################################
##############   Give Required Quests in a list   #####################################
#######################################################################################

def givequestrequiredquests(id):
	quest = 'NULL'
	node = wolfpack.getdefinition(WPDT_QUEST, str(id))

	count = node.childcount
	for i in range(0, count):
		subnode = node.getchild(i)
		if subnode.name == 'requiredquests':
			quest = subnode.text.split(',')

	return quest

#######################################################################################
##############   Give Required NPCs in a list   #######################################
#######################################################################################

def givequestnpctargets(id):
	quest = 'NULL'
	node = wolfpack.getdefinition(WPDT_QUEST, str(id))

	count = node.childcount
	for i in range(0, count):
		subnode = node.getchild(i)
		if subnode.name == 'npctargets':
			quest = subnode.text.split(',')

	return quest

#######################################################################################
##############   Give amount of each Required NPC in a list   #########################
#######################################################################################

def givequestnpceachamount(id):
	quest = 'NULL'
	node = wolfpack.getdefinition(WPDT_QUEST, str(id))

	count = node.childcount
	for i in range(0, count):
		subnode = node.getchild(i)
		if subnode.name == 'npctargetsamounts':
			quest = subnode.text.split(',')

	# Returning the list
	return quest

#######################################################################################
##############   Give amount of Required NPCs   #######################################
#######################################################################################

def givequestnpcamounts(id):
	quest = 'NULL'
	node = wolfpack.getdefinition(WPDT_QUEST, str(id))

	count = node.childcount
	for i in range(0, count):
		subnode = node.getchild(i)
		if subnode.name == 'npctargets':
			quest = subnode.text.split(',')

	# Returning the Amount
	return len( quest )

#######################################################################################
##############   Give Rewards for Quest in a List   ###################################
#######################################################################################

def givequestrewards(id):
	quest = 'NULL'
	node = wolfpack.getdefinition(WPDT_QUEST, str(id))

	count = node.childcount
	for i in range(0, count):
		subnode = node.getchild(i)
		if subnode.name == 'rewarditems':
			quest = subnode.text.split(',')

	return quest

#######################################################################################
##############   Return a Quest name by an ID Slot of a player   ######################
#######################################################################################

def givequestnameplayer(char, id):

	if not char.hastag('Quest.'+ str(id) +'.ID'):
		return "No Quest"
	else:
		questid = char.gettag('Quest.'+ str(id) +'.ID')
		return givequestname(questid)

#######################################################################################
##############   Give a Quest to the Player   #########################################
#######################################################################################

def givequesttoplayer(player, id, npc):

	freeslot = 0		# Flag
	
	for i in range(1, 11):

		if not player.hastag('Quest.'+ str(i) +'.ID'):
			freeslot = 1		# Yeah! We founded a Free Slot

			#
			# ID of the Quest
			#
			player.settag('Quest.'+ str(i) +'.ID', id)

			#
			# NPC to report Quest back
			#
			player.settag('Quest.'+ str(i) +'.NPCDest', str(npc.serial))

			#
			# NPCs Required
			#
			cont = len( givequestnpctargets(id) )

			# Just lets assign if Cont is Higher than 0
			if cont > 0:
				questlist = "0"			# Assign the First 0

				for j in range(1, cont):
					questlist += ",0"

			
				player.settag('Quest.'+ str(i) +'.ReqNPC', str(questlist))

			break
		
	
	if freeslot == 0:
		player.socket.sysmessage("You have no Free Slots for this Quest")

#######################################################################################
##############   Resign a Quest   #####################################################
#######################################################################################

def resignquest(player, id, slot):

	if not player.hastag('Quest.'+ str(slot) +'.ID'):
		player.socket.sysmessage("We have an error in quest system. You resign a Quest that you dont own")
	else:
		player.deltag('Quest.'+ str(slot) +'.ID')
		player.deltag('Quest.'+ str(slot) +'.NPCDest')
		player.deltag('Quest.'+ str(slot) +'.ReqNPC')
		player.socket.sysmessage("You resigned the selected quest...")

#######################################################################################
#############   Check if a Dead NPC is a NPC required for a Quest   ###################
#######################################################################################

def checknpcforquest(player, dead):
	
	for i in range(1, 11):

		#
		# Check if Player have a Requested NPC here
		#
		if player.hastag('Quest.'+ str(i) +'.ReqNPC'):

			#
			# Check if this NPC is one of targets
			#
			id = player.gettag('Quest.'+ str(i) +'.ID')				# Get ID of this Quest

			npclist = givequestnpctargets(id).split(',')				# Get the List of NPCs
			npcamount = player.gettag('Quest.'+ str(i) +'.ReqNPC').split(',')	# Get the Amount List for NPCs

			for j in range( 0, len(npclist) ):
				
				# Oh! Is this the NPC?
				if dead.id == npclist[j]:

					npcamount[j] += 1			# Increase 1

					player.settag('Quest.'+ str(i) +'.ReqNPC', npcamount)	# Save Tag again

#######################################################################################
##############   Return a NPC Name   ##################################################
#######################################################################################

def givenpcname(id):
	npc = 'NULL'

	node = wolfpack.getdefinition(WPDT_NPC, str(id))

	if node:
		count = node.childcount
		for i in range(0, count):
			subnode = node.getchild(i)
			if subnode.name == 'name':
				npc = subnode.text

		return npc

	else:

		return "Error"