# Microsoft Developer Studio Project File - Name="wolf" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=wolf - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "wolf.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "wolf.mak" CFG="wolf - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "wolf - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "wolf - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
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
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MT /GR /GX /O2 /Oy- /Ob0 /I "lib\Python\PC" /I "lib\Python\include" /I "lib\ZThread\include" /I "lib\wrl\include" /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "ZTHREAD_STATIC" /YX /FD /c
# SUBTRACT CPP /Fr
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 ws2_32.lib kernel32.lib user32.lib advapi32.lib ZThread.lib wrl.lib /nologo /subsystem:console /incremental:yes /map /machine:I386 /libpath:"lib\ZThread\lib" /libpath:"lib\wrl\lib" /libpath:"lib\Python\lib"

!ELSEIF  "$(CFG)" == "wolf - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /Gm /GR /GX /ZI /Od /I "lib/Python/PC" /I "lib/Python/include" /I "lib\ZThread\include" /I "lib\wrl\include" /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "ZTHREAD_STATIC" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 ws2_32.lib kernel32.lib user32.lib advapi32.lib ZThread.lib ws2_32.lib kernel32.lib user32.lib advapi32.lib wrl.lib /nologo /subsystem:console /debug /machine:I386 /out:"C:\wolfpack\wolf.exe" /pdbtype:sept /libpath:"lib\ZThread\lib" /libpath:"lib\wrl\lib" /libpath:"lib\Python\lib"

!ENDIF 

# Begin Target

# Name "wolf - Win32 Release"
# Name "wolf - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\accounts.cpp
# End Source File
# Begin Source File

SOURCE=.\basics.cpp
# End Source File
# Begin Source File

SOURCE=.\boats.cpp
# End Source File
# Begin Source File

SOURCE=.\books.cpp
# End Source File
# Begin Source File

SOURCE=.\bounty.cpp
# End Source File
# Begin Source File

SOURCE=.\cache.cpp
# End Source File
# Begin Source File

SOURCE=.\chars.cpp
# End Source File
# Begin Source File

SOURCE=.\charsmgr.cpp
# End Source File
# Begin Source File

SOURCE=.\Client.cpp
# End Source File
# Begin Source File

SOURCE=.\cmdtable.cpp
# End Source File
# Begin Source File

SOURCE=.\combat.cpp
# End Source File
# Begin Source File

SOURCE=.\commands.cpp
# End Source File
# Begin Source File

SOURCE=.\coord.cpp
# End Source File
# Begin Source File

SOURCE=.\cweather.cpp
# End Source File
# Begin Source File

SOURCE=.\dbl_single_click.cpp
# End Source File
# Begin Source File

SOURCE=.\debug.cpp
# End Source File
# Begin Source File

SOURCE=.\dragdrop.cpp
# End Source File
# Begin Source File

SOURCE=.\fishing.cpp
# End Source File
# Begin Source File

SOURCE=.\globals.cpp
# End Source File
# Begin Source File

SOURCE=.\guildstones.cpp
# End Source File
# Begin Source File

SOURCE=.\gumps.cpp
# End Source File
# Begin Source File

SOURCE=.\house.cpp
# End Source File
# Begin Source File

SOURCE=.\html.cpp
# End Source File
# Begin Source File

SOURCE=.\im.cpp
# End Source File
# Begin Source File

SOURCE=.\iserialization.cpp
# End Source File
# Begin Source File

SOURCE=.\itemid.cpp
# End Source File
# Begin Source File

SOURCE=.\items.cpp
# End Source File
# Begin Source File

SOURCE=.\itemsmgr.cpp
# End Source File
# Begin Source File

SOURCE=.\LoS.cpp
# End Source File
# Begin Source File

SOURCE=.\magic.cpp
# End Source File
# Begin Source File

SOURCE=.\mapstuff.cpp
# End Source File
# Begin Source File

SOURCE=.\MenuActions.cpp
# End Source File
# Begin Source File

SOURCE=.\msgboard.cpp
# End Source File
# Begin Source File

SOURCE=.\necro.cpp
# End Source File
# Begin Source File

SOURCE=.\netsys.cpp
# End Source File
# Begin Source File

SOURCE=.\network.cpp
# End Source File
# Begin Source File

SOURCE=.\newbie.cpp
# End Source File
# Begin Source File

SOURCE=.\npcs.cpp
# End Source File
# Begin Source File

SOURCE=.\p_ai.cpp
# End Source File
# Begin Source File

SOURCE=.\packlen.cpp
# End Source File
# Begin Source File

SOURCE=.\pfactory.cpp
# End Source File
# Begin Source File

SOURCE=.\pointer.cpp
# End Source File
# Begin Source File

SOURCE=.\qsf.cpp
# End Source File
# Begin Source File

SOURCE=.\rcvpkg.cpp
# End Source File
# Begin Source File

SOURCE=.\regions.cpp
# End Source File
# Begin Source File

SOURCE=.\remadmin.cpp
# End Source File
# Begin Source File

SOURCE=.\scriptc.cpp
# End Source File
# Begin Source File

SOURCE=.\scripts.cpp
# End Source File
# Begin Source File

SOURCE=.\serbinfile.cpp
# End Source File
# Begin Source File

SOURCE=.\serxmlfile.cpp
# End Source File
# Begin Source File

SOURCE=.\skills.cpp
# End Source File
# Begin Source File

SOURCE=.\SkiTarg.cpp
# End Source File
# Begin Source File

SOURCE=.\SndPkg.cpp
# End Source File
# Begin Source File

SOURCE=.\speech.cpp
# End Source File
# Begin Source File

SOURCE=.\sregions.cpp
# End Source File
# Begin Source File

SOURCE=.\srvparams.cpp
# End Source File
# Begin Source File

SOURCE=.\storage.cpp
# End Source File
# Begin Source File

SOURCE=.\targeting.cpp
# End Source File
# Begin Source File

SOURCE=.\targetrequests.cpp
# End Source File
# Begin Source File

SOURCE=.\telport.cpp
# End Source File
# Begin Source File

SOURCE=.\tilecache.cpp
# End Source File
# Begin Source File

SOURCE=.\Timing.cpp
# End Source File
# Begin Source File

SOURCE=.\TmpEff.cpp
# End Source File
# Begin Source File

SOURCE=.\Trade.cpp
# End Source File
# Begin Source File

SOURCE=.\trigger.cpp
# End Source File
# Begin Source File

SOURCE=.\uobject.cpp
# End Source File
# Begin Source File

SOURCE=.\utilsys.cpp
# End Source File
# Begin Source File

SOURCE=.\walking2.cpp
# End Source File
# Begin Source File

SOURCE=.\weight.cpp
# End Source File
# Begin Source File

SOURCE=.\wip.cpp
# End Source File
# Begin Source File

SOURCE=.\wolfpack.cpp
# End Source File
# Begin Source File

SOURCE=.\worldmain.cpp
# End Source File
# Begin Source File

SOURCE=.\wpconsole.cpp
# End Source File
# Begin Source File

SOURCE=.\wpdefmanager.cpp
# End Source File
# Begin Source File

SOURCE=.\wppythonscript.cpp
# End Source File
# Begin Source File

SOURCE=.\wpscriptmanager.cpp
# End Source File
# Begin Source File

SOURCE=.\wptargetrequests.cpp
# End Source File
# Begin Source File

SOURCE=.\wpxmlparser.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\accounts.h
# End Source File
# Begin Source File

SOURCE=.\basics.h
# End Source File
# Begin Source File

SOURCE=.\boats.h
# End Source File
# Begin Source File

SOURCE=.\books.h
# End Source File
# Begin Source File

SOURCE=.\bounty.h
# End Source File
# Begin Source File

SOURCE=.\chars.h
# End Source File
# Begin Source File

SOURCE=.\charsmgr.h
# End Source File
# Begin Source File

SOURCE=.\classes.h
# End Source File
# Begin Source File

SOURCE=.\Client.h
# End Source File
# Begin Source File

SOURCE=.\cmdtable.h
# End Source File
# Begin Source File

SOURCE=.\coord.h
# End Source File
# Begin Source File

SOURCE=.\cweather.h
# End Source File
# Begin Source File

SOURCE=.\dbl_single_click.h
# End Source File
# Begin Source File

SOURCE=.\debug.h
# End Source File
# Begin Source File

SOURCE=.\defines.h
# End Source File
# Begin Source File

SOURCE=.\dragdrop.h
# End Source File
# Begin Source File

SOURCE=.\globals.h
# End Source File
# Begin Source File

SOURCE=.\guildstones.h
# End Source File
# Begin Source File

SOURCE=.\gumps.h
# End Source File
# Begin Source File

SOURCE=.\hCache.h
# End Source File
# Begin Source File

SOURCE=.\house.h
# End Source File
# Begin Source File

SOURCE=.\im.h
# End Source File
# Begin Source File

SOURCE=.\inlines.h
# End Source File
# Begin Source File

SOURCE=.\iserialization.h
# End Source File
# Begin Source File

SOURCE=.\itemid.h
# End Source File
# Begin Source File

SOURCE=.\items.h
# End Source File
# Begin Source File

SOURCE=.\itemsmgr.h
# End Source File
# Begin Source File

SOURCE=.\junk.h
# End Source File
# Begin Source File

SOURCE=.\magic.h
# End Source File
# Begin Source File

SOURCE=.\mapcache.h
# End Source File
# Begin Source File

SOURCE=.\MenuActions.h
# End Source File
# Begin Source File

SOURCE=.\msgboard.h
# End Source File
# Begin Source File

SOURCE=.\mulstructs.h
# End Source File
# Begin Source File

SOURCE=.\multicache.h
# End Source File
# Begin Source File

SOURCE=.\netsys.h
# End Source File
# Begin Source File

SOURCE=.\network.h
# End Source File
# Begin Source File

SOURCE=.\packlen.h
# End Source File
# Begin Source File

SOURCE=.\pfactory.h
# End Source File
# Begin Source File

SOURCE=.\platform.h
# End Source File
# Begin Source File

SOURCE=.\prototypes.h
# End Source File
# Begin Source File

SOURCE=.\rcvpkg.h
# End Source File
# Begin Source File

SOURCE=.\regions.h
# End Source File
# Begin Source File

SOURCE=.\remadmin.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\scriptc.h
# End Source File
# Begin Source File

SOURCE=.\serbinfile.h
# End Source File
# Begin Source File

SOURCE=.\serxmlfile.h
# End Source File
# Begin Source File

SOURCE=.\SndPkg.h
# End Source File
# Begin Source File

SOURCE=.\speech.h
# End Source File
# Begin Source File

SOURCE=.\sregions.h
# End Source File
# Begin Source File

SOURCE=.\srvparams.h
# End Source File
# Begin Source File

SOURCE=.\stl_headers_end.h
# End Source File
# Begin Source File

SOURCE=.\stl_headers_start.h
# End Source File
# Begin Source File

SOURCE=.\storage.h
# End Source File
# Begin Source File

SOURCE=.\stream.h
# End Source File
# Begin Source File

SOURCE=.\structs.h
# End Source File
# Begin Source File

SOURCE=.\targeting.h
# End Source File
# Begin Source File

SOURCE=.\targetrequests.h
# End Source File
# Begin Source File

SOURCE=.\telport.h
# End Source File
# Begin Source File

SOURCE=.\tilecache.h
# End Source File
# Begin Source File

SOURCE=.\tileflags.h
# End Source File
# Begin Source File

SOURCE=.\Timing.h
# End Source File
# Begin Source File

SOURCE=.\TmpEff.h
# End Source File
# Begin Source File

SOURCE=.\Trade.h
# End Source File
# Begin Source File

SOURCE=.\trigger.h
# End Source File
# Begin Source File

SOURCE=.\typedefs.h
# End Source File
# Begin Source File

SOURCE=.\uobject.h
# End Source File
# Begin Source File

SOURCE=.\utilsys.h
# End Source File
# Begin Source File

SOURCE=.\verinfo.h
# End Source File
# Begin Source File

SOURCE=.\walking2.h
# End Source File
# Begin Source File

SOURCE=.\weight.h
# End Source File
# Begin Source File

SOURCE=.\whitespace.h
# End Source File
# Begin Source File

SOURCE=.\wip.h
# End Source File
# Begin Source File

SOURCE=.\wolfpack.h
# End Source File
# Begin Source File

SOURCE=.\worldmain.h
# End Source File
# Begin Source File

SOURCE=.\wpconsole.h
# End Source File
# Begin Source File

SOURCE=.\wpdefaultscript.h
# End Source File
# Begin Source File

SOURCE=.\wpdefmanager.h
# End Source File
# Begin Source File

SOURCE=.\wppythonscript.h
# End Source File
# Begin Source File

SOURCE=.\wpscriptmanager.h
# End Source File
# Begin Source File

SOURCE=.\wptargetrequests.h
# End Source File
# Begin Source File

SOURCE=.\wpxmlparser.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\icon2.ico
# End Source File
# Begin Source File

SOURCE=.\res.rc
# End Source File
# End Group
# End Target
# End Project
