
import wolfpack
import wolfpack.utilities

#
# Base Class for SellItems
#
class SellItem:
	def __init__(self):
		self.resell = True # Don't delete on buy
		pass

	#
	# Get the price for this item/vendor/player
	#
	def getPrice(self, vendor, player, item):
		return 0

	#
	# Check if the given item can be sold to the vendor.
	#
	def canBeSold(self, vendor, player, item):
		return False
		
	#
	# This function makes the given item go into the players
	# pack. Respect the amount!
	#
	def createItems(self, vendor, player, item, amount):
		if amount <= 0:
			return # Unable to comply
			
		backpack = player.getbackpack()

		# We're buying less than we have
		if amount < item.amount:
			bought = item.dupe()
			bought.amount = amount
			item.amount -= amount
		else:
			bought = item
			
		# Remove the vendor-tag from the item
		bought.deltag('vendor_bought')
		
		if not wolfpack.utilities.tobackpack(bought, player):			
			bought.update()

#
# GenericSellItem class
# for selling normal items
#
class GenericSellItem(SellItem):
	#
	# Baseids is a baseid or list or tuple of baseids for items
	# that should be accepted.
	# price is the price it's bought for.
	#
	def __init__(self, baseid, price):
		SellItem.__init__(self)
		if type(baseid) in (str, unicode):
			self.baseids = (baseid,)
		else:
			self.baseids = tuple(baseid)
		self.price = price

	#
	# Get the price for this item/vendor/player
	#
	def getPrice(self, vendor, player, item):
		return self.price

	#
	# Check if the given item can be bought by the vendor
	# Please note that the check for the baseid is done by the
	# vendor code.
	#
	def canBeSold(self, vendor, player, item):
		# Newbie items cannot be sold
		if item.newbie:
			return False
		
		# Invisible items cannot be sold
		if not item.visible:
			return False
		
		# Immovable items cannot be sold	
		if item.movable > 1 or not player.canpickup(item):
			return False			
		
		return True
