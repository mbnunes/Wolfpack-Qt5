#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Created by: Correa                         #
#   )).-' {{ ;'`   | Revised by:                                #
#  ( (  ;._ \\ ctr | Last Modification: Created                 #
#===============================================================#

"""
	\command decoration
	\description Places decoration read from a xml file.
	\notes The decoration will be read from a XML file called
	'data\decoration.1.xml'.
"""


import wolfpack
from wolfpack import *
from wolfpack.consts import *


def testlos( socket, command, arguments ):
    wolfpack.console.log( LOG_MESSAGE, "Message" )

def onLoad():
	wolfpack.registercommand( "testlos", testlos )

