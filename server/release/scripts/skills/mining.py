#################################################################
#   )      (\_     # WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  # Created by: codex                          #
#   )).-' {{ ;'`   # Revised by:                                #
#  ( (  ;._ \\ ctr # Last Modification: Sep, 30 '03	        #
#################################################################

from wolfpack.consts import *
import whrandom
import wolfpack
import skills
from wolfpack.time import *
from wolfpack.utilities import *

#mining calling from pickaxe.py and shovel.py

oresound = 0x126
#in seconds
miningdelay = 1600
# Name, reqSkill, minSkill, maxSkill, successClilocId, itemId, color, mutateVeinChance%, VeinChanceToFallback%

oretable = \
{
'iron':		[   0,   0, 1000, 1007072, 0x19b9,     0, 49.6,  0 ],
'dullcopper':	[ 650, 250, 1050, 1007073, 0x19b9, 0x973, 11.2, 50 ],
'shadowiron':	[ 700, 300, 1100, 1007074, 0x19b9, 0x966,  9.8, 50 ], 
'copper':	[ 750, 350, 1150, 1007075, 0x19b9, 0x960,  8.4, 50 ],
'bronze':	[ 800, 400, 1200, 1007076, 0x19b9, 0x972,    7, 50 ],
'gold':		[ 850, 450, 1250, 1007077, 0x19b9, 0x8a5,  5.6, 50 ],
'agapite':	[ 900, 500, 1300, 1007078, 0x19b9, 0x979,  4.2, 50 ],
'verite':	[ 950, 550, 1350, 1007079, 0x19b9, 0x89f,  2.8, 50 ],
'valorite':	[ 990, 590, 1390, 1007080, 0x19b9, 0x8ab,  1.4, 50 ]
}

def mining( char, pos, tool ):
   socket = char.socket

   if char.hastag( 'mining_gem' ):
      veingem = wolfpack.finditem( char.gettag( 'mining_gem' ) )
      if not veingem:
         veingem = getvein( socket, pos )
         if not veingem:
            char.deltag( 'mining_gem' )
            return OOPS
   else:
      veingem = getvein( socket, pos )
      if not veingem:
         return OOPS
      else:
         char.settag( 'mining_gem', veingem.serial )

   if char.distanceto( veingem ) > MINING_MAX_DISTANCE:
      veingem = getvein( socket, pos )

   if not veingem:
      return OOPS

   if not veingem.hastag( 'resname' ) or not veingem.hastag( 'resourcecount' ):
      return OOPS

   resname = veingem.gettag( 'resname' ) # Sometimes mutated in colored ore and back
   resourcecount = veingem.gettag( 'resourcecount' )
   reqskill = oretable[ resname ][ REQSKILL ]

   success = 0
   char.addtimer( 1400, "skills.mining.effecttimer", [oresound] )
   char.settag( 'is_mining', servertime() + miningdelay )
   char.turnto( pos )
   char.action( 11 )

   # Are you skilled enough ? And here is ore ?
   if resourcecount > 2 and char.skill[ MINING ] > reqskill:
      # Anyway you haven't 100% chance to get something :)
      if char.skill[ MINING ] > reqskill:
         if whrandom.randint( oretable[ resname ][ MINSKILL ], oretable[ resname ][ MAXSKILL ] ) < char.skill[ MINING ]:
            if whrandom.random() < 0.9:
               success = 1
               skills.successharvest( char, veingem, oretable, resname, 1 ) # 1 - amount of ore
               skills.checkskill( char, MINING, 0 )

   if success == 0:
      socket.clilocmessage( 501869, "", GRAY, NORMAL ) # You loosen some rocks but fail to find any usable ore.
      
   char.deltag('nowmining')
   return OK


def getvein( socket, pos ):
   #Check if we have ore_gems near ( range = 4 )
   gems = wolfpack.items( pos.x, pos.y, pos.map, 4 )
   if len( gems ) < 1:
      gem = wolfpack.additem( 'ore_gem' )
      gem.settag( 'resourcecount', whrandom.randint( 10, 34 ) )
      gem.settag( 'resname', 'iron' )
      gem.moveto( pos )
      gem.visible = 0
      gem.update()
      return gem
   else:
      return gems[0]


#Response from mining tool
def response( char, args, target ):
   socket = char.socket
   if not socket:
      return OOPS

   pos = target.pos

   # Player can reach that ?
   if char.pos.map != pos.map or char.pos.distance( pos ) > MINING_MAX_DISTANCE:
      socket.clilocmessage( 500446, "", RED, NORMAL ) # That is too far away
      return OK

   tool = args[0]

   #Player also can't mine when riding, polymorphed and dead.
   #Mine char ?!
   if target.char:
      socket.clilocmessage( 501863, "", RED, NORMAL ) # You can't mine that.
      return OK

   
   #Check if item is ore gem
   elif target.item and isoregem( target.item ):
      #Mine if ore gem is validated
      socket.sysmessage( 'Ore gem founded' )
      mining( char, target.pos, tool )
      return OK

   #Find tile by it's position if we haven't model
   elif target.model == 0:
      map = wolfpack.map( target.pos.x, target.pos.y, target.pos.map )
      if ismountainorcave( map['id'] ):
         mining( char, target.pos, tool )
      else:
         socket.clilocmessage( 501862, "", RED, NORMAL ) # You can't mine there.
      return OK

   #Find tile by it's model
   elif target.model != 0:
      if ismountainorcave( target.model ):
         #add new ore gem here and mine
         mining( char, target.pos, tool )
      else:
         socket.clilocmessage( 501862, "", RED, NORMAL ) # You can't mine there.
      return OK
   else:
      return OOPS

   return OK

#Sound effect
def effecttimer( char, args ):
   char.soundeffect( args[0] )
   return OK
