#!/usr/bin/env python
#################################################################
#   )      (\_     # Wolfpack 13.0.0 Build Script               #
#  ((    _/{  "-;  # Created by: Wolfpack Development Team      #
#   )).-" {{ ;"`   # Revised by: Wolfpack Development Team      #
#  ( (  ;._ \\ ctr # Last Modification: check cvs logs          #
#################################################################

# This small script will test if the headers are self sufficient
# A good C++ (and C) coding standard is that every header file
# should be self-sufficient.
#
# See "Self-Sufficient Headers", C/C++ Users Journal, October 2003.

import os
import os.path
import sys
import dircache
import fnmatch
import distutils.ccompiler
import distutils.sysconfig

WolfpackPath = "../../src"

includes = []

#for file in dircache.listdir( WolfpackPath ):
#    if fnmatch.fnmatch( file, "*.h" ):
#        includes.append( file )

for root, dirs, files in os.walk( WolfpackPath ):
    for name in files:
        if fnmatch.fnmatch( name, "*.h" ):
            includes.append( os.path.join( root, name ) )

if not os.path.exists( 'work' ):
    os.mkdir( 'work' )
    os.chdir( 'work' )
else:
    os.chdir( 'work' )
#    os.remove(  )

compiler = distutils.ccompiler.new_compiler()
compiler.verbose = 1
compiler.add_include_dir( WolfpackPath )
compiler.add_include_dir( os.getenv('QTDIR') + '/include' )
compiler.add_include_dir( distutils.sysconfig.get_python_inc() )
include = includes[0]

for include in includes:
    if os.path.exists( 'test.cpp' ):
        os.remove( 'test.cpp' )
    if os.path.exists( 'test.obj' ):
        os.remove( 'test.obj' )
    testcpp = open( 'test.cpp', 'w' )
    testcpp.write( "#include \"../%s\"\n" % include )
    testcpp.write( "int main( int argc, char** argv ) { return 0; }\n" )
    testcpp.close()
    try:
        compiler.compile( ["test.cpp"] )
    except:
        print "Error compiling %s" % include
