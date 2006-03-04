#!/usr/bin/env python
#################################################################
#   )      (\_     # Wolfpack 13.0.0 Build Script               #
#  ((    _/{  "-;  # Created by: Wolfpack Development Team      #
#   )).-" {{ ;"`   # Revised by: Wolfpack Development Team      #
#  ( (  ;._ \\ ctr # Last Modification: check cvs logs          #
#################################################################
#                                                               #
#################################################################
# A note about the search paths...                              #
# The order of searching should be:                             #
#    /usr/local/{include,lib}/{*.h,*.so,*.a}                    #
#    /usr/local/{include,lib}/{package}/{*.h,*.so,*.a}          #
#    /usr/{include,lib}/{*.h,*.so,*.a}                          #
#    /usr/{include,lib}/{package}/{*.h,*.so,*.a}                #
#                                                               #
# This will allow local installations to be searched first      #
#   followed by the system's default package files.             #
#                                                               #
#################################################################

import os
import sys
import glob
import fnmatch
import dircache
import string
import distutils.sysconfig

# Older Python lib work arounds...
try:
	from optparse import OptionParser
except:
	sys.path.append( '../tools/scripts' )
	from optparse import OptionParser

try:
	osHasPathDotSep = os.path.sep
except:
	os.path.sep = '/'

# Color codes ( borrowed from Gentoo's portage code )
colorcodes = {}
colorcodes["reset"] = "\x1b[0m"
colorcodes["bold"] = "\x1b[01m"
colorcodes["green"] = "\x1b[32;01m"
colorcodes["red"] = "\x1b[31;01m"
colorcodes["darkred"] = "\x1b[31;06m"
colorcodes["yellow"] = "\x1b[33;01m"

def nocolor():
	for x in colorcodes.keys():
		colorcodes[x] = ""
def bold( text ):
	return colorcodes["bold"] + text + colorcodes["reset"]
def red( text ):
	return colorcodes["red"] + text + colorcodes["reset"]
def darkred( text ):
	return colorcodes["darkred"] + text + colorcodes["reset"]
def green( text ):
	return colorcodes["green"] + text + colorcodes["reset"]
def yellow( text ):
	return colorcodes["yellow"] + text + colorcodes["reset"]

class AbstractExternalLibrary:

    def findFile( self, searchpath ):
        """Locates a file specified in the searchpath structure, returning a tuple
        containing filename, path, non-searched entries in the searchpath.

        """
	path = ""
	file = ""
	for entry in searchpath:
                del searchpath[-1] # remove entry from list
		pathexp, fileexp = os.path.split( entry )
		for path in glob.glob( pathexp ):
			if os.path.exists( path ):
				for file in dircache.listdir( path ):
					if fnmatch.fnmatch( file, fileexp ):
						return ( file, path, searchpath )
	return ( None, None, None )

    def check( self, options ): pass
    def includePath( self ): pass
    def librarySearchPath( self ): pass
    def toolPath( self ): pass
    


if __name__ == "__main__":
	main()
