#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Created by: DarkStorm                      #
#   )).-' {{ ;'`   | Revised by:                                #
#  ( (  ;._ \\ ctr | Last Modification: Created                 #
#===============================================================#
# Constants used in Python scripts                              #
#===============================================================#

# Which port should the remote admin run on
REMOTEADMIN_PORT = 2594

# Spell Settings
CLUMSY_DURATION = 60000

# How long does it take Cotton plants to regrow (in seconds)
COTTONPLANTS_REGROW = 30

# Potions
POTION_LESSERHEAL_RANGE = [ 1, 16 ]
POTION_HEAL_RANGE = [ 1, 16 ]
POTION_GREATERHEAL_RANGE = [ 1, 16 ]

# Fishing Settings
FISHING_MIN_FISH = 25	 # Minimum fish in a 8x8 block
FISHING_MAX_FISH = 30	 # Maximum fish in a 8x8 block
FISHING_REFILLTIME = 60 * 5 # 5 Minute refresh time
FISHING_MAX_DISTANCE = 6 # Maximum Distance to fishing ground
FISHING_BLOCK_RANGE = 13 # How many Z-Units of free space need to be above the fishing ground

# Lumberjacking Settings
LUMBERJACKING_MIN_LOGS = 10
LUMBERJACKING_MAX_LOGS = 15
LUMBERJACKING_MIN_SKILL = 0
LUMBERJACKING_MAX_SKILL = 600
LUMBERJACKING_REFILLTIME = 60 * 5 # 5 Minutes Refresh time for trees

# Skill Constants
ALCHEMY = 0
ANATOMY = 1
ANIMALLORE = 2
ITEMID = 3
ARMSLORE = 4
PARRYING = 5
BEGGING = 6
BLACKSMITHING = 7
BOWCRAFT = 8
PEACEMAKING = 9
CAMPING = 10
CARPENTRY = 11
CARTOGRAPHY = 12
COOKING = 13
DETECTINGHIDDEN = 14
ENTICEMENT = 15
EVALUATINGINTEL = 16
HEALING = 17
FISHING = 18
FORENSICS = 19
HERDING = 20
HIDING = 21
PROVOCATION = 22
INSCRIPTION = 23
LOCKPICKING = 24
MAGERY = 25
MAGICRESISTANCE = 26
TACTICS = 27
SNOOPING = 28
MUSICIANSHIP = 29
POISONING = 30
ARCHERY = 31
SPIRITSPEAK = 32
STEALING = 33
TAILORING = 34
TAMING = 35
TASTEID = 36
TINKERING = 37
TRACKING = 38
VETERINARY = 39
SWORDSMANSHIP = 40
MACEFIGHTING = 41
FENCING = 42
WRESTLING = 43
LUMBERJACKING = 44
MINING = 45
MEDITATION = 46
STEALTH = 47
REMOVETRAPS = 48
NECROMANCY = 49
FOCUS = 50
CHIVALRY = 51

ALLSKILLS = 52 # skills + 1

skillnames = [ 'alchemy', 'anatomy', 'animallore', 'itemid', 'armslore', 'parrying',
	'begging', 'blacksmithing', 'bowcraft', 'peacemaking', 'camping', 'carpentry',
	'cartography', 'cooking', 'detectinghidden', 'enticement', 'evaluatingintel',
	'healing', 'fishing', 'forensics', 'herding', 'hiding', 'provocation',
	'inscription', 'lockpicking', 'magery', 'magicresistance', 'tactics', 'snooping',
	'musicianship', 'poisoning', 'archery', 'spiritspeak', 'stealing', 'tailoring',
	'taming', 'tasteid', 'tinkering', 'tracking', 'veterinary', 'swordsmanship',
	'macefighting', 'fencing', 'wrestling', 'lumberjacking', 'mining', 'meditation',
	'stealth', 'removetraps', 'necromancy', 'focus', 'chivalry' ]

#Stats
MANA = 0
STAMINA = 1
HITPOINTS = 2
DEXTERITY = 3
INTELLIGENCE = 4
STRENGTH = 5


#Requirements
MANACOST = 0
REAGENTCONST = 1
GLB_REQUIREMENT = 2

#Damage by
SPELL = 0
COLD = 1
DISPEL = 2
ENERGY = 3
FIRE = 4
HITCHANCE = 5
LOWERATTACK = 6
LOWERDEFENCE = 7	   
#... here we need to add every spell also

#Enhancements
GOLD = 0           #Gold increase
SWINGSPEED = 1	   #Swing speed increase
POTIONS = 2	   #Enhance potions
SELFREPAIR = 3	   #Self repair
DEFENCECHANCE = 4  #Defence chance
CASTSPEED = 5	   #Faster casting
CASTRECOVERY = 6   #Faster cast recovery

#Advanced properties ID's
REGEN = 1	
BONUS = 2	
DAMAGE = 3
ENH = 4	
HIT = 5	
REQ = 6	
RESIST = 7
REFLECT = 8


# Constants for char.sound
SND_STARTATTACK = 0
SND_IDLE = 1
SND_ATTACK = 2
SND_DEFEND = 3
SND_DIE = 4

# Layer
LAYER_RIGHTHAND = 1
LAYER_LEFTHAND = 2
LAYER_SHOES = 3
LAYER_PANTS = 4
LAYER_SHIRT = 5
LAYER_HELM = 6
LAYER_GLOVES = 7
LAYER_RING = 8
LAYER_UNUSED1 = 9
LAYER_NECK = 10
LAYER_HAIR = 11
LAYER_WAIST = 12
LAYER_CHEST = 13
LAYER_BRACELET = 14
LAYER_UNUSED2 = 15
LAYER_BEARD = 16
LAYER_TORSO = 17
LAYER_EARRINGS = 18
LAYER_ARMS = 19
LAYER_CLOAK = 20
LAYER_BACKPACK = 21
LAYER_ROBE = 22
LAYER_SKIRT = 23
LAYER_LEGS = 24
LAYER_MOUNT = 25
LAYER_NPCRESTOCK = 26
LAYER_NPCNORESTOCK = 27
LAYER_NPCSELL = 28
LAYER_BANKBOX = 29
LAYER_DRAGGING = 30
LAYER_TRADING = 31

# Several Hook Constants
HOOK_OBJECT = 1
HOOK_ITEM = 2
HOOK_CHAR = 3

# Constants for the Events we can hook
EVENT_USE			= 0x00000001
EVENT_SINGLECLICK		= 0x00000002
EVENT_COLLIDEITEM		= 0x00000004
EVENT_COLLIDECHAR		= 0x00000008
EVENT_WALK			= 0x00000010
EVENT_CREATE			= 0x00000020
EVENT_LOGIN			= 0x00000040
EVENT_LOGOUT			= 0x00000080
EVENT_TALK			= 0x00000100
EVENT_WARMODETOGGLE		= 0x00000200
EVENT_HELP			= 0x00000400
EVENT_CHAT			= 0x00000800
EVENT_SKILLUSE			= 0x00001000
EVENT_SELECTCONTEXTMENU		= 0x00002000
EVENT_REQUESTCONTEXTMENU	= 0x00004000
EVENT_DROPONCHAR		= 0x00008000
EVENT_DROPONITEM		= 0x00010000
EVENT_DROPONGROUND		= 0x00020000
EVENT_PICKUP			= 0x00040000
EVENT_SPEECH			= 0x00080000
EVENT_BEGINCAST			= 0x00100000
EVENT_ENDCAST			= 0x00200000
EVENT_SPELLCHECKTARGET		= 0x00400000
EVENT_DAMAGE			= 0x08000000
EVENT_SHOWTOOLTIP		= 0x02000000
EVENT_CASTSPELL			= 0x80000000

# Effect Constants
EFFECT_MOVING        = 0
EFFECT_LIGHTNING     = 1
EFFECT_STAYSOURCEPOS = 2
EFFECT_STAYSOURCESER = 3

HOUSE = 0
BOAT = 1
CUSTOMHOUSE = 2

FALSE = 0
TRUE = 1

# Definitions for Magic (Casting Source)
CAST_BOOK = 0
CAST_SCROLL = 1

CAST_TARGET_CHAR = 0x01 # Agressive Spells, Heal, Etc.
CAST_TARGET_ITEM = 0x02 # Recall
CAST_TARGET_GROUND = 0x04 # Other

# Different Type of Damages
DAMAGE_PHYSICAL = 0
DAMAGE_MAGICAL = 1
DAMAGE_GODLY = 2
DAMAGE_HUNGER = 3

# Some nice animation definitions
ANIM_WALKUNARMED = 0x00
ANIM_WALKARMED = 0x01
ANIM_RUNUNARMED = 0x02
ANIM_RUNARMED = 0x03
ANIM_STAND = 0x04
ANIM_FIDGET1 = 0x05
ANIM_FIDGET2 = 0x06
ANIM_ATTACKREADY1 = 0x07
ANIM_ATTACKREADY2 = 0x08
ANIM_ATTACK1=0x09
ANIM_ATTACK2=0x0a
ANIM_ATTACK3=0x0b
ANIM_ATTACK4=0x0c
ANIM_ATTACK5=0x0d
ANIM_ATTACK6=0x0e
ANIM_ATTACKWALK=0x0f
ANIM_CASTDIRECTED=0x10
ANIM_CASTAREA=0x11
ANIM_ATTACKBOW=0x12
ANIM_ATTACKXBOX=0x13
ANIM_TAKEHIT=0x14
ANIM_DIE1=0x15
ANIM_DIE2=0x16
ANIM_TURN=0x1e
ANIM_ATTACK7=0x1f
ANIM_BOW=0x20
ANIM_SALUTE=0x21
ANIM_FIDGET3=0x22

# Definitions for the Sextant
SEXTANT_CENTER_X = 5936
SEXTANT_CENTER_Y = 3112
SEXTANT_MAP_HEIGHT = 5120
SEXTANT_MAP_WIDTH = 4096
