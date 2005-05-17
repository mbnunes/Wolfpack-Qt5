
#
# This script is used for the player vendor himself
#

import wolfpack
from wolfpack.consts import *
from wolfpack import tr
import system.input
import buygump
import statusgump
import payment
import time

# Reduce bank account every 8 hours
PAYMENT_INTERVAL = (8.0 * 60.0 * 60.0)

#
# Open the backpack to the doubleclicking player if doubleclicked.
#
def onShowPaperdoll(vendor, player):
	if vendor.owner == player:
		statusgump.send(vendor, player)
	else:
		showInventory(vendor, player)
	return True

#
# Show the wares of the vendor to a player
#
def showInventory(vendor, player):
	if not player.gm:
		if vendor.owner == player:
			vendor.say(1010642, "", "", False, 0x3b2, player.socket)
		else:
			vendor.say(503208, "", "", False, 0x3b2, player.socket)
	player.socket.sendcontainer(vendor.getbackpack())

#
# Give the given item to the player
#
def giveToPlayer(player, item):
	removeSellInfo(item)
	removeChildrenSellInfo(item)
	
	# Remove all Sell info from the item and its content
	player.getbackpack().additem(item, True, True, False)
	item.update()

#
# Remove the sellinfo from this item
# 
def removeSellInfo(item):
	item.deltag('pv_price')
	item.deltag('pv_description')
	item.removescript('npc.playervendor.item')
	item.resendtooltip()

#
# Remove the sellinfo tag from the children of this item
#
def removeChildrenSellInfo(item):
	for child in item.content:
		removeSellInfo(child)
		removeChildrenSellInfo(child)

#
# Set the sell info for the item
#
def setSellInfo(item, price, description):	
	bounce = False
	vendor = item.getoutmostchar()
	player = vendor.owner

	if not vendor or not player:
		return

	# Check if the item may be made not for sale
	if price == 0:
		# Container
		if item.type == 1:
			# Has to be unlocked
			if item.hasscript('lock') and item.hastag('locked'):
				vendor.say(1043298, "", "", False, 0x3b2, player.socket)
				bounce = True
			# Has to be empty
			elif len(item.content) > 0:
				vendor.say(1043299, "", "", False, 0x3b2, player.socket)
				bounce = True

		# Only books and keyrings (??) may be not for sale
		elif not item.hasscript('book'):
			vendor.say(1043301, "", "", False, 0x3b2, player.socket)
			bounce = True
			
		# Is the item allowed to be not for sale?
		if not bounce:
			item.settag('pv_price', -1)
			item.settag('pv_description', description)
			item.resendtooltip()
			
		# Bounce the item
		elif bounce:
			giveToPlayer(player, item)
	else:
		item.settag('pv_price', price)
		item.settag('pv_description', description)
		item.resendtooltip()
		
		# Remove the info from all contained items.
		removeChildrenSellInfo(item)

#
# Dont allow snooping in the player vendors belongings
#
def onSnooping(vendor, item, player):
	return True

#
# Items within the backpack may be used
#
def onRemoteUse(player, item):
	# Has to be in the inventory
	if item.getoutmostitem().layer != LAYER_BACKPACK:
		return False
	
	# Only items that are not for sale may be used (books)
	if item.hastag('pv_price') and item.gettag('pv_price') == -1:
		return True
	
	return False

#
# An item has been given to the playervendor by a player
#
def giveItem(vendor, player, item):
	# Ask for a price and description
	item.settag('pv_price', -1) # Free!
	item.settag('pv_description', '') # No special description
	if not item.hasscript('npc.playervendor.item'):
		item.addscript('npc.playervendor.item') # Event for all items within the player vendors stock
	item.resendtooltip()

	# Ask for a price / and description	
	player.socket.clilocmessage(1043303, unicode(item.getname()))
	system.input.request(player, item, 1) # Price request

#
# Get the amount of gold in the bank for this vendor
#
def getBankGold(vendor):
	if not vendor.hastag('bankaccount'):
		return 0
	else:
		amount = vendor.gettag('bankaccount')
		if amount < 0:
			return 0
		else:
			return amount

#
# Increase the bank account gold
#
def giveBankGold(vendor, amount):
	gold = getBankGold(vendor) + amount
	if gold <= 0:
		vendor.deltag('bankaccount')
	else:
		vendor.settag('bankaccount', gold)
		
#
# Get the gold in the hold
#
def getHoldGold(vendor):
	if not vendor.hastag('holdaccount'):
		return 0
	else:
		amount = vendor.gettag('holdaccount')
		if amount < 0:
			return 0
		else:
			return amount
	
#
# Give gold to the hold
#
def giveHoldGold(vendor, amount):
	gold = getHoldGold(vendor) + amount
	if gold <= 0:
		vendor.deltag('holdaccount')
	else:
		vendor.settag('holdaccount', gold)

#
# Dismiss this vendor
#
def dismiss(vendor, player):
	# The backpack has to be empty before dismissing this vendor
	if len(vendor.getbackpack().content) > 0:
		vendor.say(503229, "", "", False, 0x3b2, player.socket) # The backpack has to be empty blabla
		return False

	giveGold(vendor, player) # Put the money the vendor is holding into the bank account of the player
	
	# Put the rest of the money into the players backpack
	amount = getHoldGold(vendor)	
	if amount != 0:
		check = wolfpack.additem( "bank_check" )
		check.settag('value', amount)
		player.getbackpack().additem(check, True, True, False)
		check.update()
		
	vendor.delete() # Delete the vendor	
	
#
# An item has been dropped on the player vendor
#
def onDropOnChar(vendor, item):
	player = item.container
	
	if not player:
		return False
	
	if player != vendor.owner:
		vendor.say(503209, "", "", False, 0x3b2, player.socket) # I can only take items from the owner
		return False
		
	# If it's gold. Take it into the bank account.
	if item.id == 0xeed:
		giveBankGold(vendor, item.amount)
		item.delete()
		vendor.say(503210, "", "", False, 0x3b2, player.socket)		
		return True

	giveItem(vendor, player, item)
	vendor.getbackpack().additem(item, True, True, False) # Random pos, no stacking
	item.update()
	return True

#
# Give the gold the vendor is holding to the player.
#
def giveGold(vendor, player):
	amount = min(1000000, getHoldGold(vendor)) # Max. 1.000.000 gold
	
	if amount <= 0:
		vendor.say(503215, "", "", False, 0x3b2, player.socket)
	else:
		if amount >= 5000:
			check = wolfpack.additem( "bank_check" )
			check.settag('value', amount)
			player.getbankbox().additem(check, True, True, False)
			check.update()
		else:
			gold = wolfpack.additem("eed")
			gold.amount = amount
			if not wolfpack.utilities.tocontainer(gold, player.getbankbox()):
				gold.update()

		giveHoldGold(vendor, -amount)
		vendor.say(503234, "", "", False, 0x3b2, player.socket) # All the gold I have been carrying for you has been deposited into your bank account.

#
# Textinput to the vendor
#
def onSpeech(npc, player, text, keywords):
	if player.distanceto(npc) > 3:
		return False

	text = text.lower()
	vendorname = npc.name.lower()
	
	# vendor buy or <name> buy
	if 0x3c in keywords or (0x171 in keywords and text.startswith(vendorname)):
		if npc.owner == player:
			npc.say(503212, "", "", False, 0x3b2, player.socket)
		else:
			npc.say(503213, "", "", False, 0x3b2, player.socket)
			player.socket.attachtarget('npc.playervendor.buytarget', [])
		return True
		
	# vendor browse or <name> browse
	elif 0x3d in keywords or (0x172 in keywords and text.startswith(vendorname)):
		showInventory(npc, player)
		return True		
		
	# vendor collect or <name> collect
	elif 0x3e in keywords or (0x173 in keywords and text.startswith(vendorname)):
		if npc.owner == player:
			giveGold(npc, player)
			return True
			
	# vendor status or <name> status
	elif 0x3f in keywords or (0x174 in keywords and text.startswith(vendorname)):
		if npc.owner == player:
			statusgump.send(npc, player)
		else:
			npc.say(503226, "", "", False, 0x3b2, player.socket) # What do you care.. You dont run this shop
		return True

	# vendor dismiss or <name> dismiss
	elif 0x40 in keywords or (0x175 in keywords and text.startswith(vendorname)):
		if npc.owner == player:
			dismiss(npc, player)
			return True

	# vendor cycle, <name> cycle
	elif 0x41 in keywords or (0x176 in keywords and text.startswith(vendorname)):
		if npc.owner == player:
			npc.turnto(player)
			return True

	return False

#
# player vendor buy response
#
def buytarget(player, arguments, target):
	item = target.item
	
	if not item:
		player.socket.clilocmessage(503216) # Cannot buy that
		return
		
	vendor = item.getoutmostchar()
		
	if not vendor or not vendor.hasscript('npc.playervendor'):
		player.socket.clilocmessage(503216) # Cannot buy that
		return
		
	if not player.canreach(vendor, 3):
		player.socket.clilocmessage(500312) # You cannot reach that
		return
		
	if not item.hasscript('npc.playervendor.item') or not item.hastag('pv_price') or item.gettag('pv_price') <= 0:
		vendor.say(503202, "", "", False, 0x3b2, player.socket) # Not for sale
		return
		
	# Show sell gump
	buygump.send(player, vendor, item)

#
# Get the sell value recursively
#
def getSellValue(item):
	result = 0
	
	if item.hasscript('npc.playervendor.item'):
		price = item.gettag('pv_price')
		
		if price and price > 0:
			result += price
			
	if item.type == 1:
		for child in item.content:
			result += getSellValue(child)

	return result

#
# Get the daily rent for the vendor
#
def getRent(vendor):
	rent = 20	
	rent += getSellValue(vendor.getbackpack()) / 500	
	return rent

#
# Vendor registration
#
def onAttach(vendor):
	payment.registerVendor(vendor)

#
# Vendor deregistration
#	
def onDetach(vendor):
	payment.unregisterVendor(vendor)
	
#
# Vendor deregistration
#
def onDelete(vendor):
	payment.unregisterVendor(vendor)

#
# Set next payment tag
#
def setLastPayment(vendor):
	vendor.settag('last_payment', time.time())

#
# Vendor has run out of money. Destroy it.
#
def destroyVendor(vendor):
	vendor.say(tr('I regret nothing!'))
	vendor.invulnerable = False
	vendor.kill()

#
# Check payment of this vendor
#
def checkPayment(vendor):
	global PAYMENT_INTERVAL

	if not vendor.hastag('last_payment'):
		setLastPayment(vendor) # Start payed for one day

	last_payment = float(vendor.gettag('last_payment'))

	# Is the last payment day in the future?
	if last_payment > time.time():
		setLastPayment(vendor) # Reset this if it's bugged.

	# Has one day elapsed already?
	if last_payment + PAYMENT_INTERVAL > time.time():		
		return

	setLastPayment(vendor) # Refresh payment tag

	# Consume money
	rent = getRent(vendor)
	bank = getBankGold(vendor)
	hold = getHoldGold(vendor)
	
	# Kill vendor
	if bank + hold < rent:
		destroyVendor(vendor)
		return

	# We have enough money in the bank
	if rent <= bank:
		giveBankGold(vendor, - rent)
		return
	else:
		giveBankGold(vendor, - bank)
		rent -= bank

	giveHoldGold(vendor, - rent)
