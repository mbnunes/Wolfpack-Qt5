unit UOAnim;

interface

uses Classes, SysUtils, GR32, UOHues;

// Animation Frame
type TAnimFrame = record
  Image: TBitmap32;
  CenterX, CenterY: Integer;
end;

type PAnimFrame = ^TAnimFrame;

type TAnimation = class
private
  Frames: TList;
public
  Body: Word;
  Action: Word;
  FrameDelay: Word;
  Hue: Word;

  function getTotalHeight: Integer;
  function getTotalWidth: Integer;

  constructor Create;
  destructor Destroy; override;

  function frameCount: Integer;
  function getFrame(i: Integer): PAnimFrame;
  procedure addFrame(frame: PAnimFrame);
end;

type TBodyConv = record
  Body: Integer;
  Anim2: Integer;
  Anim3: Integer;
end;

type TBodyDef = record
  Body: Integer;
  NewBody: Integer;
  NewHue: Integer;
end;

type TAnimReader = class
private
    dStream, iStream: TStream;
    dStream2, iStream2: TStream;
    dStream3, iStream3: TStream;
    disabled: Boolean;
    BodyConv: Array of TBodyConv;
    BodyDef: Array of TBodyDef;
                     
    function GetData( Body: Word; Action: Word; var Length: Cardinal ): TStream;
    procedure LoadBodyConv(Filename: String);
    procedure LoadBodyDef(FileName: String);
public
    hues: THuesReader;

    constructor Create;
    destructor Free;
    function GetFrames( Body, Action, Color: Word): TAnimation; overload;
    function GetFrames( Body, Action: Word ): TAnimation; overload;
    procedure getBodyConv(Body: Word; var Anim2: Integer; var Anim3: Integer);
    function getBodyDef(var Body: Word; var Color: Word): Boolean;

    function Open( IndexName1, DataName1, IndexName2, DataName2, IndexName3, DataName3, BodyConv, BodyDef: String ): Boolean;
    procedure Close();
end;

implementation

uses Forms, Windows, UOUtilities, Graphics, Math, StrUtils;

constructor TAnimation.Create;
begin
  Frames := TList.Create;
end;

function TAnimation.getTotalHeight: Integer;
var
  i: Integer;
  frame: PAnimFrame;
begin
  Result := 0;

  for i := 0 to frameCount - 1 do
  begin
    frame := frames[i];
    Result := Max(Result, frame.Image.Height - frame.CenterY);
  end;
end;

function TAnimation.getTotalWidth: Integer;
var
  i: Integer;
  frame: PAnimFrame;
begin
  Result := 0;

  for i := 0 to frameCount - 1 do
  begin
    frame := frames[i];
    Result := Max(Result, frame.Image.Width - frame.CenterX);
  end;
end;

destructor TAnimation.Destroy;
var
  i: Integer;
begin
  for i := 0 to frames.Count - 1 do begin
    PAnimFrame(Frames[i]).Image.Free;
    dispose(Frames[i]);
  end;
  Frames.Free;
  inherited Destroy;
end;

function TAnimation.frameCount: Integer;
begin
  Result := Frames.Count;
end;

function TAnimation.getFrame(i: Integer): PAnimFrame;
begin
  if I >= frames.count then
    result := nil
  else
    result := frames[i];
end;

procedure TAnimation.addFrame(frame: PAnimFrame);
begin
  frames.Add(frame);
end;

type TIndexRecord = packed record
    Offset: Integer;
    Length: Integer;
    Extra: Integer
end;

// Get a TMemoryStream of the desired data
function TAnimReader.GetData( Body: Word; Action: Word; var Length: Cardinal ): TStream;
var
    IndexRecord: TIndexRecord;
    Offset: Cardinal;
    Anim2, Anim3: Integer;
begin
    // CHECK FOR VERDATA HERE

    getBodyConv(Body, Anim2, Anim3);

    // Read From Anim3.mul
    if (Anim3 <> -1) and (iStream3 <> nil) and (dStream3 <> nil) then begin
      if Anim3 < 300 then
        Offset := Anim3 * 65 + Action
      else if Anim3 < 400 then
        Offset := 33000 + (Anim3 - 300) * 110 + Action
      else
        Offset := 35000 + (Anim3 - 400) * 175 + Action;

      iStream3.Seek( Offset * 12, soFromBeginning );
      iStream3.Read( IndexRecord, SizeOf( TIndexRecord ) );

      if IndexRecord.Offset <> -1 then begin
        Length := IndexRecord.Length;
        dStream3.Seek( IndexRecord.Offset, soFromBeginning );
        Result := dStream3;
        exit;
      end;
    end;

    // Read From Anim2.mul
    if (Anim2 <> -1) and (iStream2 <> nil) and (dStream2 <> nil) then begin
      if Anim2 < 200 then
        Offset := Anim2 * 110 + Action
      else
        Offset := 22000 + (Anim2 - 200) * 65 + Action;

      iStream2.Seek( Offset * 12, soFromBeginning );
      iStream2.Read( IndexRecord, SizeOf( TIndexRecord ) );

      if IndexRecord.Offset <> -1 then begin
        Length := IndexRecord.Length;
        dStream2.Seek( IndexRecord.Offset, soFromBeginning );
        Result := dStream2;
        exit;
      end;
    end;

    if Body <= $C7 then begin
      Action := Action mod 110;
      Offset := Body * 110 + Action;
    end else if Body <= $18F then begin
      Action := Action mod 65;
      Offset := (Body - $C8) * 65 + Action + 22000;
    end else begin
      Action := Action mod 175;
      Offset := (Body - $190) * 175 + Action + 35000;
    end;

    iStream.Seek( Offset * 12, soFromBeginning );
    iStream.Read( IndexRecord, SizeOf( TIndexRecord ) );

    if IndexRecord.Offset = -1 then begin
      Result := nil;
      Length := 0;
      exit;
    end;

    Length := IndexRecord.Length;

    dStream.Seek( IndexRecord.Offset, soFromBeginning );
    Result := dStream;
end;

function TAnimReader.Open( IndexName1, DataName1, IndexName2, DataName2, IndexName3, DataName3, BodyConv, BodyDef: String ): Boolean;
begin
 	Result := False;
  try
   	dStream := TFileStream.Create( DataName1, fmOpenRead+fmShareDenyNone );
    iStream := TFileStream.Create( IndexName1, fmOpenRead+fmShareDenyNone );
    Disabled := False;
  except
    Close;
  end;

  dStream2 := nil;
  dStream3 := nil;
  iStream2 := nil;
  iStream3 := nil;

  try
    dStream2 := TFileStream.Create( DataName2, fmOpenRead+fmShareDenyNone );
    iStream2 := TFileStream.Create( IndexName2, fmOpenRead+fmShareDenyNone );
  except
    FreeAndNil(iStream2);
    FreeAndNil(dStream2);
  end;

  try
    dStream3 := TFileStream.Create( DataName3, fmOpenRead+fmShareDenyNone );
    iStream3 := TFileStream.Create( IndexName3, fmOpenRead+fmShareDenyNone );
  except
    FreeAndNil(iStream3);
    FreeAndNil(dStream3);
  end;

  // Load Bodyconv.def
  LoadBodyConv(BodyConv);

  // Load Body.def
  LoadBodyDef(BodyDef);
end;

procedure TAnimReader.Close();
begin
    FreeAndNil( dStream );
	FreeAndNil( iStream );
    disabled := True;
end;

constructor TAnimReader.Create;
begin
    disabled := True;
    dStream := nil;
    iStream := nil;
    hues := nil;
end;

destructor TAnimReader.Free;
begin
end;

function TAnimReader.GetFrames( Body, Action, Color: Word): TAnimation;
var
  Data: TStream;
  Length: Cardinal;
  Palette: Array[0..255] of TColor32;
  i, j: Integer;
  Color15: Word;
  FrameCount: Integer;
  FrameOffsets: Array of Integer;
  ImageCenterX, ImageCenterY: Smallint;
  Width, Height: Word;
  YOffset, XOffset, XRun: Integer;
  Chunk: Integer;
  SeekStart: Integer;
  PaletteIndex: Byte;
  Frame: PAnimFrame;
  Hue: PHue;
begin
  Data := GetData(Body, Action, Length);

  if Data = nil then begin
    getBodyDef(Body, Color);

    Data := GetData(Body, Action, Length);

    if Data = nil then begin
      //Application.MessageBox(PAnsiChar('Unknown Body Id: ' + IntToStr(Body)), 'Error: Invalid Body Id', MB_OK+MB_ICONERROR);
      result := nil;
      exit;
    end;
  end;

  result := TAnimation.Create;
  result.body := body;
  result.action := action;
  result.FrameDelay := 80;

  if (Hues <> nil) and (Color <> 0) then
    Hue := Hues.getHue(Color)
  else
    Hue := nil;

  // Read the palette
  for i := 0 to 255 do begin
    Data.Read(Color15, 2);

    if hue <> nil then
      Color15 := hue.Colors[((Color15 shr 10) and $1F)];

    if Color15 = 0 then
      Palette[i] := 0
    else
      Palette[i] := Color15to32(Color15);
  end;

  SeekStart := Data.Position;

  Data.Read(FrameCount, 4);
  SetLength(FrameOffsets, FrameCount);

  for i := 0 to FrameCount - 1 do begin
    Data.Read(FrameOffsets[i], 4);
  end;

  // Seek to the requested frame
  for j := 0 to FrameCount - 1 do begin
    Data.Seek(SeekStart + FrameOffsets[j], soFromBeginning);

    // Read Frame Header
    Data.Read(ImageCenterX, 2);
    Data.Read(ImageCenterY, 2);
    Data.Read(Width, 2);
    Data.Read(Height, 2);

    new(Frame);
    Frame.Image := TBitmap32.Create;
    Frame.Image.SetSize(Width, Height);
    Frame.Image.DrawMode := dmBlend;
    Frame.CenterX := ImageCenterX;
    Frame.CenterY := ImageCenterY;

    while true do begin
      // Read a Chunk
      Data.Read(Chunk, 4);

      if (Chunk = $7FFF7FFF) then
        break;

      XRun := Chunk and $FFF; // Pixel Amount
      YOffset := (Chunk shr 12) and $3FF;
      XOffset := (Chunk shr 22) and $3FF;

      if XOffset and $200 <> 0 then
        XOffset := Integer(Cardinal(XOffset) or $FFFFFC00);

      if YOffset and $200 <> 0 then
        YOffset := Integer(Cardinal(YOffset) or $FFFFFC00);

      Inc(YOffset, Height);

      for i := 0 to XRun - 1 do begin
        Data.Read(PaletteIndex, 1);

        if Palette[PaletteIndex] <> 0 then
          Frame.Image.Pixel[ImageCenterX + XOffset + i, ImageCenterY + YOffset] := Palette[PaletteIndex];
      end;
    end;

    Result.addFrame(frame);
  end;
end;

function TAnimReader.GetFrames( Body, Action: Word ): TAnimation;
begin
  Result := GetFrames(Body, Action, 0);
end;

procedure TAnimReader.LoadBodyConv(Filename: String);
var
  Input: TextFile;
  Line: String;
  Pos, Body, Anim2, Anim3: Integer;
begin
  try
    AssignFile(Input, FileName);
    Reset(Input);

    // Read Line By Line
    while not Eof(Input) do begin
      ReadLn(Input, Line);

      // Comment Lines
      if LeftStr(Line, 1) = '#' then
        continue;

      // Get First Token
      Pos := AnsiPos(Char($9), Line);
      Body := StrToIntDef(LeftStr(Line, Pos - 1), -1);

      if Body = -1 then
        continue;

      // Get Second Token
      Line := RightStr(Line, Length(Line) - pos);
      Pos := AnsiPos(Char($9), Line);
      Anim2 := StrToIntDef(LeftStr(Line, Pos - 1), -1);

      // Get Third Token
      Line := RightStr(Line, Length(Line) - pos);
      Pos := AnsiPos(Char($9), Line);
      Anim3 := StrToIntDef(LeftStr(Line, Pos - 1), -1);

      // Save Token
      Pos := Length(BodyConv);
      SetLength(BodyConv, Pos + 1);
      BodyConv[Pos].Body := Body;
      BodyConv[Pos].Anim2 := Anim2;
      BodyConv[Pos].Anim3 := Anim3;
    end;

    CloseFile(Input);
  except
  end;
end;

procedure TAnimReader.LoadBodyDef(Filename: String);
var
  Input: TextFile;
  Line: String;
  Pos, Body, NewBody, NewHue: Integer;
  NewBodies: String;
begin
  try
    AssignFile(Input, FileName);
    Reset(Input);

    // Read Line By Line
    while not Eof(Input) do begin
      ReadLn(Input, Line);

      // Comment Lines
      if LeftStr(Line, 1) = '#' then
        continue;

      // Get First Token
      Pos := AnsiPos(' ', Line);
      Body := StrToIntDef(LeftStr(Line, Pos - 1), -1);

      if Body = -1 then
        continue;

      // Get Last Token
      Line := RightStr(Line, Length(Line) - pos);
      Pos := LastDelimiter(' ', Line);
      NewBodies := LeftStr(Line, Pos - 1);

      // Get Third Token
      Line := RightStr(Line, Length(Line) - pos);
      NewHue := StrToIntDef(Line, 0);

      If AnsiPos(',', NewBodies) = 0 then begin
        NewBody := StrToIntDef(MidStr(NewBodies, 2, Length(NewBodies) - 2), Body);
      end else begin
        // Get the first number and use it
        NewBodies := RightStr(NewBodies, Length(NewBodies) - 1);
        Pos := AnsiPos(',', NewBodies);
        NewBodies := LeftStr(NewBodies, Pos - 1);
        NewBody := StrToIntDef(NewBodies, Body);
      end;

      Pos := Length(BodyDef);
      SetLength(BodyDef, Pos + 1);
      BodyDef[Pos].Body := Body;
      BodyDef[Pos].NewBody := NewBody;
      BodyDef[Pos].NewHue := NewHue;
    end;

    CloseFile(Input);
  except
  end;
end;



procedure TAnimReader.getBodyConv(Body: Word; var Anim2: Integer; var Anim3: Integer);
var
  i: Integer;
begin
  for i := 0 to Length(BodyConv) - 1 do begin
    if BodyConv[i].Body = Body then begin
      Anim2 := BodyConv[i].Anim2;
      Anim3 := BodyConv[i].Anim3;
      exit;
    end;
  end;

  Anim2 := -1;
  Anim3 := -1;
end;

function TAnimReader.getBodyDef(var Body: Word; var Color: Word): Boolean;
var
  i: Integer;
begin
  for i := 0 to Length(BodyDef) - 1 do begin
    if BodyDef[i].Body = Body then begin
      Body := BodyDef[i].NewBody;
      Color := BodyDef[i].NewHue;
      Result := true;
      exit;
    end;
  end;

  Result := false;
end;

end.
