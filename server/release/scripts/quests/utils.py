#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts
#  ((    _/{  "-;  | Created by: MagnusBr
#   )).-' {{ ;'`   | Revised by:
#  ( (  ;._ \\ ctr | Last Modification: Created
#===============================================================#
# Quest System - Utils
#===============================================================#

from wolfpack import tr
import wolfpack
from wolfpack.gumps import cGump
from wolfpack.consts import *

from quests.functions import MAXQUESTSPERTAG

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
##############   Give the number of required quests   #################################
#######################################################################################

def giveamountofrequiredquests(id):
	quest = ''
	node = wolfpack.getdefinition(WPDT_QUEST, str(id))

	count = node.childcount
	for i in range(0, count):
		subnode = node.getchild(i)
		if subnode.name == 'requiredquests':
			quest = subnode.text.split(',')

	return len(quest)

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
	quest = ''
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
##############   Give amount of each Rewards for Quest in a List   ####################
#######################################################################################

def givequesteachrewardsamount(id):
	quest = 'NULL'
	node = wolfpack.getdefinition(WPDT_QUEST, str(id))

	count = node.childcount
	for i in range(0, count):
		subnode = node.getchild(i)
		if subnode.name == 'rewardsamount':
			quest = subnode.text.split(',')

	return quest

#######################################################################################
##############   Give amount of Rewards for Quest   ###################################
#######################################################################################

def givequestrewardsamount(id):
	quest = 'NULL'
	node = wolfpack.getdefinition(WPDT_QUEST, str(id))

	count = node.childcount
	for i in range(0, count):
		subnode = node.getchild(i)
		if subnode.name == 'rewarditems':
			quest = subnode.text.split(',')

	# Returning the amount
	return len( quest )

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
##############   Check if a Player have the Required Quests   #########################
#######################################################################################

def checkhaverequiredquests(player, id):
	
	# Get Required Quests for this quest and Amount of requireds
	requiredquests = givequestrequiredquests(id)
	amountofrequired = giveamountofrequiredquests(id)
	# Just a Flag
	requiredok = 0

	if amountofrequired:
		# Player things
		tagsection = int( int(id) / int(MAXQUESTSPERTAG) )
		if player.hastag('quest.'+ str(tagsection) +'.complete'):
			# Assign Quests from player
			quests = str(player.gettag('quest.'+ str(tagsection) +'.complete')).split(',')
			# Loop
			for i in range(0, amountofrequired):
				for j in range(0, len(quests)):
					if str(requiredquests[i]) == str(quests[j]):
						requiredok += 1

		if not requiredok == amountofrequired:
			return False
		else:
			return True
	else:
		return True

#######################################################################################
##############   Check if a Player already completed a quest   ########################
#######################################################################################

def checkifquestcompleted(player, id):

	tagsection = int( int(id) / int(MAXQUESTSPERTAG) )
	
	if player.hastag('quest.'+ str(tagsection) +'.complete'):
		quests = str(player.gettag('quest.'+ str(tagsection) +'.complete')).split(',')

		for i in range(0, len(quests)):
			if int(quests[i]) == int(id):
				return True
			else:
				return False
	else:
		return False

##########################################################################################################
###################   Now things for other systems and not for Quest object   ############################
##########################################################################################################

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