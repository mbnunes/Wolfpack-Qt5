
import random
from wolfpack.consts import *
from wolfpack import console
import wolfpack
from buyitem import GenericBuyItem
from packets import BuyList

#
# Configuration values for this script
#
SPEECH_RANGE = 4 # How many tiles away can the player be
CHECK_LOS = True # Should LOS be checked for vending actions?
RESTOCK_INTERVAL = 30 * 60 * 1000 # 30 minutes default

VENDOR_CLASSES = {} # A registry for vendor classes

magic = random.random() # Generate a random magic that is used to identify the last inventory check

#
# Register a vendor class
#
def registerVendorClass(name, obj):
	VENDOR_CLASSES[name] = obj
	
#
# Unregister a vendor class
#
def unregisterVendorClass(name, obj):
	if name in VENDOR_CLASSES:
		del VENDOR_CLASSES[name]

#
# Get an instance of the vendor class for a given vendor
#
def getClassFromVendor(vendor):
	if not vendor:
		return None
		
	if not vendor.npc or not vendor.hasscript('npc.vendor'):
		return None
		
	if vendor.hastag('vendor_class'):
		vendor_class = str(vendor.gettag('vendor_class'))
		if vendor_class in VENDOR_CLASSES:
			return VENDOR_CLASSES[vendor_class]
			
	vendor_class = vendor.getstrproperty('vendor_class', '')
	
	if vendor_class in VENDOR_CLASSES:
		return VENDOR_CLASSES[vendor_class]
		
	return None

#
# Handle speech for the vendor
#
def onSpeech(vendor, speaker, text, keywords):
	vclass = getClassFromVendor(vendor)

	if vclass:
		return vclass.onSpeech(vendor, speaker, text, keywords)

	return False

#
# Base class implementing vendor functionality
#
class Vendor:
	def __init__(self):
		self.speechrange = SPEECH_RANGE
		self.checklos = CHECK_LOS
		self.buylist = [] # List of buy definitions for this vendor
		self.selllist = [] # List of sell definitions for this vendor

		# Just for testing
		self.buylist.append(GenericBuyItem('efa', 100, 10))
		buyitem = GenericBuyItem('913', 2100, 300)
		buyitem.controlslots = 1
		self.buylist.append(buyitem)

	#
	# Checks if the player can access this vendor
	# Does LOS/Dead/Distance and other checks
	#
	def checkAccess(self, vendor, player, checkdead = True):
		if player.gm:
			return True
		
		if checkdead and player.dead:
			return False
		
		# Check the distance between the speaker and the vendor
		if self.speechrange > 0:
			if self.checklos:
				if not player.canreach(vendor, self.speechrange):
					return False # Can't see him or out of range
			else:
				if player.distanceto(vendor) > self.speechrange:
					return False # Out of range

		return True

	#
	# Scale the buyprice for an item
	#
	def scaleBuyPrice(self, vendor, player, buyitem, price):
		return price

	#
	# Scale the sellprice for an item
	#
	def scaleSellPrice(self, vendor, player, sellitem, price):
		return price

	#
	# Show the sell gump to a given player
	#
	def onSell(self, vendor, player):
		vendor.say('showing selllist')
	
	#
	# Get the restock container for the given vendor
	#
	def getRestockContainer(self, vendor, dontcreate = False):
		cont = vendor.itemonlayer(LAYER_NPCRESTOCK)
		
		# Create it, if it does not exist
		if not cont and not dontcreate:
			cont = wolfpack.additem('e75')
			cont.owner = vendor
			vendor.additem(LAYER_NPCRESTOCK, cont)
			cont.update()

		return cont
		
	def getBuyContainer(self, vendor, dontcreate = False):
		cont = vendor.itemonlayer(LAYER_NPCNORESTOCK)
		
		# Create it, if it does not exist
		if not cont and not dontcreate:
			cont = wolfpack.additem('e75')
			cont.owner = vendor
			vendor.additem(LAYER_NPCNORESTOCK, cont)
			cont.update()

		return cont		
		
	def getSellContainer(self, vendor, dontcreate = False):
		cont = vendor.itemonlayer(LAYER_NPCSELL)
		
		# Create it, if it does not exist
		if not cont and not dontcreate:
			cont = wolfpack.additem('e75')
			cont.owner = vendor
			vendor.additem(LAYER_NPCSELL, cont)
			cont.update()

		return cont
	
	#
	# Ensure all three packs are there
	#
	def ensurePacks(self, vendor):
		self.getSellContainer(vendor)
		self.getRestockContainer(vendor)
		self.getBuyContainer(vendor)

	#
	# Check if the restock containers has all the required buyitems for this 
	# vendor.
	#
	def checkRestockInventory(self, vendor, force = False):
		# If the inventory check is not forced,
		# see if the last check was performed more than an hour ago
		if not force:
			vendor_magic = vendor.gettag('magic')
			if vendor_magic == magic:
				return

		restock = vendor.itemonlayer(LAYER_NPCRESTOCK)
		if restock:
			restock.delete() # Delete the old restock container

		restock = self.getRestockContainer(vendor) # Create a new restock container

		# Recreate the content from our buyitems array
		for i in range(0, len(self.buylist)):
			dispitem = self.buylist[i].createDisplayItem() # Try to create a display item
			
			# Unable to create display item
			if not dispitem:
				console.log(LOG_ERROR, "Unable to create display item for buyitem at index %u for vendor 0x%x.\n" % (i, vendor.serial))
				continue
				
			dispitem.settag('buyitemindex', i) # Store the index of the buyitem that created the dispitem
			restock.additem(dispitem) # Add the dispitem to the restock container

		# Once the check has been completed, save the vendor magic
		vendor.settag('magic', magic)

	#
	# Check Restocking of this vendor
	#
	def checkRestock(self, vendor):
		# Get the last restock time
		last_restock = vendor.gettag('lastrestock')

		# If there was no restock yet, set the time for the next one
		if not last_restock or last_restock > wolfpack.currenttime():
			vendor.settag('lastrestock', wolfpack.currenttime())

		# Check if the last restock is long enough in the past to justify a new restock
		elif last_restock + RESTOCK_INTERVAL < wolfpack.currenttime():
			self.doRestock(vendor)
			vendor.settag('lastrestock', wolfpack.currenttime())

	#
	# Restock the vendors inventory
	#
	def doRestock(self, vendor):
		restock = self.getRestockContainer(vendor, True) # Don't create a new one if there is none
		
		if not restock:
			return # Nothing to restock
			
		for item in restock.content:
			# Get the current restock amount
			instock = self.getInStock(item)
			maxstock = item.amount

			# If the item was sold out, increase the maximum amount by a factor of two
			if instock <= 0:
				maxstock = min(item.getintproperty('max_restock_amount', 999), maxstock * 2)
				
			# If we still have more than half in stock, decrease the max stock
			elif instock >= maxstock / 2:
				maxstock = max(item.getintproperty('min_restock_amount', 10), maxstock / 2)
				
			item.amount = maxstock
			item.settag('instock', maxstock) # Restock to the maximum amount

	#
	# Get the available amount from a given item
	#
	def getInStock(self, item):
		if not item.container:
			return 0

		if item.container.layer != LAYER_NPCRESTOCK:
			return item.amount
			
		else:
			instock = item.gettag('instock')
			if instock == None:
				return item.amount
			else:
				return instock

	#
	# Show the buy gump to a given player
	#
	def onBuy(self, vendor, player):
		self.checkRestockInventory(vendor) # Perform an inventory check
		self.checkRestock(vendor) # Check if the restock time has been reached

		if not self.canBuyItems(vendor, player):
			return

		if self.sendBuyList(vendor, player): # Send the list of buy items
			self.onSendBuyList(vendor, player)
			
	#
	# Show a nice hello message
	#
	def onSendBuyList(self, vendor, player):
		vendor.say(500186, "", "", False, vendor.saycolor, player.socket)

	#
	# Check if the given player can buy from this vendor
	#
	def canBuyItems(self, vendor, player):
		return True

	#
	# Send the buylist of this vendor to the given player
	#
	def sendBuyList(self, vendor, player):
		if not self.canBuyItems(vendor, player):
			return False

		if not self.checkAccess(vendor, player):
			return False
			
		self.ensurePacks(vendor) # Make sure the packs are there

		# Compile a list of items that should be sold
		buylist = BuyList()
		
		# Add items that are in the restock container		
		restock = self.getRestockContainer(vendor)
		for item in restock.content:
			buyitemindex = item.gettag('buyitemindex')
			
			if buyitemindex == None or buyitemindex < 0 or buyitemindex >= len(self.buylist):
				console.log(LOG_ERROR, "Item 0x%x has invalid buy item index %s.\n" % (item.serial, buyitemindex))
				continue
			
			buyitem = self.buylist[buyitemindex]
			
			# Scale the price
			price = self.scaleBuyPrice(vendor, player, buyitem, buyitem.getPrice(vendor, player))
			
			# Add the item to the buylist
			buylist.add(item, self.getInStock(item), price)
			
		# Send the buylist to the player
		return buylist.send(vendor, player)

	#
	# The player has said something strange to this vendor
	#
	def onUnknownSpeech(self, vendor, player, text, keywords):
		vendor.say('Huh?')
		
		vus = 0
		if player.socket.hastag('vendor_unknown_speech'):
			vus = player.socket.gettag('vendor_unknown_speech')
			
		# After speaking garbage three times, the attention will be removed
		if vus < 3:
			player.socket.settag('vendor_unknown_speech', vus + 1)
		else:
			player.socket.deltag('attention')

	#
	# Process speech input in the presence of this vendor
	#
	def onSpeech(self, vendor, player, text, keywords = []):
		text = text.lower()
		
		# Does the player have this vendors attention?
		attention = 'vendor' in text or vendor.name.lower() in text
		
		# Set the attention of the player to this npc if he talked to him
		if attention:
			if player.socket:
				player.socket.settag('attention', vendor.serial)

		# If we don't have the attention because of the text, check if the
		# player has the attention of this npc at the moment
		else:
			if player.socket:
				attention = player.socket.gettag('attention') == vendor.serial

		if not attention:
			return False
				
		# Note that we only check access to the vendor after the player has the attention
		if not self.checkAccess(vendor, player):
			return False
			
		vendor.turnto(player)

		# Process the speech input by the player.
		if 60 in keywords:
			self.onBuy(vendor, player)
		elif 333 in keywords:
			self.onSell(vendor, player)
		else:
			self.onUnknownSpeech(vendor, player, text, keywords)

		return False

	#
	# No items were bought
	#
	def onNoItemsBought(self, vendor, player, errors = False):
		if not errors:
			vendor.say(500190, "", "", False, vendor.saycolor, player.socket) # Thou hast bought nothing!
		else:
			vendor.say(500187, "", "", False, vendor.saycolor, player.socket) # Your order cannot be fulfilled, please try again.

	#
	# This event gets triggered when the player tries to buy items
	# from the vendor
	#
	def onBuyItems(self, vendor, player, bought):
		if not self.canBuyItems(vendor, player):
			return False

		if not self.checkAccess(vendor, player):
			return False

		validitems = [] # A list with 3-tuples containing the dispitem, buyitem and amount to be bought
		totalprice = 0 # Total purchase
		controlslots = player.maxcontrolslots - player.controlslots # Amount of controlslots left
		
		# Process the buy item list
		for (item, amount) in bought:
			# Check the amount of the bought item
			instock = self.getInStock(item)
			amount = min(instock, amount)
			
			if amount == 0:
				continue # The item isn't available at all
			
			# Get the buyitem
			buyitemindex = item.gettag('buyitemindex')
			
			if buyitemindex == None or buyitemindex < 0 or buyitemindex >= len(self.buylist):
				console.log(LOG_ERROR, "Item 0x%x has invalid buy item index %s.\n" % (item.serial, buyitemindex))
				continue
			
			buyitem = self.buylist[buyitemindex]
							
			# Check for the price and the controlslots
			if buyitem.controlslots * amount > controlslots:
				continue # Cannot buy this. Too few control slots.
				
			controlslots -= buyitem.controlslots * amount # Reduce remaining control slots
				
			# Scale the price
			totalprice += self.scaleBuyPrice(vendor, player, buyitem, buyitem.getPrice(vendor, player)) * amount
			
			validitems.append( (item, buyitem, amount) )
		
		if len(bought) == 0:
			self.onNoItemsBought(vendor, player)
			return False # Don't close the buygump
		elif len(validitems) == 0:
			self.onNoItemsBought(vendor, player, True)
			return False # Don't close the buygump

		player.message('You tried to buy %u items for %u gold.' % (len(validitems), totalprice))

		# Check for enough money
		if player.gm:
			remaining = 0
		else:
			remaining = totalprice
			
		if remaining > 0:		
			backpack = player.getbackpack()
			
		if remaining > 0:		
			bankbox = player.getbankbox()

		return True # Close the buygump

VENDOR_CLASSES['base'] = Vendor()
