#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Created by: DarkStorm                      #
#   )).-' {{ ;'`   | Revised by:                                #
#  ( (  ;._ \\ ctr | Last Modification: Created                 #
#===============================================================#
# Script for Spell Scrolls                                      #
#===============================================================#

import wolfpack
import magic
from magic.utilities import MODE_SCROLL
from magic.spellbook import addspell, hasspell

# calculate the spell-id of a scroll
def calcSpellId( item ):
	tile = wolfpack.tiledata( item.id )
	spell = tile[ "unknown1" ]

	if spell == 0:
		raise

	return ( spell - 1 )

def onUse( char, item ):
	try:
		spell = calcSpellId(item)		
	except:
		char.socket.sysmessage('This scroll seems to be broken.')
		return False

	magic.castSpell(char, spell + 1, MODE_SCROLL, [], None, item)

	return True

def onDropOnItem( cont, item ):
	# Who is dragging us?
	char = item.container

	if not char.ischar():
		return False

	if cont == char.getbackpack() or cont == item.container:
		return False

	try:
		spell = calcSpellId( item )
	except:
		char.socket.sysmessage( "Error while calculating scroll id." )
		return False

	if cont.hasscript( 'magic.spellbook' ):
		if spell >= 64:
			char.socket.sysmessage( "Scroll with invalid spell-id: %d" % spell )
		else:
			if not hasspell( cont, spell ):
				addspell( cont, spell )

				if item.amount > 1:
					item.amount -= 1
					item.update()
				else:
					item.delete() # Consume the scroll
				return False
			else:
				return False

	else:
		return False

	return False
