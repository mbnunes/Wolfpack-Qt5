#################################################################
#   )      (\_     # Wolfpack 13.0.0 Build Script               #
#  ((    _/{  "-;  # Created by: Wolfpack Development Team      #
#   )).-" {{ ;"`   # Revised by: Wolfpack Development Team      #
#  ( (  ;._ \\ ctr # Last Modification: check cvs logs          #
#################################################################

PROJECT = wolfpack
TARGET = wolfpack
TEMPLATE = app

CONFIG += qt thread exceptions rtti

unix {
	CONFIG += console

	# We need to remove these, unnecessary dependency
	QMAKE_LIBS_X11 -= -lX11 -lXext -lm
}

DEFINES += QT_CLEAN_NAMESPACE

RC_FILE = res.rc
OBJECTS_DIR = obj
MOC_DIR = obj

win32:DEFINES -= UNICODE

# Include configure settings
!include(config.pri) {
	message("HINT: use ./configure script!")
}

# Precompiled header
precompile_header {
#	PRECOMPILED_HEADER = wolfpack_pch.h
	INCLUDEPATH += obj
}

SQLITE_CPP = ../src/sqlite
SQLITE_H = ../src/sqlite
PYTHON_CPP = ../src/python
PYTHON_H = ../src/python
NETWORK_H = ../src/network
NETWORK_CPP = ../src/network
AI_H = ../src/ai
AI_CPP = ../src/ai
MULS_H = ../src/muls
MULS_CPP = ../src/muls

DEPENDPATH += ;$$SQLITE_H;$$PYTHON_H;$$NETWORK_H;$$AI_H;$$MULS_H;.
INCLUDEPATH += $$SQLITE_H


#modules
include($$PYTHON_CPP/python.pri)
include($$SQLITE_CPP/sqlite.pri)
include($$NETWORK_CPP/network.pri)
include($$AI_CPP/ai.pri)
include($$MULS_CPP/muls.pri)
# Common files

HEADERS += \
	../src/accounts.h \
	../src/action.h \
	../src/basechar.h \
	../src/basedef.h \
	../src/baseregion.h \
	../src/basics.h \
	../src/combat.h \
	../src/commands.h \
	../src/serverconfig.h \
	../src/console.h \
	../src/contextmenu.h \
	../src/coord.h \
	../src/corpse.h \
	../src/customtags.h \
	../src/dbdriver.h \
	../src/definable.h \
	../src/defines.h \
	../src/definitions.h \
	../src/dragdrop.h \
	../src/exceptions.h \
	../src/factory.h \
	../src/getopts.h \
	../src/guilds.h \
	../src/gumps.h \
	../src/inlines.h \
	../src/items.h \
	../src/log.h \
	../src/mapobjects.h \
	../src/md5.h \
	../src/multi.h \
	../src/npc.h \
	../src/party.h \
	../src/persistentbroker.h \
	../src/persistentobject.h \
	../src/platform.h \
	../src/player.h \
	../src/preferences.h \
	../src/profile.h \
	../src/progress.h \
	../src/pythonscript.h \
	../src/resource.h \
	../src/scriptmanager.h \
	../src/server.h \
	../src/singleton.h \
	../src/skills.h \
	../src/spawnregions.h \
	../src/speech.h \
	../src/targetrequests.h \
	../src/territories.h \
	../src/timing.h \
	../src/timers.h \
	../src/tracking.h \
	../src/trade.h \
	../src/typedefs.h \
	../src/uobject.h \
	../src/uotime.h \
	../src/verinfo.h \
	../src/walking.h \
	../src/world.h

SOURCES += \
	../src/accounts.cpp \
	../src/action.cpp \
	../src/basechar.cpp \
	../src/basedef.cpp \
	../src/basics.cpp \
	../src/combat.cpp \
	../src/commands.cpp \
	../src/content.cpp \
	../src/serverconfig.cpp \
	../src/console.cpp \
	../src/contextmenu.cpp \
	../src/coord.cpp \
	../src/corpse.cpp \
	../src/customtags.cpp \
	../src/dbdriver.cpp \
	../src/definable.cpp \
	../src/definitions.cpp \
	../src/dragdrop.cpp \
	../src/getopts.cpp \
	../src/guilds.cpp \
	../src/gumps.cpp \
	../src/items.cpp \
	../src/log.cpp \
	../src/mapobjects.cpp \
	../src/md5.cpp \
	../src/multi.cpp \
	../src/npc.cpp \
	../src/party.cpp \
	../src/persistentbroker.cpp \
	../src/persistentobject.cpp \
	../src/player.cpp \
	../src/preferences.cpp \
	../src/profile.cpp \
	../src/pythonscript.cpp \
	../src/scriptmanager.cpp \
	../src/server.cpp \
	../src/skills.cpp \
	../src/spawnregions.cpp \
	../src/speech.cpp \
	../src/targetrequests.cpp \
	../src/territories.cpp \
	../src/timing.cpp \
	../src/timers.cpp \
	../src/tracking.cpp \
	../src/trade.cpp \
	../src/uobject.cpp \
	../src/walking.cpp \
	../src/world.cpp

# Twofish Module
SOURCES += ../src/twofish/twofish2.cpp

INTERFACES =
TRANSLATIONS = \
	languages/wolfpack_pt_br.ts \
	languages/wolfpack_it.ts \
	languages/wolfpack_nl.ts \
	languages/wolfpack_es.ts \
	languages/wolfpack_de.ts \
	languages/wolfpack_fr.ts \
	languages/wolfpack_ge.ts

unix:SOURCES  += \
	../src/unix/config_unix.cpp \
	../src/unix/console_unix.cpp \
	../src/unix/getopts_unix.cpp

win32:SOURCES += \
	../src/win/config_win.cpp \
	../src/win/console_win.cpp \
	../src/win/getopts_win.cpp

DISTFILES += \
	../release/AUTHORS.txt \
	../release/LICENSE.GPL

# used by tools/translationupdate, our own version of lupdate
# WPDEFINITIONS is the folder where xml definitons are.
# WPSCRIPTS is the folder where Python scripts are.
WPDEFINITIONS = ../definitions
WPSCRIPTS = ../scripts
