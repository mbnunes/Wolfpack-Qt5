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
from wolfpack import tr, properties

####################################################################################
###################     Level Templates      #######################################
####################################################################################

# Now, its the following order to template: Gold Amount, Magic Items, Scrolls, Gems, Stack of Reagents
TEMPLATELEVEL = {
	'0': [500, 1, 2, 2, 2],		# Since I dont Have information, I maded by myself level 0 template
	'1': [1000, 5, 4, 3, 3],
	'2': [2000, 10, 8, 6, 6],
	'3': [3000, 15, 12, 9, 9],
	'4': [4000, 38, 16, 12, 12],
	'5': [5000, 50, 20, 15, 15],
	'6': [6000, 60, 23, 18, 18]
}

####################################################################################
###################     Item Templates      ########################################
####################################################################################

TREAS_JEWELERY = ['1085', '1086', '1087', '1088', '1089', '108a', '1f05', '1f06', '1f07', '1f08', '1f09', '1f0a']
TREAS_SHIELDS = ['1b72', '1b73', '1bc3', '1b76', '1b77', '1b74', '1b75', '1b7b', '1bc4', '1bc5', '1b78', '1b79', '1b79']

TREAS_MASKS = ['141b', '141c', '1549', '154a', '154b', '154c', '1545', '1546', '1547', '1548']
TREAS_HATS = ['153f', '1540', '1715', '1719', '171a', '171c', '172e', '1713', '1543', '1544', '1717', '1716',
	'171b', '1714', '1718']
TREAS_LEATHER = ['1c06', '1c07', '1c0a', '1c0b', '1db9', '1dba', '13c6', '13ce', '13c7', '13cb', '13d2', '1c00', '1c01',
	'1c08', '1c09', '13c5', '13cd', '13cc', '13d3']
TREAS_STUDDED = ['1c02', '1c03', '1c0c', '1c0d', '13d5', '13dd', '13d6', '13da', '13e1', '13d4', '13dc', '13db', '13e2']
TREAS_CHAINMAIL = ['13bb', '13c0', '13be', '13cc', '13bf', '13c4']
TREAS_RINGMAIL = ['13eb', '13f2', '13f0', '13f1', '13ee', '13ef', '13ec', '13ed']
TREAS_BONEMAIL = ['1454', '144f', '1453', '144e', '1455', '1450', '1457', '1452', '1456', '1452']
TREAS_PLATEMAIL = ['1c04', '1c05', '1412', '1419', '1415', '1416', '1410', '1417', '1414', '1418', '1413', '1411', '141a']
TREAS_DRAGONMAIL = ['2641', '2642', '2643', '2644', '2645', '2646', '2647', '2648', '2657', '2658']
TREAS_HELMS = ['140c', '140d', '1408', '1409', '140a', '140b', '140e', '140f', '1f0b', '1f0c']

TREAS_ARMOR = TREAS_LEATHER + TREAS_STUDDED + TREAS_CHAINMAIL + TREAS_RINGMAIL + TREAS_BONEMAIL + TREAS_PLATEMAIL + TREAS_DRAGONMAIL + TREAS_HELMS + TREAS_HATS + TREAS_MASKS

TREAS_WEAPONS_SWORDS = ['f4d', 'f4e', '26c7', '26bd', '26c5', '26bb', 'f5e', 'f5f', '13f6', '13f7', 'ec3', 'ec2',
	'26cb', '26c1', 'f60', 'f61', '13b7', '13b8', '13fe', '13ff', '143e', '143f', '1440', '1441', '26ce', '26cf', 'e85',
	'e86', '13b9', '13ba', 'ec4', 'ec5', '26c4', '26ba', '13b5', '13b6']
TREAS_WEAPONS_AXES = ['f49', 'f4a', 'f47', 'f48', 'f4b', 'f4c', 'f45', 'f46', '13fa', '13fb', 'f43', 'f44']
TREAS_WEAPONS_MACES = ['df0', 'df1', '13b3', '13b4', '13f8', '13f9', '143d', '143c', 'f5c', 'f5d', '143a', '143b',
	'e89', 'e8a', '26c6', '26bc', '13f4', '13f5', 'fb4', 'fb5', '13e3', '13e4', '13af', '13b0', '1438', '1439', '1406', '1407']
TREAS_WEAPONS_FENCING = ['f51', 'f52', '26c9', '26bf', '1400', '1401', '26c0', '26ca', '26c8', '26be', 'e87',
	'e88', '1402', '1403', 'f62', 'f63', '1404', '1405']
TREAS_WEAPONS_BOWS = ['13b1', '13b2', '26c2', '26cc', 'f4f', 'f50', '13fd', '13fc', '26cd', '26c3']
TREAS_ALLWEAPONS = TREAS_WEAPONS_SWORDS + TREAS_WEAPONS_AXES + TREAS_WEAPONS_MACES + TREAS_WEAPONS_FENCING + TREAS_WEAPONS_BOWS

TREAS_SCROLLS = ['1f2d', '1f2e', '1f2f', '1f30', '1f31', '1f32', '1f33', '1f34', '1f35', '1f36', '1f37', '1f38', '1f39', '1f3a', '1f3b', '1f3c', '1f3d', '1f3e', '1f3f', '1f40', '1f41', '1f42', '1f43', '1f44', '1f45', '1f46', '1f47', '1f48', '1f49', '1f4a', '1f4b', '1f4c', '1f4d', '1f4e', '1f4f', '1f50', '1f51', '1f52', '1f53', '1f54', '1f55', '1f56', '1f57', '1f58', '1f59', '1f5a', '1f5b', '1f5c', '1f5d', '1f5e', '1f5f', '1f60', '1f61', '1f62', '1f63', '1f64', '1f65', '1f66', '1f67', '1f68', '1f69', '1f6a', '1f6b', '1f6c']

TREAS_GEMS = ['f1b', 'f21', 'f12', 'f19', 'f1f', 'f23', 'f24', 'f2c', 'f17', 'f22', 'f2e', 'f1e', 'f20', 'f2d', 'f14', 'f1a', 'f1c', 'f1d',
	'f2a', 'f2b', 'f2f', 'f27', 'f28', 'f29', 'f30', 'f0f', 'f11', 'f15', 'f16', 'f18', 'f13', 'f10', 'f25', 'f26']

TREAS_PLAINREGS = ['f7a', 'f7b', 'f84', 'f85', 'f86', 'f88', 'f8c', 'f8d']
TREAS_NECROREGS = ['f78', 'f79', 'f7c', 'f7d', 'f7e', 'f7f', 'f80', 'f81', 'f82', 'f83', 'f87', 'f89', 'f8a', 'f8b', 'f8e', 'f8f', 'f90', 'f91']
TREAS_ALLREGS = TREAS_PLAINREGS + TREAS_NECROREGS

####################################################################################
###################     Constants      #############################################
####################################################################################

MINAMOUNTSTACK = 40		# Min Amount of Reagent Stack
MAXAMOUNTSTACK = 60		# Max Amount of Reagent Stack

####################################################################################
###################     Fill the Chest      ########################################
####################################################################################

def filltreasure( chest, level ):

	# Lets assign the Correct List for this Level
	templatelist = TEMPLATELEVEL[level]

	########################################
	# Now, lets begin to fill container
	########################################

	##########
	# Gold
	##########
	item = wolfpack.additem('eed')
	item.amount = templatelist[0]
	chest.additem(item)

	##########
	# Magic Items (Follow same logical from loot.py)
	##########

	for i in range(0, templatelist[1]):
	
		# Select the item-id
		value = random.random()

		# 10% Jewelry
		if value > 0.90:
			item = wolfpack.additem(random.choice(TREAS_JEWELERY))
		# 10% Shield
		elif value > 0.80:
			item = wolfpack.additem(random.choice(TREAS_SHIELDS))
		# 40% Armor
		elif value > 0.40:
			item = wolfpack.additem(random.choice(TREAS_ARMOR))
		# 40% Weapon
		else:
			item = wolfpack.additem(random.choice(TREAS_ALLWEAPONS))
						
		properties.applyRandom(item, 5, 0, 100, 0)

		chest.additem(item)

	##########
	# Scrolls
	##########
	for i in range(0, templatelist[2]):
		
		item = wolfpack.additem(random.choice( TREAS_SCROLLS ))
		chest.additem(item)

	##########
	# Gems
	##########
	for i in range(0, templatelist[3]):
		
		item = wolfpack.additem(random.choice( TREAS_GEMS ))
		chest.additem(item)

	##########
	# Reagents
	##########
	for i in range(0, templatelist[4]):
		
		item = wolfpack.additem(random.choice( TREAS_ALLREGS ))
		item.amount = random.randint(MINAMOUNTSTACK, MAXAMOUNTSTACK)
		chest.additem(item)