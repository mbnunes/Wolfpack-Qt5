#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: khpae                          #
#   )).-" {{ ;"`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification: Created                 #
#################################################################

import wolfpack
from wolfpack import tr
import magic
import magic.spell
from wolfpack.gumps import cGump
import system.input
import magic
from magic.rune import isrune
import magic.scroll
from magic.utilities import fizzle, MODE_BOOK, hasSpell
import random
import wolfpack.utilities

import housing.security
import housing.house

#
# Is Runebook?
#
def isRunebook(runebook):
	return runebook.hasscript('magic.runebook') and runebook.id == 0x22c5
	
#
# Is Rune?
#
def isRune(rune):
	return rune.hasscript('magic.rune') and (rune.id >= 0x1f14 and rune.id <= 0x1f17)

#
# Is the item a recall scroll?
#
def isRecallScroll(scroll):
	return scroll.hasscript('magic.scroll') and scroll.id == 0x1f4c

#
# Show a custom tooltip for runebooks
#
def onShowTooltip(player, runebook, tooltip):
	if runebook.hastag('description'):
		description = unicode(runebook.gettag('description'))
		if len(description) > 0:
			tooltip.add(1042971, description)

#
# Convert this runebook by moving all the old runes out of it
#
def convertRunebook(runebook):
	if runebook.container.isitem():
		wolfpack.utilities.cont2cont(runebook, runebook.container)

#
# Show the runebook as long as we are in range
#
def onUse(player, runebook):
	convertRunebook(runebook) # Legacy conversion
	
	if player.canreach(runebook, 1):
		closeGump(player, runebook)
		sendGump(player, runebook)
	return True

#
# Get the books charges
#
def getCharges(runebook):
	(charges, maxcharges) = (0, 6) # Default
	if runebook.hastag('maxcharges'):
		try:
			maxcharges = int(runebook.gettag('maxcharges'))
		except:
			maxcharges = 6
	if runebook.hastag('charges'):
		try:
			charges = min(maxcharges, int(runebook.gettag('charges')))
		except:
			charges = 0
	return (charges, maxcharges)

#
# Get a single entry or None if that entry does not exist
#
def getEntry(runebook, index):
	if index < 0 or index > 15:
		return None
		
	tagname = 'entry%u' % index
	
	if not runebook.hastag(tagname):
		return None
		
	try:
		(x, y, z, map) = runebook.gettag(tagname).split(',')
		pos = wolfpack.coord(int(x), int(y), int(z), int(map))
	except:
		return None

	return pos

#
# Retrieve the entries for this book
# Index, Name, Location (coord)
#
def getEntries(runebook):
	entries = []
	
	for i in range(0, 16):
		tagname = 'entry%u' % i
		if runebook.hastag(tagname):
			try:
				(x, y, z, map) = runebook.gettag(tagname).split(',')
				pos = wolfpack.coord(int(x), int(y), int(z), int(map))

				if runebook.hastag(tagname + 'name'):
					name = runebook.gettag(tagname + 'name')
				else:
					name = tr('(indescript)')
				
				entries.append([i, name, pos])
			except:
				raise
				runebook.deltag(tagname)
				runebook.deltag(tagname + 'name')

	return entries

#
# Get the first free index or -1 if the book is full
#
def getFreeIndex(runebook):
	for i in range(0, 16):
		if not runebook.hastag('entry%u' % i):
			return i
	return -1 # No free entry was found

#
# An item is being dropped on this runebook
#
def onDropOnItem(runebook, item):
	if not isRunebook(runebook):
		return False # Only handle for dropping something on the runebook

	if not item.container or not item.container.ischar():
		return False # Some internal error or script before us
	
	player = item.container
	
	if isRune(item):
		location = None
		if item.hastag('marked') and item.hastag('location'):
			try:
				(x, y, z, map) = item.gettag('location').split(',')
				location = wolfpack.coord(int(x), int(y), int(z), int(map))
			except:
				pass
			
		if location:
			index = getFreeIndex(runebook)
			
			if index != -1:
				tagname = 'entry%u' % index
				name = item.name.strip()
				if len(name) != 0:
					runebook.settag(tagname + 'name', name)
				else:
					runebook.deltag(tagname + 'name') # Make sure there is no old name
				runebook.settag(tagname, str(location))
				player.soundeffect(0x42)
				item.delete()
				return True
				
			else:
				player.socket.clilocmessage(502401)
		else:
			player.socket.clilocmessage(502409)		

	elif isRecallScroll(item):
		# Recharge if possible
		(charges, maxcharges) = getCharges(runebook)
		if charges < maxcharges:
			consume = min(item.amount, maxcharges - charges)
			runebook.settag('charges', charges + consume)
			player.soundeffect(0x249)
			
			if consume == item.amount:
				item.delete()
				return True # The item has been removed
			else:
				item.amount -= consume
				item.update()
				item.resendtooltip()
		else:
			player.socket.clilocmessage(502410)

	return False

	# Bounce Code
	if char.socket:
		char.socket.clilocmessage( 502401 )
	char.getbackpack().additem(item)
	item.pos = book.pos
	item.pos.z += 1
	item.update()
	return True

def closeGump(char, item):
	char.socket.closegump(0x87654322)
	char.socket.deltag('runebook_open')

def sendGump(char, item):
	char.socket.settag('runebook_open', 1)
	char.soundeffect(0x55)
	runebook = cGump(0, 0, 0, 100, 120)

	runebook.startPage( 0 )
	runebook.addGump( 100, 10, 2200 )

	# decoration bar
	n = 125
	m = 0
	for i in range( 0, 16 ):
		if( i == 8 ):
			n = 170
		if( i % 8 == 0 ):
			m = 57
		elif( i % 8 == 7 ):
			m = 59
		else:
			m = 58
		runebook.addGump( n + i * 15, 50, m )

	# page number button at the bottom of the book
	for i in range( 0, 8 ):
		j = i / 4
		runebook.addPageButton( 130 + i * 35 + j * 30, 187, i + 2225, i + 2225, i + 2 )

	# charge / max charge
	(charges, maxcharges) = getCharges(item)
	runebook.addText( 160, 40, "Charges: %i" % charges )
	runebook.addText( 300, 40, "Max Charges: %i" % maxcharges )

	runebook.startPage( 1 )

	# rename button return code = 1000
	runebook.addButton( 130, 20, 2472, 2473, 1000 )
	runebook.addText( 158, 22, tr("Rename Book") )

	# next page - top right corner
	runebook.addPageButton( 393, 14, 2206, 2206, 2 )

	entries = getEntries(item) # Get runebook entries

	# blue button and rune name
	i = 0
	for entry in entries:
		if( i < 8 ):
			x = 130
		else:
			x = 290
		y = 60 + ( i % 8 ) * 15

		# blue button return code = rune number + 1
		# tramel color : 115, feluca color : ???
		namecolor = 10
		runebook.addButton( x, y + 5, 2103, 2104, 1 + entry[0] )
		runebook.addText( x + 15, y, entry[1], namecolor )

		i += 1

	if item.hastag("default"):
		default = item.gettag("default")
	else:
		item.settag("default", 0 )
		default = 0

	# page 2 - 9
	for i in range( 2, 10 ):
		runebook.startPage( i )

		# next page - top right corner
		if( i < 9 ):
			runebook.addPageButton( 393, 14, 2206, 2206, i + 1 )

		# previous page - top left corner
		runebook.addPageButton( 124, 14, 2205, 2205, i - 1 )

		# for each sub-page
		for j in range( 0, 2 ):
			# rune number
			k = ( i - 2 ) * 2 + j
			
			if k >= len(entries):
				runebook.addText( 145 + j * 160, 60, tr('Empty') )
				continue
			
			# blue button return code = 1 + rune number
			runebook.addButton( 130 + j * 160, 65, 2103, 2104, 1 + entries[k][0] )
			# recall button return code = 301 + rune number
			runebook.addButton( 135 + j * 160, 140, 2271, 2271, 301 + entries[k][0] )
			# gate button return code = 401 + rune number
			runebook.addButton( 205 + j * 160, 140, 2291, 2291, 401 + entries[k][0] )

			namecolor = 10
			runebook.addText( 145 + j * 160, 60, entries[k][1], namecolor )

			# Drop button return code = 201 + rune number
			runebook.addButton( 135 + j * 160, 115, 2437, 2438, 201 + entries[k][0] )
			runebook.addText( 150 + j * 160, 115, tr("Drop rune") )
	
	runebook.setArgs( [ item.serial ] )
	runebook.setType( 0x87654322 )
	runebook.setCallback( "magic.runebook.callback" )

	# send it
	runebook.send( char )

# Rename the runebook
def renameBook(player, runebook):
	player.socket.clilocmessage(502414) # Enter a name...
	system.input.request(player, runebook, 1)
	
# Process text input
def onTextInput(player, runebook, id, text):
	if not player.canreach(runebook, 1):
		return True
		
	# Check the housing security
	if runebook.multi:
		if housing.house.onCheckSecurity(player, runebook.multi, runebook):
			player.socket.clilocmessage(502413)
			return True
			
	runebook.settag('description', text)
	runebook.resendtooltip()
	player.socket.sysmessage(tr("The book's title has been changed."))
	
	closeGump(player, runebook)
	sendGump(player, runebook)
	
	return True

# Process text input cancel
def onTextInputCancel(player, runebook, id):
	player.socket.clilocmessage(502415)
	
	if player.canreach(runebook, 1):
		closeGump(player, runebook)
		sendGump(player, runebook)
		
	return True

# callback function - process gump callback
def callback( char, args, target ):
	item = wolfpack.finditem(args[0])
	closeGump(char, item)

	if target.button == 0:
		return
	
	if not char.canreach(item, 1):
		return

	if target.button == 1000:
		renameBook(char, item)
		return

	button = target.button
	
	# Get selected rune number
	runenum = ( button - 1 ) % 100
	
	target = getEntry(item, runenum)
		
	if not target:
		char.socket.clilocmessage(502411) # No such rune
		return True

	# Use a charge or use the recall spell
	if( button > 0 and button < 17 ):
		(charges, maxcharges) = getCharges(item)			
		if charges > 0:
			char.say( "Kal Ort Por", 5 )
			char.addtimer( 2000, "magic.runebook.recall0", [ item.serial, target ] )
		elif hasSpell(char, 32):
			char.addtimer( 0, "magic.runebook.recall1", [item.serial, target] ) # Fall back to the Recall Spell
		else:
			char.socket.clilocmessage(502412) # No charges left

	# Drop Rune
	elif( button > 200 and button < 217 ):
		name = ''
		if item.hastag('entry%uname' % runenum):
			name = unicode(item.gettag('entry%uname' % runenum))
		
		RUNEIDS = ['1f14', '1f15', '1f16', '1f17']
		rune = wolfpack.additem(random.choice(RUNEIDS))
		rune.settag('marked', 1)
		rune.settag('location', str(target))
		rune.name = name
		
		if not wolfpack.utilities.tobackpack(rune, char):
			rune.update()
		
		char.socket.clilocmessage(502421)
		
		item.deltag('entry%u' % runenum)
		item.deltag('entry%uname' % runenum)
		
		closeGump(char, item)
		sendGump(char, item)

	# recall button - spell : 301 - 316
	elif( button > 300 and button < 317 ):
		char.addtimer( 0, "magic.runebook.recall1", [item.serial, target] )

	# gate button : 401 - 416
	elif( button > 400 and button < 417 ):
		char.addtimer( 0, "magic.runebook.gate", [item.serial, target] )

# recall using charges
def recall0( char, args ):
	runebook = wolfpack.finditem(args[0])
	location = args[1]
	
	if not runebook or not char:
		return

	if not char.gm:
		if char.iscriminal():
			if char.socket:
				char.socket.clilocmessage(1005561)
			return

		if char.attacktarget:
			if char.socket:
				char.socket.clilocmessage(1005564)
			return

	(charges, maxcharges) = getCharges(runebook)
	
	if charges <= 0:
		char.socket.clilocmessage(502412)
		return # No charges left
		
	runebook.settag('charges', charges - 1) # Reduce runebook charges

	region = None
	region = wolfpack.region(location.x, location.y, location.map)

	if not location.validspawnspot():
		char.message(501942)
		fizzle(char)
		return

	if region and region.norecallin:
		char.message(1019004)
		fizzle(char)
		return False

	# Move his pets if he has any
	if char.player:
		for follower in char.followers:
			if follower.wandertype == 4 and follower.distanceto(char) < 5:
				follower.removefromview()
				follower.moveto(location)
				follower.update(0)

	char.soundeffect(0x1fc)
	char.removefromview()
	char.moveto(location)
	char.update()
	char.socket.resendworld()
	char.soundeffect(0x1fc)
	return True

# Use the Recall spell to recall to the selected rune
def recall1( char, args ):
	runebook = wolfpack.finditem(args[0])
	target = args[1]
	
	if char and runebook:
		magic.castSpell( char, 32, MODE_BOOK, args = [target], target = runebook )

# Use the Gate spell to open a gate to the selected rune
def gate( char, args ):
	runebook = wolfpack.finditem(args[0])
	target = args[1]
	
	if char and runebook:
		magic.castSpell( char, 52, MODE_BOOK, args = [target], target = runebook )
