
import wolfpack

#
# Send a map command
#
def sendmapcommand(socket, item, command, plotting=0, x=0, y=0):
	packet = wolfpack.packet(0x56, 11)
	packet.setint(1, item.serial)
	packet.setbyte(5, command)
	if plotting:
		packet.setbyte(6, 1)
	packet.setshort(7, x)
	packet.setshort(9, y)
	packet.send(socket)

#
# Handle incoming map packets
#
def mappackets(socket, packet):
	player = socket.player
	
	# Get the basic properties
	item = wolfpack.finditem(packet.getint(1))
	protected = item.hastag('protected')
	editable = item.hastag('editable')
	
	# Retrieve the pinlist
	pins = []
	if item.hastag('pins'):
		pins = item.gettag('pins').strip().split(';')
		#for pin in pinsstrs:
		#	parts = pin.split(',')
		#	pins.append(pin)		
	
	if not item or item.getoutmostchar() != player:
		socket.clilocmessage(500685)
	else:
		# Process map command
		command = packet.getbyte(5)

		# Append to list of pins
		if command == 1:
			if not protected and editable and len(pins) < 256:
				pin = "%i,%i" % (packet.getshort(7), packet.getshort(9))
				pins.append(pin)
				item.settag('pins', ';'.join(pins))
		# Insert into list of pins
		elif command == 2:
			if not protected and editable and len(pins) < 256:
				pin = "%i,%i" % (packet.getshort(7), packet.getshort(9))
				index = packet.getbyte(6)
				pins.insert(index, pin)
				item.settag('pins', ';'.join(pins))
		# Change pin
		elif command == 3:
			if not protected and editable:
				pin = "%i,%i" % (packet.getshort(7), packet.getshort(9))
				index = packet.getbyte(6)
				if index < len(pins):
					pins[index] = pin
				item.settag('pins', ';'.join(pins))
		# Remove pin
		elif command == 4:
			if not protected and editable:
				pin = "%i,%i" % (packet.getshort(7), packet.getshort(9))
				index = packet.getbyte(6)
				if index < len(pins):
					del pins[index]
				item.settag('pins', ';'.join(pins))
		# Clear Pins
		elif command == 5:
			if not protected and editable:
				item.deltag('pins')
		# Toggle Plotting Course
		elif command == 6:
			if not protected:
				if editable:
					item.deltag('editable')
				else:
					item.settag('editable', 1)
			sendmapcommand(socket, item, 7, plotting = not editable)
		else:
			socket.sysmessage('Unknown Map Command %u.' % command)

	return 1

#
# Register the packet hook for map commands
#
def onLoad():
	wolfpack.registerpackethook(0x56, mappackets)

#
# Send a map to the client
#
def sendmap(player, item, maptype):
	xtop = 0
	ytop = 0
	xbottom = 768 * 8
	ybottom = 512 * 8
	width = 500
	height = 330
	
	# Send a map detail packet
	details = wolfpack.packet(0x90, 19)
	details.setint(1, item.serial)
	details.setshort(5, 0x139d)
	details.setshort(7, xtop) # Upper Left X
	details.setshort(9, ytop) # Upper Left Y
	details.setshort(11, xbottom) # Lower Right X
	details.setshort(13, ybottom) # Lower Right Y
	details.setshort(15, width) # Gump Width
	details.setshort(17, height) # Gump Height
	details.send(player.socket)
	
	# Remove all pins
	sendmapcommand(player.socket, item, 5)
	
	# Send all pins anew
	pins = []
	if item.hastag('pins'):
		pins = item.gettag('pins').strip().split(';')
		
	for pin in pins:
		(x, y) = pin.split(',')
		sendmapcommand(player.socket, item, 1, x=int(x), y=int(y))
	
	protected = item.hastag('protected')
	editable = item.hastag('editable')
	sendmapcommand(player.socket, item, 7, not protected and editable)

#
# Use a map
#
def onUse(player, item):
	# Has to belong to us.
	if item.getoutmostchar() != player:
		player.socket.clilocmessage(500685)
		return 1
	
	maptype = ''
	if item.hastag('type'):
		maptype = unicode(item.gettag('type'))
		
	if maptype != '':
		sendmap(player, item, maptype)
	else:
		player.socket.clilocmessage(500208)
	
	return 1
