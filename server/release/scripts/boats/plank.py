#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts		        #
#  ((    _/{  "-;  | Created by: Correa			        #
#   )).-' {{ ;'`   | Revised by: see svn logs			#
#  ( (  ;._ \\ ctr | Last Modification: see svn logs	        #
#===============================================================#
# Boats plank script                                            #
#===============================================================#

from wolfpack import tr, console
import wolfpack
from wolfpack.consts import *
import boats
from wolfpack.utilities import hex2dec

def openPlank( item ):
    if item.hastag( 'plank_open' ):
        return

    if item.id == 0x3eb1:
        item.id = 0x3ed5
    elif item.id == 0x3e8a:
        item.id = 0x3e89
    elif item.id == 0x3eb2:
        item.id = 0x3ed4
    elif item.id == 0x3e85:
        item.id = 0x3e84

    item.update()
    item.settag( 'plank_open', 1 )
    

def closePlank( item ):
    if not item.hastag( 'plank_open' ):
        return
    
    if item.id == 0x3ed5:
        item.id = 0x3eb1
    elif item.id == 0x3e89:
        item.id = 0x3e8a
    elif item.id == 0x3ed4:
        item.id = 0x3eb2
    elif item.id == 0x3e84:
        item.id = 0x3e85

    item.update()
    item.deltag( 'plank_open' )
    

def onUse( player, item ):

    if ( player.distanceto( item ) > 8 ):
        return False

    boat_serial = item.gettag( 'boat_serial' )

    if ( player.multi != None and player.multi.serial == boat_serial ):
        if ( item.hastag('plank_open') ):
            closePlank( item )
        else:
            openPlank( item )
    else:
        if not item.hastag('plank_open'):
            if not item.hastag('plank_locked'):
                openPlank( item )
            elif player.gm:
                player.say( 502502 ); # That is locked but your godly powers allow access
            else:
                player.say( 502503 ); # That is locked.
        elif not item.hastag('plank_locked'):
            newpos = wolfpack.coord( item.pos.x, item.pos.y, item.pos.z + 5, item.pos.map )
            player.moveto( newpos )
            player.update()
        elif player.gm:
            player.say( 502502 ); # That is locked but your godly powers allow access
            newpos = wolfpack.coord( item.pos.x, item.pos.y, item.pos.z + 5, item.pos.map )
            player.moveto( newpos )
            player.update()
        else:
            player.say( 502503 ); # That is locked.
    return True

def onCollide( char, item ):

    if not item.hastag('plank_open'):
        return False

    boat_serial = item.gettag( 'boat_serial' )
    if char.multi != None and char.multi.serial != boat_serial:
        return True

    rx = 0
    ry = 0

    if item.id == 0x3ed4:
        rx = 1
    elif item.id == 0x3ed5:
        rx = -1
    elif item.id == 0x3e84:
        ry = 1
    elif item.id == 0x3e89:
        ry = -1

    for i in range( 1, 6 ):
        x = item.pos.x + ( i * rx )
        y = item.pos.y + ( i * ry )
        z = 0

        for j in range( -8, 8 ):
            z = char.pos.z + j
            pos = wolfpack.coord( x, y, z, item.pos.map )
            if pos.validspawnspot():
                if i == 1 and j >= -2 and j <= 2:
                    return True
                char.moveto( pos )
                char.update()
                return True

        #z = wolfpack.getaveragez( x, y, item.pos.map )
        #pos = wolfpack.coord( x, y, z, item.pos.map )
        #if  pos.validspawnspot():
        #    if i == 1:
        #        return True
        #    char.moveto( pos )
        #    char.update()
        #    return True
    return True

