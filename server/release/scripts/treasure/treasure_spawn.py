#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: MagnusBr                       #
#   )).-' {{ ;'`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification: Created                 #
#################################################################

####################################################################################
###################     Imports      ###############################################
####################################################################################

import wolfpack
import random

####################################################################################
###################     Groups      ################################################
####################################################################################

TREASPAWNS = {
	'0': ['headless', 'mongbat', 'skeleton', 'zombie'],	# Later make the "Chest Guardians" (http://uo.stratics.com/database/view.php?db_content=hunters&id=138)
	'1': ['mongbat', 'ratman', 'headless', 'skeleton', 'zombie'],
	'2': ['orc_mage', 'gargoyle', 'gazer', 'hell_hound', 'earth_elemental'],
	'3': ['lich', 'ogre_lord', 'dread_spider', 'air_elemental', 'fire_elemental'],
	'4': ['dread_spider', 'lich_lord', 'daemon', 'elder_gazer', 'ogre_lord'],
	'5': ['lich_lord', 'daemon', 'elder_gazer', 'poison_elemental', 'blood_elemental'],
	'6': ['ancient_wyrm', 'balron', 'poison_elemental', 'blood_elemental', 'titan']
}

####################################################################################
###################     Constants      #############################################
####################################################################################

MINSPAWN = 4			# Min of Monsters on Initial Spawn
MAXSPAWN = 4			# Max of Monsters on Initial Spawn

MINLEVELFORINITIALSPAWN = 2	# Min Level to have Initial Spawn on Treasure

####################################################################################
###################     Spawning (Initial)      ####################################
####################################################################################

def treasinitialspawn( item ):

	# Spawn list for that Level
	level = item.gettag('level')
	spawnlist = TREASPAWNS[level]

	# Check Min Level for Initial Spawn
	if level < MINLEVELFORINITIALSPAWN:
		return

	# Lets check the amount of creatures. Always between 0 and 3:
	amountspawned = random.randint(MINSPAWN,MAXSPAWN)

	# Now... lets just Spawn creatures
	for i in range(0, amountspawned):
		
		npc = wolfpack.addnpc(random.choice( spawnlist ), item.pos)
		npc.update()

####################################################################################
###################     Spawning (Treasure Pick)      ##############################
####################################################################################

def treaspickspawn( item ):

	#############################################
	# Lets check the Level and assign SpawnList
	#############################################
	level = item.gettag('level')
	spawnlist = TREASPAWNS[level]

	#############################################
	# Lets check the Random Chances of Spawn
	#############################################
	value = random.random()

	# 2% of coming 5 Monsters
	if value >= 0.98:
		amount = 5
	# 3% of coming 4 Monsters
	elif value >= 0.95:
		amount = 4
	# 5% of coming 3 Monsters
	elif value >= 0.90:
		amount = 3
	# 15% of coming 2 Monsters
	elif value >= 0.75:
		amount = 2
	# 25% of coming 1 Monster
	elif value >= 0.50:
		amount = 1
	# 50% to came Nothing
	else:
		amount = 0

	#############################################
	# Now, Lets Adjust to level
	#############################################

	if amount > level:
		amount = level

	#############################################
	# Finally, Lets Spawn the Monsters
	#############################################

	if amount:

		for i in range(0, amount):
		
			npc = wolfpack.addnpc(random.choice( spawnlist ), item.pos)
			npc.update()