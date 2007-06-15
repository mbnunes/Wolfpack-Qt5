unit UOHues;

interface

uses Classes, Graphics;

type THue = packed record
  Colors: Array[0..31] of Word;
  TableStart: Word;
  TableEnd: Word;
  Name: Array[0..19] of Char;
end;

type THueBlock = packed record
  Header: Cardinal;
  Hues: Array[0..7] of THue;
end;

type TProcessedHue = Array[0..31] of TColor;
type PHue = ^THue;

type THuesReader = class(TObject)
	private
    Colors: Array[0..$BB7] of TProcessedHue;
    Hues: Array[0..374] of THueBlock;
    TrueBlack: THue;
	public
    constructor Create; overload;
    function Load( FileName: String ): Boolean;
    function getHue(Id: Word): PHue;
    function translateColor(Hue: Word; Color: TColor;
      Partial: Boolean = False): TColor;
    function getColor(Hue: Word; Color: Byte): TColor;
end;

implementation

uses Windows, SysUtils, UOUtilities;

constructor THuesReader.Create;
begin
	ZeroMemory( @Colors, $BB8 * sizeof( TProcessedHue ) );
  ZeroMemory(@Hues, $BB8 * sizeof( THue ) );
  ZeroMemory(@TrueBlack, sizeof(THue));
  TrueBlack.Name := 'True Black';
end;

function THuesReader.Load( FileName: String ): Boolean;
var
  Input: TFileStream;
  i, j, k: Integer;
begin
  Result := False;

  try
    Input := TFileStream.Create( FileName, fmOpenRead+fmShareDenyNone );

    // Read Blocks
    for i := 0 to 374 do
    begin
      Input.Read(Hues[i], sizeof(THueBlock));
      for j := 0 to 7 do
      begin
        for k := 0 to 31 do
          Colors[i * 8 + j][k] := Color15to24(Hues[i].Hues[j].Colors[k]);
      end;
    end;

    Input.Free;
    Result := True;
  except
  end;
end;

function THuesReader.getHue(Id: Word): PHue;
var
  Block: Word;
begin
  Result := nil;

  if Id = 0 then
    exit;

  If Id = 1 then
  begin
    Result := @TrueBlack;
    exit;
  end;

  Dec(Id, 1);

  Block := Id div 8;
  if Block <= 374 then
    Result := @(Hues[Block].Hues[Id mod 8]);
end;

function THuesReader.translateColor(Hue: Word; Color: TColor;
  Partial: Boolean = False): TColor;
var
  Offset: Byte;
begin
  if Hue = 1 then
  begin
    Result := clBlack;
    exit;
  end;

  if Hue = 0 then
  begin
    Result := Color;
    exit;
  end;

  Dec(Hue); // Hue 1 is Blue

  Hue := Hue mod $BB8;
  Offset := GetRValue(Color) div 8;
  Result := Colors[Hue][Offset];
end;

function THuesReader.getColor(Hue: Word; Color: Byte): TColor;
begin
  if Hue = 1 then
  begin
      Result := clBlack;
      exit;
  end;

  Dec(Hue); // Hue 1 is blue
  Hue := Hue mod $BB8;

  Result := Colors[Hue][Color mod 32];
end;

end.
