#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Created by: DarkStorm                      #
#   )).-' {{ ;'`   | Revised by:                                #
#  ( (  ;._ \\ ctr | Last Modification: Created                 #
#===============================================================#
# Recall runes                                                  #
#===============================================================#

import wolfpack
from wolfpack.gumps import cGump
from wolfpack import tr

# This function returns 1 if the tested
# item is a recall rune
def isrune( item ):
	if not item:
		return False
	else:
		if item.hastag( 'location' ) and item.hastag( 'marked' ):
			return True
		else:
			return False

def onUse( char, item ):
	# We are only handling runes
	if not isrune( item ):
		return 0

	# It needs to be on our body
	if item.getoutmostchar() != char:
		char.socket.sysmessage( "The rune needs to be in your posession to rename it." )
		return 1

	# We only can rename marked runes
	if item.gettag( 'marked' ) != 1:
		char.socket.sysmessage( "This rune is not marked." )
		return 1

	gump = cGump( 0, 0, 0, 50, 50 )
	# Header
	gump.addBackground( 0x24a4, 300, 200 )
	gump.addTilePic( 80, 33, 0x1F14 )
	gump.addHtmlGump( 10, 30, 300, 20, '<basefont size="7" color="#336699"><center>Rename rune</center></basefont>' )

	# Rename Field
	gump.addHtmlGump( 40, 60, 300, 20, 'How do you like to name the rune?' )
	gump.addResizeGump( 40, 80, 0x2486, 206, 26 )
	gump.addInputField( 43, 83, 200, 20, 0x539, 1, item.name )

	gump.addButton( 43, 125, 2128, 2129, 1000 ) # Ok
	gump.addButton( 110, 125, 2119, 2120, 0 ) # Cancel

	gump.setCallback( "magic.rune.rename_callback" )
	gump.setArgs( [ item ] )
	gump.send( char )

	return 1

def rename_callback( char, args, response ):
	if( response.button == 0 ):
		return

	item = args[0]

	# Check if we still posess the rune
	if not isrune( item ) or not item.getoutmostchar() == char:
		char.socket.sysmessage( "You cannot rename this rune." )
		return

	# Ready to rename :)
	if len( response.text[1] ) < 1:
		char.socket.sysmessage( "You have to enter a name for this rune." )
		return

	if len( response.text[1] ) > 30:
		char.socket.sysmessage( "The name has to be shorter than 30 characters." )
		return

	item.name = response.text[1]
	item.update()

def onShowTooltip(viewer, object, tooltip):
	marked = 0
	if object.hastag('marked'):
		marked = int(object.gettag('marked'))
	tooltip.reset()

	if marked:
		tooltip.add(1042971, tr("a recall rune for %s") % object.name)
	else:
		tooltip.add(1042971, tr("an unmarked recall rune"))

def onSingleClick( item, char ):
	# We are not a rune
	if not isrune( item ):
		return 0

	# We need a socket to show the speech
	if not char.socket:
		return 0

	# Blank rune
	if item.gettag( 'marked' ) == 0:
		char.socket.showspeech( item, "a Blank recall rune" )
		return 1
	else:
		char.socket.showspeech( item, "Rune to: " + item.name )
		return 1

	# Override the internal name
	return 0

# Identify the item as a rune
def onCreate( item, definition ):
	if not item.hastag('marked'):
		item.settag( 'marked', 0 )

