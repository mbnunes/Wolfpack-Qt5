#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Created by: DarkStorm                      #
#   )).-' {{ ;'`   | Revised by:                                #
#  ( (  ;._ \\ ctr | Last Modification: Created                 #
#===============================================================#
# Utility functions used in Python scripts                      #
#===============================================================#

import _wolfpack.speech

def addKeyword( script, keyword ):
	_wolfpack.speech.addKeyword( script, keyword )

def addWord( script, word ):
	_wolfpack.speech.addWord( script, word )

def addRegexp( script, regexp ):
	_wolfpack.speech.addRegexp( script, regexp )

def setCatchAll( script, catchall ):
	_wolfpack.speech.setCatchAll( script, catchall )
