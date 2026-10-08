import wolfpack.tests

def runTests(socket, command, arguments ):
    wolfpack.tests.runAll()

def onLoad():
    wolfpack.registercommand( "testpythonapi", runTests )
