
PROJECT = uoclient
TARGET = uoclient
TEMPLATE = app

CONFIG += qt thread exceptions release

win32:CONFIG += windows
unix:CONFIG += x11

# No idea what it does
# DEFINES += QT_CLEAN_NAMESPACE QT_COMPAT_WARNINGS

# not yet
#RC_FILE = res.rc
OBJECTS_DIR = obj
MOC_DIR = obj

win32:DEFINES -= UNICODE

DEPENDPATH += src
INCLUDEPATH += include;libs/include

# MAIN INCLUDES
HEADERS += include/config.h \
	include/engine.h \
	include/enums.h \
	include/exceptions.h \
	include/log.h \
	include/mersennetwister.h \
	include/paintable.h \
	include/uoclient.h
	
# GUI INCLUDES
HEADERS += include/dialogs/login.h \
	include/gui/asciilabel.h \
	include/gui/bordergump.h \
	include/gui/container.h \
	include/gui/control.h \
	include/gui/gui.h \
	include/gui/gumpimage.h \
	include/gui/image.h \
	include/gui/imagebutton.h \
	include/gui/itemimage.h \
	include/gui/label.h \
	include/gui/textfield.h \
	include/gui/tiledgumpimage.h

# MUL INCLUDES
HEADERS += include/muls/art.h \
	include/muls/asciifonts.h \
	include/muls/gumpart.h \
	include/muls/hues.h \
	include/muls/unicodefonts.h \
	include/muls/verdata.h

# MAIN srcS
SOURCES += src/config.cpp \
	src/engine.cpp \
	src/log.cpp \
	src/paintable.cpp \
	src/uoclient.cpp
	
# GUI srcS
SOURCES += src/dialogs/login.cpp \
	src/gui/asciilabel.cpp \
	src/gui/bordergump.cpp \
	src/gui/container.cpp \
	src/gui/control.cpp \
	src/gui/gui.cpp \
	src/gui/gumpimage.cpp \
	src/gui/image.cpp \
	src/gui/imagebutton.cpp \
	src/gui/itemimage.cpp \
	src/gui/label.cpp \
	src/gui/textfield.cpp \
	src/gui/tiledgumpimage.cpp

# MUL srcS
SOURCES += src/muls/art.cpp \
	src/muls/asciifonts.cpp \
	src/muls/gumpart.cpp \
	src/muls/hues.cpp \
	src/muls/unicodefonts.cpp \
	src/muls/verdata.cpp

INTERFACES =

#TRANSLATIONS = \
#	languages/wolfpack_pt_br.ts \
#	languages/wolfpack_it.ts \
#	languages/wolfpack_nl.ts \
#	languages/wolfpack_es.ts \
#	languages/wolfpack_de.ts \
#	languages/wolfpack_fr.ts \
#	languages/wolfpack_ge.ts

#unix:SOURCES  += \
#	../src/unix/config_unix.cpp \
#	../src/unix/console_unix.cpp \
#	../src/unix/getopts_unix.cpp

win32:SOURCES += \
	src/config_win.cpp \
	src/startup_win.cpp \
	src/utilities_win.cpp

#
#DISTFILES += \
#	../release/AUTHORS.txt \
#	../release/LICENSE.GPL
#