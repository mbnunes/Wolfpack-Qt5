unit uConfig;

interface

uses IniFiles, Registry;

type TConfig = class
	private
    IniCache: TMemIniFile;

  public
    constructor Create;
    destructor Destroy; override;
    function Load( FileName: String ): Boolean;
    function Save: Boolean;
    procedure setString(Name: String; Value: String);
    function getString(Name: String; Default: String): String;
    procedure setBool(Name: String; Value: Boolean);
    function getBool(Name: String; Default: Boolean): Boolean;
    procedure setInt(Name: String; Value: Integer);
    function getInt(Name: String; Default: Integer): Integer;
end;

implementation

uses SysUtils, UOUtilities, Forms, Windows;

constructor TConfig.Create;
begin
	inherited Create;

	IniCache := nil;
end;

destructor TConfig.Destroy;
begin
	IniCache.Free;
  inherited Destroy;
end;

function TConfig.Load( FileName: String ): Boolean;
begin
	Result := False;

  try
    IniCache := TMemIniFile.Create(FileName);
  except
    FreeAndNil( IniCache );
  end;
end;

function TConfig.Save: Boolean;
begin
	Result := false;

	if IniCache = nil then
    	exit;

  try
    IniCache.UpdateFile;
  except
  end;
end;

procedure TConfig.setString(Name: String; Value: String);
begin
  IniCache.WriteString('General', Name, Value);
end;

function TConfig.getString(Name: String; Default: String): String;
begin
  if not IniCache.ValueExists('General', Name) then
    IniCache.WriteString('General', Name, Default);

  Result := IniCache.ReadString('General', Name, Default);
end;

procedure TConfig.setBool(Name: String; Value: Boolean);
begin
  IniCache.WriteBool('General', Name, Value);
end;

function TConfig.getBool(Name: String; Default: Boolean): Boolean;
begin
  if not IniCache.ValueExists('General', Name) then
    IniCache.WriteBool('Geneal', Name, Default);

  Result := IniCache.ReadBool('General', Name, Default);
end;

procedure TConfig.setInt(Name: String; Value: Integer);
begin
  IniCache.WriteInteger('General', Name, Value);
end;

function TConfig.getInt(Name: String; Default: Integer): Integer;
begin
  if not IniCache.ValueExists('General', Name) then
    IniCache.WriteInteger('General', Name, Default);

  Result := IniCache.ReadInteger('General', Name, Default);
end;

end.
