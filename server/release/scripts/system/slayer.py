
import wolfpack
from wolfpack import console
from wolfpack.consts import *
import random

class SlayerEntry:
	def __init__(self, id, name, npcs):
		self.id = id
		self.name = name
		self.npcs = npcs
		self.group = None

	# Does this slayer slay the given npc?
	def slays(self, npc):
		npc.say('CHECKING SLAYING: ' + str(self.npcs))
		
		if not npc.hasstrproperty('slayer_group'):
			return False

		slayer_group = npc.getstrproperty('slayer_group', '')		
		
		if slayer_group in self.npcs:
			return True
		else:
			return False

class SlayerGroup:
	def __init__(self):
		self.opposition = None # Opposing group
		self.super = None # Super slayer for this group
		self.entries = [] # Normal Slayer Entries for this group

	def addsuper(self, sup):
		self.super = sup
		sup.group = self

	def add(self, entry):
		self.entries.append(entry)
		entry.group = self

# Create Slayer Entries
humanoid = SlayerGroup()
undead = SlayerGroup()
elemental = SlayerGroup()
abyss = SlayerGroup()
arachnid = SlayerGroup()
reptilian = SlayerGroup()

# HUMANOID SLAYER GROUP
humanoid.opposition = undead
humanoid.addsuper( SlayerEntry('repond', 1017388, ['orcs', 'ogres', 'trolls', 'giants'] ) )
humanoid.add( SlayerEntry('ogretrashing', 1017387, ['ogres']) )
humanoid.add( SlayerEntry('orcslaying', 1017385, ['orcs']) )
humanoid.add( SlayerEntry('trollslaying', 1017386, ['trolls']) )

# UNDEAD SLAYER GROUP
undead.opposition = humanoid
undead.addsuper( SlayerEntry('silver', 1017384, ['undeads']) ) # Only super slayer

# ELEMENTAL SLAYER GROUP
elemental.opposition = abyss
elemental.addsuper( SlayerEntry('elementalban', 1017409, ['bloodelementals', 'earthelementals', 'poisonelementals', 'fireelementals', 'snowelementals', 'airelementals', 'waterelementals', 'oreelementals']) )
elemental.add( SlayerEntry( 'blooddrinking', 1017407, ['bloodelementals'] ) )
elemental.add( SlayerEntry( 'earthshatter', 1017406, ['earthelementals'] ) )
elemental.add( SlayerEntry( 'elementalhealth', 1017405, ['poisonelementals'] ) )
elemental.add( SlayerEntry( 'flamedousing', 1017402, ['fireelementals'] ) )
elemental.add( SlayerEntry( 'summerwind', 1017408, ['snowelementals'] ) )
elemental.add( SlayerEntry( 'vacuum', 1017404, ['airelementals'] ) )
elemental.add( SlayerEntry( 'waterdissipation', 1017403, ['waterelementals'] ) )

# ABYSS SLAYER GROUP
abyss.opposition = elemental
abyss.addsuper( SlayerEntry('exorcism', 1017397, ['daemons', 'gargoyles', 'balron']) )
abyss.add( SlayerEntry('daemondismissal', 1017394, ['daemons']) )
abyss.add( SlayerEntry('gargoylesfoe', 1017395, ['gargoyles']) )
abyss.add( SlayerEntry('balrondamnation', 1017396, ['balron']) )

# ARACHNID SLAYER GROUP
arachnid.opposition = reptilian
arachnid.addsuper( SlayerEntry('arachniddoom', 1017401, ['spiders', 'scorpions', 'tarathans', 'mephitis']) )
arachnid.add( SlayerEntry('scorpionsbane', 1017400, ['scorpions']) )
arachnid.add( SlayerEntry('spidersdeath', 1017399, ['spiders']) )
arachnid.add( SlayerEntry('terathan', 1017390, ['terathans']) )

# REPTILIAN SLAYER GROUP
reptilian.opposition = arachnid
reptilian.addsuper( SlayerEntry( 'reptiliandeath', 1017393, ['dragons', 'lizards', 'ophidians', 'snakes'] ) )
reptilian.add( SlayerEntry( 'dragonslaying', 1017389, ['dragons'] ) )
reptilian.add( SlayerEntry( 'lizardmanslaughter', 1017392, ['lizards'] ) )
reptilian.add( SlayerEntry( 'ophidian', 1017398, ['ophidians'] ) )
reptilian.add( SlayerEntry( 'snakes', 1017391, ['snakes'] ) )

GROUPS = [humanoid, undead, elemental, abyss, arachnid, reptilian]
SLAYERS = {}

# Build Slayer Entry table
for group in GROUPS:
	if group.super:
		SLAYERS[group.super.id] = group.super
	for entry in group.entries:
		SLAYERS[entry.id] = entry

#
# Find a slayer
#
def findEntry(id):
	id = id.lower()
	if not SLAYERS.has_key(id):
		return None
	else:
		return SLAYERS[id]

#
# Get an entirely random slayer entry
#
def getRandom():
	# Select random group
	group = random.choice(GROUPS)
	
	# 10% chance for a super slayer
	if 0.10 > random.random():
		return group.super.id
	elif len(group.entries) > 0:
		return random.choice(group.entries).id

	return None
