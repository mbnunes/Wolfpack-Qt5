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

MINSPAWN = 4		# Min of Monsters on Initial Spawn
MAXSPAWN = 4		# Max of Monsters on Initial Spawn

####################################################################################
###################     Spawning (Initial)      ####################################
####################################################################################

def treasinitialspawn( item ):

	# Spawn list for that Level
	level = item.gettag('level')
	spawnlist = TREASPAWNS[level]

	# Lets check the amount of creatures. Always between 0 and 3:
	amountspawned = random.randint(MINSPAWN,MAXSPAWN)

	# Now... lets just Spawn creatures
	for i in range(0, amountspawned):
		
		npc = wolfpack.addnpc(random.choice( spawnlist ), item.pos)
		npc.update()