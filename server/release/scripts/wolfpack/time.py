#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Created by: DarkStorm                      #
#   )).-' {{ ;'`   | Revised by:                                #
#  ( (  ;._ \\ ctr | Last Modification: Created                 #
#===============================================================#
# Module with timing functions									#
#===============================================================#

import _wolfpack.time
import _wolfpack

def second():
	return _wolfpack.time.second()

def minute():
	return _wolfpack.time.minute()

def hour():
	return _wolfpack.time.hour()

def day():
	return _wolfpack.time.day()

def month():
	return _wolfpack.time.month()

def year():
	return _wolfpack.time.year()

def timestamp():
	return _wolfpack.time.timestamp()

def servertime():
	return _wolfpack.currenttime()

def currentdatetime():
	return _wolfpack.currentdatetime()

currentlightlevel = _wolfpack.time.currentlightlevel
