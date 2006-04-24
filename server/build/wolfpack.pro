#################################################################
#   )      (\_     # Wolfpack 13.0.0 Build Script               #
#  ((    _/{  "-;  # Created by: Wolfpack Development Team      #
#   )).-" {{ ;"`   # Revised by: Wolfpack Development Team      #
#  ( (  ;._ \\ ctr # Last Modification: check svn logs          #
#################################################################

PROJECT = wolfpack
TARGET = wolfpack
TEMPLATE = app

CONFIG *= qt thread exceptions rtti 
QT += network xml sql

CONFIG -= flat
DESTDIR = ../release

VERSION = 12.9.14

RC_FILE = res.rc
OBJECTS_DIR = obj
MOC_DIR = obj

DEFINES += BOOST_PYTHON_STATIC_LIB

win32:LIBS += -lws2_32 -lkernel32 -luser32 -lgdi32 -ladvapi32 -lshell32 -lole32 -loleaut32 -luuid -lodbc32 -lodbccp32

# Include configure settings
!include(config.pri) {
	message("HINT: use ./configure.py script!")
}

# Precompiled header
precompile_header {
	PRECOMPILED_HEADER = ../src/wolfpack_pch.h
	INCLUDEPATH += obj
}

# GUI module
console {
	QT -= gui
	unix:SOURCES += ../src/unix/console_unix.cpp
} else {
	include(../src/gui/gui.pri)
}

LIBS += -lboost_python -L../boost/lib/
INCLUDEPATH += ../boost/include
PYTHON_CPP = ../src/python
PYTHON_H = ../src/python
NETWORK_H = ../src/network
NETWORK_CPP = ../src/network
AI_H = ../src/ai
AI_CPP = ../src/ai
MULS_H = ../src/muls
MULS_CPP = ../src/muls

DEPENDPATH += $$PYTHON_H;$$NETWORK_H;$$AI_H;$$MULS_H;../src
INCLUDEPATH += ../src


#modules
include($$PYTHON_CPP/python.pri)
include($$NETWORK_CPP/network.pri)
include($$AI_CPP/ai.pri)
include($$MULS_CPP/muls.pri)

# Common files

HEADERS += \
	./resource.h \
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
	../src/exportdefinitions.h \
	../src/factory.h \
	../src/guilds.h \
	../src/gumps.h \
	../src/inlines.h \
	../src/items.h \
	../src/log.h \
	../src/mapobjects.h \
	../src/md5.h \
	../src/multi.h \
	../src/npc.h \
	../src/optionparser.h \
	../src/party.h \
	../src/pathfinding.h \
	../src/persistentbroker.h \
	../src/persistentobject.h \
	../src/platform.h \
	../src/player.h \
	../src/preferences.h \
	../src/profile.h \
	../src/progress.h \
	../src/pythonscript.h \
	../src/quests.h \
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
	../src/exportdefinitions.cpp \
	../src/guilds.cpp \
	../src/gumps.cpp \
	../src/items.cpp \
	../src/log.cpp \
	../src/mapobjects.cpp \
	../src/md5.cpp \
	../src/multi.cpp \
	../src/npc.cpp \
	../src/optionparser.cpp \
	../src/party.cpp \
	../src/pathfinding.cpp \
	../src/persistentbroker.cpp \
	../src/persistentobject.cpp \
	../src/player.cpp \
	../src/preferences.cpp \
	../src/profile.cpp \
	../src/pythonscript.cpp \
	../src/quests.cpp \
	../src/scriptmanager.cpp \
	../src/server.cpp \
	../src/skills.cpp \
	../src/spawnregions.cpp \
	../src/speech.cpp \
	../src/targetrequests.cpp \
	../src/territories.cpp \
	../src/timing.cpp \
	../src/timers.cpp \
	../src/trade.cpp \
	../src/uobject.cpp \
	../src/walking.cpp \
	../src/world.cpp

# Twofish Module
SOURCES += ../src/twofish/twofish2.cpp

TRANSLATIONS = \
	languages/wolfpack_pt_br.ts \
	languages/wolfpack_it.ts \
	languages/wolfpack_nl.ts \
	languages/wolfpack_es.ts \
	languages/wolfpack_de.ts \
	languages/wolfpack_fr.ts \
	languages/wolfpack_ge.ts

unix:SOURCES  += \
	../src/unix/config_unix.cpp

win32:SOURCES += \
	../src/win/config_win.cpp

DISTFILES += \
	../release/AUTHORS \
	../release/Changelog \
	../release/COPYING \
	../release/COPYRIGHT \
	../release/INSTALL \
	../release/LICENSE.GPL \
	../release/NEWS \
	../release/README


# make confclean
unix {
	confclean.depends += clean
	confclean.commands += $(DEL_FILE) config.pri 
	QMAKE_EXTRA_UNIX_TARGETS += confclean 
}

# used by tools/translationupdate, our own version of lupdate
# WPDEFINITIONS is the folder where xml definitons are.
# WPSCRIPTS is the folder where Python scripts are.
WPDEFINITIONS = ../release/definitions
WPSCRIPTS = ../release/scripts

# This should rarely change at all, so I'm placing at the botton of the file.
QMAKE_TARGET_COMPANY = Wolfpack Development Team
QMAKE_TARGET_PRODUCT = Wolfpack
QMAKE_TARGET_DESCRIPTION = Ultima Online(tm) Server Software
QMAKE_TARGET_COPYRIGHT = Copyright (c) 2000-2006 Wolfpack Development Team
