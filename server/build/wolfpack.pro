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

	# Common unix settings
	# Lets try to figure some paths

	CONFIG += console

	# MySQL
	LIBS += $$MYSQL_LIB
	INCLUDEPATH += $$MYSQL_INC

	# Python includes. Run configure script to initialize it.
	LIBS  += $$PYTHON_LIB
	INCLUDEPATH += $$PYTHON_INC

	LIBS += $$STLPORT_LIB
	INCLUDEPATH += $$STLPORT_INC

	LIBS  += -L/usr/local/lib -L/usr/lib -ldl -lutil

	# we dont use those.
	QMAKE_LIBS_X11 -= -lX11 -lXext -lm
}

RC_FILE = res.rc
OBJECTS_DIR = obj
MOC_DIR = obj

INCLUDEPATH += sqlite 
win32:DEFINES -= UNICODE

# Include configure's settings
!include(config.pri) {
	message("HINT: use ./configure script!")
}

# Precompiled header
precompile_header:PRECOMPILED_HEADER = wolfpack_pch.h

# Common files

HEADERS = \
	accounts.h \
	basechar.h \
	basedef.h \
	baseregion.h \
	basics.h \
	combat.h \
	commands.h \
	config.h \
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
	network/encryption.h \
	exceptions.h \
	factory.h \
	getopts.h \
	guilds.h \
	gumps.h \
	inlines.h \
	items.h \
	log.h \
	muls/maps.h \
	md5.h \
	multi.h \
	muls/multiscache.h \
	network/network.h \
	npc.h \
	pagesystem.h \
	party.h \
	persistentbroker.h \
	persistentobject.h \
	platform.h \
	player.h \
	preferences.h \
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
	muls/tilecache.h \
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

SOURCES = \
	accounts.cpp \
	basechar.cpp \
	basedef.cpp \
	basics.cpp \
	combat.cpp \
	commands.cpp \
	config.cpp \
	console.cpp \
	contextmenu.cpp \
	coord.cpp \
	corpse.cpp \
	customtags.cpp \
	dbdriver.cpp \
	definable.cpp \
	definitions.cpp \
	dragdrop.cpp \
	network/encryption.cpp \
	getopts.cpp \
	guilds.cpp \
	gumps.cpp \
	items.cpp \
	log.cpp \
	muls/maps.cpp \
	md5.cpp \
	multi.cpp \
	muls/multiscache.cpp \
	network/network.cpp \
	npc.cpp \
	party.cpp \
	persistentbroker.cpp \
	persistentobject.cpp \
	player.cpp \
	preferences.cpp \
	pythonscript.cpp \
	scriptmanager.cpp \
	sectors.cpp \
	server.cpp \
	skills.cpp \
	spawnregions.cpp \
	speech.cpp \
	targetrequests.cpp \
	territories.cpp \
	muls/tilecache.cpp \
	timing.cpp \
	timers.cpp \
	tracking.cpp \
	trade.cpp \
	uobject.cpp \
	walking.cpp \
	world.cpp
	
# Twofish Module
SOURCES += twofish/twofish2.c

# Network Module
# THIS IS IMPORTANT FOR MOCING!
HEADERS += \
	network/asyncnetio.h \
	network/listener.h \
	network/uopacket.h \
	network/uorxpackets.h \
	network/uosocket.h \
	network/uotxpackets.h

SOURCES += \
	network/asyncnetio.cpp \
	network/listener.cpp \
	network/uopacket.cpp \
	network/uorxpackets.cpp \
	network/uosocket.cpp \
	network/uotxpackets.cpp

# AI Module
HEADERS		+= ai/ai.h

SOURCES += \
	ai/ai.cpp \
	ai/ai_animals.cpp \
	ai/ai_humans.cpp \
	ai/ai_monsters.cpp

# Python Module
SOURCES += \
	python/char.cpp \
	python/engine.cpp \
	python/global.cpp \
	python/item.cpp \
	python/pyaccount.cpp \
	python/pyai.cpp \
	python/pycoord.cpp \
	python/pypacket.cpp \
	python/pyregion.cpp \
	python/pytooltip.cpp \
	python/socket.cpp \
	python/worlditerator.cpp

HEADERS += \
	python/content.h \
	python/worlditerator.h

# SQLite Sources
SOURCES += \
	sqlite/attach.c \
	sqlite/auth.c \
	sqlite/btree.c \
	sqlite/btree_rb.c \
	sqlite/build.c \
	sqlite/copy.c \
	sqlite/date.c \
	sqlite/delete.c \
	sqlite/expr.c \
	sqlite/func.c \
	sqlite/hash.c \
	sqlite/insert.c \
	sqlite/main.c \
	sqlite/opcodes.c \
	sqlite/os.c \
	sqlite/pager.c \
	sqlite/parse.c \
	sqlite/pragma.c \
	sqlite/printf.c \
	sqlite/random.c \
	sqlite/select.c \
	sqlite/table.c \
	sqlite/tokenize.c \
	sqlite/trigger.c \
	sqlite/update.c \
	sqlite/util.c \
	sqlite/vacuum.c \
	sqlite/vdbe.c \
	sqlite/vdbeaux.c \
	sqlite/where.c

HEADERS += \
	sqlite/btree.h \
	sqlite/config.h \
	sqlite/hash.h \
	sqlite/opcodes.h \
	sqlite/os.h \
	sqlite/pager.h \
	sqlite/parse.h \
	sqlite/sqlite.h \
	sqlite/sqliteInt.h \
	sqlite/vdbe.h \
	sqlite/vdbeInt.h

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
