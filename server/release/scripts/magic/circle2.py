
from magic import registerspell
from magic.spell import CharEffectSpell, Spell, DelayedDamageSpell
from magic.utilities import *
import random
import wolfpack
from wolfpack.utilities import tobackpack

class Agility ( CharEffectSpell ):
	def __init__( self ):
		CharEffectSpell.__init__( self, 2 )
		self.reagents = { REAGENT_MANDRAKE: 1, REAGENT_BLOODMOSS: 1 }
		self.mantra = 'Ex Uus'
		
	def effect( self, char, target ):
		statmodifier( char, target, 1, 0 )
				
		target.effect( 0x375a, 10, 15 )
		target.soundeffect( 0x28e )		
		
class Cunning ( CharEffectSpell ):
	def __init__( self ):
		CharEffectSpell.__init__( self, 2 )
		self.reagents = { REAGENT_MANDRAKE: 1, REAGENT_NIGHTSHADE: 1 }
		self.mantra = 'Uus Wis'
		
	def effect( self, char, target ):
		statmodifier( char, target, 2, 0 )
				
		target.effect( 0x375a, 10, 15 )
		target.soundeffect( 0x1eb )				

class Strength ( CharEffectSpell ):
	def __init__( self ):
		CharEffectSpell.__init__( self, 2 )
		self.reagents = { REAGENT_MANDRAKE: 1, REAGENT_NIGHTSHADE: 1 }
		self.mantra = 'Uus Mani'
		
	def effect( self, char, target ):
		statmodifier( char, target, 0, 0 )
				
		target.effect( 0x375a, 10, 15 )
		target.soundeffect( 0x1ee )		

class Harm ( DelayedDamageSpell ):
	def __init__( self ):
		DelayedDamageSpell.__init__( self, 2 )
		self.delay = None
		self.reagents = { REAGENT_SPIDERSILK: 1, REAGENT_NIGHTSHADE: 1 }
		self.mantra = 'An Mani'
		self.sound = 0x1f1
		
	def damage( self, char, target ):
		distance = target.distanceto( char )
		
		char.message( 'Distance to target: ' + str( distance ) )
		
		damage = random.randint( 1, 15 )
		
		# One field between us and the target
		if distance == 2:
			damage *= 0.5
			
		elif distance > 2:
			damage *= 0.25
		
		damage = self.scaledamage( char, target, int( damage ) )
		target.effect( 0x374a, 10, 15 )
		target.damage( DAMAGE_MAGICAL, damage, char )

class MagicTrap ( Spell ):
	def __init__( self ):
		Spell.__init__( self, 2 )
		self.validtarget = TARGET_ITEM
		self.reagents = { REAGENT_GARLIC: 1, REAGENT_SPIDERSILK: 1, REAGENT_SULFURASH: 1 }
		self.mantra = 'In Jux'
		
	def target( self, char, mode, targettype, target ):
		char.turnto( target )
	
		if not self.consumerequirements( char, mode ):
			return

		# Can this be trapped?
		if target.type != 1:
			char.message( 'You cannot trap that.' )			
		
		# Already Trapped?
		if 'magic.trap' in target.events:
			fizzle( char )
			return

		pos = target.pos		
		wolfpack.effect( 0x376a, wolfpack.coord( pos.x + 1, pos.y, pos.z, pos.map ), 9, 10 )
		wolfpack.effect( 0x376a, wolfpack.coord( pos.x, pos.y - 1, pos.z, pos.map ), 9, 10 )
		wolfpack.effect( 0x376a, wolfpack.coord( pos.x - 1, pos.y, pos.z, pos.map ), 9, 10 )
		wolfpack.effect( 0x376a, wolfpack.coord( pos.x, pos.y + 1, pos.z, pos.map ), 9, 10 )
		target.soundeffect( 0x1ef )
		
		# Add the Trap Properties to the Item
		target.settag( 'trap_owner', char.serial )
		target.settag( 'trap_damage', random.randint( 10, 50 ) )
		target.settag( 'trap_type', 'magic' )
		target.events = target.events + [ 'magic.trap' ]
		
class RemoveTrap ( Spell ):
	def __init__( self ):
		Spell.__init__( self, 2 )
		self.validtarget = TARGET_ITEM
		self.reagents = { REAGENT_BLOODMOSS: 1, REAGENT_SULFURASH: 1 }
		self.mantra = 'An Jux'
		
	def target( self, char, mode, targettype, target ):
		char.turnto( target )
	
		if not self.consumerequirements( char, mode ):
			return
	
		# Already Trapped?
		if not'magic.trap' in target.events:
			fizzle( char )
			return

		wolfpack.effect( 0x376a, target.pos, 9, 32 )
		target.soundeffect( 0x1f0 )
		
		# Add the Trap Properties to the Item
		target.deltag( 'trap_owner', char.serial )
		target.deltag( 'trap_damage', random.randint( 10, 50 ) )
		target.deltag( 'trap_type', 'magic' )
		events = target.events
		events.remove( 'magic.trap' )
		target.events = events

def onLoad():
	Agility().register( 9 )
	Cunning().register( 10 )
	#Cure().register( 11 )
	Harm().register( 12 )
	MagicTrap().register( 13 )
	RemoveTrap().register( 14 )
	#Protection().register( 15 )
	Strength().register( 16 )
