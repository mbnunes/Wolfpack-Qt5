unit UOTiledata;

interface

type TLandTiledata = packed record
	flags: Cardinal;
    texture: Word;
    name: array[0..19] of Char;
end;

type TItemTiledata = packed record
	flags: Cardinal;
    weight: Byte;
    layer: Byte;
    unknown1: word;
    unknown2: byte;
    quantity: byte;
    animation: word;
    unknown3: byte;
    hue: byte;
    unknown4: byte;
    unknown5: byte;
    height: byte;
    name: array[0..19] of Char;
end;

type PItemTiledata = ^TItemTiledata;

type TTiledataReader = class( TObject )
	private
		StaticTiles: Array[0..$3FFF] of TItemTiledata;

	public
    	constructor Create;
    	destructor Destroy; override;

        function Open( FileName: String ): Boolean;
        function getItemName( id: Word ): String;
end;

implementation

uses Classes, SysUtils;

constructor TTiledataReader.Create;
begin
end;

destructor TTiledataReader.Destroy;
begin
end;

function TTiledataReader.Open( FileName: String ): Boolean;
var
	Input: TFileStream;
    i,j: Cardinal;
begin
  Result := False;
  Input  := Nil;
  try
   	Input := TFileStream.Create( FileName, fmOpenRead+fmShareDenyNone );

    // Skip Land Tiles (for now)
    Input.Seek( 512 * ( 4 + ( 32 * sizeof( TLandTiledata ) ) ), soFromBeginning );

    // Read all blocks
    for i := 0 to 511 do
    begin
      Input.Seek( 4, soFromCurrent );

      for j := 0 to 31 do
      begin
        Input.Read( StaticTiles[ ( i * 32 ) + j ], sizeof( TItemTiledata ) );
      end;
    end;

    Input.Free;
    Result := True;
  except
    Input.Free;
  end;    
end;

function TTiledataReader.getItemName( id: Word ): String;
begin
    Assert( id <= $3fff );

    Result := StaticTiles[ id ].name;
end;

end.
