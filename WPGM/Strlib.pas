{:
PURPOSE AND IMPLEMENTATION:
	This unit defines some useful string handling routines.

HISTORY:
	??/??/96 - Created by Colin Patrick Sarsfield (colin.sarsfield@usa.net)
	11/08/96 - Left and Right functions added by CPS
	05/21/97 - IntlStrToFloat added by CPS

COPYRIGHT:
	Released into the public domain.
}
unit Strlib;

interface

uses
	Classes;

type
  CharSet = set of Char;

function Split(Source, Deli: string; StringList: TStringList): TStringList;
function BoolToStr(const b: Boolean): string;
function CompareStrNumMix(const str1, str2: String): Integer;
function DequoteString(s: String): String;
procedure DivideStrInt(const Str: string; var BegStr: string; var Int: Integer);
function DropTrailingZeros(const S: String): String;
procedure FileSplit(const Path: String; var Dir, Name, Ext: String);
function FormatAsIdentifier(const AString: String): String;
function FracToStr(const n, d: Integer): string;
function IntlStrToFloat(const AString: string): Single;
function IntToAlpha(const i: Longint): string;
function Left(const AString: string; i: Integer): string;
function LookupOrdStr(const s: String; const a: array of String): Integer;
function LTrim(const AString: string): string;
procedure MergeValues(A, B: TStringList);
function NextStrColumn(S: String; var Pos: Integer): String;
function PosAfter(SubStr, S: String; StartingAt: Integer): Integer;
function QtyList(A: TStringList): String;
function RepeatStr(const Str: String; const qty: Integer): String;
function Replace(const PTarget, BeforeStr, AfterStr: string): string;
function Right(const AString: string; i: Integer): string;
function RScan(const AString: string; chr: Char): Integer;
function RTrim(const AString: string): string;
function Scan(const AString: string; chr: Char): Integer;
{$IFNDEF WIN32}
procedure SetLength(var S: string; NewLength: Integer);
{$ENDIF}
procedure StreamToString(AStream: TStream; var AString: string);
function StringOfChar(const AChar: Char; Length: Integer): string;
procedure StringToStream(const AString: string; AStream: TStream);
function Trim(const AString: string): string;
function TrimChars(const AString: String; TrimChars: CharSet): string;

implementation

uses
  SysUtils;

{:Return a string representation for the boolean value b.  (e.g. True = 'True') }
function BoolToStr(const b: Boolean): string;
begin
  if b then
    Result := 'True'
  else
    Result := 'False';
end;

{:compare two strings.  Unlike a normal string comparison, digits 0..9 are seen
as a number with any other adjoining numbers and these numbers are compared as
whole numbers.  An example of a list properly sorted using this comparision
function:

AH-1
AH-2b
AH-12-15c
AH-31

This function is case-insensitive.}
function CompareStrNumMix(const str1, str2: String): Integer;
var
  w1, w2: String;
  p1, p2: Integer;
  isdigit1, isdigit2: Boolean;
const
  Digits = ['0'..'9'];

  procedure GetDigits(const source: String; var work: String; var pos: Integer);
  begin
    while (pos <= Length(source)) and (source[pos] in Digits) do begin
      work := work + source[pos];
      Inc(pos)
    end;
  end;
  
  procedure GetString(const source: String; var work: String; var pos: Integer);
  begin
    while (pos <= Length(source)) and not (source[pos] in Digits) do begin
      work := work + source[pos];
      Inc(pos)
    end;
  end;
begin
  { if str1 < str2 then negative
    if str1 > str2 then positive
    if str1 = str2 then zero }
  p1 := 1;
  p2 := 1;
  repeat
    { terminate with longer string being greater if end of either string is
      reached }
    if p1 > Length(str1) then begin Result := -1; Exit end;
    if p2 > Length(str2) then begin Result := 1; Exit end;
    w1 := '';
    w2 := '';

    isdigit1 := str1[p1] in Digits;
    isdigit2 := str2[p2] in Digits;
    { if different types of strings then exit }
    if isdigit1 <> isdigit2 then begin Result := Ord(isdigit1) - Ord(isdigit2); Exit end;
    if isdigit1 then begin
      { number portion }
      GetDigits(str1, w1, p1);
      GetDigits(str2, w2, p2);
      try
        Result := StrToInt(w1) - StrToInt(w2);
      except
        // if StrToInt fails, just compare the text normally
        Result := CompareText(w1, w2);
      end;
    end
    else begin
      // string portion
      GetString(str1, w1, p1);
      GetString(str2, w2, p2);
      Result := CompareText(w1, w2);
    end;
  until Result <> 0;
end;

{:Revert a quoted-style string to normal.  Leave other styles of strings
untouched.  For example,

"1"" pipe thread"

becomes

1" pipe thread
}
function DequoteString(s: String): String;
var
  Len: Integer;
begin
  Len := Length(s);
  if ((Copy(s, 1, 1) = '"') and (Copy(s, Len, 1) = '"')) then begin
    s := Copy(s, 2, Len - 2);
    s := Replace(s, '""', '"');
  end;
  Result := s;
end;

{:Assuming Str begins with non-numeric characters and ends with numeric
characters (i.e., 0..9), return the non-numeric portion in BegStr and convert
the numeric portion to an integer returning it in Int.}
procedure DivideStrInt(const Str: string; var BegStr: string; var Int: Integer);
var
	TStr: string;
	i: Integer;
begin
  for i := Length(Str) downto 1 do
    if (Str[i] < '0') or (Str[i] > '9') then
      Break;
  BegStr := Copy(Str, 1, i);
  if i = Length(Str) then
    Int := 0
  else begin
    TStr := Copy(Str, i + 1, Length(Str) - i);
    try { capture range check errors, etc. }
      Int := StrToInt(TStr);
    except
      Int := 0;
    end;
	end;
end;

{:Eliminates .00 from end of whole numbers.  (i.e., 10.00 = 10, 10.50 = 10.5,
10.05 = 10.05)  Looks through string for decimal.  If found, goes to the end
and looks back one character for a decimal or zero.  Continues that action
eliminating zeros and decimals until whole number is found to left of decimal.}
function DropTrailingZeros(const S: String): String;
var
  I: Integer;
  L: Integer;
begin
  Result := S;
  I := POS('.',Result);
  if I <> 0 then begin
    L := Length(Result);
    while Result[L] = '0' do
      dec(L);
    if Result[L] = '.' then
      dec(L);
    SetLength(Result,L)
  end;
end;

{:Split a file path "Path" into three parts:  its directory "Dir", its name
"Name", and its extension "Ext".}
procedure FileSplit(const Path: String; var Dir, Name, Ext: String);
var
	NamePos, ExtPos: Integer;
begin
	NamePos := RScan(Path, '\');
	if NamePos = 0 then NamePos := RScan(Path, ':');
	if NamePos = 0 then NamePos := 1 else Inc(NamePos);
	ExtPos := RScan(Path, '.');
	if ExtPos < NamePos then ExtPos := Length(Name);
	Dir := Copy(Path, 1, NamePos - 1);
	Name := Copy(Path, NamePos, ExtPos - NamePos);
	Ext := Copy(Path, ExtPos, Length(Path) - ExtPos + 1);
end;

{:Take a string and format it as an identifier (e.g.,
"Cond. fluid" = "CondFluid"
"24 times" = "Times"
"Cond. fluid_Water" = "CondFluid_Water"
}
function FormatAsIdentifier(const AString: String): String;
var
  i, Len: Integer;
  Cap: Boolean;
begin
  Len := Length(AString);
  Result := '';
  i := 1;
  // skip over junk at beginning
  while (i <= Len) and not (AString[i] in ['a'..'z', 'A'..'Z', '_']) do
    Inc(i);
  Cap := True;
  while i <= Len do begin
    if AString[i] = '_' then begin
      Result := Result + '_';
      Cap := True
    end
    else if AString[i] in ['a'..'z', 'A'..'Z', '0'..'9'] then begin
      if Cap then begin
        Result := Result + UpperCase(AString[i]);
        Cap := False;
      end
      else
        Result := Result + AString[i];
    end
    else
      Cap := True;
    Inc(i)
  end;
end;

{:Convert a fraction to its string representation where "n" is the numerator
and "d" is the denomiator.  Simplify the fraction if possible.  Fractions will
be displayed as an integer (optionally) followed by a fraction less than one
(optionally).  Note that this function only works for positive integers or zero. }
function FracToStr(const n, d: Integer): string;
var
  i, w, nn, dd: Integer;
begin
  // fail if n or d are negative
  if (n < 0) or (d < 0) then
    raise Exception.Create('FracToStr does not support negative integers');
  // calculate the whole number portion of the fraction
  w := n div d;
  // calculate the remaining numerator after the whole number is subtracted out
  nn := n mod d;
  // store a copy of the denominator for simplification
	dd := d;
  // check all possible divisors to simply
  for i := d - 1 downto 2 do
    // if the nominator and the denominator are both divisable by the same
    // number than divide by that number to simplify.
    if ((dd mod i) = 0) and ((nn mod i) = 0) then begin
      dd := dd div i;
      nn := nn div i;
    end;
  // if the whole number > 0 then represent it in the result
  if w > 0 then
    Result := Format('%d ', [w])
	else // otherwise blank out the result
    Result := '';
  // if the numerator is also zero than display zero
  if nn = 0 then begin
    if w = 0 then Result := '0';
  end
  else // otherwise add the string representation of the fraction to the result
    Result := Result + Format('%d/%d', [nn, dd]);
end;

{:This function will convert a String to a Single whether the decimal seperator
used is the localized (international) decimal seperator or the period (i.e., '.'). }
function IntlStrToFloat(const AString: string): Single;
var
  RCode: Integer;
begin
  try
    // StrToFloat recognizes the international decimal seperator (e.g., '.' in
    // USA, ',' in parts of Latin America)
    Result := StrToFloat(AString);
  except
    // Val always recognizes '.' as the decimal seperator
    Val(AString, Result, RCode);
		if RCode <> 0 then
			raise EConvertError.Create('Unable to convert string to number: ''' + AString + '');
  end;
end;

{:Convert an integer to a base 26 number based on the alphabet much like
column headings in most spreadsheet programs (e.g., A, B, C .., Z, AA, AB, etc.).}
function IntToAlpha(const i: Longint): string;
begin
	if i <= 26 then
    Result := Chr(Ord('A') + i - 1)
  else
    Result := IntToAlpha(i div 26) + Chr(Ord('A') + (i mod 26));
end;

{:This function returns a string "i" characters long starting at the left
(beginning) of "AString".  Note that if "AString" is less that "i" characters
long this function will return the whole string. }
function Left(const AString: string; i: Integer): string;
begin
  Result := Copy(AString, 1, i);
end;

{:This function returns an integer indicating the position of s in a.  Note that
this is often used to look up an ordinal based on its string representation;
hence its name.}
function LookupOrdStr(const s: String; const a: array of String): Integer;
var
  i: Integer;
begin
  for i := Low(a) to High(a) do
    if a[i] = s then begin
      Result := i;
      Exit;
    end;
  raise Exception.CreateFmt('Unable to lookup ordinal string value: "%s"', [s]);
end;

{:This function returns the remainder of "AString" after any leading spaces are
removed.}
function LTrim(const AString: string): string;
var
  i, b, l: Integer;
begin
  b := 1;
	l := Length(AString);
	for i := 1 to l do
		if AString[i] = ' ' then
      b := i + 1
    else
      Break;
  if b > l then
    Result := ''
  else
		Result := Copy(AString, b, l - b + 1)
end;

{:This procedure takes two TStringList's populated with lines like "coil=true"
(reference the "Values" property in the help) and merges them.  The result is
placed in A.  Note that for any strings that both A & B have B have the part to
the left of the equals sign in common the merged list will use the line from B.}
procedure MergeValues(A, B: TStringList);
var
	i, p: Integer;
begin
	for i := 0 to Pred(B.Count) do begin
		p := Pos('=', B[i]);
		if (p <> 0) then
			A.Values[Copy(B[i], 1, p - 1)] := Copy(B[i], p + 1, Length(B[i]) - p);
	end;
end;

{:This function extracts the next "column" out of "S" which should a line from
either a space or tab delimited text file.  Note that Pos should be 1 on the
first call to this function for a given string and the Pos returned by this
function should be used in subsequent calls.}
function NextStrColumn(S: String; var Pos: Integer): String;
var
  Start, Len: Integer;

  function IsSpace(C: Char): Boolean;
  begin
    Result := C in [' ', #9];
  end;
begin
  Len := Length(S);
  while IsSpace(S[Pos]) and (Pos <= Len) do Inc(Pos);
  if Pos > Len then begin Result := ''; Exit end;
  Start := Pos;
  while not IsSpace(S[Pos]) and (Pos <= Len) do Inc(Pos);
  Result := Copy(S, Start, Pos - Start);
end;

{:This function is just like the "Pos" function that is provided with Delphi
except it accepts an additional argument, "StartingAt".  This argument tells
this function where to start its search whereas the "Pos" function always starts
at position 1.}
function PosAfter(SubStr, S: String; StartingAt: Integer): Integer;
begin
	if StartingAt < 1 then StartingAt := 1;
	Delete(S, 1, StartingAt - 1);
	Result := Pos(SubStr, S);
	if Result <> 0 then
		Result := Result + StartingAt - 1;
end;

{:this function takes a TStringList and formats it as a "quantity list" of the
string list's values.  For example:
The StringList 'A','B','A','B','B','C'
becomes '(2)A/(3)B/(1)C'
If the string list contains only one item it is considered a special case and
that item is returned without any change in formatting.
Note: the string list is sorted by list function}
function QtyList(A: TStringList): String;
var
  i, qty: Integer;
  str: String;
begin
  if A.Count = 1 then begin
    Result := A[0];
    Exit;
  end;
  A.Sort;
  i := 0;
  Result := '';
  while i < A.Count do begin
    str := A[i];
    qty := 1;
    while (i < Pred(A.Count)) and (A[i + 1] = str) do begin
      Inc(qty);
      Inc(i);
    end;
    if Result <> '' then Result := Result + '/';
    Result := Result + Format('(%d)%s', [qty, str]);
    Inc(i)
  end;
end;

{:This function creates a string that is the concatenation of "qty" instances of
the string "Str".}
function RepeatStr(const Str: String; const qty: Integer): String;
var
  i: Integer;
begin
  if qty > 0 then begin
    Result := Str;
    for i := 2 to qty do
      Result := Result + Str;
  end
  else
    Result := '';
end;

{:This function returns a modified version of "PTarget" where all instances of
"BeforeStr" have been replaced by "AfterStr".}
function Replace(const PTarget, BeforeStr, AfterStr: string): string;
var
	i, j: Integer;
begin
	Result := PTarget;
	{ start at beginning of string }
  i := 1;
  while True do begin
    j := i;
    i := Pos(BeforeStr, Copy(Result, i, Length(Result) - i + 1));
    if i = 0 then
      Break;
    Inc(i, j - 1);
    Delete(Result, i, Length(BeforeStr));
    Insert(AfterStr, Result, i);
    { adjust i to correct position }
    i := i + Length(AfterStr);
  end;
  Result := Result;
end;

{:This function returns a string "i" characters long starting at the right
(end) of "AString".  Note that if "AString" is less that "i" characters
long this function will return the whole string. }
function Right(const AString: string; i: Integer): string;
begin
  if i > Length(AString) then
    Result := AString
  else
    Result := Copy(AString, Length(AString) - i + 1, i);
end;

{:This function returns the index of the last instance of "chr" in "AString".
It returns 0 if none was found.}
function RScan(const AString: string; chr: Char): Integer;
var
	i: Integer;
begin
	Result := 0;
	for i := Length(AString) downto 1 do
		if AString[i] = chr then begin
			Result := i;
			Exit;
		end;
end;

{:This function returns the remainder of "AString" after any trailing spaces are
removed.}
function RTrim(const AString: string): string;
var
  i, e: Integer;
begin
  e := Length(AString);
  for i := Length(AString) downto 1 do
    if AString[i] = ' ' then
      e := i - 1
    else
      Break;
  if 1 > e then
    Result := ''
  else
    Result := Copy(AString, 1, e)
end;

{:This function returns the index of the first instance of "chr" in "AString".
It returns 0 if none was found.}
function Scan(const AString: string; chr: Char): Integer;
var
	i: Integer;
begin
	Result := 0;
	for i := 1 to Length(AString) do
		if AString[i] = chr then begin
			Result := i;
			Exit;
		end;
end;

{$IFNDEF WIN32}

{:This function is to enable portability between 16-bit and 32-bit programs.
Since 32-bit Delphi defines a SetLength function, this defines one for 16-bit
programs.}
procedure SetLength(var S: string; NewLength: Integer);
begin
	S[0] := Char(NewLength);
end;
{$ENDIF}

{:This procedure duplicates the contents of "AStream" and places it in "AString".}
procedure StreamToString(AStream: TStream; var AString: string);
var
  L: Integer;
begin
  AStream.Read(L, Sizeof(Integer));
  SetLength(AString, L);
  AStream.Read(AString[1], L);
end;

{:This function returns a string "Length" characters long where each character
is "AChar".}
function StringOfChar(const AChar: Char; Length: Integer): string;
begin
  SetLength(Result, Length);
  FillChar(Result[1], Length, AChar);
end;

{:This procedure writes the contents of "AString" to "AStream".}
procedure StringToStream(const AString: string; AStream: TStream);
var
  L: Integer;
begin
  L := Length(AString);
  AStream.Write(L, Sizeof(Integer));
  AStream.Write(AString[1], L);
end;

{:This function returns the remainder of "AString" after any leading or trailing
spaces are removed.}
function Trim(const AString: string): string;
var
  i, b, e: Integer;
begin
  b := 1;
  e := Length(AString);
  for i := 1 to e do
    if AString[i] = ' ' then
      b := i + 1
    else
      Break;
  for i := Length(AString) downto b do
    if AString[i] = ' ' then
      e := i - 1
    else
      Break;
  if b > e then
    Result := ''
  else
    Result := Copy(AString, b, e - b + 1)
end;

function TrimChars(const AString: string; TrimChars: CharSet): string;
var
  i, b, e: Integer;
begin
  b := 1;
  e := Length(AString);
  for i := 1 to e do
    if AString[i] in TrimChars then
      b := i + 1
    else
      Break;
  for i := Length(AString) downto b do
    if AString[i] in TrimChars then
      e := i - 1
    else
      Break;
  if b > e then
    Result := ''
  else
    Result := Copy(AString, b, e - b + 1)
end;

function Split(Source, Deli: string; StringList: TStringList): TStringList;
var
   EndOfCurrentString: byte;
begin
    repeat
        EndOfCurrentString := Pos(Deli, Source);
        if EndOfCurrentString = 0 then
            StringList.add(Source)
        else
            StringList.add(Copy(Source, 1, EndOfCurrentString - 1));
        Source := Copy(Source, EndOfCurrentString + length(Deli), length(Source) - EndOfCurrentString);
    until EndOfCurrentString = 0;
    result := StringList;
end;

end.

