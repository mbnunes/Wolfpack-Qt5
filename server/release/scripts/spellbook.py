#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Created by: DarkStorm                      #
#   )).-' {{ ;'`   | Revised by:                                #
#  ( (  ;._ \\ ctr | Last Modification: Created                 #
#===============================================================#
# Script for Spellbooks                                         #
#===============================================================#

from math import floor,ceil
import wolfpack

def onLoad():
	# Register .addspell, .removespell, .editspells (?)
	wolfpack.registercommand( "addspell", "spellbook" )
	wolfpack.registercommand( "removespell", "spellbook" )

def onCommand( socket, command, arguments ):
	if command == "ADDSPELL":
		if arguments != 'all':
			try:
				if len( arguments ) < 1:
					raise Exception
	
				spell = int( arguments )
			except:
				socket.sysmessage( 'Usage: addspell <spell> or addspell all' )
				return
				
			if spell >= 64:
				socket.sysmessage( 'Values between 0 and 63 are valid.' )
				return				
		else:
			spell = 'all'

		socket.sysmessage( 'Select the spellbook you want to add the spell to.' )
		socket.attachtarget( "spellbook.addspelltarget", [ spell ] )
		
	elif command == "REMOVESPELL":
		if arguments != 'all':
			try:
				if len( arguments ) < 1:
					raise Exception
	
				spell = int( arguments )
			except:
				socket.sysmessage( 'Usage: removespell <spell> or removespell all' )
				return
				
			if spell >= 64:
				socket.sysmessage( 'Values between 0 and 63 are valid.' )
				return				
		else:
			spell = 'all'
			
		socket.sysmessage( 'Select the spellbook you want to remove the spell from.' )
		socket.attachtarget( "spellbook.removespelltarget", [ spell ] )
		
# Does the Spellbook have a specific spell?
def hasspell( item, spell ):
	if item and 'spellbook' in item.events:
		circle = int( floor( spell / 8 ) ) + 1 # 0 for first circle
		spell = spell % 8
		
		if item.hastag( 'circle' + str( circle ) ):
			spells = int( item.gettag( 'circle' + str( circle ) ) )
			
			return spells & ( 0x01 << spell )		
		
	return 0	

# Adds the specified spell to the specified spellbook
def addspell( item, spell ):
	if not item or not 'spellbook' in item.events:
		return 0

	circle = int( floor( spell / 8 ) ) + 1 # 0 for first circle
	spell = spell % 8
	
	spells = 0
	
	if item.hastag( 'circle' + str( circle ) ):
		spells = int( item.gettag( 'circle' + str( circle ) ) )
		
	spells |= 0x01 << spell
	
	item.settag( 'circle' + str( circle ), spells )
	
	return 1
	
# Removes the specified spell from the specified spellbook
def removespell( item, spell ):
	if not item or not 'spellbook' in item.events:
		return 0

	# Circle
	circle = int( floor( spell / 8 ) ) + 1 # 0 for first circle
	spell = spell % 8
	
	if item.hastag( 'circle' + str( circle ) ):
		spells = int( item.gettag( 'circle' + str( circle ) ) )
		spells &= ~( 0x01 << spell )
		item.settag( 'circle' + str( circle ), spells )
	
	return 1	

"""
	Add a spell to a spellbook
"""		
def addspelltarget( char, args, target ):
	spell = args[0]

	if not target.item or not 'spellbook' in target.item.events:
		char.socket.sysmessage( 'You need to target a spellbook.' )
		return
		
	item = target.item

	if spell == 'all':
		for i in range( 1, 9 ):
			item.settag( 'circle' + str( i ), 0xFF )
			
		char.socket.sysmessage( 'Added all spells to the spellbook.' )			
		return
	
	addspell( item, spell )
	
	circle = int( floor( spell / 8 ) ) + 1
	char.socket.sysmessage( 'Added spell %d of circle %d (Id %d) to the spellbook.' % ( spell % 8 + 1, circle, spell ) )
	
"""
	Remove a spell from a spellbook
"""		
def removespelltarget( char, args, target ):
	spell = args[0]

	if not target.item or not 'spellbook' in target.item.events:
		char.socket.sysmessage( 'You need to target a spellbook.' )
		return
		
	item = target.item

	if spell == 'all':
		for i in range( 1, 9 ):
			item.deltag( 'circle' + str( i ) )
			
		char.socket.sysmessage( 'Removed all spells from the spellbook.' )			
		return
	
	removespell( item, spell )
	
	circle = int( floor( spell / 8 ) ) + 1
	char.socket.sysmessage( 'Removed spell %d of circle %d (Id %d)from the spellbook.' % ( spell % 8 + 1, circle, spell ) )

"""
	open up a spellbook
"""
def onUse( char, item ):
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
	packet.setshort( 13, 1 ) # Scroll offset (1 = regular, 101 = paladin, 201 = necro)
			
	for i in range( 0, 8 ):
		if not item.hastag( 'circle' + str( i + 1 ) ):
			packet.setbyte( 15 + i, 0 ) # Should be unneccesary
		else:
			packet.setbyte( 15 + i, int( item.gettag( 'circle' + str( i + 1 ) ) ) ) # Should be unneccesary

	packet.send( char.socket )

	return 1
