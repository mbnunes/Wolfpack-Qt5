
import magic
from wolfpack.consts import *
from magic.utilities import *
import wolfpack.console

# Recursive Function for counting reagents
def countReagents( item, items ):
	for key in items.keys():
		if key == item.id and item.color == 0:
			items[ key ] = max( 0, items[ key ] - item.amount )
			return items # Reagents normally dont have content
			
	for subitem in item.content:
		items = countReagents( subitem, items )
		
	return items
	
# Recursive Function for removing reagents
def consumeReagents( item, items ):
	for ( key, value ) in items.items():
		if key == item.id and item.color == 0:
			if item.amount <= value:
				items[ key ] -= item.amount
				item.delete()
			else:
				item.amount -= value
				item.update()
				del items[key]

			return items # Reagents normally dont have content

	for subitem in item.content:
		items = consumeReagents( subitem, items )
		
	return items

# Basic Spell Class
class Spell:
	mana = 0
	reagents = {}
	validtarget = TARGET_IGNORE
	spellid = None
	circle = None	
	mantra = None
	harmful = 0
	affectdead = 0
	resistable = 0

	# We affect another character
	def affectchar( self, char, mode, target ):
		return 1
	
	def register( self, id ):
		self.spellid = id
		magic.registerspell( self.spellid, self )
	
	def __init__( self, circle ):
		# Set Mana
		self.circle = circle
		mana_table = [ 4, 6, 9, 11, 14, 20, 40, 50 ]
		self.mana = mana_table[ self.circle - 1 ]

	def calcdelay( self ):
		return 250 + ( 250 * self.circle )
	
	def checkrequirements( self, char, mode ):
		if char.dead:
			return 0

		if mode == MODE_BOOK:
			# Check for Mana
			if char.mana < self.mana:
				char.message( 502625 )
				return 0

			# Check for Reagents	
			if len( self.reagents ) > 0:
				items = countReagents( char.getbackpack(), self.reagents.copy() )
	
				for item in items.keys():
					if items[ item ] > 0:
						char.message( 502630 )
						return 0
	
		return 1

	def consumerequirements( self, char, mode ):
		# Check Basic Requirements before proceeding (Includes Death of Caster etc.)
		if not self.checkrequirements( char, mode ):
			fizzle( char )
			return 0
	
		# Consume Mana
		if mode == MODE_BOOK:
			if self.mana != 0:
				char.mana -= self.mana
				char.updatemana()
	
			# Consume Reagents
			if len( self.reagents ) > 0:
				consumeReagents( char.getbackpack(), self.reagents.copy() )
				
		# Check Skill
		# Inspired by RunUO :)
		minskill = max( 0, ( 100 / 7 ) - 20 )
		maxskill = min( 120, ( 100 / 7 ) + 20 )
		
		if not char.checkskill( MAGERY, int( minskill ), int( maxskill ) ):			
			fizzle( char )
			char.message( 502632 )
			return 0
			
		return 1
		
	# Not implemented yet
	def checkreflect( self, char, mode, targettype, target ):
		return 0
		
	def scaledamage( self, char, target, damage ):
		if self.checkresist( char, target ):
			target.message( 501783 )
			damage *= 0.75	
	
		if not target.socket:
			damage *= 2
	
		return damage
		
	def checkresist( self, char, target ):
		return 0
	
	def cast( self, char, mode ):
		if char.socket:	
			char.socket.settag( 'cast_target', 1 )
			char.socket.attachtarget( 'magic.target_response', [ self.spellid, mode ], 'magic.target_cancel', 'magic.target_timeout', 8000 ) # Don't forget the timeout later on
		else:
			# Callback to the NPC AI ??
			wolfpack.console.log( LOG_ERROR, "A NPC is trying to cast a spell." )

	def target( self, char, mode, targettype, target ):
		raise Exception, "Spell without target method: " + str( self.__class__.__name__ )
		
class CharEffectSpell ( Spell ):
	def __init__( self, circle ):
		Spell.__init__( self, circle )
		self.validtarget = TARGET_CHAR
		self.resistable = 1 # Most of them are resistable

	def effect( self, char, target ):
		raise Exception, "CharEffectSpell with unimplemented effect method: " + str( self.__clas__.__name__ )

	def target( self, char, mode, targettype, target ):
		if not self.consumerequirements( char, mode ):
			return	
	
		if not self.affectchar( char, mode, target ):
			return

		char.turnto( target )
		
		if self.checkreflect( char, mode, targettype, target ):
			target = char
	
		self.effect( char, target )

class DelayedDamageSpell( CharEffectSpell ):
	def __init__( self, circle ):
		CharEffectSpell.__init__( self, circle )
		self.validtarget = TARGET_CHAR
		self.harmful = 1 # All of them are harmful
		self.missile = None # If set, a missile will be shot [ id, fixed-direction, explode, speed ]
		self.delay = 1000 # Default Delay
		self.sound = None
		
	def effect( self, char, target ):
		# Shoot a missile?
		if self.missile:
			char.movingeffect( self.missile[0], target, self.missile[1], self.missile[2], self.missile[3] )
	
		if self.sound:
			char.soundeffect( self.sound )
	
		# The damage will be dealt in one second
		if not self.delay:
			self.damage( char, target )
		else:
			target.addtimer( 1000, 'magic.spell.damage_callback', [ self.spellid, char.serial ], 0, 0 )

def damage_callback( target, args ):
	spell = magic.spells[ args[0] ]
	char = wolfpack.findchar( args[1] )
	
	# Something went out of scope
	if not char or not spell:
		wolfpack.console.log( LOG_WARNING, "Either Caster or Spell went out of scope in damage_callback.\n" )
		return	

	spell.damage( char, target )
