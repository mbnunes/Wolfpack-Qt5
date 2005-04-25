
"""
	\library wolfpack.gumps
	\description Contains several functions for gumps.
"""

from types import *

#
# Default Callback
#
def dummy_callback(player, arguments, response):
	pass

class cGump:
	"""
	\function wolfpack.gumps.cGump
	\param noclose
	Defaults to 0.
	\param nomove
	Defaults to 0.
	\param nodispose
	Defaults to 0.
	\param x
	The starting x position of the gump.
	\param y
	The starting y position of the gump.
	\param callback
	Defaults to dummy_callback.
	\param args
	Defaults to an empty list.
	\param type
	Defaults to 0.
	\param serial
	Defaults to 0.
	\return Gump object
	\description Creates the initial gump.
"""
	def __init__(self, noclose=0, nomove=0, nodispose=0, x=50, y=50, callback=None, args=[], type=0, serial=0):
		self.layout = []
		self.texts = []
		self.args = args
		if not callback:
			self.callback = dummy_callback
	 	else:
			self.callback = callback
		self.noclose = noclose
		self.nomove = nomove
		self.nodispose = nodispose
		self.x = x
		self.y = y
		self.typeid = type
		self.serialid = serial

"""
	\function wolfpack.gumps.cGump.copy
	\param gump
	This is the gump which is to be copied.
	\return Returns a copy of the gump.
	\description Takes a gump and returns a copy.
"""
	def copy(othergump):
		new = cGump()
		new.layout = othergump.layout
		new.texts = othergump.texts
		new.args = othergump.args
		new.callback = othergump.callback
		new.noclose = othergump.noclose
		new.nomove = othergump.nomove
		new.nodispose = othergump.nodispose
		new.x = othergump.x
		new.y = othergump.y
		new.typeid = othergump.typeid
		new.serialid = othergump.serialid
		return new

"""
	\function wolfpack.gumps.cGump.send
	\param char
	The character to receive the gump.
	\return None
	\description Sends a gump to a character.
"""
	# Send the gump
	def send( self, char ):
		# There are two possibilities
		socket = None

		if type( char ).__name__ == "wpchar":
			socket = char.socket
		elif type( char ).__name__ == "wpsocket":
			socket = char
		else:
			raise TypeError( "You passed an invalid socket." )

		# Dump the gump data
		#file = open( 'dump.txt', 'w' )
		#file.write( "--------\nLayout:\n" )
		#for line in self.layout:
		# file.write( line + "\n" )
		#file.write( "--------\nText:\n" )
		#for line in self.texts:
		# file.write( line + "\n" )
		#file.close()

		if( self.noclose == 1 ):
			self.layout.insert( 0, '{ noclose }' )

		if( self.nodispose == 1 ):
			self.layout.insert( 0, '{ nodispose }' )

		if( self.nomove == 1 ):
			self.layout.insert( 0, '{ nomove }' )

		socket.sendgump( self.x, self.y, 0, 0, 0, self.serialid, self.typeid, self.layout, self.texts, self.callback, self.args )
		return

"""
	\function wolfpack.gumps.cGump.addRawLayout
	\param data
	Unknown
	\return None
	\description Unknown
"""
	# For "rawly" modifying the list
	def addRawLayout( self, data ):
		self.layout.append( data )
		return

"""
	\function wolfpack.gumps.cGump.addRawText
	\param data
	Unknown
	\return None
	\description Unknown
"""
	def addRawText( self, data ):
		# Find the text
		if data in self.texts:
			return self.texts.index(data)
		else:
		# Insert the text
			self.texts.append(data)
			return len(self.texts) - 1

"""
	\function wolfpack.gumps.cGump.setCallback
	\param callback
	The callback function to pass the gump to.
	\return None
	\description Defines the callback function for the gump.
"""
	# Sets the Callback function which is going to be called whenever the user
	# clicks something on the gump
	def setCallback( self, callback ):
		if not type( callback ) is FunctionType:
			raise TypeError( "You have to pass a function to setCallback" )
		else:
			self.callback = callback
		return

"""
	\function wolfpack.gumps.cGump.setArgs
	\param arguments
	ListType of arguments.
	\return None
	\description Sets a list of arguments to the gump.
"""
	# Set the arguments you want to pass to the gump-response handler later on
	def setArgs( self, args ):
		if not type( args ) is ListType:
			raise TypeError( "You have to pass a list to setArgs" )
		else:
			self.args = args
		return

"""
	\function wolfpack.gumps.cGump.setType
	\param typeid
	An unique id for the gump.
	\return None
	\description Sets the gump-type id, used to close it later on.
"""
	# Set the Gump-Type (this can be used to close it later on)
	def setType( self, typeid ):
		self.typeid = typeid
		return

"""
	\function wolfpack.gumps.cGump.setSerial
	\param serialid
	An unique serial id for the gump.
	\return None.
	\description Assigns a serial id to the gump, used to insure only one gump of this type is open at once.
"""
	# Set the Gump-Serial (this can be used to only open one gump of the same type at the same moment)
	# This will auto-close the first gump
	# 0 means the Gump will automatically choose a serial
	def setSerial( self, serialid ):
		if not type( serialid ) is IntType:
			raise TypeError( "You have to pass an integer to setSerial" )
		else:
			self.serialid = serialid
		return

"""
	\function wolfpack.gumps.cGump.addTooltip
	\param id
	The cliloc id of the tooltip to be added.
	\return None
	\description Appends a tooltip cliloc id to the gump.
"""
	### Here are the helper functions for adding the layout elements
	def addTooltip(self, id):
		if not type(id) is IntType:
			raise TypeError("addTooltip only accepts cliloc ids.")
		self.layout.append('{tooltip %d}' % id)
		return

"""
	\function wolfpack.gumps.cGump.startPage
	\param page
	The page number to start on.
	\return None
	\description Sets the starting page of the gump.
"""
	def startPage( self, page ):
		self.layout.append( "{page %u}" % page )
		return

"""
	\function wolfpack.gumps.cGump.startGroup
	\param groupid
	The group id to start with.
	\return None
	\description Sets the starting group id of the gump.
"""
	def startGroup( self, groupid ):
		self.layout.append( "{group %u}" % groupid )
		return

"""
	\function wolfpack.gumps.cGump.addText
	\param x
	The x value of the text alignment.
	\param y
	The y value of the text alignment.
	\param text
	The string value to append to the gump.
	\param hue
	The hue of the text, defaults to 0x0.
	\return None
	\description Appends text to the gump.
"""
	def addText( self, x, y, text, hue=0 ):
		self.layout.append("{text %i %i %u %u}" % (x, y, hue, self.addRawText(text)))
		return

"""
	\function wolfpack.gumps.cGump.addBackground
	\param id
	The artwork id of the background.
	\param width
	The width of the background.
	\param height
	The height of the background.
	\return None
	\description Adds a background to the gump.
"""
	def addBackground( self, id, width, height ):
		self.addResizeGump( 0, 0, id, width, height )
		return

"""
	\function wolfpack.gumps.cGump.addResizeGump
	\param x
	The starting x coordinate of the gump.
	\param y
	The starting y coordinate of the gump.
	\param id
	The artwork id used for the gump.
	\param width
	The width of the gump.
	\param height
	The height of the gump.
	\return None
	\description Resizes a gump to the given coordinates.
"""
	def addResizeGump( self, x, y, id, width, height ):
		self.layout.append( "{resizepic %i %i %u %u %u}" % ( x, y, id, width, height ) )
		return

"""
	\function wolfpack.gumps.cGump.addCroppedText
	\param x
	The x alignment of the text.
	\param y
	The y alignment of the text.
	\param width
	The weidth limit to crop the text to, in pixels.
	\param height
	The height limit to crop the text to, in pixels.
	\param text
	The text string to be added.
	\param hue
	The hue of the text.
	\return None
	\description Adds cropped text to the gump.
"""
	def addCroppedText( self, x, y, width, height, text, hue=0 ):
		self.layout.append( "{croppedtext %i %i %u %u %u %u}" % ( x, y, width, height, hue, self.addRawText( text ) ) )
		return

"""
	\function wolfpack.gumps.cGump.addButton
	\param x
	The x alignment of the button.
	\param y
	The y alignment of the button.
	\param upid
	The artwork id used when the button is not pressed.
	\param downid
	The artwork id used when the button is being pressed.
	\param returncode
	The return code passed to the callback if the button is pressed.
	\return None
	\description Adds a button to the gump.
"""
	def addButton( self, x, y, up, down, returncode ):
		self.layout.append( "{button %i %i %u %u 1 0 %u}" % ( x, y, up, down, returncode ) )
		return

"""
	\function wolfpack.gumps.cGump.addPageButton
	\param x
	The x alignment of the button.
	\param y
	The y alignment of the button.
	\param upid
	The artwork id used when the button is not pressed.
	\param downid
	The artwork id used when the button is being pressed.
	\param page
	The page to turn to.
	\return None
	\description Adds a page button to the gump.
"""
	def addPageButton( self, x, y, up, down, page ):
		self.layout.append( "{button %i %i %u %u 0 %u 0}" % ( x, y, up, down, page ) )
		return

"""
	\function wolfpack.gumps.cGump.addGump
	\param x
	The x alignment of the gump.
	\param y
	The y alignment of the gump.
	\param id
	The artwork id of the gump.
	\param hue
	The optional hue to apply to the gump artwork, defaults to 0x0.
	\return None
	\description Adds a gump object to the gump.
"""
	def addGump( self, x, y, id, hue=0 ):
		# A non-colored gump
		self.layout.append( "{gumppic %i %i %u hue=%u}" % ( x, y, id, hue ) )
		return

"""
	\function wolfpack.gumps.cGump.addTiledGump
	\param x
	The x alignement of the gump.
	\param y
	The y alignment of the gump.
	\param width
	The width of the gump.
	\param height
	The height of the gump.
	\param id
	The artwork id to be tiled.
	\param hue
	The optional hue to apply to the gump artwork, defaults to 0x0.
	\return None
	\description Adds a tiled gump object.
"""
	# Seems NOT hueable!
	def addTiledGump( self, x, y, width, height, id, hue = 0 ):
		self.layout.append( "{gumppictiled %i %i %u %u %u hue=%u}" % ( x, y, width, height, id, hue ) )
		return

"""
	\function wolfpack.gumps.cGump.addTilePic
	\param x
	The x alignment of the gump object.
	\param y
	The y alignment of the gump object.
	\param id
	The artwork id to be used.
	\return None
	\description Adds an object's artwork id to the gump.
"""
	def addTilePic( self, x, y, id ):
		self.layout.append( "{tilepic %i %i %u}" % ( x, y, id ) )
		return

"""
	\function wolfpack.gumps.cGump.addInputField
	\param x
	\param y
	\param width
	\param height
	\param hue
	\param id
	\param starttext
	\return None
	\description Adds an input field to the gump.
"""
	def addInputField( self, x, y, width, height, hue, id, starttext ):
		self.layout.append( "{textentry %i %i %u %u %u %u %u}" % ( x, y, width, height, hue, id, self.addRawText( starttext ) ) )
		return

"""
	\function wolfpack.gumps.cGump.addCheckbox
	\param x
	\param y
	\param off
	\param on
	\param id
	\param checked
	Defaults to 0 or False.
	\return None
	\description Adds a checkbox to the gump.
"""
	def addCheckbox( self, x, y, off, on, id, checked = 0 ):
		# Just to prevent errors
		if( checked != 0 ):
			checked = 1
		self.layout.append( "{checkbox %i %i %u %u %u %u}" % ( x, y, off, on, checked, id ) )
		return

"""
	\function wolfpack.gumps.cGump.addRadioButton
	\param x
	\param y
	\param off
	\param on
	\param id
	\param selected
	Defaults to 0 or False.
	\return None
	\description Adds a readio button to the gump.
"""
	def addRadioButton( self, x, y, off, on, id, selected = 0 ):
		# Just to prevent errors
		if( selected != 0 ):
			selected = 1
		self.layout.append( "{radio %i %i %u %u %u %u}" % ( x, y, off, on, selected, id ) )
		return

"""
	\function wolfpack.gumps.cGump.addHtmlGump
	\param x
	\param y
	\param width
	\param height
	\param html
	\param hasBack
	Defaults to 0 or False.
	\param canScroll
	Defaults to 0 or False.
	\return None
	\description Adds an HTML text field to the gump.
"""
	def addHtmlGump(self, x, y, width, height, html, hasBack = 0, canScroll = 0):
		if( canScroll != 0 ):
			canScroll = 1
		if( hasBack != 0 ):
			hasBack = 1
		self.layout.append( "{htmlgump %i %i %u %u %u %u %u}" % ( x, y, width, height, self.addRawText( html ), hasBack, canScroll ) )
		return

"""
	\function wolfpack.gumps.cGump.addXmfHtmlGump
	\param x
	\param y
	\param width
	\param height
	\param clilocid
	\param hasBack
	Defaults to 0 or False.
	\param canScroll
	Defaults to 0 or False.
	\param color
	Defaults to 0x0.
	\return None
	\description Unknown
"""
	def addXmfHtmlGump( self, x, y, width, height, clilocid, hasBack = 0, canScroll = 0, color = 0 ):
		if( canScroll != 0 ):
			canScroll = 1
		if( hasBack != 0 ):
			hasBack = 1

		if color != 0:
			self.layout.append( "{xmfhtmlgumpcolor %i %i %u %u %u %u %u %u}" % ( x, y, width, height, clilocid, hasBack, canScroll, color ) )
		else:
			self.layout.append( "{xmfhtmlgump %i %i %u %u %u %u %u}" % ( x, y, width, height, clilocid, hasBack, canScroll ) )
		return

"""
	\function wolfpack.gumps.cGump.addCheckerTrans
	\param x
	\param y
	\param width
	\param height
	\return None
	\description Adds a commonly used checkered transparency effect to the gump.
"""
	def addCheckerTrans( self, x, y, width, height ):
		self.layout.append( "{checkertrans %i %i %u %u}" % ( x, y, width, height ) )
		return

"""
	\function wolfpack.gumps.cGump.add
	\param line
	\return None
	\description Unknown
"""
	def add( self, line ):
		self.layout.append( line )
		return

"""
	\function wolfpack.gumps.cGump.dummyCallback
	\param player
	\param args
	\param choice
	\return None
	\description A dummy callback.
"""
	def dummyCallback( player, args, choice ):
		pass

class WarningGump:

	def __init__(self, header, headerColor, content, contentColor, width, height, callback, state ):
		self.callback = callback
		self.state = state

		self.gump = cGump( 1, 0, 0, (640 - width) / 2, (480 - height) / 2 )
		self.gump.startPage( 0 )
		self.gump.addBackground( 5054, width, height )
		self.gump.addTiledGump( 10, 10, width - 20, 20, 2624 )
		self.gump.addCheckerTrans( 10, 10, width - 20, 20 )
		self.gump.addXmfHtmlGump( 10, 10, width - 20, 20, header, 0, 0, headerColor )
		self.gump.addTiledGump( 10, 40, width - 20, height - 80, 2624 )
		self.gump.addCheckerTrans( 10, 40, width - 20, height - 80 )
		if type( content ) is IntType:
			self.gump.addXmfHtmlGump( 10, 40, width - 20, height - 80, content, 0, 1, contentColor )
		else:
			self.gump.addHtmlGump( 10, 40, width - 20, height - 80, "<BASEFONT COLOR=#%x>%s</BASEFONT>" % ( contentColor, content ), 0, 1 )

		self.gump.addTiledGump( 10, height - 30, width - 20, 20, 2624 )
		self.gump.addCheckerTrans( 10, height - 30, width - 20, 20 )
		self.gump.addButton( 10, height - 30, 4005, 4007, 1 )
		self.gump.addXmfHtmlGump( 40, height - 30, 170, 20, 1011036, 0, 0, 32767 )

		self.gump.addButton( 10 + ((width - 20) / 2), height - 30, 4005, 4007, 0 )
		self.gump.addXmfHtmlGump( 40 + ((width - 20) / 2), height - 30, 170, 20, 1011012, 0, 0, 32767 )

		# set the callback

	def send( self, char ) :
		# There are two possibilities
		socket = None

		if type( char ).__name__ == "wpchar":
			socket = char.socket
		elif type( char ).__name__ == "wpsocket":
			socket = char
		else:
			raise TypeError( "You passed an invalid socket." )

		self.gump.setArgs([self.callback, self.state])
		self.gump.setCallback(WarningGump_onResponse)
		self.gump.send( socket )



def WarningGump_onResponse( player, args, choice ):
	socket = player.socket
	callback = args[0]
	state = args[1]
	if not callback:
		return
	if choice.button == 0 or not socket:
		callback( player, False, state )
	if choice.button == 1:
		callback( player, True, state )


class NoticeGump:

	def __init__(self, header, headerColor, content, contentColor, width, height, callback, state ):
		self.callback = callback
		self.state = state

		self.gump = cGump( 1, 0, 0, (640 - width) / 2, (480 - height) / 2 )
		self.gump.startPage( 0 )
		self.gump.addBackground( 5054, width, height )
		self.gump.addTiledGump( 10, 10, width - 20, 20, 2624 )
		self.gump.addCheckerTrans( 10, 10, width - 20, 20 )
		self.gump.addXmfHtmlGump( 10, 10, width - 20, 20, header, 0, 0, headerColor )
		self.gump.addTiledGump( 10, 40, width - 20, height - 80, 2624 )
		self.gump.addCheckerTrans( 10, 40, width - 20, height - 80 )
		if type( content ) is IntType:
			self.gump.addXmfHtmlGump( 10, 40, width - 20, height - 80, content, 0, 1, contentColor )
		else:
			self.gump.addHtmlGump( 10, 40, width - 20, height - 80, "<BASEFONT COLOR=#%x>%s</BASEFONT>" % ( contentColor, content ), 0, 1 )

		self.gump.addTiledGump( 10, height - 30, width - 20, 20, 2624 )
		self.gump.addCheckerTrans( 10, height - 30, width - 20, 20 )
		self.gump.addButton( 10, height - 30, 4005, 4007, 0 )
		self.gump.addXmfHtmlGump( 40, height - 30, 170, 20, 1011036, 0, 0, 32767 )

	def send( self, char ) :
		# There are two possibilities
		socket = None

		if type( char ).__name__ == "wpchar":
			socket = char.socket
		elif type( char ).__name__ == "wpsocket":
			socket = char
		else:
			raise TypeError( "You passed an invalid socket." )

		self.gump.setArgs([self.callback, self.state])
		self.gump.setCallback(NoticeGump_onResponse)
		self.gump.send( socket )


def NoticeGump_onResponse( player, args, choice ):
	socket = player.socket
	callback = args[0]
	state = args[1]
	if not callback:
		return
	if choice.button == 0 or not socket:
		callback( player, state )
