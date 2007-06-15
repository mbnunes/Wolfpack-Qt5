program WPGM;

uses
  Forms,
  Main in 'Main.pas' {frmMain},
  UOTiledata in 'UOLib\UOTiledata.pas',
  UOUtilities in 'UOLib\UOUtilities.pas',
  UOArt in 'UOLib\UOArt.pas',
  uConfig in 'uConfig.pas',
  UOHues in 'UOHues.pas',
  uChooseHue in 'uChooseHue.pas' {frmChooseHue},
  UOAnim in 'UOLib\UOAnim.pas',
  uCenter in 'uCenter.pas' {frmCenter},
  Overview in 'Overview.pas' {frmOverview},
  Progress in 'Progress.pas' {frmProgress},
  Spawnregions in 'Spawnregions.pas' {frmRegions},
  uPatternColor in 'uPatternColor.pas' {frmPatternColor};

{$R *.RES}

begin
  Application.Initialize;
  Application.Title := 'Wolfpack - GM Tool';
  Application.CreateForm(TfrmMain, frmMain);
  Application.CreateForm(TfrmChooseHue, frmChooseHue);
  Application.CreateForm(TfrmCenter, frmCenter);
  Application.CreateForm(TfrmOverview, frmOverview);
  Application.CreateForm(TfrmRegions, frmRegions);
  Application.CreateForm(TfrmPatternColor, frmPatternColor);
  Application.Run;
end.
