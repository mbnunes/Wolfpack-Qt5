
#
# This script is used for the player vendors backpack.
#

from npc import playervendor

#
# Show the content of the playervendor pack to the player
# using it.
#
def onUse(player, backpack):
	vendor = backpack.container
	playervendor.showInventory(backpack.container, player)
	return True

#
# Ask for a price for items dropped into the backpack
#
def onDropOnItem(container, item):
	vendor = container.container
	player = item.container

	if not vendor or not player or not player.ischar() or (not player.gm and vendor.owner != player):
		return False # Cancel

	# It already has vending information
	if item.hasscript('npc.playervendor.item'):
		return False

	playervendor.giveItem(vendor, player, item)
	return False
