[Setup]
InternalCompressLevel=ultra
OutputBaseFilename=uoclient-setup
SolidCompression=true
Compression=lzma/ultra
AppCopyright=© 2005 Sebastian Hartte
AppName=Wolfpack Client
LicenseFile=LICENSE
AppMutex=UOCLIENT_MUTEX
RestartIfNeededByRun=false
DefaultDirName={pf}\Wolfpack Client
AllowRootDirectory=true
DirExistsWarning=yes
AllowNoIcons=true
DefaultGroupName=Wolfpack Client
ShowLanguageDialog=yes
LanguageDetectionMethod=locale
AppPublisher=Wolfpack Team
AppPublisherURL=http://www.wpdev.org
AppSupportURL=http://www.wpdev.org/phpBB2/
AppID=WolfpackClient
UninstallDisplayName=Wolfpack Client
AppVerName=Wolfpack Client Beta
OutputDir=install
SourceDir=..\
WizardImageFile=C:\uoclient\install\logo.bmp
[Files]
Source: LICENSE; DestDir: {app}; Flags: overwritereadonly ignoreversion replacesameversion
Source: release\update.exe; DestDir: {app}; Flags: overwritereadonly ignoreversion replacesameversion
Source: release\uoclient.exe; DestDir: {app}; Flags: overwritereadonly ignoreversion replacesameversion
Source: release\python24.dll; DestDir: {app}; Flags: overwritereadonly ignoreversion replacesameversion
Source: release\openal32.dll; DestDir: {app}; Flags: overwritereadonly ignoreversion replacesameversion
Source: release\dialogs\*.py; DestDir: {app}\dialogs; Flags: recursesubdirs overwritereadonly ignoreversion replacesameversion
Source: release\dialogs\*.xml; DestDir: {app}\dialogs; Flags: recursesubdirs overwritereadonly ignoreversion replacesameversion
Source: release\scripts\*.pyo; DestDir: {app}\scripts; Flags: recursesubdirs overwritereadonly ignoreversion replacesameversion
Source: release\QtCore4.dll; DestDir: {app}; Flags: overwritereadonly ignoreversion replacesameversion
Source: release\QtOpenGL4.dll; DestDir: {app}; Flags: overwritereadonly ignoreversion replacesameversion
Source: release\QtNetwork4.dll; DestDir: {app}; Flags: overwritereadonly ignoreversion replacesameversion
Source: release\QtXml4.dll; DestDir: {app}; Flags: overwritereadonly ignoreversion replacesameversion
Source: release\QtGui4.dll; DestDir: {app}; Flags: overwritereadonly ignoreversion replacesameversion
Source: release\msvcp71.dll; DestDir: {app}; Flags: overwritereadonly ignoreversion replacesameversion
Source: release\msvcr71.dll; DestDir: {app}; Flags: overwritereadonly ignoreversion replacesameversion
Source: release\7zip.dll; DestDir: {app}; Flags: overwritereadonly ignoreversion replacesameversion
[Dirs]
Name: {app}\downloads
Name: {app}\logs
Name: {app}\profiles
Name: {app}\screenshots
Name: {app}\dialogs
Name: {app}\scripts
[Icons]
Name: {group}\Wolfpack Client; Filename: {app}\uoclient.exe; WorkingDir: {app}; IconFilename: {app}\uoclient.exe; Comment: Start the Wolfpack Client; IconIndex: 0
Name: {group}\Uninstall; Filename: {uninstallexe}; WorkingDir: {app}; IconFilename: {uninstallexe}
Name: {group}\Client Forums; Filename: http://www.wpdev.org/phpBB2/viewforum.php?f=18
Name: {userappdata}\Microsoft\Internet Explorer\Quick Launch\Wolfpack Client; WorkingDir: {app}; Filename: {app}\uoclient.exe; Tasks: quicklaunchicon

[UninstallDelete]
Name: {app}\logs\*.log; Type: filesandordirs; Tasks: 
Name: {app}\config.xml; Type: files
Name: {app}\dialogs\*.pyo; Type: filesandordirs
Name: {app}\downloads\*.*; Type: filesandordirs
Name: {app}\downloads\; Type: dirifempty
Name: {app}\logs\; Type: dirifempty
[Tasks]
Name: quicklaunchicon; Description: Create a &Quick Launch icon; GroupDescription: Icons:; Flags: checkedonce
[Run]
Filename: {app}\uoclient.exe; WorkingDir: {app}; Description: Run Wolfpack Client; Flags: nowait skipifdoesntexist postinstall
