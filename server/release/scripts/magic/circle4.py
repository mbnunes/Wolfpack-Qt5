
from magic.spell import CharEffectSpell, Spell, DelayedDamageSpell
from magic.utilities import *
import random
import wolfpack
from wolfpack.utilities import tobackpack

class Curse ( CharEffectSpell ):
  def __init__( self ):
    CharEffectSpell.__init__( self, 4 )
    self.reagents = { REAGENT_NIGHTSHADE: 1, REAGENT_GARLIC: 1, REAGENT_SULFURASH: 1 }
    self.mantra = 'Des Sanct'
    self.harmful = 1
    
  def effect( self, char, target ):
    statmodifier( char, target, 3, 1 )
        
    target.effect( 0x373a, 10, 15 )
    target.soundeffect( 0x1ea )

class GreaterHeal ( CharEffectSpell ):
  def __init__( self ):
    CharEffectSpell.__init__( self, 4 )
    self.reagents = { REAGENT_GARLIC: 1, REAGENT_GINSENG: 1, REAGENT_SPIDERSILK: 1 }
    self.mantra = 'In Vas Mani'
    
  def effect( self, char, target ):
    # 40% of Magery + 1-10
    amount = int( 0.04 * char.skill[ MAGERY ] ) + random.randint( 1, 10 )
    target.hitpoints = min( target.maxhitpoints, target.hitpoints + amount )
    target.updatehealth()        
        
    target.effect( 0x376A, 9, 32 )
    target.soundeffect( 0x202 )

class Lightning ( DelayedDamageSpell ):
  def __init__( self ):
    DelayedDamageSpell.__init__( self, 4 )
    self.reagents = { REAGENT_SULFURASH: 1, REAGENT_MANDRAKE: 1 }
    self.mantra = 'Por Ort Grav'
    self.delay = None

  def damage(self, char, target):
    target.lightning()
    target.soundeffect(0x29)
    
    damage = self.scaledamage(char, target, random.randint(12, 21))
    target.damage(DAMAGE_MAGICAL, damage, char)

class ManaDrain ( DelayedDamageSpell ):
  def __init__( self ):
    DelayedDamageSpell.__init__( self, 4 )
    self.reagents = { REAGENT_SULFURASH: 1, REAGENT_MANDRAKE: 1 }
    self.mantra = 'Ort Rel'
    self.delay = None

  def damage( self, char, target ):
    target.effect(0x3779, 10, 25)
    target.soundeffect(0x28e)
  
    if self.checkresist(char, target):
      target.message( 501783 )
    else:
      target.mana -= random.randint(1, target.mana)
      target.updatemana()
    
class Recall ( Spell ):
  def __init__( self ):
    Spell.__init__( self, 4 )
    self.reagents = { REAGENT_BLOODMOSS: 1, REAGENT_SULFURASH: 1, REAGENT_GARLIC: 1 }
    self.mantra = 'Kal Ort Por'
    self.validtarget = TARGET_ITEM
    
  def target( self, char, mode, targettype, target ):  
    char.turnto( target )
    
    # We can only recall from recall runes
    if not 'recall_rune' in target.events:
      char.message( 502357 )
      return

    if not self.consumerequirements( char, mode ):
      return
      
    if not target.hastag( 'marked' ) or target.gettag( 'marked' ) != 1:
      char.message( 502354 )
      fizzle( char )
      return
    
    region = wolfpack.region( char.pos.x, char.pos.y, char.pos.map )
    
    if region.norecallout:
      char.message( 501802 )
      fizzle( char )
      return
      
    location = target.gettag( 'location' )
    location = location.split( "," )
    location = wolfpack.coord( location[0], location[1], location[2], location[3] )

    region = wolfpack.region( location.x, location.y, location.map )
    
    if region.norecallin:
      char.message( 1019004 )
      fizzle( char )
      return        

    char.soundeffect( 0x1fc )
    char.removefromview()
    char.moveto( location )
    char.update()
    char.socket.resendworld()
    char.soundeffect( 0x1fc )

class FireField( Spell ):
  def __init__( self ):
    Spell.__init__( self, 4 )
    self.reagents = { REAGENT_BLACKPEARL: 1, REAGENT_SPIDERSILK: 1, REAGENT_SULFURASH: 1 }
    self.mantra = 'In Flam Grav'
    self.validtarget = TARGET_GROUND
    self.harmful = 1
    self.resistable = 1
    
  def target(self, char, mode, targettype, target):
    char.turnto(target)
    
    if not char.canreach(target, 10):
      char.message(500237)
      return
      
    if not self.consumerequirements( char, mode ):
      return
    
    xdiff = abs( target.x - char.pos.x )
    ydiff = abs( target.y - char.pos.y )
    
    positions = []
    
    # West / East
    if xdiff > ydiff:
      itemid = 0x3996
      for i in range( -2, 3 ):
        positions.append( wolfpack.coord( target.x, target.y + i, target.z, target.map ) )
              
    # North South
    else:    
      itemid = 0x398c
      for i in range( -2, 3 ):
        positions.append( wolfpack.coord( target.x + i, target.y, target.z, target.map ) )    

    serials = []
    
    char.soundeffect( 0x20c )
    
    for pos in positions:
      newitem = wolfpack.newitem( 1 )
      newitem.id = itemid
      newitem.moveto( pos )
      newitem.decay = 0 # Dont decay. TempEffect will take care of them
      newitem.settag('dispellable_field', 1)
      newitem.settag('source', char.serial)
      newitem.events = ['magic.firefield']
      newitem.update()
      serials.append(newitem.serial)
      
    duration = int( ( 4 + char.skill[ MAGERY ] * 0.05 ) * 1000 )
    wolfpack.addtimer( duration, "magic.utilities.field_expire", serials, 1 )

def onLoad():
  #ArchCure().register( 25 )
  #ArchProtection().register( 26 )
  Curse().register( 27 )
  FireField().register( 28 )
  GreaterHeal().register( 29 )
  Lightning().register( 30 )
  ManaDrain().register( 31 )
  Recall().register( 32 )
