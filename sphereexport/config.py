#
# CONFIGURATION
#

SAVEDIRECTORY = 'D:\\sphere\\save\\'
SPHERETABLES = 'D:\\sphere\\scripts\\spheretables.scp'
SPHEREPATH = 'D:\\sphere\\'
TILEDATA = 'C:\\Games\\Ultima Online\\Ancient Realms\\tiledata.mul'

#
# ORE TRANSLATION TABLE
# LEFT: Itemid in sphere
# RIGHT: ['name', 'baseid', 'resname', color]
#
ORES = {
	# These are converted to iron
	'I_ORE_OLD_COPPER': ['#1042853', 'iron_ore_1', 'iron', 0],
	'I_ORE_COPPER': ['#1042853', 'iron_ore_1', 'iron', 0],
	'I_ORE_DULL_COPPER': ['#1042853', 'iron_ore_1', 'iron', 0],
	'I_ORE_BRONZE': ['#1042853', 'iron_ore_1', 'iron', 0],
	'I_ORE_RUSTY': ['#1042853', 'iron_ore_1', 'iron', 0],
	'I_ORE_IRON': ['#1042853', 'iron_ore_1', 'iron', 0],
	
	# These are converted to dull copper
	'I_ORE_NEBEL': ['#1042845', 'dullcopper_ore_1', 'dullcopper', 0x973],
	'I_ORE_SILVER': ['#1042845', 'dullcopper_ore_1', 'dullcopper', 0x973],
	
	# These are converted to shadow iron
	'I_ORE_SHADOW' : ['#1042846', 'shadowiron_ore_1', 'shadowiron', 0x966],
	'I_ORE_ROSE' : ['#1042846', 'shadowiron_ore_1', 'shadowiron', 0x966],
	'I_ORE_AGAPITE' : ['#1042846', 'shadowiron_ore_1', 'shadowiron', 0x966],
	
	# These are converted to copper
	'I_ORE_VERITE': ['#1042847', 'copper_ore_1', 'copper', 0x96d],
	'I_ORE_GOLD': ['#1042847', 'copper_ore_1', 'copper', 0x96d],
	
	# These are converted to bronze
	'I_ORE_VULKAN': ['#1042848', 'bronze_ore_1', 'bronze', 0x972],
	'I_ORE_VALORITE': ['#1042848', 'bronze_ore_1', 'bronze', 0x972],
	
	# These are converted to gold
	'I_ORE_RUBIN': ['#1042849', 'gold_ore_1', 'gold', 0x8a5],
	'I_ORE_SMARAGD': ['#1042849', 'gold_ore_1', 'gold', 0x8a5],
	
	# These are converted to agapite
	'I_ORE_BLOODROCK': ['#1042850', 'agapite_ore_1', 'agapite', 0x979],
	'I_ORE_BLACKROCK': ['#1042850', 'agapite_ore_1', 'agapite', 0x979],
	'I_ORE_MYTHERIL': ['#1042850', 'agapite_ore_1', 'agapite', 0x979],
	'I_ORE_TALIUM': ['#1042850', 'agapite_ore_1', 'agapite', 0x979],
	'I_ORE_MINDURIUM': ['#1042850', 'agapite_ore_1', 'agapite', 0x979],
	'I_ORE_ETERIUM': ['#1042850', 'agapite_ore_1', 'agapite', 0x979],
	
	# These are converted to verite 
	'I_ORE_ENDURIUM': ['#1042851', 'verite_ore_1', 'verite', 0x89f],
	'I_ORE_ARKANIUM': ['#1042851', 'verite_ore_1', 'verite', 0x89f],
	
	# These are converted to valorite
	'I_ORE_TITANIUM': ['#1042852', 'valorite_ore_1', 'valorite', 0x8ab],
	'I_ORE_TENEBRACUM': ['#1042852', 'valorite_ore_1', 'valorite', 0x8ab],
}

#
# INGOT TRANSLATION TABLE
# LEFT: Itemid in sphere
# RIGHT: ['name', 'baseid', 'resname', color]
#
INGOTS = {
	# These are converted to iron
	'I_INGOT_OLD_COPPER': ['#1042692', 'iron_ingot', 'iron', 0],
	'I_INGOT_COPPER': ['#1042692', 'iron_ingot', 'iron', 0],
	'I_INGOT_DULL_COPPER': ['#1042692', 'iron_ingot', 'iron', 0],
	'I_INGOT_BRONZE': ['#1042692', 'iron_ingot', 'iron', 0],
	'I_INGOT_RUSTY': ['#1042692', 'iron_ingot', 'iron', 0],
	'I_INGOT_IRON': ['#1042692', 'iron_ingot', 'iron', 0],
	
	# These are converted to dull copper
	'I_INGOT_NEBEL': ['#1042684', 'dullcopper_ingot', 'dullcopper', 0x973],
	'I_INGOT_SILVER': ['#1042684', 'dullcopper_ingot', 'dullcopper', 0x973],
	
	# These are converted to shadow iron
	'I_INGOT_SHADOW' : ['#1042685', 'shadowiron_ingot', 'shadowiron', 0x966],
	'I_INGOT_ROSE' : ['#1042685', 'shadowiron_ingot', 'shadowiron', 0x966],
	'I_INGOT_AGAPITE' : ['#1042685', 'shadowiron_ingot', 'shadowiron', 0x966],
	
	# These are converted to copper
	'I_INGOT_VERITE': ['#1042686', 'copper_ingot', 'copper', 0x96d],
	'I_INGOT_GOLD': ['#1042686', 'copper_ingot', 'copper', 0x96d],
	
	# These are converted to bronze
	'I_INGOT_VULKAN': ['#1042687', 'bronze_ingot', 'bronze', 0x972],
	'I_INGOT_VALORITE': ['#1042687', 'bronze_ingot', 'bronze', 0x972],
	
	# These are converted to gold
	'I_INGOT_RUBIN': ['#1042688', 'gold_ingot', 'gold', 0x8a5],
	'I_INGOT_SMARAGD': ['#1042688', 'gold_ingot', 'gold', 0x8a5],
	
	# These are converted to agapite
	'I_INGOT_BLOODROCK': ['#1042689', 'agapite_ingot', 'agapite', 0x979],
	'I_INGOT_BLACKROCK': ['#1042689', 'agapite_ingot', 'agapite', 0x979],
	'I_INGOT_MYTHERIL': ['#1042689', 'agapite_ingot', 'agapite', 0x979],
	'I_INGOT_TALIUM': ['#1042689', 'agapite_ingot', 'agapite', 0x979],
	'I_INGOT_MINDURIUM': ['#1042689', 'agapite_ingot', 'agapite', 0x979],
	'I_INGOT_ETERIUM': ['#1042689', 'agapite_ingot', 'agapite', 0x979],
	
	# These are converted to verite 
	'I_INGOT_ENDURIUM': ['#1042690', 'verite_ingot', 'verite', 0x89f],
	'I_INGOT_ARKANIUM': ['#1042690', 'verite_ingot', 'verite', 0x89f],
	
	# These are converted to valorite
	'I_INGOT_TITANIUM': ['#1042691', 'valorite_ingot', 'valorite', 0x8ab],
	'I_INGOT_TENEBRACUM': ['#1042691', 'valorite_ingot', 'valorite', 0x8ab],
}

#
# Conversion table for armors and weapons
# The key is the color here. 
# Everything else would be impossible.
# The right list is: [resname, color]
#
COLORS = {
	0x590: ['iron', 0], # Old Copper
	0x641: ['iron', 0], # Copper
	0x60C: ['iron', 0], # Copper
	0x60A: ['iron', 0], # Dull Copper
	0x6D6: ['iron', 0], # Bronze
	0x750: ['iron', 0], # Rusty
	0x0: ['iron', 0], # Iron
	0x763: ['iron', 0], # Iron

	0x76C: ['dullcopper', 0x973], # Nebel
	0x482: ['dullcopper', 0x973], # Silber

	0x770: ['shadowiron', 0x966], # Shadow
	0x665: ['shadowiron', 0x966], # Rose
	0x400: ['shadowiron', 0x966], # Agapite

	0x7D1: ['copper', 0x96D], # Verite
	0x45E: ['copper', 0x96D], # Gold
	0x84D: ['copper', 0x96D], # Gold

	0x845: ['bronze', 0x972], # Vulkan
	0x515: ['bronze', 0x972], # Valorite

	0x846: ['gold', 0x8A5], # Rubin
	0x7D6: ['gold', 0x8A5], # Smaragd

	0x4C2: ['agapite', 0x979], # Bloodrock
	0x455: ['agapite', 0x979], # Blackrock
	0x52D: ['agapite', 0x979], # Mytheril
	0x528: ['agapite', 0x979], # Mytheril
	0x374: ['agapite', 0x979], # Talium
	0x709: ['agapite', 0x979], # Mindurium
	0x454: ['agapite', 0x979], # Eterium

	0x3E6: ['verite', 0x89F], # Endurium
	0x8AF: ['verite', 0x89F], # Arkanium

	0x385: ['valorite', 0x8AB], # Titanium
	0x3C2: ['valorite', 0x8AB], # Tenebracum
}

#
# NPC conversion table
# This is used to convert spawnpoints etc.
#
NPCS = {
	'C_ZOMBIE': 'zombie',
	'C_HEADLESS': 'headless',
	'C_SPECTRE': 'shade',
	'C_DOG': 'dog',
	'C_TROLL_W_AXE': 'troll',
	'C_TROLL': 'troll2',
	'C_TROLL_W_MACE': 'troll3',
	'C_SCORPION_GIANT': 'giant_scorpion',
	'C_SNAKE_GIANT': 'serpent_gaint',
	'C_CYCLOPS': 'cyclops',
	'C_M_LAVA_LIZARD': 'lava_lizard',
	'C_ELEM_EARTH': 'earth_elemental',
	'C_ELEM_FIRE': 'fire_elemental',
	'C_ORC': 'orc_1',
	'C_ORC_W_CLUB': 'orc_2',
	'C_ORC_LORD': 'orcish_lord',
	'C_SKELETON': 'skeleton',
	'C_M_ORC_MAGE': 'orc_mage',
	'C_M_ORC_CAPTAIN': 'orc_captain',
	'C_LIZARDMAN': 'lizardman_1',
	'C_M_ELEM_POISON': 'poison_elemental',
	'C_LIZARDMAN_SPEAR': 'lizardman_2',
	'C_LIZARDMAN_MACE': 'lizardman_3',
	'C_TOAD_GIANT': 'giant_toad',
	'C_M_SKELETON_MAGE': 'skeletal_mage',
	'C_GARGOYLE': 'gargoyle',
	'C_M_SKELETON_KNIGHT': 'skeleton_axe', # TODO
	'C_LICHE': 'lich',
	'C_M_WRAITH': 'shade', # TODO
	'C_M_SCORPION_THRALL': 'spider_giant', # TODO
	'C_M_OPHIDIAN_ARCHMAGE': 'ophidian_apprentice_mage',
	'C_M_OPHIDIAN_KNIGHT': 'ophidian_knight_errant',
	'C_GAZER': 'gazer',
	'C_OPHIDIAN_QUEEN': 'ophidian_matriarch',
	'C_M_HELLHOUND': 'hell_hound',
	'C_M_ELEM_BLOOD': 'blood_elemental',
	'C_M_GAZER_ELDER': 'elder_gazer',
	'C_M_MUMMY': 'mummy',
	'C_M_GHOUL': 'zombie', # TODO
	'C_M_HELLCAT': 'hellcat',
	'C_M_LICHE_LORD': 'lich_lord',
	'C_GRIM_REAPER': 'zombie', # TODO
	'C_M_EFREET': 'efreet',
	'C_RATMAN_W_MACE': 'ratman_3',
	'C_RATMAN_W_SWORD': 'ratman_2',
	'C_RATMAN': 'ratman_1',
	'C_SKELETON_W_AXE': 'skeleton_axe',
	'C_SKELETON_W_SWORD': 'skeleton_sword',
	'C_TERATHAN_WARRIOR': 'terathan_warrior',
	'C_TERATHAN_DRONE': 'terathan_drone',
	'C_M_TERATHAN_AVENGER': 'terathan_avenger',
	'C_TERATHAN_MATRIARCH': 'terathan_matriarch',
	'C_M_GARGOYLE_STONE': 'stone_gargoyle', 
	'C_ELEM_AIR': 'air_elemental',
	'C_M_SCORPION_QUEEN': 'giant_scorpion', # TODO
	'C_M_SCORPION_KING': 'giant_scorpion', # TODO
	'C_M_CYCLOPS_KING': 'cyclops', # TODO
	'C_HARPY': 'harpy',
	'C_M_STONEHARPY': 'stone_harpy',
	'C_CAT': 'cat',
	'C_RABBIT': 'rabbit',
	'C_SNAKE': 'snake',
	'C_M_SKELETONARCHER': 'skeleton', # TODO
	'C_RAT': 'rat',
	'C_BIRD': 'bird',
	'C_DOE': 'hind',
	'C_A_WOLF_GRAY': 'grey_wolf',
	'C_SHEEP_WOOLLY': 'sheep_unsheered',
	'C_EAGLE': 'eagle',
	'C_WISP': 'wisp',
	'C_A_JACKRABBIT': 'jack_rabbit',
	'C_PIG': 'pig',
	'C_PIG_BIG': 'boar',
	'C_A_DIREWOLF': 'dire_wolf',
	'C_SHEEP_SHORN': 'sheep_sheered',
	'C_BIRD_RAVEN': 'bird', # TODO
	'C_CHICKEN': 'chicken',
	'C_COW_BW': 'cow_black',
	'C_COW_BROWN': 'cow_brown',
	'C_BULL_BROWN_LT': 'brown_bull', # ???
	'C_BULL_BROWN_DK': 'black_bull', # ???
	'C_A_MUSTANG_SKY': 'horse_1', # TODO
	'C_STAG': 'great_hart',
	'C_A_BLACK_BEAR': 'black_bear',
	'C_BIRD_TROPICAL': 'bird', # TODO
	'C_BIRD_JUNGLE': 'bird', # TODO
	'C_WOLF': 'timber_wolf',
	'C_A_WOLF_WHITE': 'white_wolf',
	'C_HORSE_GRAY': 'horse_1',
	'C_BEAR_GRIZZLY': 'brown_bear', # TODO
	'C_BEAR_BROWN': 'brown_bear',	
	'C_REAPER': 'reaper',
	'C_LLAMA': 'llama',
	'C_HORSE_BROWN_LT': 'horse_2',
	'C_HORSE_BROWN_DK': 'horse_4',
	'C_HORSE_TAN': 'horse_3',
	'C_ETTIN': 'ettin',
	'C_SLIME': 'slime',
	'C_M_KRAKEN': 'kraken',
	'C_CORPSER': 'corpser', 
	'C_DOLPHIN': 'dolphin',
	'C_DAEMON': 'daemon',
	'C_OGRE': 'ogre',
	'C_TITAN': 'titan',
	'C_M_LAVA_SERPENT': 'lava_serpent',
	'C_MONGBAT': 'mongbat',
	'C_ELEM_WATER': 'water_elemental',
	'C_ETTIN_W_AXE': 'ettin2',
	'C_M_BALRON': 'balron',
	'C_M_OGRE_LORD': 'ogre_lord',
	'C_M_ETTIN_LORD': 'ettin', # TODO
	'C_M_FROSTOOZE': 'frost_ooze',
	'C_M_SWAMP_TENTACLES': 'swamp_tentacle',
	'C_M_SILVER_SERPENT': 'silver_serpent',
	'C_M_SCORPION_PRINCE': 'giant_scorpion', # TODO
	'C_M_DRAGON_ELF': 'dragon_gray', # TODO
	
	# Human NPCs
	'C_H_TAVERNKEEPER': 'tavernkeeper_male',
	'C_H_TAVERNKEEPER_F': 'tavernkeeper_female',
	'C_H_TANNER': 'tanner_male',	
	'C_H_TANNER_F': 'tanner_female',
	'C_H_INNKEEPER': 'innkeeper_male',
	'C_H_INNKEEPER_F': 'innkeeper_female',
	'C_H_BAKER': 'baker_male',
	'C_H_BAKER_F': 'baker_female',
	'C_H_TAILOR': 'tailor_male',
	'C_H_TAILOR_F': 'tailor_female',
	'C_H_TINKER': 'tinker_male',
	'C_H_TINKER_F': 'tinker_female',
	'C_H_HEALER': 'healer_male',
	'C_H_HEALER_F': 'healer_female',
	'C_H_BLACKSMITH': 'blacksmith_male',
	'C_H_BLACKSMITH_F': 'blacksmith_female',
	'C_H_ALCHEMIST': 'alchemist_male',
	'C_H_ALCHEMIST_F': 'alchemist_female',
	'C_H_PROVIS': 'provisioner_male',
	'C_H_PROVIS_F': 'provisioner_female',	
	'C_H_SCRIBE': 'base_male', # TODO
	'C_H_SCRIBE_F': 'base_female', # TODO
	'C_H_ARMORER': 'armorer_male',
	'C_H_ARMORER_F': 'armorer_female',	
	'C_H_JEWELER': 'jeweler_male',
	'C_H_JEWELER_F': 'jeweler_female',
	'C_H_BARBER': 'barber_male',
	'C_H_BARBER_F': 'barber_female',
	'C_H_COBBLER': 'cobbler_male',
	'C_H_COBBLER_F': 'cobbler_female',
	'C_H_STALL': 'stablemaster_male',
	'C_H_STALL_F': 'stablemaster_female',
	'C_H_ANIMALTRAINER': 'base_male', # TODO
	'C_H_ANIMALTRAINER_F': 'base_female', # TODO
	'C_H_VEGISELLER': 'base_male', # TODO
	'C_H_VEGISELLER_F': 'base_male', # TODO
	'C_H_BUTCHER': 'butcher_male',
	'C_H_BUTCHER_F': 'butcher_female',
	'C_H_BOWYER': 'bowyer_male',
	'C_H_BOWYER_F': 'bowyer_female',
	'C_H_FISHER': 'fisherman_male',
	'C_H_FISHER_F': 'fisherman_female',
	'C_H_BANKER': 'banker_male',
	'C_H_BANKER_F': 'banker_female',
	'C_H_SCULPTOR': 'base_male', # TODO
	'C_H_BANDIT_M': 'base_male', # TODO
	'C_H_BANDIT_F': 'base_female', # TODO
	'C_H_COOK': 'cook_male', # TODO
	'C_H_COOK_F': 'cook_female', # TODO
	'C_H_FARMER': 'base_female', # TODO
	'C_H_FARMER_F': 'base_female', # TODO
	'C_H_BARBARIAN_WARRIOR': 'base_male', # TODO
			
	# Custom
	'C_M_GOBLIN': 'goblin',
	'C_M_GOBLIN_CHIEF': 'goblin_chief',
	'C_M_GOBLIN_SHAMAN': 'goblin_shaman',
	'C_M_GOBLIN_MAGE': 'goblin_mage',
	'C_M_GOBLIN_LORD': 'goblin_lord',		
	'C_SPINNENMAENNCHEN': 'spider_giant',
	'C_SPINNENKOENIGIN': 'spider_giant',
	'C_RIESENSPINNE': 'spider_giant',
	'C_SPINNENWAECHTERIN': 'spider_giant',	
	'C_IRRER': 'base_male',
	'C_WAERTER': 'base_male',
	'C_WACHE_KRON_W': 'base_female',
	'C_WACHE_KRON_M': 'base_male',
	'C_WACHE_KRON_W_E': 'base_female',
	'C_WACHE_KRON_M_E': 'base_male',	
	'C_HE_BANKER_F': 'base_female',
	'C_HE_JEWELER_F': 'base_female',	
	'C_HE_ALCHEMIST_F': 'base_female',
	'C_HE_TINKER': 'base_male',
	'C_HE_ALCHEMIST': 'base_male',
	'C_HE_CARPENTER': 'base_male',
	'C_HE_WEAPONSMITH_BLUNT': 'base_male',
	'C_HE_ANIMALTRAINER_F': 'base_female',
	'C_HE_VEGISELLER_F': 'base_female',
	'C_HE_WEAPONSMITH_BLADE_F': 'base_female',
	'C_HE_BOWYER': 'base_male',
	'C_HE_BAKER_F': 'base_female',
	'C_HE_WEAVER_F': 'base_female',
	'C_WACHE_HE': 'base_male',
	'C_WACHE_HEMAGE': 'base_male',
	'C_WACHE_ORK': 'base_male',
	'C_WACHE_ASTA_M_E': 'base_female',
	'C_ENTHAUPTETER': 'base_male',
	'C_HE_FISHER': 'base_male',
	'C_HE_TAILOR_F': 'base_female',		
}

#
# A NPC AI Translation table
#
NPCAI = {
	0: 'Normal_Base', # NO BRAIN
	1: 'Animal_Wild', # Animal (Tameable)
	2: 'Normal_Base', # Generic Human
	3: 'Normal_Base', # Healer with Res ability
	4: 'Human_Guard', # City Guards
	5: 'Human_Vendor', # Banker
	6: 'Human_Vendor', # Vendor
	7: 'Normal_Base', # Beggar
	8: 'Human_Stablemaster', # Stabler (might need overhaul)
	9: 'Normal_Base', # Thief
	10: 'Monster_Aggressive_L1', # Monster
	11: 'Monster_Aggressive_L1', # Berserk (Blades, Vortex)
	12: 'Monster_Aggressive_L1', # Undead
	13: 'Monster_Aggressive_L1', # Dragon
	14: 'Normal_Base', # Vendor offduty (?)
	15: 'Normal_Base', # Town Crier
	16: 'Normal_Base', # Conjured
}
