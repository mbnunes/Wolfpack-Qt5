
import wolfpack
import plants.maingump
import plants.genuses
import plants.hues
import plants.resources
import plants.seed
import plants
from consts import *
import beverage
import potions.utilities
import potionkeg
import time
from wolfpack.consts import *
from wolfpack import tr
import random

#
# This is the delay for a growth check for plants
# Note: THIS IS IN SECONDS!
#
GROWTHINTERVAL = 23 * 60 * 60 # Default is 23 hours

#
# This flag indicates whether growing in the bankbox is allowed.
# RunUO allows this, but the codex of wisdom says otherwise.
#
GROWTHALLOWBANK = False

#
# This flag indicates whether growing in the backpack is allowed
#
GROWTHALLOWBACKPACK = True

#
# This flag indicates whether growing in a multi is allowed
#
GROWTHALLOWMULTI = True

#
# This is the default value of resources per plant
#
DEFAULTSEEDS = 8

#
# This is the default value of seeds per plant
#
DEFAULTRESOURCES = 8

#
# Manage the growth indicator
#
def getGrowthIndicator(plant):
	if plant.hastag('growthindicator'):
		return int(plant.gettag('growthindicator'))
	else:
		return GROWTH_NONE
def setGrowthIndicator(plant, growth):
	plant.settag('growthindicator', int(growth))

#
# Set/Get if this plant has been pollinated (stage9 and up)
#
def getPollinated(plant):
	return plant.gettag('pollinated') == 1
def setPollinated(plant, pollinated):
	plant.settag('pollinated', int(pollinated))

#
# Set/get the amount of available seeds for harvesting
#
def getAvailableSeeds(plant):
	if plant.hastag('availableseeds'):
		return int(plant.gettag('availableseeds'))
	else:
		return 0
def setAvailableSeeds(plant, value):
	if value <= 0:
		plant.deltag('availableseeds')
	else:
		plant.settag('availableseeds', int(value))

#
# Set/get the amount of remaining seeds for growth
#
def getRemainingSeeds(plant):
	if plant.hastag('remainingseeds'):
		return int(plant.gettag('remainingseeds'))
	else:
		return DEFAULTSEEDS
def setRemainingSeeds(plant, value):
	if value == DEFAULTSEEDS:
		plant.deltag('remainingseeds')
	else:
		plant.settag('remainingseeds', int(value))

#
# Set/get the amount of available resources for harvesting
#
def getAvailableResources(plant):
	if plant.hastag('availableresources'):
		return int(plant.gettag('availableresources'))
	else:
		return 0
def setAvailableResources(plant, value):
	if value <= 0:
		plant.deltag('availableresources')
	else:
		plant.settag('availableresources', int(value))

#
# Set/get the amount of remaining resources for growth
#
def getRemainingResources(plant):
	if plant.hastag('remainingresources'):
		return int(plant.gettag('remainingresources'))
	else:
		return DEFAULTRESOURCES
def setRemainingResources(plant, value):
	if value == DEFAULTRESOURCES:
		plant.deltag('remainingresources')
	else:
		plant.settag('remainingresources', int(value))

#
# Get the timestamp for the next growth
#
def getNextGrowth(plant):
	if plant.hastag('nextgrowth'):
		return int(plant.gettag('nextgrowth'))
	else:
		return 0
def setNextGrowth(plant, nextgrowth):
	plant.settag('nextgrowth', int(nextgrowth))

#
# Target response for pouring something into the plant
#
def pour_target(player, arguments, target):
	plant = wolfpack.finditem(arguments[0])
	
	if not plant or not player.canreach(plant, 3):
		return # Can no longer reach the plant
	
	if target.item and player.canreach(target.item, 3) and player.canpickup(target.item):
		pour(player, plant, target.item)

	# Always show the plant gump in the end
	onUse(player, plant)

#
# Pour something into the plant
#
def pour(player, plant, item):
	status = getStatus(plant)
	
	if status >= STATUS_DEAD:
		return

	if status == STATUS_DECORATIVE:
		plant.say(1053049, '', '', False, 0x3b2, player.socket) # This is a decorative plant, it does not need watering!
		return

	if not checkAccess(player, plant):
		plant.say(1061856, '', '', False, 0x3b2, player.socket)
		return

	# Beverage -> Water
	if item.hasscript('beverage'):
		# It's not water or doesn't have enough content
		if item.gettag('fluid') != 'water' or not item.gettag('quantity'):
			plant.say(1053069, '', '', False, 0x3b2, player.socket) # You can't use that on a plant!
		else:
			beverage.consume(item) # Consume one use
			setWater(plant, getWater(plant) + 1) # Give one more water unit
			plant.resendtooltip()
			
			player.soundeffect(0x4e) # Soundeffect
			plant.say(1061858, '', '', False, 0x3b2, player.socket) # You soften the dirt with water.			

	# Potion
	elif item.hasscript('potions'):
		potiontype = potions.utilities.getPotionType(item)
		if applyPotion(player, plant, potiontype):			
			potions.utilities.consumePotion(player, item)
			player.soundeffect(0x240)

	# Potion Keg
	elif potionkeg.isPotionkeg(item):
		# Empty potion kegs won't work
		if not item.gettag('kegfill'):
			plant.say(1053069, '', '', False, 0x3b2, player.socket) # You can't use that on a plant!
		else:
			potiontype = potions.utilities.getPotionType(item)
			if applyPotion(player, plant, potiontype):
				item.settag('kegfill', item.gettag('kegfill') - 1)
				item.resendtooltip()
				player.soundeffect(0x240)

	# Something Else
	else:
		plant.say(1053069, '', '', False, 0x3b2, player.socket) # You can't use that on a plant!	

#
# Apply a certain potion type to the plant
#
def applyPotion(player, plant, potiontype, simulate = False):
	status = getStatus(plant)
	
	# Decorative plants don't need any potions
	if status == STATUS_DECORATIVE:
		if not simulate:
			plant.say(1053049, '', '', False, 0x3b2, player.socket) # This is a decorative plant, it does not need watering!
		return False

	# Bowls of Dirt can only be watered
	if status == STATUS_DIRT:
		if not simulate:
			plant.say(1053066, '', '', False, 0x3b2, player.socket) # You should only pour potions on a plant or seed!
		return False

	full = False

	# Greater Poison or Deadly Poison
	if potiontype in [16, 17]:
		if getPoisonPotion(plant) >= 2:
			full = True
		elif not simulate:
			setPoisonPotion(plant, getPoisonPotion(plant) + 1)

	# Greater Cure
	elif potiontype == 6:
		if getCurePotion(plant) >= 2:
			full = True
		elif not simulate:
			setCurePotion(plant, getCurePotion(plant) + 1)		

	# Greater Heal
	elif potiontype == 3:
		if getHealPotion(plant) >= 2:
			full = True
		elif not simulate:
			setHealPotion(plant, getHealPotion(plant) + 1)

	# Greater Strength
	elif potiontype == 10:
		if getStrengthPotion(plant) >= 2:
			full = True
		elif not simulate:
			setStrengthPotion(plant, getStrengthPotion(plant) + 1)

	# The lesser variants of the potions
	elif potiontype in [1, 2, 4, 5, 9, 14, 15]:
		if not simulate:
			plant.say(1053068, '', '', False, 0x3b2, player.socket) # This potion is not powerful enough to use on a plant!
		return False

	# Nothing we could use
	else:
		if not simulate:
			plant.say(1053069, '', '', False, 0x3b2, player.socket) # You can't use that on a plant!
		return False

	# The potion type was right, but the plant already had enough of it
	if full:
		if not simulate:
			plant.say(1053065, '', '', False, 0x3b2, player.socket) # The plant is already soaked with this type of potion!
		return False
	else:
		if not simulate:
			plant.say(1053067, '', '', False, 0x3b2, player.socket) # You pour the potion over the plant.
		return True

#
# Calculate cliloc id for plant status
#
def getStatusCliloc(plant):
	status = getStatus(plant)

	if status >= STATUS_PLANT:
		return 1060812
	elif status >= STATUS_SAPLING:
		return 1023305
	else:
		return 1060810

#
# Manage system states
#
def getInfestation(plant):
	if plant.hastag('infestation'):
		return int(plant.gettag('infestation'))
	else:
		return 0
def setInfestation(plant, level):
	plant.settag('infestation', max(0, min(2, level)))

def getFungus(plant):
	if plant.hastag('fungus'):
		return int(plant.gettag('fungus'))
	else:
		return 0
def setFungus(plant, level):
	plant.settag('fungus', max(0, min(2, level)))

def getPoison(plant):
	if plant.hastag('poison'):
		return int(plant.gettag('poison'))
	else:
		return 0
def setPoison(plant, level):
	plant.settag('poison', max(0, min(2, level)))

def getDisease(plant):
	if plant.hastag('disease'):
		return int(plant.gettag('disease'))
	else:
		return 0
def setDisease(plant, level):
	plant.settag('disease', max(0, min(2, level)))

def getWater(plant):
	if plant.hastag('water'):
		return int(plant.gettag('water'))
	else:
		return 0
def setWater(plant, level):
	plant.settag('water', max(0, min(4, level)))

def getPoisonPotion(plant):
	if plant.hastag('poisonpotion'):
		return int(plant.gettag('poisonpotion'))
	else:
		return 0
def setPoisonPotion(plant, level):
	plant.settag('poisonpotion', max(0, min(2, level)))

def getCurePotion(plant):
	if plant.hastag('curepotion'):
		return int(plant.gettag('curepotion'))
	else:
		return 0
def setCurePotion(plant, level):
	plant.settag('curepotion', max(0, min(2, level)))	

def getHealPotion(plant):
	if plant.hastag('healpotion'):
		return int(plant.gettag('healpotion'))
	else:
		return 0
def setHealPotion(plant, level):
	plant.settag('healpotion', max(0, min(2, level)))

def getStrengthPotion(plant):
	if plant.hastag('strengthpotion'):
		return int(plant.gettag('strengthpotion'))
	else:
		return 0
def setStrengthPotion(plant, level):
	plant.settag('strengthpotion', max(0, min(2, level)))	

#
# Manage the HealthStatus
#
def getHealthStatus(plant):
	health = (float(getHealth(plant)) / float(getMaxHealth(plant))) * 100

	if health < 33:
		return HEALTH_DYING
	elif health < 66:
		return HEALTH_WILTED
	elif health < 100:
		return HEALTH_HEALTHY
	else:
		return HEALTH_VIBRANT

#
# Manage the current plant health
#
def getHealth(plant):
	if not plant.hastag('health'):
		return getMaxHealth(plant)
	else:
		return int(plant.gettag('health'))
def setHealth(plant, health):
	maxhealth = getMaxHealth(plant)

	if health != getHealth(plant):
		if health <= 0:
			plant.settag('health', 0)
			die(plant)
		else:
			plant.settag('health', min(maxhealth, health))
		plant.resendtooltip()

#
# Get maximum plant health
# Depends on the status
#
def getMaxHealth(plant):
	status = getStatus(plant)
	return status * 2 + 10

#
# Manage the plant genus
#
def getGenus(plant):
	if not plant.hastag('genus'):
		return plants.genuses.getByIndex(0)
	else:
		return plants.genuses.getByIndex(int(plant.gettag('genus')))
def setGenus(plant, genus):
	if type(genus) == int:
		plant.settag('genus', genus)
	else:
		plant.settag('genus', genus.index)

#
# Manage the plant hue
#
def getHue(plant):
	if not plant.hastag('hue'):
		return plants.hues.getById(plants.hues.COLOR_PLAIN)
	else:
		return plants.hues.getById(int(plant.gettag('hue')))
def setHue(plant, hue):
	if type(hue) == int:
		plant.settag('hue', hue)
	else:
		plant.settag('hue', hue.id)		

#
# Manage the seed genus
#
def getSeedGenus(plant):
	if not plant.hastag('seedgenus'):
		return plants.genuses.getByIndex(0)
	else:
		return plants.genuses.getByIndex(int(plant.gettag('seedgenus')))
def setSeedGenus(plant, genus):
	if type(genus) == int:
		plant.settag('seedgenus', genus)
	else:
		plant.settag('seedgenus', genus.index)

#
# Manage the seed hue
#
def getSeedHue(plant):
	if not plant.hastag('seedhue'):
		return plants.hues.getById(plants.hues.COLOR_PLAIN)
	else:
		return plants.hues.getById(int(plant.gettag('seedhue')))
def setSeedHue(plant, hue):
	if type(hue) == int:
		plant.settag('seedhue', hue)
	else:
		plant.settag('seedhue', hue.id)

#
# Manage the Status of the plant
#
def getStatus(plant):
	if plant.hastag('status'):
		return int(plant.gettag('status'))
	else:
		return STATUS_DIRT
def setStatus(plant, status):
	oldstatus = getStatus(plant)

	if oldstatus == status or status < STATUS_DIRT or status > STATUS_DEAD:
		return

	ratio = float(getHealth(plant)) / float(getMaxHealth(plant))

	plant.settag('status', int(status)) # Set the new status

	# When changing to decorative, we should remove all the unneccesary tags
	if status == STATUS_DECORATIVE:
		genus = getGenus(plant)
		hue = getHue(plant)
	
		resetPlant(plant, True, True)
		plant.removescript('plants.plant')

		plant.name = '#1061924' # Decorative plant
		plant.id = genus.itemid
		plant.movable = 1
		plant.color = hue.color
		plant.update()

	# Otherwise adjust the hitpoints
	else:
		hits = max(1, int(getMaxHealth(plant) * ratio))
		setHealth(plant, hits)
		updatePlant(plant)

	if status == STATUS_DEAD:
		resetPlant(plant, True, True)
		plant.removescript('plants.plant')

#
# Manage the fertile indicator for a plant
#
def getFertile(plant):
	return plant.gettag('fertile') == 1
def setFertile(plant, fertile):
	plant.settag('fertile', int(fertile))

#
# Shoul the plant type be shown?
#
def getShowType(plant):
	return plant.gettag('showtype') == 1
def setShowType(plant, value):
	if not value:
		plant.deltag('showtype')
	else:
		plant.settag('showtype', 1)

#
# Lets this plant die
#
def die(plant):
	status = getStatus(plant)
	
	if status >= STATUS_FULLGROWN:
		setStatus(plant, STATUS_DEAD)
	else:
		setStatus(plant, STATUS_DIRT)
		resetPlant(plant, True)

#
# Checks if the player can access the plant
#
def checkAccess(player, plant):
	# It either has to be in the backpack
	if plant.getoutmostitem() == player.getbackpack():
		return True

	# Or locked down (the multi will do the other check)
	if plant.multi or plant.lockeddown:
		return True

	return False

#
# Returns True if this plant can still grow.
#
def canGrow(plant):	
	return getStatus(plant) < STATUS_DECORATIVE

#
# This checks if the plant is at a valid growth location
# That is either the backpack of the user or a multi
#
def validGrowthLocation(plant):
	if plant.multi or plant.lockeddown:
		return GROWTHALLOWMULTI

	cont = plant.getoutmostitem()
	owner = cont.container

	# If it's in the posession of a player. Check
	# for the settings.
	if owner and owner.player:
		if cont.layer == LAYER_BACKPACK:
			return GROWTHALLOWBACKPACK

		if cont.layer == LAYER_BANKBOX:
			return GROWTHALLOWBANK

	return False

#
# This function returns true if there are mali
# for this plant
#
def hasMali(plant):
	return getInfestation(plant) > 0 or getFungus(plant) > 0 or getPoison(plant) > 0 or getDisease(plant) > 0 or getWater(plant) != 2

#
# Apply beneficial effects from potions etc.
#
def applyBoni(plant):
	poisonpotion = getPoisonPotion(plant)
	infestation = getInfestation(plant)

	# Poison potions kill off infestations
	if poisonpotion >= infestation:
		setPoisonPotion(plant, poisonpotion - infestation)
		setInfestation(plant, 0)
	else:
		setInfestation(plant, infestation - poisonpotion)
		setPoisonPotion(plant, 0)

	curepotion = getCurePotion(plant)
	fungus = getFungus(plant)

	# cure potions kill off fungus
	if curepotion >= fungus:
		setCurePotion(plant, curepotion - fungus)
		setFungus(plant, 0)
	else:
		setFungus(plant, fungus - curepotion)
		setCurePotion(plant, 0)

	healpotion = getHealPotion(plant)
	poison = getPoison(plant)
	
	# heal potions kill off poison
	if healpotion >= poison:
		setHealPotion(plant, healpotion - poison)
		setPoison(plant, 0)
	else:
		setPoison(plant, poison - healpotion)
		setHealPotion(plant, 0)

	healpotion = getHealPotion(plant)
	disease = getDisease(plant)

	# heal potions kill off disease
	if healpotion >= disease:
		setHealPotion(plant, healpotion - disease)
		setDisease(plant, 0)
	else:
		setDisease(plant, disease - healpotion)
		setHealPotion(plant, 0)

	# if there are no malicious effects left, the plant heals
	if not hasMali(plant):
		healpotion = getHealPotion(plant)
		health = getHealth(plant)
		if healpotion > 0:
			setHealth(plant, health + healpotion * 7)
		else:
			setHealth(plant, health + 2)

	# Always reset heal potions after a growth check
	setHealPotion(plant, 0)

#
# Apply malicious effects from disease etc.
#
def applyMali(plant):
	damage = 0 # Amount of damage taken

	# Damage from infestation
	infestation = getInfestation(plant)
	if infestation > 0:
		damage += infestation * random.randint(3, 6)

	# Damage from fungus
	fungus = getFungus(plant)
	if fungus > 0:
		damage += fungus * random.randint(3, 6)

	# Damage from poison
	poison = getPoison(plant)
	if poison > 0:
		damage += poison * random.randint(3, 6)

	# Damage from disease
	disease = getDisease(plant)
	if disease > 0:
		damage += disease * random.randint(3, 6)				

	# Damage from over oder underwatering the plant
	water = getWater(plant)
	if water > 2:
		damage += (water - 2) * random.randint(3, 6)
	elif water < 2:
		damage += (2 - water) * random.randint(3, 6)

	# Damage the plant
	setHealth(plant, getHealth(plant) - damage)

	# Check if the plant can still grow or died
	status = getStatus(plant)
	return status > STATUS_DIRT and status < STATUS_DECORATIVE

#
# Set the malicious effects for the next growth check
# They apply damage to the plant.
#
def updateMali(plant):
	strengthPotion = getStrengthPotion(plant)
	water = getWater(plant)
	genus = getGenus(plant)
	hue = getHue(plant)

	# Check if the plant gets infected with insects
	infestationChance = 0.30 - strengthPotion * 0.075 + ( water - 2 ) * 0.10

	# Flowers have a 10% higher chance to become infected
	if genus.flower:
		infestationChance += 0.10

	# For plants with a bright color, the chance is another 10% higher
	if hue.bright:
		infestationChance += 0.10

	# Check if we get infected
	if infestationChance >= random.random():
		setInfestation(plant, getInfestation(plant) + 1)

	# Check if the plant gets infected with fungus
	fungusChance = 0.15 - strengthPotion * 0.075 + ( water - 2 ) * 0.10

	if fungusChance >= random.random():
		setFungus(plant, getFungus(plant) + 1)

	# Consume water
	water = getWater(plant)	
	if water > 2 or 0.9 >= random.random():
		setWater(plant, water - 1)

	# Too many poison and cure potions will cause disease and poison
	poisonPotion = getPoisonPotion(plant)
	if poisonPotion > 0:
		setPoison(plant, poisonPotion)
		setPoisonPotion(plant, 0)

	curePotion = getCurePotion(plant)
	if curePotion > 0:
		setDisease(plant, curePotion)
		setCurePotion(plant, 0)

	# Consume all strength potions
	setStrengthPotion(plant, 0)

#
# Grow the plant
#
def growPlant(plant):
	healthstatus = getHealthStatus(plant)
	status = getStatus(plant)
	hue = getHue(plant)
	genus = getGenus(plant)

	# Not healthy enough to grow
	if healthstatus in [HEALTH_DYING, HEALTH_WILTED]:
		setGrowthIndicator(plant, GROWTH_NOTHEALTHY)

	# If Fertile Dirt was used to fill the bowl, there is 
	# a 10% chance that the plant will grow twice up until
	# stage 5
	if getFertile(plant) and status <= STATUS_STAGE5 and 0.10 >= random.random():
		setStatus(plant, status + 2)
		setGrowthIndicator(plant, GROWTH_DOUBLEGROWN)

	# Normal Growth
	elif status < STATUS_STAGE9:
		setStatus(plant, status + 1)
		setGrowthIndicator(plant, GROWTH_GROWN)

	# We've reached the final stage
	# Now we produce resources and seeds
	else:
		# Produce Seeds
		if getPollinated(plant):
			remainingSeeds = getRemainingSeeds(plant)
			if remainingSeeds > 0 and genus.crossable and hue.crossable:
				setRemainingSeeds(plant, remainingSeeds - 1)
				setAvailableSeeds(plant, getAvailableSeeds(plant) + 1)

		# Produce Resources
		remainingResources = getRemainingResources(plant)
		if remainingResources > 0 and plants.resources.canProduce(genus, hue):
			setRemainingResources(plant, remainingResources - 1)
			setAvailableResources(plant, getAvailableResources(plant) + 1)

		# Indicate the Growth but don't change the status anymore
		setGrowthIndicator(plant, GROWTH_GROWN)

	# From Stage 9 and up, the plant is automatically pollinated by itself
	if status >= STATUS_STAGE9 and not getPollinated(plant):
		setPollinated(plant, True)
		setSeedGenus(plant, genus)
		setSeedHue(plant, hue)

#
# This function does all neccesary growth checks for this plant
#
def growthCheck(plant):
	# Check if this plant can grow
	if not canGrow(plant):
		return

	# Get the next growth time
	nextGrowth = getNextGrowth(plant)

	# It's not time yet
	if nextGrowth > time.time():
		setGrowthIndicator(plant, GROWTH_DELAY)
		return

	# Now that we're checking, set the next growth
	setNextGrowth(plant, time.time() + GROWTHINTERVAL)

	# First check if the location is ok
	if not validGrowthLocation(plant):
		setGrowthIndicator(plant, GROWTH_INVALIDLOCATION)
		return

	status = getStatus(plant)

	# If this is just a bowl of dirt, there is a high chance of
	# loosing water
	if status == STATUS_DIRT:
		water = getWater(plant)

		# 90% chance of loosing water if there is more than 2 water units inside
		if water > 2 or random.random() < 0.90:
			setWater(plant, water - 1)
			plant.resendtooltip()

		return # No further growth checks for dirt

	# Apply beneficial stuff
	applyBoni(plant)

	# Apply malicious stuff
	if not applyMali(plant):
		return # The plant died

	# Grow one stage up
	growPlant(plant)

	# Set Mali for new growth round
	updateMali(plant)

	# Finally resend the tooltip (health changes i.e.)
	plant.resendtooltip()

#
# Reset the properties of the plant to the default
#
def resetPlant(plant, resetpotions = False, full = False):
	plant.deltag('growthindicator')

	if not full:
		setNextGrowth(plant, time.time() + GROWTHINTERVAL)		
	else:
		plant.deltag('nextgrowth')
		plant.deltag('water')
		plant.deltag('genus')
		plant.deltag('hue')
		plant.deltag('fertile')
		plant.deltag('seedhue')
		plant.deltag('seedgenus')
		plant.deltag('showtype')
		plant.deltag('status')

	plant.deltag('health')
	plant.deltag('infestation')
	plant.deltag('fungus')
	plant.deltag('poison')
	plant.deltag('disease')

	if resetpotions:
		plant.deltag('poisonpotion')
		plant.deltag('curepotion')
		plant.deltag('healpotion')
		plant.deltag('strengthpotion')

	plant.deltag('pollinated')
	plant.deltag('availableseeds')
	plant.deltag('remainingseeds')
	plant.deltag('availableresources')
	plant.deltag('remainingresources')

#
# Plant a seed in the bowl
#
def plantSeed(player, plant, seed):
	status = getStatus(plant)

	# FullGrown means that there is no real bowl left
	if status >= STATUS_FULLGROWN:
		plant.say(1061919, '', '', False, 0x3b2, player.socket) # You must use a seed on a bowl of dirt!

	# Can we even access the bowl?
	elif not checkAccess(player, plant):
		plant.say(1061856, '', '', False, 0x3b2, player.socket) # You must have the item in your backpack or locked down in order to use it.

	# We can only plant a seed into a bowl of dirt
	elif status != STATUS_DIRT:
		if status >= STATUS_PLANT:
			player.socket.showspeech(plant, tr("This bowl of dirt already has a plant in it!"))
		elif status >= STATUS_SAPLING:
			player.socket.showspeech(plant, tr("This bowl of dirt already has a sapling in it!"))
		else:
			player.socket.showspeech(plant, tr("This bowl of dirt already has a seed in it!"))

	# Not enough water
	elif getWater(plant) < 2:
		plant.say(1061920, '', '', False, 0x3b2, player.socket) # The dirt in this bowl needs to be softened first.

	# We can plant our seed here
	else:
		# Transfer seed properties
		setGenus(plant, plants.seed.getGenus(seed))
		setHue(plant, plants.seed.getHue(seed))
		setShowType(plant, plants.seed.getShowType(seed))

		seed.delete() # Remove the seed

		setStatus(plant, STATUS_SEED) # Set to the intial state		
		resetPlant(plant) # Start a new growth cycle

		plant.say(1061922, '', '', False, 0x3b2, player.socket) # You plant the seed in the bowl of dirt.

#
# Call this to update the state information of this plant
# if something changed.
#
def updatePlant(plant):
	status = getStatus(plant)
	genus = getGenus(plant)
	hue = getHue(plant)

	if status >= STATUS_DEAD:
		plant.name = '#1027069' # twigs
		plant.id = 0x1b9d
		plant.color = hue.color
	elif status >= STATUS_FULLGROWN:
		if status >= STATUS_DECORATIVE:
			plant.name = '#1061924' # a decorative plant
		else:
			plant.name = genus.name
		plant.id = genus.itemid
		plant.color = hue.color
		plant.movable = True
	elif status >= STATUS_PLANT:
		plant.id = 0x1600
		plant.color = 0
	else:
		plant.name = '#1029913' # plant bowl
		plant.id = 0x1602
		plant.color = 0

	plant.update()

#
# Show the plant gump (?)
#
def onUse(player, plant):
	if getStatus(plant) >= STATUS_DECORATIVE:
		return True # Decorative and dead plants don't respond to this

	if not checkAccess(player, plant):
		plant.say(1061856, '', '', False, 0x3b2, player.socket) # You must have the item in your backpack or locked down in order to use it.
	else:
		plants.maingump.send(player, plant)

	return True

#
# Get a cliloc id describing the waterlevel
#
def getWaterCliloc(plant):
	water = getWater(plant)
	if water <= 1:
		return 1060826
	else:
		return 1060827 + water - 2

#
# Get a cliloc for the plant status
#	
def getStatusCliloc(plant):
	status = getStatus(plant)

	if status >= STATUS_PLANT:
		return 1060812
	elif status >= STATUS_SAPLING:
		return 1023305
	else:
		return 1060810

#
# Display the tooltip
#
def onShowTooltip(player, plant, tooltip):
	status = getStatus(plant)
	genus = getGenus(plant)
	hue = getHue(plant)
	showtype = getShowType(plant)

	# Nothing to add for a decorative or dead plant (shouldn't have the event anyway)
	if status >= STATUS_DECORATIVE:
		return False

	# Full grown plants get additional info
	elif status >= STATUS_FULLGROWN:
		if genus.specialtitle:
			title = genus.specialtitle
		else:
			title = hue.name

		args = "#%u\t#%u\t%s" % (1060822 + getHealthStatus(plant), title, genus.name)

		if genus.plant:
			if hue.bright:
				tooltip.add(1061891, args) # a ~1_HEALTH~ bright ~2_COLOR~ ~3_NAME~
			else:
				tooltip.add(1061889, args) # a ~1_HEALTH~ ~2_COLOR~ ~3_NAME~
		else:
			if hue.bright:
				tooltip.add(1061892, args) # a ~1_HEALTH~ bright ~2_COLOR~ ~3_NAME~ plant
			else:
				tooltip.add(1061890, args) # a ~1_HEALTH~ ~2_COLOR~ ~3_NAME~ plant

	# Growing plants get other info
	elif status >= STATUS_SEED:
		if genus.specialtitle:
			title = genus.specialtitle
		else:
			title = hue.name

		args = "#%u\t#%u\t#%u" % (getWaterCliloc(plant), 1060822 + getHealthStatus(plant), title)

		if getShowType(plant):
			args += "\t" + genus.name

			if genus.plant and status == STATUS_PLANT:
				if hue.bright:
					tooltip.add(1060832, args) # a bowl of ~1_val~ dirt with a ~2_val~ bright ~3_val~ ~4_val~
				else:
					tooltip.add(1060831, args) # a bowl of ~1_val~ dirt with a ~2_val~ ~3_val~ ~4_val~
			else:
				args += "\t#%u" % getStatusCliloc(plant)

				if hue.bright:
					tooltip.add(1061887, args) # a bowl of ~1_val~ dirt with a ~2_val~ bright ~3_val~ ~4_val~ ~5_val~
				else:					
					tooltip.add(1061888, args) # a bowl of ~1_val~ dirt with a ~2_val~ ~3_val~ ~4_val~ ~5_val~
		else:
			args += "\t#%u" % getStatusCliloc(plant)

			if hue.bright:
				tooltip.add(1060832, args) # a bowl of ~1_val~ dirt with a ~2_val~ [bright] ~3_val~ ~4_val~
			else:					
				tooltip.add(1060831, args) # a bowl of ~1_val~ dirt with a ~2_val~ ~3_val~ ~4_val~

	# A bowl of dirt only gets the water level
	else:
		tooltip.add(1060830, '#%u' % getWaterCliloc(plant)) # a bowl of ~1_val~ dirt

	return False

#
# Register the plant with the global registry
#
def onAttach(obj):
	plants.register(obj)

#
# Unregister the plant with the global registry
#
def onDetach(obj):
	plants.unregister(obj)
