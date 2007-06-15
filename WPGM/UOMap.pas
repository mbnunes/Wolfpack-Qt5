unit UOMap;

interface

uses Classes, UOCache;

type TMapCell = packed record
	Id: Word;
    Height: Shortint;
end;

type TMapBlock = packed record
	Header: Cardinal;
    Cells: Array[0..63] of TMapCell;
end;

type TMapReader = class( TObject )
	private
    Input: TFileStream;
    Height, Width: Word;
    Cache: TCache;
    Disabled: Boolean;
    LastModifyTime: TDateTime;

	public
    function getHeight: Word;
    function getWidth: Word;
    constructor Create;
    destructor Destroy; override;

    function Open( FileName: String; MapWidth: Word = 0; MapHeight: Word = 0 ): Boolean;
    procedure Close;

    procedure ReadBlock( XBlock: Word; YBlock: Word; var MapBlock: TMapBlock );
    procedure ReadCell( X: Word; Y: Word; var MapCell: TMapCell );
    function LastModified(): TDateTime;
    //procedure WriteBlock( X: Word, Y: Word; MapBlock: TMapBlock );
    //procedure WriteCell( X: Word; Y: Word; MapCell: TMapCell );
end;

implementation

uses Windows, Dialogs, SysUtils;

function TMapReader.getWidth: Word;
begin
  Result := Width;
end;

function TMapReader.getHeight: Word;
begin
  Result := Height;
end;

constructor TMapReader.Create;
begin
  Input := nil;
  Width := 0;
  Height := 0;
  Cache := TCache.Create( 100 ); // 100 Blocks
  Disabled := True;
  LastModifyTime := GetTime;  
end;

destructor TMapReader.Destroy;
begin
	Input.Free;
	Cache.Free;
end;

function TMapReader.Open( FileName: String; MapWidth: Word; MapHeight: Word ): Boolean;
begin
	Result := False;
  Input := nil;

	try
    Input := TFileStream.Create( FileName, fmOpenRead+fmShareDenyNone );
    Result := True;

    // Measure Map Height
    Height := MapHeight;
    Width := MapWidth;

    if Height = 0 then case Input.Size of
    77070336:
      begin
        Width := 768;
        Height := 512;
        Disabled := False;
      end;

    11289600:
      begin
        Width := 288;
        Height := 200;
        Disabled := False;
      end;

    16056320:
      begin
        Width := 320;
        Height := 256;
        Disabled := False;
      end;

    6421156:
      begin
        Width := 181;
        Height := 181;
        Disabled := False;
      end;
    end;

    // Try to read the modify time
    LastModifyTime := FileDateToDateTime(FileAge(FileName));
  except
    Input.Free;
  end;
end;

procedure TMapReader.Close;
begin
	Input.Free;
    Input := nil;
end;

procedure TMapReader.ReadBlock(XBlock: Word; YBlock: Word; var MapBlock: TMapBlock);
var
  BlockId: Cardinal;
  CacheEntry: ^TMapBlock;
begin
  if Disabled then begin
    ZeroMemory(@MapBlock, sizeof(TMapBlock));
    exit;
  end;

  BlockId := XBlock * Height + YBlock;

  // See if the block is cached
  CacheEntry := Cache.Lookup(BlockId);

  if CacheEntry <> nil then begin
    MapBlock := CacheEntry^;
    exit;
  end;

  ZeroMemory(@MapBlock, sizeof(TMapBlock));
  if (Input = nil) or (Height = 0) then
    exit;

  new(CacheEntry);
  Input.Seek( BlockId * sizeof( TMapBlock ), soFromBeginning );
  Input.Read(CacheEntry^, sizeof(TMapBlock));
  MapBlock := CacheEntry^;
  Cache.Add(BlockId, CacheEntry);  
end;

procedure TMapReader.ReadCell( X: Word; Y: Word; var MapCell: TMapCell );
var
	Block: TMapBlock;
begin
	ZeroMemory( @MapCell, sizeof( TMapCell ) );

    if( Input = nil ) or ( Height = 0 ) then
    	exit;

    ReadBlock( X div 8, Y div 8, Block );

	MapCell := Block.Cells[ ( ( Y mod 8 ) * 8 ) + ( X mod 8 ) ];   
end;

function TMapReader.LastModified(): TDateTime;
begin
  Result := LastModifyTime;
end;

end.
