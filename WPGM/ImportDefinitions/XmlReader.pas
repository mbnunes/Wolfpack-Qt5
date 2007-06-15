{:
PURPOSE AND IMPLEMENTATION:
	This unit defines classes for parsing a well-formed standalone XML document in
  to a tree of nodes describing the data therein.  These classes only support a
  subset of the XML 1.0 recommendation and have not been analyzed for
  conformance with the recommendation.  This module also only supports 8-bit
  character sets and makes no special allowances for UTF-8.  Nor has any support
  for the "<!DOCTYPE" node been implemented.  Parsing of the XML declaration
  lazily uses the attribute parsing code which is not correct (but works).  This
  module also freely trims element whitespace which is not always correct.

  Q:  Why use this module instead of Microsoft's XML parser or the OpenXML
      project?

  A:  I wrote this module to be free of dependencies on outside libraries and to
      be small and fast.  Neither of the above libraries met that criteria.  I
      didn't like the idea of requiring my users to install Microsoft's XML
      library (and the potential compatibility problems that arise).  Nor did I
      like the idea of adding 135 kb (OpenXML) to my executable just to load files
      in XML format.  Instead this unit only uses about 7 kb.

  This was written for Delphi 5.  It may work for other versions, but does
  require method overloading which was introduced in Delphi 4.

HISTORY:
	09/12/00 - Created by Colin Patrick Sarsfield

DISCLAIMER:
  Use at your own risk.

COPYRIGHT:
	Released into the public domain by Colin Sarsfield 9/13/00
}
unit XMLReader;

interface

uses
  Classes;

type
  CharSet = set of Char;

  TXMLNode = class;

  TXMLNodeType = (xntDocument, xntElement, xntAttribute, xntProcessingInstruction,
      xntXMLDeclaration);

  TXMLReader = class
  private
    CurChar: Char;
    function ParseAttribute(Node: TXMLNode): Boolean;
    function ParseAttValue(var AttValue: String): Boolean;
    function ParseCDSect(Node: TXMLNode): Boolean;
    function ParseCharData(Node: TXMLNode): Boolean;
    function ParseComment: Boolean;
    procedure ParseContent(Node: TXMLNode);
    procedure ParseDocTypeDecl(Node: TXMLNode);
    function ParseElement(Node: TXMLNode): Boolean;
    function ParseEq: Boolean;
    function ParseETag(Node: TXMLNode): Boolean;
    function ParseMisc(Node: TXMLNode): Boolean;
    function ParseName(var Name: String): Boolean;
    function ParseProcessingInstruction(Node: TXMLNode): Boolean;
    function ParseProlog(Node: TXMLNode): Boolean;
    function ParseReference: Char;
    function ParseWhitespace: Boolean;
    procedure ParseXMLDecl(Node: TXMLNode);
    procedure ReportError(Err: String);
  protected
    function DoesStreamMatchString(Str: String): Boolean;
    function EndOfData: Boolean; virtual; abstract;
    procedure FreeState(State: TObject); virtual; abstract;
    procedure ReadChar; virtual; abstract;
    procedure ReadCharAndCheckForEnd;
    procedure RestoreState(State: TObject); virtual; abstract;
    function SaveState: TObject; virtual; abstract;
  public
    function ParseDocument: TXMLNode;
  end;

  TXMLStringReader = class(TXMLReader)
  private
    FData: String;
    FPosition: Integer;
  protected
    function EndOfData: Boolean; override;
    procedure FreeState(State: TObject); override;
    procedure ReadChar; override;
    procedure RestoreState(State: TObject); override;
    function SaveState: TObject; override;
  public
    constructor Create(AData: String);
  end;

  TXMLNode = class
  private
    FData: String;
    FName: String;
    FNodes: TList;
    FNodeType: TXMLNodeType;
    function GetNodeCount: Integer;
    function GetNodes(Index: Integer): TXMLNode;
    function GetTotalNodeCount: Integer;
    function GetAsInteger: Integer;
    function GetAsSingle: Single;
    function GetAsDateTime: TDateTime;
  public
    constructor Create; virtual;
    destructor Destroy; override;

    function AddNode(Node: TXMLNode): Integer;
    function Description: String;
    function FindNode(const Name: String; const Index: Integer): TXMLNode; overload;
    function FindNode(const Name: String): TXMLNode; overload;
    function FindNodeExc(const Name: String): TXMLNode;
    function HasNode(const Name: String): Boolean;
    procedure LookupBasicData(const Name: String; var LData: String); overload;
    procedure LookupBasicData(const Name: String; var LData: TDateTime); overload;
    procedure LookupBasicData(const Name: String; var LData: Integer); overload;
    procedure LookupBasicData(const Name: String; var LData: Smallint); overload;
    procedure LookupBasicData(const Name: String; var LData: Boolean); overload;
    procedure LookupBasicData(const Name: String; var LData: Single); overload;
    procedure LookupBasicData(const Name: String; LData: TStrings); overload;
    procedure RemoveNode(Index: Integer); overload;
    procedure RemoveNode(Node: TXMLNode); overload;

    property AsDateTime: TDateTime read GetAsDateTime;
    property AsInteger: Integer read GetAsInteger;
    property AsSingle: Single read GetAsSingle;
    property Data: String read FData write FData;
    property Name: String read FName write FName;
    property NodeCount: Integer read GetNodeCount;
    property Nodes[Index: Integer]: TXMLNode read GetNodes;
    property NodeType: TXMLNodeType read FNodeType write FNodeType;
    property TotalNodeCount: Integer read GetTotalNodeCount;
  end;

const
  TXMLNodeTypeStrings: array[TXMLNodeType] of String = ('Document', 'Element',
      'Attribute', 'ProcessingInstruction', 'XMLDeclaration');

implementation

uses
  SysUtils, Strlib;

const
  LetterSet: CharSet = ['A'..'Z', 'a'..'z'];
  WhitespaceSet: CharSet = [' ', #9, #10, #13];
  CombiningCharSet: CharSet = [];
  DigitSet: CharSet = ['0'..'9'];
  ExtenderSet: CharSet = [#$b7];
  FirstNameCharSet: CharSet = ['_', ':', 'A'..'Z', 'a'..'z'];
var
  NameCharSet: CharSet;

type
  TXMLStringReaderState = class
  public
    Position: Integer;
    CurChar: Char;
  end;

{ TXMLNode }

constructor TXMLNode.Create;
begin
  FNodes := TList.Create;
end;

destructor TXMLNode.Destroy;
var
  i: Integer;
begin
  inherited;
  for i := 0 to Pred(NodeCount) do
    Nodes[i].Free;
  FNodes.Free;
end;

function TXMLNode.AddNode(Node: TXMLNode): Integer;
begin
  Result := FNodes.Add(Node)
end;

function TXMLNode.Description: String;
begin
  Result := Name + ': ' + TXMLNodeTypeStrings[NodeType];
  if Data <> '' then
    Result := Result + ' = ' + Data
end;

function TXMLNode.FindNode(const Name: String): TXMLNode;
begin
  Result := FindNode(Name, 0)
end;

function TXMLNode.FindNode(const Name: String;
  const Index: Integer): TXMLNode;
var
  i, ThisIndex: Integer;
begin
  ThisIndex := 0;
  for i := 0 to Pred(NodeCount) do
    if Nodes[i].Name = Name then begin
      if ThisIndex = Index then begin
        Result := Nodes[i];
        Exit;
      end;
      Inc(ThisIndex);
    end;
  Result := nil;
end;

function TXMLNode.FindNodeExc(const Name: String): TXMLNode;
begin
  Result := FindNode(Name, 0);
  if Result = nil then
    raise Exception.Create('Node "' + Name + '" not found');
end;

function TXMLNode.GetAsDateTime: TDateTime;
begin
  Result := StrToDateTime(Data)
end;

function TXMLNode.GetAsInteger: Integer;
begin
  try
    Result := StrToInt(Data);
  except
    raise Exception.Create('Node "' + Name + '" does not contain a valid integer');
  end;
end;

function TXMLNode.GetAsSingle: Single;
begin
  try
    Result := StrToFloat(Data);
  except
    raise Exception.Create('Node "' + Name + '" does not contain a valid floating point value');
  end;
end;

function TXMLNode.GetNodeCount: Integer;
begin
  Result := FNodes.Count
end;

function TXMLNode.GetNodes(Index: Integer): TXMLNode;
begin
  Result := TXMLNode(FNodes[Index])
end;

function TXMLNode.GetTotalNodeCount: Integer;
var
  i: Integer;
begin
  Result := 1;
  for i := 0 to Pred(NodeCount) do
    Result := Result + Nodes[i].TotalNodeCount;
end;

function TXMLNode.HasNode(const Name: String): Boolean;
begin
  Result := FindNode(Name, 0) <> nil
end;

procedure TXMLNode.LookupBasicData(const Name: String; var LData: String);
var
  Node: TXMLNode;
begin
  Node := FindNode(Name);
  if Node <> nil then
    LData := Node.Data
  else
    LData := ''
end;

procedure TXMLNode.LookupBasicData(const Name: String; var LData: TDateTime);
var
  Node: TXMLNode;
begin
  Node := FindNode(Name);
  if Node <> nil then
    LData := Node.AsDateTime
  else
    LData := 0
end;

procedure TXMLNode.LookupBasicData(const Name: String;
  var LData: Smallint);
var
  L: Integer;
begin
  LookupBasicData(Name, L);
  LData := L
end;

procedure TXMLNode.LookupBasicData(const Name: String; var LData: Integer);
var
  Node: TXMLNode;
begin
  Node := FindNode(Name);
  if Node <> nil then
    LData := Node.AsInteger
  else
    LData := 0
end;

procedure TXMLNode.LookupBasicData(const Name: String; var LData: Boolean);
begin
  LData := HasNode(Name)
end;

procedure TXMLNode.LookupBasicData(const Name: String; var LData: Single);
var
  Node: TXMLNode;
begin
  Node := FindNode(Name);
  if Node <> nil then
    LData := Node.AsSingle
  else
    LData := 0
end;

procedure TXMLNode.LookupBasicData(const Name: String; LData: TStrings);
var
  i: Integer;
  Node, SubNode: TXMLNode;
begin
  LData.Clear;
  Node := FindNode(Name);
  if Node = nil then Exit;
  for i := 0 to Pred(Node.NodeCount) do begin
    SubNode := Node.Nodes[i];
    if (SubNode.Name = 'S') and (SubNode.NodeType = xntElement) then
      LData.Add(SubNode.Data);
  end;
end;

procedure TXMLNode.RemoveNode(Index: Integer);
begin
  RemoveNode(Nodes[Index])
end;

procedure TXMLNode.RemoveNode(Node: TXMLNode);
begin
  FNodes.Remove(Node);
end;

{ TXMLStringReader }

constructor TXMLStringReader.Create(AData: String);
begin
  FPosition := 1;
  FData := AData;
end;

function TXMLStringReader.EndOfData: Boolean;
begin
  Result := FPosition > Length(FData)
end;

procedure TXMLStringReader.FreeState(State: TObject);
begin
  State.Free;
end;

procedure TXMLStringReader.ReadChar;
begin
  if FPosition > Length(FData) then
    CurChar := #0
  else begin
    CurChar := FData[FPosition];
    Inc(FPosition)
  end
end;

procedure TXMLStringReader.RestoreState(State: TObject);
var
  XSRState: TXMLStringReaderState;
begin
  XSRState := (State as TXMLStringReaderState);
  FPosition := XSRState.Position;
  CurChar := XSRState.CurChar;
end;

function TXMLStringReader.SaveState: TObject;
var
  XSRState: TXMLStringReaderState;
begin
  XSRState := TXMLStringReaderState.Create;
  XSRState.Position := FPosition;
  XSRState.CurChar := CurChar;
  Result := XSRState;
end;

{ TXMLReader }

function TXMLReader.DoesStreamMatchString(Str: String): Boolean;
var
  i: Integer;
  State: TObject;
begin
  if Str = '' then begin Result := False; Exit end;
  State := SaveState;
  i := 1;
  repeat
    Result := Str[i] = CurChar;
    Inc(i);
    ReadChar;
  until (not Result) or EndOfData or (i > Length(Str));

  if not Result then
    RestoreState(State);
  FreeState(State);
end;


function TXMLReader.ParseAttribute(Node: TXMLNode): Boolean;
var
  Name, AttValue: String;
  ThisNode: TXMLNode;
begin
  Result := False;
  if not ParseName(Name) then Exit;
  if not ParseEq then
    ReportError('Equals sign missing in attribute');
  if not ParseAttValue(AttValue) then
    ReportError('Attribute "' + Name + '" missing value');
  ThisNode := TXMLNode.Create;
  ThisNode.NodeType := xntAttribute;
  ThisNode.Name := Name;
  ThisNode.Data := AttValue;
  Node.AddNode(ThisNode);
  Result := True;
end;

function TXMLReader.ParseAttValue(var AttValue: String): Boolean;
var
  QuoteChar: Char;
begin
  QuoteChar := CurChar;
  Result := False;
  if not (CurChar in ['"', '''']) then Exit;
  ReadCharAndCheckForEnd;

  while CurChar <> QuoteChar do begin
    if CurChar = '<' then
      ReportError('Invalid character "' + CurChar + '" in attribute value')
    else if CurChar = '&' then
      CurChar := ParseReference;
    AttValue := AttValue + CurChar;
    ReadCharAndCheckForEnd;
  end;
  ReadCharAndCheckForEnd;
  Result := True;
end;

function TXMLReader.ParseCDSect(Node: TXMLNode): Boolean;
var
  CData: String;
begin
  Result := False;
  if not DoesStreamMatchString('<![CDATA[') then Exit;
  repeat
    CData := CData + CurChar;
    ReadCharAndCheckForEnd;
  until Right(CData, 3) = ']]>';
  Node.Data := Node.Data + Left(CData, Length(CData) - 3);
  Result := True;
end;

function TXMLReader.ParseCharData(Node: TXMLNode): Boolean;
var
  CharData: String;
begin
  while not (CurChar in ['<', '&', #0]) do begin
    CharData := CharData + CurChar;
    ReadChar;
  end;
  if Pos(']]>', CharData) <> 0 then
    ReportError('Character data cannot contain "]]>"');
  Node.Data := Node.Data + CharData;
  Result := CharData <> '';
end;

function TXMLReader.ParseComment: Boolean;
var
  Comment: String;
  Terminated: Boolean;
begin
  Result := False;
  if not DoesStreamMatchString('<!--') then Exit;
  repeat
    Comment := Comment + CurChar;
    Terminated := Right(Comment, 3) = '-->';
    if EndOfData then Break;
    ReadChar;
  until Terminated;
  if Right(Comment, 4) = '--->' then
    ReportError('Comments cannot end in "--->"');
  Result := True;
end;

procedure TXMLReader.ParseContent(Node: TXMLNode);
var
  FoundContent: Boolean;
begin
  repeat
    FoundContent := True;
    if ParseCDSect(Node) then
    else if ParseProcessingInstruction(Node) then
    else if ParseComment then
    else if ParseElement(Node) then
    else if ParseCharData(Node) then
    else if CurChar = '&' then
      Node.Data := Node.Data + ParseReference
    else
      FoundContent := False;
  until not FoundContent;
end;

procedure TXMLReader.ParseDocTypeDecl(Node: TXMLNode);
begin
  if not DoesStreamMatchString('<!DOCTYPE') then Exit;
//  ReportError('DocTypeDecl parsing not yet implemented');
end;

function TXMLReader.ParseDocument: TXMLNode;
begin
  ReadCharAndCheckForEnd;
  Result := TXMLNode.Create;
  Result.Name := '';
  Result.NodeType := xntDocument;
  if not ParseProlog(Result) then
    ReportError('Missing prolog');
  if not ParseElement(Result) then
    ReportError('Missing element');
  while ParseMisc(Result) do
    ;
end;

function TXMLReader.ParseElement(Node: TXMLNode): Boolean;
var
  Name: String;
  ThisNode: TXMLNode;
  State: TObject;
begin
  Result := False;
  if CurChar <> '<' then Exit;
  State := SaveState;
  ReadCharAndCheckForEnd;
  if not ParseName(Name) then begin
    RestoreState(State);
    FreeState(State);
    Exit;
  end;
  
  FreeState(State);

  ThisNode := TXMLNode.Create;
  Node.AddNode(ThisNode);
  try
    ThisNode.Name := Name;
    ThisNode.NodeType := xntElement;
    repeat
      ParseWhitespace;
    until not ParseAttribute(ThisNode);

    if CurChar = '/' then begin
      // EmptyElemTag
      ReadCharAndCheckForEnd;
      if CurChar <> '>' then
        ReportError('Tag not terminated properly');
      ReadCharAndCheckForEnd;
    end else if CurChar = '>' then begin
      // STag
      ReadCharAndCheckForEnd;
      ParseContent(ThisNode);
      ThisNode.Data := TrimChars(ThisNode.Data, [' ', #9, #13, #10]);
      if not ParseETag(ThisNode) then
        ReportError('No ending tag for ' + Name);
    end else
      ReportError('Tag not terminated properly');
  except
    Node.RemoveNode(ThisNode);
    ThisNode.Free;
    raise;
  end;
  Result := True;
end;

function TXMLReader.ParseEq: Boolean;
begin
  ParseWhitespace;
  Result := CurChar = '=';
  ReadCharAndCheckForEnd;
  ParseWhitespace;
end;

function TXMLReader.ParseETag(Node: TXMLNode): Boolean;
var
  Name: String;
begin
  Result := False;
  if not DoesStreamMatchString('</') then Exit;
  if not ParseName(Name) then Exit;
  if Name <> Node.Name then
    ReportError('End tag name "' + Name + '" does not match start tag name "' + Node.Name + '"');
  ParseWhitespace;
  if CurChar <> '>' then
    ReportError('End tag missing ">"');
  ReadChar;
  Result := True;
end;

function TXMLReader.ParseMisc(Node: TXMLNode): Boolean;
begin
  Result := ParseWhitespace;
  if Result then Exit;
  Result := ParseComment;
  if Result then Exit;
  Result := ParseProcessingInstruction(Node);
end;

function TXMLReader.ParseName(var Name: String): Boolean;
begin
  Result := False;
  if not (CurChar in FirstNameCharSet) then Exit;
  Result := True;
  Name := '';
  repeat
    Name := Name + CurChar;
    ReadCharAndCheckForEnd;
  until not (CurChar in NameCharSet);
end;

function TXMLReader.ParseProcessingInstruction(Node: TXMLNode): Boolean;
var
  Name, Chars: String;
  ThisNode: TXMLNode;
  Terminated: Boolean;
begin
  Result := False;
  if not DoesStreamMatchString('<?') then Exit;
  if not ParseName(Name) then
    ReportError('Processing instruction does not contain valid name');
  if Name = 'xml' then
    ReportError('Processing instruction cannot have the reserved name "xml"');
  ThisNode := TXMLNode.Create;
  try
    ThisNode.Name := Name;
    ThisNode.NodeType := xntProcessingInstruction;
    ParseWhitespace;
    repeat
      Chars := Chars + CurChar;
      Terminated := Right(Chars, 2) = '?>';
      if EndOfData then break;
      ReadChar;
    until Terminated;
    if not Terminated then
      ReportError('Unterminated processing instruction');
    ThisNode.Data := Left(Chars, Length(Chars) - 2);
  except
    ThisNode.Free;
    raise;
  end;
  Node.AddNode(ThisNode);
  Result := True;
end;

function TXMLReader.ParseProlog(Node: TXMLNode): Boolean;
begin
  ParseXMLDecl(Node);
  while ParseMisc(Node) do
    ;
  ParseDocTypeDecl(Node);
  while ParseMisc(Node) do
    ;
  Result := True;
end;

function TXMLReader.ParseReference: Char;
var
  RefStr: String;
begin
  Result := #0; // silence compiler warning
  if CurChar <> '&' then
    ReportError('All references must begin with "&"');
  ReadCharAndCheckForEnd;
  while CurChar <> ';' do begin
    RefStr := RefStr + CurChar;
    ReadCharAndCheckForEnd;
  end;
  if Left(RefStr, 2) = '#x' then
    try
      Result := Chr(StrToInt('$' + Right(RefStr, Length(RefStr) - 2)));
    except
      ReportError('Invalid integer in reference &' + RefStr + ';');
    end
  else if Left(RefStr, 1) = '#' then
    try
      Result := Chr(StrToInt(Right(RefStr, Length(RefStr) - 1)));
    except
      ReportError('Invalid integer in reference &' + RefStr + ';');
    end
  else if RefStr = 'amp' then
    Result := '&'
  else if RefStr = 'lt' then
    Result := '<'
  else if RefStr = 'gt' then
    Result := '>'
  else if RefStr = 'quot' then
    Result := '"'
  else if RefStr = 'apos' then
    Result := ''''
  else
    ReportError('Unrecognized reference');

  ReadCharAndCheckForEnd;
end;

function TXMLReader.ParseWhitespace: Boolean;
begin
  Result := False;
  while (CurChar in [' ', #13, #10, #9]) do begin
    Result := True;
    ReadChar;
    if EndOfData then Exit;
  end;
end;

procedure TXMLReader.ParseXMLDecl(Node: TXMLNode);
var
  ThisNode: TXMLNode;
begin
  if not DoesStreamMatchString('<?xml') then Exit;
  ThisNode := TXMLNode.Create;
  ThisNode.NodeType := xntXMLDeclaration;
  try
    repeat
      ParseWhitespace;
    until not ParseAttribute(ThisNode);
    ParseWhitespace;
    if not DoesStreamMatchString('?>') then
      ReportError('XML declaration not terminated with "?>"');
  except
    ThisNode.Free;
    raise;
  end;
  Node.AddNode(ThisNode);
end;

procedure TXMLReader.ReadCharAndCheckForEnd;
begin
  if EndOfData then
    ReportError('Unexpected end of data');
  ReadChar;
end;

procedure TXMLReader.ReportError(Err: String);
begin
  raise Exception.Create(Err);
end;

initialization
  NameCharSet := LetterSet + CombiningCharSet + DigitSet + ExtenderSet
      + ['.', '-', '_', ':'];
end.
