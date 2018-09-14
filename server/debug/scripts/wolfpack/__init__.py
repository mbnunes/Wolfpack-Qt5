#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Created by: DarkStorm                      #
#   )).-' {{ ;'`   | Revised by:                                #
#  ( (  ;._ \\ ctr | Last Modification: Created                 #
#===============================================================#

"""
	\library wolfpack
	\description Contains several server functions.
"""
import _wolfpack
from wolfpack.consts import *
from types import *

d = globals()
for (name, obj) in _wolfpack.__dict__.items():
	d[name] = obj
	
"""
	\function wolfpack.findobject
	\param serial The serial number of the object.
	\return An <object id="item">item</object>, <object id="char">char</object>, or None object.
	\description This function tries to find an item or character with the given serial and
	returns an object if either is found. Otherwise it returns None.
"""
def findobject(serial):
	if serial == -1 or serial == 0:
		return None
	elif serial >= 0x40000000:
		return finditem(serial)
	else:
		return findchar(serial)
