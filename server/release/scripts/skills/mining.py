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

# Name, reqSkill, minSkill, maxSkill, successClilocId, mutateVeinChance%, chanceToFail%
oretable = \
[
['iron',	0,	0,	100,	1007072,	49,6,	0],
['dullcopper',	65,	25,	105,	1007073,	11,2,	50],
['shadowiron',	70,	30,	110,	1007074,	9,8,	50], 
['copper',	75,	35,	115,	1007075,	8,4,	50],
['bronze',	80,	40,	120,	1007076,	7,	50],
['gold',	85,	45,	125,	1007077,	5,6,	50],
['agapite',	90,	50,	130,	1007078,	4,2,	50],
['verite',	95,	55,	135,	1007079,	2,8,	50],        
['valorite',	99,	59,	139,	1007080,	1,4,	50]
]

def mining( char, pos, tool ):
   socket = char.socket

   if char.hastag('mining_gem'):
      gemserial = char.gettag('mining_gem')

   veingem = getvein( socket, pos )
   resourcecount = veingem.gettag( 'resourcecount' )
   socket.sysmessage( 'Mining in progress... Total resourcecount here:' + str( resourcecount )  )
   char.deltag('nowmining')
   return OK


def getvein( socket, pos ):
   #Check if we have ore_gems near ( range = 4 )
   gems = wolfpack.items( pos.x, pos.y, pos.map, 4 )
   if len( gems ) < 1:
      gem = wolfpack.additem( 'ore_gem' )
      gem.settag( 'resourcecount', whrandom.randint( 10, 34 ) )
      gem.moveto( pos )
      gem.visible = 1
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
      socket.clilocmessage( 500446, "", YELLOW, NORMAL ) # That is too far away
      return OK

   # Already mining ?
   if char.hastag('is_mining'):
      socket.clilocmessage( 503029, "", YELLOW, NORMAL ) # You are already digging.
      return OK

   tool = args[0]

   #Player also can't mine when riding, polymorphed and dead.
   #Mine char ?!
   if target.char:
      socket.clilocmessage( 501863, "", YELLOW, NORMAL ) # You can't mine that.
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
         socket.clilocmessage( 501862, "", YELLOW, NORMAL ) # You can't mine there.
      return OK

   #Find tile by it's model
   elif target.model != 0:
      if ismountainorcave( target.model ):
         #add new ore gem here and mine
         mining( char, target.pos, tool )
      else:
         socket.clilocmessage( 501862, "", YELLOW, NORMAL ) # You can't mine there.
      return OK
   else:
      return OOPS

   return OK
