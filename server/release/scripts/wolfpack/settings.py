#===============================================================#
#   )      (\_     | WOLFPACK 13.0.0 Scripts                    #
#  ((    _/{  "-;  | Created by: Correa                         #
#   )).-' {{ ;'`   | Revised by:                                #
#  ( (  ;._ \\ ctr | Last Modification: Created                 #
#===============================================================#
# Functions for modifying settings				#
#===============================================================#

import _wolfpack.settings
import _wolfpack

def getBool( section, key, default ):
	return _wolfpack.settings.getBool()

def setBool( section, key, value ):
	return _wolfpack.settings.setBool( section, key, value )

def getNumber( section, key, default ):
	return _wolfpack.settings.getNumber( section, key, default )

def setNumber( section, key, value ):
	return _wolfpack.settings.setNumber( section, key, value )

def getString( section, key, default ):
	return _wolfpack.settings.getString( section, key, default )

def setString( section, key, value ):
	return _wolfpack.settings.setString( section, key, value )

def reload():
	return _wolfpack.settings.reload()

def save():
	return _wolfpack.settings.save()
