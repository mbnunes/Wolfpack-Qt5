# Microsoft Developer Studio Project File - Name="wolf" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** NICHT BEARBEITEN **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=wolf - Win32 Debug
!MESSAGE Dies ist kein gültiges Makefile. Zum Erstellen dieses Projekts mit NMAKE
!MESSAGE verwenden Sie den Befehl "Makefile exportieren" und führen Sie den Befehl
!MESSAGE 
!MESSAGE NMAKE /f "wolf.mak".
!MESSAGE 
!MESSAGE Sie können beim Ausführen von NMAKE eine Konfiguration angeben
!MESSAGE durch Definieren des Makros CFG in der Befehlszeile. Zum Beispiel:
!MESSAGE 
!MESSAGE NMAKE /f "wolf.mak" CFG="wolf - Win32 Debug"
!MESSAGE 
!MESSAGE Für die Konfiguration stehen zur Auswahl:
!MESSAGE 
!MESSAGE "wolf - Win32 Release" (basierend auf  "Win32 (x86) Console Application")
!MESSAGE "wolf - Win32 Debug" (basierend auf  "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "wolf"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "wolf - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release\Intermediate"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /I "$(QTDIR)\include" /I "..\src\sqlite" /I "..\build" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "QT_DLL" /D "QT_NO_STL" /D "QT_THREAD_SUPPORT" /D "__VC6" /Fr /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib ole32.lib advapi32.lib ws2_32.lib comctl32.lib $(QTDIR)\lib\qt-mt331.lib shell32.lib /nologo /subsystem:windows /map /machine:I386 /out:"release\wolfpack.exe" /libpath:"lib\ZThread\lib" /libpath:"lib\Python\lib" /libpath:"lib\bugreport\lib" /libpath:"flatstore\Release" /opt:ref /opt:nowin98
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "wolf - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug\Intermediate"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MD /W3 /Gm /GR /GX /ZI /Od /I "$(QTDIR)\include" /I "..\src\sqlite" /I "..\build" /D "_CONSOLE" /D "_MBCS" /D "_DEBUG" /D "WIN32" /D "QT_DLL" /D "QT_NO_STL" /D "QT_THREAD_SUPPORT" /D "__VC6" /Fr /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /fo"debug\res.res" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib ole32.lib advapi32.lib ws2_32.lib $(QTDIR)\lib\qt-mt331.lib shell32.lib /nologo /version:12.9 /subsystem:windows /pdb:none /debug /machine:I386 /out:".\debug\wolfpack.exe" /fixed:no
# SUBTRACT LINK32 /map
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Desc=MoveIt
PostBuild_Cmds=copy debug\wolfpack.exe d:\Wolfpack\WolfpackEXE\debug\current\wolfpack.exe
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "wolf - Win32 Release"
# Name "wolf - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\src\accounts.cpp
# End Source File
# Begin Source File

SOURCE=..\src\action.cpp
# End Source File
# Begin Source File

SOURCE=..\src\ai\ai.cpp
# End Source File
# Begin Source File

SOURCE=..\src\ai\ai_animals.cpp
# End Source File
# Begin Source File

SOURCE=..\src\ai\ai_bladespirit.cpp
# End Source File
# Begin Source File

SOURCE=..\src\ai\ai_commoner.cpp
# End Source File
# Begin Source File

SOURCE=..\src\ai\ai_energyvortex.cpp
# End Source File
# Begin Source File

SOURCE=..\src\ai\ai_humans.cpp
# End Source File
# Begin Source File

SOURCE=..\src\ai\ai_mage.cpp
# End Source File
# Begin Source File

SOURCE=..\src\ai\ai_monsters.cpp
# End Source File
# Begin Source File

SOURCE=..\src\basechar.cpp
# End Source File
# Begin Source File

SOURCE=..\src\basedef.cpp
# End Source File
# Begin Source File

SOURCE=..\src\basics.cpp
# End Source File
# Begin Source File

SOURCE=..\src\combat.cpp
# End Source File
# Begin Source File

SOURCE=..\src\commands.cpp
# End Source File
# Begin Source File

SOURCE=..\src\win\config_win.cpp
# End Source File
# Begin Source File

SOURCE=..\src\console.cpp
# End Source File
# Begin Source File

SOURCE=..\src\win\console_win.cpp
# End Source File
# Begin Source File

SOURCE=..\src\content.cpp
# End Source File
# Begin Source File

SOURCE=..\src\contextmenu.cpp
# End Source File
# Begin Source File

SOURCE=..\src\coord.cpp
# End Source File
# Begin Source File

SOURCE=..\src\corpse.cpp
# End Source File
# Begin Source File

SOURCE=..\src\customtags.cpp
# End Source File
# Begin Source File

SOURCE=..\src\dbdriver.cpp
# End Source File
# Begin Source File

SOURCE=..\src\definable.cpp
# End Source File
# Begin Source File

SOURCE=..\src\definitions.cpp
# End Source File
# Begin Source File

SOURCE=..\src\dragdrop.cpp
# End Source File
# Begin Source File

SOURCE=..\src\exportdefinitions.cpp
# End Source File
# Begin Source File

SOURCE=..\src\getopts.cpp
# End Source File
# Begin Source File

SOURCE=..\src\win\getopts_win.cpp
# End Source File
# Begin Source File

SOURCE=..\src\guilds.cpp
# End Source File
# Begin Source File

SOURCE=..\src\gumps.cpp
# End Source File
# Begin Source File

SOURCE=..\src\items.cpp
# End Source File
# Begin Source File

SOURCE=..\src\log.cpp
# End Source File
# Begin Source File

SOURCE=..\src\mapobjects.cpp
# End Source File
# Begin Source File

SOURCE=..\src\muls\maps.cpp
# End Source File
# Begin Source File

SOURCE=..\src\md5.cpp
# End Source File
# Begin Source File

SOURCE=..\src\multi.cpp
# End Source File
# Begin Source File

SOURCE=..\src\muls\multiscache.cpp
# End Source File
# Begin Source File

SOURCE=..\src\npc.cpp
# End Source File
# Begin Source File

SOURCE=..\src\party.cpp
# End Source File
# Begin Source File

SOURCE=..\src\pathfinding.cpp
# End Source File
# Begin Source File

SOURCE=..\src\persistentbroker.cpp
# End Source File
# Begin Source File

SOURCE=..\src\persistentobject.cpp
# End Source File
# Begin Source File

SOURCE=..\src\player.cpp
# End Source File
# Begin Source File

SOURCE=..\src\preferences.cpp
# End Source File
# Begin Source File

SOURCE=..\src\profile.cpp
# End Source File
# Begin Source File

SOURCE=..\src\pythonscript.cpp
# End Source File
# Begin Source File

SOURCE=..\src\scriptmanager.cpp
# End Source File
# Begin Source File

SOURCE=..\src\server.cpp
# End Source File
# Begin Source File

SOURCE=..\src\serverconfig.cpp
# End Source File
# Begin Source File

SOURCE=..\src\skills.cpp
# End Source File
# Begin Source File

SOURCE=..\src\spawnregions.cpp
# End Source File
# Begin Source File

SOURCE=..\src\speech.cpp
# End Source File
# Begin Source File

SOURCE=..\src\targetrequests.cpp
# End Source File
# Begin Source File

SOURCE=..\src\territories.cpp
# End Source File
# Begin Source File

SOURCE=..\src\muls\tilecache.cpp
# End Source File
# Begin Source File

SOURCE=..\src\timers.cpp
# End Source File
# Begin Source File

SOURCE=..\src\timing.cpp
# End Source File
# Begin Source File

SOURCE=..\src\tracking.cpp
# End Source File
# Begin Source File

SOURCE=..\src\trade.cpp
# End Source File
# Begin Source File

SOURCE=..\src\twofish\twofish2.cpp
# End Source File
# Begin Source File

SOURCE=..\src\uobject.cpp
# End Source File
# Begin Source File

SOURCE=..\src\walking.cpp
# End Source File
# Begin Source File

SOURCE=..\src\world.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\src\accounts.h
# End Source File
# Begin Source File

SOURCE=..\src\action.h
# End Source File
# Begin Source File

SOURCE=..\src\twofish\aes.h
# End Source File
# Begin Source File

SOURCE=..\src\ai\ai.h
# End Source File
# Begin Source File

SOURCE=..\src\ai\ai_bladespirit.h
# End Source File
# Begin Source File

SOURCE=..\src\ai\ai_commoner.h
# End Source File
# Begin Source File

SOURCE=..\src\ai\ai_energyvortex.h
# End Source File
# Begin Source File

SOURCE=..\src\ai\ai_mage.h
# End Source File
# Begin Source File

SOURCE=..\src\basechar.h
# End Source File
# Begin Source File

SOURCE=..\src\basedef.h
# End Source File
# Begin Source File

SOURCE=..\src\baseregion.h
# End Source File
# Begin Source File

SOURCE=..\src\basics.h
# End Source File
# Begin Source File

SOURCE=..\src\combat.h
# End Source File
# Begin Source File

SOURCE=..\src\commands.h
# End Source File
# Begin Source File

SOURCE=..\src\console.h
# End Source File
# Begin Source File

SOURCE=..\src\content.h
# End Source File
# Begin Source File

SOURCE=..\src\contextmenu.h
# End Source File
# Begin Source File

SOURCE=..\src\coord.h
# End Source File
# Begin Source File

SOURCE=..\src\corpse.h
# End Source File
# Begin Source File

SOURCE=..\src\customtags.h
# End Source File
# Begin Source File

SOURCE=..\src\dbdriver.h
# End Source File
# Begin Source File

SOURCE=..\src\twofish\debug.h
# End Source File
# Begin Source File

SOURCE=..\src\definable.h
# End Source File
# Begin Source File

SOURCE=..\src\defines.h
# End Source File
# Begin Source File

SOURCE=..\src\definitions.h
# End Source File
# Begin Source File

SOURCE=..\src\dragdrop.h
# End Source File
# Begin Source File

SOURCE=..\src\exceptions.h
# End Source File
# Begin Source File

SOURCE=..\src\exportdefinitions.h
# End Source File
# Begin Source File

SOURCE=..\src\factory.h
# End Source File
# Begin Source File

SOURCE=..\src\getopts.h
# End Source File
# Begin Source File

SOURCE=..\src\guilds.h
# End Source File
# Begin Source File

SOURCE=..\src\gumps.h
# End Source File
# Begin Source File

SOURCE=..\src\inlines.h
# End Source File
# Begin Source File

SOURCE=..\src\items.h
# End Source File
# Begin Source File

SOURCE=..\src\log.h
# End Source File
# Begin Source File

SOURCE=..\src\mapobjects.h
# End Source File
# Begin Source File

SOURCE=..\src\muls\maps.h
# End Source File
# Begin Source File

SOURCE=..\src\md5.h
# End Source File
# Begin Source File

SOURCE=..\src\mersennetwister.h
# End Source File
# Begin Source File

SOURCE=..\src\multi.h
# End Source File
# Begin Source File

SOURCE=..\src\muls\multiscache.h
# End Source File
# Begin Source File

SOURCE=..\src\npc.h
# End Source File
# Begin Source File

SOURCE=..\src\objectdef.h
# End Source File
# Begin Source File

SOURCE=..\src\party.h
# End Source File
# Begin Source File

SOURCE=..\src\pathfinding.h
# End Source File
# Begin Source File

SOURCE=..\src\persistentbroker.h
# End Source File
# Begin Source File

SOURCE=..\src\persistentobject.h
# End Source File
# Begin Source File

SOURCE=..\src\platform.h
# End Source File
# Begin Source File

SOURCE=..\src\twofish\platform.h
# End Source File
# Begin Source File

SOURCE=..\src\player.h
# End Source File
# Begin Source File

SOURCE=..\src\preferences.h
# End Source File
# Begin Source File

SOURCE=..\src\profile.h
# End Source File
# Begin Source File

SOURCE=..\src\progress.h
# End Source File
# Begin Source File

SOURCE=..\src\pythonscript.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=..\src\scriptmanager.h
# End Source File
# Begin Source File

SOURCE=..\src\server.h
# End Source File
# Begin Source File

SOURCE=..\src\serverconfig.h
# End Source File
# Begin Source File

SOURCE=..\src\singleton.h
# End Source File
# Begin Source File

SOURCE=..\src\skills.h
# End Source File
# Begin Source File

SOURCE=..\src\spawnregions.h
# End Source File
# Begin Source File

SOURCE=..\src\speech.h
# End Source File
# Begin Source File

SOURCE=..\src\twofish\table.h
# End Source File
# Begin Source File

SOURCE=..\src\targetrequest.h
# End Source File
# Begin Source File

SOURCE=..\src\targetrequests.h
# End Source File
# Begin Source File

SOURCE=..\src\territories.h
# End Source File
# Begin Source File

SOURCE=..\src\muls\tilecache.h
# End Source File
# Begin Source File

SOURCE=..\src\timers.h
# End Source File
# Begin Source File

SOURCE=..\src\timing.h
# End Source File
# Begin Source File

SOURCE=..\src\tracking.h
# End Source File
# Begin Source File

SOURCE=..\src\trade.h
# End Source File
# Begin Source File

SOURCE=..\src\typedefs.h
# End Source File
# Begin Source File

SOURCE=..\src\uobject.h
# End Source File
# Begin Source File

SOURCE=..\src\uotime.h
# End Source File
# Begin Source File

SOURCE=..\src\verinfo.h
# End Source File
# Begin Source File

SOURCE=..\src\walking.h
# End Source File
# Begin Source File

SOURCE=..\src\wolfpack_pch.h
# End Source File
# Begin Source File

SOURCE=..\src\world.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\icon2.ico
# End Source File
# Begin Source File

SOURCE=.\logo.bmp
# End Source File
# Begin Source File

SOURCE=.\res.rc
# End Source File
# End Group
# Begin Group "Network"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\src\network\asyncnetio.cpp
# End Source File
# Begin Source File

SOURCE=..\src\network\asyncnetio.h
# End Source File
# Begin Source File

SOURCE=..\src\network\encryption.cpp
# End Source File
# Begin Source File

SOURCE=..\src\network\encryption.h
# End Source File
# Begin Source File

SOURCE=..\src\network\listener.cpp
# End Source File
# Begin Source File

SOURCE=..\src\network\listener.h
# End Source File
# Begin Source File

SOURCE=..\src\network\network.cpp
# End Source File
# Begin Source File

SOURCE=..\src\network\network.h
# End Source File
# Begin Source File

SOURCE=..\src\network\uopacket.cpp
# End Source File
# Begin Source File

SOURCE=..\src\network\uopacket.h
# End Source File
# Begin Source File

SOURCE=..\src\network\uorxpackets.cpp
# End Source File
# Begin Source File

SOURCE=..\src\network\uorxpackets.h
# End Source File
# Begin Source File

SOURCE=..\src\network\uosocket.cpp
# End Source File
# Begin Source File

SOURCE=..\src\network\uosocket.h
# End Source File
# Begin Source File

SOURCE=..\src\network\uotxpackets.cpp
# End Source File
# Begin Source File

SOURCE=..\src\network\uotxpackets.h
# End Source File
# End Group
# Begin Group "Python"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\src\python\char.cpp
# End Source File
# Begin Source File

SOURCE=..\src\python\engine.cpp
# End Source File
# Begin Source File

SOURCE=..\src\python\engine.h
# End Source File
# Begin Source File

SOURCE=..\src\python\global.cpp
# End Source File
# Begin Source File

SOURCE=..\src\python\gump.h
# End Source File
# Begin Source File

SOURCE=..\src\python\item.cpp
# End Source File
# Begin Source File

SOURCE=..\src\python\objectcache.h
# End Source File
# Begin Source File

SOURCE=..\src\python\pyaccount.cpp
# End Source File
# Begin Source File

SOURCE=..\src\python\pyaction.cpp
# End Source File
# Begin Source File

SOURCE=..\src\python\pyaction.h
# End Source File
# Begin Source File

SOURCE=..\src\python\pyai.cpp
# End Source File
# Begin Source File

SOURCE=..\src\python\pycontent.h
# End Source File
# Begin Source File

SOURCE=..\src\python\pycoord.cpp
# End Source File
# Begin Source File

SOURCE=..\src\python\pypacket.cpp
# End Source File
# Begin Source File

SOURCE=..\src\python\pypacket.h
# End Source File
# Begin Source File

SOURCE=..\src\python\pyregion.cpp
# End Source File
# Begin Source File

SOURCE=..\src\python\pyspawnregion.cpp
# End Source File
# Begin Source File

SOURCE=..\src\python\pyspawnregion.h
# End Source File
# Begin Source File

SOURCE=..\src\python\pytooltip.cpp
# End Source File
# Begin Source File

SOURCE=..\src\python\region.cpp
# End Source File
# Begin Source File

SOURCE=..\src\python\regioniterator.h
# End Source File
# Begin Source File

SOURCE=..\src\python\skills.h
# End Source File
# Begin Source File

SOURCE=..\src\python\socket.cpp
# End Source File
# Begin Source File

SOURCE=..\src\python\target.cpp
# End Source File
# Begin Source File

SOURCE=..\src\python\target.h
# End Source File
# Begin Source File

SOURCE=..\src\python\tempeffect.h
# End Source File
# Begin Source File

SOURCE=..\src\python\utilities.h
# End Source File
# Begin Source File

SOURCE=..\src\python\worlditerator.cpp
# End Source File
# Begin Source File

SOURCE=..\src\python\worlditerator.h
# End Source File
# End Group
# Begin Group "SQLite"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\src\sqlite\btree.h
# End Source File
# Begin Source File

SOURCE=..\src\sqlite\config.h
# End Source File
# Begin Source File

SOURCE=..\src\sqlite\hash.h
# End Source File
# Begin Source File

SOURCE=..\src\sqlite\opcodes.h
# End Source File
# Begin Source File

SOURCE=..\src\sqlite\os.h
# End Source File
# Begin Source File

SOURCE=..\src\sqlite\pager.h
# End Source File
# Begin Source File

SOURCE=..\src\sqlite\parse.h
# End Source File
# Begin Source File

SOURCE=..\src\sqlite\sqlite.h
# End Source File
# Begin Source File

SOURCE=..\src\sqlite\sqlite_attach.c
# End Source File
# Begin Source File

SOURCE=..\src\sqlite\sqlite_auth.c
# End Source File
# Begin Source File

SOURCE=..\src\sqlite\sqlite_btree.c
# End Source File
# Begin Source File

SOURCE=..\src\sqlite\sqlite_btree_rb.c
# End Source File
# Begin Source File

SOURCE=..\src\sqlite\sqlite_build.c
# End Source File
# Begin Source File

SOURCE=..\src\sqlite\sqlite_copy.c
# End Source File
# Begin Source File

SOURCE=..\src\sqlite\sqlite_date.c
# End Source File
# Begin Source File

SOURCE=..\src\sqlite\sqlite_delete.c
# End Source File
# Begin Source File

SOURCE=..\src\sqlite\sqlite_expr.c
# End Source File
# Begin Source File

SOURCE=..\src\sqlite\sqlite_func.c
# End Source File
# Begin Source File

SOURCE=..\src\sqlite\sqlite_hash.c
# End Source File
# Begin Source File

SOURCE=..\src\sqlite\sqlite_insert.c
# End Source File
# Begin Source File

SOURCE=..\src\sqlite\sqlite_main.c
# End Source File
# Begin Source File

SOURCE=..\src\sqlite\sqlite_opcodes.c
# End Source File
# Begin Source File

SOURCE=..\src\sqlite\sqlite_os.c
# End Source File
# Begin Source File

SOURCE=..\src\sqlite\sqlite_pager.c
# End Source File
# Begin Source File

SOURCE=..\src\sqlite\sqlite_parse.c
# End Source File
# Begin Source File

SOURCE=..\src\sqlite\sqlite_pragma.c
# End Source File
# Begin Source File

SOURCE=..\src\sqlite\sqlite_printf.c
# End Source File
# Begin Source File

SOURCE=..\src\sqlite\sqlite_random.c
# End Source File
# Begin Source File

SOURCE=..\src\sqlite\sqlite_select.c
# End Source File
# Begin Source File

SOURCE=..\src\sqlite\sqlite_table.c
# End Source File
# Begin Source File

SOURCE=..\src\sqlite\sqlite_tokenize.c
# End Source File
# Begin Source File

SOURCE=..\src\sqlite\sqlite_trigger.c
# End Source File
# Begin Source File

SOURCE=..\src\sqlite\sqlite_update.c
# End Source File
# Begin Source File

SOURCE=..\src\sqlite\sqlite_util.c
# End Source File
# Begin Source File

SOURCE=..\src\sqlite\sqlite_vacuum.c
# End Source File
# Begin Source File

SOURCE=..\src\sqlite\sqlite_vdbe.c
# End Source File
# Begin Source File

SOURCE=..\src\sqlite\sqlite_vdbeaux.c
# End Source File
# Begin Source File

SOURCE=..\src\sqlite\sqlite_where.c
# End Source File
# Begin Source File

SOURCE=..\src\sqlite\sqliteInt.h
# End Source File
# Begin Source File

SOURCE=..\src\sqlite\vdbe.h
# End Source File
# Begin Source File

SOURCE=..\src\sqlite\vdbeInt.h
# End Source File
# End Group
# Begin Group "SQLite3"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\src\sqlite3\alter.c
# End Source File
# Begin Source File

SOURCE=..\src\sqlite3\attach.c
# End Source File
# Begin Source File

SOURCE=..\src\sqlite3\auth.c
# End Source File
# Begin Source File

SOURCE=..\src\sqlite3\btree.c
# End Source File
# Begin Source File

SOURCE=..\src\sqlite3\btree.h
# End Source File
# Begin Source File

SOURCE=..\src\sqlite3\build.c
# End Source File
# Begin Source File

SOURCE=..\src\sqlite3\config.h
# End Source File
# Begin Source File

SOURCE=..\src\sqlite3\date.c
# End Source File
# Begin Source File

SOURCE=..\src\sqlite3\delete.c
# End Source File
# Begin Source File

SOURCE=..\src\sqlite3\expr.c
# End Source File
# Begin Source File

SOURCE=..\src\sqlite3\func.c
# End Source File
# Begin Source File

SOURCE=..\src\sqlite3\hash.c
# End Source File
# Begin Source File

SOURCE=..\src\sqlite3\hash.h
# End Source File
# Begin Source File

SOURCE=..\src\sqlite3\insert.c
# End Source File
# Begin Source File

SOURCE=..\src\sqlite3\keywordhash.h
# End Source File
# Begin Source File

SOURCE=..\src\sqlite3\legacy.c
# End Source File
# Begin Source File

SOURCE=..\src\sqlite3\main.c
# End Source File
# Begin Source File

SOURCE=..\src\sqlite3\opcodes.c
# End Source File
# Begin Source File

SOURCE=..\src\sqlite3\opcodes.h
# End Source File
# Begin Source File

SOURCE=..\src\sqlite3\os.h
# End Source File
# Begin Source File

SOURCE=..\src\sqlite3\os_common.h
# End Source File
# Begin Source File

SOURCE=..\src\sqlite3\os_unix.c
# End Source File
# Begin Source File

SOURCE=..\src\sqlite3\os_unix.h
# End Source File
# Begin Source File

SOURCE=..\src\sqlite3\os_win.c
# End Source File
# Begin Source File

SOURCE=..\src\sqlite3\os_win.h
# End Source File
# Begin Source File

SOURCE=..\src\sqlite3\pager.c
# End Source File
# Begin Source File

SOURCE=..\src\sqlite3\pager.h
# End Source File
# Begin Source File

SOURCE=..\src\sqlite3\parse.c
# End Source File
# Begin Source File

SOURCE=..\src\sqlite3\parse.h
# End Source File
# Begin Source File

SOURCE=..\src\sqlite3\pragma.c
# End Source File
# Begin Source File

SOURCE=..\src\sqlite3\printf.c
# End Source File
# Begin Source File

SOURCE=..\src\sqlite3\random.c
# End Source File
# Begin Source File

SOURCE=..\src\sqlite3\select.c
# End Source File
# Begin Source File

SOURCE=..\src\sqlite3\sqlite3.h
# End Source File
# Begin Source File

SOURCE=..\src\sqlite3\sqliteInt.h
# End Source File
# Begin Source File

SOURCE=..\src\sqlite3\table.c
# End Source File
# Begin Source File

SOURCE=..\src\sqlite3\tokenize.c
# End Source File
# Begin Source File

SOURCE=..\src\sqlite3\trigger.c
# End Source File
# Begin Source File

SOURCE=..\src\sqlite3\update.c
# End Source File
# Begin Source File

SOURCE=..\src\sqlite3\utf.c
# End Source File
# Begin Source File

SOURCE=..\src\sqlite3\util.c
# End Source File
# Begin Source File

SOURCE=..\src\sqlite3\vacuum.c
# End Source File
# Begin Source File

SOURCE=..\src\sqlite3\vdbe.c
# End Source File
# Begin Source File

SOURCE=..\src\sqlite3\vdbe.h
# End Source File
# Begin Source File

SOURCE=..\src\sqlite3\vdbeapi.c
# End Source File
# Begin Source File

SOURCE=..\src\sqlite3\vdbeaux.c
# End Source File
# Begin Source File

SOURCE=..\src\sqlite3\vdbeInt.h
# End Source File
# Begin Source File

SOURCE=..\src\sqlite3\vdbemem.c
# End Source File
# Begin Source File

SOURCE=..\src\sqlite3\where.c
# End Source File
# End Group
# Begin Source File

SOURCE=.\icon_green.ico
# End Source File
# Begin Source File

SOURCE=.\icon_red.ico
# End Source File
# End Target
# End Project
