program ImportDefinitions;

uses
  Forms,
  Main in 'Main.pas' {MainForm},
  WpConfiguration in 'WpConfiguration.pas';

{$R *.res}

begin
  Application.Initialize;
  Application.CreateForm(TMainForm, MainForm);
  Application.Run;
end.
