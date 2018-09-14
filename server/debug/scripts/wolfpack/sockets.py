#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Created by: DarkStorm                      #
#   )).-' {{ ;'`   | Revised by:                                #
#  ( (  ;._ \\ ctr | Last Modification: Created                 #
#===============================================================#

"""
	\library wolfpack.sockets
	\description Contains functions for sockets.
"""

import _wolfpack.sockets

d = globals()
for (name, obj) in _wolfpack.sockets.__dict__.items():
	d[name] = obj
