
import wolfpack
from wolfpack.consts import *
import wolfpack.gumps
import time

pages = None

#
# The class for a single page
#
class Page:
	def __init__(self, char):
		self.account = char.account.name
		self.serial = char.serial
		self.name = char.name
		self.pos = char.pos
		self.category = ''
		self.message = ['', '', '', '']
		self.created = 0
		self.assigned = None

	def tostring(self):
		pass

#
# The page system class
#
class Pages:
	def __init__(self):
		self.pages = {}

	"""
		Save the pages
	"""
	def save(self):
		pass
		
	"""
		Load the pages
	"""
	def load(self):
		pass

	"""
		Get a page from the pages map.
	"""
	def find(self, name):
		name = name.lower()
		if not self.pages.has_key(name):
			return None
		else:
			return self.pages[name]
	
	"""
		Add a page to the pages map.
	"""
	def add(self, page, notify = True):
		self.pages[page.account.lower()] = page
		self.save()

		if notify:
			pass
		
	"""
		Resolve page.
	"""
	def remove(self, page, notify = False):
		if self.pages.has_key(page.account.lower()):
			del self.pages[page.account.lower()]
			self.save()
			
			# Notify Counselors and GMs with the pagenotify flag activated
			if notify:
				pass

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
		pages.remove(page, True) # Notify GMs
		char.socket.sysmessage('Your page has been removed.')
		return

	if len(response.text[1]) + len(response.text[2]) + len(response.text[3]) + len(response.text[4]) == 0:
		char.socket.sysmessage('You have to enter a message for your page.')
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
	char.socket.sysmessage('Your page has been queued.')

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
# Initialize the page system
#
def onLoad():
	global pages
	pages = Pages()
	wolfpack.registerglobal(EVENT_HELP, "system.pages")

#
# Finalize the page system.
#	
def onUnload():
	pass
