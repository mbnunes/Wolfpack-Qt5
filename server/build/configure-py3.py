#!/usr/bin/env python3
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

# These are the variables we are trying to figure out
py_libpath = ""
py_libfile = ""
py_incpath = ""
qt_qmake = ""
qt_dir = ""
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

def buildLibLine( path, file ):

	if path == "" or file == "":
		return ""

	if file[0:3] == "lib":
		file = file[3:]
	if file[-3:] == ".so":
		file = file[0:-3]
	if file[-2:] == ".a":
		file = file[0:-2]
	if file[-4:] == ".lib":
		file = file[0:-4]
	result = "-L\"%s\" -l%s" % ( path, file )
	return result

def findFile( searchpath ):
	path = ""
	file = ""
	for entry in searchpath:
		pathexp, fileexp = os.path.split( entry )
		for path in glob.glob( pathexp ):
			if os.path.exists( path ):
				for file in os.listdir( path ):
						if fnmatch.fnmatch(file, fileexp):
								return (file, path)
	return ( None, None )


def checkQt(options):
	sys.stdout.write("Checking QT Configuration:\n")
	if sys.platform == "win32":
		QMAKE_EXECUTABLE = "qmake.exe"
	else:
		QMAKE_EXECUTABLE = "qmake"

	global qt_dir

	sys.stdout.write( "  Checking QT installation:             " )
	if not options.qt_dir:
		if ( "QTDIR" in os.environ and os.path.exists( os.environ["QTDIR"] ) ):
				qt_dir = os.environ["QTDIR"]
				sys.stdout.write( green("Pass\n") )
				sys.stdout.write( "  Found value for QTDIR:                %s\n" % qt_dir)
		else:
				sys.stdout.write( red("Fail") + "\n")
				sys.stdout.write( "  You must properly setup the QTDIR environment variable or use --qt-directory parameter!\n")
				sys.exit();
	else:
		qt_dir = options.qt_dir
		sys.stdout.write( green("Pass\n") )
		sys.stdout.write( "  Manually specified value:             %s\n" % qt_dir )

	sys.stdout.write( "  Verifying QT installation:            " )
	if ( qt_dir and qt_dir != "" and os.path.exists( qt_dir ) ):
		sys.stdout.write( green("Pass\n") )
	else:
		sys.stdout.write( red("Fail") + "\n" )
		sys.stdout.write( "  You must properly setup the QTDIR environment variable or use --qt-directory parameter!\n" )
		sys.exit(1);
	sys.stdout.write( "  Searching for qmake executable:       " )
	temp = ""

	QMAKESEARCHPATH = [ os.path.join(os.path.join(qt_dir, "bin"), QMAKE_EXECUTABLE) ]
	for dir in str.split( os.environ["PATH"], os.path.pathsep ):
		QMAKESEARCHPATH.append( os.path.join( dir, QMAKE_EXECUTABLE ) )

	qmake_file, qmake_path = findFile(QMAKESEARCHPATH)
	if not qmake_file:
		sys.stdout.write( red("Fail") + "\n" )
		sys.stdout.write( "Couldn't find qmake" )
		return False
	global qt_qmake;
	qt_qmake = os.path.join(qmake_path, qmake_file)
	sys.stdout.write( "%s\n" % qt_qmake )
	sys.stdout.write("\n")
	return True

def checkPython( options, lookForHeaders, lookForLib ):
	sys.stdout.write("Checking Python Configuration:\n")
	# Default Blank
	PYTHONLIBSEARCHPATH = []
	PYTHONLIBSTATICSEARCHPATH = []
	PYTHONINCSEARCHPATH = []
	# Attept to find the system's configuration
	PYTHONINCSEARCHPATH = [ distutils.sysconfig.get_python_inc() + os.path.sep + "Python.h" ]

	if "DESTSHARED" in distutils.sysconfig.get_config_vars():
		PYTHONLIBSEARCHPATH = [ distutils.sysconfig.get_config_vars()["DESTSHARED"] + os.path.sep + "libpython*" ]

	# Windows Search Paths
	if sys.platform == "win32":
		PYTHONLIBSEARCHPATH += [ sys.prefix + "\Libs\python*.lib" ]
		PYTHONINCSEARCHPATH += [ sys.prefix + "\include\Python.h" ]
		PYTHONLIBSTATICSEARCHPATH += [ sys.prefix + "\Libs\python*.lib" ]
	# Linux and BSD Search Paths
	elif sys.platform in ("linux2", "freebsd4", "freebsd5", "linux"):
		PYTHONLIBSEARCHPATH += [ \
			# Python 3.8
			"/usr/lib/python3.8/config-3.8-x86_64-linux-gnu/libpython3.8*.so", \
			"/usr/local/lib/[Pp]ython*/libpython3.8*.so", \
			"/usr/local/lib/[Pp]ython*/config/libpython3.8*.so", \
			"/usr/lib/x86_64-linux-gnu/libpython3.8*.so", \
			"/usr/lib/[Pp]ython*/libpython3.8*.so", \
			"/usr/lib/[Pp]ython*/config/libpython3.8*.so", \
			# Python 3.7
			"/usr/lib/python3.7/config-3.7-x86_64-linux-gnu/libpython3.7*.so", \
			"/usr/local/lib/[Pp]ython*/libpython3.7*.so", \
			"/usr/local/lib/[Pp]ython*/config/libpython3.7*.so", \
			"/usr/lib/x86_64-linux-gnu/libpython3.7*.so", \
			"/usr/lib/[Pp]ython*/libpython3.7*.so", \
			"/usr/lib/[Pp]ython*/config/libpython3.7*.so", \
			# Python 3.6
			"/usr/lib/python3.6/config-3.6m-x86_64-linux-gnu/libpython3.6*.so", \
			"/usr/local/lib/[Pp]ython*/libpython3.6*.so", \
			"/usr/local/lib/[Pp]ython*/config/libpython3.6*.so", \
			"/usr/lib/x86_64-linux-gnu/libpython3.6*.so", \
			"/usr/lib/[Pp]ython*/libpython3.6*.so", \
			"/usr/lib/[Pp]ython*/config/libpython3.6*.so" ]
		PYTHONLIBSTATICSEARCHPATH += [ \
			# Python 3.8
			"/usr/lib/python3.8/config-3.8-x86_64-linux-gnu/libpython3.8*.a", \
			"/usr/local/lib/[Pp]ython3.8*/libpython3.8*.a", \
			"/usr/local/lib/[Pp]ython3.8*/config/libpython3.8*.a", \
			"/usr/lib/x86_64-linux-gnu/libpython3.8*.a", \
			"/usr/lib/[Pp]ython3.8*/libpython3.8*.a", \
			"/usr/lib/[Pp]ython3.8*/config/libpython3.8*.a", \
			# Python 3.7
			"/usr/lib/python3.7/config-3.7-x86_64-linux-gnu/libpython3.7*.a", \
			"/usr/local/lib/[Pp]ython3.67*/libpython3.7*.a", \
			"/usr/local/lib/[Pp]ython3.7*/config/libpython3.7*.a", \
			"/usr/lib/x86_64-linux-gnu/libpython3.7*.a", \
			"/usr/lib/[Pp]ython3.7*/libpython3.7*.a", \
			"/usr/lib/[Pp]ython3.7*/config/libpython3.7*.a", \
			# Python 3.6
			"/usr/lib/python3.6/config-3.6m-x86_64-linux-gnu/libpython3.6*.a", \
			"/usr/local/lib/[Pp]ython3.6*/libpython3.6*.a", \
			"/usr/local/lib/[Pp]ython3.6*/config/libpython3.6*.a", \
			"/usr/lib/x86_64-linux-gnu/libpython3.6*.a", \
			"/usr/lib/[Pp]ython3.6*/libpython3.6*.a", \
			"/usr/lib/[Pp]ython3.6*/config/libpython3.6*.a" ]
		PYTHONINCSEARCHPATH += [ \
			"/usr/local/include/Python.h", \
			"/usr/include/Python.h", \
			# Python 3.8
			"/usr/include/python3.8/Python.h", \
			"/usr/include/[Pp]ython3.8*/Python.h", \
			# Python 3.7
			"/usr/include/python3.7/Python.h", \
			"/usr/include/[Pp]ython3.7*/Python.h", \
			# Python 3.6
			"/usr/include/python3.6m/Python.h", \
			"/usr/include/[Pp]ython3.6*/Python.h" ]
	# MacOSX Search Paths
	elif sys.platform == "darwin":
		PYTHONINCSEARCHPATH += [ \
			"/System/Library/Frameworks/Python.framework/Versions/Current/Headers/Python.h" ]
		PYTHONLIBSEARCHPATH += [ ]
		PYTHONLIBSTATICSEARCHPATH += [ \
			"/usr/local/lib/[Pp]ython*/config/libpython*.a", \
			"/System/Library/Frameworks/Python.framework/Versions/Current/Python", \
			"/System/Library/Frameworks/Python.framework/Versions/Current/lib/[Pp]ython*/config/libpython*.a" ]
	# Undefined OS
	else:
		sys.stdout.write(red("ERROR")+": Unknown platform %s to checkPython()\n" % sys.platform )
		sys.exit(1)

	# if --static
	if options.staticlink:
		PYTHONLIBSEARCHPATH = None
		PYTHONLIBSEARCHPATH = PYTHONLIBSTATICSEARCHPATH

	# if it was overiden...
	if options.py_incpath:
		PYTHONINCSEARCHPATH = None
		PYTHONINCSEARCHPATH = [ options.py_incpath ]
	if options.py_libpath:
		PYTHONLIBSEARCHPATH = None
		PYTHONLIBSEARCHPATH = [ options.py_libpath ]

	sys.stdout.write( "  Checking Python version:              " )
	if sys.hexversion >= 50727408:
		sys.stdout.write(green("Pass\n"))
	else:
		sys.stdout.write( red("Fail") + "\n" )
		sys.stdout.write( bold("  Wolfpack requires Python version >= 3.6\n") )
		sys.exit(1)

	sys.stdout.write( "  Checking CPU byte order:              %s\n" % sys.byteorder )
	if sys.byteorder != 'little':
		sys.stdout.write(yellow("Warning:") + " Wolfpack support for big endian systems is completely experimental and unlikey to work!\n" )

	if lookForHeaders:
		global py_incpath
		global py_incfile
		sys.stdout.write( "  Searching for Python includes:        " )
		py_incfile, py_incpath = findFile( PYTHONINCSEARCHPATH )
		if ( py_incfile ):
			sys.stdout.write( "%s\n" % py_incpath )
		else:
			sys.stdout.write(red("Not Found!") + "\n")
			sys.exit(1)

	if lookForLib:
		sys.stdout.write( "  Searching for Python library:         " )
		global py_libpath
		global py_libfile
		py_libfile, py_libpath = findFile( PYTHONLIBSEARCHPATH )
		if ( py_libfile ):
			sys.stdout.write( "%s\n" % os.path.join( py_libpath, py_libfile ) )
		else:
			sys.stdout.write(red("Not Found!") + "\n")
			sys.exit(1)

	sys.stdout.write("\n")
	return True

# Entry point
def main():
	# Options for qmake
	DEFINES = ""
	CONFIG = ""
	# Setup command line parser
	parser = OptionParser(version="%prog 0.5")
	parser.add_option("--dsp", action="store_true", dest="dsp", help="also Generate Visual Studio project files")
	parser.add_option("--nocolor", action="store_true", dest="nocolor", help="disable color output support on this script")
	parser.add_option("--python-includes",  dest="py_incpath", help="Python include path")
	parser.add_option("--python-libraries", dest="py_libpath", help="Python library path")
	parser.add_option("--qt-directory", dest="qt_dir", help="Base directory of Qt")
	parser.add_option("--static", action="store_true", dest="staticlink", help="Build wolfpack using static libraries")
	parser.add_option("--enable-debug", action="store_true", dest="enable_debug", help="Enables basic debugging support.")
	parser.add_option("--enable-aidebug", action="store_true", dest="enable_aidebug", help="Enabled debugging of NPC AI.")
	parser.add_option("--enable-translation", action="store_true", dest="enable_translation", help="Enable non-English language support.")
	parser.add_option("--enable-gui", action="store_true", dest="enable_gui", help="Enables Graphic User Interface" )

	if sys.platform == "win32":
		parser.set_defaults(enable_gui=True)
	else:
		parser.set_defaults(enable_gui=False)

	# Now let's parse the options
	(options, args) = parser.parse_args()

	if options.nocolor or sys.platform == "win32":
		nocolor()

	pyIncSearch = True
	pyLibSearch = True
	if sys.platform == "darwin":
		pyLibSearch = False

	# Neat Header
	#headerbuffer = "                                        "
	#sys.stdout.write( " \n" )
	#sys.stdout.write( headerbuffer + red( "  )      (\_     ") + "\n" )
	#sys.stdout.write( headerbuffer + red( " ((    _/{  \"-;  ") + "\n" )
	#sys.stdout.write( headerbuffer + red( "  )).-\" {{ ;\"`   ") + "\n" )
	#sys.stdout.write( "Running Wolfpack Configuration:         " )
	#sys.stdout.write( red( " ( (  ;._ \\\\     " ) + "\n" )
	#sys.stdout.write( headerbuffer + green( "****************\n" ) )

	# Check QT Settings
	checkQt (options)
	# Check Python Settings
	checkPython( options, pyIncSearch, pyLibSearch )

	# Create config.pri
	global py_libpath
	global py_libfile
	global py_incpath
	global qt_qmake
	global qt_dir
	global mysql_libpath
	global mysql_libfile
	global mysql_incpath

	config = open("config.pri", "w")
	config.write("# WARNING: This file was automatically generated by configure.py\n")
	config.write("#          any changes to this file will be lost!\n\n")

	# Build Python LIBS and Includes
	if sys.platform == "darwin":
		# MacPython is build as a Framework, not a library :/
		PY_LIBDIR = distutils.sysconfig.get_config_vars("LINKFORSHARED")[0]
		PY_LIBDIR += " -flat_namespace"
	else:
		PY_LIBDIR = buildLibLine( py_libpath, py_libfile ).replace("\\","/")
	config.write("PY_LIBDIR = %s\n" % PY_LIBDIR)
	config.write("PY_INCDIR = %s\n" % py_incpath.replace("\\","/"))

	# QT stuff
	config.write("QTDIR = %s\n" % qt_dir.replace("\\","/") )

	# if --debug
	sys.stdout.write("Build mode:                             ")
	if options.enable_debug:
		DEFINES += "_DEBUG "
		CONFIG += "debug warn_on "
		sys.stdout.write("Debug, ")
	else:
		CONFIG += "release warn_off "
		sys.stdout.write("Release, ")

	# if --static
	if options.staticlink:
		sys.stdout.write("Static Libraries\n")
	else:
		DEFINES += "Py_ENABLE_SHARED "
		sys.stdout.write("Shared Libraries\n")

	# if --enable-gui
	sys.stdout.write("GUI:                                    ")
	if not options.enable_gui:
		CONFIG += "console"
		sys.stdout.write("Disabled\n")
	else:
		sys.stdout.write("Enabled\n")

	# if --enable-translations
	sys.stdout.write("Translation:                            ")
	if not options.enable_translation:
		DEFINES += "QT_NO_TRANSLATION "
		sys.stdout.write("Disabled\n")
	else:
		sys.stdout.write("Enabled\n")

	# if --aidebug
	sys.stdout.write("AI debugging:                           ")
	if options.enable_aidebug:
		DEFINES += "_AIDEBUG "
		sys.stdout.write("Enabled\n")
	else:
		sys.stdout.write("Disabled\n")

	config.write("DEFINES += %s\n" % DEFINES)
	config.write("CONFIG += %s\n" % CONFIG)
	config.write("LIBS += $$PY_LIBDIR $$MySQL_LIBDIR \n")
	config.write("INCLUDEPATH += $$PY_INCDIR $$MySQL_INCDIR \n")
	config.close()

	sys.stdout.write("\nGenerating makefile... ")
	sys.stdout.flush()
	exitcode = os.spawnv(os.P_WAIT, qt_qmake, [qt_qmake, "wolfpack.pro"])

	if exitcode != 0:
		sys.stdout.write("Error generating Makefile!\n")
		sys.exit(1)

	if options.dsp:
		sys.stdout.write("Generating Visual Studio project files... \n")
		os.spawnv(os.P_WAIT, qt_qmake, [qt_qmake, "wolfpack.pro", "-t vcapp"])
	sys.stdout.write(bold(green("Done\n")))
	sys.stdout.write(bold("Configure finished. Please run 'make' now.\n"))
	sys.stdout.write("To reconfigure, run /usr/bin/gmake confclean and configure.py\n")
	sys.stdout.write("\n")

if __name__ == "__main__":
	main()
