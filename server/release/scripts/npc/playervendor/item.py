
#
# This script is used for items vended by the player vendor.
#

from wolfpack.consts import *
from wolfpack import tr, callevent, hasevent
from npc.playervendor import setSellInfo, removeSellInfo, removeChildrenSellInfo
import book
from npc import playervendor

#
# Show containers and books
#
def onUse(player, item):
	if item.type == 1:
		player.socket.sendcontainer(item)
	elif item.hasscript('book'):
		book.onUse(player, item)

	return True

#
# Property setting has been canceled
#
def onTextInputCancel(player, item, inputid):
	# Check if the vendor the item is in is owned by us
	vendor = item.getoutmostchar()

	if not vendor or not vendor.hasscript('npc.playervendor'):
		return

	if not player.gm and vendor.owner != player:
		player.socket.sysmessage(tr('You cannot set the properties of items in a player vendor not owned by you.'))
		return

	setSellInfo(item, 0, '')

#
# Set properties of the vended item
#
def onTextInput(player, item, inputid, text):
	# Check if the vendor the item is in is owned by us
	vendor = item.getoutmostchar()

	if not vendor or not vendor.hasscript('npc.playervendor'):
		return

	if not player.gm and vendor.owner != player:
		player.socket.sysmessage(tr('You cannot set the properties of items in a player vendor not owned by you.'))
		return

	parts = text.split(' ', 1)
	description = ''

	try:
		price = int(parts[0])
	except:
		price = 0

	if price < 0:
		price = 0

	if len(parts) == 2:
		description = parts[1]
	else:
		description = ''

	setSellInfo(item, price, description)

#
# Show extra tooltip information for vended items
#
def onShowTooltip(viewer, item, tooltip):
	## Call the other tooltip scripts first
	scripts = item.scripts + item.basescripts.split(',') # Build the scriptlist

	for script in scripts:
		if len(script) == 0 or script == 'npc.playervendor.item': # Ignore us
			continue

		if hasevent(script, EVENT_SHOWTOOLTIP):
			result = callevent(script, EVENT_SHOWTOOLTIP, (viewer, item, tooltip))

			if result:
				break

	if not item.hastag('pv_price'):
		price = -1
	else:
		price = item.gettag('pv_price')

	if price == -1:
		tooltip.add(1043307, "") # Not for sale
	elif price == 0:
		tooltip.add(1043306, "") # Free!
	else:
		tooltip.add(1043304, str(price)) # Price...

	if item.hastag('pv_description'):
		description = item.gettag('pv_description').strip()
		if len(description) != 0:
			tooltip.add(1043305, description) # Description

	return True

#
# Another item is dropped onto this item. Take care of prizing.
#
def onDropOnItem(container, item):
	outmost = container.getoutmostchar()

	# We're not being dropped onto the belongings of another playervendor.
	# Remove all vending information
	if not outmost.hasscript('npc.playervendor'):
		removeSellInfo(item) # Remove scripts and tags
		removeChildrenSellInfo(item)
		return False

	# If the container is not the backpack itself...
	if container.container and container.container.ischar():
		return False

	return playervendor.backpack.onDropOnItem(container.getoutmostitem(), item)
