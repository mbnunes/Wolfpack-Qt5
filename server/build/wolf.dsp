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
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /GR /GX /O2 /Ob0 /I "lib/Python/PC" /I "lib/Python/include" /I "lib\ZThread\include" /I "$(QTDIR)\include" /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "ZTHREAD_SHARED" /D "QT_DLL" /D "QT_THREAD_SUPPORT" /Fr /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib advapi32.lib ws2_32.lib $(QTDIR)\lib\qt-mt312.lib libmysql.lib /nologo /subsystem:console /map /machine:I386 /nodefaultlib:"libcmt MSVCRTD" /out:"D:\wolfpack\wolfpack.exe" /libpath:"lib\ZThread\lib" /libpath:"lib\Python\lib" /libpath:"lib\bugreport\lib" /libpath:"flatstore\Release" /opt:ref /opt:nowin98
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "wolf - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "c:\wolfpack"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MD /Gm /GR /GX /ZI /Od /I "lib\bugreport" /I "$(QTDIR)\include" /D "_CONSOLE" /D "_MBCS" /D "_DEBUG" /D "WIN32" /D "QT_DLL" /D "QT_NO_STL" /D "QT_THREAD_SUPPORT" /FR /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib advapi32.lib ws2_32.lib $(QTDIR)\lib\qt-mt312.lib libmysql.lib /nologo /version:12.9 /subsystem:console /map /debug /machine:I386 /out:"..\Wolfpack.exe" /pdbtype:sept /libpath:"lib\bugreport\lib" /libpath:"flatstore\Debug"
# SUBTRACT LINK32 /pdb:none

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

SOURCE=.\ai.cpp
# End Source File
# Begin Source File

SOURCE=.\ai_animals.cpp
# End Source File
# Begin Source File

SOURCE=.\ai_monsters.cpp
# End Source File
# Begin Source File

SOURCE=.\ai_vendors.cpp
# End Source File
# Begin Source File

SOURCE=.\basechar.cpp
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

SOURCE=.\chars.cpp
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

SOURCE=.\encryption.cpp
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

SOURCE=.\iserialization.cpp
# End Source File
# Begin Source File

SOURCE=.\itemid.cpp
# End Source File
# Begin Source File

SOURCE=.\items.cpp
# End Source File
# Begin Source File

SOURCE=.\LoS.cpp
# End Source File
# Begin Source File

SOURCE=.\makemenus.cpp
# End Source File
# Begin Source File

SOURCE=.\maps.cpp
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

SOURCE=.\npc.cpp
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

SOURCE=.\player.cpp
# End Source File
# Begin Source File

SOURCE=.\preferences.cpp
# End Source File
# Begin Source File

SOURCE=.\pythonscript.cpp
# End Source File
# Begin Source File

SOURCE=.\resources.cpp
# End Source File
# Begin Source File

SOURCE=.\scriptmanager.cpp
# End Source File
# Begin Source File

SOURCE=.\sectors.cpp
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

SOURCE=.\twofish\TWOFISH2.C
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

SOURCE=.\wolfpack.cpp
# End Source File
# Begin Source File

SOURCE=.\world.cpp
# End Source File
# Begin Source File

SOURCE=.\wpconsole.cpp
# End Source File
# Begin Source File

SOURCE=.\wpdefmanager.cpp
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

SOURCE=.\ai.h
# End Source File
# Begin Source File

SOURCE=.\basechar.h
# PROP Ignore_Default_Tool 1
# End Source File
# Begin Source File

SOURCE=.\baseregion.h
# End Source File
# Begin Source File

SOURCE=.\basics.h
# End Source File
# Begin Source File

SOURCE=.\boats.h
# PROP Ignore_Default_Tool 1
# End Source File
# Begin Source File

SOURCE=.\books.h
# PROP Ignore_Default_Tool 1
# End Source File
# Begin Source File

SOURCE=.\chars.h
# PROP Ignore_Default_Tool 1
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
# PROP Ignore_Default_Tool 1
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

SOURCE=.\encryption.h
# End Source File
# Begin Source File

SOURCE=.\exceptions.h
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

!IF  "$(CFG)" == "wolf - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - MOCing house.h...
InputDir=.
InputPath=.\house.h
InputName=house

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "wolf - Win32 Debug"

# PROP Ignore_Default_Tool 1

!ENDIF 

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
# PROP Ignore_Default_Tool 1
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

SOURCE=.\maps.h
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

SOURCE=.\npc.h
# PROP Ignore_Default_Tool 1
# End Source File
# Begin Source File

SOURCE=.\python\objectcache.h
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
# PROP Ignore_Default_Tool 1
# End Source File
# Begin Source File

SOURCE=.\pfactory.h
# End Source File
# Begin Source File

SOURCE=.\platform.h
# End Source File
# Begin Source File

SOURCE=.\player.h
# PROP Ignore_Default_Tool 1
# End Source File
# Begin Source File

SOURCE=.\preferences.h
# End Source File
# Begin Source File

SOURCE=.\prototypes.h
# End Source File
# Begin Source File

SOURCE=.\pythonscript.h
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

SOURCE=.\scriptmanager.h
# End Source File
# Begin Source File

SOURCE=.\sectors.h
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

SOURCE=.\twofish\twofish.h
# End Source File
# Begin Source File

SOURCE=.\typedefs.h
# End Source File
# Begin Source File

SOURCE=.\uobject.h
# PROP Ignore_Default_Tool 1
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

SOURCE=.\wolfpack.h
# End Source File
# Begin Source File

SOURCE=.\world.h
# End Source File
# Begin Source File

SOURCE=.\wpconsole.h
# End Source File
# Begin Source File

SOURCE=.\wpdefmanager.h
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

SOURCE=.\python\multi.cpp
# End Source File
# Begin Source File

SOURCE=.\python\pyaccount.cpp
# End Source File
# Begin Source File

SOURCE=.\python\pycoord.cpp
# End Source File
# Begin Source File

SOURCE=.\python\pypacket.cpp
# End Source File
# Begin Source File

SOURCE=.\python\pypacket.h
# End Source File
# Begin Source File

SOURCE=.\python\pyregion.cpp
# End Source File
# Begin Source File

SOURCE=.\python\pytooltip.cpp
# End Source File
# Begin Source File

SOURCE=.\python\regioniterator.h
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
# End Group
# Begin Group "Generated"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\moc_commands.cpp

!IF  "$(CFG)" == "wolf - Win32 Release"

!ELSEIF  "$(CFG)" == "wolf - Win32 Debug"

# ADD CPP /GR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\moc_gumps.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_iserialization.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_makemenus.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_multis.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_resources.cpp
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

SOURCE=.\network\moc_uosocket.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_wptargetrequests.cpp
# End Source File
# End Group
# Begin Group "SQLite"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\sqlite\attach.c
# End Source File
# Begin Source File

SOURCE=.\sqlite\auth.c
# End Source File
# Begin Source File

SOURCE=.\sqlite\btree.c
# End Source File
# Begin Source File

SOURCE=.\sqlite\btree.h
# End Source File
# Begin Source File

SOURCE=.\sqlite\btree_rb.c
# End Source File
# Begin Source File

SOURCE=.\sqlite\build.c
# End Source File
# Begin Source File

SOURCE=.\sqlite\config.h
# End Source File
# Begin Source File

SOURCE=.\sqlite\copy.c
# End Source File
# Begin Source File

SOURCE=.\sqlite\delete.c
# End Source File
# Begin Source File

SOURCE=.\sqlite\expr.c
# End Source File
# Begin Source File

SOURCE=.\sqlite\func.c
# End Source File
# Begin Source File

SOURCE=.\sqlite\hash.c
# End Source File
# Begin Source File

SOURCE=.\sqlite\hash.h
# End Source File
# Begin Source File

SOURCE=.\sqlite\insert.c
# End Source File
# Begin Source File

SOURCE=.\sqlite\main.c
# End Source File
# Begin Source File

SOURCE=.\sqlite\opcodes.c
# End Source File
# Begin Source File

SOURCE=.\sqlite\opcodes.h
# End Source File
# Begin Source File

SOURCE=.\sqlite\os.c
# End Source File
# Begin Source File

SOURCE=.\sqlite\os.h
# End Source File
# Begin Source File

SOURCE=.\sqlite\pager.c
# End Source File
# Begin Source File

SOURCE=.\sqlite\pager.h
# End Source File
# Begin Source File

SOURCE=.\sqlite\parse.c
# End Source File
# Begin Source File

SOURCE=.\sqlite\parse.h
# End Source File
# Begin Source File

SOURCE=.\sqlite\pragma.c
# End Source File
# Begin Source File

SOURCE=.\sqlite\printf.c
# End Source File
# Begin Source File

SOURCE=.\sqlite\random.c
# End Source File
# Begin Source File

SOURCE=.\sqlite\select.c
# End Source File
# Begin Source File

SOURCE=.\sqlite\sqlite.h
# End Source File
# Begin Source File

SOURCE=.\sqlite\sqliteInt.h
# End Source File
# Begin Source File

SOURCE=.\sqlite\table.c
# End Source File
# Begin Source File

SOURCE=.\sqlite\tokenize.c
# End Source File
# Begin Source File

SOURCE=.\sqlite\trigger.c
# End Source File
# Begin Source File

SOURCE=.\sqlite\update.c
# End Source File
# Begin Source File

SOURCE=.\sqlite\util.c
# End Source File
# Begin Source File

SOURCE=.\sqlite\vacuum.c
# End Source File
# Begin Source File

SOURCE=.\sqlite\vdbe.c
# End Source File
# Begin Source File

SOURCE=.\sqlite\vdbe.h
# End Source File
# Begin Source File

SOURCE=.\sqlite\where.c
# End Source File
# End Group
# End Target
# End Project
