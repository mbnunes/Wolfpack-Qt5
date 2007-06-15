unit UOArt;

interface

uses Classes, SysUtils, GR32, UOHues;

type TArtReader = class
private
    dStream, iStream: TStream;
    disabled: Boolean;
    function GetData( ID: Word; var Length: Integer ): TStream;
public
    hues: THuesReader;
    
    constructor Create;
    destructor Free;
    function GetTile( ID, Color: Word ): TBitmap32; overload;
    function GetTile( Id: Word ): TBitmap32; overload;

    function Open( IndexName: String; DataName: String ): Boolean;
    procedure Close();
end;

var
    Art: TArtReader;

implementation

uses Forms, Windows, UOUtilities, Graphics;

type TIndexRecord = packed record
    Offset: Integer;
    Length: Integer;
    Extra: Integer
end;

// Get a TMemoryStream of the desired data
function TArtReader.GetData( ID: Word; var Length: Integer ): TStream;
var
    IndexRecord: TIndexRecord;
    Offset: Cardinal;
begin
    Result := nil;

    // CHECK FOR VERDATA HERE

    Offset := 12 * ( $4000 + Id );

    iStream.Seek( Offset, soFromBeginning );
    iStream.Read( IndexRecord, SizeOf( TIndexRecord ) );
    Length := IndexRecord.Length;

    if Length = -1 then
      exit;

    dStream.Seek( IndexRecord.Offset, soFromBeginning );
    Result := dStream;
end;

function TArtReader.Open( IndexName: String; DataName: String ): Boolean;
begin
 	Result := False;
  try
   	dStream := TFileStream.Create( DataName, fmOpenRead+fmShareDenyNone );
    iStream := TFileStream.Create( IndexName, fmOpenRead+fmShareDenyNone );
    Disabled := False;
  except
    Close;
  end;
end;

procedure TArtReader.Close();
begin
    FreeAndNil( dStream );
	FreeAndNil( iStream );
    disabled := True;
end;

constructor TArtReader.Create;
begin
    disabled := True;
    dStream := nil;
    iStream := nil;
    hues := nil;
end;

destructor TArtReader.Free;
begin
end;

// Create an Art-tile
function TArtReader.GetTile( ID, Color: Word ): TBitmap32;
var
    Data: TStream;
    x, y, i: Cardinal;
    Length: Integer;
    Width, Height: Word;
    LookupTable: Array of Word;
    DataSTArtReader: Cardinal;
    XOffset, RunLength, RunColor: Word;
    hue: PHue;
begin
    if (Color <> 0) and (hues <> nil) then begin
      hue := hues.getHue(color);
    end else begin
      hue := nil;
    end;

    Result := TBitmap32.Create;

    if disabled then
    begin
        Result.SetSize( 44, 44 );
        Result.Clear( Color32( clBtnFace ) );
        Result.Line( 1, 1, 43, 43, clRed32 );
        Result.Line( 43, 0, 0, 43, clRed32 );
        exit;
    end;

    Data := GetData( Id, Length );

    if( Length = -1 ) then
        exit;

    Data.Seek( 4, soFromCurrent );
    Data.Read( Width, 2 );
    Data.Read( Height, 2 );

    SetLength( LookupTable, Height );
    for i := 0 to Height-1 do
        Data.Read( LookupTable[i], 2 );

    DataSTArtReader := Data.Position;
    Result.SetSize( Width, Height );
    //Result.Clear( Color32( clBtnFace ) );
    Result.Clear(Color32(clBtnFace));

    // Draw the "tile" 3d wireframe
    for x := 0 to 21 do begin
      Result.PixelS[((Width - 1) div 2) - x, Height - 1 - x] := clRed32;
      Result.PixelS[((Width - 1) div 2) + x, Height - 1 - x] := clRed32;
      Result.PixelS[((Width - 1) div 2) - x, Height - 44 + x] := clRed32;
      Result.PixelS[((Width - 1) div 2) + x, Height - 44 + x] := clRed32;
    end;

    // Parse each line of the image
    for y := 0 to Height-1 do
    begin
        Data.Seek( DataSTArtReader + ( LookupTable[y] * 2 ), soFromBeginning );
        X := 0;

        // RLE Decoding (Read Chunks)
        repeat begin
            Data.Read( XOffset, 2 );
            Data.Read( RunLength, 2 );

            inc( x, XOffset );

            if( RunLength > 0 ) then
                for i := 0 to RunLength-1 do
                begin
                    Data.Read( RunColor, 2 );

                    // INSERT HUE PROCESSING HERE
                    if hue <> nil then begin
                      RunColor := hue.Colors[((RunColor shr 10) and $1F)];
                    end;

                    // 0 RRRRR GG|GGG BBBBB Encoding
                    try
                        Result.Pixel[ x+i, y ] := Color15to24( RunColor );
                    except
                        Application.MessageBox( PChar( 'Invalid Data, X=' + IntToStr(x) + ', Y=' + IntToStr(y) ), 'Invalid Data', 0 );
                    end;
                end;

            inc( X, RunLength );

        end until ( ( RunLength = 0 ) and ( XOffset = 0 ) );
    end;
end;

function TArtReader.GetTile( Id: Word ): TBitmap32;
begin
    Result := GetTile( Id, 0 );
end;

end.
