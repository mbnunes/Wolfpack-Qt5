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
import _wolfpack

"""
	\function wolfpack.settings.getBool
	\param section
	\param key
	\param default
	\return Boolean
	\description Returns the boolean value for the key in a given section.
"""
def getBool( section, key, default ):
	return _wolfpack.settings.getBool()

"""
	\function wolfpack.settings.setBool
	\param section
	\param key
	\param default
	\description Sets a boolean value for the key in a given section.
"""
def setBool( section, key, value ):
	return _wolfpack.settings.setBool( section, key, value )

"""
	\function wolfpack.settings.getNumber
	\param section
	\param key
	\param default
	\return Number
	\description Returns the number value for the key in a given section.
"""
def getNumber( section, key, default ):
	return _wolfpack.settings.getNumber( section, key, default )

"""
	\function wolfpack.settings.setNumber
	\param section
	\param key
	\param default
	\description Sets a number value for the key in a given section.
"""
def setNumber( section, key, value ):
	return _wolfpack.settings.setNumber( section, key, value )

"""
	\function wolfpack.settings.getString
	\param section
	\param key
	\param default
	\return String
	\description Returns the string value for the key in a given section.
"""
def getString( section, key, default ):
	return _wolfpack.settings.getString( section, key, default )

"""
	\function wolfpack.settings.setString
	\param section
	\param key
	\param default
	\description Sets a string value for the key in a given section.
"""
def setString( section, key, value ):
	return _wolfpack.settings.setString( section, key, value )

"""
	\function wolfpack.settings.reload
	\description Reload the settings.
"""
def reload():
	return _wolfpack.settings.reload()

"""
	\function wolfpack.settings.save
	\description Saves the settings.
"""
def save():
	return _wolfpack.settings.save()
