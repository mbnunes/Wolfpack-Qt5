#
# Wolfpack TMake project
#
#
#

PROJECT         = Wolfpack Emu
TARGET          = wolfpack
TEMPLATE        = app
CONFIG          = console release thread
INCLUDEPATH     = lib/ZThread/include lib/wrl/include
DEFINES         = REENTRANT ZTHREAD_STATIC
win32:OBJECTS_DIR = obj
win32-msvc:DEFINES += WIN32 NDEBUG _CONSOLE _MBCS
win32-g++:DEFINES = WIN32

unix:LIBS       = -Llib/ZThread/lib -Llib/wrl/lib -lZThread -lwrl


unix:DEFINES    += NDEBUG
unix:TMAKE_CXXFLAGS = -funsigned-char

win32-g++:TMAKE_CXXFLAGS = -funsigned-char
win32-g++:LIBS= -Llib/ZThread/lib/ -Llib/wrl/lib -lwsock32 -lZThread -lwrl
win32-msvc:RC_FILE         = res.rc
win32-msvc:LIBS      = ws2_32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comdlg32.lib ws2_32.lib ZThread.lib wrl.lib
win32-msvc:TMAKE_CXXFLAGS = /J /nologo /ML /W3 /GX /O2 /YX /FD /c
win32-borland:DEFINES= WIN32 __borland__
win32-borland:TMAKE_CXXFLAGS = -K -5 -w-8057 -w-8066 -w-8060 -w-8027 -w-8059 -w-8004 -w-8012
win32-borland:LIBS = ws2_32.lib ZThread.lib wrl.lib

HEADERS         = Client.h \
		  SndPkg.h \
		  SrvParms.h \
		  Timing.h \
		  TmpEff.h \
		  Trade.h \
		  accounts.h \
		  admin.h \
		  archive.h \
		  basics.h \
		  boats.h \
		  books.h \
		  bounty.h \
		  chars.h \
                  charsmgr.h \
		  classes.h \
		  cmdtable.h \
		  coord.h \
		  combat.h \
		  cweather.h \
		  dbl_single_click.h \
		  debug.h \
		  defines.h \
		  dragdrop.h \
		  globals.h \
		  guildstones.h \
		  gumps.h \
		  hCache.h \
		  hFile.h \
		  im.h \
		  inlines.h \
                  iserialization.h \
		  itemid.h \
		  items.h \
                  itemsmgr.h \
		  junk.h \
		  mapstuff.h \
		  msgboard.h \
		  mstring.h \
		  netsys.h \
		  network.h \
		  platform.h \
		  prototypes.h \
                  races.h \
                  rcvpkg.h \
		  regions.h \
		  remadmin.h \
		  resource.h \
		  scriptc.h \
		  speech.h \
		  sregions.h \
                  srvparams.h \
                  sertextfile.h \
                  serbinfile.h \
		  stl_headers_end.h \
		  stl_headers_start.h \
		  storage.h \
		  stream.h \
		  structs.h \
		  targeting.h \
		  telport.h \
		  trigger.h \
		  typedefs.h \
		  uobject.h \
		  utilsys.h \
		  verinfo.h \
		  weight.h \
		  whitespace.h \
		  wip.h \
		  wolfpack.h \
		  worldmain.h \
		  wpconsole.h \
		  tilecache.h \
		  walking2.h \
		  packlen.h \
		  extract.h

SOURCES         = Client.cpp \
		  LoS.cpp \
		  SkiTarg.cpp \
		  SndPkg.cpp \
		  SrvParms.cpp \
		  Timing.cpp \
		  TmpEff.cpp \
		  Trade.cpp \
		  accounts.cpp \
		  admin.cpp \
		  archive.cpp \
		  basics.cpp \
		  boats.cpp \
		  books.cpp \
		  bounty.cpp \
		  cache.cpp \
                  chars.cpp \
                  charsmgr.cpp \
		  cmdtable.cpp \
		  combat.cpp \
		  commands.cpp \
		  coord.cpp \
		  cWeather.cpp \
		  dbl_single_click.cpp \
		  debug.cpp \
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
		  mapstuff.cpp \
		  msgboard.cpp \
		  mstring.cpp \
		  necro.cpp \
		  netsys.cpp \
		  network.cpp \
		  newbie.cpp \
		  npcs.cpp \
		  p_ai.cpp \
		  pointer.cpp \
		  qsf.cpp \
                  races.cpp \
                  rcvpkg.cpp \
		  regions.cpp \
		  remadmin.cpp \
		  scriptc.cpp \
		  scripts.cpp \
                  sertextfile.cpp \
                  serbinfile.cpp \
		  skills.cpp \
		  speech.cpp \
		  sregions.cpp \
                  srvparams.cpp \
		  storage.cpp \
		  targeting.cpp \
		  telport.cpp \
		  translate.cpp \
		  trigger.cpp \
		  uobject.cpp \
		  utilsys.cpp \
		  weight.cpp \
		  wip.cpp \
		  wolfpack.cpp \
		  worldmain.cpp \
		  wpconsole.cpp \
		  tilecache.cpp \
		  walking2.cpp \
		  packlen.cpp
INTERFACES	=
TRANSLATIONS    = \
                  languages/wolfpack_pt-BR.ts \
                  languages/wolfpack_it.ts

