#################################################################
#   )      (\_     # Wolfpack 13.0.0 Build Script               #
#  ((    _/{  "-;  # Created by: Wolfpack Development Team      #
#   )).-" {{ ;"`   # Revised by: Wolfpack Development Team      #
#  ( (  ;._ \\ ctr # Last Modification: check cvs logs          #
#################################################################

PROJECT	= wolfpack
TARGET		= wolfpack
TEMPLATE	+= app
CONFIG		+= qt console thread exceptions rtti

# disable run of qmake after Makefile modification
QMAKE =

unix {

	# Common unix settings
	# Lets try to figure some paths
	message("HINT: use ./configure script!")

	# MySQL
	LIBS += $$MYSQL_LIB
	INCLUDEPATH += $$MYSQL_INC

	# Python includes. Run configure script to initialize it.
	INCLUDEPATH += $$PYTHON_INC

	INCLUDEPATH += /usr/local/include/stlport sqlite network
	LIBS  += -L/usr/local/lib -L/usr/lib -ldl -lpython2.3 -lutil
	LIBS  += -L$$PYTHON_LIB

	# we dont use those.
	QMAKE_LIBS_X11 -= -lX11 -lXext -lm

	# TODO: rewrite and put into ./configure
	release {
		CONFIG += warn_off
		linux {
			QMAKE_CFLAGS_RELEASE = -O3
			QMAKE_CXXFLAGS += -march=athlon-xp -O3 -pipe -fomit-frame-pointer -falign-functions=16 -falign-labels=8 -falign-loops=8 -falign-jumps=8 -fsched-spec-load -frerun-loop-opt -finline-limit=800 -funroll-loops -fprefetch-loop-arrays -ffast-math -mfpmath=sse -msse -m3dnow -fschedule-insns2 -fexpensive-optimizations -fmove-all-movables -fdelete-null-pointer-checks

		}
	}

	CFLAGS = $$CXXFLAGS
}

RC_FILE = res.rc
OBJECTS_DIR = obj
MOC_DIR = obj

win32:DEFINES  += WIN32
win32-msvc:DEFINES +=  _CONSOLE _MBCS
win32:INCLUDEPATH += lib/Python/PC C:/mysql/include/
win32-g++:LIBS= -lwsock32
win32-msvc:LIBS      = kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comdlg32.lib ws2_32.lib
win32-msvc:TMAKE_CXXFLAGS = /J /nologo /ML /W3 /GX /O2 /YX /FD /c
win32-borland:TMAKE_CXXFLAGS =  -K -6 -q -x -WM -w-8057 -w-8066 -w-8060 -w-8027 -w-8059 -w-8004 -w-8012
win32-borland:LIBS += ws2_32.lib

# Common files

HEADERS         = \
			Timing.h \
			TmpEff.h \
			Trade.h \
			accounts.h \
			basics.h \
			basechar.h \
			basedef.h \
			boats.h \
			coord.h \
			combat.h \
			commands.h \
			console.h \
			dbl_single_click.h \
			dbdriver.h \
			definable.h \
			defines.h \
			dragdrop.h \
			encryption.h \
			exceptions.h \
			globals.h \
			gumps.h \
			house.h \
			inlines.h \
			itemid.h \
			items.h \
			log.h \
			makemenus.h \
			multis.h \
			network.h \
			npc.h \
			platform.h \
			persistentbroker.h \
			persistentobject.h \
			preferences.h \
			player.h \
			pythonscript.h \
			resource.h \
			resources.h \
			scriptmanager.h \
			sectors.h \
		  spawnregions.h \
		  speech.h \
			srvparams.h \
			skills.h \
			structs.h \
			targetrequests.h \
			territories.h \
			tracking.h \
			typedefs.h \
			uobject.h \
			verinfo.h \
			wolfpack.h \
			tilecache.h \
			walking.h \
			world.h \
			wpdefmanager.h \
			wptargetrequests.h \
			corpse.h

SOURCES         = \
		accounts.cpp \
		basechar.cpp \
		basedef.cpp \
		basics.cpp \
		boats.cpp \
		chars.cpp \
		combat.cpp \
		commands.cpp \
		console.cpp \
		contextmenu.cpp \
		coord.cpp \
		corpse.cpp \
		customtags.cpp \
		dbl_single_click.cpp \
		dbdriver.cpp \
		definable.cpp \
		dragdrop.cpp \
		globals.cpp \
		gumps.cpp \
		house.cpp \
		itemid.cpp \
		items.cpp \
		log.cpp \
		makemenus.cpp \
		maps.cpp \
		multis.cpp \
		multiscache.cpp \
		network.cpp \
		npc.cpp \
		encryption.cpp \
    party.cpp \
		persistentbroker.cpp \
		persistentobject.cpp \
		preferences.cpp \
		player.cpp \
		pythonscript.cpp \
		resources.cpp \
		sectors.cpp \
		scriptmanager.cpp \
		skills.cpp \
		speech.cpp \
		spawnregions.cpp \
		srvparams.cpp \
		targetrequests.cpp \
		territories.cpp \
		tilecache.cpp \
		Timing.cpp \
		tmpeff.cpp \
		tracking.cpp \
		Trade.cpp \
		uobject.cpp \
		wolfpack.cpp \
		walking.cpp \
		world.cpp \
		wpdefmanager.cpp \
		wptargetrequests.cpp

# Twofish Module
SOURCES		+= twofish/twofish2.c

# Network Module
# THIS IS IMPORTANT FOR MOCING!
HEADERS		+= network/asyncnetio.h \
		   network/listener.h \
		   network/uopacket.h \
		   network/uorxpackets.h \
		   network/uosocket.h \
		   network/uotxpackets.h

SOURCES		+= network/asyncnetio.cpp \
		   network/listener.cpp \
		   network/uopacket.cpp \
		   network/uorxpackets.cpp \
		   network/uosocket.cpp \
		   network/uotxpackets.cpp

# AI Module
HEADERS		+= ai/ai.h

SOURCES		+= ai/ai.cpp \
		   ai/ai_animals.cpp \
		   ai/ai_humans.cpp \
		   ai/ai_monsters.cpp

# Python Module
SOURCES		+= python/char.cpp \
		   python/engine.cpp \
		   python/global.cpp \
		   python/item.cpp \
	  	   python/multi.cpp \
		   python/pyaccount.cpp \
		   python/pycoord.cpp \
		   python/pypacket.cpp \
		   python/pyregion.cpp \
		   python/pytooltip.cpp \
		   python/socket.cpp \
		   python/worlditerator.cpp

HEADERS		+= python/content.h \
		   python/worlditerator.h

# SQLite Sources
SOURCES 	+=	sqlite/attach.c \
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

HEADERS		+=	sqlite/btree.h \
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

INTERFACES	=
TRANSLATIONS    = \
		languages/wolfpack_pt-BR.ts \
		languages/wolfpack_it.ts \
		languages/wolfpack_nl.ts \
		languages/wolfpack_es.ts \
		languages/wolfpack_de.ts \
		languages/wolfpack_fr.ts \
		languages/wolfpack_ge.ts

unix:SOURCES  += srvparams_unix.cpp \
		 console_unix.cpp

win32:SOURCES += srvparams_win.cpp \
		 console_win.cpp

DISTFILES     += AUTHORS.txt \
		 COPYING.txt \
		 LICENSE.GPL

