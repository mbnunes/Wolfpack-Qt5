
import wolfpack

#
# Table for preset maps
#
MAP_PRESETS = {
	# Width, Height, xtop, ytop, xbottom, ybottom
	'britain': [200, 200, 1092, 1396, 1736, 1924], # map of Britain
	'britaintoskarabrae': [200, 200, 256, 1792, 1736, 2560], # map of Britain to Skara Brae
	'britaintotrinsic': [200, 200, 1024, 1280, 2304, 3072], # map of Britain to Trinsic
	'bucsden': [200, 200, 2500, 1900, 3000, 2400], # map of Buccaneer's Den
	'bucsdentomagincia': [200, 200, 2560, 1792, 3840, 2560], # map of Buccaneer's Den to Magincia
	'bucsdentoocllo': [200, 200, 2560, 1792, 3840, 3072], # map of Buccaneer's Den to Ocllo
	'jhelom': [200, 200, 1088, 3572, 1528, 4056], # map of Jhelom
	'magincia': [200, 200, 3530, 2022, 3818, 2298], # map of Magincia
	'maginciatoocllo': [200, 200, 3328, 1792, 3840, 2304], # map of Magincia to Ocllo
	'minoc': [200, 200, 2360, 356, 2706, 702], # map of Minoc
	'minoctoyew': [200, 200, 0, 256, 2304, 3072], # map of Minoc to Yew
	'minoctovesper': [200, 200, 2467, 572, 2878, 746], # map of Minoc to Vesper
	'moonglow': [200, 200, 4156, 808, 4732, 1528], # map of Moonglow
	'moonglowtonujelm': [200, 200, 3328, 768, 4864, 1536], # map of Moonglow to Nujelm
	'nujelm': [200, 200, 3446, 1030, 3832, 1424], # map of Nujelm
	'nujelmtomagincia': [200, 200, 3328, 1024, 3840, 2304], # map of Nujelm to Magincia
	'occlo': [200, 200, 3582, 2456, 3770, 2742], # map of Ocllo
	'serpentshold': [200, 200, 2714, 3329, 3100, 3639], # map of Serpent's Hold
	'serpentsholdtoocllo': [200, 200, 2560, 2560, 3840, 3840], # map of Serpent's Hold to Ocllo
	'skarabrae': [200, 200, 524, 2064, 960, 2452], # map of Skara Brae
	'world': [200, 200, 0, 0, 5199, 4095], # map of The World
	'trinsic': [200, 200, 1792, 2630, 2118, 2952], # map of Trinsic
	'trinsictobucsden': [200, 200, 1792, 1792, 3072, 3072], # map of Trinsic to Buccaneer's Den
	'trinsictojhelom': [200, 200, 256, 1792, 2304, 4095], # map of Trinsic to Jhelom
	'vesper': [200, 200, 2636, 592, 3064, 1012], # map of Vesper
	'vespertonujelm': [200, 200, 2636, 592, 3840, 1536], # map of Vesper to Nujelm
	'yew': [200, 200, 236, 741, 766, 1269], # map of Yew
	'yewtobritain': [200, 200, 0, 512, 1792, 2048], # map of Yew to Britain
}

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
	if maptype == 'preset' and item.hastag('preset'):
		preset = item.gettag('preset')
		if not MAP_PRESETS.has_key(preset):
			player.socket.sysmessage('Unknown map preset: %s.' % preset)
			return
		(width, height, xtop, ytop, xbottom, ybottom) = MAP_PRESETS[preset]
	elif maptype == 'world':
		(width, height, xtop, ytop, xbottom, ybottom) = (400, 400, 0, 0, 5119, 4095)
	elif maptype == 'custom':
		(width, height, xtop, ytop, xbottom, ybottom) = (200, 200, 0, 0, 5119, 4095)
		if item.hastag('width'):
			width = int(item.gettag('width'))
		if item.hastag('height'):
			height = int(item.gettag('height'))			
		if item.hastag('xtop'):
			xtop = int(item.gettag('xtop'))
		if item.hastag('xbottom'):
			xbottom = int(item.gettag('xbottom'))
		if item.hastag('ytop'):
			ytop = int(item.gettag('ytop'))
		if item.hastag('ybottom'):
			ybottom = int(item.gettag('ybottom'))						
	else:
		player.socket.sysmessage('Unknown map type: %s.' % maptype)
		return
	
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
