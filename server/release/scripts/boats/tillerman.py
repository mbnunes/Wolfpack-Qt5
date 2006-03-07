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

    # Check for collision!

    xOffset = speed * rx;
    yOffset = speed * ry;

    # Move the boat
    newCoord = wolfpack.coord( boat.pos.x + xOffset, boat.pos.y + yOffset, boat.pos.z, boat.pos.map )
    boat.moveto( newCoord )
    boat.update()

    # Move boat subparts
    for i in range( 1, int( boat.gettag('boat_part_count') ) ):
        item = wolfpack.finditem( int( boat.gettag('boat_part%i' % i) ) )
        if item != None:
            newCoord = wolfpack.coord( item.pos.x + xOffset, item.pos.y + yOffset, item.pos.z, item.pos.map )
            item.moveto( newCoord )
            item.update()

    # Move other items/chars inside the boat
    listitems = boat.objects
    for item in listitems:
        newCoord = wolfpack.coord( item.pos.x + xOffset, item.pos.y + yOffset, item.pos.z, item.pos.map )
        item.moveto( newCoord )
        item.update()

def doBoatMovement( boat, args ):
    speed = args[0]
    interval = args[1]
    direction = args[2]
    single = args[3]
    Move( boat, direction, speed, True )
    if not single and boat.hastag( 'boat_moving' ):
        boat.addtimer( interval, doBoatMovement, list(args) )

def StartMoving( boat, tillerman, direction, speed, interval, single, message ):

    if boat.hastag( 'boat_anchored' ):
        if message and tillerman != None:
            tillerman.say( 501419 ) # Ar, the anchor is down sir!
        return False

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
        elif keyword == 0x50: # left
            StartMove( boat, obj, Left, False )
            return True

        elif keyword == 0x6a: # Lower anchor
            LowerAnchor( boat, obj, True )
            return True
        elif keyword == 0x6b: # Raise anchor
            RaiseAnchor( boat, obj, True )
            return True
            
        
            
