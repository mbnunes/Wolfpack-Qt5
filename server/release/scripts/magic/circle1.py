
from magic import registerspell
from magic.spell import CharEffectSpell, Spell, DelayedDamageSpell
from magic.utilities import *
import random
import wolfpack
from wolfpack.utilities import tobackpack

class Clumsy ( CharEffectSpell ):
  def __init__( self ):
    CharEffectSpell.__init__( self, 1 )
    self.reagents = { REAGENT_BLOODMOSS: 1, REAGENT_NIGHTSHADE: 1 }
    self.mantra = 'Uus Jux'
    self.harmful = 1
    
  def effect( self, char, target ):
    statmodifier( char, target, 1, 1 )
        
    target.effect( 0x3779, 10, 15 )
    target.soundeffect( 0x1df )
    
class Feeblemind ( CharEffectSpell ):
  def __init__( self ):
    CharEffectSpell.__init__( self, 1 )
    self.reagents = { REAGENT_GINSENG: 1, REAGENT_NIGHTSHADE: 1 }
    self.mantra = 'Rel Wis'
    self.harmful = 1
    
  def effect( self, char, target ):
    statmodifier( char, target, 2, 1 )
        
    target.effect( 0x3779, 10, 15 )
    target.soundeffect( 0x1e5 )    
        
class Weaken ( CharEffectSpell ):
  def __init__( self ):
    CharEffectSpell.__init__( self, 1 )
    self.reagents = { REAGENT_GARLIC: 1, REAGENT_NIGHTSHADE: 1 }
    self.mantra = 'Des Mani'
    self.harmful = 1
    
  def effect( self, char, target ):
    statmodifier( char, target, 0, 1 )
        
    target.effect( 0x3779, 10, 15 )
    target.soundeffect( 0x1e6 )        

class CreateFood( Spell ):
  def __init__( self ):
    Spell.__init__( self, 1 )
    self.reagents = { REAGENT_GARLIC: 1, REAGENT_GINSENG: 1, REAGENT_MANDRAKE: 1 }
    self.mantra = 'In Mani Ylem'

  def cast( self, char, mode ):
    if not self.consumerequirements( char, mode ):
      return
      
    # Randomly select one id of food
    food = [ "9d1", "9d2", "9d3", "97c", "9eb", "9f2", "97b", "9b7" ]
    
    item = wolfpack.additem( random.choice( food ) )
    foodname = " " + item.getname()
    if not tobackpack( item, char ):
      item.update()

    if char.socket:
      char.socket.clilocmessage(1042695, "", 0x3b2, 3, None, foodname)
    char.soundeffect( 0x1e2 )

class Heal ( CharEffectSpell ):
  def __init__( self ):
    CharEffectSpell.__init__( self, 1 )
    self.reagents = { REAGENT_GARLIC: 1, REAGENT_GINSENG: 1, REAGENT_SPIDERSILK: 1 }
    self.mantra = 'In Mani'
    
  def effect( self, char, target ):
    # 10% of Magery + 1-5
    amount = int( 0.01 * char.skill[ MAGERY ] ) + random.randint( 1, 5 )
    target.hitpoints = min( target.maxhitpoints, target.hitpoints + amount )
    target.updatehealth()        
        
    target.effect( 0x376A, 9, 32 )
    target.soundeffect( 0x1f2 )

class NightSight ( CharEffectSpell ):
        def __init__( self ):
                CharEffectSpell.__init__( self, 1 )
                self.reagents = { REAGENT_SPIDERSILK: 1, REAGENT_SULFURASH: 1 }
                self.mantra = 'In Lor'

        def effect( self, char, target ):
                if not target:
                    return
                if target.skill[ MAGERY ] < 100:
                    char.socket.sysmessage( "Target must have at least 10 points of magery skill." )
                    return;
                target.soundeffect( 0x1e4 )
                target.effect( 0x373a )
                target.disturb()

                duration = 60000 + char.skill[ MAGERY ]
                target.settag( "nightsight", 1 )
                target.fixedlight = 0
                target.addtimer( duration, "magic.circle1.nightsightrestore", [] )


def nightsightrestore( char, args ):
    if not char:
        return
    if char.hastag("nightsight"):
        char.deltag("nightsight")
        char.fixedlight = 255
        
                
    
class MagicArrow ( DelayedDamageSpell ):
  def __init__( self ):
    DelayedDamageSpell.__init__( self, 1 )
    self.reagents = { REAGENT_SULFURASH: 1 }
    self.mantra = 'In Por Ylem'
    self.missile = [ 0x36e4, 0, 1, 5, 0 ]
    self.sound = 0x1e5
    
  def damage( self, char, target ):
    damage = self.scaledamage( char, target, random.randint( 4, 8 ) )
    target.damage( DAMAGE_MAGICAL, damage, char )

class ReactiveArmor( Spell ):
  def __init__( self ):
    Spell.__init__( self, 1 )
    self.reagents = { REAGENT_GARLIC: 1, REAGENT_SPIDERSILK: 1, REAGENT_SULFURASH: 1 }
    self.mantra = 'Flam Sanct'
    
  def cast( self, char, mode ):
    # Do we still have damage to absorb?
    if char.hastag( 'damage_absorb' ) and 'magic.reactivearmor' in char.events:
      char.message( 1005559 )
      return
  
    if not self.consumerequirements( char, mode ):
      return
      
    amount = min( 75, int( ( char.skill[ MAGERY ] + char.skill[ MEDITATION ] + char.skill[ INSCRIPTION ] ) / 30 ) )

    events = char.events
    if not 'magic.reactivearmor' in events:
      events.append( 'magic.reactivearmor' )
    char.events = events

    char.settag( 'damage_absorb', amount )
    
    char.effect( 0x376a, 9, 32 )
    char.soundeffect( 0x1f2 )

# Register the spells in this module (magic.circle1)
def onLoad():
  Clumsy().register( 1 )
  CreateFood().register( 2 )
  Feeblemind().register( 3 )
  Heal().register( 4 )
  MagicArrow().register( 5 )
  NightSight().register( 6 )
  ReactiveArmor().register( 7 )
  Weaken().register( 8 )
