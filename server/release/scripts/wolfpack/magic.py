#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Created by: Correa                         #
#   )).-' {{ ;'`   | Revised by:                                #
#  ( (  ;._ \\ ctr | Last Modification: Created                 #
#===============================================================#
# Main Magic Script												#
#===============================================================#

from wolfpack.consts import *
import wolfpack

# Spell:
# {
#   'name': 'Clumsy',
#   'mantra': 'In Jux Sanct',
#   'cast': function
# }

# Spell Registry
spells = {}

reagents = {
		0xf7a: 'Black Pearls',
		0xf7b: 'Bloodmoss',
		0xf84: 'Garlic',
		0xf85: 'Ginseng',
		0xf86: 'Mandrake',
		0xf88: 'Nightshade',
		0xf8c: 'Sulfurous Ash',
		0xf8d: 'Spider\'s Silk'
}

# Recursive Function for counting reagents
def countReagents( item, items ):
	for key in items.keys():
		if key == item.id and item.color == 0:
			items[ key ] = max( 0, items[ key ] - item.amount )
			return items # Reagents normally dont have content
			
	for subitem in item.content:
		items = countReagents( subitem, items )
		
	return items

# Check if the character has enough reagents to cast the spell
def checkreagents( char, blackpearl=0, bloodmoss=0, garlic=0, ginseng=0, mandrake=0, nightshade=0, spidersilk=0, sulfurash=0 ):
	items = {
		0xf7a: blackpearl,
		0xf7b: bloodmoss,
		0xf84: garlic,
		0xf85: ginseng,
		0xf86: mandrake,
		0xf88: nightshade,
		0xf8c: sulfurash,
		0xf8d: spidersilk
	}	

	items = countReagents( char.getbackpack(), items )
	missing = []
	
	for item in items.keys():
		if items[ item ] > 0:
			missing.append( reagents[ item ] )
			
	if len( missing ) > 0:
		char.socket.sysmessage( 'You lack %s to cast this spell' % ', '.join( missing ) )
		return 0
	else:
		return 1
	
# Check if the character has enough mana to cast the spell
def checkmana( player, mana ):
	if player.mana < mana:
		player.socket.sysmessage( "You lack sufficient mana to cast this spell" )
		return 0
	
	return 1
		
# Register spell information in the global registry		
def registerspell( id, spell ):
	global spells
	
	if spells.has_key( id ):
		return
	
	spells[ id ] = spell

def fizzle( char ):
	# Remove a possible timer/targetrequest
	char.dispel( char, 1, "cast_delay" )

	eventlist = char.events
	eventlist.remove( 'wolfpack.magic' )
	char.events = eventlist

	char.effect( 0x3735, 1, 30 )
	char.soundeffect( 0x5c )

def onLoad():
	wolfpack.registerglobal( HOOK_CHAR, EVENT_CASTSPELL, "wolfpack.magic" )

def onCastSpell( char, spell ):
	if char.frozen:
		if char.socket:
			char.socket.sysmessage( "You can't do that while you are frozen." )
			return

	eventlist = char.events
	
	if 'wolfpack.magic' in eventlist:
		if char.socket:
			char.socket.sysmessage( "You are already casting a spell" )
			return
			
	if not spells.has_key( spell ):
		char.socket.sysmessage( "This spell has not been implemented yet." )
		return
	
	if not spells[ spell ][ 'requirements' ]( char, spell, CAST_BOOK ):
		return
	
	char.say( spells[ spell ][ 'mantra' ] )
	
	char.events =  [ 'wolfpack.magic' ] + eventlist
	char.action( ANIM_CASTAREA )
	char.addtimer( spells[ spell ][ 'delay' ], 'wolfpack.magic.callback', [ spell, CAST_BOOK ], 0, 0, "cast_delay" )
	
def callback( char, args ):
	eventlist = char.events
	eventlist.remove( 'wolfpack.magic' )
	char.events = eventlist

	spell = spells[ args[0] ]
	
	if spell.has_key( 'target' ):
		# Show a target cursor
		char.socket.settag( 'cast_target', 1 )
		char.socket.attachtarget( '' )
	else:
		spells[ args[0] ][ 'callback' ]( char, args[0], args[1] )
	
# These Events happen for characters  who are casting at the moment
def onDamage( char, type, amount, source ):
	fizzle( char )

def onWalk( char, direction, sequence ):
	running = direction & 0x80
	direction &= 0x7F

	# Just turning
	if direction != char.direction:
		return
	
	fizzle( char )
	
def onWarModeToggle( char, warmode ):
	fizzle( char )
	
def onLogin( char ):
	# This only happens if the server crashes or otherwise casting has been canceled
	char.message( "Willkommen" )
	eventlist = char.events
	if 'wolfpack.magic' in eventlist:
		eventlist.remove( 'wolfpack.magic' )
		char.events = eventlist
