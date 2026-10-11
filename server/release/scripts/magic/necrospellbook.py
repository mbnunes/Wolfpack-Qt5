#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Created by: Radiant                        #
#   )).-' {{ ;'`   | Revised by:                                #
#  ( (  ;._ \\ ctr | Last Modification: Created                 #
#===============================================================#
# Necromancy Spellbook                                          #
#===============================================================#
import wolfpack
from math import floor
from wolfpack import tr

def countspells(item):
	count = 0

	for i in range( 1, 4 ):
		if item.hastag('circle' + str(i)):
			spells = int(item.gettag('circle' + str(i)))
			for j in range(0, 9):
				if (spells >> j) & 0x01:
					count += 1
	return count

def hasspell( item, spell ):
	if item and item.hasscript( 'magic.necrospellbook' ):
		spell = spell - 100

		circle = int( floor( spell / 8 ) ) + 1 # 0 for first circle
		spell = spell % 8

		if item.hastag( 'circle' + str( circle ) ):
			spells = int( item.gettag( 'circle' + str( circle ) ) )

			return spells & ( 0x01 << spell )

	return False

def addspell( item, spell ):
	if not item or not item.hasscript( 'magic.necrospellbook' ):
		return 0
	spell = spell - 100
	circle = int( floor( spell / 8 ) ) + 1 # 0 for first circle
	spell = spell % 8
	spells = 0
	if item.hastag( 'circle' + str( circle ) ):
		spells = int( item.gettag( 'circle' + str( circle ) ) )
	spells |= 0x01 << spell
	item.settag( 'circle' + str( circle ), spells )
	item.resendtooltip()

	return True


def onUse(char, item):
	if item.getoutmostchar() != char:
		char.socket.sysmessage(tr('The book has to be in your belongings to be used.'))
		return True

	import magic.utilities
	magic.utilities.open_spellbook( char, item, 101, 3 )
	return True

def onShowTooltip(viewer, object, tooltip):
	tooltip.add(1042886, str(countspells(object)))
