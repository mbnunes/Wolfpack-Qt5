[_ISTool]
EnableISX=true

[Setup]
Compression=bzip
AppName=Wolfpack
AppVerName=Wolfpack 12.9.3 (Beta)
LicenseFile=LICENSE.GPL
DefaultDirName=c:\wolfpack\
AppPublisher=Wolfpack Project
AppPublisherURL=http://www.wpdev.org/
AppSupportURL=http://www.wpdev.org/
AppUpdatesURL=http://www.wpdev.org/
AppVersion=Wolfpack 12.9.3 (Beta)
AppCopyright=Copyright © 1998-2003 Wolfpack Dev Team
UninstallDisplayName=Wolfpack
DisableProgramGroupPage=true
AlwaysShowDirOnReadyPage=true
ShowTasksTreeLines=false
OutputBaseFilename=setup-wolfpack
UninstallDisplayIcon={app}\wolfpack.exe
InfoAfterFile=C:\Wazzup.pse.txt


[Files]
Source: ..\python-dll\*.pyd; DestDir: {app}\python-dll\
Source: ..\libmySQL.dll; DestDir: {app}
Source: ..\qt-mt312.dll; DestDir: {app}
Source: ..\python22.dll; DestDir: {app}
Source: ..\wolfpack.exe; DestDir: {app}
Source: ..\xmlscripts\*.*; DestDir: {app}; Flags: recursesubdirs

[Dirs]
Name: {app}\python-dll

[Run]
Filename: {app}\update-scripts.bat; WorkingDir: {app}; Description: Update with latest scripts; StatusMsg: Updating scripts; Flags: shellexec postinstall runmaximized; Tasks: Update_scripts_now

[Tasks]
Name: Update_scripts_now; Description: Download updates from scripts directly from cvs repository
[Code]
var
  InstallModePrompt: TArrayOfString;
  InstallModeValues: TArrayOfString;
  QConfig  		   : String;
  QConfig_Modules  : String;
  QMAKE_CACHE      : String;
  QT_AVALIABLE_MODULES : TArrayOfString;

///////////////////////////////////////////////////////////////////////////////
// Find given text in given file, and insert
// given textline after line with text found.
// Created: March 27, 2002 by S.T.I.Bracke
// Revised: May 22, 2002 by S.T.I.Bracke
function AddTextAfterLine(strFilename, strFind, strAddLine: String): Boolean;
var
  strTemp : String;
  iLineCounter : LongInt;
  a_strTextfile : TArrayOfString;
  fAlreadyExist : Boolean;

begin
  { Load textfile into string array }
  LoadStringsFromFile(strFilename, a_strTextfile);

  { Loop trough all textlines to check if line to add already exists }
  for iLineCounter := 1 to GetArrayLength(a_strTextfile) do
    begin
      if (a_strTextfile[iLineCounter] = strAddLine) then
       // old line fAlreadyExist := fAlreadyExist or True
      // new line suggeste by Malcolm Smith
       fAlreadyExist :=  True
    end;

  { Only execute when new line doesn't exist in textfile yet }
  if not fAlreadyExist then
    begin
      { Search trough all textlines for given text }
      for iLineCounter := 1 to GetArrayLength(a_strTextfile) do
        begin
          { Add new textline after textline with text searched for }
          if (Pos(strFind, a_strTextfile[iLineCounter]) > 0) then
            a_strTextfile[iLineCounter] := a_strTextfile[iLineCounter] + #13 + #10 + strAddLine;
        end;

      { Save string array to textfile (overwrite, no append!) }
      SaveStringsToFile(strFilename, a_strTextfile, False);
    end;

  Result := True;

end;

///////////////////////////////////////////////////////////////////////////////
// Find given text in given file, and replace
// complete textline with given text.
// Created: March 27, 2002 by S.T.I.Bracke
// Revised: May 22, 2002 by S.T.I.Bracke
function ReplaceLine(strFilename, strFind, strNewLine: String): Boolean;
var
  strTemp : String;
  iLineCounter : Integer;
  a_strTextfile : TArrayOfString;

begin
  { Load textfile into string array }
  LoadStringsFromFile(strFilename, a_strTextfile);

  { Search trough all textlines for given text }
  for iLineCounter := 1 to GetArrayLength(a_strTextfile) do
    begin
      { Overwrite textline when text searched for is part of it }
      if (Pos(strFind, a_strTextfile[iLineCounter]) > 0) then
        a_strTextfile[iLineCounter] := strNewLine;
    end;

  { Save string array to textfile (overwrite, no append!) }
  SaveStringsToFile(strFilename, a_strTextfile, False);

  Result := True;

end;

function InitializeSetup(): Boolean;
begin
  { Set prompts used on custom wizard pages }
  SetArrayLength(InstallModePrompt, 2)
  InstallModePrompt[0] := 'New Instalation';
  InstallModePrompt[1] := 'Upgrade Install';

  { Set default values }
  SetArrayLength(InstallModeValues, 2);
  InstallModeValues[0] := '1';
  InstallModeValues[1] := '0';

  { Avaliable Modules }
  SetArrayLength(QT_AVALIABLE_MODULES, 13);
  QT_AVALIABLE_MODULES[ 0] := 'styles';
  QT_AVALIABLE_MODULES[ 1] := 'tools';
  QT_AVALIABLE_MODULES[ 2] := 'kernel';
  QT_AVALIABLE_MODULES[ 3] := 'widgets';
  QT_AVALIABLE_MODULES[ 4] := 'dialogs';
  QT_AVALIABLE_MODULES[ 5] := 'iconview';
  QT_AVALIABLE_MODULES[ 6] := 'workspace';
  QT_AVALIABLE_MODULES[ 7] := 'network';
  QT_AVALIABLE_MODULES[ 8] := 'canvas';
  QT_AVALIABLE_MODULES[ 9] := 'table';
  QT_AVALIABLE_MODULES[10] := 'xml';
  QT_AVALIABLE_MODULES[11] := 'opengl';
  QT_AVALIABLE_MODULES[12] := 'sql';


  { Let Setup run }
  Result := True;
end;

procedure RegisterPreviousData(PreviousDataKey: Integer);
begin
  { Store the settings so we can restore them next time }
end;

function ScriptDlgPages(CurPage: Integer; BackClicked: Boolean): Boolean;
var
  i, j, CurSubPage: Integer;
  Next, NextOk: Boolean;
  CheckListBox: TNewCheckListBox;
  QMAKE_CACHE_LINES : TArrayOfString;
  QMODULES_LINES : TArrayOfString;
  QCONFIG_LINES  : TArrayOfString;

begin
  i := wpInstalling;
  i := wpFinished;
  if (not BackClicked and (CurPage = wpInstalling)) or (BackClicked and (CurPage = wpFinished)) then begin
    { Insert multiple custom wizard page between two non custom pages }
    { Now we must handle navigation between the custom pages ourselves }
    { First find out on which page we should start }
    if not BackClicked then
      CurSubPage := 0
    else
      CurSubPage := 2;
    { Then open the custom wizard page }
    ScriptDlgPageOpen();
	{ Set the main caption }
	ScriptDlgPageSetCaption('Wolfpack Setup Options');
    { Loop while we are still on a custom page and Setup has not been terminated }
    while (CurSubPage >= 0) and (CurSubPage <= 2) and not Terminated do begin
      case CurSubPage of
        0:
          begin
            ScriptDlgPageSetSubCaption1('Configuration');
            ScriptDlgPageSetSubCaption2('Please specify the installation type you would like to perform');
            { ask for build type }
            Next := InputOptionArray(InstallModePrompt, InstallModeValues, True, True);
            if Next then begin
              NextOk := True;
            end;
          end;
        1:
          begin
            { Then ask for modules }

            ScriptDlgPageClearCustom();
            ScriptDlgPageSetSubCaption1('Select Modules to build');
            ScriptDlgPageSetSubCaption2('Please specify the modules and options to build in');
            CheckListBox := TNewCheckListBox.Create(WizardForm.ScriptDlgPanel);
            CheckListBox.Flat := True;
            CheckListBox.Width := WizardForm.ScriptDlgPanel.Width;
            CheckListBox.Height := WizardForm.ScriptDlgPanel.Height;
            CheckListBox.Parent := WizardForm.ScriptDlgPanel;
            CheckListBox.AddCheckBox('Canvas Module', '', 0, True, True, False, nil);
            CheckListBox.AddCheckBox('Iconview Module', '', 0, True, True, False, nil);
            CheckListBox.AddCheckBox('Network Module', '', 0, True, True, False, nil);
            CheckListBox.AddCheckBox('OpenGL Module', '', 0, True, True, False, nil);
            CheckListBox.AddCheckBox('STL Support', '', 0, True, True, False, nil);
            CheckListBox.AddCheckBox('Table Module', '', 0, True, True, False, nil);
            CheckListBox.AddCheckBox('Thread Module', '', 0, True, True, False, nil);
            CheckListBox.AddCheckBox('XML Module', '', 0, True, True, False, nil);
            CheckListBox.AddCheckBox('Workspace Module', '', 0, True, True, False, nil);
            CheckListBox.AddCheckBox('ZLib Support', '', 0, True, True, False, nil);
            CheckListBox.AddCheckBox('PNG Support', '', 0, True, True, False, nil);
            CheckListBox.AddCheckBox('MNG Support', '', 0, True, True, False, nil);
//            CheckListBox.AddRadioButton('Built-in', '', 1, 0, True, True, nil);
//            CheckListBox.AddRadioButton('Plugin', '', 1, 0, False, True, nil);
            CheckListBox.AddCheckBox('Big Codecs', '', 0, True, True, False, nil);
//            CheckListBox.AddRadioButton('Built-in', '', 1, 0, True, True, nil);
//            CheckListBox.AddRadioButton('Plugin', '', 1, 0, False, True, nil);
            CheckListBox.AddCheckBox('JPEG Support', '', 0, True, True, False, nil);
//            CheckListBox.AddRadioButton('Built-in', '', 1, 0, True, True, nil);
//            CheckListBox.AddRadioButton('Plugin', '', 1, 0, False, True, nil);
            CheckListBox.AddCheckBox('GIF Support', '', 0, True, True, False, nil);
//            CheckListBox.AddRadioButton('Built-in', '', 1, 0, True, True, nil);
//            CheckListBox.AddRadioButton('Plugin', '', 1, 0, False, True, nil);
            Next := ScriptDlgPageProcessCustom();

			if Next then begin
			end;

			{ Save Configuration to .qmake.cache }
			QMAKE_CACHE := AddBackSlash(ExpandConstant('{app}')) + '.qmake.cache';
			DeleteFile(QMAKE_CACHE);
			SetArrayLength(QMAKE_CACHE_LINES, 30);
			i := 0;
			QMAKE_CACHE_LINES[i] := 'OBJECTS_DIR=tmp\obj';                                                      i := i + 1;
			QMAKE_CACHE_LINES[i] := 'MOC_DIR=tmp\moc';                                                          i := i + 1;
			QMAKE_CACHE_LINES[i] := 'DEFINES += ';                                                              i := i + 1;
			QMAKE_CACHE_LINES[i] := 'INCLUDEPATH += ';                                                          i := i + 1;
			QMAKE_CACHE_LINES[i] := 'sql-drivers +=';                                                           i := i + 1;
			QMAKE_CACHE_LINES[i] := 'sql-plugins +=';                                                           i := i + 1;
			QMAKE_CACHE_LINES[i] := 'styles += windows';                                                        i := i + 1;
			QMAKE_CACHE_LINES[i] := 'style-plugins += motif motifplus platinum sgi cde';                        i := i + 1;
			QMAKE_CACHE_LINES[i] := 'imageformat-plugins += jpeg mng';                                          i := i + 1;
			QMAKE_CACHE_LINES[i] := 'QT_PRODUCT=qt-enterprise';                                                 i := i + 1;
			SaveStringsToFile(QMAKE_CACHE, QMAKE_CACHE_LINES, True);

			NextOk := True;
          end;
        2:
          begin
          end;
      end;
      if Next then begin
        { Go to the next page, but only if the user entered correct information }
        if NextOk then
          CurSubPage := CurSubPage + 1;
      end else
        CurSubPage := CurSubPage - 1;
    end;
    { See NextButtonClick and BackButtonClick: return True if the click should be allowed }
    if not BackClicked then
      Result := Next
    else
      Result := not Next;
    { Close the wizard page. Do a FullRestore only if the click (see above) is not allowed }
    ScriptDlgPageClose(not Result);
  end else begin
    Result := True;
  end;
end;

function NextButtonClick(CurPage: Integer): Boolean;
begin
  Result := ScriptDlgPages(CurPage, False);
end;

function BackButtonClick(CurPage: Integer): Boolean;
begin
  Result := ScriptDlgPages(CurPage, True);
end;

[InstallDelete]
Name: {app}\qt-mt311.dll; Type: files
