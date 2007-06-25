unit Overview;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, ExtCtrls, GR32, GR32_Image, UOMap, UOStatics, GR32_Layers;

type
  TfrmOverview = class(TForm)
    Image: TImage32;
    procedure ontopTimerTimer(Sender: TObject);
    procedure FormCreate(Sender: TObject);
    procedure ImageMouseDown(Sender: TObject; Button: TMouseButton;
      Shift: TShiftState; X, Y: Integer; Layer: TCustomLayer);
    procedure ImageDblClick(Sender: TObject);
  private
    { Private declarations }
    Map: Integer; // Map id
    MapPreviews: Array[0..4] of TBitmap32;
    CurrentX, CurrentY: Integer;
    MapHeight, MapWidth: Integer;

  public
    { Public declarations }
    Generating: Boolean;
    
    procedure showMap(Map: Integer);
  end;

var
  frmOverview: TfrmOverview;

implementation

uses Main, Progress;

{$R *.dfm}

procedure TfrmOverview.ontopTimerTimer(Sender: TObject);
begin
  if frmMain.cbAlwaysOnTop.Checked then
    frmOverview.FormStyle := fsStayOnTop;
end;

procedure TfrmOverview.FormCreate(Sender: TObject);
begin
  // Clear the map previe fields
  MapPreviews[0] := nil;
  MapPreviews[1] := nil;
  MapPreviews[2] := nil;
  MapPreviews[3] := nil;
  Map := -1;
  Generating := False;
end;

procedure TfrmOverview.ShowMap(Map: Integer);
var
  Preview: TBitmap32;
  x, y: Integer;
  MapReader: TMapReader;
  StaticReader: TStaticReader;
  MapCell: TMapCell;
  StaticCell: TStaticBlock;
  HighestZ: Shortint;
  CellColor: TColor;
  i: Integer;
  StaidxTime, StaticsTime, MapTime: TDateTime;
  Prgr: TFrmProgress;
begin
  if Generating then
    exit;

  if (Map < 0) or (Map > 4) then
    exit; // Invalid Map

  Self.Map := Map; // Save Map Id

  // Get a map preview...
  if MapPreviews[Map] <> nil then begin
    Image.Bitmap := MapPreviews[Map];
    exit;
  end;

  // Try to read the map from file
  Generating := True;
  Preview := TBitmap32.Create;

  try
    // Try getting the modify time
    // ModifyTime := FileDateToDateTime(FileAge(Format('map%u.bmp', [Map])));
    MapTime := StrToDateTimeDef(Config.getString(Format('Map%uModify', [Map]), ''), Time);
    StaticsTime := StrToDateTimeDef(Config.getString(Format('Statics%uModify', [Map]), ''), Time);
    StaidxTime := StrToDateTimeDef(Config.getString(Format('Staidx%uModify', [Map]), ''), Time);

    // Check if the loaded maps/statics have different times
    if (Maps[Map].LastModified = MapTime) and (Statics[Map].LastDataModification = StaticsTime) and
      (Statics[Map].LastIndexModification = StaidxTime) then
    begin
      Preview.LoadFromFile(Format('map%u.bmp', [Map]));
      MapPreviews[Map] := Preview;
      Image.Bitmap := Preview;
      ClientWidth := Preview.Width;
      ClientHeight := Preview.Height;
      MapWidth := Preview.Width;
      MapHeight := Preview.Height;
      Generating := False;
      exit;
    end;
    Preview.Free;
  except
    Preview.Free;
  end;

  // Create a map preview
  Prgr := TfrmProgress.Create(self);
  Prgr.FormStyle := fsStayOnTop;
  Prgr.Show;

  MapWidth := Maps[Map].getWidth;
  MapHeight := Maps[Map].getHeight;

  Prgr.pgBar.Max := MapWidth * MapHeight;

  Preview := TBitmap32.Create;
  Preview.SetSize(MapWidth, MapHeight);
  Preview.Clear(clBlack32);

  // Cache the readers
  MapReader := Maps[Map];
  StaticReader := Statics[Map];

  // Create the preview
  for x := 0 to MapWidth - 1 do begin
    for y := 0 to MapHeight - 1 do begin
      MapReader.ReadCell(x * 8, y * 8, MapCell);
      CellColor := RadarCol.GetLandColor(MapCell.Id);

      // Get Static Tiles
      HighestZ := MapCell.Height;
      StaticReader.ReadBlock(x, y, StaticCell);

      for i := 0 to Length(StaticCell) - 1 do begin
        if StaticCell[i].X <> 0 then
          continue;

        if StaticCell[i].Y <> 0 then
          continue;

        if StaticCell[i].Z < HighestZ then
          continue;

        if StaticCell[i].Hue = 0 then
          CellColor := RadarCol.GetItemColor(StaticCell[i].Id)
        else begin
          CellColor := RadarCol.GetItemColor(StaticCell[i].Id);
          CellColor := Hues.translateColor(StaticCell[i].Hue, CellColor);
        end;

        HighestZ := StaticCell[i].Z;
      end;

      Preview.Pixel[x, y] := CellColor;
    end;
    Prgr.pgBar.StepBy(MapHeight);
    Application.ProcessMessages;
  end;

  MapPreviews[Map] := Preview;
  Image.Bitmap := Preview;

  // Write the bitmap to a file
  Preview.SaveToFile(Format('map%u.bmp', [Map]));

  // Save the modification times of the statics and map
  Config.setString(Format('Map%uModify', [Map]), DateTimeToStr(MapReader.LastModified));
  Config.setString(Format('Statics%uModify', [Map]), DateTimeToStr(StaticReader.LastDataModification));
  Config.setString(Format('Staidx%uModify', [Map]), DateTimeToStr(StaticReader.LastIndexModification));
  Config.Save;

  ClientWidth := Preview.Width;
  ClientHeight := Preview.Height;

  Prgr.Free;

  Generating := False;
end;

procedure TfrmOverview.ImageMouseDown(Sender: TObject;
  Button: TMouseButton; Shift: TShiftState; X, Y: Integer;
  Layer: TCustomLayer);
begin
  if (x < 0) or (y < 0) or (x >= MapWidth) or (y >= MapHeight) then
    exit;

  CurrentX := X * 8; // Save for DClick
  CurrentY := Y * 8; // Save for DClick

  MapCenterX := X * 8;
  MapCenterY := Y * 8;
  MapCenterMap := Self.Map;
  frmMain.pbMap.Invalidate;
end;

procedure TfrmOverview.ImageDblClick(Sender: TObject);
begin
  frmMain.Go1Click(Self);
end;

end.
