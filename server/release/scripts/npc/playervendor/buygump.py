
import wolfpack
import wolfpack.gumps
from wolfpack.consts import *
from wolfpack import tr
import npc.playervendor

#
# This file shows a sell gump to the player
#
def send(player, vendor, item):
	gump = wolfpack.gumps.cGump(x=100, y=200)
	
	gump.startPage(0)
	gump.addResizeGump(100, 10, 5054, 300, 150)

	gump.addXmfHtmlGump(125, 20, 250, 24, 1019070, False, False) # You have agreed to purchase:
	
	description = ''
	if item.hastag('pv_description'):
		description = item.gettag('pv_description').strip()
		
	if len(description) != 0:
		gump.addText(125, 45, description, 0)
	else:
		gump.addXmfHtmlGump(125, 45, 250, 24, 1019072, False, False)
		
	gump.addXmfHtmlGump(125, 70, 250, 24, 1019071, False, False)
	gump.addText(125, 95, str(item.gettag('pv_price')), 0)
	
	gump.addButton(250, 130, 4005, 4007, 0) # Cancel
	gump.addXmfHtmlGump(282, 130, 100, 24, 1011012, False, False)
	
	gump.addButton(120, 130, 4005, 4007, 1) # Ok
	gump.addXmfHtmlGump(152, 130, 100, 24, 1011036, False, False)
	
	gump.setCallback(response)
	gump.setArgs([item.serial, item.amount, int(item.gettag('pv_price'))])
	gump.send(player)

#
# Buy response
#
def response(player, arguments, response):
	if response.button == 0:
		return
		
	item = wolfpack.finditem(arguments[0])
	oldamount = arguments[1]
	oldprice = arguments[2]
	
	if not item:
		return
		
	outmost = item.getoutmostchar()
	
	# Check if the item is still available for the same price in the vendor.
	if not outmost or not outmost.hasscript('npc.playervendor'):
		player.socket.sysmessage(tr('The item you tried to buy is no longer available for sale.'))
		return
	
	if not player.canreach(outmost, 3):
		player.socket.clilocmessage(500312)
		return
		
	price = int(item.gettag('pv_price')) # Get item price
	
	# Same amount? Still vending? Same price?
	if oldamount != item.amount or not item.hasscript('npc.playervendor.item') or oldprice != price:
		outmost.say(503216, "", "", False, 0x3b2, player.socket)
		return	

	# Check if the player has enough gold
	pack_gold = player.getbackpack().countresource(0xEED, 0x0)
	bank_gold = player.getbankbox().countresource(0xEED, 0x0)
	
	if pack_gold + bank_gold < price: 
		outmost.say(503205, "", "", False, 0x3b2, player.socket) # You can't afford this
		return	
		
	# Consume from pack_gold first
	price = player.getbackpack().useresource(price, 0xEED, 0x0)
	
	if price > 0:
		player.getbankbox().useresource(price, 0xEED, 0x0)
	
	npc.playervendor.giveToPlayer(player, item)
	npc.playervendor.giveHoldGold(outmost, oldprice)
	player.soundeffect(0x37, 0)
