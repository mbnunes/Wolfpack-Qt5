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

SQLITE_CPP = sqlite
SQLITE_H = sqlite
PYTHON_CPP = python
PYTHON_H = python
NETWORK_H = network
NETWORK_CPP = network
AI_H = ai
AI_CPP = ai
MULS_H = muls
MULS_CPP = muls

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
	accounts.h \
	action.h \
	basechar.h \
	basedef.h \
	baseregion.h \
	basics.h \
	combat.h \
	commands.h \
	serverconfig.h \
	console.h \
	contextmenu.h \
	coord.h \
	corpse.h \
	customtags.h \
	dbdriver.h \
	definable.h \
	defines.h \
	definitions.h \
	dragdrop.h \
	exceptions.h \
	factory.h \
	getopts.h \
	guilds.h \
	gumps.h \
	inlines.h \
	items.h \
	log.h \
	md5.h \
	multi.h \
	npc.h \
	pagesystem.h \
	party.h \
	persistentbroker.h \
	persistentobject.h \
	platform.h \
	player.h \
	preferences.h \
	profile.h \
	progress.h \
	pythonscript.h \
	resource.h \
	scriptmanager.h \
	sectors.h \
	server.h \
	singleton.h \
	skills.h \
	spawnregions.h \
	speech.h \
	targetrequests.h \
	territories.h \
	timing.h \
	timers.h \
	tracking.h \
	trade.h \
	typedefs.h \
	uobject.h \
	uotime.h \
	verinfo.h \
	walking.h \
	world.h

SOURCES += \
	accounts.cpp \
	action.cpp \
	basechar.cpp \
	basedef.cpp \
	basics.cpp \
	combat.cpp \
	commands.cpp \
	serverconfig.cpp \
	console.cpp \
	contextmenu.cpp \
	coord.cpp \
	corpse.cpp \
	customtags.cpp \
	dbdriver.cpp \
	definable.cpp \
	definitions.cpp \
	dragdrop.cpp \
	getopts.cpp \
	guilds.cpp \
	gumps.cpp \
	items.cpp \
	log.cpp \
	md5.cpp \
	multi.cpp \
	npc.cpp \
	party.cpp \
	persistentbroker.cpp \
	persistentobject.cpp \
	player.cpp \
	preferences.cpp \
	profile.cpp \
	pythonscript.cpp \
	scriptmanager.cpp \
	sectors.cpp \
	server.cpp \
	skills.cpp \
	spawnregions.cpp \
	speech.cpp \
	targetrequests.cpp \
	territories.cpp \
	timing.cpp \
	timers.cpp \
	tracking.cpp \
	trade.cpp \
	uobject.cpp \
	walking.cpp \
	world.cpp

# Twofish Module
SOURCES += twofish/twofish2.cpp

INTERFACES =
TRANSLATIONS = \
	languages/wolfpack_pt-BR.ts \
	languages/wolfpack_it.ts \
	languages/wolfpack_nl.ts \
	languages/wolfpack_es.ts \
	languages/wolfpack_de.ts \
	languages/wolfpack_fr.ts \
	languages/wolfpack_ge.ts

unix:SOURCES  += \
	unix/config_unix.cpp \
	unix/console_unix.cpp \
	unix/getopts_unix.cpp

win32:SOURCES += \
	win/config_win.cpp \
	win/console_win.cpp \
	win/getopts_win.cpp

DISTFILES += \
	data/AUTHORS.txt \
	LICENSE.GPL

