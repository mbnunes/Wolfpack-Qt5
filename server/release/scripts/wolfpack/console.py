#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Created by: DarkStorm                      #
#   )).-' {{ ;'`   | Revised by:                                #
#  ( (  ;._ \\ ctr | Last Modification: Created                 #
#===============================================================#
# Console                     									#
#===============================================================#

"""
	\library wolfpack.console
	\description Contains several server functions for the console.
"""

import _wolfpack.console

log = _wolfpack.console.log
send = _wolfpack.console.send

"""
	\function wolfpack.console.getbuffer
	\return Console Buffer
	\description Returns the current console buffer.
"""
def getbuffer():
	return _wolfpack.console.getbuffer()

"""
	\function wolfpack.console.reloadScripts
	\return None
	\description Tells the console to reload scripts.
"""
def reloadScripts():
	return _wolfpack.console.reloadScripts()


