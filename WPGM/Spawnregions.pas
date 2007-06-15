unit Spawnregions;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, StdCtrls, GR32, GR32_Blend, GR32_Image, UOMap, UOStatics, ClipBrd;

type
  TfrmRegions = class(TForm)
    cbMap: TComboBox;
    pbMap: TPaintBox32;
    sbX: TScrollBar;
    sbY: TScrollBar;
    Label1: TLabel;
    Label2: TLabel;
    lblScreen: TLabel;
    lblMap: TLabel;
    lbRects: TListBox;
    btnAdd: TButton;
    btnRemove: TButton;
    Label3: TLabel;
    lblSelected: TLabel;
    Button1: TButton;
    Button2: TButton;
    Button3: TButton;
    Button4: TButton;
    btnZoomOut: TButton;
    btnZoomIn: TButton;
    btnRedraw: TButton;
    procedure FormShow(Sender: TObject);
    procedure FormCreate(Sender: TObject);
    procedure pbMapPaintBuffer(Sender: TObject);
    procedure sbXScroll(Sender: TObject; ScrollCode: TScrollCode;
      var ScrollPos: Integer);
    procedure sbYScroll(Sender: TObject; ScrollCode: TScrollCode;
      var ScrollPos: Integer);
    procedure FormDestroy(Sender: TObject);
    procedure FormResize(Sender: TObject);
    procedure pbMapMouseMove(Sender: TObject; Shift: TShiftState; X,
      Y: Integer);
    procedure btnRemoveClick(Sender: TObject);
    procedure pbMapMouseDown(Sender: TObject; Button: TMouseButton;
      Shift: TShiftState; X, Y: Integer);
    procedure pbMapMouseUp(Sender: TObject; Button: TMouseButton;
      Shift: TShiftState; X, Y: Integer);
    procedure btnAddClick(Sender: TObject);
    procedure Button1Click(Sender: TObject);
    procedure Button2Click(Sender: TObject);
    procedure Button3Click(Sender: TObject);
    procedure Button4Click(Sender: TObject);
    procedure lbRectsKeyDown(Sender: TObject; var Key: Word;
      Shift: TShiftState);
    procedure lbRectsDblClick(Sender: TObject);
    procedure btnZoomOutClick(Sender: TObject);
    procedure btnZoomInClick(Sender: TObject);
    procedure btnRedrawClick(Sender: TObject);
  private
    { Private declarations }
    Places: Array of TRect;

  public
    { Public declarations }
    procedure ChangeMap(MapId: Byte);
    procedure RedrawBuffer(Buffer: TBitmap32; StartX: Integer; StartY: Integer; Width: Integer; Height: Integer);

    function MapToBuffer(Point: TPoint): TPoint;
    function BufferToMap(Point: TPoint): TPoint;
    procedure AddRect(X1: Integer; Y1: Integer; X2: Integer; Y2: Integer);
    procedure UpdateSelectionLabel;
  end;

var
  frmRegions: TfrmRegions;
  Map: TMapReader;
  Statics: TStaticReader;
  Scale: Single;
  Buffer: TBitmap32; // This is for copying map2map
  SelStartX, SelStartY: Integer;
  SelEndX, SelEndY: Integer;

implementation

{$R *.dfm}

uses Main, Math, XMLReader, StrLib, UORadarCol;

function TfrmRegions.MapToBuffer(Point: TPoint): TPoint;
begin
  Result.X := Ceil((Point.X - sbX.Position) / Scale);
  Result.Y := Ceil((Point.Y - sbY.Position) / Scale);
end;

function TfrmRegions.BufferToMap(Point: TPoint): TPoint;
begin
  Result.X := Ceil(Point.X * Scale) + sbX.Position;
  Result.Y := Ceil(Point.Y * Scale) + sbY.Position;
end;

procedure TfrmRegions.RedrawBuffer(Buffer: TBitmap32; StartX: Integer; StartY: Integer; Width: Integer; Height: Integer);
var
  X, Y, RealX, RealY: Integer;
  MapBlock: TMapBlock;
  i, j, k, l: Integer;
  MapX, MapY: Integer; // Map Coordinates
  MapPoint, ScreenPoint: TPoint;
  MinBlockX, MinBlockY: Integer; // The lowest blocks we draw
  MaxBlockX, MaxBlockY: Integer; // The highest blocks we draw
  BlendColor: TColor32;
  PixelCount: Integer;
  StaticBlock: TStaticBlock;
  DrawColor: TColor32;
  DrawColorZ: Integer;
begin
  BlendColor := Color32(255, 255, 255, 127);

  MapPoint := BufferToMap(Point(StartX, StartY));
  MinBlockX := MapPoint.X div 8;
  MinBlockY := MapPoint.Y div 8;

  MapPoint := BufferToMap(Point(StartX + Width - 1, StartY + Height - 1));
  MaxBlockX := MapPoint.X div 8;
  MaxBlockY := MapPoint.Y div 8;

  // Scale down/up ? (If Scale > 1, we draw less blocks), thus
  // we have to divide and round up
  {MaxBlockX := Ceil(MaxBlockX / Scale);
  MaxBlockY := Ceil(MaxBlockY / Scale);}

  for X := MinBlockX to MaxBlockX + 1 do
    for Y := MinBlockY to MaxBlockY + 1 do begin
      ScreenPoint := MapToBuffer(Point(X * 8, Y * 8));

      // Out of screen check
      if (ScreenPoint.X < -7) or (ScreenPoint.Y < -7) then
        continue;

      // Out of screen check
      if (ScreenPoint.X >= Buffer.Width) or (ScreenPoint.Y >= Buffer.Height) then
        continue;

      Map.ReadBlock(X, Y, MapBlock);
      Statics.ReadBlock(X, Y, StaticBlock);

      for i := 0 to 63 do begin
        PixelCount := Ceil(1.0 / Scale);

        DrawColor := RadarCol.GetLandColor(MapBlock.Cells[i].Id);
        DrawColorZ := MapBlock.Cells[i].Height;

        // Get the topmost color here
        for j := 0 to Length(StaticBlock) - 1 do begin
          if (StaticBlock[j].X = i mod 8) and (StaticBlock[j].Y = i div 8) and
            (StaticBlock[j].Z >= DrawColorZ) then begin
              DrawColorZ := StaticBlock[j].Z;
              DrawColor := RadarCol.GetItemColor(StaticBlock[j].Id);
            end;
        end;

        RealX := ScreenPoint.X + (i mod 8) * PixelCount;
        RealY := ScreenPoint.Y + (i div 8) * PixelCount;
        if Scale <> 1.0 then begin
          for j := 0 to PixelCount - 1 do begin
            for k := 0 to PixelCount - 1 do begin
              Buffer.PixelS[RealX + j, RealY + k] := DrawColor;
            end;
          end;
        end else begin
          Buffer.PixelS[RealX, RealY] := DrawColor;
        end;

        MapX := X * 8 + i mod 8;
        MapY := Y * 8 + i div 8;

        // Pr�fen ob der Punkt teil eines Rectangles ist
        for j := 0 to Length(Places) - 1 do begin
          if (Places[j].Left <= MapX) and (Places[j].Right >= MapX)
            and (Places[j].Top <= MapY) and (Places[j].Bottom >= MapY) then
          begin
            // Found a match!!
            Buffer.PixelS[RealX, RealY] := BlendReg(BlendColor, Buffer.PixelS[RealX, RealY]);
            EMMS();
            //break;
          end;
        end;
      end;
    end;

  // Redraw selection rectangle
  if (SelStartX <> -1) and (SelStartY <> -1) then begin
    MapPoint := MapToBuffer(Point(SelStartX, SelStartY));

    pbMap.Buffer.Canvas.Brush.Style := bsClear;
    pbMap.Buffer.Canvas.Pen.Color := clWhite;
    pbMap.Buffer.Canvas.Pen.Style := psDot;

    if (SelEndX <> -1) and (SelEndY <> -1) then begin
      ScreenPoint := MapToBuffer(Point(SelEndX, SelEndY));
    end else begin
      ScreenPoint := pbMap.ScreenToClient(Mouse.CursorPos);
    end;

    pbMap.Buffer.Canvas.Rectangle(MapPoint.X, MapPoint.Y, ScreenPoint.X, ScreenPoint.Y);
  end;
end;

procedure TfrmRegions.ChangeMap(MapId: Byte);
begin
  if MapId >= Length(Maps) then
    exit;

  Map := Main.Maps[MapId];
  Statics := Main.Statics[MapId];

  sbX.Max := Map.getWidth * 8 - 1 - pbMap.ClientWidth;
  sbX.Min := 0;
  sbX.Position := 0;

  sbY.Max := Map.getHeight * 8 - 1 - pbMap.ClientHeight;
  sbY.Min := 0;
  sbY.Position := 0;

  pbMap.Invalidate;
end;

procedure TfrmRegions.FormShow(Sender: TObject);
begin
  ChangeMap(0);
end;

procedure TfrmRegions.FormCreate(Sender: TObject);
begin
  Map := nil;
  Statics := nil;
  Scale := 1;
  Buffer := TBitmap32.Create;

  SelStartX := -1;
  SelStartY := -1;
  SelEndX := -1;
  SelEndY := -1;

  {AddRect(1776, 60, 3060, 670);
  AddRect(1900, 670, 2515, 870);
  AddRect(2515, 670, 2762, 870);
  AddRect(3009, 100, 3055, 720);
  AddRect(3055, 150, 3110, 655);
  AddRect(3110, 320, 3210, 595);
  AddRect(3210, 375, 3360, 550);
  AddRect(3360, 410, 3424, 510);
  AddRect(3424, 410, 3453, 465);}
end;

procedure TfrmRegions.pbMapPaintBuffer(Sender: TObject);
begin
  RedrawBuffer(pbMap.Buffer, 0, 0, pbMap.Buffer.Width, pbMap.Buffer.Height);
end;

procedure TfrmRegions.sbXScroll(Sender: TObject; ScrollCode: TScrollCode;
  var ScrollPos: Integer);
var
  Diff: Integer;
  OldPos: Integer;
begin
  Diff := ScrollPos - (Sender as TScrollBar).Position;

  if (Buffer.Width <> pbMap.Buffer.Height) or (Buffer.Height <> pbMap.Buffer.Height) then
  begin
    Buffer.SetSizeFrom(pbMap.Buffer);
  end;

  if (Abs(Diff) > pbMap.Width) then
    pbMap.Invalidate
  else begin
    pbMap.Buffer.DrawTo(Buffer);
    Buffer.DrawTo(pbMap.Buffer, - Ceil(Diff / Scale), 0);

    OldPos := sbX.Position;
    sbX.Position := ScrollPos;

    // If Diff is Positive, redraw right part
    if Diff >= 0 then
      RedrawBuffer(pbMap.Buffer, pbMap.Buffer.Width - Ceil(Diff / Scale), 0, Trunc(Diff / Scale), pbMap.Buffer.Height);

    // If Diff is Negative, redraw left part
    if Diff < 0 then
      RedrawBuffer(pbMap.Buffer, 0, 0, - Trunc(Diff / Scale), pbMap.Buffer.Height);

    sbX.Position := OldPos;                              

    pbMap.Flush;
  end;
end;

procedure TfrmRegions.sbYScroll(Sender: TObject; ScrollCode: TScrollCode;
  var ScrollPos: Integer);
var
  Diff: Integer;
  OldPos: Integer;
begin
  Diff := ScrollPos - (Sender as TScrollBar).Position;

  if (Buffer.Width <> pbMap.Buffer.Height) or (Buffer.Height <> pbMap.Buffer.Height) then
  begin
    Buffer.SetSizeFrom(pbMap.Buffer);
  end;

  if (Abs(Diff) > pbMap.Height) then
    pbMap.Invalidate
  else begin
    pbMap.Buffer.DrawTo(Buffer);
    Buffer.DrawTo(pbMap.Buffer, 0, - Ceil(Diff / Scale));

    OldPos := sbY.Position;
    sbY.Position := ScrollPos;

    // If Diff is Positive, redraw right part
    if Diff >= 0 then
      RedrawBuffer(pbMap.Buffer, 0, pbMap.Height - Ceil(Diff / Scale), pbMap.Buffer.Width, Ceil(Diff / Scale));

    // If Diff is Negative, redraw right part
    if Diff < 0 then
      RedrawBuffer(pbMap.Buffer, 0, 0, pbMap.Buffer.Width, - Ceil(Diff / Scale));

    sbY.Position := OldPos;
    pbMap.Flush;
  end;
end;

procedure TfrmRegions.FormDestroy(Sender: TObject);
begin
  Buffer.Free;
end;

procedure TfrmRegions.FormResize(Sender: TObject);
var
  OldX, OldY: Integer;
begin
  OldX := sbX.Position;
  OldY := sbY.Position;

  ChangeMap(MapId);
  sbX.Position := OldX;
  sbY.Position := OldY;
  pbMap.Invalidate;
end;

procedure TfrmRegions.pbMapMouseMove(Sender: TObject; Shift: TShiftState;
  X, Y: Integer);
var
  P: TPoint;
begin
  P := BufferToMap(Point(X, Y));

  lblScreen.Caption := Format('%d,%d', [X, Y]);
  lblMap.Caption := Format('%d,%d', [P.X, P.Y]);

  if ((SelStartX <> -1) or (SelStartY <> -1)) and
    ((SelEndX = -1) and (SelEndY = -1)) then begin
    pbMap.Invalidate;
  end;
end;

procedure TfrmRegions.btnRemoveClick(Sender: TObject);
var
  i: Integer;
begin
  if lbRects.ItemIndex <> -1 then begin
    if lbRects.ItemIndex < Length(Places) - 1 then begin
      // Copy Downwards
      for i := lbRects.ItemIndex to Length(Places) - 2 do begin
        Places[i] := Places[i+1];
      end;
      SetLength(Places, Length(Places) - 1);
      pbMap.Invalidate;
    end;
    lbRects.Items.Delete(lbRects.ItemIndex);
  end;
end;

procedure TFrmRegions.AddRect(X1: Integer; Y1: Integer; X2: Integer; Y2: Integer);
var
  Temp: Integer;
begin
  if X1 > X2 then begin
    Temp := X1;
    X1 := X2;
    X2 := temp;
  end;

  if Y1 > Y2 then begin
    Temp := Y1;
    Y1 := Y2;
    Y2 := temp;
  end;

  SetLength(Places, Length(Places) + 1);
  Places[Length(Places) - 1] := Rect(X1, Y1, X2, Y2);

  lbRects.Items.Add(Format('%d,%d to %d,%d', [X1, Y1, X2, Y2]));
end;

procedure TfrmRegions.pbMapMouseDown(Sender: TObject; Button: TMouseButton;
  Shift: TShiftState; X, Y: Integer);
var
  P: TPoint;
begin
  SelStartX := -1;
  SelStartY := -1;
  SelEndX := -1;
  SelEndY := -1;

  P := BufferToMap(Point(X, Y));
  SelStartX := P.X;
  SelStartY := P.Y;

  UpdateSelectionLabel;
  pbMap.Invalidate;
end;

procedure TfrmRegions.pbMapMouseUp(Sender: TObject; Button: TMouseButton;
  Shift: TShiftState; X, Y: Integer);
var
  P: TPoint;
begin
  P := BufferToMap(Point(X, Y));
  SelEndX := P.X;
  SelEndY := P.Y;
  UpdateSelectionLabel;
  pbMap.Invalidate;
end;

procedure TFrmRegions.UpdateSelectionLabel;
begin
  if (SelStartX = -1) or (SelStartY = -1) or (SelEndX = -1) or (SelEndY = -1) then
  begin
    lblSelected.Caption := '';
    btnAdd.Enabled := False;
  end else begin
    lblSelected.Caption := Format('%d,%d to %d,%d', [SelStartX, SelStartY, SelEndX, SelEndY]);
    btnAdd.Enabled := True;
  end;
end;

procedure TfrmRegions.btnAddClick(Sender: TObject);
begin
  AddRect(SelStartX, SelStartY, SelEndX, SelEndY);
  SelStartX := -1;
  SelStartY := -1;
  SelEndX := -1;
  SelEndY := -1;
  pbMap.Invalidate;
end;

procedure TfrmRegions.Button1Click(Sender: TObject);
var
  i: Integer;
  Result: String;
  ClipBoard: TClipboard;
begin
  ClipBoard := TClipBoard.Create;

  for i := 0 to Length(Places) - 1 do begin
    Result := Result + Format(#9 + #9 + '<rectangle from="%d,%d" to="%d,%d" map="%u" />' + #13, [Places[i].Left, Places[i].Top, Places[i].Right, Places[i].Bottom, cbMap.ItemIndex]);
  end;

  ClipBoard.SetTextBuf(PChar(result));
end;

procedure TfrmRegions.Button2Click(Sender: TObject);
var
  i: Integer;
  Result: String;
  ClipBoard: TClipboard;
begin
  ClipBoard := TClipBoard.Create;

  for i := 0 to Length(Places) - 1 do begin
    Result := Result + Format(#9 + #9 + '<rectangle x1="%d" y1="%d" x2="%d" y2="%d" map="%u" />' + #13, [Places[i].Left, Places[i].Top, Places[i].Right, Places[i].Bottom, cbMap.ItemIndex]);
  end;

  ClipBoard.SetTextBuf(PChar(result));
  ClipBoard.Free;
end;

procedure TfrmRegions.Button3Click(Sender: TObject);
var
  ClipBoard: TClipboard;
  Content: String;
  Reader: TXMLReader;
  Document, Node: TXMLNode;
  i: Integer;
  sFrom, sTo: String;
  Parts: TStringList;
  x1, x2, y1, y2: Integer;
begin
  ClipBoard := TClipBoard.Create;
  Content := '<rectangles>' + ClipBoard.AsText + '</rectangles>';

  Reader := nil;
  Document := nil;
  Parts := TStringList.Create;

  try
    Reader := TXMLStringReader.Create(Content);
    Document := Reader.ParseDocument.Nodes[0];

    // Iterate over the rectangle nodes
    for i := 0 to Document.NodeCount - 1 do begin
      Node := Document.Nodes[i];

      if Node.Name <> 'rectangle' then
        continue; // Skip this node

      // Look up From + To
      Node.LookupBasicData('from', sFrom);
      Node.LookupBasicData('to', sTo);

      Parts.Clear;
      Split(sFrom, ',', Parts);
      if Parts.Count <> 2 then
        continue; // From attribute is incorrect

      x1 := StrToIntDef(Parts[0], 0);
      y1 := StrToIntDef(Parts[1], 0);

      Parts.Clear;
      Split(sTo, ',', Parts);
      if Parts.Count <> 2 then
        continue; // To attribute is incorrect

      x2 := StrToIntDef(Parts[0], 0);
      y2 := StrToIntDef(Parts[1], 0);

      AddRect(x1, y1, x2, y2);
    end;
  except
    on e: Exception do begin
      Application.MessageBox(PAnsiChar('The following error occured while parsing the XML definition: ' + #13 + e.Message), 'Error', MB_OK+MB_ICONERROR);
    end;
  end;

  Reader.Free;
  Document.Free;
  Parts.Free;
  ClipBoard.Free;
end;

procedure TfrmRegions.Button4Click(Sender: TObject);
var
  ClipBoard: TClipboard;
  Content: String;
  Reader: TXMLReader;
  Document, Node: TXMLNode;
  i: Integer;
  Temp: String;
  x1, x2, y1, y2: Integer;
begin
  ClipBoard := TClipBoard.Create;
  Content := '<rectangles>' + ClipBoard.AsText + '</rectangles>';

  Reader := nil;
  Document := nil;

  try
    Reader := TXMLStringReader.Create(Content);
    Document := Reader.ParseDocument.Nodes[0];

    // Iterate over the rectangle nodes
    for i := 0 to Document.NodeCount - 1 do begin
      Node := Document.Nodes[i];

      if Node.Name <> 'rectangle' then
        continue; // Skip this node

      Node.LookupBasicData('x1', temp);
      x1 := StrToIntDef(temp, 0);

      Node.LookupBasicData('x2', temp);
      x2 := StrToIntDef(temp, 0);

      Node.LookupBasicData('y1', temp);
      y1 := StrToIntDef(temp, 0);

      Node.LookupBasicData('y2', temp);
      y2 := StrToIntDef(temp, 0);

      AddRect(x1, y1, x2, y2);
    end;
  except
    on e: Exception do begin
      Application.MessageBox(PAnsiChar('The following error occured while parsing the XML definition: ' + #13 + e.Message), 'Error', MB_OK+MB_ICONERROR);
    end;
  end;

  Reader.Free;
  Document.Free;
  ClipBoard.Free;
end;

procedure TfrmRegions.lbRectsKeyDown(Sender: TObject; var Key: Word;
  Shift: TShiftState);
begin
  if Key = 46 then
    btnRemoveClick(Sender);
end;

procedure TfrmRegions.lbRectsDblClick(Sender: TObject);
var
  centerx, centery: Integer;
begin
  if lbRects.ItemIndex = -1 then
    exit;

  // center the map view on the dbl clicked region
  CenterX := (Places[lbRects.ItemIndex].Right - Places[lbRects.ItemIndex].Left) div 2;
  CenterX := CenterX + Places[lbRects.ItemIndex].Left;

  CenterY := (Places[lbRects.ItemIndex].Bottom - Places[lbRects.ItemIndex].Top) div 2;
  CenterY := CenterY + Places[lbRects.ItemIndex].Top;

  // Now take the viewport into account
  CenterX := CenterX - Trunc((pbMap.ClientWidth div 2) * Scale);
  CenterY := CenterY - Trunc((pbMap.ClientHeight div 2) * Scale);

  if CenterX > sbX.Max then
    CenterX := sbX.Max;

  if CenterY > sbY.Max then
    CenterY := sbY.Max;

  if CenterX < 0 then
    CenterX := 0;

  if CenterY < 0 then
    CenterY := 0;

  sbX.Position := CenterX;
  sbY.Position := CenterY;
  pbMap.Invalidate;
end;

procedure TfrmRegions.btnZoomOutClick(Sender: TObject);
begin
  Scale := Scale / 0.5;
  pbMap.Invalidate;
end;

procedure TfrmRegions.btnZoomInClick(Sender: TObject);
begin
  Scale := Scale * 0.5;
  pbMap.Invalidate;
end;

procedure TfrmRegions.btnRedrawClick(Sender: TObject);
begin
  pbMap.Invalidate;
end;

end.
