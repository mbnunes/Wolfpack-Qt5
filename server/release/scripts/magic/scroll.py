#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Created by: DarkStorm                      #
#   )).-' {{ ;'`   | Revised by:                                #
#  ( (  ;._ \\ ctr | Last Modification: Created                 #
#===============================================================#
# Script for Spell Scrolls                                      #
#===============================================================#

from magic.spellbook import addspell
import wolfpack

# calculate the spell-id of a scroll
def calcSpellId( item ):
	tile = wolfpack.tiledata( item.id )
	spell = tile[ "unknown1" ]

	if spell == 0:
		raise

	return ( spell - 1 )

def onUse( char, item ):
	try:
		char.message( 'Casting spell %d' % calcSpellId( item ) )
	except:
		char.socket.sysmessage( 'Broken scroll' )
		return 0

	if item.amount > 1:
		item.amount -= 1
		item.update()
	else:
		item.delete()

	return 1

def onDropOnItem( cont, item ):
	# Who is dragging us?
	char = item.container

	if not char.ischar():
		return 0

	try:
		spell = calcSpellId( item )
	except:
		char.socket.sysmessage( "Error while calculating scroll id." )
		return 0

	if spell >= 64:
		char.socket.sysmessage( "Scroll with invalid spell-id: %d" % spell )
		return 0

	addspell( cont, spell )

	if item.amount > 1:
		item.amount -= 1
		return 0
	else:
		item.delete() # Consume the scroll

	return 1
