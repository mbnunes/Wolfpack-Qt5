
from magic.spell import CharEffectSpell, Spell, DelayedDamageSpell
from magic.utilities import *
import random
import wolfpack
from wolfpack.utilities import tobackpack

class BladeSpirits ( Spell ):
  def __init__( self ):
    Spell.__init__( self, 5 )
    self.reagents = { REAGENT_BLACKPEARL: 1, REAGENT_MANDRAKE: 1, REAGENT_NIGHTSHADE: 1 }
    self.mantra = 'In Jux Hur Ylem'
    self.validtarget = TARGET_GROUND
    
  def target( self, char, mode, targettype, target ):  
    char.turnto( target )
    
    if not char.canreach(target, 10):
      char.message(500237)
      return
      
    if not self.consumerequirements(char, mode):
      return
    
    char = wolfpack.addnpc('blade_spirit', target)                 
    
def onLoad():
  BladeSpirits().register( 33 )
