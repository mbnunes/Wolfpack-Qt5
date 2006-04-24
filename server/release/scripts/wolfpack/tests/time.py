
from wolfpack import *

def runTests():
    if time.minute() not in range( 0, 59 ):
        return False
    if time.hour() not in range( 0, 23 ):
        return False
    if time.days() < 0:
        return False
    if time.minutes() < 0:
        return False
    if time.currentlightlevel() not in range( 0, 30 ):
        return False


def runAll():
    console.sendprogress("Testing wolfpack.time functions")
    if not runTests():
        console.sendfail()
        return False
    else:
        console.senddone()
        return True
    
    
