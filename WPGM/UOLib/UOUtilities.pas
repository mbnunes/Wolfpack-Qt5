unit UOUtilities;

interface

uses Graphics, GR32, Classes;

function Color15to24( Color: Word ): TColor;
function Color15to32( Color: Word ): TColor32;
function SendCommand( Command: String; Activate: Boolean = False ): Boolean;
function GetUoPath: String;
procedure Split(const Delimiter: Char; Input: string; const Strings: TStrings);

implementation

uses Windows, Forms, Messages, Registry, SysUtils, Main, uConfig;

procedure Split
   (const Delimiter: Char;
    Input: string;
    const Strings: TStrings) ;
begin
   Assert(Assigned(Strings)) ;
   Strings.Clear;
   Strings.Delimiter := Delimiter;
   Strings.DelimitedText := Input;
end;

function Color15to32( Color: Word ): TColor32;
begin
	Result := Color32( ((Color shr 10) and $1F) * 8,
                ((Color shr 5) and $1F) * 8,
                  ((Color and $1F) * 8),
                  255);
end;

function Color15to24( Color: Word ): TColor;
begin
	Result := TColor( ( ( (Color shr 10 ) and $1F ) * 8 shl 16 ) or
                ( ( ( Color shr 5  ) and $1F ) * 8  shl 8 ) or
                  ( Color and $1F) * 8 );
end;

function SendCommand(Command: String; Activate: Boolean): Boolean;
var
	Window: HWND;
    PStr: PChar;
    i: Integer;
  Prefix: String;
begin
  Prefix := Main.Config.getString('Command Prefix', '''');
  Command := Prefix + Command;

  Result := False;
  Window := FindWindow('Ultima Online', nil);

  if Window = 0 then
  begin
    Application.MessageBox(PChar('Please start the Ultima Online client before sending any commands.' + #13 + 'Ignored Command: ' + Command ), 'Ultima Online Error', MB_OK+MB_ICONWARNING);
    exit;
  end;

  PStr := PChar(Command);

  SendMessage(Window, WM_CHAR, 13, 0);

  for i := 0 to Length( Command ) - 1 do
  begin
    PostMessage(Window, WM_CHAR, Integer( PStr[i] ), 0);
  end;

  PostMessage(Window, WM_CHAR, 13, 0);

  if Activate then
  begin
    BringWindowToTop(Window);
    SetFocus(Window);
  end;
end;

function GetUoPath: String;
var
	Registry: TRegistry;
begin
  Registry := TRegistry.Create;
  Result := '';

  Registry.RootKey := HKEY_LOCAL_MACHINE;
  if not Registry.OpenKey( 'SOFTWARE\Origin Worlds Online\Ultima Online\1.0', False ) then
  begin
    Registry.Free;
  exit;
  end;

  if not Registry.ValueExists( 'ExePath' ) then
  begin
    Registry.Free;
    exit;
  end;

  Result := ExtractFilePath( Registry.ReadString( 'ExePath' ) );

  Registry.Free;
end;

end.
