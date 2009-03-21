program WPGM;

uses
  Forms,
  midaslib,
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
  XMLReader in '..\src\XMLReader.pas',
  RegionWizard in '..\src\RegionWizard.pas' {frmRegionW1},
  RegionWizard2Region in '..\src\RegionWizard2Region.pas' {frmRegionW2R},
  RegionWizard3Region in '..\src\RegionWizard3Region.pas' {FrmRegionW3R},
  RegionWizard4Region in '..\src\RegionWizard4Region.pas' {frmRergionW4R},
  RegionWizard2Spawn in '..\src\RegionWizard2Spawn.pas' {frmRegionW2S},
  RegionWizard3Spawn in '..\src\RegionWizard3Spawn.pas' {FrmRegionW3S},
  RegionWizard4Spawn in '..\src\RegionWizard4Spawn.pas' {FrmRegionW4S},
  RegionEditR in '..\src\RegionEditR.pas' {FrmRegionEdit},
  RegionEditS in '..\src\RegionEditS.pas' {FrmRegionEditS};

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
  Application.CreateForm(TfrmRegionW1, frmRegionW1);
  Application.CreateForm(TfrmRegionW2R, frmRegionW2R);
  Application.CreateForm(TFrmRegionW3R, FrmRegionW3R);
  Application.CreateForm(TfrmRergionW4R, frmRergionW4R);
  Application.CreateForm(TfrmRegionW2S, frmRegionW2S);
  Application.CreateForm(TFrmRegionW3S, FrmRegionW3S);
  Application.CreateForm(TFrmRegionW4S, FrmRegionW4S);
  Application.CreateForm(TFrmRegionEdit, FrmRegionEdit);
  Application.CreateForm(TFrmRegionEditS, FrmRegionEditS);
  Application.Run;
end.
