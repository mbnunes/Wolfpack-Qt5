
# Python imports
import random
import time

# Wolfpack imports
import wolfpack
import wolfpack.time
from wolfpack.consts import *
from wolfpack import console, tr

# Vendor imports
from buyitem import GenericBuyItem
from sellitem import GenericSellItem
from packets import BuyList, SellList

#
# Configuration values for this script
#
SPEECH_RANGE = 4 # How many tiles away can the player be
CHECK_LOS = True # Should LOS be checked for vending actions?
RESTOCK_INTERVAL = 30 * 60 * 1000 # 30 minutes default
GOLDTOBANK = 500 # If we'd get more than "x" gold coins from a transaction. 
				 # Put it into our bank account as a check instead.
BACKPACKTRESHOLD = 2000 # If we purchase less than this amount, the money for a purchase will
						# only come from our backpack. Otherwise from backpack and/or bank.
MAXITEMSPERSELL = 100 # The number of items a player can sell per transaction
BOUGHTITEMDECAY = 60 * 60 	# The time in seconds a bought item decays after
							# default is 60 seconds.

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
		self.selllist.append(GenericSellItem('f9e', 127))
		self.selllist.append(GenericSellItem('e21', 18))
		self.selllist.append(GenericSellItem('e75', 100))

		self.sellmap = None # Generate on sell

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
	def scaleBuyPrice(self, vendor, player, item, price):
		return price
		
	#
	# Scale the price an item is being reselled for
	#
	def scaleResellPrice(self, vendor, player, item, price):
		return int(price * 1.9)

	#
	# Scale the sellprice for an item
	#
	def scaleSellPrice(self, vendor, player, item, price):
		return price

	#
	# Sell items decay after a given linger time in the vendor
	#
	def checkItemDecay(self, vendor):
		# Iterate trough all items in the norestock pack and see
		# if they should decay
		norestock = self.getBuyContainer(vendor)
		currenttime = int(time.time())
		todelete = [] # List of items that should be deleted
		
		for item in norestock.content:
			vendor_bought = item.gettag('vendor_bought')
			
			if vendor_bought == None:
				todelete.append(item) # Remove the item if it doesn't have a bought time

			elif vendor_bought > currenttime:
				item.settag('vendor_bought', currenttime) # Set the bought time to now if the item was bought in the future
				
			elif vendor_bought + BOUGHTITEMDECAY < currenttime:
				todelete.append(item)
				
		# Delete items if neccesary
		for item in todelete:
			item.delete()

	#
	# Check the cached sell mapping
	#
	def checkSellMap(self):
		if self.sellmap:
			return
			
		self.sellmap = {}
		
		for item in self.selllist:
			if not isinstance(item, GenericSellItem):
				continue
				
			for baseid in item.baseids:
				self.sellmap[baseid] = item

	#
	# Show the sell gump to a given player
	#
	def onSell(self, vendor, player):
		self.checkSellMap()
		
		if not self.canSellItems(vendor, player):
			return

		if self.sendSellList(vendor, player): # Send the list of sell items
			self.onSendSellList(vendor, player)

	#
	# Populate a sell list based on the sellmap of this vendor instance
	#
	def buildSellList(self, vendor, player, selllist, container):
		content = container.content
		baseid = container.baseid
		
		# Empty containers can be sold
		if len(content) == 0 and baseid in self.sellmap:
			sellitem = self.sellmap[baseid]
			# Check if the item can really be sold to the vendor
			if sellitem.canBeSold(vendor, player, container):
				price = self.scaleSellPrice(vendor, player, container, sellitem.getPrice(vendor, player, container))
				if price > 0:
					selllist.add(container, price)
					return
			
		# Process the content of the container
		for item in content:
			# Recurse into subcontainers
			if item.type == 1:			
				self.buildSellList(vendor, player, selllist, item) # Recurse into the container
				
			# Otherwise check if its a sellable item
			elif item.baseid in self.sellmap:
				sellitem = self.sellmap[item.baseid]
				# Check if the item can really be sold
				if sellitem.canBeSold(vendor, player, item):
					price = self.scaleSellPrice(vendor, player, item, sellitem.getPrice(vendor, player, item))
					if price > 0:
						selllist.add(item, price)

	#
	# Send the list of possible sell items to the player
	#
	def sendSellList(self, vendor, player):
		if not self.canSellItems(vendor, player):
			return False

		if not self.checkAccess(vendor, player):
			return False
			
		self.ensurePacks(vendor) # Make sure the packs are there

		# Compile a list of items that should be sold
		selllist = SellList()

		# Recursively iterate over the backpack of the player
		backpack = player.getbackpack()
		self.buildSellList(vendor, player, selllist, backpack)
		
		if selllist.count() == 0:
			self.onNothingToSell(vendor, player)
			return False

		# Send the buylist to the player
		return selllist.send(vendor, player)
		
	#
	# The player has nothing to sell to this vendor
	#
	def onNothingToSell(self, vendor, player):
		player.socket.showspeech(vendor, tr("You have nothing I would be interested in."), vendor.saycolor)

	#
	# Notification event if the sell list is shown
	#
	def onSendSellList(self, vendor, player):
		pass

	#
	# Check if the given player can sell items to this vendor
	#
	def canSellItems(self, vendor, player):
		return True
	
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
		if not last_restock or last_restock > wolfpack.time.currenttime():
			vendor.settag('lastrestock', wolfpack.time.currenttime())

		# Check if the last restock is long enough in the past to justify a new restock
		elif last_restock + RESTOCK_INTERVAL < wolfpack.time.currenttime():
			self.doRestock(vendor)
			vendor.settag('lastrestock', wolfpack.time.currenttime())

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
		self.checkItemDecay(vendor) # Check for decayed sell items
		self.checkSellMap()

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
		
		restock = self.getRestockContainer(vendor)	
		norestock = self.getBuyContainer(vendor)
				
		# Add items that are in the restock container
		for item in restock.content:
			buyitemindex = item.gettag('buyitemindex')
			
			if buyitemindex == None or buyitemindex < 0 or buyitemindex >= len(self.buylist):
				console.log(LOG_ERROR, "Item 0x%x has invalid buy item index %s.\n" % (item.serial, buyitemindex))
				continue
			
			buyitem = self.buylist[buyitemindex]
			
			# Scale the price
			price = self.scaleBuyPrice(vendor, player, buyitem, buyitem.getPrice(vendor, player))
			
			# Add the item to the buylist
			buylist.add(restock, item, self.getInStock(item), price)

		# Add items from the no restock container
		for item in norestock.content:
			baseid = item.baseid

			if not baseid in self.sellmap:
				continue # Should we delete this?

			sellitem = self.sellmap[baseid]

			price = self.scaleResellPrice(vendor, player, item, sellitem.getPrice(vendor, player, item))

			if price <= 0:
				continue # We don't donate...

			# Another quirk in the osi protocol. no restock items are still in the
			# restock pack, oherwise they don't have a price.
			buylist.add(restock, item, self.getInStock(item), price)

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
	# This is triggered if the player buys too many items
	#
	def onTooManyItemsSold(self, vendor, player):
		player.socket.sysmessage('You cannot sell more than %u items at a time.' % MAXITEMSPERSELL)

	#
	# This event gets triggered when the player tries to sell items
	# to the vendor
	#
	def onSellItems(self, vendor, player, sold):		
		if not self.canSellItems(vendor, player):
			return False

		if not self.checkAccess(vendor, player):
			return False
		
		# Limit this to MAXITEMSPERSELL for server performance
		if len(sold) > MAXITEMSPERSELL:
			self.onTooManyItemsSold(vendor, player)
			return False

		totalgold = 0
		count = 0
		boughtitems = self.getBuyContainer(vendor)

		for (item, amount) in sold:
			# Validate the amount
			amount = min(amount, item.amount)

			if amount <= 0:
				continue # We're not selling anything at all

			baseid = item.baseid
		
			# Check if the item is really sellable
			if not baseid in self.sellmap:
				continue
				
			sellitem = self.sellmap[baseid]
			
			if not sellitem.canBeSold(vendor, player, item):
				continue # Skip an unsellable item
				
			price = self.scaleSellPrice(vendor, player, item, sellitem.getPrice(vendor, player, item))

			if price <= 0:
				continue # The player would get nothing for it

			# Give the item to the vendor for reselling
			if sellitem.resell:
				item.removefromview() # Whatever happens here, the player wont see the item anymore
				item.settag('vendor_bought', int(time.time())) # Save the time the item was bought
				wolfpack.utilities.tocontainer(item, boughtitems) # Simply move to the norestock pack

			# Just consume the item instead
			else:
				if amount >= item.amount:
					item.delete()
				else:
					item.amount -= amount
					item.update()

			totalgold += price * amount
			count += 1

		# Award the player his money
		if totalgold > 0:
			player.soundeffect(0x37)
			if totalgold > GOLDTOBANK:
				# Create a check in his bankbox
				check = wolfpack.additem('bank_check')
				check.settag('value', totalgold)
				
				# Put the check into his bank box
				bankbox = player.getbankbox()
				if not wolfpack.utilities.tocontainer(check, bankbox):
					check.update()
				
				tobank = True
				
			else:
				# Simply dump the money in his backpack
				remaining = totalgold
				backpack = player.getbackpack()
				while remaining > 60000:
					gold = wolfpack.additem('eed')
					gold.amount = 60000
					if not wolfpack.utilities.tocontainer(gold, backpack):
						gold.update()
					remaining -= 60000
				if remaining > 0:
					gold = wolfpack.additem('eed')
					gold.amount = remaining
					if not wolfpack.utilities.tocontainer(gold, backpack):
						gold.update()
						
				tobank = False
			
			self.onItemsSold(vendor, player, count, totalgold, tobank)
			

		return True

	#
	# We sold <count> items to <player> for <totalgold> gold coins. 
	# And if <tobank> is true, we put it as a check to the players bank account.
	#
	def onItemsSold(self, vendor, player, count = 0, totalgold = 0, tobank = False):
		text = None
		
		if count > 0 and totalgold > 0:
			if tobank:
				text = tr('I bought %u items from you. I sent a check for %u gold to your bank account.' % (count, totalgold))
			else:
				text = tr('I bought %u items from you. Here are your %u gold.' % (count, totalgold))
		
		if text:
			player.socket.showspeech(vendor, text, vendor.saycolor)

	#
	# No items were bought
	#
	def onNoItemsBought(self, vendor, player, errors = False):
		if not errors:
			vendor.say(500190, "", "", False, vendor.saycolor, player.socket) # Thou hast bought nothing!
		else:
			vendor.say(500187, "", "", False, vendor.saycolor, player.socket) # Your order cannot be fulfilled, please try again.

	#
	# The player tried to buy something, but didn't have enough money
	# Bankbox indicates, that the bankbox was checked and didn't have enough money.
	#
	def onInsufficientFunds(self, vendor, player, bankbox = False):
		if not bankbox:
			vendor.say(500192, "", "", False, vendor.saycolor, player.socket) # Begging thy pardon, but thou canst not afford that.
		else:
			vendor.say(500191, "", "", False, vendor.saycolor, player.socket) # Begging thy pardon, but thy bank account lacks these funds.

	#
	# This event is triggered when the player successfully buys items.
	# The partial flag indicates, that not all of his requests could be fullfilled.
	# The bank flag indicates, that the amount was withdrawn from the bank account
	# of the player. Amount is the amount of gold that has been withdrawn.
	#
	def onItemsBought(self, vendor, player, amount = 0, frombank = False, partial = False):
		if not partial:
			if player.gm:
				player.socket.showspeech(vendor, tr("I would not presume to charge thee anything.  Here are the goods you requested."), vendor.saycolor)
			elif frombank:
				player.socket.showspeech(vendor, tr("The total of thy purchase is %u gold, which has been withdrawn from your bank account.  My thanks for the patronage.") % amount, vendor.saycolor)
			else:
				player.socket.showspeech(vendor, tr("The total of thy purchase is %u gold.  My thanks for the patronage.") % amount, vendor.saycolor)
		else:
			if player.gm:
				player.socket.showspeech(vendor, tr("I would not presume to charge thee anything.  Unfortunately, I could not sell you all the goods you requested."), vendor.saycolor)
			elif frombank:
				player.socket.showspeech(vendor, tr("The total of thy purchase is %u gold, which has been withdrawn from your bank account.  My thanks for the patronage.  Unfortunately, I could not sell you all the goods you requested.") % amount, vendor.saycolor)
			else:
				player.socket.showspeech(vendor, tr("The total of thy purchase is %u gold.  My thanks for the patronage.  Unfortunately, I could not sell you all the goods you requested.") % amount, vendor.saycolor)			

	#
	# This is a helper function, that recursively counts the gold 
	# within the given container, appends found golditems to the passed
	# array of golditems and returns the new total amount of gold.
	# Maxtotalgold is a cap to make sure that we're not counting more gold than
	# we need.
	#
	def countGoldInCont(self, container, golditems, totalgold = 0, maxtotalgold = -1):
		eedhash = hash('eed') # Cache the hash
		for item in container.content:
			if hash(item.baseid) == eedhash:
				totalgold += item.amount
				golditems.append(item)
				
				# Make sure we don't count too many items
				if maxtotalgold != -1 and totalgold >= maxtotalgold:
					break
			
			# Recurse into containers
			elif item.type == 1:
				totalgold += self.countGoldInCont(item, golditems, totalgold, maxtotalgold)
				
				# Make sure we don't count too many items
				if maxtotalgold != -1 and totalgold >= maxtotalgold:
					break

		return totalgold

	#
	# These constants are the return values for the consumeGold function.
	#
	CONSUME_NONE = 0
	CONSUME_BACKPACK = 1
	CONSUME_BANKBOX = 2

	#
	# Check and consume the given amount of gold
	# return True if the gold was successfully consumed.
	# False otherwise
	#
	def consumeGold(self, vendor, player, amount):
		if player.gm:
			return Vendor.CONSUME_BACKPACK # No cost for staff members
		
		golditems = [] # Instances of gold items found in the backpack and bankbox.
		totalfound = 0 # Total amount of gold found in the backpack and bankbox.
		
		backpack = player.getbackpack() # Get player backpack
		bankbox = player.getbankbox() # Get players bankbox
			
		# Check the backpack first
		totalfound += self.countGoldInCont(backpack, golditems, totalfound, amount)

		# If we purchased for less than BACKPACKTRESHOLD gold pieces, the bankbox wont be checked
		if totalfound < amount and amount < BACKPACKTRESHOLD:
			self.onInsufficientFunds(vendor, player, False)
			return Vendor.CONSUME_NONE
		elif totalfound >= amount:
			for item in golditems:
				# The item is sufficient to fullfill our needs
				if item.amount > amount:
					item.amount -= amount
					item.update()
					break
				
				# We need to consume more than this
				elif item.amount <= amount:
					amount -= item.amount
					item.delete()
			return Vendor.CONSUME_BACKPACK
			
		# We're purchasing for more than BACKPACKTRESHOLD gold and we weren't able to pay yet.
		# Get enough golditems from the bankaccount until our requirement is fullfilled
		totalfound += self.countGoldInCont(bankbox, golditems, totalfound, amount)

		if totalfound < amount:
			self.onInsufficientFunds(vendor, player, True)
			return Vendor.CONSUME_NONE

		# Delete enough gold we found to fullfill the demand
		for item in golditems:
			# The item is sufficient to fullfill our needs
			if item.amount > amount:
				item.amount -= amount
				item.update()
				break
			
			# We need to consume more than this
			elif item.amount <= amount:
				amount -= item.amount
				item.delete()

		return Vendor.CONSUME_BANKBOX

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
		partial = False # We assume we can buy everything here
		
		# Process the buy item list
		for (item, amount) in bought:
			# Check the amount of the bought item
			instock = self.getInStock(item)
			amount = min(instock, amount)
			
			if amount == 0:
				continue # The item isn't available at all
			
			# Get the buyitem/sellitem
			if item.container.layer == LAYER_NPCNORESTOCK:
				if item.baseid not in self.sellmap:
					continue # Invalid item
					
				sellitem = self.sellmap[item.baseid]
				
				price = self.scaleResellPrice(vendor, player, item, sellitem.getPrice(vendor, player, item)) * amount
	
				if price <= 0:
					continue # We don't donate...
					
				totalprice += price
				
				validitems.append( (item, sellitem, amount) )
			else:
				buyitemindex = item.gettag('buyitemindex')
				
				if buyitemindex == None or buyitemindex < 0 or buyitemindex >= len(self.buylist):
					console.log(LOG_ERROR, "Item 0x%x has invalid buy item index %s.\n" % (item.serial, buyitemindex))
					continue
				
				buyitem = self.buylist[buyitemindex]
	
				# Check for the price and the controlslots
				if buyitem.controlslots * amount > controlslots:
					amount = controlslots / buyitem.controlslots
					partial = True
					
					if amount == 0:
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

		# Check for enough money
		consume = self.consumeGold(vendor, player, totalprice)
		if consume == Vendor.CONSUME_NONE:
			return False # Dont close the buygump
			
		player.soundeffect(0x32) # Play a gold soundeffect

		# Create the bought items.
		for (item, buyitem, amount) in validitems:
			buyitem.createItems(vendor, player, item, amount)

		frombank = consume == Vendor.CONSUME_BANKBOX # Was the amount deducted from the bank?
		self.onItemsBought(vendor, player, totalprice, frombank, partial) # Notify the event handler

		return True # Close the buygump
