#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Created by: DarkStorm                      #
#   )).-' {{ ;'`   | Revised by:                                #
#  ( (  ;._ \\ ctr | Last Modification: Created                 #
#===============================================================#
# Console
#===============================================================#

"""
	\library wolfpack.console
	\description Contains several server functions for the console.
"""

import _wolfpack.console

d = globals()
for (name, obj) in _wolfpack.console.__dict__.items():
	d[name] = obj
