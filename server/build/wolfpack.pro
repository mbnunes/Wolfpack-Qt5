#
# Wolfpack TMake project
#
#
#

PROJECT         = wolfpack
TARGET          = wolfpack
TEMPLATE        = app
CONFIG          = console debug thread
INCLUDEPATH     = lib/ZThread/include lib/wrl/include lib/Python/include
DEFINES         = REENTRANT ZTHREAD_STATIC NDEBUG
win32:DEFINES  += WIN32 
win32:OBJECTS_DIR = obj
win32-msvc:DEFINES +=  _CONSOLE _MBCS
win32:INCLUDEPATH += lib/Python/PC
unix:INCLUDEPATH += lib/Python

unix:LIBS       = -Llib/ZThread/lib -Llib/wrl/lib -Llib/Python -ldl -lZThread -lwrl -lpython2.2



unix:TMAKE_CXXFLAGS = -funsigned-char
win32-g++:TMAKE_CXXFLAGS = -funsigned-char
win32-g++:LIBS= -Llib/ZThread/lib/ -Llib/wrl/lib -lwsock32 -lZThread -lwrl
win32-msvc:RC_FILE         = res.rc
win32-msvc:LIBS      = kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comdlg32.lib ws2_32.lib ZThread.lib wrl.lib
win32-msvc:TMAKE_CXXFLAGS = /J /nologo /ML /W3 /GX /O2 /YX /FD /c

win32-borland:TMAKE_CXXFLAGS =  -K -6 -q -x -WM -w-8057 -w-8066 -w-8060 -w-8027 -w-8059 -w-8004 -w-8012
win32-borland:LIBS = ws2_32.lib lib/ZThread/lib/ZThread.lib lib/wrl/lib/wrl.lib

HEADERS         = client.h \
		  SndPkg.h \
		  Timing.h \
		  TmpEff.h \
		  Trade.h \
		  accounts.h \
		  basics.h \
		  boats.h \
		  books.h \
		  bounty.h \
		  chars.h \
                  charsmgr.h \
		  classes.h \
		  coord.h \
		  combat.h \
		  commands.h \
		  cweather.h \
		  dbl_single_click.h \
		  debug.h \
		  definable.h \
		  defines.h \
		  dragdrop.h \
		  globals.h \
		  guildstones.h \
		  gumps.h \
		  hCache.h \
		  im.h \
		  inlines.h \
                  iserialization.h \
		  itemid.h \
		  items.h \
                  itemsmgr.h \
		  junk.h \
                  magic.h \
		  makemenu.h \
		  mapstuff.h \
		  msgboard.h \
		  netsys.h \
		  network.h \
                  pfactory.h \
		  platform.h \
		  prototypes.h \
		  regions.h \
		  remadmin.h \
		  resource.h \
		  spawnregions.h \
		  speech.h \
                  srvparams.h \
                  serxmlfile.h \
                  serbinfile.h \
		  stream.h \
		  structs.h \
		  targeting.h \
                  targetrequests.h \
		  telport.h \
                  territories.h \
		  typedefs.h \
		  uobject.h \
		  utilsys.h \
		  verinfo.h \
		  weight.h \
		  whitespace.h \
		  wolfpack.h \
		  worldmain.h \
		  wpconsole.h \
		  tilecache.h \
		  walking.h \
		  packlen.h \
		  extract.h \
		  wpdefaultscript.h \
		  wpdefmanager.h \
		  wpscriptmanager.h \
		  menuactions.h \
		  wptargetrequests.h \
		  corpse.h
		  
# Python Module
		+= python/content.h	  
		  
SOURCES         = client.cpp \
		  LoS.cpp \
		  corpse.cpp \
		  SkiTarg.cpp \
		  SndPkg.cpp \
		  Timing.cpp \
		  tmpeff.cpp \
		  Trade.cpp \
		  accounts.cpp \
		  basics.cpp \
		  boats.cpp \
		  books.cpp \
		  bounty.cpp \
		  cache.cpp \
                  chars.cpp \
                  charsmgr.cpp \
		  combat.cpp \
		  commands.cpp \
		  coord.cpp \
		  tilecache.cpp \
		  customtags.cpp \
		  cWeather.cpp \
		  dbl_single_click.cpp \
		  debug.cpp \
		  definable.cpp \
		  dragdrop.cpp \
		  fishing.cpp \
		  globals.cpp \
		  guildstones.cpp \
		  gumps.cpp \
		  house.cpp \
		  html.cpp \
		  im.cpp \
                  iserialization.cpp \
		  itemid.cpp \
		  items.cpp \
                  itemsmgr.cpp \
		  magic.cpp \
		  makemenu.cpp \
		  mapstuff.cpp \
		  msgboard.cpp \
		  necro.cpp \
		  netsys.cpp \
		  network.cpp \
		  npcs.cpp \
		  p_ai.cpp \
                  pfactory.cpp \
		  regions.cpp \
		  remadmin.cpp \
		  scripts.cpp \
                  serxmlfile.cpp \
                  serbinfile.cpp \
		  skills.cpp \
		  speech.cpp \
		  spawnregions.cpp \
                  srvparams.cpp \
		  storage.cpp \
		  targeting.cpp \
		  targetrequests.cpp \
		  telport.cpp \
                  territories.cpp \
		  uobject.cpp \
		  utilsys.cpp \
		  weight.cpp \
		  wolfpack.cpp \
		  worldmain.cpp \
		  wpconsole.cpp \
		  walking.cpp \
		  packlen.cpp \
		  wpdefmanager.cpp \
		  wpscriptmanager.cpp \
		  menuactions.cpp \
		  wptargetrequests.cpp 

# Network Module
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
		   python/pycoord.cpp \
		   python/socket.cpp

INTERFACES	=
TRANSLATIONS    = \
                  languages/wolfpack_pt-BR.ts \
                  languages/wolfpack_it.ts \
                  languages/wolfpack_nl.ts
