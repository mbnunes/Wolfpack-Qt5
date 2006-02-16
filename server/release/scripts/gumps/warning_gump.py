from wolfpack.gumps import cGump
from types import *

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
