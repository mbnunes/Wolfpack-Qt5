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
# ADD CPP /nologo /G5 /MD /W4 /GX /Zi /O2 /Oy- /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /FR /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib Ws2_32.lib /nologo /subsystem:console /debug /machine:I386 /opt:ref
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
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /G5 /MD /w /W0 /GX /ZI /Od /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib Ws2_32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# SUBTRACT LINK32 /pdb:none /incremental:no

!ENDIF 

# Begin Target

# Name "wolf - Win32 Release"
# Name "wolf - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\accounts.cpp

!IF  "$(CFG)" == "wolf - Win32 Release"

!ELSEIF  "$(CFG)" == "wolf - Win32 Debug"

# ADD CPP /W4

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\admin.cpp

!IF  "$(CFG)" == "wolf - Win32 Release"

!ELSEIF  "$(CFG)" == "wolf - Win32 Debug"

# ADD CPP /W4

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\archive.cpp

!IF  "$(CFG)" == "wolf - Win32 Release"

!ELSEIF  "$(CFG)" == "wolf - Win32 Debug"

# ADD CPP /W4

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\basics.cpp

!IF  "$(CFG)" == "wolf - Win32 Release"

!ELSEIF  "$(CFG)" == "wolf - Win32 Debug"

# ADD CPP /W4

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\boats.cpp

!IF  "$(CFG)" == "wolf - Win32 Release"

!ELSEIF  "$(CFG)" == "wolf - Win32 Debug"

# ADD CPP /W4

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\books.cpp

!IF  "$(CFG)" == "wolf - Win32 Release"

!ELSEIF  "$(CFG)" == "wolf - Win32 Debug"

# ADD CPP /W4

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\bounty.cpp

!IF  "$(CFG)" == "wolf - Win32 Release"

!ELSEIF  "$(CFG)" == "wolf - Win32 Debug"

# ADD CPP /W4

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\cache.cpp

!IF  "$(CFG)" == "wolf - Win32 Release"

!ELSEIF  "$(CFG)" == "wolf - Win32 Debug"

# ADD CPP /W4

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\cFile.cpp

!IF  "$(CFG)" == "wolf - Win32 Release"

!ELSEIF  "$(CFG)" == "wolf - Win32 Debug"

# ADD CPP /W4

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\char_array.cpp

!IF  "$(CFG)" == "wolf - Win32 Release"

!ELSEIF  "$(CFG)" == "wolf - Win32 Debug"

# ADD CPP /W4

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\CharWrap.cpp

!IF  "$(CFG)" == "wolf - Win32 Release"

!ELSEIF  "$(CFG)" == "wolf - Win32 Debug"

# ADD CPP /W4

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Client.cpp

!IF  "$(CFG)" == "wolf - Win32 Release"

!ELSEIF  "$(CFG)" == "wolf - Win32 Debug"

# ADD CPP /W4

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\cmdtable.cpp

!IF  "$(CFG)" == "wolf - Win32 Release"

!ELSEIF  "$(CFG)" == "wolf - Win32 Debug"

# ADD CPP /W4

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\combat.cpp

!IF  "$(CFG)" == "wolf - Win32 Release"

!ELSEIF  "$(CFG)" == "wolf - Win32 Debug"

# ADD CPP /W4

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\commands.cpp

!IF  "$(CFG)" == "wolf - Win32 Release"

!ELSEIF  "$(CFG)" == "wolf - Win32 Debug"

# ADD CPP /W4

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\coord.cpp

!IF  "$(CFG)" == "wolf - Win32 Release"

!ELSEIF  "$(CFG)" == "wolf - Win32 Debug"

# ADD CPP /W4

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\dbl_single_click.cpp

!IF  "$(CFG)" == "wolf - Win32 Release"

!ELSEIF  "$(CFG)" == "wolf - Win32 Debug"

# ADD CPP /W4

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\debug.cpp

!IF  "$(CFG)" == "wolf - Win32 Release"

!ELSEIF  "$(CFG)" == "wolf - Win32 Debug"

# ADD CPP /W4

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\dragdrop.cpp

!IF  "$(CFG)" == "wolf - Win32 Release"

!ELSEIF  "$(CFG)" == "wolf - Win32 Debug"

# ADD CPP /W4

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\fishing.cpp

!IF  "$(CFG)" == "wolf - Win32 Release"

!ELSEIF  "$(CFG)" == "wolf - Win32 Debug"

# ADD CPP /W4

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\globals.cpp

!IF  "$(CFG)" == "wolf - Win32 Release"

!ELSEIF  "$(CFG)" == "wolf - Win32 Debug"

# ADD CPP /W4

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\guildstones.cpp

!IF  "$(CFG)" == "wolf - Win32 Release"

!ELSEIF  "$(CFG)" == "wolf - Win32 Debug"

# ADD CPP /W4

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gumps.cpp

!IF  "$(CFG)" == "wolf - Win32 Release"

!ELSEIF  "$(CFG)" == "wolf - Win32 Debug"

# ADD CPP /W4

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\house.cpp

!IF  "$(CFG)" == "wolf - Win32 Release"

!ELSEIF  "$(CFG)" == "wolf - Win32 Debug"

# ADD CPP /W4

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\html.cpp

!IF  "$(CFG)" == "wolf - Win32 Release"

!ELSEIF  "$(CFG)" == "wolf - Win32 Debug"

# ADD CPP /W4

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\im.cpp

!IF  "$(CFG)" == "wolf - Win32 Release"

!ELSEIF  "$(CFG)" == "wolf - Win32 Debug"

# ADD CPP /W4

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\item_array.cpp

!IF  "$(CFG)" == "wolf - Win32 Release"

!ELSEIF  "$(CFG)" == "wolf - Win32 Debug"

# ADD CPP /W4

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\itemid.cpp

!IF  "$(CFG)" == "wolf - Win32 Release"

!ELSEIF  "$(CFG)" == "wolf - Win32 Debug"

# ADD CPP /W4

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\items.cpp

!IF  "$(CFG)" == "wolf - Win32 Release"

!ELSEIF  "$(CFG)" == "wolf - Win32 Debug"

# ADD CPP /W4

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ItemWrap.cpp

!IF  "$(CFG)" == "wolf - Win32 Release"

!ELSEIF  "$(CFG)" == "wolf - Win32 Debug"

# ADD CPP /W4

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\LoS.cpp

!IF  "$(CFG)" == "wolf - Win32 Release"

!ELSEIF  "$(CFG)" == "wolf - Win32 Debug"

# ADD CPP /W4

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\magic.cpp

!IF  "$(CFG)" == "wolf - Win32 Release"

!ELSEIF  "$(CFG)" == "wolf - Win32 Debug"

# ADD CPP /W4

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\mapcache.cpp

!IF  "$(CFG)" == "wolf - Win32 Release"

!ELSEIF  "$(CFG)" == "wolf - Win32 Debug"

# ADD CPP /W4

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\mapstuff.cpp

!IF  "$(CFG)" == "wolf - Win32 Release"

!ELSEIF  "$(CFG)" == "wolf - Win32 Debug"

# ADD CPP /W4

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\msgboard.cpp

!IF  "$(CFG)" == "wolf - Win32 Release"

!ELSEIF  "$(CFG)" == "wolf - Win32 Debug"

# ADD CPP /W4

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\mstring.cpp

!IF  "$(CFG)" == "wolf - Win32 Release"

!ELSEIF  "$(CFG)" == "wolf - Win32 Debug"

# ADD CPP /W4

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\multicache.cpp

!IF  "$(CFG)" == "wolf - Win32 Release"

!ELSEIF  "$(CFG)" == "wolf - Win32 Debug"

# ADD CPP /W4

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\necro.cpp

!IF  "$(CFG)" == "wolf - Win32 Release"

!ELSEIF  "$(CFG)" == "wolf - Win32 Debug"

# ADD CPP /W4

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\netsys.cpp

!IF  "$(CFG)" == "wolf - Win32 Release"

!ELSEIF  "$(CFG)" == "wolf - Win32 Debug"

# ADD CPP /W4

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\network.cpp

!IF  "$(CFG)" == "wolf - Win32 Release"

!ELSEIF  "$(CFG)" == "wolf - Win32 Debug"

# ADD CPP /W4

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\newbie.cpp

!IF  "$(CFG)" == "wolf - Win32 Release"

!ELSEIF  "$(CFG)" == "wolf - Win32 Debug"

# ADD CPP /W4

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\npcs.cpp

!IF  "$(CFG)" == "wolf - Win32 Release"

!ELSEIF  "$(CFG)" == "wolf - Win32 Debug"

# ADD CPP /W4

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\p_ai.cpp

!IF  "$(CFG)" == "wolf - Win32 Release"

!ELSEIF  "$(CFG)" == "wolf - Win32 Debug"

# ADD CPP /W4

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\pointer.cpp

!IF  "$(CFG)" == "wolf - Win32 Release"

!ELSEIF  "$(CFG)" == "wolf - Win32 Debug"

# ADD CPP /W4

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\qsf.cpp

!IF  "$(CFG)" == "wolf - Win32 Release"

!ELSEIF  "$(CFG)" == "wolf - Win32 Debug"

# ADD CPP /W4

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\rcvpkg.cpp

!IF  "$(CFG)" == "wolf - Win32 Release"

!ELSEIF  "$(CFG)" == "wolf - Win32 Debug"

# ADD CPP /W4

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\regions.cpp

!IF  "$(CFG)" == "wolf - Win32 Release"

!ELSEIF  "$(CFG)" == "wolf - Win32 Debug"

# ADD CPP /W4

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\remadmin.cpp

!IF  "$(CFG)" == "wolf - Win32 Release"

!ELSEIF  "$(CFG)" == "wolf - Win32 Debug"

# ADD CPP /W4

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\scriptc.cpp

!IF  "$(CFG)" == "wolf - Win32 Release"

!ELSEIF  "$(CFG)" == "wolf - Win32 Debug"

# ADD CPP /W4

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\scripts.cpp

!IF  "$(CFG)" == "wolf - Win32 Release"

!ELSEIF  "$(CFG)" == "wolf - Win32 Debug"

# ADD CPP /W4

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\skills.cpp

!IF  "$(CFG)" == "wolf - Win32 Release"

!ELSEIF  "$(CFG)" == "wolf - Win32 Debug"

# ADD CPP /W4

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\SkiTarg.cpp

!IF  "$(CFG)" == "wolf - Win32 Release"

!ELSEIF  "$(CFG)" == "wolf - Win32 Debug"

# ADD CPP /W4

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\SndPkg.cpp

!IF  "$(CFG)" == "wolf - Win32 Release"

!ELSEIF  "$(CFG)" == "wolf - Win32 Debug"

# ADD CPP /W4

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\speech.cpp

!IF  "$(CFG)" == "wolf - Win32 Release"

!ELSEIF  "$(CFG)" == "wolf - Win32 Debug"

# ADD CPP /W4

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sregions.cpp

!IF  "$(CFG)" == "wolf - Win32 Release"

!ELSEIF  "$(CFG)" == "wolf - Win32 Debug"

# ADD CPP /W4

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\SrvParms.cpp

!IF  "$(CFG)" == "wolf - Win32 Release"

!ELSEIF  "$(CFG)" == "wolf - Win32 Debug"

# ADD CPP /W4

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\storage.cpp

!IF  "$(CFG)" == "wolf - Win32 Release"

!ELSEIF  "$(CFG)" == "wolf - Win32 Debug"

# ADD CPP /W4

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\targeting.cpp

!IF  "$(CFG)" == "wolf - Win32 Release"

!ELSEIF  "$(CFG)" == "wolf - Win32 Debug"

# ADD CPP /W4

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\telport.cpp

!IF  "$(CFG)" == "wolf - Win32 Release"

!ELSEIF  "$(CFG)" == "wolf - Win32 Debug"

# ADD CPP /W4

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\tilecache.cpp

!IF  "$(CFG)" == "wolf - Win32 Release"

!ELSEIF  "$(CFG)" == "wolf - Win32 Debug"

# ADD CPP /W4

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Timing.cpp

!IF  "$(CFG)" == "wolf - Win32 Release"

!ELSEIF  "$(CFG)" == "wolf - Win32 Debug"

# ADD CPP /W4

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\TmpEff.cpp

!IF  "$(CFG)" == "wolf - Win32 Release"

!ELSEIF  "$(CFG)" == "wolf - Win32 Debug"

# ADD CPP /W4

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Trade.cpp

!IF  "$(CFG)" == "wolf - Win32 Release"

!ELSEIF  "$(CFG)" == "wolf - Win32 Debug"

# ADD CPP /W4

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\translate.cpp

!IF  "$(CFG)" == "wolf - Win32 Release"

!ELSEIF  "$(CFG)" == "wolf - Win32 Debug"

# ADD CPP /W4

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\trigger.cpp

!IF  "$(CFG)" == "wolf - Win32 Release"

!ELSEIF  "$(CFG)" == "wolf - Win32 Debug"

# ADD CPP /W4

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\utilsys.cpp

!IF  "$(CFG)" == "wolf - Win32 Release"

!ELSEIF  "$(CFG)" == "wolf - Win32 Debug"

# ADD CPP /W4

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\walking.cpp

!IF  "$(CFG)" == "wolf - Win32 Release"

!ELSEIF  "$(CFG)" == "wolf - Win32 Debug"

# ADD CPP /W4

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\weight.cpp

!IF  "$(CFG)" == "wolf - Win32 Release"

!ELSEIF  "$(CFG)" == "wolf - Win32 Debug"

# ADD CPP /W4

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\wip.cpp

!IF  "$(CFG)" == "wolf - Win32 Release"

!ELSEIF  "$(CFG)" == "wolf - Win32 Debug"

# ADD CPP /W4

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\wolfpack.cpp

!IF  "$(CFG)" == "wolf - Win32 Release"

!ELSEIF  "$(CFG)" == "wolf - Win32 Debug"

# ADD CPP /W4

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\worldmain.cpp

!IF  "$(CFG)" == "wolf - Win32 Release"

!ELSEIF  "$(CFG)" == "wolf - Win32 Debug"

# ADD CPP /W4

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\wpconsole.cpp

!IF  "$(CFG)" == "wolf - Win32 Release"

!ELSEIF  "$(CFG)" == "wolf - Win32 Debug"

# ADD CPP /W4

!ENDIF 

# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\accounts.h
# End Source File
# Begin Source File

SOURCE=.\admin.h
# End Source File
# Begin Source File

SOURCE=.\archive.h
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

SOURCE=.\char_array.h
# End Source File
# Begin Source File

SOURCE=.\chars.h
# End Source File
# Begin Source File

SOURCE=.\CharWrap.h
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

SOURCE=.\end_pack.h
# End Source File
# Begin Source File

SOURCE=.\globals.h
# End Source File
# Begin Source File

SOURCE=.\gumps.h
# End Source File
# Begin Source File

SOURCE=.\hCache.h
# End Source File
# Begin Source File

SOURCE=.\hFile.h
# End Source File
# Begin Source File

SOURCE=.\im.h
# End Source File
# Begin Source File

SOURCE=.\inlines.h
# End Source File
# Begin Source File

SOURCE=.\item_array.h
# End Source File
# Begin Source File

SOURCE=.\itemid.h
# End Source File
# Begin Source File

SOURCE=.\items.h
# End Source File
# Begin Source File

SOURCE=.\ItemWrap.h
# End Source File
# Begin Source File

SOURCE=.\junk.h
# End Source File
# Begin Source File

SOURCE=.\mapcache.h
# End Source File
# Begin Source File

SOURCE=.\msgboard.h
# End Source File
# Begin Source File

SOURCE=.\mstring.h
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

SOURCE=.\oem.h
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

SOURCE=.\SndPkg.h
# End Source File
# Begin Source File

SOURCE=.\speech.h
# End Source File
# Begin Source File

SOURCE=.\sregions.h
# End Source File
# Begin Source File

SOURCE=.\SrvParms.h
# End Source File
# Begin Source File

SOURCE=.\start_pack.h
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

SOURCE=.\utilsys.h
# End Source File
# Begin Source File

SOURCE=.\verinfo.h
# End Source File
# Begin Source File

SOURCE=.\walking.h
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
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res.rc
# End Source File
# End Group
# End Target
# End Project
