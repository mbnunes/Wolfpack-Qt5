import struct
import _wolfpack

def parseXML( filename ):
	return _wolfpack.parsexmldocument( filename )

class gump:
	def __init__( self ):
		self.x = 100
		self.y = 100
		self.serial = 0
		self.type = 0
		self.commands = []
		self.texts = []
		self.noclose = 0
		self.nomove = 0
		self.nodispose = 0
	
	# Adds a background to the gump
	def addBackground( self, x, y, id, width, height ):
		self.addRawCommand( "resizepic %i %i %i %i %i" % ( x, y, id, width, height ) )
		
	def addPage( self, page ):
		self.addRawCommand( "page " + str( page ) )#
		
	def addGump( self, x, y, id ):
		self.addRawCommand( "gumppic %i %i %i" % ( x, y, id ) )
		
	#UI16 x, UI16 y, UI16 ImageUp, UI16 ImageDown, UI16 Behaviour, UI16 Page, UI32 UniqueID
	# Page = the page it is on
	# UniqueID = the page it leads to IF behaviour == 0
	def addButton( self, x, y, imageUp, imageDown, type, page, uniqueid ):
		self.addRawCommand( "button %i %i %i %i %i %i %i" % ( x, y, imageUp, imageDown, type, page, uniqueid ) )
		
	def addText( self, x, y, hue, text ):
		for i in range( len( self.texts ) ):
			if self.texts[ i ] == text:
				self.addRawCommand( "text %i %i %i %i" % ( x, y, hue, i ) )
				return
				
		self.texts.append( text )
		self.addRawCommand( "text %i %i %i %i" % ( x, y, hue, len( self.texts ) - 1 ) )

	# Method for adding a raw Command
	def addRawCommand( self, command ):
		self.commands.append( "{ " + command + " }" )
	
	# Method for adding a raw Text-line
	def addRawText( self, text ):
		self.texts.append( text )
	
	def send( self, target ):
		# Send Gump Menu Dialog (Variable # of bytes) 
		# BYTE cmd 
		# BYTE[2] blockSize
		# BYTE[4] id
		# BYTE[4] gumpid
		# BYTE[4] x
		# BYTE[4] y
		# BYTE[2] command section length
		# 	BYTE[?] commands (zero terminated)
		# BYTE[2] numTextLines
		# 	BYTE[2] text length (in unicode (2 byte) characters.)
		# 	BYTE[?] text (in unicode)

		pklen = 24
		commlen = 1
		
		if self.nomove == 1:
			pklen += 10
			commlen += 10

		if self.noclose == 1:
			pklen += 11
			commlen += 11

		if self.nodispose == 1:
			pklen += 13
			commlen += 13

		for command in self.commands:
			pklen += len( command )
			commlen += len( command )
			
		for text in self.texts:
			pklen += 2 + ( len( text )*2 ) # Unicode
		
		# Network byte-order
		gumpPacket = struct.pack( "!BHLLLLH", 0xB0, pklen, self.serial, self.type, self.x, self.y, commlen )
		
		## Now for the commands
		if self.noclose == 1:
			gumpPacket += "{ noclose }"
			
		if self.nomove == 1:
			gumpPacket += "{ nomove }"
			
		if self.nodispose == 1:
			gumpPacket += "{ nodispose }"
		
		for command in self.commands:
			gumpPacket += command

		# A null-terminated string for ALL layout stuff
		gumpPacket += struct.pack( "!B", 0x00 )					
		gumpPacket += struct.pack( "!H", len( self.texts ) )
		
		for text in self.texts:
			gumpPacket += struct.pack( "!H", len( text ) * 2 )
			gumpPacket += unicode( text )

		target.send( gumpPacket, pklen )
