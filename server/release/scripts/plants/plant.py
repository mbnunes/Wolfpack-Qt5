
import wolfpack
import plants.maingump
import plants.genuses
import plants.hues
from consts import *
import beverage
import potions.utilities
import potionkeg

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
		return plants.hues.getById(int(plant.gettag('hues')))
def setHue(plant, hue):
	if type(hue) == int:
		plant.settag('hue', hue)
	else:
		plant.settag('hue', hue.id)		

#
# Manage the Status of the plant
#
def getStatus(plant):
	if plant.hastag('status'):
		return int(plant.gettag('status'))
	else:
		return STATUS_DIRT
def setStatus(plant, status):
	plant.settag('status', int(status))
	
#
# Manage the fertile indicator for a plant
#
def getFertile(plant):
	return plant.gettag('fertile') == 1
def setFertile(plant, fertile):
	plant.settag('fertile', int(fertile))

#
# Lets this plant die
#
def die(plant):
	pass

#
# Checks if the player can access the plant
#
def checkAccess(player, plant):
	# It either has to be in the backpack
	if plant.getoutmostitem() == player.getbackpack():
		return True
		
	# Or locked down (the multi will do the other check)
	if plant.multi:
		return True
		
	return False

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
# Display the tooltip
#
def onShowTooltip(player, plant, tooltip):
	return False
