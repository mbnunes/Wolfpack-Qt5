
from wolfpack import *
from wolfpack.consts import *

def testConsoleLog():
    console.log( LOG_MESSAGE, "LOG_MESSAGE test" )
    console.log( LOG_ERROR, "LOG_ERROR test" )
    console.log( LOG_PYTHON, "LOG_PYTHON test" )
    console.log( LOG_WARNING, "LOG_WARNING test" )
    console.log( LOG_NOTICE, "LOG_NOTICE test" )
    console.log( LOG_TRACE, "LOG_TRACE test" )
    console.log( LOG_DEBUG, "LOG_DEBUG test" )

def testConsoleSend():
    console.send(" console.send test message\n" )
    console.send(u" console.send unicode message\n" )

def testConsoleSendProgress():
    console.sendprogress("Testing progress, result Fail")
    console.sendfail()
    console.sendprogress("Testing progress, result Done")
    console.senddone()
    console.sendprogress("Testing progress, result Skip")
    console.sendskip()

def testConsoleGetBuffer():
    console.sendprogress("Testing console.getbuffer")
    text = console.getbuffer()
    if ( len(text) < 5 ):
        console.sendfail()
    else:
        console.senddone()

def runAll():
    testConsoleSend()
    testConsoleLog()
    testConsoleSendProgress()
    testConsoleGetBuffer()

