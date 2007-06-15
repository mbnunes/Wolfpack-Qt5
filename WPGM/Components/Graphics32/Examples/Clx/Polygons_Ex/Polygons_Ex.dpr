program Polygons_Ex;

{$I GR32.inc}

{%File '..\..\GR32.inc'}

uses
  {$IFDEF CLX}
  QForms,
  {$ELSE}
  Forms,
  {$ENDIF}  
  MainUnit in 'MainUnit.pas' {Form1};

{$R *.res}

begin
  Application.Initialize;
  Application.CreateForm(TForm1, Form1);
  Application.Run;
end.
