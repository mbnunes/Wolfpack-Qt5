#!/usr/bin/env python
#################################################################
#   )      (\_     # Wolfpack 13.0.0 Build Script               #
#  ((    _/{  "-;  # Created by: Wolfpack Development Team      #
#   )).-" {{ ;"`   # Revised by: Wolfpack Development Team      #
#  ( (  ;._ \\ ctr # Last Modification: check svn logs          #
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
import re

try:
    import distutils.sysconfig
except:
    sys.stdout.write("Invalid Python installation. It's missing the distutils package\n")
    sys.stdout.write("If you are using linux, check for python dev packages, or consider switching to some other distro without morron packagers" )
    sys.exit( 1 )


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

def get_compiler():
	from distutils.ccompiler import new_compiler
	qmakespec = os.environ.get('QMAKESPEC')
	if not qmakespec:
		return new_compiler()
	else:
		if qmakespec.startswith('win32-msvc'):
			return new_compiler( 'nt', 'msvc' )
		elif qmakespec == "win32-g++":
			return new_compiler( 'nt', 'mingw32' )
		elif qmakespec == "win32-borland":
			return new_compiler( 'nt', 'bcpp' )
		elif qmakespec == "macx-mwerks":
			return new_compiler( 'posix', 'mwerks' )
		else:
			return new_compiler( None, 'unix' )


class AbstractExternalLibrary:
	def __init__( self, output = sys.stdout ):
		self.output = output
		self.incPath = ""
		self.libSearchPath = ""
		self._toolPath = ""
		self.libs = []
    
	def out( self, message ):
		self.output.write( message )

	def findFile( self, searchpath ):
		"""Locates a file specified in the searchpath structure, returning a tuple
			containing filename, path, non-searched entries in the searchpath.
		"""
		path = ""
		file = ""
		searched = set()
		for entry in searchpath:
			searched.add(entry)
			pathexp, fileexp = os.path.split( entry )
			for path in glob.glob( pathexp ):
				if os.path.exists( path ):
					for file in os.listdir( path ):
						if fnmatch.fnmatch( file, fileexp ):
							return ( file, path, set(searchpath) - searched )
		return ( None, None, None )

	def find_library_file( self, dirs, lib, debug = False, static = False ):
		"""Locates a library in the specified dirs, guessing the local filename based on library name"""
		compiler = get_compiler()
		shared_f = compiler.library_filename( lib, lib_type='shared' )
		static_f = compiler.library_filename( lib, lib_type='static' )
		dylib_f = None
		try:
			dylib_f = compiler.library_filename( lib, lib_type='dylib' )
		except:
			pass

		for pathexp in dirs:
			for dir in glob.glob( pathexp ):
				shared = os.path.join(dir, shared_f)
				if dylib_f:
					dylib = os.path.join(dir, dylib_f)
				else:
					dylib = None
				static = os.path.join(dir, static_f)
                # We're second-guessing the linker here, with not much hard
                # data to go on: GCC seems to prefer the shared library, so I'm
                # assuming that *all* Unix C compilers do.  And of course I'm
                # ignoring even GCC's "-static" option.  So sue me.
				if not static and dylib and os.path.exists(dylib):
					return dir
				elif not static and os.path.exists(shared):
					return dir
				elif os.path.exists(static):
					return dir

		# Can't find it
		return None


	def check( self, options ):
		raise NotImplementedError
    
	def includePath( self ):
		return self.incPath

	def librarySearchPath( self ): 
		return self.libSearchPath

	def toolPath( self ): 
		return self._toolPath
    
	def libraryFiles( self ):
		return self.libs


class PythonLibrary( AbstractExternalLibrary ):
	def __init__( self, minversion ):
		AbstractExternalLibrary.__init__( self )
		self.minversion = minversion

	def check( self, options ):
		self.out( "Checking Python Configuration:\n" )

		# Default Blank
		PYTHONLIBSEARCHPATH = []
		PYTHONINCSEARCHPATH = []
		# Attept to find the system's configuration
		PYTHONINCSEARCHPATH = [ distutils.sysconfig.get_python_inc() + os.path.sep + "Python.h" ]

		CONFIGVARS = distutils.sysconfig.get_config_vars()
		if "DESTSHARED" in CONFIGVARS:
			PYTHONLIBSEARCHPATH = [ CONFIGVARS["DESTSHARED"] ]
		if "DESTLIB" in CONFIGVARS:
			PYTHONLIBSEARCHPATH.append( CONFIGVARS["DESTLIB"] )

		# Windows Search Paths
		if sys.platform == "win32":
			PYTHONLIBSEARCHPATH += [ sys.prefix + "\libs" ]
			PYTHONINCSEARCHPATH += [ sys.prefix + "\include\Python.h" ]
		# Linux and BSD Search Paths
		elif sys.platform in ("linux2", "freebsd4", "freebsd5", "openbsd3"):

			if "MULTIARCH" in CONFIGVARS and "DESTLIB" in CONFIGVARS:
				PYTHONLIBSEARCHPATH.append( CONFIGVARS["DESTLIB"] + "/config-" + CONFIGVARS["MULTIARCH"] )

			PYTHONLIBSEARCHPATH += [ \
				# Python 2.6 - Look for this first
				"/usr/local/lib/", \
				"/usr/local/lib/[Pp]ython*/", \
				"/usr/local/lib/[Pp]ython*/config/", \
				"/usr/lib/", \
				"/usr/lib/[Pp]ython*/", \
				"/usr/lib/[Pp]ython*/config/" ]
			PYTHONINCSEARCHPATH += [ \
				"/usr/local/include/Python.h", \
				"/usr/include/Python.h", \
				# Python 2.7
				"/usr/local/include/[Pp]ython2.7*/Python.h", \
				"/usr/include/[Pp]ython2.7*/Python.h" \
				# Python 2.6
				"/usr/local/include/[Pp]ython2.6*/Python.h", \
				"/usr/include/[Pp]ython2.6*/Python.h" \
				# Python 2.4
				"/usr/local/include/[Pp]ython2.4*/Python.h", \
				"/usr/include/[Pp]ython2.4*/Python.h" ]

		# MacOSX Search Paths
		elif sys.platform == "darwin":
			PYTHONINCSEARCHPATH += [ \
				"/System/Library/Frameworks/Python.framework/Versions/Current/Headers/Python.h" ]
			PYTHONLIBSEARCHPATH +=  [ \
				"/usr/local/lib/[Pp]ython*/config/", \
				"/System/Library/Frameworks/Python.framework/Versions/Current/Python", \
				"/System/Library/Frameworks/Python.framework/Versions/Current/lib/[Pp]ython*/config/" ]
		# Undefined OS
		else:
			self.out(yellow("  WARNING")+": Unknown platform %s to LibraryPython::check()\nLibrary autosearch might not work properly\n" % sys.platform )

		# if it was overiden...
		if options.py_incpath:
			PYTHONINCSEARCHPATH = [ options.py_incpath ]
		if options.py_libpath:
			PYTHONLIBSEARCHPATH = [ options.py_libpath ]

		self.out( "  Checking CPU byte order:              %s\n" % sys.byteorder )
		if sys.byteorder != 'little':
			self.out(yellow("Warning:") + " Wolfpack support for big endian systems is completely experimental and unlikey to work!\n" )

		self.out( "  Checking Python version:              " )
		if sys.hexversion >= self.minversion:
			self.out(green("Pass\n"))
		else:
			self.out( red("Fail") + "\n" )
			self.out( bold("  Wolfpack requires Python version >= 2.4.0\n") )
			sys.exit(1);

		# Search for python
		self.out( "  Searching for Python includes:        " )
		filename, path, searchpath = self.findFile( PYTHONINCSEARCHPATH )
		if filename:
			self.out( "%s\n" % path )
			self.incPath = path
		else:
			self.out(red("Not Found!") + "\n")

		if sys.platform != "darwin":
			self.out( "  Searching for Python library:         " )
			if sys.platform == "win32":
				library24 = "python24"
				library26 = "python26"
			else:
				library24 = "python2.4"
				library26 = "python2.6"
				library27 = "python2.7"
			path = self.find_library_file( PYTHONLIBSEARCHPATH, library27, static = options.staticlink )
			self.libs = [ library27 ]
			if not path:
				path = self.find_library_file( PYTHONLIBSEARCHPATH, library26, static = options.staticlink )
				self.libs = [ library26 ]
			if not path:
				path = self.find_library_file( PYTHONLIBSEARCHPATH, library24, static = options.staticlink )
				self.libs = [ library24 ]

			if path:
				self.out( "%s\n" % path )
				self.libSearchPath = path
			else:
				self.out(red("Not Found!") + "\n")
				sys.exit(1)
		self.out( "\n" )
		return True


class QtLibrary( AbstractExternalLibrary ):
	def __init__( self, minversion ):
		AbstractExternalLibrary.__init__( self )
		self.minversion = minversion
		if sys.platform == "win32":
			self.qmakeExecutable = "qmake.exe"
		else:
			self.qmakeExecutable = "qmake"
        
	def check( self, options ):
		self.out("Checking QT Configuration:\n")
        
		qt_dir = ""
		self.out( "  Checking QT installation:             " )
		if not options.qt_dir:
			if ( "QTDIR" in os.environ and os.path.exists( os.environ["QTDIR"] ) ):
				qt_dir = os.environ["QTDIR"]
				self.out( green("Pass\n") )
				self.out( "  Found value for QTDIR:                %s\n" % qt_dir )
			else:
				self.out( red("Fail") + "\n" )
				self.out( "  You must properly setup the QTDIR environment variable or use --qt-directory parameter!\n" )
				sys.exit( 1 );
		else:
			qt_dir = options.qt_dir
			self.out( "  Manually specified value:             %s\n" % qt_dir )
			if os.path.exists( qt_dir ):
				self.out( green("Pass\n") )
			else:
				self.out( red("Fail") + "\n" )
				self.out( "Specified folder %s doesn't exist\n" % qt_dir )
				sys.exit( 1 )
		self.libSearchPath = qt_dir
		self.out( "  Searching for qmake executable:       " )
		temp = ""

		QMAKESEARCHPATH = [ os.path.join(os.path.join(qt_dir, "bin"), self.qmakeExecutable) ]
		for dir in os.environ["PATH"].split( os.path.pathsep ):
			QMAKESEARCHPATH.append( os.path.join( dir, self.qmakeExecutable ) )

		qmake_file, qmake_path, searchpath = self.findFile(QMAKESEARCHPATH)
		if not qmake_file:
			self.out( red("Fail") + "\n" )
			self.out( "Couldn't find qmake\n" )
			return False

		qt_qmake = os.path.join(qmake_path, qmake_file)
		self._toolPath = qmake_path
		self.out( "%s\n" % qt_qmake )

		# Check Qt Version
		self.out( "  Checking Qt version:                  " )
		import tempfile
		fd, fname = tempfile.mkstemp('', 'qmakerun', text=True)
		os.close( fd )
		os.system( qt_qmake + (" -v > %s" % fname ) )
		f = open( fname, 'rt' )
		lines = f.readlines()
		f.close()
		os.remove( fname )
		if len(lines) < 2:
			self.out( red("Fail") + "\n" )
			self.out( "Couldn't run qmake -v to figure out Qt version\n" )
			return False
		version = lines[1].split()[3]
		if not version:
			self.out( red("Fail") + "\n" )
			self.out( "Unrecognized output from qmake -v\n" )
			return False
		if version >= self.minversion:
			self.out( green("Pass") + "\n" )
		else:
			self.out( red("Fail") + "\n" )
			self.out( "You need Qt version >= 4.0.0\n" )

		self.out("\n")
		return True

	def runQMake( self, projectfile, options ):
		qt_qmake = os.path.join(self.toolPath(), self.qmakeExecutable)
		if options:
			return os.spawnv(os.P_WAIT, qt_qmake, [qt_qmake, projectfile, options])
		else:
			return os.spawnv(os.P_WAIT, qt_qmake, [qt_qmake, projectfile])


def main():
	""" Entry Point """

	parser = OptionParser(version="%prog 1.0")
	parser.add_option("--dsp", action="store_true", dest="dsp", help="also Generate Visual Studio project files")
	parser.add_option("--nocolor", action="store_true", dest="nocolor", help="disable color output support on this script")
	parser.add_option("--python-includes",  dest="py_incpath", help="Python include path")
	parser.add_option("--python-libraries", dest="py_libpath", help="Python library path")
	parser.add_option("--qt-directory", dest="qt_dir", help="Base directory of Qt")
	parser.add_option("--static", action="store_true", dest="staticlink", help="Build wokfpack using static libraries")
	parser.add_option("--enable-debug", action="store_true", dest="enable_debug", help="Enables basic debugging support.")
	parser.add_option("--enable-aidebug", action="store_true", dest="enable_aidebug", help="Enabled debugging of NPC AI.")
	parser.add_option("--enable-translation", action="store_true", dest="enable_translation", help="Enable non-English language support.")
	parser.add_option("--enable-gui", action="store_true", dest="enable_gui", help="Enables Graphic User Interface" )
	parser.add_option("--dry-run", action="store_true", dest="dry_run", help="Run checks without changing config.pri" )
    
	if sys.platform == "win32":
		parser.set_defaults(enable_gui=True)
	else:
		parser.set_defaults(enable_gui=False)
    
	# Now let's parse the options
	(options, args) = parser.parse_args()

	if options.nocolor or sys.platform == "win32":
		nocolor()
    
	checkQt = QtLibrary( "4.0.0" )
	if not checkQt.check( options ):
		sys.exit( 1 )
    
	checkPython = PythonLibrary( 0x020200F0 )
	if not checkPython.check( options ):
		sys.exit( 1 )

	if options.dry_run:
		sys.exit( 0 )
    
	config = open( "config.pri", "wt" )
	config.write("# WARNING: This file was automatically generated by configure.py\n")
	config.write("#          any changes to this file will be lost!\n\n")

	# Build Python LIBS and Includes
	PY_LIBDIR = ""
	CONFIG = ""
	DEFINES = ""
	if sys.platform == "darwin":
		# MacPython is build as a Framework, not a library :/
		PY_LIBDIR = distutils.sysconfig.get_config_vars("LINKFORSHARED")[0]
		PY_LIBDIR += " -flat_namespace"
	else:
		PY_LIBDIR = buildLibLine( checkPython.librarySearchPath(), checkPython.libraryFiles()[0] )
		if sys.platform != "win32":
			for lib in distutils.sysconfig.get_config_vars("LIBS"):
				PY_LIBDIR += " %s" % lib
	config.write("PY_LIBDIR = %s\n" % PY_LIBDIR)
	config.write("PY_INCDIR = %s\n" % checkPython.includePath() )

	# QT stuff
	#config.write("QTDIR = %s\n" % checkQt.librarySearchPath() )

	# if --debug
	sys.stdout.write("Build mode:                             ")
	if options.enable_debug:
		DEFINES += "_DEBUG "
		CONFIG += "debug warn_on "
		sys.stdout.write("Debug\n")
	else:
		CONFIG += "release warn_off "
		sys.stdout.write("Release\n")

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
	exitcode = checkQt.runQMake("wolfpack.pro", None)
	if exitcode != 0:
		sys.stdout.write("Error generating Makefile!\n")
		sys.exit(1)        

	if options.dsp:
		sys.stdout.write("Generating Visual Studio project files... \n")
		checkQt.runQMake( "wolfpack.pro", "-t vcapp" )
	sys.stdout.write(bold(green("Done\n")))
	sys.stdout.write(bold("Configure finished. Please run 'make' now.\n"))
	sys.stdout.write("To reconfigure, run 'make confclean' and configure2.py\n")
	sys.stdout.write("\n")

if __name__ == "__main__":
	main()
