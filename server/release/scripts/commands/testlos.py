#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Created by: Correa                         #
#   )).-' {{ ;'`   | Revised by:                                #
#  ( (  ;._ \\ ctr | Last Modification: Created                 #
#===============================================================#

"""
	\command testlos
	\description Tests line of sight between you and the target?
	\notes Tests line of sight between you and the target?
"""


import wolfpack
from wolfpack import *
from wolfpack.consts import *


def testlos( socket, command, arguments ):
    wolfpack.console.log( LOG_MESSAGE, "Message" )

def onLoad():
	wolfpack.registercommand( "testlos", testlos )

