#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: codex                          #
#   )).-' {{ ;'`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification: Created                 #
#################################################################

from wolfpack.consts import *
import wolfpack

# Register as a global script
def onLoad():
   wolfpack.registerglobal( HOOK_CHAR, EVENT_SHOWTOOLTIP, "tooltip" )
   wolfpack.registerglobal( HOOK_ITEM, EVENT_SHOWTOOLTIP, "tooltip" )

def onShowToolTip( sender, target, tooltip ):  
   if( target.isitem() ):
      if( target.amount > 1 ):
         multiitem( target, tooltip )
      else:
         tooltip.add( 1050045, " \t" + target.getname() + "\t " )
   else:
      tooltip.add( 1050045, " \t" + target.name + "\t " ) 

   tooltip.send( sender )

   return 1

def armor( target, tooltip ):
   tooltip.add( 1050045, " \tArmor test\t " ) #prefix name suffix
   tooltip.add( 1060448, "3" )                #Physical Resist 
   tooltip.add( 1060447, "1" )                #Fire Resist
   tooltip.add( 1060445, "1" )                #Cold Resist
   tooltip.add( 1060449, "5" )                #Poison Resist
   tooltip.add( 1060446, "3" )                #Energy Resist
   tooltip.add( 1061170, "40" )               #Strength Requirement
   tooltip.add( 1060639, "46\t46" )           #Durability
    
def multiitem( target, tooltip ):
   tooltip.add( 1050039, str( target.amount ) + "\t" + target.getname() ) #$amount $name

def container( target, tooltip ):
   tooltip.add( 1050045, "Bag" )
   tooltip.add( 1050044, "1000\t10000" )     #$count items, $weight stones

def shield( target, tooltip ):
   tooltip.add( 1050045, "Shield of exceptional quality" )
   tooltip.add( 1060636, "" )          #Exceptional
   tooltip.add( 1060448, "1" )         #Physical resist
   tooltip.add( 1061170, "95" )        #Strength requirement
   tooltip.add( 1060639, "132\t132" )  #Durability

def hat( target, tooltip ):
   tooltip.add( 1060447, "5" )         #Fire Resist
   tooltip.add( 1060445, "9" )         #Cold Resist
   tooltip.add( 1060449, "5" )         #Poison Resist
   tooltip.add( 1060446, "3" )         #Energy Resist
   tooltip.add( 1061170, "10" )        #Strength requirement
   tooltip.add( 1060639, "24\t24" )    #Durability
   
