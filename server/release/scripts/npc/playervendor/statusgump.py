
#
# This file shows a status gump to the player.
#

OWNER_TYPE = 0x73fbda10

from wolfpack import tr
import wolfpack.gumps
import npc.playervendor
import customizegump

def close(player):
	player.socket.closegump(OWNER_TYPE)

def send(vendor, player):
	close(player)
	customizegump.close(player)
	
	gump = wolfpack.gumps.cGump(x=50, y=200)

	gump.startPage(0)
	gump.addResizeGump(25, 10, 5054, 530, 140)
	
	gump.addXmfHtmlGump(425, 25, 120, 20, 1019068, False, False) # See Goods
	gump.addXmfHtmlGump(425, 48, 120, 20, 1019069, False, False) # Customize
	gump.addXmfHtmlGump(425, 72, 120, 20, 1011012, False, False) # Cancel
	
	gump.addButton(390, 25, 4005, 4007, 1) # See Goods
	gump.addButton(390, 48, 4005, 4007, 2) # Customize
	gump.addButton(390, 72, 4005, 4007, 0) # Cancel

	hold = npc.playervendor.getHoldGold(vendor)
	bank = npc.playervendor.getBankGold(vendor)
	rent = npc.playervendor.getRent(vendor)
	
	gump.addXmfHtmlGump(40, 72, 260, 20, 1038321, False, False) # Hold Gold
	gump.addXmfHtmlGump(40, 96, 260, 20, 1038322, False, False) # Bank Gold
	gump.addText(300, 72, str(hold), 0) # Gold in hold
	gump.addText(300, 96, str(bank), 0) # Gold in bank
	
	gump.addHtmlGump(40, 120, 260, 20, tr("My charge per day is:"), False, False)
	gump.addText(200, 120, str(rent), 0)
	
	remaining = (hold + bank) / rent
	
	interval = npc.playervendor.PAYMENT_INTERVAL # This value is the length of an ingame payment day in realtime seconds	
		
	gump.addXmfHtmlGump(40, 25, 260, 20, 1038318, False, False) # how many days the vendor will work. This is ingame days. (sucks)
	gump.addXmfHtmlGump(40, 48, 260, 20, 1038319, False, False) # earth days. (one uo day: 8 hours, meaning: / 3)
	
	gump.addText(300, 25, str(remaining))
	gump.addText(300, 48, str(int(remaining * interval / 86400)))
	
	gump.setType(OWNER_TYPE)
	gump.setCallback(callback)
	gump.setArgs([vendor.serial])
	gump.send(player)

#
# Owner gump response
#
def callback(player, arguments, response):
	if response.button == 0:
		return
		
	vendor = wolfpack.findchar(arguments[0])
	
	if not vendor:
		return # Vendor went out of scope
	
	# Send inventory
	if response.button == 1:
		npc.playervendor.showInventory(vendor, player)
	elif response.button == 2:
		pass
