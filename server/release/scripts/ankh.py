
#
# This script is attached to all ankhs and the ankh
# context menu.
#

import wolfpack
from wolfpack.gumps import cGump
from wolfpack import tr
from wolfpack.consts import MAX_TITHING_POINTS

def lockKarma(player, ankh):
	if not player.karmalock and player.canreach(ankh, 2):
		player.karmalock = True
		player.socket.clilocmessage(1060192)

def unlockKarma(player, ankh):
	if player.karmalock and player.canreach(ankh, 2):
		player.karmalock = False
		player.socket.clilocmessage(1060191)

def showTitheGump(player, ankh, amount = 0):
	if not ankh or not player.canreach(ankh, 2):
		return
	
	dialog = cGump(x=100, y=100)
	
	# Count Gold in Backpack
	backpack = player.getbackpack()
	gold = backpack.countitems(['eed'])
	
	# Normalize the amount of gold to tithe
	amount = max(0, min(gold, amount))
	
	# Gump Header
	dialog.startPage(0)
	dialog.addGump(30, 30, 102)
	dialog.addXmfHtmlGump(95, 100, 120, 100, 1060198, False, False)
	
	# Remaining Gold
	dialog.addText(57, 274, tr("Gold:"))
	dialog.addText(87, 274, str(gold - amount), 53)

	# Current Offering
	dialog.addText(137, 274, tr("Tithe:"))
	dialog.addText(172, 274, str(amount), 53)

	# Decrease Offering
	dialog.addButton(105, 230, 5220, 5220, 2)
	dialog.addButton(113, 230, 5222, 5222, 2)
	dialog.addText(108, 228, "<")
	dialog.addText(112, 228, "<")
	
	dialog.addButton(127, 230, 5223, 5223, 1)
	dialog.addText(131, 228, "<")

	# Increase Offering
	dialog.addButton(147, 230, 5224, 5224, 3)
	dialog.addText(153, 228, ">")

	dialog.addButton(168, 230, 5220, 5220, 4)
	dialog.addButton(176, 230, 5222, 5222, 4)
	dialog.addText(172, 228, ">")
	dialog.addText(176, 228, ">")

	dialog.addButton(217, 272, 4023, 4024, 5) # Make Offering

	dialog.setArgs([amount, ankh.serial])
	dialog.setCallback('ankh.titheGoldCallback')
	dialog.send(player)

def titheGoldCallback(player, arguments, response):
	(amount, ankh) = (arguments[0], wolfpack.finditem(arguments[1]))
	
	# Count Gold in Backpack
	backpack = player.getbackpack()
	gold = backpack.countitems(['eed'])

	if response.button == 0:
		player.message(1060193)
	elif response.button == 1:
		showTitheGump(player, ankh, amount - 100) # Decrease by 100
	elif response.button == 2:
		showTitheGump(player, ankh, 0) # Set to 0
	elif response.button == 3:
		showTitheGump(player, ankh, amount + 100) # Increase by 100
	elif response.button == 4:
		showTitheGump(player, ankh, gold) # Set to Maximum
	elif response.button == 5:
		amount = max(0, min(gold, amount)) # Normalize the amount of gold to tithe		

		# Get the tithing points of the user
		tithing_points = player.gettag('tithing_points')
		if not tithing_points:
			tithing_points = 0
		
		# If we would hit the maximum, only consume the remaining points
		if tithing_points + amount > MAX_TITHING_POINTS:
			amount = MAX_TITHING_POINTS - tithing_points

		if amount == 0:
			player.message(1060193) # If we don't tithe any gold, cancel
		elif backpack.removeitems(['eed'], amount) == 0: # Otherwise reduce the gold
			player.soundeffect(0x2e6)
			player.soundeffect(0x243)
			player.message(1060195) # You tithe lalala gold
			player.settag('tithing_points', tithing_points + amount)
			player.socket.resendstatus() # Resend the amount of gold
		else:
			player.message(1060194) # Not enough gold

def titheGold(player, ankh):
	if not player.dead and player.canreach(ankh, 2):
		showTitheGump(player, ankh, 0)
		return True
	return False

def resurrectPlayer(player, ankh):
	if not player.dead:
		return False
		
	if not player.canreach(ankh, 2):
		player.socket.clilocmessage(500446)
		return False
		
	if not player.pos.validspawnspot():
		player.socket.clilocmessage(502391)
		return False

	player.resurrect()
	return True

#
# Context menu events
#
def onContextCheckVisible(player, ankh, tag):
	if tag == 1:
		return not player.karmalock # We can lock the karma if its unlocked
	elif tag == 2:
		return player.karmalock # We can unlock the karma only if its locked
	else:
		return True

def onContextCheckEnabled(player, ankh, tag):
	if tag == 3:
		return player.dead # Only dead players can be resurrected
	else:
		return True

def onContextEntry(player, ankh, entry):
	if entry == 1:
		lockKarma(player, ankh)
	elif entry == 2:
		unlockKarma(player, ankh)
	elif entry == 3:
		resurrectPlayer(player, ankh)
	elif entry == 4:
		titheGold(player, ankh)
