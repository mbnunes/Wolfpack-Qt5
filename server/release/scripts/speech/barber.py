#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Created by: DarkStorm                      #
#   )).-' {{ ;'`   | Revised by: Naddel                         #
#  ( (  ;._ \\ ctr | Last Modification: Created                 #
#===============================================================#
# Speech Script for Barbers                                     #
#===============================================================#

from wolfpack.consts import *
from wolfpack.gumps import cGump
import wolfpack
import wolfpack.console

# Gump Types
TYPE_BUYGUMP = 259823146
TYPE_CHANGEHAIR = 259823147
TYPE_CHANGEHUE = 259823148

REGULAR = [
	[ "*****", 1602, 26 ],
	[ "*****", 1628, 27 ],
	[ "*****", 1502, 32 ],
	[ "*****", 1302, 32 ],
	[ "*****", 1402, 32 ],
	[ "*****", 1202, 24 ],
	[ "*****", 2402, 29 ],
	[ "*****", 2213, 6 ],
	[ "*****", 1102, 8 ],
	[ "*****", 1110, 8 ],
	[ "*****", 1118, 16 ],
	[ "*****", 1134, 16 ],
]

BRIGHT = [
	[ "*****", 12, 10 ],
	[ "*****", 32, 5 ],
	[ "*****", 38, 8 ],
	[ "*****", 54, 3 ],
	[ "*****", 62, 10 ],
	[ "*****", 81, 2 ],
	[ "*****", 89, 2 ],
	[ "*****", 1153, 2 ],
]

def changeHairHue(vendor, char, item, args):
	char.socket.closegump(TYPE_BUYGUMP)
	char.socket.closegump(TYPE_CHANGEHAIR)
	char.socket.closegump(TYPE_CHANGEHUE)
	
	price = item[1]
	if args[0] == 'all':
		layers = [LAYER_HAIR, LAYER_BEARD]
	elif args[0] == 'hair':
		layers = [LAYER_HAIR]
	elif args[0] == 'facial':
		layers = [LAYER_BEARD]
		
	if args[1] == 'regular':
		entries = REGULAR
	else:
		entries = BRIGHT
	
	gump = wolfpack.gumps.cGump()
	gump.setType(TYPE_CHANGEHUE)
	
	gump.startPage(0)
	gump.addResizeGump( 100, 10, 2600, 350, 370 )
	gump.addResizeGump( 120, 54, 5100, 110, 270 )
	gump.addXmfHtmlGump( 155, 25, 240, 30, 1011013, False, False ) # <center>Hair Color Selection Menu</center>	

	gump.addXmfHtmlGump( 150, 330, 220, 35, 1011014, False, False ) # // Dye my hair this color!
	gump.addButton( 380, 330, 4005, 4007, 1 )
	
	for i in range(0, len(entries)):
		gump.addText(130, 59 + i * 22, entries[i][0], entries[i][1] - 1)
		gump.addPageButton( 207, 60 + i * 22, 5224, 5224, 1 + i )
		
	for i in range(0, len(entries)):
		gump.startPage(i + 1)

		for j in range(0, entries[i][2]):
			gump.addText(278 + (j / 16) * 80, 52 + (j % 16) * 17, entries[i][0], entries[i][1] + j - 1)
			gump.addRadioButton( 255 + (j / 16) * 80, 52 + (j % 16) * 17, 210, 211, j * len(entries) + i, False )
	
	gump.setCallback('speech.barber.changehairhue_response')
	gump.setArgs([vendor.serial, entries, layers, price])
	gump.send(char)
	
def changehairhue_response(char, arguments, response):
	(vendor, entries, layers, price) = arguments
	vendor = wolfpack.findchar(vendor)
	
	if not vendor:
		return
	
	if response.button == 0 or len(response.switches) == 0:
		vendor.say(1013009, "", "", False, vendor.saycolor, char.socket)		
		return	
		
	# Get the hair color
	index = int(response.switches[0] % len(entries))
	offset = int(response.switches[0] / len(entries))
	
	if index >= len(entries):
		vendor.say(1013009, "", "", False, vendor.saycolor, char.socket)		
		return
		
	if offset >= entries[index][2]:
		vendor.say(1013009, "", "", False, vendor.saycolor, char.socket)
		return

	hue = entries[index][1] + offset

	gold = char.getbackpack().countitems(['eed'])
	gold += char.getbankbox().countitems(['eed'])
	
	if price > gold and not char.gm:
		vendor.say(1042293, "", "", False, vendor.saycolor, char.socket)
		return
		
	found = False
	items = []
	for layer in layers:
		item = char.itemonlayer(layer)
		if item:
			found = True
			items.append(item)
			
	if not found:
		vendor.say(502623, "", "", False, vendor.saycolor, char.socket)
		return
		
	if not char.gm:
		rest = char.getbackpack().removeitems(['eed'], arguments[3])
		if rest != 0:
			char.getbankbox().removeitems(['eed'], rest)
		
	for item in items:
		item.color = hue
		item.update()
		
# Entries
HAIR = [
	[ 50700,  70 - 137,  20 -  60, '203b' ],
	[ 60710, 193 - 260,  18 -  60, '2045' ],
	[ 50703, 316 - 383,  25 -  60, '2044' ],
	[ 60708,  70 - 137,  75 - 125, '203c' ],
	[ 60900, 193 - 260,  85 - 125, '2047' ],
	[ 60713, 320 - 383,  85 - 125, '204a' ],
	[ 60702,  70 - 137, 140 - 190, '203d' ],
	[ 60707, 193 - 260, 140 - 190, '2049' ],
	[ 60901, 315 - 383, 150 - 190, '2048' ],
	[ 0, 0, 0, None ],
]
	
BEARDS = [
	[ 50800, 120 - 187,  30 -  80, '2040' ],
	[ 50904, 243 - 310,  33 -  80, '204b' ],
	[ 50906, 120 - 187, 100 - 150, '204d' ],
	[ 50801, 243 - 310,  95 - 150, '203e' ],
	[ 50802, 120 - 187, 173 - 220, '203f' ],
	[ 50905, 243 - 310, 165 - 220, '204c' ],
	[ 50808, 120 - 187, 242 - 290, '2041' ],
	[ 0, 0, 0, None ],
]

def changeHairStyle(vendor, char, item, args):
	char.socket.closegump(TYPE_BUYGUMP)
	char.socket.closegump(TYPE_CHANGEHAIR)
	char.socket.closegump(TYPE_CHANGEHUE)
	
	facial = args[0] == 'facial'
	
	if facial:
		entries = BEARDS
		tableWidth = 2
		tableHeight = int((len(entries) + tableWidth - 1) / tableWidth)
		offsetWidth = 123
		offsetHeight = 70
		
	else:
		entries = HAIR
		tableWidth = 3
		tableHeight = int((len(entries) + tableWidth - 2) / tableWidth)
		offsetWidth = 123
		offsetHeight = 65

	gump = wolfpack.gumps.cGump()
	gump.setType(TYPE_CHANGEHAIR)
	
	gump.startPage(0)
	gump.addResizeGump(0, 0, 2600, 81 + tableWidth * offsetWidth, 105 + tableHeight * offsetHeight )
	gump.addButton( 45, 45 + tableHeight * offsetHeight, 4005, 4007, 1 )
	gump.addXmfHtmlGump(77 , 45 + tableHeight * offsetHeight, 90, 50, 1006044, False, False) # Ok
	gump.addButton( 81 + tableWidth * offsetWidth - 180, 45 + tableHeight * offsetHeight, 4005, 4007, 0)
	gump.addXmfHtmlGump(81 + tableWidth * offsetWidth - 148 , 45 + tableHeight * offsetHeight, 90, 50, 1006045, False, False) # Cancel

	if facial:
		gump.addXmfHtmlGump( 55, 15, 200, 20, 1018354, False, False) # New Beard
	else:
		gump.addXmfHtmlGump( 50, 15, 350, 20, 1018353, False, False) # New Hairstyle
		
		
	for i in range(0, len(entries)):
		xTable = i % tableWidth
		yTable = i / tableWidth
		
		if entries[i][0] != 0:
			gump.addRadioButton( 40 + xTable * offsetWidth, 70 + yTable * offsetHeight, 208, 209, i, False )
			gump.addResizeGump( 87 + xTable * offsetWidth, 50 + yTable * offsetHeight, 2620, 50, 50)
			gump.addGump( 87 + xTable * offsetWidth + entries[i][1], 50 + yTable * offsetHeight + entries[i][2], entries[i][0] )
		elif facial:
			gump.addRadioButton( 40 + xTable * offsetWidth, 70 + yTable * offsetHeight, 208, 209, i, False )
			gump.addXmfHtmlGump( 65 + xTable * offsetWidth, 70 + yTable * offsetHeight, 85, 35, 1011064, False, False) # Bald
		else:
			gump.addRadioButton( 40 + (xTable + 1) * offsetWidth, 240, 208, 209, i, False )
			gump.addXmfHtmlGump( 65 + (xTable + 1) * offsetWidth, 240, 85, 35, 1011064, False, False) # Bald

	gump.setCallback('speech.barber.hairstyle_response')
	gump.setArgs([vendor.serial, entries, facial, item[1]])
	gump.send(char)
		
def hairstyle_response(char, arguments, response):
	vendor = wolfpack.findchar(arguments[0])
	if not vendor:
		return
	
	if response.button == 0 or len(response.switches) == 0 or response.switches[0] >= len(arguments[1]):
		vendor.say(1013009, "", "", False, vendor.saycolor, char.socket)
		return
		
	item = arguments[1][response.switches[0]]
	facial = arguments[2]
	
	oldcolor = 0
	if facial:
		current = char.itemonlayer(LAYER_BEARD)
	else:
		current = char.itemonlayer(LAYER_HAIR)
		
	if current:
		oldcolor = current.color
	else:
		if facial:
			other = char.itemonlayer(LAYER_HAIR)
		else:
			other = char.itemonlayer(LAYER_BEARD)
		if other:
			oldcolor = other.color
		
	if not current and not item[3] or current and current.baseid == item[3]:
		return

	# Gold?
	female = char.id == 0x191
	gold = char.getbackpack().countitems(['eed'])
	gold += char.getbankbox().countitems(['eed'])
		
	if female and facial:
		vendor.say(1010639, "", "", False, vendor.saycolor, char.socket)
		return
	
	# Check if we can afford it
	if arguments[3] > gold and not char.gm:
		vendor.say(1042293, "", "", False, vendor.saycolor, char.socket)
		return	
		
	rest = char.getbackpack().removeitems(['eed'], arguments[3])
	if rest != 0:
		char.getbankbox().removeitems(['eed'], rest)
	
	if item[3]:
		newhair = wolfpack.additem(item[3])
		
		if newhair:
			newhair.color = oldcolor
			if current:
				current.delete()
			if facial:
				char.additem(LAYER_BEARD, newhair)
			else:
				char.additem(LAYER_HAIR, newhair)
			newhair.update()
	else:
		if current:
			current.delete()

# Cliloc Id, Price, Beard (True/False), function pointer, arguments
sellList = [
	[1018357, 50000, False, changeHairStyle, ['hair']],
	[1018358, 50000, True, changeHairStyle, ['facial']],
	[1018359, 50, False, changeHairHue, ['all', 'regular']],
	[1018360, 500000, False, changeHairHue, ['all', 'bright']],
	[1018361, 30000, False, changeHairHue, ['hair', 'regular']],
	[1018362, 30000, True, changeHairHue, ['facial', 'regular']],
	[1018363, 500000, False, changeHairHue, ['hair', 'bright']],
	[1018364, 500000, True, changeHairHue, ['facial', 'bright']],
]

def onSpeech( listener, speaker, text, keywords ):
	# Check if our name is in the beginning of the string
	if not text.lower().startswith( listener.name.lower() ) and not text.lower().startswith( 'vendor' ):
		return 0
		
	if 369 not in keywords and 60 not in keywords:
		return 0

	if ( speaker.distanceto( listener ) > 4 ):
		return 0

	if ( abs( speaker.pos.z - listener.pos.z ) > 5 ):
		return 0

	if speaker.id != 0x190 and speaker.id != 0x191:
		listener.say("I can't cut your hair!")
	else:
		gump(listener, speaker)
		
	return True

def gump( listener, speaker ):
	speaker.socket.closegump(TYPE_BUYGUMP)
	speaker.socket.closegump(TYPE_CHANGEHAIR)
	speaker.socket.closegump(TYPE_CHANGEHUE)
	
	female = speaker.id == 0x191
	gold = speaker.getbackpack().countitems(['eed'])
	gold += speaker.getbankbox().countitems(['eed'])
	
	count = 0
	
	for i in range(0, len(sellList)):
		item = sellList[i]
		if speaker.gm or (gold >= item[1] and (not female or not item[2])):
			count += 1

	gump = wolfpack.gumps.cGump()

	gump.addResizeGump(50, 10, 2600, 450, 100 + count * 25)
	gump.startPage(0)
	gump.addXmfHtmlGump( 100, 40, 350, 20, 1018356, False, False) # Choose your hairstyle change:

	offset = 0
	for i in range(0, len(sellList)):
		item = sellList[i]
		if not speaker.gm and (gold < item[1] or (female and item[2])):
			continue
		
		gump.addXmfHtmlGump(140, 75 + offset * 25, 360, 20, item[0], False, False) # Choose your hairstyle change:
		gump.addButton( 100, 75 + offset * 25, 4005, 4007, 1 + i)
		
		offset += 1

	gump.setType(TYPE_BUYGUMP)
	gump.setArgs([listener.serial])
	gump.setCallback("speech.barber.gump_response")
	gump.send(speaker)

def gump_response(char, arguments, response):
	vendor = wolfpack.findchar(arguments[0])
	if not vendor or vendor.distanceto(char) > 4:
		return
	
	if response.button == 0:
		return
		
	if response.button > len(sellList):		
		return
		
	item = sellList[response.button - 1]

	female = char.id == 0x191	
	gold = char.getbackpack().countitems(['eed'])
	gold += char.getbankbox().countitems(['eed'])
		
	# Check if we can afford it
	if female and item[2]:
		vendor.say(1010639, "", "", False, vendor.saycolor, char.socket)
		return
		
	if item[1] > gold and not char.gm:
		vendor.say(1042293, "", "", False, vendor.saycolor, char.socket)
		return
		
	item[3](vendor, char, item, item[4])
