
from magic import registerspell
from magic.spell import CharEffectSpell, Spell, DelayedDamageSpell
from magic.utilities import *
import random
import wolfpack
from wolfpack.utilities import tobackpack

class Bless ( CharEffectSpell ):
	def __init__( self ):
		CharEffectSpell.__init__( self, 3 )
		self.reagents = { REAGENT_MANDRAKE: 1, REAGENT_GARLIC: 1 }
		self.mantra = 'Rel Sanct'
		
	def effect( self, char, target ):
		statmodifier( char, target, 3, 0 )
				
		target.effect( 0x374a, 10, 15 )
		target.soundeffect( 0x1ea )

class Fireball ( DelayedDamageSpell ):
	def __init__( self ):
		DelayedDamageSpell.__init__( self, 3 )
		self.reagents = { REAGENT_BLACKPEARL: 1 }
		self.mantra = 'Vas Flam'
		self.sound = 0x44b
		self.missile = [ 0x36D4, 0, 1, 7, 0 ]
		
	def damage( self, char, target ):
		damage = self.scaledamage( char, target, random.randint( 10, 17 ) )
		target.damage( DAMAGE_MAGICAL, damage, char )

# Until containers are implemented in python,
# this spell is a security risk
#class Telekinesis( Spell ):
#	def __init__( self ):
#		Spell.__init__( self, 3 )
#		self.reagents = { REAGENT_GARLIC: 1, REAGENT_SPIDERSILK: 1, REAGENT_SULFURASH: 1 }
#		self.mantra = 'Ort Por Ylem'
#		self.validtarget = TARGET_ITEM
#		
#	def target( self, char, mode, targettype, target ):

class WallOfStone( Spell ):
	def __init__( self ):
		Spell.__init__( self, 3 )
		self.reagents = { REAGENT_BLOODMOSS: 1, REAGENT_GARLIC: 1 }
		self.mantra = 'In Sanct Ylem'
		self.validtarget = TARGET_GROUND
		
	def target( self, char, mode, targettype, target ):	
		char.turnto( target )
		
		if not char.cansee(target):
			char.message( 500237 )
			return
			
		if not self.consumerequirements( char, mode ):
			return
		
		xdiff = abs( target.x - char.pos.x )
		ydiff = abs( target.y - char.pos.y )
		
		if xdiff > ydiff:
			positions = [ wolfpack.coord( target.x, target.y - 1, target.z, target.map ), wolfpack.coord( target.x, target.y, target.z, target.map ), wolfpack.coord( target.x, target.y + 1, target.z, target.map ) ]
		else:
			positions = [ wolfpack.coord( target.x - 1, target.y, target.z, target.map ), wolfpack.coord( target.x, target.y, target.z, target.map ), wolfpack.coord( target.x + 1, target.y, target.z, target.map ) ]
		
		# At least one spell should look fancy
		# Calculate the Angle here
		serials = []
		
		char.soundeffect( 0x1f6 )
		
		for pos in positions:
			newitem = wolfpack.newitem( 1 )
			newitem.id = 0x80
			newitem.moveto( pos )
			newitem.decay = 0 # Dont decay. TempEffect will take care of them
			newitem.update()
			newitem.settag( 'dispellable_field', 1 )
			serials.append( newitem.serial )
			wolfpack.effect( 0x376a, pos, 9, 10 )
			
		wolfpack.addtimer( 10000, "magic.utilities.field_expire", serials, 1 )

class Teleport( Spell ):
	def __init__( self ):
		Spell.__init__( self, 3 )
		self.reagents = { REAGENT_BLOODMOSS: 1, REAGENT_MANDRAKE: 1 }
		self.mantra = 'Rel Por'
		self.validtarget = TARGET_GROUND
		
	def target( self, char, mode, targettype, target ):	
		char.turnto( target )
		
		# Line of Sight (touch!! or else we can teleport trough windows)
		if not char.cansee( target, 1 ) and not char.gm:
			char.message( 500237 )
			return
			
		# Check if the target tile is blocked or in a multi
		if ( not target.validspawnspot() or target.findmulti( target )  )and not char.gm:
			char.message( 501942 )
			return
			
		if not self.consumerequirements( char, mode ):
			return

		source = char.pos

		char.removefromview()
		char.moveto( target )
		char.update()
		
		wolfpack.effect( 0x3728, source, 10, 15 )
		wolfpack.effect( 0x3728, target, 10, 15 )

		char.soundeffect( 0x1fe )


class MagicLock( Spell ):
	def __init__( self ):
		Spell.__init__( self, 3 )
		self.reagents = { REAGENT_BLOODMOSS: 1, REAGENT_SULFURASH: 1, REAGENT_GARLIC: 1 }
		self.mantra = 'An Por'
		self.validtarget = TARGET_ITEM
		
	def target( self, char, mode, targettype, target ):	
		char.turnto( target )
		
		# We can only lock unlocked chests
		if target.type != 1 or 'lock' in target.events or target.container:
			char.message( 501762 )
			return
		
		# We cannot use this on locked down chests
		if target.magic == 4:
			char.message( 501761 )
			return
				
		if not self.consumerequirements( char, mode ):
			return

		events = target.events
		events.append( 'lock' )
		target.events = events
		
		target.settag( 'lock', 'magic' )
		
		char.message( 501763 )
		wolfpack.effect( 0x376a, target.pos, 9, 32 )
		target.soundeffect( 0x1fa )
		
class Unlock( Spell ):
	def __init__( self ):
		Spell.__init__( self, 3 )
		self.reagents = { REAGENT_BLOODMOSS: 1, REAGENT_SULFURASH: 1 }
		self.mantra = 'Ex Por'
		self.validtarget = TARGET_ITEM
		
	def target( self, char, mode, targettype, target ):	
		char.turnto( target )
		
		# We can only lock unlocked chests
		if not 'lock' in target.events:
			char.message( 503101 )
			return
		
		if target.gettag( 'lock' ) != 'magic':
			char.message( 503099 )
			return
				
		if not self.consumerequirements( char, mode ):
			return

		events = target.events
		events.remove( 'lock' )
		target.events = events
		
		target.deltag( 'lock', 'magic' )
		
		wolfpack.effect( 0x376a, target.pos, 9, 32 )
		target.soundeffect( 0x1ff )

def onLoad():
	Bless().register( 17 )
	Fireball().register( 18 )
	MagicLock().register( 19 )
	#Poison().register( 20 )
	#Telekinesis().register( 21 )
	Teleport().register( 22 )
	Unlock().register( 23 )
	WallOfStone().register( 24 )
