#
# Wolfpack qmake project
#
#
#

PROJECT         = wolfpack
TARGET          = wolfpack
TEMPLATE       += app
CONFIG         += qt console thread exceptions rtti
INCLUDEPATH    += lib/ZThread/include lib/Python/include
DEFINES        += REENTRANT ZTHREAD_STATIC WP_DONT_USE_HASH_MAP

unix {

# Common unix settings
	INCLUDEPATH += /usr/local/include/stlport lib/Python /usr/include/mysql
	LIBS  = -L/usr/local/lib -Llib/ZThread/lib -Llib/Python -L/usr/lib/mysql -ldl -lZThread -lpython2.2 -lmysqlclient -lutil
	
# Optional compile modes	
	
	release {
		CONFIG += warn_off
		linux {
			QMAKE_CXXFLAGS -= -O2
			QMAKE_CXXFLAGS += -mcpu=athlon-xp -O3 -pipe -fomit-frame-pointer -fsched-spec-load -frerun-loop-opt -fprefetch-loop-arrays -ffast-math
		}
	}
	debug {
		CONFIG += warn_on
	}
	static {
		QMAKE_LFLAGS += -static
	}
}

RC_FILE = res.rc
OBJECTS_DIR = obj

win32:DEFINES  += WIN32 
win32-msvc:DEFINES +=  _CONSOLE _MBCS
win32:INCLUDEPATH += lib/Python/PC
win32-g++:TMAKE_CXXFLAGS = -funsigned-char
win32-g++:LIBS= -Llib/ZThread/lib/ -lwsock32 -lZThread
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
		  boats.h \
		  books.h \
		  chars.h \
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
                  magic.h \
		  makemenus.h \
		  multis.h \
		  network.h \
                  pfactory.h \
		  platform.h \
		  prototypes.h \
		  persistentbroker.h \
		  persistentobject.h \
		  preferences.h \
		  mapobjects.h \
		  resource.h \
		  resources.h \
		  spawnregions.h \
		  speech.h \
                  srvparams.h \
                  serxmlfile.h \
                  serbinfile.h \
                  skills.h \
		  stream.h \
		  structs.h \
		  targeting.h \
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
		  wpdefaultscript.h \
		  wpdefmanager.h \
		  wpscriptmanager.h \
		  wptargetrequests.h \
		  corpse.h \
		  spellbook.h \
                  newmagic.h
		  
SOURCES         = \
		  LoS.cpp \
		  corpse.cpp \
		  Timing.cpp \
		  tmpeff.cpp \
		  Trade.cpp \
		  accounts.cpp \
		  basics.cpp \
		  boats.cpp \
		  books.cpp \
		  char_flatstore.cpp \
                  chars.cpp \
		  combat.cpp \
		  commands.cpp \
		  contextmenu.cpp \
		  coord.cpp \
		  tilecache.cpp \
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
	   	  item_flatstore.cpp \
		  itemid.cpp \
		  items.cpp \
		  magic.cpp \
		  makemenus.cpp \
		  maps.cpp \
		  mapobjects.cpp \
		  multis.cpp \
		  multiscache.cpp \
		  necro.cpp \
		  network.cpp \
		  npcs.cpp \
		  p_ai.cpp \
		  encryption.cpp \
                  pfactory.cpp \
  		  persistentbroker.cpp \
		  persistentobject.cpp \
		  preferences.cpp \
		  resources.cpp \
                  serxmlfile.cpp \
                  serbinfile.cpp \
		  skills.cpp \
		  speech.cpp \
		  spawnregions.cpp \
                  srvparams.cpp \
		  storage.cpp \
		  targetactions.cpp \
		  targeting.cpp \
		  targetrequests.cpp \
                  territories.cpp \
                  tracking.cpp \
		  uobject.cpp \
		  uobject_flatstore.cpp \
		  utilsys.cpp \
		  wolfpack.cpp \
		  wpconsole.cpp \
		  walking.cpp \
		  world.cpp \
		  wpdefmanager.cpp \
		  wpdefaultscript.cpp \
		  wpscriptmanager.cpp \
		  wptargetrequests.cpp \
		  newmagic.cpp \
		  spellbook.cpp

# Twofish Module
HEADERS		+= twofish/twofish.h

SOURCES		+= twofish/twofish.cpp

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
		   python/pyregion.cpp \
		   python/pytooltip.cpp \	
		   python/socket.cpp \
		   python/wppythonscript.cpp

HEADERS		+= python/content.h

# Flatstore Module
HEADERS		+= flatstore/exceptions.h \
		   flatstore/flatstore.h \
		   flatstore/flatstore_keys.h \
		   flatstore/version.h

SOURCES		+= flatstore/exceptions.cpp \
	   	   flatstore/flatstore.cpp \
	   	   flatstore/flatstore_c.cpp

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

