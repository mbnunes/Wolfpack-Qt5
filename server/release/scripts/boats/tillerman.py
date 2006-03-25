#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts		        #
#  ((    _/{  "-;  | Created by: Correa			        #
#   )).-' {{ ;'`   | Revised by: see svn logs			#
#  ( (  ;._ \\ ctr | Last Modification: see svn logs	        #
#===============================================================#
# Boats tillerman script                                        #
#===============================================================#

from wolfpack import tr, console
import wolfpack
from wolfpack.consts import *
import boats
from wolfpack.utilities import hex2dec

import wolfpack.console

Forward = 0
ForwardLeft = 7
ForwardRight = 1
Backward = 4
BackwardLeft = 5
BackwardRight = 3
Left = 6
Right = 2

FastInterval = 750
SlowInterval = 750

SlowDriftInterval = 1500
FastDriftInterval = 750

SlowSpeed = 1
FastSpeed = 3

SlowDriftSpeed = 1
FastDriftSpeed = 1

def SetName():
    pass

def RemoveName():
    pass

def GiveName():
    pass

def MovementOffset( direction ):
    rx = 0
    ry = 0

    if direction == Forward:
        ry = ry - 1
    elif direction == ForwardLeft:
        ry = ry - 1
        rx = rx - 1
    elif direction == ForwardRight:
        ry = ry - 1
        rx = rx + 1
    elif direction == Backward:
        ry = ry + 1
    elif direction == BackwardLeft:
        ry = ry + 1
        rx = rx - 1
    elif direction == BackwardRight:
        ry = ry + 1
        rx = rx + 1
    elif direction == Left:
        rx = rx - 1
    elif direction == Right:
        rx = rx + 1
    
    return rx, ry

def Move( boat, direction, speed, message ):
    if boat.hastag('boat_anchored'):
        return False

    rx, ry = MovementOffset( ( direction + int( boat.gettag('boat_facing') ) & 0x07 ) )

    xOffset = speed * rx;
    yOffset = speed * ry;

    # Move the boat
    newCoord = wolfpack.coord( boat.pos.x + xOffset, boat.pos.y + yOffset, boat.pos.z, boat.pos.map )

    # Check collision here
    if not wolfpack.canboatmoveto( boat, newCoord ):
        if message:
            tillerman = wolfpack.finditem( int( boat.gettag('boat_tillerman') ) )
            if tillerman != None:
                tillerman.say( 501424 ) # Ar, we've stopped sir.
        return False;        

    
    boat.moveto( newCoord )
    boat.update()

    # Move boat subparts
    for i in range( 1, int( boat.gettag('boat_part_count') ) + 1 ):
        item = wolfpack.finditem( int( boat.gettag('boat_part%i' % i) ) )
        if item != None:
            newCoord = wolfpack.coord( item.pos.x + xOffset, item.pos.y + yOffset, item.pos.z, item.pos.map )
            item.moveto( newCoord )
            item.update()

    # Move other items/chars inside the boat
    listitems = boat.objects
    for item in listitems:
	if not item.hastag('boat_serial'):
		newCoord = wolfpack.coord( item.pos.x + xOffset, item.pos.y + yOffset, item.pos.z, item.pos.map )
		item.moveto( newCoord )
		item.update()
    return True

def doBoatMovement( boat, args ):
    speed = args[0]
    interval = args[1]
    direction = args[2]
    single = args[3]

    if not boat: # linger timer from removed boat
        return
    
    if not Move( boat, direction, speed, True ):
        return
    
    if not single and boat.hastag( 'boat_moving' ):
        boat.addtimer( interval, doBoatMovement, list(args) )

def StartMoving( boat, tillerman, direction, speed, interval, single, message ):

    if boat.hastag( 'boat_anchored' ):
        if message and tillerman != None:
            tillerman.say( 501419 ) # Ar, the anchor is down sir!
        return False

    if boat.hastag( 'boat_moving' ):
        StopMove( boat, tillerman, False )
    
    boat.settag( 'boat_moving', 1 )
    boat.addtimer( interval, doBoatMovement, [speed, interval, direction, single] )
    return True

def StartMove( boat, tillerman, direction, fast ):

    drift = False
    if direction != Forward and direction != ForwardLeft and direction != ForwardRight:
        drift = True
    interval = SlowInterval;
    speed = SlowSpeed
    if drift and fast:
        interval = FastDriftInterval
        speed = FastDriftSpeed
    elif not drift and fast:
        interfal = FastInterval
        speed = FastSpeed
    elif drift and not fast:
        interval = SlowDriftInterval
        speed = SlowDriftSpeed

    if StartMoving( boat, tillerman, direction, speed, interval, False, True ):
        if tillerman != None:
            tillerman.say( 501429 ) # Aye aye sir.
        return True
    return False

def StopMove( boat, tillerman, message ):
    if not boat.hastag( 'boat_moving' ):
        if message and tillerman != None:
            tillerman.say( 501443 ) # Er, the ship is not moving sir.
        return False
    boat.deltag( 'boat_moving' )

    if message and tillerman != None:
        tillerman.say( 501429 ) # Aye aye sir.
    return True

def LowerAnchor( boat, tillerman, message ):
    if boat.hastag( 'boat_anchored' ):
        if message and tillerman != None:
            tillerman.say( 501445 ) # Ar, the anchor was already dropped sir.
        return False

    StopMove( boat, tillerman, False )

    boat.settag( 'boat_anchored', 1 )

    if message and tillerman != None:
        tillerman.say( 501444 ) # Ar, anchor dropped sir.

    return True

def RaiseAnchor( boat, tillerman, message ):
    if not boat.hastag( 'boat_anchored' ):
        if message and tillerman != None:
            tillerman.say( 501447 ) # Ar, the anchor has not been dropped sir.
        return False

    boat.deltag( 'boat_anchored' )

    if message and tillerman != None:
        tillerman.say( 501446 ) # Ar, anchor raised sir.

    return True

def Rotate( boat, coord, count ):
    rx = coord.x - boat.pos.x
    ry = coord.y - boat.pos.y

    for i in range( 0, count ):
        temp = rx
        rx = -ry
        ry = temp
    return wolfpack.coord( boat.pos.x + rx, boat.pos.y + ry, coord.z, coord.map )

def SetFacing( boat, direction ):

    # Checking collision
    newboatid = boat.id
    if direction == Forward:
        newboatid = int( boat.gettag('boat_id_north') )
    elif direction == Backward:
        newboatid = int( boat.gettag('boat_id_south') )
    elif direction == Left:
        newboatid = int( boat.gettag('boat_id_west') )
    elif direction == Right:
        newboatid = int( boat.gettag('boat_id_east') )

    oldboatid = boat.id
    boat.id = newboatid
    if not wolfpack.canboatmoveto( boat, boat.pos ):
        boat.id = oldboatid
        return False;        

    old = int( boat.gettag('boat_facing') )
    boat.settag( 'boat_facing', direction )
    rx, ry = MovementOffset( direction )

    count = ( direction - old ) & 0x07
    count = count / 2

    # Move boat subparts
    for i in range( 1, int( boat.gettag('boat_part_count') ) + 1 ):
        item = wolfpack.finditem( int( boat.gettag('boat_part%i' % i) ) )
        if item != None:
            #wolfpack.console.send('direction = %i\n' % direction )
            if item.id in [ 0x3e4b, 0x3e4e, 0x3e50, 0x3e53 ]: # Tillerman
                distance = max( abs( boat.pos.y - item.pos.y ), abs( boat.pos.x - item.pos.x ) )
                p = wolfpack.coord( boat.pos.x + ( rx * -distance ), boat.pos.y + ( ry * -distance ), item.pos.z, item.pos.map )
                if direction == Forward:
                    p.x = p.x + 1
                    item.id = 0x3e4e
                elif direction == Backward:
                    item.id = 0x3e4b
                elif direction == Left:
                    item.id = 0x3e50
                elif direction == Right:
                    item.id = 0x3e53
                item.moveto( p )
            elif item.id in [ 0x3e65, 0x3e93, 0x3eae, 0x3eb9 ]: # Hold
                distance = max( abs( boat.pos.y - item.pos.y ), abs( boat.pos.x - item.pos.x ) )
                p = wolfpack.coord( boat.pos.x + ( rx * distance ), boat.pos.y + ( ry * distance ), item.pos.z, item.pos.map )
                if direction == Forward:
                    #p.x = p.x + 1
                    item.id = 0x3eae
                elif direction == Backward:
                    item.id = 0x3eb9
                elif direction == Left:
                    item.id = 0x3e93
                elif direction == Right:
                    item.id = 0x3e65
                item.moveto( p )
            elif item.id in [ 0x3ed4, 0x3ed5, 0x3e84, 0x3e89, 0x3eb2, 0x3eb1, 0x3e85, 0x3e8a ]: # gang planks
                if item.hastag('plank_open'):
                    if item.hastag('plank_starboard'):
                        if direction == Forward:
                            item.id = 0x3ed4
                        elif direction == Backward:
                            item.id = 0x3ed5
                        elif direction == Left:
                            item.id = 0x3e89
                        elif direction == Right:
                            item.id = 0x3e84
                    else:
                        if direction == Forward:
                            item.id = 0x3ed5
                        elif direction == Backward:
                            item.id = 0x3ed4
                        elif direction == Left:
                            item.id = 0x3e84
                        elif direction == Right:
                            item.id = 0x3e89
                else:
                    if item.hastag('plank_starboard'):
                        if direction == Forward:
                            item.id = 0x3eb2
                        elif direction == Backward:
                            item.id = 0x3eb1
                        elif direction == Left:
                            item.id = 0x3e8a
                        elif direction == Right:
                            item.id = 0x3e85
                    else:
                        if direction == Forward:
                            item.id = 0x3eb1
                        elif direction == Backward:
                            item.id = 0x3eb2
                        elif direction == Left:
                            item.id = 0x3e85
                        elif direction == Right:
                            item.id = 0x3e8a
                item.moveto( Rotate( boat, item.pos, count ) )
            item.update()

    # Rotate other items/chars inside the boat
    listitems = boat.objects
    for item in listitems:
	if not item.hastag('boat_serial'):
		item.moveto( Rotate( boat, item.pos, count ) )
		if item.ischar:
			item.direction += int( direction ) - int( old )
		item.update()

            
    boat.update()

    return True
    

def doBoatTurn( boat, args ):
    offset = args[0]
    message = args[1]

    if boat.hastag( 'boat_anchored' ):
        return False
    if SetFacing( boat, ( offset + int( boat.gettag('boat_facing') ) ) & 0x07 ):
        return True
    else:
        if message:
            boat.say( 501423 ) # Ar, can't turn sir.
        return False
    
def BoatRename( item, player ):
    pass

def DryDock( item, player ):
    pass

def StartTurn( boat, tillerman, offset, message ):
    if boat.hastag( 'boat_anchored' ):
        if message and tillerman != None:
            tillerman.say( 501419 ) # Ar, the anchor is down sir!
        return False

    if boat.hastag( 'boat_moving' ):
        StopMove( boat, tillerman, False )

    boat.addtimer( 500, doBoatTurn, [offset, message] )

    if message and tillerman != None:
        tillerman.say( 501429 ) # Aye aye sir.
    return True
    

def onSpeech( obj, player, text, keywords ):

    if player.multi == None:
        return False
    
    if player.multi.serial != obj.gettag('boat_serial'):
        return False

    boat = player.multi

    for keyword in keywords:
        obj.say( "%s" % str( keyword ) )
        if keyword == 0x42: # rename
            SetName()
            return True
        elif keyword == 0x43: # remove name
            RemoveName()
            return True
        elif keyword == 0x44: # give name
            GiveName()
            return True
        elif keyword == 0x45: # forward
            StartMove( boat, obj, Forward, True )
            return True
        elif keyword == 0x46: # backward
            StartMove( boat, obj, Backward, True )
            return True
        elif keyword == 0x47: # left
            StartMove( boat, obj, Left, True )
            return True
        elif keyword == 0x48: # right
            StartMove( boat, obj, Right, True )
            return True
        elif keyword == 0x4b: # forward left
            StartMove( boat, obj, ForwardLeft, True )
            return True
        elif keyword == 0x4c: # forward right
            StartMove( boat, obj, ForwardRight, True )
            return True
        elif keyword == 0x4d: # backward left
            StartMove( boat, obj, BackwardLeft, True )
            return True
        elif keyword == 0x4e: # backward right
            StartMove( boat, obj, BackwardRight, True )
            return True
        elif keyword == 0x4f: # stop
            StopMove( boat, obj, True )
            return True
        elif keyword == 0x49 or keyword == 0x65: # turn right
            StartTurn( boat, obj, 2, True )
        elif keyword == 0x4A or keyword == 0x66: # turn left
            StartTurn( boat, obj, -2, True )
        elif keyword == 0x50: # left
            StartMove( boat, obj, Left, False )
            return True
        elif keyword == 0x6a: # Lower anchor
            LowerAnchor( boat, obj, True )
            return True
        elif keyword == 0x6b: # Raise anchor
            RaiseAnchor( boat, obj, True )
            return True
        elif keyword == 0x67: # turn around, come about
            StartTurn( boat, obj, -4, True )
            

def onUse( player, item ):

    if player.multi.serial != obj.gettag('boat_serial'):
        DryDock( item, player )
    else:
        RenameBoat( item, player )
            
