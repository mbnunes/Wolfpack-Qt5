#===============================================================#
#	 )			(\_ | WOLFPACK 13.0.0 Scripts                   #
#	((		_/{	"-;	| Created by: Radiant						#
#	 )).-' {{ ;'`	| Revised by:                               #
#	( (	;._ \\ ctr  | Last Modification: Created                #
#===============================================================#
# Script for Necromancer spellbooks								#
#===============================================================#

import wolfpack
from magic.spellbook import countspells

def onUse(char, item):
	if item.getoutmostchar() != char:
		char.socket.sysmessage('The book has to be in your belongings to be used.')
		return 1

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
	packet.setshort( 13, 101 ) # Scroll offset (1 = regular, 101 = paladin, 201 = necro)

	for i in range( 0, 2 ):
		if not item.hastag( 'circle' + str( i + 1 ) ):
			packet.setbyte( 15 + i, 0 )
		else:
			packet.setbyte( 15 + i, int( item.gettag( 'circle' + str( i + 1 ) ) ) )

	packet.send( char.socket )
	return 1

def onShowTooltip(viewer, object, tooltip):
	tooltip.add(1042886, str(countspells(object)))
