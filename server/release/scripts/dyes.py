
import wolfpack
import leatherdye
import specialdye
import runedye

#
# Check the dyes
#
def checkdyes(char, item, wearout = 0):
	if not item:
		return 0

	# Has to be in our posession
	if item.getoutmostchar() != char:
		char.socket.clilocmessage(500364)
		return 0

	# We do not allow "invulnerable" tools.
	if not item.hastag('remaining_uses'):
		char.socket.clilocmessage(500858)
		item.delete()
		return 0

	if wearout:
		uses = int(item.gettag('remaining_uses'))
		uses -= 1
		if uses <= 0:
			char.socket.clilocmessage(500858)
			item.delete()
			return 0
		else:
			item.settag('remaining_uses', uses)
			item.resendtooltip()

	return 1

#
# Process the target response from the client
#
def target(player, arguments, target):
	# Needs to in our belongings
	if target.item.getoutmostchar() != player:
		player.socket.clilocmessage(500364)
		return
	
	# Check the target
	if not target.item or target.item.baseid not in ['fab', 'leatherdye', 'specialdye', 'runedye']:
		player.socket.clilocmessage(500857)
		return

	# Wear out the tools
	dyes = wolfpack.finditem(arguments[0])
	if not checkdyes(player, dyes):
		return

	checkdyes(player, dyes, 1) # Wear out

	if target.item.baseid == 'leatherdye':
		leatherdye.pickHue(player, target.item) # Use special dye gump
		return
	elif target.item.baseid == 'specialdye':
		specialdye.pickHue(player, target.item) # Use special dye gump
		return
	elif target.item.baseid == 'runedye':
		runedye.pickHue(player, target.item) # Use special dye gump
		return

	# Send the dye dialog
	packet = wolfpack.packet(0x95, 9)
	packet.setint(1, target.item.serial)
	packet.setshort(7, 0xfab)
	packet.send(player.socket)

#
# Process the client response
#
def dyeresponse(socket, packet):
	serial = packet.getint(1)
	color = packet.getshort(7)

	# See if the color is invalid.
	if color < 2 or color > 0x3e9:
		socket.sysmessage("You selected an invalid color.")
		return 1
		
	# See if the serial is pointing to a valid object.
	item = wolfpack.finditem(serial)
	
	# Check if it's a dyetub and if its in our belongings
	if not item or item.baseid != 'fab' or item.getoutmostchar() != socket.player:
		socket.clilocmessage(500857)
		return 1
	
	item.color = color
	item.update()
	socket.player.soundeffect(0x023e)
	return 1

#
# Register our packet handler for the
# dye packet.
#
def onLoad():
	wolfpack.registerpackethook(0x95, dyeresponse)

#
# Show the hue selection gump.
#
def onUse(player, item):
	player.socket.clilocmessage(500856) # Select the dye tub to use the dyes on
	player.socket.attachtarget('dyes.target', [item.serial])
	return True

#
# Add the remaining uses to the tooltip
#
def onShowTooltip(viewer, object, tooltip):
	if object.hastag('remaining_uses'):
		tooltip.add(1060584, str(object.gettag('remaining_uses')))
