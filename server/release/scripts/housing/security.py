
import wolfpack.gumps
from housing.house import checkAccess
from housing.consts import *

#
# Security settings can only be changed by the house owner
#
def onContextCheckVisible(player, item, tag):
	return checkAccess(player, item.multi, ACCESS_OWNER)

#
# Show the security setting gump
#
def onContextEntry(player, item, entry):
	if entry != 67:
		return False

	if not checkAccess(player, item.multi, ACCESS_OWNER):
		return False

	showGump(player, item)
	return True

#
# Get the security level for the given item
# 0 Owner Only
# 1 Co Owners
# 2 Friends
# 3 Anyone
#
def getLevel(item):
	if item.hastag('security_level'):
		return int(item.gettag('security_level'))
	return 0

#
# Show the gump for a given securable item
#
def showGump(player, item):
	if not item.multi:
		return
	owner = item.multi.owner
	level = getLevel(item)		
	
	dialog = wolfpack.gumps.cGump()
	
	dialog.startPage(0)
	dialog.addBackground(5054, 220, 160)

	dialog.addTiledGump( 10, 10, 200, 20, 5124 )
	dialog.addTiledGump( 10, 40, 200, 20, 5124 )
	dialog.addTiledGump( 10, 70, 200, 80, 5124 )
		
	dialog.addCheckerTrans( 10, 10, 200, 140 )
	
	dialog.addXmfHtmlGump( 10, 10, 200, 20, 1061276, False, False, 32767 )
	dialog.addXmfHtmlGump( 10, 40, 100, 20, 1041474, False, False, 32767 )

	if not owner:
		dialog.addText( 110, 40, "", 1152 )
	else:
		dialog.addText( 110, 40, owner.name, 1152 )
				
	if level == 0:
		dialog.addButton( 10, 70, 4006, 4007, 1 )
		dialog.addXmfHtmlGump( 45, 70, 150, 20, 1061277, False, False, 0x7F18 )
	else:
		dialog.addButton( 10, 70, 4005, 4007, 1 )
		dialog.addXmfHtmlGump( 45, 70, 150, 20, 1061277, False, False, 0x7FFF )	
		
	if level == 1:
		dialog.addButton( 10, 90, 4006, 4007, 2 )
		dialog.addXmfHtmlGump( 45, 90, 150, 20, 1061278, False, False, 0x7F18 )
	else:
		dialog.addButton( 10, 90, 4005, 4007, 2 )
		dialog.addXmfHtmlGump( 45, 90, 150, 20, 1061278, False, False, 0x7FFF )	
	
	if level == 2:
		dialog.addButton( 10, 110, 4006, 4007, 3 )
		dialog.addXmfHtmlGump( 45, 110, 150, 20, 1061279, False, False, 0x7F18 )
	else:
		dialog.addButton( 10, 110, 4005, 4007, 3 )
		dialog.addXmfHtmlGump( 45, 110, 150, 20, 1061279, False, False, 0x7FFF )	
	
	if level == 3:
		dialog.addButton( 10, 130, 4006, 4007, 4 )
		dialog.addXmfHtmlGump( 45, 130, 150, 20, 1061626, False, False, 0x7F18 )
	else:
		dialog.addButton( 10, 130, 4005, 4007, 4 )
		dialog.addXmfHtmlGump( 45, 130, 150, 20, 1061626, False, False, 0x7FFF )	
		
	dialog.setArgs([item.serial])
	dialog.setCallback("housing.security.callback")
	dialog.send(player)

#
# Gump callback
#
def callback(player, arguments, response):
	if response.button <= 0 or response.button >= 5:
		return
	
	item = wolfpack.finditem(arguments[0])
	if item and item.hasscript('housing.security'):
		level = response.button - 1
		current_level = getLevel(item)
		
		if level == current_level:
			player.socket.clilocmessage(1061281)
		else:				
			if level == 0:
				item.deltag('security_level')
			else:
				item.settag('security_level', level)
			player.socket.clilocmessage(1061280)
