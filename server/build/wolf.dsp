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
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /GR /GX /O2 /Oy- /Ob0 /I "lib/Python/PC" /I "lib/Python/include" /I "lib\ZThread\include" /I "$(QTDIR)\include" /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "ZTHREAD_SHARED" /D "QT_DLL" /D "WP_DONT_USE_HASH_MAP" /Fr /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 ws2_32.lib ZThread.lib $(QTDIR)\lib\qt-mt310.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib libmysql.lib /nologo /subsystem:console /incremental:yes /map /machine:I386 /out:"C:\wolfpack\wolfpack.exe" /libpath:"lib\ZThread\lib" /libpath:"lib\Python\lib"
# SUBTRACT LINK32 /nodefaultlib

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
# ADD CPP /nologo /MDd /Gm /GR /GX /ZI /Od /I "lib/Python/PC" /I "lib/Python/include" /I "lib\bugreport" /I "lib\ZThread\include" /I "$(QTDIR)\include" /I "c:\mysql\include" /D "_CONSOLE" /D "_MBCS" /D "ZTHREAD_STATIC" /D "PY_NOSOCKETS" /D "_DEBUG" /D "WIN32" /D "QT_DLL" /D "WP_DONT_USE_HASH_MAP" /D "QT_NO_STL" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 ws2_32.lib ZThread.lib $(QTDIR)\lib\qt-mt310.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib libmysql.lib pdh.lib /nologo /version:12.9 /subsystem:console /map /debug /machine:I386 /nodefaultlib:"libcmt" /nodefaultlib:"libcmtd" /out:"C:\Wolfpack\Wolfpack.exe" /pdbtype:sept /libpath:"lib\ZThread\lib" /libpath:"lib\Python\lib" /libpath:"lib\bugreport\lib"
# SUBTRACT LINK32 /pdb:none /incremental:no

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

SOURCE=.\chars.cpp
# End Source File
# Begin Source File

SOURCE=.\charsmgr.cpp
# End Source File
# Begin Source File

SOURCE=.\combat.cpp
# End Source File
# Begin Source File

SOURCE=.\commands.cpp
# End Source File
# Begin Source File

SOURCE=.\contextmenu.cpp
# End Source File
# Begin Source File

SOURCE=.\coord.cpp
# End Source File
# Begin Source File

SOURCE=.\corpse.cpp
# End Source File
# Begin Source File

SOURCE=.\customtags.cpp
# End Source File
# Begin Source File

SOURCE=.\dbdriver.cpp
# End Source File
# Begin Source File

SOURCE=.\dbl_single_click.cpp
# End Source File
# Begin Source File

SOURCE=.\debug.cpp
# End Source File
# Begin Source File

SOURCE=.\definable.cpp
# End Source File
# Begin Source File

SOURCE=.\dragdrop.cpp
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

SOURCE=.\makemenus.cpp
# End Source File
# Begin Source File

SOURCE=.\mapobjects.cpp
# End Source File
# Begin Source File

SOURCE=.\maps.cpp
# End Source File
# Begin Source File

SOURCE=.\msgboard.cpp
# End Source File
# Begin Source File

SOURCE=.\multis.cpp
# End Source File
# Begin Source File

SOURCE=.\multiscache.cpp
# End Source File
# Begin Source File

SOURCE=.\network.cpp
# End Source File
# Begin Source File

SOURCE=.\newmagic.cpp
# End Source File
# Begin Source File

SOURCE=.\npcs.cpp
# End Source File
# Begin Source File

SOURCE=.\onlinestatus.cpp
# End Source File
# Begin Source File

SOURCE=.\p_ai.cpp
# End Source File
# Begin Source File

SOURCE=.\persistentbroker.cpp
# End Source File
# Begin Source File

SOURCE=.\persistentobject.cpp
# End Source File
# Begin Source File

SOURCE=.\pfactory.cpp
# End Source File
# Begin Source File

SOURCE=.\preferences.cpp
# End Source File
# Begin Source File

SOURCE=.\remadmin.cpp
# End Source File
# Begin Source File

SOURCE=.\resources.cpp
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

SOURCE=.\spawnregions.cpp
# End Source File
# Begin Source File

SOURCE=.\speech.cpp
# End Source File
# Begin Source File

SOURCE=.\spellbook.cpp
# End Source File
# Begin Source File

SOURCE=.\srvparams.cpp
# End Source File
# Begin Source File

SOURCE=.\srvparams_win.cpp
# End Source File
# Begin Source File

SOURCE=.\storage.cpp
# End Source File
# Begin Source File

SOURCE=.\targetactions.cpp
# End Source File
# Begin Source File

SOURCE=.\targeting.cpp
# End Source File
# Begin Source File

SOURCE=.\targetrequests.cpp
# End Source File
# Begin Source File

SOURCE=.\territories.cpp
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

SOURCE=.\tracking.cpp
# End Source File
# Begin Source File

SOURCE=.\Trade.cpp
# End Source File
# Begin Source File

SOURCE=.\uobject.cpp
# End Source File
# Begin Source File

SOURCE=.\utilsys.cpp
# End Source File
# Begin Source File

SOURCE=.\walking.cpp
# End Source File
# Begin Source File

SOURCE=.\weight.cpp
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

SOURCE=.\wpdefaultscript.cpp
# End Source File
# Begin Source File

SOURCE=.\wpdefmanager.cpp
# End Source File
# Begin Source File

SOURCE=.\wpscriptmanager.cpp
# End Source File
# Begin Source File

SOURCE=.\wptargetrequests.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Group "Skills No. 1"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\tracking.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\accounts.h
# End Source File
# Begin Source File

SOURCE=.\baseregion.h
# End Source File
# Begin Source File

SOURCE=.\basics.h
# End Source File
# Begin Source File

SOURCE=.\boats.h

!IF  "$(CFG)" == "wolf - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - MOCing boats.h...
InputDir=.
InputPath=.\boats.h
InputName=boats

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "wolf - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - MOCing boats.h...
InputDir=.
InputPath=.\boats.h
InputName=boats

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\books.h
# End Source File
# Begin Source File

SOURCE=.\bounty.h
# End Source File
# Begin Source File

SOURCE=.\chars.h

!IF  "$(CFG)" == "wolf - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - MOCing chars.h...
InputDir=.
InputPath=.\chars.h
InputName=chars

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "wolf - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - MOCing chars.h...
InputDir=.
InputPath=.\chars.h
InputName=chars

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\charsmgr.h
# End Source File
# Begin Source File

SOURCE=.\classes.h
# End Source File
# Begin Source File

SOURCE=.\combat.h
# End Source File
# Begin Source File

SOURCE=.\commands.h

!IF  "$(CFG)" == "wolf - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - MOCing commands.h...
InputDir=.
InputPath=.\commands.h
InputName=commands

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "wolf - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - MOCing commands.h...
InputDir=.
InputPath=.\commands.h
InputName=commands

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\contextmenu.h
# End Source File
# Begin Source File

SOURCE=.\coord.h
# End Source File
# Begin Source File

SOURCE=.\corpse.h
# End Source File
# Begin Source File

SOURCE=.\customtags.h
# End Source File
# Begin Source File

SOURCE=.\dbdriver.h
# End Source File
# Begin Source File

SOURCE=.\dbl_single_click.h
# End Source File
# Begin Source File

SOURCE=.\debug.h
# End Source File
# Begin Source File

SOURCE=.\definable.h
# PROP Ignore_Default_Tool 1
# End Source File
# Begin Source File

SOURCE=.\defines.h
# End Source File
# Begin Source File

SOURCE=.\dragdrop.h
# End Source File
# Begin Source File

SOURCE=.\factory.h
# End Source File
# Begin Source File

SOURCE=.\globals.h
# End Source File
# Begin Source File

SOURCE=.\guildstones.h
# End Source File
# Begin Source File

SOURCE=.\gumps.h

!IF  "$(CFG)" == "wolf - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - MOCing gumps.h...
InputDir=.
InputPath=.\gumps.h
InputName=gumps

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "wolf - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - MOCing gumps.h...
InputDir=.
InputPath=.\gumps.h
InputName=gumps

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\house.h
# End Source File
# Begin Source File

SOURCE=.\inlines.h
# End Source File
# Begin Source File

SOURCE=.\iserialization.h

!IF  "$(CFG)" == "wolf - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - MOCing iserialization.h...
InputDir=.
InputPath=.\iserialization.h
InputName=iserialization

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "wolf - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - MOCing iserialization.h...
InputDir=.
InputPath=.\iserialization.h
InputName=iserialization

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\itemid.h
# End Source File
# Begin Source File

SOURCE=.\items.h

!IF  "$(CFG)" == "wolf - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - MOCing items.h...
InputDir=.
InputPath=.\items.h
InputName=items

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "wolf - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - MOCing items.h...
InputDir=.
InputPath=.\items.h
InputName=items

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\itemsmgr.h
# End Source File
# Begin Source File

SOURCE=.\magic.h
# End Source File
# Begin Source File

SOURCE=.\makemenus.h

!IF  "$(CFG)" == "wolf - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - MOCing makemenus.h...
InputDir=.
InputPath=.\makemenus.h
InputName=makemenus

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "wolf - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - MOCing makemenus.h...
InputDir=.
InputPath=.\makemenus.h
InputName=makemenus

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\mapobjects.h
# End Source File
# Begin Source File

SOURCE=.\maps.h
# End Source File
# Begin Source File

SOURCE=.\msgboard.h
# End Source File
# Begin Source File

SOURCE=.\mulstructs.h
# End Source File
# Begin Source File

SOURCE=.\multis.h

!IF  "$(CFG)" == "wolf - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - MOCing multis.h...
InputDir=.
InputPath=.\multis.h
InputName=multis

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "wolf - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - MOCing multis.h...
InputDir=.
InputPath=.\multis.h
InputName=multis

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\multiscache.h
# End Source File
# Begin Source File

SOURCE=.\network.h
# End Source File
# Begin Source File

SOURCE=.\newmagic.h
# End Source File
# Begin Source File

SOURCE=.\onlinestatus.h
# End Source File
# Begin Source File

SOURCE=.\pagesystem.h
# End Source File
# Begin Source File

SOURCE=.\persistentbroker.h

!IF  "$(CFG)" == "wolf - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - MOCing persistentbroker.h...
InputDir=.
InputPath=.\persistentbroker.h
InputName=persistentbroker

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "wolf - Win32 Debug"

# PROP Ignore_Default_Tool 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\persistentobject.h

!IF  "$(CFG)" == "wolf - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - MOCing persistentobject.h...
InputDir=.
InputPath=.\persistentobject.h
InputName=persistentobject

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "wolf - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - MOCing persistentobject.h...
InputDir=.
InputPath=.\persistentobject.h
InputName=persistentobject

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\pfactory.h
# End Source File
# Begin Source File

SOURCE=.\platform.h
# End Source File
# Begin Source File

SOURCE=.\preferences.h
# End Source File
# Begin Source File

SOURCE=.\prototypes.h
# End Source File
# Begin Source File

SOURCE=.\remadmin.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\resources.h

!IF  "$(CFG)" == "wolf - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - MOCing resources.h...
InputDir=.
InputPath=.\resources.h
InputName=resources

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "wolf - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - MOCing resources.h...
InputDir=.
InputPath=.\resources.h
InputName=resources

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\serbinfile.h
# End Source File
# Begin Source File

SOURCE=.\serxmlfile.h
# End Source File
# Begin Source File

SOURCE=.\singleton.h
# End Source File
# Begin Source File

SOURCE=.\skills.h
# End Source File
# Begin Source File

SOURCE=.\spawnregions.h
# End Source File
# Begin Source File

SOURCE=.\speech.h
# End Source File
# Begin Source File

SOURCE=.\spellbook.h

!IF  "$(CFG)" == "wolf - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - MOCing spellbook.h...
InputDir=.
InputPath=.\spellbook.h
InputName=spellbook

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "wolf - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - MOCing spellbook.h...
InputDir=.
InputPath=.\spellbook.h
InputName=spellbook

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\srvparams.h
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

SOURCE=.\targetactions.h
# End Source File
# Begin Source File

SOURCE=.\targeting.h
# End Source File
# Begin Source File

SOURCE=.\targetrequests.h

!IF  "$(CFG)" == "wolf - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - MOCing targetrequests.h...
InputDir=.
InputPath=.\targetrequests.h
InputName=targetrequests

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "wolf - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - MOCing targetrequests.h...
InputDir=.
InputPath=.\targetrequests.h
InputName=targetrequests

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\territories.h

!IF  "$(CFG)" == "wolf - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - MOCing territories.h...
InputDir=.
InputPath=.\territories.h
InputName=territories

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "wolf - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - MOCing territories.h...
InputDir=.
InputPath=.\territories.h
InputName=territories

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

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

!IF  "$(CFG)" == "wolf - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - MOCing TmpEff.h...
InputDir=.
InputPath=.\TmpEff.h
InputName=TmpEff

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "wolf - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - MOCing TmpEff.h...
InputDir=.
InputPath=.\TmpEff.h
InputName=TmpEff

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Trade.h
# End Source File
# Begin Source File

SOURCE=.\typedefs.h
# End Source File
# Begin Source File

SOURCE=.\uobject.h

!IF  "$(CFG)" == "wolf - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - MOCing uobject.h...
InputDir=.
InputPath=.\uobject.h
InputName=uobject

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "wolf - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - MOCing uobject.h...
InputDir=.
InputPath=.\uobject.h
InputName=uobject

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

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

SOURCE=.\wpscriptmanager.h
# End Source File
# Begin Source File

SOURCE=.\wptargetrequests.h

!IF  "$(CFG)" == "wolf - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - MOCing wptargetrequests.h...
InputDir=.
InputPath=.\wptargetrequests.h
InputName=wptargetrequests

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "wolf - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - MOCing wptargetrequests.h...
InputDir=.
InputPath=.\wptargetrequests.h
InputName=wptargetrequests

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

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
# Begin Group "Network"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\network\asyncnetio.cpp
# End Source File
# Begin Source File

SOURCE=.\network\asyncnetio.h
# End Source File
# Begin Source File

SOURCE=.\network\listener.cpp
# End Source File
# Begin Source File

SOURCE=.\network\listener.h
# End Source File
# Begin Source File

SOURCE=.\network\uopacket.cpp
# End Source File
# Begin Source File

SOURCE=.\network\uopacket.h
# End Source File
# Begin Source File

SOURCE=.\network\uorxpackets.cpp
# End Source File
# Begin Source File

SOURCE=.\network\uorxpackets.h
# End Source File
# Begin Source File

SOURCE=.\network\uosocket.cpp
# End Source File
# Begin Source File

SOURCE=.\network\uosocket.h

!IF  "$(CFG)" == "wolf - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - MOCing uosocket.h...
InputDir=.\network
InputPath=.\network\uosocket.h
InputName=uosocket

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "wolf - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - MOCing uosocket.h...
InputDir=.\network
InputPath=.\network\uosocket.h
InputName=uosocket

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\network\uotxpackets.cpp
# End Source File
# Begin Source File

SOURCE=.\network\uotxpackets.h
# End Source File
# End Group
# Begin Group "Python"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\python\char.cpp
# End Source File
# Begin Source File

SOURCE=.\python\content.h
# End Source File
# Begin Source File

SOURCE=.\python\engine.cpp
# End Source File
# Begin Source File

SOURCE=.\python\engine.h
# End Source File
# Begin Source File

SOURCE=.\python\global.cpp
# End Source File
# Begin Source File

SOURCE=.\python\gump.h
# End Source File
# Begin Source File

SOURCE=.\python\item.cpp
# End Source File
# Begin Source File

SOURCE=.\python\pyaccount.cpp
# End Source File
# Begin Source File

SOURCE=.\python\pycoord.cpp
# End Source File
# Begin Source File

SOURCE=.\python\pyregion.cpp
# End Source File
# Begin Source File

SOURCE=.\python\skills.h
# End Source File
# Begin Source File

SOURCE=.\python\socket.cpp
# End Source File
# Begin Source File

SOURCE=.\python\target.h
# End Source File
# Begin Source File

SOURCE=.\python\tempeffect.h
# End Source File
# Begin Source File

SOURCE=.\python\utilities.h
# End Source File
# Begin Source File

SOURCE=.\python\wppythonscript.cpp
# End Source File
# Begin Source File

SOURCE=.\python\wppythonscript.h
# End Source File
# End Group
# Begin Group "Generated"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\moc_boats.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_chars.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_commands.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_gumps.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_iserialization.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_items.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_makemenus.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_multis.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_persistentobject.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_resources.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_spellbook.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_targetrequests.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_territories.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_TmpEff.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_uobject.cpp
# End Source File
# Begin Source File

SOURCE=.\network\moc_uosocket.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_wptargetrequests.cpp
# End Source File
# End Group
# End Target
# End Project
