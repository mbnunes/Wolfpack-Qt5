#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Created by: Correa                         #
#   )).-' {{ ;'`   | Revised by:                                #
#  ( (  ;._ \\ ctr | Last Modification: Created                 #
#===============================================================#
# Functions for modifying settings				                      #
#===============================================================#

"""
	\library wolfpack.settings
	\description Contains several server functions for settings.
"""

import _wolfpack.settings

d = globals()
for (name, obj) in _wolfpack.settings.__dict__.items():
	d[name] = obj
