#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Created by: Naddel                         #
#   )).-' {{ ;'`   | Revised by:                                #
#  ( (  ;._ \\ ctr | Last Modification: Created                 #
#===============================================================#
# Bushido Spellbook                                             #
#===============================================================#
import wolfpack
from math import floor
from wolfpack import tr

def countspells(item):
	count = 0

	for i in range( 1, 2 ):
		if item.hastag('circle' + str(i)):
			spells = int(item.gettag('circle' + str(i)))
			for j in range(0, 9):
				if (spells >> j) & 0x01:
					count += 1
	return count

def hasspell( item, spell ):
	if item and item.hasscript( 'magic.bushidospellbook' ):
		spell = spell - 400

		circle = int( floor( spell / 8 ) ) + 1 # 0 for first circle
		spell = spell % 8

		if item.hastag( 'circle' + str( circle ) ):
			spells = int( item.gettag( 'circle' + str( circle ) ) )

			return spells & ( 0x01 << spell )

	return False

def addspell( item, spell ):
	if not item or not item.hasscript( 'magic.bushidospellbook' ):
		return 0

	spell = spell - 400
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

	# This is annoying and eats bandwith but its the only way to "reopen" the spellbook
	# once its already open.
	#char.socket.removeobject(item)
	if item.container and item.container.isitem():
		char.socket.sendobject(item.container)
	char.socket.sendobject(item)

	packet = wolfpack.packet( 0x24, 7 )
	packet.setint( 1, item.serial )
	packet.setshort( 5, 0xffff )
	packet.send( char.socket )

	packet = wolfpack.packet( 0xbf, 23 )
	packet.setshort( 1, 23 )	 # Packet length
	packet.setshort( 3, 0x1b )	 # 0xbf subcommand
	packet.setshort( 5, 1	 )	 # Unknown. Maybe it's a subsubcommand ?
	packet.setint( 7, item.serial ) # Spellbook serial
	packet.setshort( 11, item.id ) # Item id
	packet.setshort( 13, 401 ) # Scroll offset (1 = regular, 101 = necro, 201 = paladin, 400 = bushido, 601 = arcanist)

	for i in range( 0, 1 ):
		if not item.hastag( 'circle' + str( i + 1 ) ):
			packet.setbyte( 15 + i, 0 )
		else:
			packet.setbyte( 15 + i, int( item.gettag( 'circle' + str( i + 1 ) ) ) )

	packet.send( char.socket )
	return True

def onShowTooltip(viewer, object, tooltip):
	tooltip.add(1042886, str(countspells(object)))
