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

from quests import MAXQUESTSPERTAG

#######################################################################################################
#######################################################################################################
#######################     BASICS FOR QUESTS     #####################################################
#######################################################################################################
#######################################################################################################

#######################################################################################
##############   Return a Quest Name   ################################################
#######################################################################################

def givequestname(id):
	quest = ''

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
##############   Return a Quest Description   #########################################
#######################################################################################

def givequestdescription(id):
	quest = ''
	node = wolfpack.getdefinition(WPDT_QUEST, str(id))

	count = node.childcount
	for i in range(0, count):
		subnode = node.getchild(i)
		if subnode.name == 'description':
			quest = subnode.text

	return quest

#######################################################################################################
#######################################################################################################
#######################     REQUIRED PARAMETERS TO ASSIGN QUEST     ###################################
#######################################################################################################
#######################################################################################################

#######################################################################################
##############   Give Required Quests in a list   #####################################
#######################################################################################

def givequestrequiredquests(id):
	quest = ''
	node = wolfpack.getdefinition(WPDT_QUEST, str(id))

	count = node.childcount
	for i in range(0, count):
		subnode = node.getchild(i)
		if subnode.name == 'requiredquests':
			if len(subnode.text):
				quest = subnode.text.split(',')

	return quest

#######################################################################################
##############   Give Required Skills in a list   #####################################
#######################################################################################

def givequestrequiredskills(id):
	skills = ''
	node = wolfpack.getdefinition(WPDT_QUEST, str(id))

	count = node.childcount
	for i in range(0, count):
		subnode = node.getchild(i)
		if subnode.name == 'requiredskills':
			if len(subnode.text):
				skills = subnode.text.split(',')

	return skills

#######################################################################################
##############   Give Required Skills amount in a list   ##############################
#######################################################################################

def givequestrequiredskillamounts(id):
	skillamounts = ''
	node = wolfpack.getdefinition(WPDT_QUEST, str(id))

	count = node.childcount
	for i in range(0, count):
		subnode = node.getchild(i)
		if subnode.name == 'requiredskillamounts':
			if len(subnode.text):
				skillamounts = subnode.text.split(',')

	return skillamounts

#######################################################################################
##############   Give Required Classes in a list   ####################################
#######################################################################################
# Exclusive for Class Shards

def givequestrequiredclasses(id):
	quest = ''
	node = wolfpack.getdefinition(WPDT_QUEST, str(id))

	count = node.childcount
	for i in range(0, count):
		subnode = node.getchild(i)
		if subnode.name == 'requiredclasses':
			if len(subnode.text):
				quest = subnode.text.split(',')

	return quest

#######################################################################################
##############   Give Required Races in a list   ######################################
#######################################################################################
# Exclusive for Race Shards

def givequestrequiredraces(id):
	quest = ''
	node = wolfpack.getdefinition(WPDT_QUEST, str(id))

	count = node.childcount
	for i in range(0, count):
		subnode = node.getchild(i)
		if subnode.name == 'requiredraces':
			if len(subnode.text):
				quest = subnode.text.split(',')

	return quest

#######################################################################################
##############   Give Required Level   ################################################
#######################################################################################
# Exclusive for XP and Level Shards

def givequestrequiredlevel(id):
	quest = 0
	node = wolfpack.getdefinition(WPDT_QUEST, str(id))

	count = node.childcount
	for i in range(0, count):
		subnode = node.getchild(i)
		if subnode.name == 'requiredlevel':
			if len(subnode.text):
				quest = subnode.text

	return int(quest)

#######################################################################################################
#######################################################################################################
#######################     REWARDS FOR QUEST     #####################################################
#######################################################################################################
#######################################################################################################

#######################################################################################
##############   Give Rewards for Quest in a List   ###################################
#######################################################################################

def givequestrewards(id):
	quest = ''
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
	quest = ''
	node = wolfpack.getdefinition(WPDT_QUEST, str(id))

	count = node.childcount
	for i in range(0, count):
		subnode = node.getchild(i)
		if subnode.name == 'rewardsamount':
			quest = subnode.text.split(',')

	return quest

#######################################################################################
##############   Give amount of each Rewards for Quest in a List   ####################
#######################################################################################
# Exclusive for XP and Level Shards

def givequestxpreward(id):
	quest = ''
	node = wolfpack.getdefinition(WPDT_QUEST, str(id))

	count = node.childcount
	for i in range(0, count):
		subnode = node.getchild(i)
		if subnode.name == 'rewardxp':
			quest = subnode.text

	return int(quest)

#######################################################################################################
#######################################################################################################
#######################     FUNCTIONS FOR EACH QUEST     ##############################################
#######################################################################################################
#######################################################################################################

#######################################################################################
##############   Return the Assign Function for this Quest   ##########################
#######################################################################################

def givequestassignfunction(id):
	quest = ''
	node = wolfpack.getdefinition(WPDT_QUEST, str(id))

	count = node.childcount
	for i in range(0, count):
		subnode = node.getchild(i)
		if subnode.name == 'functionassign':
			quest = subnode.text

	return quest

#######################################################################################
##############   Return the Resign Function for this Quest   ##########################
#######################################################################################

def givequestresignfunction(id):
	quest = ''
	node = wolfpack.getdefinition(WPDT_QUEST, str(id))

	count = node.childcount
	for i in range(0, count):
		subnode = node.getchild(i)
		if subnode.name == 'functionresign':
			quest = subnode.text

	return quest

#######################################################################################
##############   Return the Complete Function for this Quest   ########################
#######################################################################################

def givequestcompletefunction(id):
	quest = ''
	node = wolfpack.getdefinition(WPDT_QUEST, str(id))

	count = node.childcount
	for i in range(0, count):
		subnode = node.getchild(i)
		if subnode.name == 'functioncomplete':
			quest = subnode.text

	return quest

#######################################################################################
##############   Return the Fail Function for this Quest   ############################
#######################################################################################

def givequestfailfunction(id):
	quest = ''
	node = wolfpack.getdefinition(WPDT_QUEST, str(id))

	count = node.childcount
	for i in range(0, count):
		subnode = node.getchild(i)
		if subnode.name == 'functionfail':
			quest = subnode.text

	return quest

#######################################################################################################
#######################################################################################################
#######################     MODULE: KILL NPCs     #####################################################
#######################################################################################################
#######################################################################################################

#######################################################################################
##############   Give Required NPCs in a list   #######################################
#######################################################################################

def givequestnpctargets(id):
	quest = ''
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
	quest = ''
	node = wolfpack.getdefinition(WPDT_QUEST, str(id))

	count = node.childcount
	for i in range(0, count):
		subnode = node.getchild(i)
		if subnode.name == 'npcamounts':
			quest = subnode.text.split(',')

	# Returning the list
	return quest

#######################################################################################
##############   Give Regions for kill each NPCs in a list   ##########################
#######################################################################################

def givequestnpcregions(id):
	quest = ''
	node = wolfpack.getdefinition(WPDT_QUEST, str(id))

	count = node.childcount
	for i in range(0, count):
		subnode = node.getchild(i)
		if subnode.name == 'npcregions':
			quest = subnode.text.split(',')

	return quest

#######################################################################################################
#######################################################################################################
#######################     MODULE: REQUIRED ITEMS     ################################################
#######################################################################################################
#######################################################################################################

#######################################################################################
##############   Give Required Items in a list   ######################################
#######################################################################################

def givequestitemtargets(id):
	quest = ''
	node = wolfpack.getdefinition(WPDT_QUEST, str(id))

	count = node.childcount
	for i in range(0, count):
		subnode = node.getchild(i)
		if subnode.name == 'itemtargets':
			quest = subnode.text.split(',')

	return quest

#######################################################################################
##############   Give amount of each Required Item in a list   ########################
#######################################################################################

def givequestitemeachamount(id):
	quest = ''
	node = wolfpack.getdefinition(WPDT_QUEST, str(id))

	count = node.childcount
	for i in range(0, count):
		subnode = node.getchild(i)
		if subnode.name == 'itemamounts':
			quest = subnode.text.split(',')

	# Returning the list
	return quest

#######################################################################################
#########   Give ID of NPCs that drops each required item in a list   #################
#######################################################################################

def givequestitemloots(id):
	quest = ''
	node = wolfpack.getdefinition(WPDT_QUEST, str(id))

	count = node.childcount
	for i in range(0, count):
		subnode = node.getchild(i)
		if subnode.name == 'itemloots':
			quest = subnode.text.split(',')

	return quest

#######################################################################################
#########   Give Region of NPCs that drops each required item in a list   #############
#######################################################################################

def givequestitemlootsregions(id):
	quest = ''
	node = wolfpack.getdefinition(WPDT_QUEST, str(id))

	count = node.childcount
	for i in range(0, count):
		subnode = node.getchild(i)
		if subnode.name == 'itemlootregions':
			quest = subnode.text.split(',')

	return quest

#######################################################################################################
#######################################################################################################
#######################     MODULE: TIMED QUESTS     ##################################################
#######################################################################################################
#######################################################################################################

#######################################################################################
##############   Give Time for this Quest   ###########################################
#######################################################################################

def givequesttimer(id):
	quest = ''
	node = wolfpack.getdefinition(WPDT_QUEST, str(id))

	count = node.childcount
	for i in range(0, count):
		subnode = node.getchild(i)
		if subnode.name == 'questtimer':
			quest = subnode.text

	return int(quest)

#######################################################################################################
#######################################################################################################
#######################     MODULE: REPORT QUEST     ##################################################
#######################################################################################################
#######################################################################################################

#######################################################################################
#########   Give the UID(s) of NPC to report Quest back   #############################
#######################################################################################

def givequestreportuids(id):
	quest = ''
	node = wolfpack.getdefinition(WPDT_QUEST, str(id))

	count = node.childcount
	for i in range(0, count):
		subnode = node.getchild(i)
		if subnode.name == 'reportuid':
			quest = subnode.text.split(',')

	return quest

#######################################################################################
#########   Give the ID(s) of NPC to report Quest back   ##############################
#######################################################################################

def givequestreportids(id):
	quest = ''
	node = wolfpack.getdefinition(WPDT_QUEST, str(id))

	count = node.childcount
	for i in range(0, count):
		subnode = node.getchild(i)
		if subnode.name == 'reportid':
			quest = subnode.text.split(',')

	return quest

#######################################################################################################
#######################################################################################################
#######################     CHECK REQUIREDS FOR PLAYER (GENERIC UTILS)     ############################
#######################################################################################################
#######################################################################################################

#######################################################################################
##############   Check if a Player have all Requirements   ############################
#######################################################################################

def checkquestrequirements(player, id):
	
	if not checkhaverequiredquests(player, id):
		return False

	if not checkhaverequiredskills(player, id):
		return False

	# Here, check Class, Race and Level too if your Shard supports it.
	# This sample checks for tags 'classe' for player class and 'raca' for player Race. Level is checked by 'level' tag.
	# Adjust it for your own Shard
	
	if not checkhaverequiredclass(player, id):
		return False

	if not checkhaverequiredrace(player, id):
		return False

	if not checkhaverequiredlevel(player, id):
		return False

	return True

#######################################################################################
##############   Check if a Player have the Required Quests   #########################
#######################################################################################

def checkhaverequiredquests(player, id):
	
	# Get Required Quests for this quest and Amount of requireds
	requiredquests = givequestrequiredquests(id)
	amountofrequired = len(requiredquests)
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
##############   Check if a Player have the Required Skills   #########################
#######################################################################################

def checkhaverequiredskills(player, id):
	
	# Get Required Skills for this quest and Amount of requireds
	requiredskills = givequestrequiredskills(id)
	num_skills = len(requiredskills)

	# Get Required Skill amounts for this quest and Amount of requireds
	requiredskillamounts = givequestrequiredskillamounts(id)
	num_skillamounts = len(requiredskillamounts)

	# how many skills do we really have to check?
	num_check = min( num_skills, num_skillamounts )

	try:
		for i in range(0, num_check):
			# check for each skill its requirement
			if player.skill[ skillnamesids[ requiredskills[i] ] ] < int(requiredskillamounts[i]):
				return False		
	except:
		# catch misspelled skill names etc.
		return False

	return True

#######################################################################################
##############   Check if a Player have the Required Class   ##########################
#######################################################################################

def checkhaverequiredclass(player, id):
	
	# Get Required Quests for this quest and Amount of requireds
	requiredclasses = givequestrequiredclasses(id)
	amountofrequired = len(requiredclasses)
	# Just a Flag
	requiredok = 0
	# The Class Tag. Edit it for your shard
	classe = player.gettag('classe')

	if amountofrequired:
		# Player things
		for i in range(0, amountofrequired):
			if requiredclasses[i] == classe:
				requiredok = 1			

		if not requiredok:
			return False
		else:
			return True
	else:
		return True

#######################################################################################
##############   Check if a Player have the Required Race   ###########################
#######################################################################################

def checkhaverequiredrace(player, id):
	
	# Get Required Quests for this quest and Amount of requireds
	requiredraces = givequestrequiredraces(id)
	amountofrequired = len(requiredraces)
	# Just a Flag
	requiredok = 0
	# The Class Tag. Edit it for your shard
	race = player.gettag('raca')

	if amountofrequired:
		# Player things
		for i in range(0, amountofrequired):
			if requiredraces[i] == race:
				requiredok = 1			

		if not requiredok:
			return False
		else:
			return True
	else:
		return True

#######################################################################################
##############   Check if a Player have the Required Level   ##########################
#######################################################################################

def checkhaverequiredlevel(player, id):
	
	# Get Required Quests for this quest and Amount of requireds
	requiredlevel = givequestrequiredlevel(id)

	# The Level Tag. Edit it for your shard
	if player.hastag('level'):
		level = int( player.gettag('level') )
	else:
		return True

	if requiredlevel:
		if requiredlevel > level:
			return False
		else:
			return True
	else:
		return True

#######################################################################################################
#######################################################################################################
#######################     PLAYER UTILS     ##########################################################
#######################################################################################################
#######################################################################################################

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
##############   Check if a Player already completed a quest   ########################
#######################################################################################

def checkifquestcompleted(player, id):

	tagsection = int( int(id) / int(MAXQUESTSPERTAG) )
	
	if player.hastag('quest.'+ str(tagsection) +'.complete'):
		quests = str(player.gettag('quest.'+ str(tagsection) +'.complete')).split(',')

		flag = 0

		for i in range(0, len(quests)):
			if int(quests[i]) == int(id):
				flag = 1
				break

		if flag == 0:
			return False
		else:
			return True
	else:
		return False

#######################################################################################################
#######################################################################################################
#######################     MISC UTILS     ############################################################
#######################################################################################################
#######################################################################################################

#######################################################################################
##############   Return a NPC Name   ##################################################
#######################################################################################

def givenpcname(id):
	npc = ''

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

#######################################################################################
##############   Return a Item Name   #################################################
#######################################################################################

def giveitemname(id):
	item = ''

	node = wolfpack.getdefinition(WPDT_ITEM, str(id))

	if node:
		count = node.childcount
		for i in range(0, count):
			subnode = node.getchild(i)
			if subnode.name == 'name':
				item = subnode.text

		return item

	else:

		return "Error"
