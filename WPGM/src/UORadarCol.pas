unit UORadarCol;

interface

uses Classes, Graphics;

type TRadarColReader = class( TObject )
	private
		Colors: Array[0..$7FFF] of TColor;

	public
    	constructor Create; overload;
        function Load( FileName: String ): Boolean;

        function GetLandColor( Id: Word ): TColor;
        function GetItemColor( Id: Word ): TColor;
end;

implementation

uses Windows, SysUtils, UOUtilities;

constructor TRadarColReader.Create;
begin
	ZeroMemory( @Colors, $8000 * sizeof( TColor ) );
end;

function TRadarColReader.Load( FileName: String ): Boolean;
var
	Input: TFileStream;
    i: Integer;
begin
	Result := False;

	try
   	Input := TFileStream.Create( FileName, fmOpenRead+fmShareDenyNone );

		for i := 0 to $7FFF do
        begin
			Input.Read( Colors[i], 2 );
            Colors[i] := Color15to24( Colors[i] );
        end;

        Input.Free;
        Result := True;
    except
    end;
end;

function TRadarColReader.GetLandColor( Id: Word ): TColor;
begin
	if Id >= $7FFF then
    	Result := clBlack
    else
    	Result := Colors[ Id ];
end;

function TRadarColReader.GetItemColor( Id: Word ): TColor;
begin
	Result := GetLandColor($4000 + Id);
end;

end.
