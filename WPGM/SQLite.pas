unit SQLite;

interface

type TSQLiteDB     = Pointer;
type TSQLiteResult = ^PChar;

function  SQLite_Open           (dbname: PChar; mode: Integer; var ErrMsg: PChar): TSQLiteDB; cdecl; external 'sqlite.dll' name 'sqlite_open';
procedure SQLite_Close          (db: TSQLiteDB); cdecl; external 'sqlite.dll' name 'sqlite_close';
function  SQLite_Exec           (db: TSQLiteDB; SQLStatement: PChar; CallbackPtr: Pointer; Sender: TObject; var ErrMsg: PChar): integer; cdecl; external 'sqlite.dll' name 'sqlite_exec';
function  SQLite_Version        (): PChar; cdecl; external 'sqlite.dll' name 'sqlite_libversion';
function  SQLite_Encoding       (): PChar; cdecl; external 'sqlite.dll' name 'sqlite_libencoding';
function  SQLite_ErrorString    (ErrNo: Integer): PChar; cdecl; external 'sqlite.dll' name 'sqlite_error_string';
function  SQLite_GetTable       (db: TSQLiteDB; SQLStatement: PChar; var ResultPtr: TSQLiteResult; var RowCount: Cardinal; var ColCount: Cardinal; var ErrMsg: PChar): integer; cdecl; external 'sqlite.dll' name 'sqlite_get_table';
procedure SQLite_FreeTable      (Table:TSQLiteResult ); cdecl; external 'sqlite.dll' name 'sqlite_free_table';
procedure SQLite_FreeMem        (P: PChar); cdecl; external 'sqlite.dll' name 'sqlite_freemem';
function  SQLite_Complete       (P: PChar): boolean; cdecl; external 'sqlite.dll' name 'sqlite_complete';
function  SQLite_LastInsertRowID(db: TSQLiteDB): integer; cdecl; external 'sqlite.dll' name 'sqlite_last_insert_rowid';
procedure SQLite_Cancel         (db: TSQLiteDB); cdecl; external 'sqlite.dll' name 'sqlite_interrupt';
procedure SQLite_BusyHandler    (db: TSQLiteDB; CallbackPtr: Pointer; Sender: TObject); cdecl; external 'sqlite.dll' name'sqlite_busy_handler' ;
procedure SQLite_BusyTimeout    (db: TSQLiteDB; TimeOut: integer); cdecl; external 'sqlite.dll' name 'sqlite_busy_timeout';
function  SQLite_Changes        (db: TSQLiteDB): integer; cdecl; external 'sqlite.dll' name 'sqlite_changes';

const
  SQLITE_OK         =  0;   // Successful result
  SQLITE_ERROR      =  1;   // SQL error or missing database
  SQLITE_INTERNAL   =  2;   // An internal logic error in SQLite
  SQLITE_PERM       =  3;   // Access permission denied
  SQLITE_ABORT      =  4;   // Callback routine requested an abort
  SQLITE_BUSY       =  5;   // The database file is locked
  SQLITE_LOCKED     =  6;   // A table in the database is locked
  SQLITE_NOMEM      =  7;   // A malloc() failed
  SQLITE_READONLY   =  8;   // Attempt to write a readonly database
  SQLITE_INTERRUPT  =  9;   // Operation terminated by sqlite_interrupt()
  SQLITE_IOERR      = 10;   // Some kind of disk I/O error occurred
  SQLITE_CORRUPT    = 11;   // The database disk image is malformed
  SQLITE_NOTFOUND   = 12;   // (Internal Only) Table or record not found
  SQLITE_FULL       = 13;   // Insertion failed because database is full
  SQLITE_CANTOPEN   = 14;   // Unable to open the database file
  SQLITE_PROTOCOL   = 15;   // Database lock protocol error
  SQLITE_EMPTY      = 16;   // (Internal Only) Database table is empty
  SQLITE_SCHEMA     = 17;   // The database schema changed
  SQLITE_TOOBIG     = 18;   // Too much data for one row of a table
  SQLITE_CONSTRAINT = 19;   // Abort due to contraint violation
  SQLITE_MISMATCH   = 20;   // Data type mismatch
  SQLITE_MISUSE     = 21;   // Library used incorrectly
  SQLITE_NOLFS      = 22;   // Uses OS features not supported on host
  SQLITE_AUTH       = 23;   // Authorization denied
  SQLITE_FORMAT     = 24;   // Auxiliary database format error
  SQLITE_ROW        = 100;  // sqlite_step() has another row ready
  SQLITE_DONE       = 101;  // sqlite_step() has finished executing


implementation

end.
