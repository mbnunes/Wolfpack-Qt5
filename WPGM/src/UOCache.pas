unit UOCache;

interface

uses Classes;

(*
	TCache
    A utility class for caching objects.
*)
type TCache = class( TObject )
	private
    	Data: TList;
        Size: Cardinal;
        
    public
    	constructor Create( Size: Cardinal );
        destructor Destroy; override;

        procedure Add( Key: Cardinal; Data: Pointer );
        function Lookup( Key: Cardinal ): Pointer;
end;

implementation

uses IniFiles;

type TPrivateCacheObject = record
	Key: Cardinal;
    Data: Pointer;
end;

type PPrivateCacheObject = ^TPrivateCacheObject;

constructor TCache.Create( Size: Cardinal );
begin
	Data := TList.Create;
    Self.Size := Size;
end;

destructor TCache.Destroy;
var
	i: Integer;
begin
	  for i := 0 to Data.Count - 1 do
    begin
    	Dispose( PPrivateCacheObject( Data[i] ).Data );
	    Dispose( Data[i] );
    end;
    Data.Free;
end;

procedure TCache.Add( Key: Cardinal; Data: Pointer );
var
	CacheObject: PPrivateCacheObject;
begin
	new( CacheObject );
    CacheObject.Key := Key;
    CacheObject.Data := Data;

    if ( Self.Data.Count >= Integer( Self.Size ) ) and ( Self.Size > 0 ) then
    begin
		Dispose( PPrivateCacheObject( Self.Data[0] ).Data );
        Dispose( Self.Data[0] );
        Self.Data.Delete( 0 );
    end;

    Self.Data.Add( CacheObject );
end;

function TCache.Lookup( Key: Cardinal ): Pointer;
var
	I: Integer;
begin
	Result := nil;

	for I := 0 to ( Data.Count - 1 ) do
    begin
    	if PPrivateCacheObject( Data[i] ).Key = Key then
        begin
        	Result := PPrivateCacheObject( Data[i] ).Data;
            Exit;
        end;
    end;
end;

end.
 