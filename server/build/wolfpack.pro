#
# Wolfpack qmake project
#
#
#

PROJECT         = wolfpack
TARGET          = wolfpack
TEMPLATE       	+= app
CONFIG        	+= qt console thread exceptions rtti
INCLUDEPATH	+= lib/Python/include

unix {

# Common unix settings
	INCLUDEPATH += /usr/local/include/stlport lib/Python /usr/include/mysql /usr/local/lib/mysql/include/mysql lib/Python/Include network
	LIBS  += -L/usr/local/lib/mysql/lib/mysql -L/usr/local/lib -Llib/Python -L/usr/lib/mysql -ldl -lpython2.2 -lmysqlclient -lutil
	
# Optional compile modes	
	
	release {
		CONFIG += warn_off
		linux {
			QMAKE_CXXFLAGS -= -O3
			QMAKE_CXXFLAGS += -march=athlon-xp -O3 -pipe -fomit-frame-pointer -falign-functions=16 -falign-labels=8 -falign-loops=8 -falign-jumps=8 -fsched-spec-load -frerun-loop-opt -finline-limit=800 -funroll-loops -fprefetch-loop-arrays -ffast-math -mfpmath=sse -msse -m3dnow -fschedule-insns2 -fexpensive-optimizations -fmove-all-movables -fdelete-null-pointer-checks
			
		}
	}
	debug {
		CONFIG += warn_on
		linux {
			QMAKE_CXXFLAGS += -g 
		}
	}
	static {
		QMAKE_LFLAGS += -static
	}
}

RC_FILE = res.rc
OBJECTS_DIR = obj

win32:DEFINES  += WIN32 
win32-msvc:DEFINES +=  _CONSOLE _MBCS
win32:INCLUDEPATH += lib/Python/PC C:/mysql/include/
win32-g++:LIBS= -lwsock32
win32-msvc:LIBS      = kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comdlg32.lib ws2_32.lib ZThread.lib
win32-msvc:TMAKE_CXXFLAGS = /J /nologo /ML /W3 /GX /O2 /YX /FD /c
win32-borland:TMAKE_CXXFLAGS =  -K -6 -q -x -WM -w-8057 -w-8066 -w-8060 -w-8027 -w-8059 -w-8004 -w-8012
win32-borland:LIBS = ws2_32.lib lib/ZThread/lib/ZThread.lib

# Common files

HEADERS         = \
		  Timing.h \
		  TmpEff.h \
		  Trade.h \
		  accounts.h \
		  basics.h \
		  basechar.h \
		  boats.h \
		  coord.h \
		  combat.h \
		  commands.h \
		  dbl_single_click.h \
		  dbdriver.h \
		  debug.h \
		  definable.h \
		  defines.h \
		  dragdrop.h \
		  encryption.h \
		  exceptions.h \
		  globals.h \
		  guildstones.h \
		  gumps.h \
		  house.h \
		  inlines.h \
                  iserialization.h \
		  itemid.h \
		  items.h \
		  makemenus.h \
		  multis.h \
		  network.h \
		  npc.h \
                  pfactory.h \
		  platform.h \
		  prototypes.h \
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
                  serxmlfile.h \
                  serbinfile.h \
                  skills.h \
		  stream.h \
		  structs.h \
                  targetrequests.h \
                  territories.h \
                  tracking.h \
		  typedefs.h \
		  uobject.h \
		  utilsys.h \
		  verinfo.h \
		  wolfpack.h \
		  wpconsole.h \
		  tilecache.h \
		  walking.h \
		  world.h \
		  wpdefmanager.h \
		  wptargetrequests.h \
		  corpse.h
		  
SOURCES         = \
		accounts.cpp \
		ai.cpp \
		ai_animals.cpp \
		ai_monsters.cpp \
		ai_vendors.cpp \
		basechar.cpp \
		basics.cpp \
		boats.cpp \
		chars.cpp \
		combat.cpp \
		commands.cpp \
		contextmenu.cpp \		  		  
		coord.cpp \
		corpse.cpp \
		customtags.cpp \
		dbl_single_click.cpp \
		dbdriver.cpp \
		debug.cpp \
		definable.cpp \
		dragdrop.cpp \
		globals.cpp \
		guildstones.cpp \
		gumps.cpp \
		house.cpp \
		iserialization.cpp \
		itemid.cpp \
		items.cpp \
		LoS.cpp \
		makemenus.cpp \
		maps.cpp \
		multis.cpp \
		multiscache.cpp \
		network.cpp \
		npc.cpp \
		encryption.cpp \
		pfactory.cpp \
		persistentbroker.cpp \
		persistentobject.cpp \
		preferences.cpp \
		player.cpp \
		pythonscript.cpp \
		resources.cpp \
		sectors.cpp \
		serxmlfile.cpp \
		serbinfile.cpp \
		scriptmanager.cpp \
		skills.cpp \
		speech.cpp \
		spawnregions.cpp \
		srvparams.cpp \
		storage.cpp \
		targetactions.cpp \
		targetrequests.cpp \
		territories.cpp \
		tilecache.cpp \
		Timing.cpp \
		tmpeff.cpp \
		tracking.cpp \
		Trade.cpp \                  
		uobject.cpp \
		utilsys.cpp \
		wolfpack.cpp \
		wpconsole.cpp \
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

HEADERS		+= python/content.h

# SQLite Sources
SOURCES 	+=	sqlite/attach.c \
				sqlite/auth.c \
				sqlite/btree.c \
				sqlite/btree_rb.c \
				sqlite/build.c \
				sqlite/copy.c \
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
				sqlite/vdbe.h

INTERFACES	=
TRANSLATIONS    = \
                  languages/wolfpack_pt-BR.ts \
                  languages/wolfpack_it.ts \
                  languages/wolfpack_nl.ts \
                  languages/wolfpack_es.ts \
                  languages/wolfpack_de.ts \
                  languages/wolfpack_fr.ts \
                  languages/wolfpack_ge.ts

unix:SOURCES  += srvparams_unix.cpp
win32:SOURCES += srvparams_win.cpp

