#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts
#  ((    _/{  "-;  | Created by: MagnusBr
#   )).-' {{ ;'`   | Revised by:
#  ( (  ;._ \\ ctr | Last Modification: Created
#===============================================================#
# Quest System - Player Menu Things
#===============================================================#

from wolfpack import tr
import wolfpack
from wolfpack.gumps import cGump
from wolfpack.consts import *
from quests.utils import *
from quests.functions import *

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
##############   Showing the Menu with Quest Description   ############################
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
# Response for Menu with Description
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

		dialog.addText(145, 220, "Total", 55)
		dialog.addText(260, 220, killedlist[page], 90)	
	
		dialog.addText(145, 240, "Return To", 55)
		dialog.addText(260, 240, npcdest.name + " (" + npcdest.region.name + ")", 90)
	
		if (page + 1) < npcamount:
			dialog.addButton(302, 365, 12009, 12010, 3)		# Continue
		elif itemamount:
			dialog.addButton(302, 365, 12009, 12010, 3)		# Continue

	elif (page - npcamount) < itemamount:
		dialog.addText(103, 180, "Get", 55)
		dialog.addText(145, 180, eachitemamount[page - npcamount], 90)
		dialog.addText(180, 180, giveitemname(itemlist[page - npcamount]), 1149)

		dialog.addText(145, 220, "Total", 55)
		# Trying amount
		backpack = player.getbackpack()
		itemcount = backpack.countitems( [itemlist[page - npcamount]] )
		dialog.addText(260, 220, str(itemcount), 90)
	
		dialog.addText(145, 240, "Return To", 55)
		dialog.addText(260, 240, npcdest.name + " (" + npcdest.region.name + ")", 90)

		if ((page - npcamount) + 1) < itemamount:
			dialog.addButton(302, 365, 12009, 12010, 3)		# Continue

	dialog.addButton(340, 395, 12012, 12013, 0)		# Okay
	dialog.addButton(105, 395, 12021, 12022, 1)		# Resign
	dialog.addButton(145, 365, 12015, 12016, 2)		# Previous

	dialog.setArgs( [id, slot, page] )
	dialog.setCallback( questdetailsresponse )

	dialog.send( player.socket )

#######################################################################################
# Response for Menu with Quest Details
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
