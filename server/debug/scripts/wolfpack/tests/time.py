
from wolfpack import *

def runTests():
    result = True
    if time.minute() not in range( 0, 59 ):
        console.send(" - time.minute() fail\n")
        result = False
    if time.hour() not in range( 0, 23 ):
        console.send(" - time.hour() fail\n")
        result = False
    if time.days() < 0:
        console.send(" - time.days() fail\n")
        result = False
    if time.minutes() < 0:
        console.send(" - time.minutes() fail\n")
        result = False
    if time.currentlightlevel() not in range( 0, 30 ):
        console.send("time.currentlightlevel() fail\n")
        result = False
    return result


def runAll():
    console.sendprogress("Testing wolfpack.time functions")
    if not runTests():
        console.sendfail()
        return False
    else:
        console.senddone()
        return True
    
    
