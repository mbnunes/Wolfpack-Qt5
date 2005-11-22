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