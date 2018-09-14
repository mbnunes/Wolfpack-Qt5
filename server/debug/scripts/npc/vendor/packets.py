
import wolfpack
from wolfpack.consts import *
import npc.vendor

#
# This class manages the list of items that the
# player can sell to the npc.
#
class SellList:
	# Indices for the items list
	ITEM_SERIAL = 0
	ITEM_ID = 1
	ITEM_COLOR = 2
	ITEM_AMOUNT = 3
	ITEM_PRICE = 4
	ITEM_DESCRIPTION = 5

	def __init__(self):
		# List of items for the sell list
		# serial, model, color, amount, price, name
		self.items = []
		
		# List of item objects whose tooltips need to be sent
		self.tooltips = []		
		
	#
	# Add an item to the sell list
	#
	def add(self, item, price, description = None):
		if description == None:
			if item.name == '':
				description = '%u' % (1020000 + item.id)
			else:
				description = item.name

		self.tooltips.append(item)
		item = (item.serial, item.id, item.color, item.amount, price, description.encode('utf-8'))
		self.items.append(item)		

	#
	# Return the number of items in this list
	#
	def count(self):
		return len(self.items)

	#
	# Build the sell list packet and return it
	#
	def buildSellPacket(self, vendor):
		# Compile the size of the packet
		packetsize = 9
		for item in self.items:
			packetsize += 15 + len(item[SellList.ITEM_DESCRIPTION])
			
		packet = wolfpack.packet(0x9e, packetsize)
		packet.setshort(1, packetsize)
		packet.setint(3, vendor.serial)
		packet.setshort(7, len(self.items))

		offset = 9
		
		# Store every item in the sell list
		for item in self.items:
			packet.setint(offset, item[SellList.ITEM_SERIAL])
			packet.setshort(offset + 4, item[SellList.ITEM_ID])
			packet.setshort(offset + 6, item[SellList.ITEM_COLOR])
			packet.setshort(offset + 8, item[SellList.ITEM_AMOUNT])
			packet.setshort(offset + 10, item[SellList.ITEM_PRICE])
			packet.setshort(offset + 12, len(item[SellList.ITEM_DESCRIPTION]) + 1) # Description length
			packet.setascii(offset + 14, item[SellList.ITEM_DESCRIPTION])
			offset += 15 + len(item[SellList.ITEM_DESCRIPTION])
		
		return packet

	#
	# Send the sell list for the given vendor to the player
	#
	def send(self, vendor, player):
		socket = player.socket
				
		packet = self.buildSellPacket(vendor)
		packet.send(socket)
		
		# Send the tooltips
		for item in self.tooltips:
			socket.sendobject(item)
			socket.sendtooltip(item)
		
		return True

#
# This class manages the packets to send a list of
# buyable items to the client
#
class BuyList:
	# Indices for the items list
	ITEM_CONTAINER = 0
	ITEM_SERIAL = 1
	ITEM_ID = 2
	ITEM_COLOR = 3
	ITEM_AMOUNT = 4
	ITEM_PRICE = 5
	ITEM_DESCRIPTION = 6

	def __init__(self):
		#
		# This list contains tuples with the following content:
		# (containerserial, dispitemserial, dispitemid, dispitemhue, availableamount, price, description)
		#
		self.items = []
		
		# This list contains the item objects whose tooltips have to be sent to the client.
		self.tooltips = []

	#
	# Add an item to the buylist
	#
	def add(self, container, displayitem, amount, price, description = None):
		if amount == 0 or len(self.items) >= 255:
			return # Dont add this item
		
		if description == None:
			if displayitem.name == '':
				description = '#%u' % (1020000 + displayitem.id)
			else:
				description = displayitem.name
				
		item = (container.serial, displayitem.serial, displayitem.id, displayitem.color, amount, price, description.encode('utf-8'))
		self.items.append(item)
		self.tooltips.append(displayitem)

	#
	# Create a packet with the vendors pack content
	#
	def createContentPacket(self, vendor):
		size = 5 + len(self.items) * 19
		packet = wolfpack.packet(0x3c, size)
		
		packet.setshort(1, size)
		packet.setshort(3, len(self.items)) # Set the amount of items in the list
		
		offset = 5
		
		# I've always hated their way of sending vendor info
		for i in range(len(self.items) - 1, -1, -1):
			item = self.items[i] # Get the current item

			packet.setint(offset, item[BuyList.ITEM_SERIAL])
			packet.setshort(offset + 4, item[BuyList.ITEM_ID])
			packet.setbyte(offset + 6, 0) # ? -> offset
			packet.setshort(offset + 7, item[BuyList.ITEM_AMOUNT])
			packet.setshort(offset + 9, i + 1) # Starting at 1 instead of 0
			packet.setshort(offset + 11, 1) # y offset
			packet.setint(offset + 13, item[BuyList.ITEM_CONTAINER])
			packet.setshort(offset + 17, item[BuyList.ITEM_COLOR])
			
			offset += 19 # Increase offset
		
		return packet

	#
	# Build the packet with the price information about items
	#
	def createBuyInfoPacket(self, vendor):
		size = 8
		
		for item in self.items:
			size += 6 + len(item[BuyList.ITEM_DESCRIPTION])
		
		# Get the restock container serial from the vendor
		restock = vendor.itemonlayer(LAYER_NPCRESTOCK)
				
		packet = wolfpack.packet(0x74, size)
		packet.setshort(1, size)
		
		if restock:
			packet.setint(3, restock.serial)
		else:
			packet.setint(3, -1)
			
		packet.setbyte(7, len(self.items)) # Set length of list
		
		offset = 8
		
		for item in self.items:
			packet.setint(offset, item[BuyList.ITEM_PRICE]) # Price
			packet.setbyte(offset + 4, len(item[BuyList.ITEM_DESCRIPTION]) + 1) # Length of Description
			packet.setascii(offset + 5, item[BuyList.ITEM_DESCRIPTION]) # Copy the string into the packet
			offset += 5 + len(item[BuyList.ITEM_DESCRIPTION]) + 1
			
		return packet
	
	#
	# Create the show gump packet
	#
	def createShowGumpPacket(self, vendor):
		packet = wolfpack.packet(0x24, 7)
		packet.setint(1, vendor.serial)
		packet.setshort(5, 0x30) # Seems like a constant
		return packet

	#
	# Send the buy list to a given player
	#
	def send(self, vendor, player):		
		if len(self.items) == 0:
			return False
		
		socket = player.socket # Cache the object
		
		# Make sure the packs are known to the player
		# If this fails, the client will CRASH!
		socket.sendobject(vendor.itemonlayer(LAYER_NPCRESTOCK))
		socket.sendobject(vendor.itemonlayer(LAYER_NPCNORESTOCK))
		socket.sendobject(vendor.itemonlayer(LAYER_NPCSELL))

		self.items.sort(lambda x,y : cmp(x[1], y[1])) # Sort the itemlist by serials first

		self.createContentPacket(vendor).send(socket) # Send the content packet
		self.createBuyInfoPacket(vendor).send(socket) # Send the buy info packet
		self.createShowGumpPacket(vendor).send(socket) # Show the buy gump
		socket.resendstatus() # Resend the status (Gold)
		
		# Send the item tooltips to the client
		for item in self.tooltips:
			socket.sendtooltip(item)
		
		return True

#
# Close the buy gump for the given vendor
#
def closeBuyGump(vendor, player):
	packet = wolfpack.packet(0x3b, 8)
	packet.setshort(1, 8)
	packet.setint(3, vendor.serial)
	packet.setbyte(4, 0)
	packet.send(player.socket)

# Handle an incoming buy packet
def incomingBuyPacket(socket, packet):
	player = socket.player
	size = packet.getshort(1)
	
	if size < 8:
		return True # Bogous packet
	
	vendor = wolfpack.findchar(packet.getint(3)) # Find the vendor
	
	if not vendor or vendor == player:
		closeBuyGump(vendor, player)
		return True # Invalid buy packet
	
	flag = packet.getbyte(7) # Flag if items have been bought or not (0 = no, 2 = yes)
	
	if flag == 0:
		closeBuyGump(vendor, player)
		return True # No Items bought
	
	amount = (size - 8) / 7 # Amount of items
	offset = 8
	
	bought = [] # List of tuples (item, amount)
	
	for i in range(0, amount):
		item = wolfpack.finditem(packet.getint(offset + 1))
		amount = packet.getshort(offset + 5)
		offset += 7
			
		# Check if the item really is a vendable item	
		if item and item.container and item.container.container == vendor and item.container.layer in [LAYER_NPCRESTOCK, LAYER_NPCNORESTOCK]:
			bought.append( (item, amount) )
			
	# We collected the list of bought items here.
	# Forward it to the vendor.
	vclass = npc.vendor.getClassFromVendor(vendor)
	
	if vclass and vclass.onBuyItems(vendor, player, bought):
		closeBuyGump(vendor, player)

	return True
	
# Handle an incoming sell packet
def incomingSellPacket(socket, packet):
	player = socket.player
	size = packet.getshort(1)
	
	if size < 9:
		return True # Bogous packet
	
	vendor = wolfpack.findchar(packet.getint(3)) # Find the vendor
	
	if not vendor or vendor == player:
		closeBuyGump(vendor, player)
		return True # Invalid sell packet
	
	count = packet.getshort(7) # Number of items following
	
	if count == 0:
		closeBuyGump(vendor, player)
		return True # No Items sold
	
	# Validate packet size
	if size != 9 + count * 6:
		socket.sysmessage('You sent an invalid vendor sell packet.')
		return True # invalid packet

	offset = 9	
	sold = [] # List of tuples (item, amount)
	
	for i in range(0, count):
		item = wolfpack.finditem(packet.getint(offset))
		amount = packet.getshort(offset + 4)
		offset += 6
			
		# Check if the item can really be sold by the player
		if item and item.getoutmostchar() == player:
			sold.append( (item, amount) )
			
	# We collected the list of sold items here.
	# Forward it to the vendor.
	vclass = npc.vendor.getClassFromVendor(vendor)
	
	if vclass and vclass.onSellItems(vendor, player, sold):
		closeBuyGump(vendor, player)

	return True

def onLoad():
	wolfpack.registerpackethook(0x3b, incomingBuyPacket)
	wolfpack.registerpackethook(0x9f, incomingSellPacket)
	
def onUnload():
	pass
