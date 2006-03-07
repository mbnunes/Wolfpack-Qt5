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

Forward = 1
ForwardLeft = 2
ForwardRight = 3
Backward = 4
BackwardLeft = 5
BackwardRight = 6
Left = 7
Right = 8

FastInterval = 750
SlowInterval = 750

SlowDriftInterval = 1500
FastDriftInterval = 750

SlowSpeed = 1
FastSpeed = 3

def SetName():
    pass

def RemoveName():
    pass

def GiveName():
    pass

def MoveForward():
    pass

def MoveBackward():
    pass

def doBoatMovement( boat, args ):
    boat.say('I should move now!')
    pass

def StartMoving( boat, tillerman, direction, speed, interval, single, message ):

    if boat.hastag( 'boat_anchored' ):
        if message and tillerman != None:
            tillerman.say( 501419 ) # Ar, the anchor is down sir!
        return False

    boat.addtimer( interval, doBoatMovement, [speed, direction] )
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
            return True
        elif keyword == 0x50: # left
            StartMove( boat, obj, Left, False )
            return True
            
        
            
