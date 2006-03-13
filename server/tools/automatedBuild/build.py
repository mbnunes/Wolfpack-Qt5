#!/usr/bin/env python
#################################################################
#   )      (\_     # Wolfpack 13.0.0 Build Script               #
#  ((    _/{  "-;  # Created by: Wolfpack Development Team      #
#   )).-" {{ ;"`   # Revised by: Wolfpack Development Team      #
#  ( (  ;._ \\ ctr # Last Modification: check svn logs          #
#################################################################


import sys, os, zipfile, datetime

QTDIR = 'F:\\Qt-MingW\\4.1.1'
QMAKESPEC = 'win32-g++'
COMPILER_PATH = 'F:\\MingW\\bin'
BUILD_PATH = 'c:\\wolfpack\\server'
MYSQL_DLL_PATH = 'C:\\Arquivos de programas\\MySQL\\MySQL Server 4.1\\lib\\opt'
SVN_SSH = 'plink'

os.environ['QTDIR'] = QTDIR
os.environ['QMAKESPEC'] = QMAKESPEC
os.environ['SVN_SSH'] = SVN_SSH

PATH = os.environ['PATH']
PATH += QTDIR + '\\bin;' + COMPILER_PATH
os.environ['PATH'] = PATH

os.chdir( BUILD_PATH )

for line in os.popen('svn up'):
    print line

os.chdir( BUILD_PATH + os.path.sep + 'build' )

for line in os.popen('mingw32-make distclean'):
    print line

for line in os.popen( 'configure.py' ).readlines():
    print line

for line in os.popen('mingw32-make'):
    print line

fileName = "wolfpack-snapshot-%s.zip" % datetime.date.today()
file = zipfile.ZipFile(fileName, "w")
file.write('..\\release\\wolfpack.exe', 'wolfpack.exe', zipfile.ZIP_DEFLATED )
file.write('..\\release\\ChangeLog', 'ChangeLog', zipfile.ZIP_DEFLATED )
file.write('..\\release\\AUTHORS', 'AUTHORS', zipfile.ZIP_DEFLATED )
file.write('..\\release\\LICENSE.GPL', 'LICENSE.GPL', zipfile.ZIP_DEFLATED )
file.write( MYSQL_DLL_PATH + os.path.sep + 'libmysql.dll', 'libmysql.dll', zipfile.ZIP_DEFLATED )
file.write(QTDIR + os.path.sep + 'bin' + os.path.sep + 'QtCore4.dll', 'QtCore4.dll', zipfile.ZIP_DEFLATED )
file.write(QTDIR + os.path.sep + 'bin' + os.path.sep + 'QtGui4.dll', 'QtGui4.dll', zipfile.ZIP_DEFLATED )
file.write(QTDIR + os.path.sep + 'bin' + os.path.sep + 'QtNetwork4.dll', 'QtNetwork4.dll', zipfile.ZIP_DEFLATED )
file.write(QTDIR + os.path.sep + 'bin' + os.path.sep + 'QtXml4.dll', 'QtXml4.dll', zipfile.ZIP_DEFLATED )
file.write(QTDIR + os.path.sep + 'bin' + os.path.sep + 'QtSql4.dll', 'QtSql4.dll', zipfile.ZIP_DEFLATED )
file.write(QTDIR + os.path.sep + 'bin' + os.path.sep + 'mingwm10.dll', 'mingwm10.dll', zipfile.ZIP_DEFLATED )
file.close()


# Now upload to berlios Anonymous FTP
os.system( 'pscp %s correa@shell.berlios.de:/home/groups/ftp/pub/wolfpack' % fileName )
