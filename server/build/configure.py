#!/usr/bin/env python2.3
#################################################################
#   )      (\_     # Wolfpack 13.0.0 Build Script               #
#  ((    _/{  "-;  # Created by: Wolfpack Development Team      #
#   )).-" {{ ;"`   # Revised by: Wolfpack Development Team      #
#  ( (  ;._ \\ ctr # Last Modification: check cvs logs          #
#################################################################

import os
import sys
import glob
import fnmatch
import dircache
import string
from optparse import OptionParser

# These are the variables we are trying to figure out
py_libpath = ""
py_libfile = ""
py_incpath = ""
qt_qmake = ""
mysql_libpath = ""
mysql_libfile = ""
mysql_incpath = ""

# Color codes ( borrowed from Gentoo's portage code )
colorcodes = {}
colorcodes["reset"] = "\x1b[0m"
colorcodes["bold"] = "\x1b[01m"
colorcodes["green"] = "\x1b[32;01m"
colorcodes["red"] = "\x1b[31;01m"
colorcodes["darkred"] = "\x1b[31;06m"

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


def buildLibLine( path, file ):
	
	if path == "" or file == "":
		return ""
	
	if sys.platform == "win32":
		return path + os.path.pathsep + file
	else:
		if file[0:3] == "lib":
			file = file[3:]
		if file[-3:] == ".so":
			file = file[0:-3]
		result = "-L%s -l%s" % ( path, file )
		return result

def findFile( searchpath ):
	path = ""
	file = ""
	for entry in searchpath:
		pathexp, fileexp = os.path.split( entry )
		for path in glob.glob(pathexp):
			if os.path.exists(path):
				for file in dircache.listdir(path):
					if fnmatch.fnmatch( file, fileexp ):
						return (file, path)
	return (None, None)


def checkQt():

	if sys.platform == "win32":
		QMAKE_EXECUTABLE = "qmake.exe"
	else:
		QMAKE_EXECUTABLE = "qmake"

	sys.stdout.write( "Checking QTDIR enviroment variable..." )
	if ( os.environ.has_key("QTDIR") and os.path.exists( os.environ["QTDIR"] ) ):
		sys.stdout.write( "ok\n" )
	else:
		sys.stdout.write( red("failed") + "\n" )
		sys.stdout.write( "You must properly setup QTDIR" )
		sys.exit();
	sys.stdout.write( "Searching for qmake..." )
	temp = ""

	QMAKESEARCHPATH = [ os.path.join(os.path.join(os.environ["QTDIR"], "bin"), QMAKE_EXECUTABLE) ]
	for dir in string.split( os.environ["PATH"], os.path.pathsep ):
		QMAKESEARCHPATH.append( os.path.join(dir, QMAKE_EXECUTABLE) )

	qmake_file, qmake_path = findFile(QMAKESEARCHPATH)
	global qt_qmake;
	qt_qmake = os.path.join(qmake_path, qmake_file)
	sys.stdout.write( "%s\n" % qt_qmake )

	return True

def checkMySQL(options):
	if sys.platform == "win32":
		MySQL_LIBSEARCHPATH = [ sys.prefix + "\Libs\mysqlclient*.lib" ]
		MySQL_INCSEARCHPATH = [ sys.prefix + "\include\mysql.h" ]
	elif sys.platform == "linux2":
		MySQL_LIBSEARCHPATH = [ "/usr/local/lib/mysql/libmysqlclient*.so", \
					"/usr/lib/mysql/libmysqlclient*.so", \
					"/usr/local/lib/libmysqlclient*.so", \
					"/usr/lib/libmysqlclient*.so" ]
		MySQL_LIBSTATICSEARCHPATH = [ "/usr/local/lib/mysql/libmysqlclient*.a", \
					"/usr/lib/mysql/libmysqlclient*.a", \
					"/usr/local/lib/libmysqlclient*.a", \
					"/usr/lib/libmysqlclient*.a" ]
		MySQL_INCSEARCHPATH = [ "/usr/local/include/mysql/mysql.h", \
					"/usr/include/mysql/mysql.h", \
					"/usr/local/include/mysql.h", \
					"/usr/include/mysql.h" ]
	elif sys.platform == "freebsd4" or sys.platform == " freebsd5":
		MySQL_LIBSEARCHPATH = [ "/usr/local/lib/mysql/libmysqlclient*.so", \
					"/usr/lib/mysql/libmysqlclient*.so", \
					"/usr/local/lib/libmysqlclient*.so", \
					"/usr/lib/libmysqlclient*.so" ]
		MySQL_LIBSTATICSEARCHPATH = [ "/usr/local/lib/mysql/libmysqlclient*.a", \
					"/usr/lib/mysql/libmysqlclient*.a", \
					"/usr/local/lib/libmysqlclient*.a", \
					"/usr/lib/libmysqlclient*.a" ]
		MySQL_INCSEARCHPATH = [ "/usr/local/include/mysql/mysql.h", \
					"/usr/include/mysql/mysql.h", \
					"/usr/local/include/mysql.h", \
					"/usr/include/mysql.h" ]
	else:
		sys.stdout.write("ERROR: Unknown platform %s to checkMySQL()\n" % sys.platform )
		sys.exit()

	# if --static
	if options.staticlink:
		MySQL_LIBSEARCHPATH = MySQL_LIBSTATICSEARCHPATH

	global mysql_libpath
	global mysql_libfile

	mysql_libfile, mysql_libpath = findFile( MySQL_LIBSEARCHPATH )
	if ( mysql_libfile ):
		sys.stdout.write("%s\n" % os.path.join( py_libpath, py_libfile ) )
	else:
		sys.stdout.write("Not Found!\n")
		sys.exit()

	global mysql_incpath
	mysql_incfile = None
	sys.stdout.write( "Searching for MySQL includes... " )
	mysql_incfile, mysql_incpath = findFile( MySQL_INCSEARCHPATH )
	if ( mysql_incfile ):
		sys.stdout.write( "%s\n" % mysql_incpath )
	else:
		sys.stdout.write("Not Found!\n")
		sys.exit()

	return True

def checkPython(options):
	if sys.platform == "win32":
		PYTHONLIBSEARCHPATH = [ sys.prefix + "\Libs\python*.lib" ]
		PYTHONINCSEARCHPATH = [ sys.prefix + "\include\Python.h" ]
	elif sys.platform == "linux2":
		PYTHONLIBSEARCHPATH = [ "/usr/local/lib/libpython2.3*.so", \
					 "/usr/local/lib/[Pp]ython*/libpython2.3*.so", \
					 "/usr/lib/libpython2.3*.so", \
					 "/usr/lib/[Pp]ython*/libpython2.3*.so", \
					 "/usr/lib/[Pp]ython*/config/libpython2.3*.so", \
					 "/usr/local/lib/[Pp]ython*/config/libpython2.3*.so"]
		PYTHONLIBSTATICSEARCHPATH = [ "/usr/local/lib/libpython2.3*.a", \
					 "/usr/local/lib/[Pp]ython2.3*/libpython2.3*.a", \
					 "/usr/lib/libpython2.3*.a", \
					 "/usr/lib/[Pp]ython2.3*/libpython2.3*.a", \
					 "/usr/lib/[Pp]ython2.3*/config/libpython2.3*.a", \
					 "/usr/local/lib/[Pp]ython2.3*/config/libpython2.3*.a"]
		PYTHONINCSEARCHPATH = [ "/usr/local/include/[Pp]ython2.3*/Python.h", \
					 "/usr/include/[Pp]ython2.3*/Python.h"]
	elif sys.platform == "freebsd4" or sys.platform == "freebsd5":
		PYTHONLIBSEARCHPATH = [ "/usr/local/lib/libpython2.3*.so", \
					 "/usr/local/lib/[Pp]ython2.3*/libpython2.3*.so", \
					 "/usr/lib/libpython2.3*.so", \
					 "/usr/lib/[Pp]ython2.3*/libpython2.3*.so", \
					 "/usr/lib/[Pp]ython2.3*/config/libpython2.3*.so", \
					 "/usr/local/lib/[Pp]ython2.3*/config/libpython2.3*.so"]
		PYTHONLIBSTATICSEARCHPATH = [ "/usr/local/lib/libpython2.3*.a", \
					 "/usr/local/lib/[Pp]ython2.3*/libpython2.3*.a", \
					 "/usr/lib/libpython2.3*.a", \
					 "/usr/lib/[Pp]ython2.3*/libpython2.3*.a", \
					 "/usr/lib/[Pp]ython2.3*/config/libpython2.3*.a", \
					 "/usr/local/lib/[Pp]ython2.3*/config/libpython2.3*.a"]
		PYTHONINCSEARCHPATH = [ "/usr/local/include/[Pp]ython2.3*/Python.h", \
					 "/usr/include/[Pp]ython2.3*/Python.h"]
	elif sys.platform == "darwin":
		PYTHONINCSEARCHPATH = [ "/System/Library/Frameworks/Python.framework/Versions/Current/Headers/Python.h" ]
		PYTHONLIBSEARCHPATH = [ ]
		PYTHONLIBSTATICSEARCHPATH = []
	else:
		sys.stdout.write(red("ERROR")+": Unknown platform %s to checkPython()\n" % sys.platform )
		sys.exit()

	# if --static
	if options.staticlink:
		PYTHONLIBSEARCHPATH = PYTHONLIBSTATICSEARCHPATH

	# if it was overiden...
	if options.py_incpath:
		PYTHONINCSEARCHPATH = [ options.py_incpath ]
	if options.py_libpath:
		PYTHONLIBSEARCHPATH = [ options.py_libpath ]

	sys.stdout.write( "Checking Python version... " )
	if sys.hexversion >= 0x020300F0:
		sys.stdout.write("ok\n")
	else:
		sys.stdout.write( red("failed") + "\n" )
		sys.stdout.write( bold("Wolfpack requires Python version greater than 2.3.0 ") )
		sys.exit();


	sys.stdout.write( "Checking CPU byte order... %s\n" % sys.byteorder )
	if sys.byteorder != 'little':
		sys.stdout.write("\nError: Wolfpack currently only supports little endian systems\n" )
		sys.exit();

	sys.stdout.write( "Searching for Python library... " )

	global py_libpath
	global py_libfile

	py_libfile, py_libpath = findFile( PYTHONLIBSEARCHPATH )
	if ( py_libfile ):
		sys.stdout.write("%s\n" % os.path.join( py_libpath, py_libfile ) )
	else:
		sys.stdout.write(red("Not Found!") + "\n")
		sys.exit()

	global py_incpath
	py_incfile = None
	sys.stdout.write( "Searching for Python includes... " )
	py_incfile, py_incpath = findFile( PYTHONINCSEARCHPATH )
	if ( py_incfile ):
		sys.stdout.write( "%s\n" % py_incpath )
	else:
		sys.stdout.write(red("Not Found!") + "\n")
		sys.exit()

	return True

# Entry point
def main():
	# Options for qmake
	DEFINES = ""
	CONFIG = ""

	# Setup command line parser
	parser = OptionParser(version="%prog 0.1")
	parser.add_option("--dsp", action="store_true", dest="dsp", help="also Generate Visual Studio project files")
	parser.add_option("--nocolor", action="store_true", dest="nocolor", help="disable color output support on this script")
	parser.add_option("--python-includes",  dest="py_incpath", help="Python include directory")
	parser.add_option("--python-libraries", dest="py_libpath", help="Python library path")
	parser.add_option("--qt-directory", dest="qt_dir", help="Base directory of Qt")
	parser.add_option("--static", action="store_true", dest="staticlink", help="Build wokfpack using static libraries")
	parser.add_option("--enable-debug", action="store_true", dest="enable_debug", help="Enables basic debugging support.")
	parser.add_option("--enable-aidebug", action="store_true", dest="enable_aidebug", help="Enabled debugging of NPC AI.")
	parser.add_option("--enable-mysql", action="store_true", dest="enable_mysql", help="Enables MySQL support.")
	(options, args) = parser.parse_args()

	if options.nocolor:
		nocolor()

	checkPython(options)
	if options.enable_mysql:
		CONFIG += "mysql "
		DEFINES += "MYSQL_DRIVER "
		checkMySQL(options)
	checkQt()

	# Create config.pri
	global py_libpath
	global py_libfile
	global py_incpath
	global qt_qmake
	global mysql_libpath
	global mysql_libfile
	global mysql_incpath

	config = file("config.pri", "w")
	config.write("# WARNING: This file was automatically generated by configure.py\n")
	config.write("#          any changes to this file will be lost!\n")

	# Build Python LIBS and Includes
	PY_LIBDIR = buildLibLine( py_libpath, py_libfile ) 
	config.write("PY_INCDIR = %s\n" % ( py_incpath ) )
	config.write("PY_LIBDIR = %s\n" % PY_LIBDIR)


	# Build MySQL Libs and Includes
	MySQL_LIBDIR = buildLibLine( mysql_libpath, mysql_libfile )
	config.write("MySQL_INCDIR = %s\n" % mysql_incpath )
	config.write("MySQL_LIBDIR = %s\n" % MySQL_LIBDIR )

	# if --debug
	if options.enable_debug:
		DEFINES += "_DEBUG "
		CONFIG += "debug "
	# if --aidebug
	if options.enable_aidebug:
		DEFINES += "_AIDEBUG "

	config.write("DEFINES += %s\n" % DEFINES)
	config.write("CONFIG += %s\n" % CONFIG)
	config.write("LIBS += $$PY_LIBDIR $$MySQL_LIBDIR \n")
	config.write("INCLUDEPATH += $$PY_INCDIR $$MySQL_INCDIR \n")
	config.close()

	sys.stdout.write(green("Generating makefile..."))
	sys.stdout.flush()
	os.spawnv(os.P_WAIT, qt_qmake, [qt_qmake, "wolfpack.pro"])
	if options.dsp:
		sys.stdout.write("Generating Visual Studio project files...\n")
		os.spawnv(os.P_WAIT, qt_qmake, [qt_qmake, "wolfpack.pro", "-t vcapp"])
	sys.stdout.write(bold("Done\n"))
	sys.stdout.write(bold("Configure finished. Please run 'make' now.\n"))

if __name__ == "__main__":
	main()
