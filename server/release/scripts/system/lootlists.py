
# Definition Lists
# Stuff like weapons, Armor, Instruments, Etc
# See system.loot for examples of it's use.
# Please use DEF_ as a prefix

# Weapon Def Lists
DEF_WEAPONS_SWORDS = ['f4d', 'f4e', '26c7', '26bd', '26c5', '26bb', 'f5e', 'f5f', '13f6', '13f7', 'ec3', 'ec2',
	'26cb', '26c1', 'f60', 'f61', '13b7', '13b8', '13fe', '13ff', '143e', '143f', '1440', '1441', '26ce', '26cf', 'e85',
	'e86', '13b9', '13ba', 'ec4', 'ec5', '26c4', '26ba', '13b5', '13b6']
DEF_WEAPONS_AXES = ['f49', 'f4a', 'f47', 'f48', 'f4b', 'f4c', '1142', '1143', 'f45', 'f46', '13fa', '13fb', 'f43', 'f44']
DEF_WEAPONS_MACES = ['df0', 'df1', '13b3', '13b4', '13f8', '13f9', '143d', '143c', 'f5c', 'f5d', '143a', '143b',
	'e89', 'e8a', '26c6', '26bc', '13f4', '13f5', 'fb4', 'fb5', '13e3', '13e4', '13af', '13b0', '1438', '1439', '1406', '1407']
DEF_WEAPONS_FENCING = ['f51', 'f52', '26c9', '26bf', '1400', '1401', '26c0', '26ca', '26c8', '26be', 'e87',
	'e88', '1402', '1403', 'f62', 'f63', '1404', '1405']
DEF_WEAPONS_BOWS = ['13b1', '13b2', '26c2', '26cc', 'f4f', 'f50', '13fd', '13fc', '26cd', '26c3']
DEF_ALLWEAPONS = WEAPONS_SWORDS + WEAPONS_AXES + WEAPONS_MACES + WEAPONS_FENCING + WEAPONS_BOWS

# Armor Definition Lists
DEF_ORCMASKS = ['141b', '141c']
DEF_ORCHELMS = ['1f0b', '1f0c']
DEF_TRIBALMASKS = ['1549', '154a', '154b', '154c']
DEF_MAKS = ['1545', '1546', '1547', '1548', '141b', '141c']
DEF_HATS = ['153f', '1540', '1545', '1546', '1715', '1719', '171a', '171c', '172e', '1713', '141b', '141c',
	'1543', '1544', '1717', '1716', '1549', '154a', '154b', '154c', '171b', '1714', '1718']
DEF_LEATHER = ['1c06', '1c07', '1c0a', '1c0b', '1db9', '1dba', '13c6', '13ce', '13c7', '13cb', '13d2', '1c00', '1c01',
	'1c08', '1c09', '13c5', '13cd', '13cc', '13d3']
DEF_STUDDED = ['1c02', '1c03', '1c0c', '1c0d', '13d5', '13dd', '13d6', '13da', '13e1', '13d4', '13dc', '13db', '1322']
DEF_CHAINMAIL = ['13bb', '13c0', '13be', '13cc', '13bf', '13c4']
DEF_RINGMAIL = ['13eb', '13f2', '13f0', '13f1', '13ee', '13ef', '13ec', '13ed']
DEF_BONEMAIL = ['1454', '144f', '1453', '144e', '1455', '1450', '1457', '1452', '1456', '1452']
DEF_PLATEMAIL = ['1c04', '1c05', '1412', '1419', '1415', '1416', '1410', '1417', '1414', '1418', '1413', '1411', '141a']
DEF_DRAGONMAIL = ['2641', '2642', '2643', '2644', '2645', '2646', '2647', '2648', '2657', '2658']
DEF_HELMS = ['140c', '140d', '1408', '1409', '140a', '140b', '140e', '140f', '1f0b', '1f0c']
DEF_SHIELDS = ['1b72', '1b73', '1bc3', '1b76', '1b77', '1b74', '1b75', '1b7b', '1bc4', '1bc5', '1b78', '1b79', '1b79']
DEF_ALLARMOR = DEF_HATS + DEF_LEATHER + DEF_STUDDED + DEF_CHAINMAIL + DEF_RINGMAIL + DEF_BONEMAIL + DEF_PLATEMAIL + DEF_DRAGONMAIL + DEF_HELMS + DEF_SHIELDS + DEF_SHIELDS

# Misc Equipment Lists
DEF_CANDLETORCH = []
DEF_INSTRUMENTS = ['e9c', 'eb2', 'eb1', 'eb3', 'e9d', 'e9e']

# Foods & Drinks
DEF_FOOD_MEATS = ['976', '977', '978', '979', '97b', '9b7', '9b8', '9bb', '9bc', '9c0', '9c1', '9c9', '9d3', '1608',
	'160a', '1e1d', '1e1e', '1e1f', '1e1c']
DEF_FOOD_MISC = ['97c', '97d', '97e', '98c', '9b6', '9c4', '9c5', '9c6', '9c7', '9c8', '9e9', '9ea', '9eb', '9ec', '9fa',
	 '103b', '103c', '1040', '1041', '160c', '160b', '1ad3']
DEF_FOOD_GREENS = ['994', '9d0', '9d1', '9d2', 'c5c', 'c5d', 'c61', 'c62', 'c63', 'd39', 'd3a', 'c64', 'c65', 'c66',
	'c67', 'c6a', 'c6b', 'c6c', 'c6d', 'c6e', 'c70', 'c71', 'c72', 'c73', 'c74', 'c75', 'c77', 'c78', 'c79', 'c7a', 'c7b', 'c7c',
	'c7f', 'c80', 'c81', 'c82', 'd1a', '171d', '171e', '171f', '1720', '1721', '1722', '1723', '1724', '1725', '1726', '1727',
	'1728', '1729', '172a', '172b', '172c', '172d']
DEF_DRINKS_NORMAL = ['9ad', '9f0', '9f2', 'ff8', 'ff9', '1f89', '1f8a', '1f8b', '1f8c', '1f91', '1f92', '1f93', '1f94']
DEF_DRINKS_ALCOHOL = ['98d', '98e', '99b', '99c', '99d', '99e', '99f', '9a0', '9a1', '9a2', '9ee', '9ef', '1f7d', '1f7d',
	'1f7e', '1f7f', '1f80', '1f85', '1f86', '1f87', '1f88', '1f8d', '1f8e', '1f8f', '1f90', '1f95', '1f96', '1f97', '1f98', '1f99',
	'1f9a', '1f9b', '1f9c', '1f9d', '1f9e']
DEF_ALLDRINKS = DEF_DRINKS_NORMAL + DEF_DRINKS_ALCOHOL
DEF_ALLFOOD = DEF_FOOD_MEATS + DEF_FOOD_VEGGIES + DEF_FOODS_MISC

# Clothes
DEF_SHIRTS = []
DEF_SHOES = []
DEF_PANTS = []
DEF_FEMALE_PANTS = []
DEF_ROBES = []
DEF_DRESSES = []
DEF_CLOTHES_ALL = DEF_SHIRTS + DEF_SHOES + DEF_PANTS + DEF_FEMALE_PANTS + DEF_ROBES + DEF_DRESSES + DEF_HATS

# Potions
DEF_ALLPOTIONS = ['f06', 'f07', 'f08', 'f09', 'f0a', 'f0b', 'f0c', 'f0d']

# Reagents
DEF_PLAINREGS = ['f7a', 'f7b', 'f84', 'f85', 'f86', 'f88', 'f8c', 'f8d']
DEF_NECROREGS = ['f78', 'f79', 'f7c', 'f7d', 'f7e', 'f7f', 'f80', 'f81', 'f82', 'f83', 'f87', 'f89', 'f8a', 'f8b', 'f8e', 'f8f', 'f90', 'f91']
DEF_ALLREGS = PLAINREGS + NECROREGS



