unit SQLiteTable;

{
  Simple classes for using SQLite's exec and get_table.

  TSQLiteDatabase wraps the calls to opend and close an SQLite database.
  It also wraps SQLite_exec for queries that do not return a result set

  TSQLiteTable wraps sqlite_get_table.
  It allows accessing fields by name as well as index and can step through a
  result set with the Next procedure.

  Created by Pablo Pissanetzky (pablo@myhtpc.net)
}

interface

uses
  SQLite , Classes;

type

  TSQLiteTable = class;

  TSQLiteDatabase = class
  private
    fDB : TSQLiteDB;
  public
    constructor Create( const FileName : string );
    destructor Destroy; override;
    function GetTable( const SQL : string ) : TSQLiteTable;
    procedure ExecSQL( const SQL : string );
  end;

  TSQLiteTable = class
  private
    fTable    : TSQLiteResult;
    fRecord   : TSQLiteResult;
    fRowCount : Cardinal;
    fColCount : Cardinal;
    fCols     : TStringList;
    fRow      : Cardinal;
    function GetFields(I: Integer): PChar;
    function GetEOF: Boolean;
    function GetColumns(I: Integer): string;
    function GetFieldByName(FieldName: string): PChar;
    function GetCount: Integer;
  public
    constructor Create( DB : TSQLiteDatabase; const SQL : string );
    destructor Destroy; override;
    property EOF : Boolean read GetEOF;
    property Fields[ I : Integer ] : PChar read GetFields;
    property FieldByName[ FieldName : string ] : PChar read GetFieldByName;
    property Columns[ I : Integer ] : string read GetColumns;
    property ColCount : Cardinal read fColCount;
    property RowCount : Cardinal read fRowCount;
    property Row : Cardinal read fRow;
    procedure Next;

    // The property Count is used when you execute count(*) queries.
    // It returns 0 if the result set is empty or the value of the
    // first field as an integer.
    property Count : Integer read GetCount;
  end;

implementation

uses
  SysUtils;

//------------------------------------------------------------------------------
// TSQLiteDatabase
//------------------------------------------------------------------------------

constructor TSQLiteDatabase.Create(const FileName: string);
var
  Msg : PChar;
begin
  inherited Create;

  Msg := nil;
  try

    fDB := SQLite_Open( PChar( FileName ) , 0 , Msg );

    if not Assigned( fDB ) then
      raise Exception.CreateFmt( 'Failed to open database "%s" : %s' , [ FileName , Msg ] );

  finally
    if Assigned( Msg ) then
      SQLite_FreeMem( Msg );
  end;
end;

//..............................................................................

destructor TSQLiteDatabase.Destroy;
begin
  if Assigned( fDB ) then
    SQLite_Close( fDB );
  inherited;
end;

//..............................................................................

procedure TSQLiteDatabase.ExecSQL(const SQL: string);
var
  Msg : PChar;
begin
  Msg := nil;
  try
    if SQLite_Exec( fDB , PChar( SQL ) , nil , nil , Msg ) <> SQLITE_OK then
      raise Exception.CreateFmt( 'Error executing SQL "%s" : %s' , [ SQL , Msg ] );
  finally
    if Assigned( Msg ) then
      SQLite_FreeMem( Msg );
  end;
end;

//..............................................................................

function TSQLiteDatabase.GetTable(const SQL: string): TSQLiteTable;
begin
  Result := TSQLiteTable.Create( Self , SQL );
end;

//------------------------------------------------------------------------------
// TSQLiteTable
//------------------------------------------------------------------------------

constructor TSQLiteTable.Create(DB: TSQLiteDatabase; const SQL: string);
var
  Msg : PChar;
  I   : Integer;
begin
  fTable  := nil;
  Msg     := nil;
  try
    if SQLite_GetTable( DB.fDB , PChar( SQL ) , fTable , fRowCount , fColCount , Msg ) <> SQLITE_OK then
      raise Exception.CreateFmt( 'Error executing SQL "%s" : %s' , [ SQL , Msg ] );

    fRecord := fTable;

    fCols := TStringList.Create;

    if fRowCount > 0 then
      begin
        for I := 0 to Pred( fColCount ) do
          begin
            fCols.Add( fRecord^ );
            Inc( fRecord );
          end;
      end;

    fRow := 0;

  finally
    if Assigned( Msg ) then
      SQLite_FreeMem( Msg );
  end;
end;

//..............................................................................

destructor TSQLiteTable.Destroy;
begin
  if Assigned( fTable ) then
    SQLite_FreeTable( fTable );
  if Assigned( fCols ) then
    fCols.Free;
  inherited;
end;

//..............................................................................

function TSQLiteTable.GetColumns(I: Integer): string;
begin
  Result := fCols[ I ];
end;

//..............................................................................

function TSQLiteTable.GetCount: Integer;
begin
  if not EOF then
    Result := StrToInt( Fields[ 0 ] )
  else
    Result := 0;
end;

//..............................................................................

function TSQLiteTable.GetEOF: Boolean;
begin
  Result := fRow >= fRowCount;
end;

//..............................................................................

function TSQLiteTable.GetFieldByName(FieldName: string): PChar;
begin
  Result := GetFields( fCols.IndexOf( FieldName ) );
end;

//..............................................................................

function TSQLiteTable.GetFields(I: Integer): PChar;
var
  P : TSQLiteResult;
begin
  Result := nil;

  if not EOF then
    begin
      P := fRecord;
      Inc( P , I );
      Result := P^;
    end;
end;

//..............................................................................

procedure TSQLiteTable.Next;
begin
  if not EOF then
    begin
      Inc( fRecord , fColCount );
      Inc( fRow );
    end;
end;


end.
