#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Created by: DarkStorm                      #
#   )).-' {{ ;'`   | Revised by:                                #
#  ( (  ;._ \\ ctr | Last Modification: Created                 #
#===============================================================#
# Python class for gumps.    									#
#===============================================================#

from types import *

class cGump:
	def __init__(self, noclose=0, nomove=0, nodispose=0, x=0, y=0):
		self.layout = []
		self.texts = []
		self.args = []
		self.callback = ""
		self.noclose = noclose
		self.nomove = nomove
		self.nodispose = nodispose
		self.x = x
		self.y = y
		self.typeid = 0
		self.serialid = 0

	# Send the gump
	def send( self, char ):
		# There are two possibilities
		socket = None

		if type( char ).__name__ == "wpchar":
			socket = char.socket
		elif type( char ).__name__ == "wpsocket":
			socket = char

		# Dump the gump data
		#file = open( 'dump.txt', 'w' )
		#file.write( "--------\nLayout:\n" )
		#file.writelines( self.layout )
		#file.write( "--------\nText:\n" )
		#file.writelines( self.texts )
		#file.close()

		if not socket:
			raise TypeError( "You passed an invalid socket." )
		else:
			socket.sendgump( self.x, self.y, self.nomove, self.noclose, self.nodispose, self.serialid, self.typeid, self.layout, self.texts, self.callback, self.args )

	# For "rawly" modifying the list
	def addRawLayout( self, data ):
		self.layout.append( data )

	def addRawText( self, data ):
		self.texts.append( data )
		return len( self.texts ) - 1

	# Sets the Callback function which is going to be called whenever the user
	# clicks something on the gump
	def setCallback( self, callback ):
		if not type( callback ) is StringType:
			raise TypeError( "You have to pass a string to setCallback" )
		else:
			self.callback = callback

	# Set the arguments you want to pass to the gump-response handler later on
	def setArgs( self, args ):
		if not type( args ) is ListType:
			raise TypeError( "You have to pass a list to setArgs" )
		else:
			self.args = args

	# Set the Gump-Type (this can be used to close it later on)
	def setType( self, typeid ):
		if not type( typeid ) is IntType:
			raise TypeError( "You have to pass an integer to setType" )
		else:
			self.typeid = typeid

	# Set the Gump-Serial (this can be used to only open one gump of the same type at the same moment)
	# This will auto-close the first gump
	# 0 means the Gump will automatically choose a serial
	def setSerial( self, serialid ):
		if not type( serialid ) is IntType:
			raise TypeError( "You have to pass an integer to setSerial" )
		else:
			self.serialid = serialid

	### Here are the helper functions for adding the layout elements
	def startPage( self, page ):
		self.layout.append( "{page %u}" % page )

	def startGroup( self, groupid ):
		self.layout.append( "{group %u}" % groupid )

	def addText( self, x, y, text, hue=0 ):
		self.layout.append( "{text %i %i %u %u}" % ( x, y, hue, self.addRawText( text ) ) )

	def addBackground( self, id, width, height ):
		self.addResizeGump( 0, 0, id, width, height )

	def addResizeGump( self, x, y, id, width, height ):
		self.layout.append( "{resizepic %i %i %u %u %u}" % ( x, y, id, width, height ) )

	def addCroppedText( self, x, y, width, height, text, hue=0 ):
		self.layout.append( "{croppedtext %i %i %u %u %u %u}" % ( x, y, width, height, hue, self.addRawText( text ) ) )

	def addButton( self, x, y, up, down, returncode ):
		self.layout.append( "{button %i %i %u %u 1 0 %u}" % ( x, y, up, down, returncode ) )

	def addPageButton( self, x, y, up, down, page ):	
		self.layout.append( "{button %i %i %u %u 0 %u 0}" % ( x, y, up, down, page ) )

	def addGump( self, x, y, id, hue = -1 ):
		# A non-colored gump
		if hue == -1:
			self.layout.append( "{gumppic %i %i %u}" % ( x, y, id ) )
		else:
			self.layout.append( "{gumppic %i %i %u hue=%u}" % ( x, y, id, hue ) )

	def addTiledGump( self, x, y, width, height, id, hue = 0 ):
		if hue == -1:
			self.layout.append( "{gumppictiled %i %i %u %u %u}" % ( x, y, width, height, id ) )
		else:
			self.layout.append( "{gumppictiled %i %i %u %u %u hue=%u}" % ( x, y, width, height, id, hue ) )

	# Sadly it's not possible to hue this
	def addTilePic( self, x, y, id ):
		self.layout.append( "{tilepic %i %i %u}" % ( x, y, id ) )

	def addInputField( self, x, y, width, height, hue, id, starttext ):
		self.layout.append( "{textentry %i %i %u %u %u %u %u}" % ( x, y, width, height, hue, id, self.addRawText( starttext ) ) )

	def addCheckbox( self, x, y, off, on, id, checked = 0 ):
		# Just to prevent errors
		if( checked != 0 ):
			checked = 1

		self.layout.append( "{checkbox %i %i %u %u %u %u}" % ( x, y, off, on, checked, id ) )

	def addRadioButton( self, x, y, off, on, id, selected = 0 ):
		# Just to prevent errors
		if( selected != 0 ):
			selected = 1

		self.layout.append( "{radio %i %i %u %u %u %u}" % ( x, y, off, on, selected, id ) )

	def addHtmlGump( self, x, y, width, height, html, hasBack = 0, canScroll = 0 ):
		if( canScroll != 0 ):
			canScroll = 1
		if( hasBack != 0 ):
			hasBack = 1

		self.layout.append( "{htmlgump %i %i %u %u %u %u %u}" % ( x, y, width, height, self.addRawText( html ), hasBack, canScroll ) )

	def addXmfHtmlGump( self, x, y, width, height, clilocid, hasBack = 0, canScroll = 0 ):
		if( canScroll != 0 ):
			canScroll = 1
		if( hasBack != 0 ):
			hasBack = 1

		self.layout.append( "{xmfhtmlgump %i %i %u %u %u %u %u}" % ( x, y, width, height, clilocid, hasBack, canScroll ) )

	def addCheckerTrans( self, x, y, width, height ):
		self.layout.append( "{checkertrans %i %i %u %u}" % ( x, y, width, height ) )
