# Script for handling Python errors

import sys
import traceback
import cStringIO
import time
import types

def handle_error( exception, value, tb ):

	output = cStringIO.StringIO()
	traceback.print_exception( exception, value, tb, None, output )

	print output.getvalue()

	if type( exception ) == types.ClassType:
		stype = exception.__name__
	else:
		stype = exception

	log = open( 'python.log', 'a' )
	log.write( "[%s] Exception (%s) occured:\n" % ( time.strftime( '%d.%m.%Y %H:%M:%S' ), stype ) )
	log.write( output.getvalue() )
	log.write( "\n" )
	log.close()

# Register our Errorhandler
def onLoad():
	global oldhook
	oldhook = sys.excepthook
	sys.excepthook = handle_error

	# Clear the Python Logfile
	file = open( 'python.log', 'w' )
	log.write( "[%s] Errorhandler installed\n" % time.strftime( '%d.%m.%Y %H:%M:%S' ) )
	file.close()

# Unregister the ErrorHandler
def onUnload():
	global oldhook
	sys.excepthook = oldhook
