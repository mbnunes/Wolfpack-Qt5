#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Created by: DarkStorm                      #
#   )).-' {{ ;'`   | Revised by:                                #
#  ( (  ;._ \\ ctr | Last Modification: Created                 #
#===============================================================#
# Recall runes                                                  #
#===============================================================#

# This function returns 1 if the tested
# item is a recall rune
def isrune( item ):
	if not item:
		return 0

	if item.hastag( 'charges' ) and item.hastag( 'marked' ):
		return 1
	else:
		return 0

def onUse( char, item ):
	# We are only handling runes
	if not isrune( item ):
		return 0

	char.message( "renaming recall runes is not supported right now" )
	return 1

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

# Make the item a rune
def onCreate( item, definition ):
	item.settag( 'marked', 0 )
	item.settag( 'charges', 0 )
