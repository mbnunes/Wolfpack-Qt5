#######################################################
#
# Wrapper for wolfpack.console
#
#######################################################

from _wolfpack import *

def send( Message ):
	console_send( Message )
	
def progress( Message ):
	console_progress( Message )
	
def printDone():
	console_printDone()
	
def printFail():
	console_printFail()
	
def printSkip():
	console_printSkip()
