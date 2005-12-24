#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts
#  ((    _/{  "-;  | Created by: MagnusBr
#   )).-' {{ ;'`   | Revised by:
#  ( (  ;._ \\ ctr | Last Modification: Created
#===============================================================#
# Quest System - Default Functions
#===============================================================#

#######################################################################################
##############   Imports   ############################################################
#######################################################################################

from wolfpack import tr
import wolfpack
from wolfpack.gumps import cGump
from wolfpack.consts import *
from quests.utils import *

#######################################################################################
##############   Give a Quest to the Player   #########################################
#######################################################################################

def givequesttoplayer(player, id, npc):
	freeslot = 0		# Flag
	alreadyhave = 0		# Another Flag

	# Checking if Player dont have this Quest
	for i in range(1, 11):
		if player.hastag('Quest.'+ str(i) +'.ID'):
			playerid = player.gettag('Quest.'+ str(i) +'.ID')
			if playerid == id:
				alreadyhave = 1
				break

	# Checking if Player never completed this Quest
	if checkifquestcompleted(player, id):
		alreadyhave = 1
	
	# So... lets assign Quest
	if not alreadyhave:
	
		# Now checking for free slots
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

				player.socket.sysmessage("You assigned this Quest!")

				break
		
	
		if freeslot == 0:
			player.socket.sysmessage("You have no Free Slots for this Quest")

	else:
		player.socket.sysmessage("You already have or already completed this Quest")

#######################################################################################
##############   Resign a Quest   #####################################################
#######################################################################################

def resignquest(player, id, slot):
	if not player.hastag('Quest.'+ str(slot) +'.ID'):
		player.socket.sysmessage("We have an error in quest system. You resign a Quest that you dont own")
	else:
		player.deltag('Quest.'+ str(slot) +'.ID')
		player.deltag('Quest.'+ str(slot) +'.NPCDest')

		if player.hastag('Quest.'+ str(slot) +'.ReqNPC'):
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

			npclist = givequestnpctargets(id)					# Get the List of NPCs
			npcamount = player.gettag('Quest.'+ str(i) +'.ReqNPC').split(',')	# Get the Amount List for NPCs

			for j in range( 0, len(npclist) ):
				
				# Oh! Is this the NPC?
				if dead.baseid == npclist[j]:

					eachamount = givequestnpceachamount(id)					# Amount for each NPC for this Quest

					if int(npcamount[j]) < int(eachamount[j]):
					
						# Increase 1
						tempamount = int(npcamount[j])
						tempamount += 1
						npcamount[j] = str(tempamount)

						# Getting Tag back
						tempstorage = str(npcamount[0])

						if len(npcamount) > 1:
							for k in range( 1, len(npcamount) ):
								tempstorage += ","
								tempstorage += str(npcamount[k])
					
						player.settag('Quest.'+ str(i) +'.ReqNPC', tempstorage)	# Save Tag again

						player.socket.sysmessage("You killed " + str(tempamount) + " of " + str(eachamount[j]) + " npcs for a Quest")

#######################################################################################
##############   Report a Quest to a NPC   ############################################
#######################################################################################

def reportquestnpc(player, npc, slot):
	# Lets find ID of this quest
	id = player.gettag('Quest.'+ str(slot) +'.ID')

	# Lets try the lists huh?
	eachnpcamount = givequestnpceachamount(id)
	if player.hastag('Quest.'+ str(slot) +'.ReqNPC'):
		killedlist = player.gettag('Quest.'+ str(slot) +'.ReqNPC').split(',')

	eachitemamount = givequestitemeachamount(id)
	itemtargets = givequestitemtargets(id)

	# Now the amount of NPCs
	npcamount = givequestnpcamounts(id)
	itemamount = givequestitemamounts(id)

	# The Flag to check things
	flag = 0

	# The Looping to check NPCs
	if npcamount:
		for i in range( 0, npcamount ):

			# Checking if we killed less NPCs than necessay
			if int(killedlist[i]) < int(eachnpcamount[i]):
				flag = 1

	# Now, the Loop to check items
	if itemamount:
		for i in range( 0, itemamount ):

			# Checking if we have less Items than necessay
			backpack = player.getbackpack()
			itemcount = backpack.countitems( [itemtargets[i]] )

			if itemcount < int(eachitemamount[i]):
				flag = 1


	# Check if we completed the quest or not
	if flag:
		npc.say("You not completed all required things for this quest. Come back when you complete all tasks.")
	else:
		npc.say("You completed all tasks for this quest! Now let me give to you your reward.")
		completequest(player, slot, id)

#######################################################################################
##############   Complete the Quest and give rewards   ################################
#######################################################################################

def completequest(player, slot, id):
	# Get Backpack
	backpack = player.getbackpack()
	
	# Let's delete the tags
	player.deltag('Quest.'+ str(slot) +'.ID')
	player.deltag('Quest.'+ str(slot) +'.NPCDest')
	player.deltag('Quest.'+ str(slot) +'.ReqNPC')

	# Let's Consume required items
	itemamount = givequestitemamounts(id)
	itemtargets = givequestitemtargets(id)
	eachitemamount = givequestitemeachamount(id)

	if itemamount:
		for i in range( 0, itemamount ):

			# Checking if we have less Items than necessay
			backpack = player.getbackpack()
			itemcount = backpack.countitems( [itemtargets[i]] )
			backpack.removeitems( [itemtargets[i]], int(eachitemamount[i]) )

	# Now, rewards time. First, lists. After, amount of rewards
	rewards = givequestrewards(id)
	rewardamounts = givequesteachrewardsamount(id)
	amountofrewards = givequestrewardsamount(id)

	# Lets see if reward is not an empty list
	if amountofrewards:

		# Loop giving the amounts
		for i in range( 0, amountofrewards ):

			# Add item, set amount, send to backpack
			item = wolfpack.additem( rewards[i] )
			item.amount = int(rewardamounts[i])
			backpack.additem( item )
			item.update()

	# Lets save this Quest in a Tag to player never try this again with same character
	tagsection = int ( id / MAXQUESTSPERTAG )

	if player.hastag('quest.'+ str(tagsection) +'.complete'):
		
		quests = str(player.gettag('quest.'+ str(tagsection) +'.complete'))
		quests += ',' + str(id)

		player.settag('quest.'+ str(tagsection) +'.complete', quests)

	else:
		player.settag('quest.'+ str(tagsection) +'.complete', id)
