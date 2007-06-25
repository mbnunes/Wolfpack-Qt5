program WPGM;

uses
  Forms,
  Configuration in '..\src\Configuration.pas' {FrmConfig},
  Main in '..\src\Main.pas' {frmMain},
  Overview in '..\src\Overview.pas' {frmOverview},
  Progress in '..\src\Progress.pas' {frmProgress},
  Spawnregions in '..\src\Spawnregions.pas' {frmRegions},
  SQLite3 in '..\src\SQLite3.pas',
  SQLiteTable3 in '..\src\SQLiteTable3.pas',
  Strlib in '..\src\Strlib.pas',
  uCenter in '..\src\uCenter.pas' {frmCenter},
  uChooseHue in '..\src\uChooseHue.pas' {frmChooseHue},
  uConfig in '..\src\uConfig.pas',
  UOCache in '..\src\UOCache.pas',
  UOHues in '..\src\UOHues.pas',
  UOMap in '..\src\UOMap.pas',
  UORadarCol in '..\src\UORadarCol.pas',
  UOStatics in '..\src\UOStatics.pas',
  uPatternColor in '..\src\uPatternColor.pas' {frmPatternColor},
  XMLReader in '..\src\XMLReader.pas';

{$R *.RES}

begin
  Application.Initialize;
  Application.Title := 'Wolfpack - GM Tool';
  Application.CreateForm(TfrmMain, frmMain);
  Application.CreateForm(TfrmOverview, frmOverview);
  Application.CreateForm(TfrmRegions, frmRegions);
  Application.CreateForm(TfrmCenter, frmCenter);
  Application.CreateForm(TfrmChooseHue, frmChooseHue);
  Application.CreateForm(TfrmPatternColor, frmPatternColor);
  Application.Run;
end.
