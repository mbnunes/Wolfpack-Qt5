#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Created by: DarkStorm                      #
#   )).-' {{ ;'`   | Revised by:                                #
#  ( (  ;._ \\ ctr | Last Modification: Created                 #
#===============================================================#
# Speech Script for Barbers                                     #
#===============================================================#

from wolfpack.consts import *
from wolfpack.gumps import cGump
import wolfpack

prices = {
			# Remove
			0x0000: 50000,

			# Hair
			0x203b: 50000,
			0x203c: 50000,
			0x203d: 50000,
			0x2044: 50000,
			0x2045: 50000,
			0x2046: 50000,
			0x2047: 50000,
			0x2048: 50000,
			0x2049: 50000,
			0x204a: 50000,

			# Beards
			0x203E: 50000,
			0x203F: 50000,
			0x2040: 50000,
			0x2041: 50000,
			0x204B: 50000,
			0x204C: 50000,
			0x204D: 50000
		}
		
def onSpeech( listener, speaker, text, keywords ):
	# Check if our name is in the beginning of the string
	if not text.lower().startswith( listener.name.lower() ):
		return 0

	if speaker.id != 0x190 and speaker.id != 0x191:
		listener.say( "I can't cat your hair!",5 )
		return 1

	# We only have one keyword
	gump = cGump( 0, 0, 0, 50, 50 )
	gump.addBackground( 0x24a4, 425, 400 )
	gump.startPage( 0 )
	gump.addHtmlGump( 10, 30, 450, 20, '<basefont size="7" color="#336699"><center>Barber shop</center></basefont>' )
	gump.addTilePic( 155, 31, 0xDFD )

	# Page Switches (only for men)
	if speaker.id == 0x190:
		gump.addPageButton( 40, 61, 0x4B9, 0x4BA, 0x11 )
		gump.addHtmlGump( 60, 60, 100, 20, '<basefont color="#333333"><u>Hair</u></basefont>' )

		gump.addPageButton( 190, 61, 0x4B9, 0x4BA, 0x21 )
		gump.addHtmlGump( 210, 60, 100, 20, '<basefont color="#333333"><u>Beard</u></basefont>' )

	# Hair
	addHairPage( speaker, gump, 0x11, 0x203B ) # Short Hair
	addHairPage( speaker, gump, 0x12, 0x203C ) # Long Hair
	addHairPage( speaker, gump, 0x13, 0x203D ) # Ponytail
	addHairPage( speaker, gump, 0x14, 0x2044 ) # Mohawk
	addHairPage( speaker, gump, 0x15, 0x2045 ) # Pageboy
	addHairPage( speaker, gump, 0x16, 0x2046 ) # Buns hair
	addHairPage( speaker, gump, 0x17, 0x2047 ) # Afro
	addHairPage( speaker, gump, 0x18, 0x2048 ) # Receding hair
	addHairPage( speaker, gump, 0x19, 0x2049 ) # 2 pig-tails
	addHairPage( speaker, gump, 0x1A, 0x204a ) # Krisna Hair
	addHairPage( speaker, gump, 0x1B, 0x0000 ) # None

	if speaker.id == 0x190:
		# Beard
		gump.startPage( 0x21 )

		# BEARDS
		addBeardPage( speaker, gump, 0x21, 0x203E ) # Long Beard
		addBeardPage( speaker, gump, 0x22, 0x203F ) # Short Beard
		addBeardPage( speaker, gump, 0x23, 0x2040 ) # Goatee
		addBeardPage( speaker, gump, 0x24, 0x2041 ) # Mustache
		addBeardPage( speaker, gump, 0x25, 0x204B ) # Med Short Beard
		addBeardPage( speaker, gump, 0x26, 0x204C ) # Med Long Beard
		addBeardPage( speaker, gump, 0x27, 0x204D ) # Vandyke
		addBeardPage( speaker, gump, 0x28, 0x0000 ) # None

	gump.startPage( 0 )
	gump.addButton( 350, 335, 2119, 2120, 0 ) # Cancel

	gump.setCallback( "speech.barber.gump_callback" )
	gump.setArgs( [ listener.serial ] )
	gump.send( speaker )

	return 1

def addHairPage( char, gump, page, id ):
	gump.startPage( page )
	addHairButtons( gump, page & 0x000F )
	gump.addResizeGump( 200, 95, 0x2486, 190, 225 )

	haircolor = getHairColor( char )

	# Add a colored base body
	if char.id == 0x190:
		gump.addGump( 200, 60, 0xC, char.skin )
		gump.addGump( 200, 60, 0xC503 ) # Fancy Shirt
		gump.addGump( 200, 60, 0xC4FF ) # Long Pants
		gump.addGump( 200, 60, 0xC52D ) # Boots
		if id != 0:
			gump.addGump( 200, 60, getHairGump( id ), haircolor )

		beard = char.itemonlayer( LAYER_BEARD )
		if beard:
			gumpid = getBeardGump( beard.id )
			if gumpid != 0:
				gump.addGump( 200, 60, gumpid, beard.color )
	else:
		gump.addGump( 200, 60, 0xD, char.skin )
		gump.addGump( 200, 60, 0xEC3D ) # Boots
		gump.addGump( 200, 60, 0xC511 ) # Skirt
		gump.addGump( 200, 60, 0xEC13 ) # Fancy Shirt

		if id != 0:
			gump.addGump( 200, 60, getHairGump( id )+10000, haircolor )
	
	# Ok button for this page
	gump.addButton( 208, 292, 0x850, 0x851, id | 0x4000 ) # 0x4000 = Hair
	gump.addHtmlGump( 275, 292, 150, 20, '<basefont color="#336699"><u>Price:</u><basefont color="#333333"> %i gold</basefont></basefont>' % prices[ id ] )

# Detail page for a certain beard
def addBeardPage( char, gump, page, id ):
	gump.startPage( page )
	addBeardButtons( gump, page & 0x000F )
	gump.addResizeGump( 200, 95, 0x2486, 190, 225 )

	beardcolor = getBeardColor( char )

	# Add a colored base body
	gump.addGump( 200, 60, 0xC, char.skin )
	gump.addGump( 200, 60, 0xC503 ) # Fancy Shirt
	gump.addGump( 200, 60, 0xC4FF ) # Long Pants
	gump.addGump( 200, 60, 0xC52D ) # Boots
	if id != 0:
		gump.addGump( 200, 60, getBeardGump( id ), beardcolor )

	hair = char.itemonlayer( LAYER_HAIR )
	if hair:
		gumpid = getHairGump( hair.id )
		if gumpid != 0:
			gump.addGump( 200, 60, gumpid, hair.color )

	# Ok button for this page
	gump.addButton( 208, 292, 0x850, 0x851, id | 0x8000 ) # 0x8000 = Beard
	gump.addHtmlGump( 275, 292, 150, 20, '<basefont color="#336699"><u>Price:</u><basefont color="#333333"> %i gold</basefont></basefont>' % prices[ id ] )

# Get a valid hair color
def getHairColor( char ):
	hair = char.itemonlayer( LAYER_HAIR )
	if hair:
		return hair.color

	beard = char.itemonlayer( LAYER_BEARD )
	if beard:
		return beard.color	

	return 0

# Get a valid hair color
def getBeardColor( char ):
	beard = char.itemonlayer( LAYER_BEARD )
	if beard:
		return beard.color	

	hair = char.itemonlayer( LAYER_HAIR )
	if hair:
		return hair.color

	return 0

# Buttons for Hair Types
def addHairButtons( gump, active=0 ):
	if active == 1:
		gump.addGump( 40, 101, 0x4BA )
	else:
		gump.addPageButton( 40, 101, 0x4B9, 0x4BA, 0x11 )
	gump.addHtmlGump( 60, 100, 100, 20, 'Short Hair' )

	if active == 2:
		gump.addGump( 40, 121, 0x4BA )
	else:
		gump.addPageButton( 40, 121, 0x4B9, 0x4BA, 0x12 )
	gump.addHtmlGump( 60, 120, 100, 20, 'Long Hair' )

	if active == 3:
		gump.addGump( 40, 141, 0x4BA )
	else:
		gump.addPageButton( 40, 141, 0x4B9, 0x4BA, 0x13 )
	gump.addHtmlGump( 60, 140, 100, 20, 'Ponytail' )

	if active == 4:
		gump.addGump( 40, 161, 0x4BA )
	else:
		gump.addPageButton( 40, 161, 0x4B9, 0x4BA, 0x14 )
	gump.addHtmlGump( 60, 160, 100, 20, 'Mohawk' )

	if active == 5:
		gump.addGump( 40, 181, 0x4BA )
	else:
		gump.addPageButton( 40, 181, 0x4B9, 0x4BA, 0x15 )
	gump.addHtmlGump( 60, 180, 100, 20, 'Pageboy' )

	if active == 6:
		gump.addGump( 40, 201, 0x4BA )
	else:
		gump.addPageButton( 40, 201, 0x4B9, 0x4BA, 0x16 )
	gump.addHtmlGump( 60, 200, 100, 20, 'Buns hair' )

	if active == 7:
		gump.addGump( 40, 221, 0x4BA )
	else:
		gump.addPageButton( 40, 221, 0x4B9, 0x4BA, 0x17 )
	gump.addHtmlGump( 60, 220, 100, 20, 'Afro' )

	if active == 8:
		gump.addGump( 40, 241, 0x4BA )
	else:
		gump.addPageButton( 40, 241, 0x4B9, 0x4BA, 0x18 )
	gump.addHtmlGump( 60, 240, 100, 20, 'Receding hair' )

	if active == 9:
		gump.addGump( 40, 261, 0x4BA )
	else:
		gump.addPageButton( 40, 261, 0x4B9, 0x4BA, 0x19 )
	gump.addHtmlGump( 60, 260, 100, 20, '2 pig-tails' )

	if active == 10:
		gump.addGump( 40, 281, 0x4BA )
	else:
		gump.addPageButton( 40, 281, 0x4B9, 0x4BA, 0x1A )
	gump.addHtmlGump( 60, 280, 100, 20, 'Krisna Hair' )

	if active == 11:
		gump.addGump( 40, 301, 0x4BA )
	else:
		gump.addPageButton( 40, 301, 0x4B9, 0x4BA, 0x1B )
	gump.addHtmlGump( 60, 300, 100, 20, 'None' )

# Buttons for Beard Types
def addBeardButtons( gump, active=0 ):
	if active == 1:
		gump.addGump( 40, 101, 0x4BA )
	else:
		gump.addPageButton( 40, 101, 0x4B9, 0x4BA, 0x21 )
	gump.addHtmlGump( 60, 100, 100, 20, 'Long Beard' )

	if active == 2:
		gump.addGump( 40, 121, 0x4BA )
	else:
		gump.addPageButton( 40, 121, 0x4B9, 0x4BA, 0x22 )
	gump.addHtmlGump( 60, 120, 100, 20, 'Short Beard' )

	if active == 3:
		gump.addGump( 40, 141, 0x4BA )
	else:
		gump.addPageButton( 40, 141, 0x4B9, 0x4BA, 0x23 )
	gump.addHtmlGump( 60, 140, 100, 20, 'Goatee' )

	if active == 4:
		gump.addGump( 40, 161, 0x4BA )
	else:
		gump.addPageButton( 40, 161, 0x4B9, 0x4BA, 0x24 )
	gump.addHtmlGump( 60, 160, 100, 20, 'Mustache' )

	if active == 5:
		gump.addGump( 40, 181, 0x4BA )
	else:
		gump.addPageButton( 40, 181, 0x4B9, 0x4BA, 0x25 )
	gump.addHtmlGump( 60, 180, 100, 20, 'Med Short Beard' )

	if active == 6:
		gump.addGump( 40, 201, 0x4BA )
	else:
		gump.addPageButton( 40, 201, 0x4B9, 0x4BA, 0x26 )
	gump.addHtmlGump( 60, 200, 100, 20, 'Med Long Beard' )

	if active == 7:
		gump.addGump( 40, 221, 0x4BA )
	else:
		gump.addPageButton( 40, 221, 0x4B9, 0x4BA, 0x27 )
	gump.addHtmlGump( 60, 220, 100, 20, 'Vandyke' )

	if active == 8:
		gump.addGump( 40, 241, 0x4BA )
	else:
		gump.addPageButton( 40, 241, 0x4B9, 0x4BA, 0x28 )
	gump.addHtmlGump( 60, 240, 100, 20, 'None' )

beards = {
		0x203E: 0xC671,
		0x203F: 0xC672,
		0x2040: 0xC670,
		0x2041: 0xC673,
		0x204B: 0xC6D8,
		0x204C: 0xC6D9,
		0x204D: 0xC6DA
		}

def getBeardGump( id ):
	if beards.has_key( id ):
		return beards[ id ]
	else:
		return 0

hairs = {
		0x203B: 0xC60C,
		0x203C: 0xC60D,
		0x203D: 0xC60E,
		0x2044: 0xC60F,
		0x2045: 0xC616,
		0x2046: 0xC618,
		0x2047: 0xC6D4,
		0x2048: 0xC6D5,
		0x2049: 0xC6D6,
		0x204A: 0xC6D7
		}

def getHairGump( id ):
	if hairs.has_key( id ):
		return hairs[ id ]
	else:
		return 0

### Callback
def gump_callback( char, args, response ):
	if response.button == 0:
		return

	# Check if the vendor is ok
	vendor = wolfpack.findchar( args[0] )

	if not vendor:
		return

	if vendor.distanceto( char ) > 5:
		vendor.say( "I can't cut your hair from over there!",5 )
		char.socket.sysmessage( "You can't reach the vendor." )
		return

	id = response.button & 0x3FFF
	
	if not prices.has_key( id ):
		vendor.say( "I can't cut your hair like this!",5 )
		return

	price = prices[ id ]

	if char.countresource( 0xeed, 0 ) < price:
		vendor.say( "You don't have enoug gold!",5 )
		return

	char.useresource( price, 0xeed, 0 )
	char.soundeffect( 0x37, 0 )
	vendor.say( "That is %i gold." % price,5 )

	color = 0

	# Hair
	if response.button & 0x4000:
		# Remove old hair
		color = getHairColor( char )
		hair = char.itemonlayer( LAYER_HAIR )
		if hair:
			hair.delete()

	# Beard
	elif response.button & 0x8000:
		if char.id == 0x191:
			char.socket.sysmessage( "You can't have a beard" )
			return

		# Remove old beard
		color = getBeardColor( char )
		beard = char.itemonlayer( LAYER_BEARD )
		if beard:
			beard.delete()		

	else:
		return

	# None
	if id == 0:
		return

	item = wolfpack.additem( "%x" % id )
	if not item:
		print "Invalid defintion: %x\n" % id
		return

	item.color = color
	char.equip( item )
