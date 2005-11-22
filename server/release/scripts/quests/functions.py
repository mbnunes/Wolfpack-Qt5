#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts
#  ((    _/{  "-;  | Created by: MagnusBr
#   )).-' {{ ;'`   | Revised by:
#  ( (  ;._ \\ ctr | Last Modification: Created
#===============================================================#
# Quest System - Default Functions
#===============================================================#

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
		player.socket.sysmessage("You already have this Quest")

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
