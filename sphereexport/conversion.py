
from types import *
from config import ORES, INGOTS, COLORS, NPCS, NPCAI
from math import ceil, floor
from resources import MISSINGNPCS, TILEINFO, findobject
from consts import *
import random

def quote(name):
	return name.replace("'", "''")

def savetag(output, serial, name, value):	
	if type(value) == IntType:
		tagtype = 'Int'
	elif type(value) == FloatType:
		tagtype = 'Double'
	else:
		tagtype = 'String'
	value = str(value)

	sql = "INSERT INTO tags VALUES(%u, '%s', '%s', '%s');\n"
	output.write(sql % (serial, quote(name), tagtype, quote(value)))

def convertitem(output, item):
	if item.amount == 0:
		return

	classname = 'cItem' # fixed for now
	name = item.name
	
	# Replace %[^%]%
	
	tags = {}
	dispid = item.dispid
	events = []
	baseid = '%x' % item.dispid
	contserial = -1
	layer = 0
	itemtype = 0
	amount = item.amount
	hits = 0
	maxhits = 0
	magic = 0 # Gm movable
	owner = -1 # Normal
	visible = 0 # normally visible
	spawnregion = ''
	flags = 0
	buyprice = item.price
	sellprice = floor(item.price * 0.75)
	restock = item.amount # No shop items should be imported (RESTOCK: item.layer)

	if item.container:
		contserial = item.container.serial
		if contserial < 0x40000000:
			layer = item.layer

	# Delete restock information for unvended items.	
	if not item.container or (item.container.isitem() and (item.container.layer < 26 or item.container.layer > 28)):
		restock = 0
	else:
		# switch... restock is the amount we can sell while amount is the maximum amount.
		if item.pos[2] > 0:
			amount = item.pos[2]
		elif amount < 1:
			amount = 1

	# Locked Items
	if item.type == T_DOOR or item.type == T_DOOR_LOCKED or \
		item.type == T_CONTAINER or item.type == T_CONTAINER_LOCKED:
		if item.getproperty('MORE1'):
			events.append('lock')
			tags['lock'] = item.getproperty('MORE1')
			
	if item.type == T_DOOR_LOCKED or item.type == T_CONTAINER_LOCKED:
		tags['locked'] = 1

	# Doors
	if item.type in [T_DOOR_LOCKED, T_DOOR_OPEN, T_DOOR]:
		events.append('door')

	# Containers
	elif item.type in [T_CONTAINER, T_CONTAINER_LOCKED, T_EQ_BANK_BOX, T_EQ_VENDOR_BOX]:
		itemtype = 1

	# Rocks (Disregard i_worldgem_bit's with T_ROCK type)
	elif item.type == T_ROCK:
		if item.baseid == 'I_WORLDGEM_BIT':
			return

	# Mount Items
	elif item.type == T_EQ_HORSE:
		if not item.hasproperty('MORE2'):
			return
		tags['pet'] = item.hex2dec(item.getproperty('MORE2'))

	# Figurine Items (Stablemasters, Shrinked NPCs)
	elif item.type == T_FIGURINE:		
		if item.hasproperty('MORE2'):
			pet = findobject(item.hex2dec(item.getproperty('MORE2')))
			if pet and pet.saved:
				# "UPDATE" it
				# This is rather special but should work
				pass
				
			# Shrinked NPCs shouldn't be owned by anyone
			# Otherwise they count agains the follower limit
			elif pet:
				events.append('figurine')
				tags['pet'] = pet.serial
				pet.stablemaster = item.serial
				if item.link != 0:
					tags['player'] = item.link

	# Keys
	elif item.type == T_KEY:
		events.append('key')
		if name[:7] == 'Key to:':
			name = name[7:]

		if item.getproperty('MORE1'):
			tags['lock'] = item.getproperty('MORE1')

	# Light objects
	elif item.type in [T_LIGHT_LIT, T_LIGHT_OUT]:
		events.append('lightsource')

	# Food
	elif item.type == T_FOOD:
		events.append('food')
		
	# Res Shrines (Walk On)
	elif item.type == T_SHRINE:
		itemtype = 16
		events.append('shrine')
		
 	# Convert Spellbook
	elif item.type == T_SPELLBOOK:
 		events.append('spellbook')
 		spells1 = 0
 		spells2 = 0
 		if item.hasproperty('MORE1'):
 			spells1 = item.hex2dec(item.getproperty('MORE1'))
		if item.hasproperty('MORE2'):
 			spells2 = item.hex2dec(item.getproperty('MORE2'))
 		for circle in range(1, 5):
 			spells = (spells1 >> ((circle - 1) * 8)) & 0xFF
 			tags['circle%u' % circle] = spells

 		for circle in range(5, 9):
 			spells = (spells2 >> ((circle - 5) * 8)) & 0xFF
 			if spells != 0:
 				tags['circle%u' % circle] = spells

	# Convert Armors
	# Try to convert it to one of the "normal" ores/colors
	# Simply use the real displayid for getting the new color/resname/etc.
	elif item.type == T_ARMOR:
		dura = item.hex2dec(item.getproperty('MORE1', '0'))
		hits = dura & 0xFFFF
		maxhits = (dura >> 16) & 0xFFFF
		itemtype = 1009
		name = ''
		events.append('equipment')

		# Find a suitable color and resname here.
		if COLORS.has_key(item.color):
			info = COLORS[item.color]
			item.color = info[1]
			tags['resname'] = info[0]
		else:
			print "Unknown color %x for armor %x found. Converting to iron." % (item.serial,item.color)
			# Exceptional items are not convertable, crafted by will be discarded as well.
			tags['resname'] = 'iron'
			item.color = 0

		# Grant special effects based on the resname
		DURABONUS = {
			'dullcopper': 1.5,
			'shadowiron': 2.0,
			'valorite': 1.5
			}
			
		if DURABONUS.has_key(tags['resname']):
			bonus = DURABONUS[tags['resname']]
			if hits == maxhits:
				maxhits = int(ceil(maxhits * bonus))
				hits = maxhits
			else:
				maxhits = int(ceil(maxhits * bonus))
				hits = int(ceil(hits * bonus))

		# Dont forget the suitable strength requirement as well.
		# Use the default table here.
		pass

	#
	# There is only one type of leather yet.
	#
	elif item.type == T_ARMOR_LEATHER:
		item.name = ''
		item.color = 0
		tags['resname'] = 'leather'
		dura = item.hex2dec(item.getproperty('MORE1', '0'))
		hits = dura & 0xFFFF
		maxhits = (dura >> 16) & 0xFFFF
		itemtype = 1009
		events.append('equipment')

	# There is nothing we could do to convert these
	# at the moment. Maybe just skip?
	elif item.type in [T_WEAPON_MACE_SMITH, T_WEAPON_MACE_SHARP, T_WEAPON_SWORD, T_WEAPON_FENCE, T_WEAPON_BOW, T_WEAPON_MACE_STAFF, T_WEAPON_XBOW, T_WEAPON_AXE]:
		if item.type == T_WEAPON_MACE_SMITH:
			tags['remaining_uses'] = 50
			events.append('skills.blacksmithing')
			
		# Translate types
		if item.type == T_WEAPON_MACE_SMITH:
			itemtype = 1004
		elif item.type == T_WEAPON_MACE_SHARP:
			itemtype = 1002 # Axe
			events.append('weapons.blades')
		elif item.type == T_WEAPON_AXE:
			itemtype = 1002 # Axe
			events.append('weapons.blades')
		elif item.type == T_WEAPON_MACE_STAFF:
			itemtype = 1003
		elif item.type == T_WEAPON_SWORD:
			itemtype = 1001
			events.append('weapons.blades')
		elif item.type == T_WEAPON_FENCE:
			itemtype = 1005
			events.append('weapons.blades')
		elif item.type == T_WEAPON_BOW:
			tags['range'] = 12
			itemtype = 1006
		elif item.type == T_WEAPON_XBOW:
			tags['range'] = 12
			itemtype = 1007

	# No support for wands in wolfpack yet.
	elif item.type == T_WAND:
		pass

	# Scissors
	elif item.type == T_SCISSORS:
		events.append('scissors')

	# Make sure resources will work in wolfpack
	elif item.type == T_LOG:
		name = ''
		baseid = '1bdd'
		dispid = 0x1bdd
		item.color = 0
		tags['resname'] = 'plainwood'
		
	elif item.type == T_BOARD:
		name = ''
		baseid = '1bd7'
		dispid = 0x1bd7
		item.color = 0
		tags['resname'] = 'plainwood'

	# Convert all types of hides to one type of hide
	elif item.type == T_HIDE:
		item.color = 0
		tags['resname'] = 'leather'
		name = ''
		baseid = 'leather_hides'
		
	# Convert all leather types to one leathertype
	elif item.type == T_LEATHER:
		item.color = 0
		tags['resname'] = 'leather'
		name = ''
		baseid = 'leather_cut'
		
	# Spawn Detected
	elif item.type == T_SPAWN_CHAR:
		if item.hasproperty('MORE1') and item.hasproperty('MOREP'):
			if item.hasproperty('MORE2'):
				more2 = item.hex2dec(item.getproperty('MORE2'))
			else:
				more2 = 0
			dispid = 0x1F13 # Worldgem Large
			item.color = 0
			npc = item.getproperty('MORE1').upper()
			props = map(int, item.getproperty('MOREP').split(','))
			if not NPCS.has_key(npc):
				if not npc in MISSINGNPCS:
					MISSINGNPCS.append(npc)
			else:
				tags['spawndef'] = NPCS[npc]
				tags['spawntype'] = 1
				tags['current'] = more2 # We only increase this number by npcs we find
				tags['mininterval'] = props[0]
				tags['maxinterval'] = props[1]
				if len(props) >= 3:
					tags['area'] = props[2]
				tags['maximum'] = amount
				events.append('spawngem')

		else:
			print "Broken NPC Spawn: 0%x" % item.serial
	
	# recall rune
	elif item.type == T_RUNE:
		events.append('recall_rune')
		if item.hasproperty('MOREP') and item.hasproperty('MORE1'):
			tags['charges'] = item.hex2dec(item.getproperty('MORE1'))
			location = ['0', '0', '0', '1']
			morep = item.getproperty('MOREP').split(',')
			for i in range(0, len(morep)):
				location[i] = morep[i]
			tags['marked'] = 1
			tags['location'] = ','.join(location)			
			
		# Normalize name
		if name.startswith('Rune to:'):
			name = name[8:]
			
	# Keyrings
	elif item.type == T_KEYRING:
		events.append('keyring')
	
	elif item.type == T_ORE:
		# Resolve the correct wolfpack ore based on a 
		# sphere2wolfpack translation table (config)
		events.append('ore')
		dispid = 0x19b7 # We convert to one ore-id
		itemtype = 1102

		if not ORES.has_key(item.baseid):
			print "Unknown Ore: %s. Converting to iron." % item.baseid
			name = '#1042853'
			baseid = 'iron_ore_1' # Convert to the correct baseid
			tags['resname'] = 'iron'
			item.color = 0 # Convert to the correct color
		else:
			ore = ORES[item.baseid]
			name = ore[0]
			baseid = ore[1] # Convert to the correct baseid
			tags['resname'] = ore[2]
			item.color = ore[3] # Convert to the correct color
			
	elif item.type == T_INGOT:
		# Resolve the correct wolfpack ore based on a 
		# sphere2wolfpack translation table (config)
		dispid = 0x1bf2
		if not INGOTS.has_key(item.baseid):
			print "Unknown Ingot: %s. Converting to iron." % item.baseid
			name = '#1042692'
			baseid = 'iron_ingot' # Convert to the correct baseid
			tags['resname'] = 'iron'
			item.color = 0 # Convert to the correct color
		else:
			ingot = INGOTS[item.baseid]
			name = ingot[0]
			baseid = ingot[1] # Convert to the correct baseid
			tags['resname'] = ingot[2]
			item.color = ingot[3] # Convert to the correct color			
		
	# Teleporters	
	elif item.type == T_TELEPAD:
		# More 1: Players only
		# More 2: No noise
		# MoreP: Target
		if item.hasproperty('MOREP'):
			target = item.getproperty('MOREP').split(',')
			x = 0
			y = 0
			z = 0
			x = int(target[0])
			if len(target) >= 2:
				y = int(target[1])
			if len(target) >= 3:
				z = int(target[2])
			target = '%u,%u,%d,1' % (x, y, z)
			tags['target'] = target

		if item.hasproperty('MORE2'):
			prop = item.hex2dec(item.getproperty('MORE2'))
			if prop != 0:
				tags['silent'] = 1
				
		if item.hasproperty('MORE1'):
			prop = item.hex2dec(item.getproperty('MORE1'))
			if prop != 0:
				tags['playersonly'] = 1

		events.append('gate')

	# Carpentry Tools
	elif item.type == T_CARPENTRY:
		events.append('skills.carpentry')
		tags['remaining_uses'] = 50
		
	# Tailoring Tools
	elif item.type == T_SEWING_KIT:
		events.append('skills.tailoring')
		tags['remaining_uses'] = 50
		
	# Tinker Tools
	elif item.type == T_TINKER_TOOLS:
		events.append('skills.tinkering')
		tags['remaining_uses'] = 50

	# Convert book
	elif item.type == T_BOOK:
		events.append('book')

		if item.hasproperty('AUTHOR'):
			tags['author'] = item.getproperty('AUTHOR')

		# Process all pages (1-64)
		for i in range(0, 64):
			page = 'BODY.%u' % i
			if item.hasproperty(page):
				tags['page%u' % (i + 1)] = item.getproperty(page).replace("\t", "\n")

	# Raw Meat -> Cooking
	elif item.type == T_FOOD_RAW:
		events.append('cooking')

	# Bulletin Boards
	elif item.type == T_BBOARD:
		events.append('bulletinboard')

	# Dye Tub + Hairdye
	elif item.type in [T_DYE_VAT, T_HAIR_DYE]:
		events.append('environment')

	# Dyes
	elif item.type == T_DYE:
		events.append('dyes')
		tags['remaining_uses'] = 10

	# Bandages
	elif item.type == T_BANDAGE:
		events.append('bandages')

	# Musical Instrument
	elif item.type == T_MUSICAL:
		events.append('environment')

	# Training
	elif item.type == T_ARCHERY_BUTTE:
		events.append('archery_butte')

	elif item.type == T_TRAIN_DUMMY:
		events.append('training_dummy')
		
	else:
		# Add a tag containing the original sphere 
		# information
		tags['sphere_conversion'] = 'Type: %u, Baseid: %s' % (item.type, item.baseid)

	# Process Attributes
	if item.attr & 0x0010: # Never movable
		magic = 2 # GM Movable
		
	# If not stated otherwise
	#if not item.attr & 0x4000:	
	#	if item.attr & (ATTR_MOVE_NEVER|ATTR_MOVE_ALWAYS|ATTR_OWNED|ATTR_INVIS|ATTR_STATIC):
	#		flags |= 0x01 # No Decay
	#	elif TILEINFO[item.dispid]['weight'] == 255:
	#		flags |= 0x01 # No Decay
	flags |= 0x01 # Everything is nodecay *whee*
			
	if item.dye:
		flags |= 0x80

	if item.attr & 0x0004: # Newbie
		flags |= 0x02 # Newbie

	if item.attr & 0x0080: # Invisible
		visible = 2

	# UObjectmap Entry
	sql = "INSERT INTO uobjectmap VALUES(%u, '%s');\n"
	output.write(sql % (item.serial, classname))

	# UObject entry
	sql = "INSERT INTO uobject VALUES('%s', %u, %d, %u, %d, %d, %d, %u, '%s', '%s', %u);\n"
	output.write(sql % (quote(name), item.serial, -1, 0, item.pos[0], item.pos[1], item.pos[2], item.pos[3], quote(','.join(events)), '', len(tags) != 0))

	# ITEM entry
	sql = "INSERT INTO items VALUES(%u, %u, %u, %d, %u, %u, %u, %u, %f, %u, %u, %u, %d, %u, '%s', %u, %u, %u, %u, '%s');\n"
	output.write(sql % (item.serial, dispid, item.color, contserial, layer, itemtype, amount, 0, item.weight, hits, maxhits, magic, \
		owner, visible, spawnregion, flags, sellprice, buyprice, restock, baseid))

	# Tags
	for (name, value) in tags.items():
		savetag(output, item.serial, name, value)
		
	# Export all subitems
	for cont in item.content:
		convertitem(output, cont)

def convertchar(output, char):
	if len(char.name) == 0:
		charname = char.basechar.charname
	else:
		charname = char.name
	events = []
	tags = {}
	title = ''
	creationdate = '2004-01-01T00:00:00'
	body = char.dispid
	obody = char.dispid
	skin = char.color
	oskin = char.color
	saycolor = random.randint(2, 1000)
	emotecolor = 0x23
	orgname = charname
	strength = char.strength
	dexterity = char.dexterity
	intelligence = char.intelligence
	strength2 = 0
	dexterity2 = 0
	intelligence2 = 0
	maxhitpoints = char.strength # This is super important for NPCs
	hitpoints = char.hitpoints
	maxstamina = char.dexterity
	stamina = char.stamina
	maxmana = char.intelligence
	mana = char.mana
	karma = 0
	fame = 0
	kills = 0
	deaths = 0
	direction = 0
	skills = {}
	if char.hasproperty('TITLE'):
		title = char.getproperty('TITLE')
	if char.hasproperty('OKARMA'):
		karma = char.hex2dec(char.getproperty('OKARMA'))
	if char.hasproperty('DIR'):
		direction = char.hex2dec(char.getproperty('DIR'))
	if char.hasproperty('OFAME'):
		fame = char.hex2dec(char.getproperty('OFAME'))
	if char.hasproperty('KILLS'):
		kills = char.hex2dec(char.getproperty('KILLS'))
	if char.hasproperty('DEATHS'):
		deaths = char.hex2dec(char.getproperty('DEATHS'))
	defense = 0 # Unused
	hunger = 6 # We start well fed
	if char.hasproperty('FOOD'):
		hunger = min(6, max(1, char.hex2dec(char.getproperty('FOOD'))))
	poison = 0
	poisoned = 0
	murdertime = 0	
	criminaltime = 0
	gender = 0 #male
	if body == 0x191 or body == 0x193:
		gender = 1
	propertyflags = 0
	murderer = -1 # Who murdererd me
	guarding = -1 # Who am i guarding
	hitpointbonus = 0
	staminabonus = 0
	manabonus = 0
	strcap = 125
	dexcap = 125
	intcap = 125
	statcap = 225
	contextmenus = []

	if char.hasproperty('SPEECHCOLOR'):
		saycolor = char.hex2dec(char.getproperty('SPEECHCOLOR'))

	if char.attr & 0x01: # Invul
		propertyflags |= 0x1000
	if char.attr & 0x04: # Frozen
		propertyflags |= 0x100

	# Check all subitems
	# Thanks to SQL order doesnt matter
	for item in char.content:
		if item.layer == 0:
			print "Contained item %x with invalid layer." % item.serial
		elif item.layer <= 24 or (item.layer >= 26 and item.layer <= 29):
			convertitem(output, item)
		else:
			if item.layer == 30:
				if item.color == MEMORY_ISPAWNED:
					# The spawned item is in LINK
					events = ['system.spawns'] + events
					tags['spawner'] = item.link
				elif item.color == MEMORY_IPET:
					owner = item.link
					if char.attr & 0x8000000:
						propertyflags |= 0x04
			
			# We are mounted
			elif item.layer == 25 and item.hasproperty('MORE2'):
				pet = findobject(item.hex2dec(item.getproperty('MORE2')))
				if pet:
					# Create a mountitem and set the stablemaster serial for the ridden
					# pet.
					pet.stablemaster = char.serial
					convertitem(output, item)

	if char.hasproperty('ACCOUNT'):		
		classname = 'cPlayer'
		account = char.getproperty('ACCOUNT').lower()	
		additionalflags = 0
		visualrange = 18
		fixedlight = 0
		if char.hasproperty('PROFILE'):
			profiletxt = char.getproperty('PROFILE').replace('\\r', "\n")
		else:
			profiletxt = ''

		# Custom Ancient-Realms conversion
		if 'E_R_M' in char.events:
			tags['race'] = 'mensch'
		elif 'E_R_W' in char.events:
			tags['race'] = 'waldelf'
		elif 'E_R_V' in char.events:
			tags['race'] = 'vampir'
		elif 'E_R_DW' in char.events:
			tags['race'] = 'vampir'
		elif 'E_R_DR' in char.events:
			tags['race'] = 'drow'
		elif 'E_R_O' in char.events:
			tags['race'] = 'ork'
		elif 'E_R_Z' in char.events:
			tags['race'] = 'zwerg'
		elif 'E_R_H' in char.events:
			tags['race'] = 'hochelf'

		extra = "INSERT INTO players VALUES(%u,'%s',%u,%u,'%s',%u,%u,%u,%u);\n"
		extra = extra % (char.serial, quote(account), additionalflags, visualrange, quote(profiletxt), 0, 0, 0, 0)
		
	else:
		classname = 'cNPC'
		totame = 1500 # Untameable
		summontime = 0
		additionalflags = 0
		owner = -1
		carve = ''
		spawnregion = ''
		lootlist = ''
		ai = ''
		brain = char.npc
		if not NPCAI.has_key(brain):
			print "Unknown NPC Brain %u for npc %x" % (brain, char.serial)
		else:
			ai = NPCAI[brain]
			# Invulnerable
			if brain == 6:
				propertyflags |= 0x1000
				contextmenus.append('vendor_menu')
			elif brain == 5:
				propertyflags |= 0x1000
				events.append('speech.banker')
				title = 'the Banker'
				contextmenus.append('banker_menu')
				contextmenus.append('vendor_menu')

		wandertype = 3
		wanderx1 = char.pos[0]
		wanderx2 = 0
		wandery1 = char.pos[1]
		wandery2 = 0
		wanderradius = 0
		if char.hasproperty('HOME'):
			home = char.getproperty('HOME').split(',')
			home = map(int, home)
			wanderx1 = home[0]
			wandery1 = home[1]
		if char.hasproperty('HOMEDIST'):
			wanderradius = char.hex2dec(char.getproperty('HOMEDIST'))
			
		# NPCACT_STAY => Stay where you are
		if char.action == 101:
			wandertype = 0
			
		elif char.action == 103:
			wandertype = 1

		fleeat = 10
		spellslow = 0
		spellshigh = 0
		
		extra = "INSERT INTO npcs VALUES(%u, %u, %u, %u, %u, %u, %d, '%s', '%s', %d, '%s', '%s', %u, %d, %d, %d, %d, %d, %u, %u, %u);\n"
		extra = extra % (char.serial, 0, 0, totame, summontime, additionalflags, owner, quote(carve), quote(spawnregion), char.stablemaster, \
			quote(lootlist), quote(ai), wandertype, wanderx1, wanderx2, wandery1, wandery2, wanderradius, fleeat, spellslow, spellshigh)

	sql = "INSERT INTO uobjectmap VALUES(%u, '%s');\n"
	output.write(sql % (char.serial, classname))

	# UObject entry
	sql = "INSERT INTO uobject VALUES('%s', %u, %d, %u, %d, %d, %d, %u, '%s', '%s', %u);\n"
	output.write(sql % (quote(charname), char.serial, -1, direction, char.pos[0], char.pos[1], char.pos[2], char.pos[3], quote(','.join(events)), quote(','.join(contextmenus)), len(tags) != 0))

	sql = "INSERT INTO characters VALUES(%u, '%s', '%s', '%s', %u, %u, %u, %u, %u, %u, %u, %d, %u, %d, %u, %d, %u, %u, %u, %u, %u, %u, %d, %d, %u, %u, \
		%u, %u, %u, %u, %u, %u, %u, %u, %d, %d, %d, %d, %d, %u, %u, %u, %u);\n"
	sql = sql % (char.serial, quote(orgname), quote(title), quote(creationdate), body, obody, skin, oskin, saycolor, emotecolor, strength, strength2, dexterity, dexterity2,
		intelligence, intelligence2, maxhitpoints, hitpoints, maxstamina, stamina, maxmana, mana, karma, fame, kills, deaths, defense, hunger, \
		poison, poisoned, murdertime, criminaltime, gender, propertyflags, murderer, guarding, hitpointbonus, staminabonus, manabonus, strcap, dexcap, intcap, \
		statcap)
	output.write(sql)

	output.write(extra)
	
	for (name, skillid) in SKILLNAMES.items():
		# Autocap at 1000
		# Also look up the locks
		if char.hasproperty(name):
			value = int(char.getproperty(name))
			lock = 0
			if char.hasproperty('SKILLLOCK[%u]' % skillid):
				lock = int(char.getproperty('SKILLLOCK[%u]' % skillid))
			sql = "INSERT INTO skills VALUES(%u,%u,%u,%u,1000);\n"
			sql = sql % (char.serial, skillid, value, lock)
			output.write(sql)

	# Tags
	for (name, value) in tags.items():
		savetag(output, item.serial, name, value)

	char.saved = 1
