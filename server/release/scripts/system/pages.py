
import wolfpack
from wolfpack.consts import *
import wolfpack.gumps
import commands.who
import wolfpack.sockets
import time
from urllib import quote, unquote

pages = None

#
# Wrapper for the page details gump
#
def page_details_response(player, arguments, response):		
	page = arguments[0]
	
	# Check if the page is still valid
	global pages
	if page not in pages.pages:
		return
		
	page.details_response(player, response)

#
# The class for a single page
#
class Page:
	def __init__(self, char):
		if type(char) == str:
			values = char.split(';')
			self.account = unquote( values[0] )
			self.serial = int(values[1])
			self.name = unquote(values[2])
			(x, y, z, m) = values[3].split(',')
			self.pos = wolfpack.coord(int(x), int(y), int(z), int(m))
			self.category = unquote(values[4])
			self.message = ['', '', '', '']
			self.message[0] = unicode(unquote(values[5]))
			self.message[1] = unicode(unquote(values[6]))
			self.message[2] = unicode(unquote(values[7]))
			self.message[3] = unicode(unquote(values[8]))
			self.created = float(values[9])		
		else:
			self.account = char.account.name
			self.serial = char.serial
			self.name = char.name
			self.pos = char.pos
			self.category = ''
			self.message = ['', '', '', '']
			self.created = 0

		self.assigned = None
		
	"""
		Return an integer for the category of this page.
	"""
	def type(self):
		if self.category == 'BUG':
			return 0
		elif self.category == 'STUCK':
			return 1
		elif self.category == 'GAMEPLAY':
			return 2
		else:
			return 3

	"""
		Build a string for the database.
	"""
	def tostring(self):
		values = []
		values.append( quote( self.account ) )
		values.append( str( self.serial ) )
		values.append( quote( self.name ) )		
		values.append( '%d,%d,%d,%d' % ( self.pos.x, self.pos.y, self.pos.z, self.pos.map ) )
		values.append( quote( self.category ) )
		values.append( quote( self.message[0] ) )
		values.append( quote( self.message[1] ) )
		values.append( quote( self.message[2] ) )
		values.append( quote( self.message[3] ) )
		values.append( str( self.created ) )
		return ";".join(values)
		
	"""
		Build a notification string for being shown to gms.
	"""
	def notification(self, updated = False):
		char = wolfpack.findchar(self.serial)
		if not char:
			return None
				
		if not updated:
			result = "New %s page from '%s' on account '%s':" % (self.category, char.name, self.account)
		else:
			result = "Updated %s page from '%s' on account '%s':" % (self.category, char.name, self.account)

		return result
		
	"""
		Show details for this page
	"""
	def details(self, player):
		char = wolfpack.findchar(self.serial)
		
		if not char:
			return
		
		# Assign the page to the viewer
		if not self.assigned:		
			# Requeue an old page the gm is assigned to
			global pages
			page = pages.findassigned(player.serial)					
			if page:
				page.assigned = None
				player.socket.sysmessage('You requeue your previous page.', 0x846)
				player.log(LOG_MESSAGE, "%s requeues gm page from %s (%s).\n" % (player.name, char.name, self.account))
		
			player.log(LOG_MESSAGE, "%s assigns gm page from %s (%s) to himself.\n" % (player.name, char.name, self.account))
			self.assigned = player.serial
			player.socket.sysmessage('You assign the gm page to yourself.', 0x846)
		
		assigned = self.assigned == player.serial
		if not assigned and self.assigned:
			player.socket.sysmessage('This page is already assigned to someone else.', 0x846)
			return
						
		gump = wolfpack.gumps.cGump()
		
		# Background
		gump.addBackground( 0xE10, 440, 440 ) 
		gump.addResizeGump( 195, 360, 0xBB8, 205, 20 )
		gump.addCheckerTrans( 15, 15, 410, 410 )

		# Gump Header
		gump.addGump( 165, 18, 0xFA2 )
		gump.addText( 200, 20, "Page Menu", 0x530 )
			
		gump.addText( 150, 400, "Category: %s" % self.category, 0x844 + 5 * self.type() );
		
		gump.startPage(1)
		hue = 0x834
		
		gump.addText( 50, 60, "Char name:", hue )
		gump.addText( 200, 60, char.name, hue )
		gump.addText( 50, 80, "Account name:", hue )
		gump.addText( 200, 80, self.account, hue )
		gump.addText( 50, 100, "Char position:", hue )
		gump.addText( 200, 100, str(char.pos), hue )
		gump.addText( 50, 120, "Page sent from:", hue )
		gump.addText( 200, 120, str(self.pos), hue )
		gump.addText( 50, 140, "Date/time:", hue )
		gump.addText( 200, 140, time.strftime(FORMAT_DATETIME, time.localtime(self.created)), hue )
		
		gump.addText( 50, 160, "Message:", hue )
		html = u"<body text=\"#0000FF\" leftmargin=\"0\" topmargin=\"0\" marginwidth=\"0\" marginheight=\"0\">%s</body>" % u"<br>".join(self.message)
		gump.addResizeGump( 45, 180, 0xBB8, 345, 84 )
		gump.addHtmlGump( 50, 180, 340, 80, unicode(html) )

		# Commands
		gump.addButton( 20, 280, 0xFA5, 0xFA7, 1 )
		gump.addText( 55, 280, "Go to char position", 0x834 )
		gump.addButton( 20, 300, 0xFA5, 0xFA71, 2 )
		gump.addText( 55, 300, "Bring char", 0x834 )
		gump.addButton( 20, 320, 0xFA5, 0xFA7, 3 )
		gump.addText( 55, 320, "Go to page position", 0x834 )
		gump.addButton( 20, 360, 0xFBD, 0xFBF, 4 )

		if char.socket:
			gump.addText( 55, 360, "Send message:", 0x834 )
			gump.addInputField( 200, 360, 190, 16, 0x834, 1, "<msg>" )
		
			gump.addButton( 220, 280, 0xFAB, 0xFAD, 5 )
			gump.addText( 255, 280, "Show socket info gump", 0x834 )

		gump.addButton( 220, 300, 0xFA5, 0xFA7, 6 )
		gump.addText( 255, 300, "Requeue page", 0x834 )
		gump.addButton( 220, 320, 0xFA5, 0xFA7, 7 )
		gump.addText( 255, 320, "Delete page", 0x834 )

		# X button
		gump.addText( 70, 400, "Close", 0x834 )
		gump.addButton( 30, 400, 0xFB1, 0xFB3, 0 )
		
		gump.setCallback("system.pages.page_details_response")
		gump.setArgs([self])
		gump.send(player.socket)
		
	"""
		Response from the details gump
	"""
	def details_response(self, player, response):
		if response.button == 0:
			return
	
		char = wolfpack.findchar(self.serial)
		
		if not char:
			return
	
		# Go to character
		if response.button == 1:
			player.removefromview()
			player.moveto(char.pos)
			player.update()
			player.socket.resendworld()
						
		# Bring character
		elif response.button == 2:
			char.removefromview()
			char.moveto(player.pos)
			char.update()
			if char.socket:
				char.socket.resendworld()

		# Go to page position
		elif response.button == 3:
			player.removefromview()
			player.moveto(self.pos)
			player.update()
			player.socket.resendworld()
			
		# Send message
		elif response.button == 4:
			if char.socket:
				char.socket.sysmessage('A message from %s: %s' % (player.name, response.text[1]), 0x846)
				player.socket.sysmessage('The message has been sent.', 0x846)
			else:
				player.socket.sysmessage('This player is currently offline. The message has not been sent.', 0x846)
			player.log(LOG_MESSAGE, "%s sends player %s (%s) a message: %s\n" % (player.name, char.name, self.account, response.text[1]))

		# Show socket info gump
		elif response.button == 5:
			if char.socket:
				commands.who.details(player, char)
			else:
				player.socket.sysmessage('This player is currently offline.', 0x846)
			
		# Requeue page
		elif response.button == 6:
			self.assigned = None
			player.socket.sysmessage('You requeue the page.', 0x846)
			player.log(LOG_MESSAGE, "%s requeues gm page from %s (%s).\n" % (player.name, char.name, self.account))
		
		# Delete page
		elif response.button == 7:
			self.assigned = None
			global pages
			pages.remove(self)
			
			if char.socket:
				char.socket.sysmessage('Your gm page has been deleted.', 0x846)

			player.socket.sysmessage('You remove the page.', 0x846)
			player.log(LOG_MESSAGE, "%s deletes gm page from %s (%s).\n" % (player.name, char.name, self.account))

#
# The page system class
#
class Pages:
	def __init__(self):
		self.pages = []

	"""
		Save the pages
	"""
	def save(self):
		i = 0
		for page in self.pages:
			i += 1
			wolfpack.setoption('page_%u' % i, page.tostring())
		wolfpack.setoption('pages', i)

	"""
		Load the pages
	"""
	def load(self):
		count = int(wolfpack.getoption('pages', '0'))
		for i in range(1, count+1):
			pagestr = wolfpack.getoption('page_%u' % i, '')
			if pagestr == '':
				continue
			page = Page(pagestr)
			
			char = wolfpack.findchar(page.serial)
			if char:			
				self.add(page, False)

	"""
		Get a page from the pages map.
	"""
	def find(self, name):
		name = hash(name.lower())
		for page in self.pages:
			if hash(page.account.lower()) == name:
				return page
		return None
	
	"""
		Check for invalidly assigned pages
	"""
	def check(self):
		toremove = []
		for page in self.pages:
			char = wolfpack.findchar(page.serial)
			if not char:
				toremove.append(page)
				continue
			if page.assigned:
				char = wolfpack.findchar(page.assigned)
				if not char or not char.socket:
					page.assigned = None
					continue
		for page in toremove:
			self.remove(page)
	
	"""
		Return the page count.
	"""
	def count(self):
		return len(self.pages)
		
	"""
		Get a page from the pages map.
	"""
	def findassigned(self, serial):
		for page in self.pages:
			if page.assigned == serial:
				return page
		return None
	
	"""
		Add a page to the pages map.
	"""
	def add(self, page, notify = True):
		updated = False
		if page in self.pages:
			updated = True
			self.pages.remove(page)
		self.pages.append(page)
		self.save()

		if notify:
			char = wolfpack.findchar(page.serial)
			if char:
				if updated:
					char.log(LOG_MESSAGE, "%s (%s) updates his gm page [%s]:\n%s\n" % (char.name, page.account, page.category, "\n".join(page.message)))					
				else:					
					char.log(LOG_MESSAGE, "%s (%s) makes a gm page [%s]:\n%s\n" % (char.name, page.account, page.category, "\n".join(page.message)))
		
			notification = page.notification(updated)
			socket = wolfpack.sockets.first()
			while socket:
				player = socket.player
				
				# If the pagenotify flag is on and it is a staff account,
				# send a notification about the new or updated page.
				if player and player.rank > 1 and player.account.flags & 0x10 != 0:				
					# Send the notification to the socket
					socket.sysmessage(notification, 0x846)
					
					for line in page.message:
						if len(line) > 0:
							socket.sysmessage(line, 0x843)

				socket = wolfpack.sockets.next()
		
	"""
		Resolve page.
	"""
	def remove(self, page):
		if page in self.pages:
			self.pages.remove(page)
			self.save()

#
# help_response
#
def help_response(char, arguments, response):
	global pages

	if response.button == 0:
		return

	page = pages.find(char.account.name)

	# Delete previous page
	if page and 5 in response.switches:
		pages.remove(page)
		char.socket.sysmessage('Your page has been removed.', 0x846)
		char.log(LOG_MESSAGE, "%s (%s) deletes his gm page.\n" % (char.name, page.account))
		return

	if len(response.text[1]) + len(response.text[2]) + len(response.text[3]) + len(response.text[4]) == 0:
		char.socket.sysmessage('You have to enter a message for your page.', 0x846)
		onHelp(char)
		return

	# Gather information
	message = (response.text[1], response.text[2], response.text[3], response.text[4])
	created = time.time()

	if not page:
		page = Page(char)

	page.message = message
	page.created = created
	if 1 in response.switches:
		page.category = 'BUG'
	elif 2 in response.switches:
		page.category = 'STUCK'
	elif 3 in response.switches:
		page.category = 'GAMEPLAY'
	else:
		page.category = 'BUILD'

	pages.add(page, True)
	char.socket.sysmessage('Your page has been queued.', 0x846)

#
# The global onHelp event
#
def onHelp(char):
	global pages	
	page = pages.find(char.account.name)
	
	gump = wolfpack.gumps.cGump()
	
	# Background
	if page:
		gump.addResizeGump(0, 40, 0xA28, 450, 330 + 40)
	else:
		gump.addResizeGump(0, 40, 0xA28, 450, 330)
	
	# Header
	gump.addGump( 105, 18, 0x58B ) # Fancy top-bar
	gump.addGump( 182, 0, 0x589 ) # "Button" like gump
	gump.addGump( 193, 10, 0x15E9 ) # "Button" like gump
	gump.addText( 190, 90, "Help menu", 0x530 )
	
	gump.addText( 50, 120, "Message:", 0x834 )
	gump.addResizeGump( 45, 140, 0xBB8, 345, 84 )
	
	# Message input
	lines = ['', '', '', '']
	if page:
		lines = page.message
	
	gump.addInputField( 50, 140, 330, 16, 0x834, 1, lines[0] )
	gump.addInputField( 50, 160, 330, 16, 0x834, 2, lines[1] )
	gump.addInputField( 50, 180, 330, 16, 0x834, 3, lines[2] )
	gump.addInputField( 50, 200, 330, 16, 0x834, 4, lines[3] )
	
	# Category selection
	category = 'BUG'
	if page:
		category = page.category
	
	gump.startGroup(1)
	gump.addText(50, 240, "Category:", 0x834)
	
	gump.addRadioButton( 50, 260, 0xD0, 0xD1, 1, category == 'BUG' )
	gump.addText( 80, 260, 'BUG', 0x834 )
	
	gump.addRadioButton( 250, 260, 0xD0, 0xD1, 2, category == 'STUCK' )
	gump.addText( 280, 260, 'STUCK', 0x834 )	
	
	gump.addRadioButton( 50, 280, 0xD0, 0xD1, 3, category == 'GAMEPLAY' )
	gump.addText( 80, 280, 'GAMEPLAY', 0x834 )	
	
	gump.addRadioButton( 250, 280, 0xD0, 0xD1, 4, category == 'BUILD' )
	gump.addText( 280, 280, 'BUILD', 0x834 )

	# Delete page if it exists
	if page:
		gump.addCheckbox( 50, 320, 0xd2, 0xd3, 5, 0 )
		gump.addText( 80, 320, 'Delete page', 0x834 )
	
		gump.addButton( 50, 320 + 40, 0xF9, 0xF8, 1 ) # OK button
		gump.addButton( 120, 320 + 40, 0xF3, 0xF1, 0 ) # Cancel button
	else:
		gump.addButton( 50, 320, 0xF9, 0xF8, 1 ) # OK button
		gump.addButton( 120, 320, 0xF3, 0xF1, 0 ) # Cancel button

	gump.startPage( 1 );

	gump.setCallback('system.pages.help_response')
	gump.send(char.socket)
	
	return True

#
# Response to the pages gump.
#
def pages_gump_response(player, arguments, response):	
	if response.button == 0:
		return
	
	# Show a detail gump for a given page.
	global pages
	id = response.button - 1
	
	if id < 0 or id >= len(pages.pages):
		return
	else:
		page = pages.pages[id]
		page.details(player)
	
#
# Show a pages gump to the player.
#
def pages_gump(player):
	global pages
	pages.check() # Make sure all pages are valid
	
	gump = wolfpack.gumps.cGump()
	
	gump.startPage(0)
	
	# Gump Background
	gump.addBackground( 0xE10, 480, 360 ) # Background
	gump.addCheckerTrans( 15, 15, 450, 330 )
	
	# Gump Header
	gump.addGump( 180, 18, 0xFA8 )
	gump.addText( 215, 20, "Page Queue", 0x530 )

	# Close Button
	gump.addText( 70, 320, "Close", 0x834 )
	gump.addButton( 30, 320, 0xFB1, 0xFB3, 0 )

	count = (pages.count() + 9) / 10

	# Iterate this loop once for every page	
	for i in range(0, count):
		gump.startPage(i + 1)
		offset = 0

		for j in range(i * 10, i * 10 + 10):
			if j < pages.count():
				page = pages.pages[j]
				pagetype = page.type()

				if not page.assigned:
					gump.addButton( 20, 60 + offset, 0xFA5, 0xFA7, 1 + j )

				char = wolfpack.findchar(page.serial)

				if char.socket:
					gump.addText( 55, 60 + offset, "%s (%s)" % (char.name, page.account), 0x844 + 5 * pagetype )
					gump.addText( 200, 60 + offset, page.category, 0x844 + 5 * pagetype )
					gump.addText( 280, 60 + offset, time.strftime(FORMAT_DATETIME, time.localtime(page.created)), 0x844 + 5 * pagetype )
				else:
					gump.addText( 55, 60 + offset, "%s (%s)" % (char.name, page.account), 2100 )
					gump.addText( 200, 60 + offset, page.category, 2100 )
					gump.addText( 280, 60 + offset, time.strftime(FORMAT_DATETIME, time.localtime(page.created)), 21000 )
					
				offset += 20
				
		gump.addText( 280, 320, "Page %u of %u" % ( i + 1, count ), 0x834 )

		# Are we on the first page
		if i > 0:
			gump.addPageButton( 240, 320, 0x0FC, 0x0FC, i - 1 )

		# Would the next page be valid?
		if i + 1 < count:
			gump.addPageButton( 260, 320, 0x0FA, 0x0FA, i + 2 )

	gump.setCallback("system.pages.pages_gump_response")
	gump.send(player.socket)

#
# If we're not assigned to any page, get a new one.
#
def page(socket, command, arguments):
	player = socket.player	
	page = pages.findassigned(player.serial)
	
	# Show a selection gump
	if not page:
		pages_gump(player)
	
	# Process page commands
	else:
		page.details(player)

#
# Initialize the page system
#
def onLoad():
	global pages
	pages = Pages()	
	wolfpack.registerglobal(EVENT_HELP, "system.pages")
	wolfpack.registercommand("page", page)
	
	if not wolfpack.isstarting():
		pages.load()
		
#
# Server startup complete
#
def onServerStart():
	global pages
	pages.load()
