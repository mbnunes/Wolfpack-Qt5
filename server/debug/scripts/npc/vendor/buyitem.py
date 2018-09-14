
import wolfpack
import wolfpack.utilities

#
# Base Class for BuyItems
#
class BuyItem:
	def __init__(self):
		self.controlslots = 0 # How many control slots this buy item required to be bought
		pass

	#
	# Get the price for this item/vendor/player
	#
	def getPrice(self, vendor, player):
		pass

	#
	# Check if this item can be bought by the given player
	#
	def canBeBought(self, vendor, player):
		return True

	#
	# Create the corresponding item with the given amount for
	# the given player
	#
	def createItems(self, vendor, player, item, amount = 1):
		pass

	#
	# Create a display item for this buyitem and return it
	#
	def createDisplayItem(self):
		pass

#
# GenericBuyItem class
# for buying normal items
#
class GenericBuyItem(BuyItem):
	#
	# Baseid is the id of the item that should be created
	# price is the price it's sold at
	# amount is the amount of items that should be sold
	#
	def __init__(self, baseid, price, amount = 20):
		BuyItem.__init__(self)
		self.amount = amount
		self.baseid = baseid
		self.price = price

	#
	# Get the price for this item/vendor/player
	#
	def getPrice(self, vendor, player):
		return self.price

	#
	# Create the corresponding item with the given amount for
	# the given player. Item is the display item assigned
	# to this buyitem instance.
	#
	def createItems(self, vendor, player, item, amount = 1):
		if amount <= 0:
			return # Unable to comply
			
		backpack = player.getbackpack()
		
		# We will at the very least require one item, 
		# so create that one here.
		bought = wolfpack.additem(self.baseid)
		
		stackable = bought.canstack(bought) # Check if bought could stack with bought
		
		if stackable:
			bought.amount = amount
			if not wolfpack.utilities.tobackpack(bought, player):
				bought.update()
		elif not stackable:
			if not wolfpack.utilities.tobackpack(bought, player):
				bought.update()

			# Create amount - 1 items
			for i in range(0, amount - 1):
				bought = wolfpack.additem(self.baseid)
				if not wolfpack.utilities.tobackpack(bought, player):
					bought.update()

	#
	# Create a display item for this buyitem and return it
	#
	def createDisplayItem(self):
		item = wolfpack.additem(self.baseid)
		item.amount = self.amount
		item.movable = 3 # Not movable... (just to make sure noone steals it by whatever means)
		return item
