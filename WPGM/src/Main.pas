unit Main;

interface

uses
  Windows, Messages, SysUtils, Classes, Graphics, Controls, Forms, Dialogs,
   ComCtrls, ImgList, ExtCtrls, SQLite3, SQLiteTable3, UOTiledata,
  GR32, GR32_Image, UOArt, UOAnim, StdCtrls, uConfig, XPMan, UOMap, UOStatics,
  Menus, UOHues, Buttons, SyncObjs, VirtualTrees, UORadarCol, ToolWin;

type Txy = record
  x: word;
  y: word;
  //map: string;
  //staidx: string;
  //stamul: string;
end;

type Tuofiles = record
  type_ : string;
  filename: string;
end;

type TNode = record
    Id: Cardinal;
    Name: WideString;
end;

type TLocationNode = record
   Id: Cardinal;
   Name: WideString;
   LocId: String;
   PosX: Cardinal;
   PosY: Cardinal;
   PosZ: ShortInt;
   PosMap: Byte;
end;

type TItemNode = record
    Id: Cardinal;
    Name: WideString;
    ArtId: Word;
    Color: Word;
    AddId: String;
end;

type TMultiNode = record
    Id: Cardinal;
    Name: WideString;
    AddId: String;
    MultiId: Word;
end;

type TNpcEquipment = record
  Layer: Byte;
  Anim: Word;
  Color: Word;
end;

type TNpcNode = record
    Id: Cardinal;
    Name: WideString;
    BodyId: Word;
    Skin: Word;
    AddId: String;
    Equipment: Array of TNpcEquipment;
end;

type PMultiNode = ^TMultiNode;
type PItemNode = ^TItemNode;
type PNPCNode = ^TNPCNode;
type PLocationNode = ^TLocationNode;
type PNode = ^TNode;

type
  TfrmMain = class(TForm)
    Pages: TPageControl;
    tsItems: TTabSheet;
    tsSettings: TTabSheet;
    pnlItems: TPanel;
    spItems: TSplitter;
    vtCategories: TVirtualStringTree;
    vtItems: TVirtualStringTree;
    XPManifest1: TXPManifest;
    gbSettingsGeneral: TGroupBox;
    cbAlwaysOnTop: TCheckBox;
    btnAddItem: TButton;
    btnRemoveItem: TButton;
    lblCommandPrefix: TLabel;
    eCommandPrefix: TEdit;
    TravelMap: TTabSheet;
    imgItemPreview: TImage32;
    pnlTravel: TPanel;
    vtLocCategories: TVirtualStringTree;
    vtLocItems: TVirtualStringTree;
    pbMap: TPaintBox32;
    spTravel: TSplitter;
    spMap: TSplitter;
    pmTravel: TPopupMenu;
    Go1: TMenuItem;
    btnTravel: TButton;
    btnSend: TButton;
    tsCommands: TTabSheet;
    GroupBox1: TGroupBox;
    btnCmdAllmove: TButton;
    btnCmdAllshow: TButton;
    btnCmdInvisible: TButton;
    btnCmdStaff: TButton;
    btnCmdInvul: TButton;
    btnCmdPageNotify: TButton;
    GroupBox2: TGroupBox;
    btnCmdShutdown: TButton;
    btnCmdReloadPython: TButton;
    btnCmdReloadScripts: TButton;
    btnCmdReloadAccounts: TButton;
    btnCmdReloadAll: TButton;
    btnSave: TButton;
    tsItemManip: TTabSheet;
    tsTags: TTabSheet;
    GroupBox3: TGroupBox;
    eItemHue: TEdit;
    btnChangeItemHue: TButton;
    btnCmdDye: TButton;
    pbHuePreview: TPaintBox32;
    GroupBox4: TGroupBox;
    btnCmdDupe: TButton;
    btnCmdInfo: TButton;
    btnCmdRemove: TButton;
    btnCmdLock: TButton;
    btnCmdNuke: TButton;
    GroupBox5: TGroupBox;
    cbItemProperty: TComboBox;
    eItemPropValue: TEdit;
    btnCmdItemSet: TButton;
    btnCmdItemShow: TButton;
    Label1: TLabel;
    btnCmdItemTileColor: TButton;
    GroupBox6: TGroupBox;
    Label2: TLabel;
    eTagValue: TEdit;
    btnTagSet: TButton;
    eTagName: TEdit;
    Label3: TLabel;
    btnTagShow: TButton;
    btnTagDelete: TButton;
    cbTagType: TComboBox;
    btnTagInfo: TButton;
    GroupBox7: TGroupBox;
    Button2: TButton;
    btnCmdSendOther: TButton;
    Send1: TMenuItem;
    eMoveX: TEdit;
    eMoveY: TEdit;
    eMoveZ: TEdit;
    btnCmdMove: TButton;
    Label4: TLabel;
    Label5: TLabel;
    Label6: TLabel;
    btnCmdMoveUp: TBitBtn;
    btnCmdMoveDown: TBitBtn;
    btnTileItem: TButton;
    eZLevel: TEdit;
    Label7: TLabel;
    btnCmdItemNuke: TButton;
    GroupBox8: TGroupBox;
    btnCmdAddEvent: TButton;
    btnCmdRemoveEvent: TButton;
    btnCmdShowEventlist: TButton;
    btnCmdWhere: TButton;
    btnCmdServertime: TButton;
    cbEventName: TComboBox;
    tsNpcs: TTabSheet;
    pnNpcs: TPanel;
    Splitter1: TSplitter;
    vtNpcCategories: TVirtualStringTree;
    vtNpcs: TVirtualStringTree;
    btnAddNpc: TButton;
    btnRemoveNpc: TButton;
    Splitter2: TSplitter;
    pbNpcPreview: TPaintBox32;
    animtimer: TTimer;
    TabSheet1: TTabSheet;
    Panel1: TPanel;
    vtMultis: TVirtualStringTree;
    btnAddMulti: TButton;
    btnRemoveMulti: TButton;
    btnPreviewMulti: TButton;
    N1: TMenuItem;
    Felucca1: TMenuItem;
    Trammel1: TMenuItem;
    Ilshenar1: TMenuItem;
    Malas1: TMenuItem;
    Center1: TMenuItem;
    SamuraiEmpire1: TMenuItem;
    cbStatic: TCheckBox;
    btnMapOverview: TButton;
    ontopTimer: TTimer;
    Label8: TLabel;
    eSpawnRadius: TEdit;
    Label9: TLabel;
    eSpawnMin: TEdit;
    Label10: TLabel;
    eSpawnMax: TEdit;
    btnNpcSpawn: TButton;
    btnRegions: TButton;
    Button1: TButton;
    TabSheet2: TTabSheet;
    GroupBox9: TGroupBox;
    cbBuildZ: TComboBox;
    Label11: TLabel;
    gbBuild: TGroupBox;
    sbBuild: TScrollBox;
    tsCustom: TTabSheet;
    btnCustomPopup: TPopupMenu;
    Remove1: TMenuItem;
    Change1: TMenuItem;
    AddButton1: TMenuItem;
    VirtualStringTree1: TVirtualStringTree;
    BitFiles: TBitBtn;
    TbQuick: TToolBar;
    ToolButton1: TToolButton;
    ToolButton2: TToolButton;
    ToolButton3: TToolButton;
    ToolButton4: TToolButton;
    ToolButton5: TToolButton;
    ToolButton6: TToolButton;
    ToolButton7: TToolButton;
    Splitter3: TSplitter;
    procedure FormCreate(Sender: TObject);
    procedure FormDestroy(Sender: TObject);
    procedure vtCategoriesGetText(Sender: TBaseVirtualTree;
      Node: PVirtualNode; Column: TColumnIndex; TextType: TVSTTextType;
      var CellText: WideString);
    procedure vtCategoriesChange(Sender: TBaseVirtualTree;
      Node: PVirtualNode);
    procedure vtItemsGetText(Sender: TBaseVirtualTree; Node: PVirtualNode;
      Column: TColumnIndex; TextType: TVSTTextType;
      var CellText: WideString);
    procedure vtCategoriesExpanding(Sender: TBaseVirtualTree;
      Node: PVirtualNode; var Allowed: Boolean);
    procedure vtItemsCompareNodes(Sender: TBaseVirtualTree; Node1,
      Node2: PVirtualNode; Column: TColumnIndex; var Result: Integer);
    procedure vtItemsHeaderClick(Sender: TVTHeader; Column: TColumnIndex;
      Button: TMouseButton; Shift: TShiftState; X, Y: Integer);
    procedure btnAddItemClick(Sender: TObject);
    procedure vtItemsChange(Sender: TBaseVirtualTree; Node: PVirtualNode);
    procedure btnRemoveItemClick(Sender: TObject);
    procedure cbAlwaysOnTopClick(Sender: TObject);
    procedure btnSaveClick(Sender: TObject);
    procedure FormClose(Sender: TObject; var Action: TCloseAction);
    procedure pbMapPaintBuffer(Sender: TObject);
    procedure pbMapMouseDown(Sender: TObject; Button: TMouseButton;
      Shift: TShiftState; X, Y: Integer);
    procedure vtLocCategoriesExpanding(Sender: TBaseVirtualTree;
      Node: PVirtualNode; var Allowed: Boolean);
    procedure vtLocCategoriesChange(Sender: TBaseVirtualTree;
      Node: PVirtualNode);
    procedure vtLocItemsGetText(Sender: TBaseVirtualTree;
      Node: PVirtualNode; Column: TColumnIndex; TextType: TVSTTextType;
      var CellText: WideString);
    procedure vtLocItemsCompareNodes(Sender: TBaseVirtualTree; Node1,
      Node2: PVirtualNode; Column: TColumnIndex; var Result: Integer);
    procedure vtLocItemsHeaderClick(Sender: TVTHeader;
      Column: TColumnIndex; Button: TMouseButton; Shift: TShiftState; X,
      Y: Integer);
    procedure vtLocItemsChange(Sender: TBaseVirtualTree;
      Node: PVirtualNode);
    procedure Go1Click(Sender: TObject);
    procedure btnSendClick(Sender: TObject);
    procedure btnCmdAllmoveClick(Sender: TObject);
    procedure btnCmdAllshowClick(Sender: TObject);
    procedure btnCmdInvisibleClick(Sender: TObject);
    procedure btnCmdStaffClick(Sender: TObject);
    procedure btnCmdInvulClick(Sender: TObject);
    procedure btnCmdShutdownClick(Sender: TObject);
    procedure btnCmdReloadAllClick(Sender: TObject);
    procedure btnCmdReloadConfigClick(Sender: TObject);
    procedure btnCmdReloadAccountsClick(Sender: TObject);
    procedure btnCmdReloadPythonClick(Sender: TObject);
    procedure btnCmdReloadScriptsClick(Sender: TObject);
    procedure btnCmdDyeClick(Sender: TObject);
    procedure pbHuePreviewPaintBuffer(Sender: TObject);
    procedure eItemHueChange(Sender: TObject);
    procedure btnChangeItemHueClick(Sender: TObject);
    procedure btnCmdDupeClick(Sender: TObject);
    procedure btnCmdInfoClick(Sender: TObject);
    procedure btnCmdRemoveClick(Sender: TObject);
    procedure btnCmdLockClick(Sender: TObject);
    procedure btnCmdNukeClick(Sender: TObject);
    procedure btnCmdItemSetClick(Sender: TObject);
    procedure btnCmdItemShowClick(Sender: TObject);
    procedure cbItemPropertyChange(Sender: TObject);
    procedure btnCmdItemTileColorClick(Sender: TObject);
    procedure btnTagInfoClick(Sender: TObject);
    procedure eTagNameChange(Sender: TObject);
    procedure btnTagSetClick(Sender: TObject);
    procedure btnTagShowClick(Sender: TObject);
    procedure btnTagDeleteClick(Sender: TObject);
    procedure btnCmdSendOtherClick(Sender: TObject);
    procedure Button2Click(Sender: TObject);
    procedure btnCmdMoveClick(Sender: TObject);
    procedure btnCmdMoveUpClick(Sender: TObject);
    procedure btnCmdMoveDownClick(Sender: TObject);
    procedure btnTileItemClick(Sender: TObject);
    procedure btnCmdShowEventlistClick(Sender: TObject);
    procedure eEventNameChange(Sender: TObject);
    procedure btnCmdAddEventClick(Sender: TObject);
    procedure btnCmdRemoveEventClick(Sender: TObject);
    procedure btnCmdWhereClick(Sender: TObject);
    procedure btnCmdServertimeClick(Sender: TObject);
    procedure vtNpcCategoriesExpanding(Sender: TBaseVirtualTree;
      Node: PVirtualNode; var Allowed: Boolean);
    procedure vtNpcCategoriesChange(Sender: TBaseVirtualTree;
      Node: PVirtualNode);
    procedure vtNpcsCompareNodes(Sender: TBaseVirtualTree; Node1,
      Node2: PVirtualNode; Column: TColumnIndex; var Result: Integer);
    procedure vtNpcsGetText(Sender: TBaseVirtualTree; Node: PVirtualNode;
      Column: TColumnIndex; TextType: TVSTTextType;
      var CellText: WideString);
    procedure btnRemoveNpcClick(Sender: TObject);
    procedure btnAddNpcClick(Sender: TObject);
    procedure vtNpcsChange(Sender: TBaseVirtualTree; Node: PVirtualNode);
    procedure vtNpcsHeaderClick(Sender: TVTHeader; Column: TColumnIndex;
      Button: TMouseButton; Shift: TShiftState; X, Y: Integer);
    procedure pbNpcPreviewPaintBuffer(Sender: TObject);
    procedure animtimerTimer(Sender: TObject);
    procedure vtMultisGetText(Sender: TBaseVirtualTree; Node: PVirtualNode;
      Column: TColumnIndex; TextType: TVSTTextType;
      var CellText: WideString);
    procedure vtMultisCompareNodes(Sender: TBaseVirtualTree; Node1,
      Node2: PVirtualNode; Column: TColumnIndex; var Result: Integer);
    procedure vtMultisHeaderClick(Sender: TVTHeader; Column: TColumnIndex;
      Button: TMouseButton; Shift: TShiftState; X, Y: Integer);
    procedure vtMultisChange(Sender: TBaseVirtualTree; Node: PVirtualNode);
    procedure btnPreviewMultiClick(Sender: TObject);
    procedure btnAddMultiClick(Sender: TObject);
    procedure Felucca1Click(Sender: TObject);
    procedure Trammel1Click(Sender: TObject);
    procedure Ilshenar1Click(Sender: TObject);
    procedure Malas1Click(Sender: TObject);
    procedure Center1Click(Sender: TObject);
    procedure SamuraiEmpire1Click(Sender: TObject);
    procedure btnStaticItemClick(Sender: TObject);
    procedure btnMapOverviewClick(Sender: TObject);
    procedure ontopTimerTimer(Sender: TObject);
    procedure btnNpcSpawnClick(Sender: TObject);
    procedure eSpawnRadiusExit(Sender: TObject);
    procedure eSpawnMinExit(Sender: TObject);
    procedure eSpawnMaxExit(Sender: TObject);
    procedure btnRegionsClick(Sender: TObject);
    procedure Button1Click(Sender: TObject);
    procedure sbBuildResize(Sender: TObject);
    procedure clickBuildButton(Sender: TObject);
    procedure sbBuildClick(Sender: TObject);
    procedure sbBuildMouseWheelDown(Sender: TObject; Shift: TShiftState;
      MousePos: TPoint; var Handled: Boolean);
    procedure sbBuildMouseWheelUp(Sender: TObject; Shift: TShiftState;
      MousePos: TPoint; var Handled: Boolean);
    procedure customButtonClick(Sender: TObject);
    procedure customButtonMouseDown(Sender: TObject; Button: TMouseButton;
      Shift: TShiftState; X, Y: Integer);
    procedure Remove1Click(Sender: TObject);
    procedure Change1Click(Sender: TObject);
    procedure AddButton1Click(Sender: TObject);
    procedure tsCustomResize(Sender: TObject);
    procedure BitFilesClick(Sender: TObject);
  private
    { Private-Deklarationen }
    UpdateMutex: TCriticalSection;
  public
    { Public-Deklarationen }
    procedure pixelCombine(F: TColor32; var B: TColor32; M: TColor32);
    procedure loadMultis;
    procedure addBuildButton(Id: String; ItemId: Word; Text: String);
    procedure rebuildBuildMenu;
    procedure reorderCustomButtons;
    procedure loadCustomButtons;
    procedure saveCustomButtons;
    procedure changeCustomButton(Button: TButton; NewButton: Boolean = False);
  end;

const
  // OK OK I can make it better.. but in the future :D
  FILESMAX = 31;
  FILESMIN = 0;
  UOFILES : array[FILESMIN..FILESMAX] of string = ('TILEDATA.MUL', 'ARTIDX.MUL', 'ART.MUL', 'ANIM.IDX', 'ANIM.MUL', 'ANIM2.IDX', 'ANIM2.MUL', 'ANIM3.IDX', 'ANIM3.MUL', 'ANIM4.IDX', 'ANIM4.MUL', 'ANIM5.IDX', 'ANIM5.MUL', 'BODYCONV.DEF', 'BODY.DEF', 'RADARCOL.MUL', 'HUES.MUL', 'MAP0.MUL', 'STAIDX0.MUL', 'STATICS0.MUL', 'MAP1.MUL', 'STAIDX1.MUL', 'STATICS1.MUL', 'MAP2.MUL', 'STAIDX2.MUL', 'STATICS2.MUL', 'MAP3.MUL', 'STAIDX3.MUL', 'STATICS3.MUL', 'MAP4.MUL', 'STAIDX4.MUL', 'STATICS4.MUL');
  UOREQ   : array[FILESMIN..FILESMAX] of boolean = (true,           true,          true,        true,         true,      false,          false,           false,       false,           false,       false,        false,         false,       true,         true,       true,           true,      true,       true,           false,      false,          false,            true,          false,        false,         false,         false,      false,          false,         false,        false,       false);



var
  frmMain: TfrmMain;
  SQLiteDb: TSQLiteDatabase;
  ItemCategory: Cardinal;
  Tiledata: TTiledataReader;
  Art: TArtReader;
  Anim: TAnimReader;
  Maps: Array[0..4] of TMapReader;
  Statics: Array[0..4] of TStaticReader;
  Hues: THuesReader;
  RadarCol: TRadarcolReader;
  Config: TConfig;
  MapCenterX, MapCenterY: Cardinal;
  MapCenterZ: ShortInt;
  MapCenterMap: Byte;
  MapId: Cardinal;
  npcPreview: Array of TAnimation;
  npcPreviewFrame: Integer;
  BuildCurrentX: Integer;
  BuildCurrentY: Integer;
  MapsSize: Array[0..4] of Txy;

implementation

uses UOUtilities, uChooseHue, Math, uCenter, Overview, Spawnregions,
  uPatternColor     , Configuration;

{$R *.DFM}



function openFiles(var config: tconfig): Integer;
var
  i,err : integer;
  Prgr: TFrmConfig;
  UOPath: String;
begin
    err := 0;
    // Verify the files
    UOPath := getUoPath;
    Tiledata := TTiledataReader.Create;
    Art := TArtReader.Create;
    Anim := TAnimReader.Create;

    Hues := THuesReader.Create;
    Art.hues := Hues; // Tell the art reader about the hues
    Anim.hues := Hues; // Tell the anim reader about the hues

    for I := FILESMIN to FILESMAX do
    begin

        //Prgr.Hide;
        if (not fileexists(Config.getString(UOFILES[i] +' Path',UOPath + UOFILES[i]))) and (UOREQ[i]) then
        begin
          err := 1;

        end;

    end;
    // Require files not found, show the config from
    if err <> 0 then
    begin
      Showmessage('Some required files are missing!');
      Prgr := Tfrmconfig.Create(nil);
      Prgr.FormStyle := fsStayOnTop;
      // TODO load the files on edits...
      Prgr.ShowModal;
      // reload?
      Showmessage('Please reopen WPGM for the new setting be loaded.');
      result := 0;
    end
    else
    begin

      MapCenterX := 0;
      MapCenterY := 0;
      MapCenterZ := 0;
      MapCenterMap := 0;

      // Should be updated to support multiple maps
      for i := 0 to 4 do begin
        Maps[i] := TMapReader.Create;
        Statics[i] := TStaticReader.Create;
      end;
      RadarCol := TRadarColReader.Create;

      Tiledata.Open(Config.getString('TILEDATA.MUL Path',
          UOPath + 'tiledata.mul'));
      Art.Open(Config.getString('ARTIDX.MUL Path', UOPath + 'artidx.mul'),
          Config.getString('ART.MUL Path', UOPath + 'art.mul'));
      Anim.Open(Config.getString('BODYCONV.DEF Path', UOPath + 'bodyconv.def'),
          Config.getString('BODY.DEF Path', UOPath + 'body.def'),
          Config.getString('ANIM.IDX Path', UOPath + 'anim.idx'),
          Config.getString('ANIM.MUL Path', UOPath + 'anim.mul'),
          Config.getString('ANIM2.IDX Path', UOPath + 'anim2.idx'),
          Config.getString('ANIM2.MUL Path', UOPath + 'anim2.mul'),
          Config.getString('ANIM3.IDX Path', UOPath + 'anim3.idx'),
          Config.getString('ANIM3.MUL Path', UOPath + 'anim3.mul'),
          Config.getString('ANIM4.IDX Path', UOPath + 'anim4.idx'),
          Config.getString('ANIM4.MUL Path', UOPath + 'anim4.mul'),
          Config.getString('ANIM5.IDX Path', UOPath + 'anim5.idx'),
          Config.getString('ANIM5.MUL Path', UOPath + 'anim5.mul')
          );
      RadarCol.Load(Config.getString('RADARCOL.MUL Path',
          UOPath + 'radarcol.mul'));
      Hues.Load(Config.getString('HUES.MUL Path',
          UOPath + 'hues.mul'));

      MapsSize[0].x := Config.getInt('Map 0 Width', 896);
      MapsSize[0].y := Config.getInt('Map 0 Height', 512);
      MapsSize[1].x := Config.getInt('Map 1 Width', 896);
      MapsSize[1].y := Config.getInt('Map 1 Height', 512);
      MapsSize[2].x := Config.getInt('Map 2 Width', 288);
      MapsSize[2].y := Config.getInt('Map 2 Height', 200);
      MapsSize[3].x := Config.getInt('Map 3 Width', 320);
      MapsSize[3].y := Config.getInt('Map 3 Height', 256);
      MapsSize[4].x := Config.getInt('Map 4 Width', 181);
      MapsSize[4].y := Config.getInt('Map 4 Height', 181);

      Maps[0].Open(
          Config.getString('MAP0.MUL Path', UOPath + 'map0.mul'),
          MapsSize[0].x,
          MapsSize[0].y);

      Statics[0].Open(
          Config.getString('STAIDX0.MUL Path', UOPath + 'staidx0.mul'),
          Config.getString('STATICS0.MUL Path', UOPath + 'statics0.mul'),
          MapsSize[0].y,
          MapsSize[0].x);

      if FileExists(Config.getString('MAP1.MUL Path', UOPath + 'map1.mul')) then
      begin
          Maps[1].Open(
            Config.getString('MAP1.MUL Path', UOPath + 'map1.mul'),
            MapsSize[1].x,
            MapsSize[1].y);

          Statics[1].Open(
            Config.getString('STAIDX1.MUL Path', UOPath + 'staidx1.mul'),
            Config.getString('STATICS1.MUL Path', UOPath + 'statics1.mul'),
            MapsSize[1].y,
            MapsSize[1].x);
      end
      else
      begin
          Maps[1] := Maps[0];
          Statics[1] := Statics[0];
      end;


      Maps[2].Open(Config.getString('MAP2.MUL Path', UOPath + 'map2.mul'),
          MapsSize[2].x,
          MapsSize[2].y);

      STATICS[2].Open(
          Config.getString('STAIDX2.MUL Path', UOPath + 'STAIDX2.MUL'),
          Config.getString('STATICS2.MUL Path', UOPath + 'STATICS2.MUL'),
          MapsSize[2].y,
          MapsSize[2].x);

      Maps[3].Open(Config.getString('Map3.MUL Path', UOPath + 'map3.MUL'),
          MapsSize[3].x,
          MapsSize[3].y);

      STATICS[3].Open(
          Config.getString('STAIDX3.MUL Path', UOPath + 'STAIDX3.MUL'),
          Config.getString('STATICS3.MUL Path', UOPath + 'STATICS3.MUL'),
          MapsSize[3].y,
          MapsSize[3].x);

      Maps[4].Open(Config.getString('Map4.MUL Path', UOPath + 'map4.MUL'),
          MapsSize[4].x,
          MapsSize[4].y);

      STATICS[4].Open(
          Config.getString('STAIDX4.MUL Path', UOPath + 'STAIDX4.MUL'),
          Config.getString('STATICS4.MUL Path', UOPath + 'STATICS4.MUL'),
          MapsSize[4].y,
          MapsSize[4].x);
      result := 0;
    end;
end;

procedure TfrmMain.FormCreate(Sender: TObject);
var
    RootQuery: TSQLiteTable;
    CountQuery: TSQLiteTable;
    Node: PNode;
    TreeNode: PVirtualNode;
    i: Integer;
    value: String;
begin
    inherited;

    UpdateMutex := TCriticalSection.Create;

    npcPreview := nil;
    npcPreviewFrame := 0;
    Config := TConfig.Create;
    Config.Load(ExtractFilePath( Application.ExeName ) + 'config.ini');

    vtCategories.NodeDataSize := sizeof( TNode );
    vtNpcCategories.NodeDataSize := sizeof(TNode);
    vtLocCategories.NodeDataSize := sizeof(TNode);

    vtItems.NodeDataSize := sizeof( TItemNode );
    vtLocItems.NodeDataSize := sizeof(TLocationNode);
    vtNpcs.NodeDataSize := sizeof( TNpcNode );
    vtMultis.NodeDataSize := sizeof( TMultiNode );

    eSpawnRadius.Text := Config.getString('SpawnRadius', '5');
    eSpawnMin.Text := Config.getString('SpawnMin', '5');
    eSpawnMax.Text := Config.getString('SpawnMax', '10');
    eCommandPrefix.Text := Config.getString('Command Prefix', '''');
    cbAlwaysOnTop.Checked := Config.getBool('Always On Top', False);
    cbAlwaysOnTopClick(nil); // Refresh the Status


    // Query initial count of Items
    try
      //    TSQLiteDatabase
    	SQLiteDb := TSQLiteDatabase.Create( 'categories.db' );
        SQLiteDb.ExecSQL( 'PRAGMA default_cache_size = 10000;' );
        SQLiteDb.ExecSQL( 'PRAGMA default_synchronous = OFF;' );
        SQLiteDb.ExecSQL( 'PRAGMA full_column_names = OFF;' );
        SQLiteDb.ExecSQL( 'PRAGMA show_datatypes = OFF;' );

        if not SqLitedb.TableExists('categories') then
        begin
          // Table not exists, maybe databse don't exists too
          Application.MessageBox(PChar('Please check you categories.db, It''s corrupted'), 'Error', MB_OK+MB_ICONERROR );
          exit;
        end;


        RootQuery := TSQLiteTable.Create( SQLiteDb, 'SELECT id,name FROM categories WHERE type = 0 AND parent = 0 ORDER BY name ASC;' );
        //RootQuery := SQLiteDb.GetTable( 'SELECT id,name FROM categories WHERE type = 0 AND parent = 0 ORDER BY name ASC' );
        while not RootQuery.EOF do
        begin
          TreeNode := vtCategories.AddChild( nil );
          Node := vtCategories.GetNodeData( TreeNode );
          Node.Id := StrToInt( RootQuery.Fields[0] );
          Node.Name := UTF8Decode(RootQuery.Fields[1]);
          TreeNode.States := [ vsInitialized ] + TreeNode.States;

          // Check for Children
          CountQuery := SQLiteDb.GetTable( Format( 'SELECT COUNT(*) FROM categories WHERE type = 0 AND parent = %u', [ Node.Id ] ) );

          if StrToInt(CountQuery.Fields[0]) > 0 then
            TreeNode.States := [ vsHasChildren ] + TreeNode.States;
          CountQuery.Free;
          RootQuery.Next;
        end;
        RootQuery.Free;

        RootQuery := TSQLiteTable.Create( SQLiteDb, 'SELECT id,name FROM locationcategories WHERE type = 0 AND parent = 0 ORDER BY name ASC;' );
        while not RootQuery.EOF do
        begin
          TreeNode := vtLocCategories.AddChild( nil );
          Node := vtLocCategories.GetNodeData( TreeNode );
          Node.Id := StrToInt( RootQuery.Fields[0] );
          Node.Name := UTF8Decode(RootQuery.Fields[1]);

          TreeNode.States := [ vsInitialized ] + TreeNode.States;

          // Check for Children
          CountQuery := SQLiteDb.GetTable( Format( 'SELECT COUNT(*) FROM locationcategories WHERE type = 0 AND parent = %u', [ Node.Id ] ) );

          if StrToInt(CountQuery.Fields[0]) > 0 then
            TreeNode.States := [ vsHasChildren ] + TreeNode.States;
          CountQuery.Free;
          RootQuery.Next;
        end;
        RootQuery.Free;

        RootQuery := TSQLiteTable.Create( SQLiteDb, 'SELECT id,name FROM npccategories WHERE type = 0 AND parent = 0 ORDER BY name ASC;' );
        while not RootQuery.EOF do
        begin
          TreeNode := vtNpcCategories.AddChild( nil );
          Node := vtNpcCategories.GetNodeData( TreeNode );
          Node.Id := StrToInt( RootQuery.Fields[0] );
          Node.Name := UTF8Decode(RootQuery.Fields[1]);
          TreeNode.States := [ vsInitialized ] + TreeNode.States;

          // Check for Children
          CountQuery := SQLiteDb.GetTable( Format( 'SELECT COUNT(*) FROM npccategories WHERE type = 0 AND parent = %u', [ Node.Id ] ) );

          if StrToInt(CountQuery.Fields[0]) > 0 then
            TreeNode.States := [ vsHasChildren ] + TreeNode.States;
          CountQuery.Free;
          RootQuery.Next;
        end;
        RootQuery.Free;

        loadMultis;

        {load UO files}
        if openFiles(config) <> 0 then
        begin
          showmessage('Some errors happen while loading files, please verify the config.ini file');
        end;

    except
    	on E: Exception do
        begin
	    	Application.MessageBox(PChar(E.Message), 'Error', MB_OK+MB_ICONERROR );
	        PostQuitMessage( 0 );
	        Exit;
        end;
    end;

  // Load Position
  Left := Config.getInt('Window Position X', Left);
  Top := Config.getInt('Window Position Y', Top);
  Width := Config.getInt('Window Width', Width);
  Height := Config.getInt('Window Height', Height);
  Pages.ActivePageIndex := Config.getInt('Current Page', 0);

  if Left > Screen.Width then
    Left := Screen.Width - Width;

  if Top > Screen.Height then
    Top := Screen.Height - Height;

  for i := 0 to Config.getInt('EventCount', 0) - 1 do
  begin
    Value := Config.getString(Format('Event%u', [i]), '');

    if Value <> '' then
      cbEventName.Items.Add(Value);
  end;

  rebuildBuildMenu;
  loadCustomButtons;
  ontopTimer.Enabled := true;
end;

{
	Handler for WM_DESTROY
}
procedure TfrmMain.FormDestroy(Sender: TObject);
var
  i : integer;
begin
	SQLiteDb.Free;
    Tiledata.Free;

    Art.Close;
    Art.Free;

    Anim.Close;
    Anim.Free;


    for I := 0 to 4 do
    begin
      if i <> 1 then
      begin
        if maps[i] <> nil then
        begin
          Maps[i].Close;
          Maps[i].Free;
        end;

        if Statics[i] <> nil then
        begin
          Statics[i].Close;
          Statics[i].Free;
        end;
      end;
    end;

    RadarCol.Free;
    Hues.Free;

    Config.Save;
    Config.Free;

  UpdateMutex.Free;

  inherited;
end;

procedure TfrmMain.vtCategoriesGetText(Sender: TBaseVirtualTree;
  Node: PVirtualNode; Column: TColumnIndex; TextType: TVSTTextType;
  var CellText: WideString);
begin
  if TextType = ttNormal then
  	CellText := PNode( Sender.GetNodeData( Node ) ).Name;
end;

{
	An Item Category has been selected.
}
procedure TfrmMain.vtCategoriesChange(Sender: TBaseVirtualTree;
  Node: PVirtualNode);
var
	MyData: PNode;
    ItemData: PItemNode;
	Query: TSQLiteTable;
    TreeNode: PVirtualNode;
begin
    vtItems.Clear;
    btnAddItem.Enabled := False;
    btnTileItem.Enabled := False;

    if Node = nil then
        Exit;

    MyData := Sender.GetNodeData(Node);
    try
      Query := TSQLiteTable.Create( SQLiteDb, Format( 'SELECT id,name,artid,addid,color FROM items WHERE parent = %u ORDER BY name;', [ MyData.Id ] ) );
      while not Query.EOF do
      begin
        TreeNode := vtItems.AddChild( nil );
        ItemData := vtItems.GetNodeData( TreeNode );

        ItemData.Id := StrToInt( Query.Fields[0] );
        ItemData.Name := UTF8Decode(Query.Fields[1]);
        ItemData.ArtId := StrToInt( Query.Fields[2] );
        ItemData.AddId := Query.Fields[3];
        ItemData.Color := StrToIntDef( Query.Fields[4], 0);

        Query.Next;
      end;
      Query.Free;
      vtItems.SortTree(vtItems.Header.SortColumn, vtItems.Header.SortDirection);
    except
    	on E: Exception do
        begin
	    	Application.MessageBox( PChar( 'Error retrieving ChildCount: ' + E.Message ), 'Error', MB_OK+MB_ICONERROR );
	        PostQuitMessage( 0 );
	        Exit;
        end;
    end;
end;

procedure TfrmMain.vtItemsGetText(Sender: TBaseVirtualTree;
  Node: PVirtualNode; Column: TColumnIndex; TextType: TVSTTextType;
  var CellText: WideString);
var
	MyData: PItemNode;
begin
	MyData := Sender.GetNodeData( Node );

    case Column of
		0:
        begin
        	if Length( MyData.Name ) = 0 then
            	CellText := Tiledata.getItemName( MyData.ArtId )
            else
		        CellText := MyData.Name;
        end;

        1: CellText := MyData.AddId;
    end;
end;

procedure TfrmMain.vtCategoriesExpanding(Sender: TBaseVirtualTree;
  Node: PVirtualNode; var Allowed: Boolean);
var
	MyData: PNode;
    RootQuery, CountQuery: TSQLiteTable;
    TreeNode: PVirtualNode;
begin
	// Children Already Added
    if ( Sender.ChildCount[ Node ] > 0 ) or not ( vsHasChildren in Node.States ) then
    	exit;

	// Add the Children
    MyData := Sender.GetNodeData( Node );

	try
        RootQuery := TSQLiteTable.Create( SQLiteDb, Format( 'SELECT id,name FROM categories WHERE type = 0 AND parent = %u ORDER BY name ASC;', [ MyData.Id ] ) );

        while not RootQuery.EOF do
        begin
	        TreeNode := vtCategories.AddChild( Node );
            MyData := vtCategories.GetNodeData( TreeNode );
            MyData.Id := StrToInt( RootQuery.Fields[0] );
            MyData.Name := UTF8Decode(RootQuery.Fields[1]);
            TreeNode.States := [ vsInitialized ] + TreeNode.States;

            // Check for Children
            CountQuery := SQLiteDb.GetTable( Format( 'SELECT COUNT(*) FROM categories WHERE type = 0 AND parent = %u', [ MyData.Id ] ) );

			if StrToInt( CountQuery.Fields[0] ) > 0 then
            	TreeNode.States := [ vsHasChildren ] + TreeNode.States;

            CountQuery.Free;

            RootQuery.Next;
        end;

        RootQuery.Free;
    except
    	on E: Exception do
      begin
        Application.MessageBox( PChar( E.Message ), 'Error', MB_OK+MB_ICONERROR );
        PostQuitMessage( 0 );
        Exit;
      end;
    end;
end;

procedure TfrmMain.vtItemsCompareNodes(Sender: TBaseVirtualTree; Node1,
  Node2: PVirtualNode; Column: TColumnIndex; var Result: Integer);
var
	MyData1, MyData2: PItemNode;
begin
	MyData1 := Sender.GetNodeData( Node1 );
	MyData2 := Sender.GetNodeData( Node2 );

    case Column of
        0: Result := CompareStr( MyData1.Name, MyData2.Name );
        1: Result := CompareStr( MyData1.AddId, MyData2.AddId );
    end;
end;

procedure TfrmMain.vtItemsHeaderClick(Sender: TVTHeader;
  Column: TColumnIndex; Button: TMouseButton; Shift: TShiftState; X,
  Y: Integer);
begin
	// When changing we keep our sort direction
    if Sender.SortColumn = Column then
    begin
		if Sender.SortDirection = sdAscending then
        	Sender.SortDirection := sdDescending
        else
            Sender.SortDirection := sdAscending;
        exit;
    end;

  	Sender.SortColumn := Column;
    Sender.SortDirection := sdAscending;
end;

procedure TfrmMain.btnAddItemClick(Sender: TObject);
var
	NodeData: PItemNode;
  Node: PVirtualNode;
begin
  Node := vtItems.RootNode.FirstChild;
  while Node <> nil do
  begin
    if vsSelected in Node.States then
    begin
      NodeData := vtItems.GetNodeData(Node);
      if cbStatic.Checked then
        SendCommand('static ' + NodeData.AddId, true)
      else
        SendCommand('add ' + NodeData.AddId, true);
      exit;
    end;
    Node := Node.NextSibling;
  end;
end;

procedure TfrmMain.vtItemsChange(Sender: TBaseVirtualTree;
  Node: PVirtualNode);
var
	MyItemData: PItemNode;
  btmap: TBitmap32;
begin
  // Display a Preview Item
  if Node <> nil then
  begin
    MyItemData := Sender.GetNodeData(Node);
    btmap := Art.GetTile(MyItemData.ArtId, MyItemData.Color);
    imgItemPreview.Bitmap.SetSizeFrom(btmap);
    imgItemPreview.Bitmap.Clear(Color32(clBtnFace));
    btmap.DrawTo(imgItemPreview.Bitmap, 0, 0);
  end else begin
    imgItemPreview.Bitmap.SetSize(0, 0);
  end;

  btnAddItem.Enabled := False;
  btnTileItem.Enabled := False;

  // See if there is some selected node
  Node := vtItems.RootNode.FirstChild;
  while Node <> nil do
  begin
    if vsSelected in Node.States then
    begin
      btnAddItem.Enabled := True;
      btnTileItem.Enabled := True;
      exit;
    end;
    Node := Node.NextSibling;
  end;
end;

procedure TfrmMain.btnRemoveItemClick(Sender: TObject);
begin
    SendCommand('remove', true);
end;

procedure TfrmMain.cbAlwaysOnTopClick(Sender: TObject);
var
  lastpage: Integer;
begin
  lastpage := Pages.ActivePageIndex;

  if not cbAlwaysOnTop.Checked then
    frmMain.FormStyle := fsNormal
  else
    frmMain.FormStyle := fsStayOnTop;

  Pages.ActivePageIndex := lastpage;
end;

procedure TfrmMain.btnSaveClick(Sender: TObject);
begin
  Config.setBool('Always On Top', cbAlwaysOnTop.Checked);
  Config.setString('Command Prefix', eCommandPrefix.Text);
  Config.Save;
end;

procedure TfrmMain.FormClose(Sender: TObject; var Action: TCloseAction);
var
  i: Integer;
begin
  // Save Position
  Config.setInt('Window Position X', Left);
  Config.setInt('Window Position Y', Top);
  Config.setInt('Window Width', Width);
  Config.setInt('Window Height', Height);
  Config.setInt('Current Page', Pages.ActivePageIndex);

  // Save the history of event names
  Config.setInt('EventCount', cbEventName.Items.Count);
  for i := 0 to cbEventName.Items.Count - 1 do
  begin
    Config.setString(Format('Event%u', [i]), cbEventName.Items[i]);
  end;

  inherited;  
end;

procedure TfrmMain.rebuildBuildMenu;
var
  i: Integer;
begin
  if sbBuild.ControlCount > 0 then begin
    UpdateMutex.Enter;
    BuildCurrentX := 8;
    BuildCurrentY := 8 - sbBuild.VertScrollBar.Position;

    for i := 0 to sbBuild.ControlCount - 1 do begin
      // Reposition
      sbBuild.Controls[i].Left := BuildCurrentX;
      sbBuild.Controls[i].Top := BuildCurrentY;

      Inc(BuildCurrentX, sbBuild.Controls[i].Width + 8);

      // If the next button wouldn't fit anymore, go to the next row
      if BuildCurrentX + sbBuild.Controls[i].Width + 8 > sbBuild.ClientWidth then begin
        BuildCurrentX := 8;
        Inc(BuildCurrentY, sbBuild.Controls[i].Height + 8);
      end;
    end;
    UpdateMutex.Leave;
    exit;
  end;

  BuildCurrentX := 8;
  BuildCurrentY := 8;

  // Add Build Table
  addBuildButton('woodenboardse', $4ab, 'Floor');
  addBuildButton('woodenboardsn', $4b5, 'Floor');

  addBuildButton('woodenplankse', $4c7, 'Floor');
  addBuildButton('woodenplanksn', $4d5, 'Floor');

  addBuildButton('woodenlogse1', $507, 'Floor');
  addBuildButton('woodenlogse2', $505, 'Floor');

  addBuildButton('woodenlogsn1', $4c2, 'Floor');
  addBuildButton('woodenlogsn2', $4c4, 'Floor');

  addBuildButton('redfloor', $4f3, 'Floor');
  addBuildButton('bluefloor', $4f2, 'Floor');

  // Walls
  addBuildButton('wall1', $1c, 'Wall');
  addBuildButton('wall1small', $25, 'Wall');
  addBuildButton('wall1tiny', $2f, 'Wall');
  
  addBuildButton('wall2', $58, 'Wall');
  addBuildButton('wall2small', $5f, 'Wall');
  addBuildButton('wall2smaller', $63, 'Wall');
  addBuildButton('wall2tiny', $69, 'Wall');

  addBuildButton('wall3small', $2ce, 'Wall');

  addBuildButton('wall4', $c8, 'Wall');
  addBuildButton('wall4small', $de, 'Wall');

  addBuildButton('wall5', $1d0, 'Wall');
  addBuildButton('wall5small', $1e9, 'Wall');

  addBuildButton('woodenwall1', $c, 'Wall');
  addBuildButton('woodenwall1small', $12, 'Wall');
  addBuildButton('woodenwall1tiny', $16, 'Wall');
  addBuildButton('woodenwall2', $a8, 'Wall');
  addBuildButton('woodenwall2small', $b7, 'Wall');
  addBuildButton('woodenwall2tiny', $bf, 'Wall');

  addBuildButton('plasterwall1', $12e, 'Wall');
  addBuildButton('plasterwall1wrecked', $389, 'Wall');
  addBuildButton('brickwall', $34, 'Wall');
  addBuildButton('brickwallsmall', $3e, 'Wall');
  addBuildButton('brickwalltiny', $42, 'Wall');

  // Roofs
  addBuildButton('slateroofe', $5a1, 'Roof');
  addBuildButton('slateroofn', $595, 'Roof');
  addBuildButton('stonetileroofe', $193d, 'Roof');
  addBuildButton('stonetileroofn', $193e, 'Roof');
  addBuildButton('thatchroofe', $5a8, 'Roof');
  addBuildButton('thatchroofn', $5a5, 'Roof');
  addBuildButton('tileroofe', $5bf, 'Roof');
  addBuildButton('tileroofn', $5b3, 'Roof');
  addBuildButton('woodenshinglese', $5ce, 'Roof');
  addBuildButton('woodenshinglesn', $5c2, 'Roof');
  addBuildButton('palmroofe', $593, 'Roof');
  addBuildButton('palmroofn', $590, 'Roof');
  addBuildButton('logroofe', $5fc, 'Roof');
  addBuildButton('logroofn', $5f0, 'Roof');
  addBuildButton('hayroofe', $26ea, 'Roof');
  addBuildButton('hayroofn', $26da, 'Roof');
  addBuildButton('woodenslatee', $28d8, 'Roof');
  addBuildButton('woodenslaten', $28c8, 'Roof');
  addBuildButton('barkroofe', $2708, 'Roof');
  addBuildButton('barkroofn', $26f8, 'Roof');

  // Carpets
  addBuildButton('bluecarpet1a', $abe, 'Carpet');
  addBuildButton('bluecarpet1b', $abd, 'Carpet');
  addBuildButton('bluecarpet1c', $abf, 'Carpet');
  addBuildButton('bluecarpet1d', $ac0, 'Carpet');
  addBuildButton('bluecarpet2', $ad1, 'Carpet');
  addBuildButton('bluecarpet3a', $aec, 'Carpet');
  addBuildButton('bluecarpet3b', $aed, 'Carpet');
  addBuildButton('browncarpet', $ada, 'Carpet');

  addBuildButton('redcarpet1a', $ac8, 'Carpet');
  addBuildButton('redcarpet1b', $ac6, 'Carpet');
  addBuildButton('redcarpet1c', $ac7, 'Carpet');
  addBuildButton('redcarpet2', $aeb, 'Carpet');
  addBuildButton('grayrug', $1df7, 'Carpet');
  addBuildButton('greenrug1', $ab3, 'Carpet');
  addBuildButton('greenrug2', $1dea, 'Carpet');
  addBuildButton('greenrug3', $1de9, 'Carpet');

  addBuildButton('bearskinn', $1e37, 'Carpet');
  addBuildButton('bearskine', $1e41, 'Carpet');
  addBuildButton('polarskinn', $1e4a, 'Carpet');
  addBuildButton('polarskine', $1e54, 'Carpet');

  addBuildButton('stonefountain', $1741, 'Fountains');
  addBuildButton('sandstonefountain', $19d3, 'Fountains');
end;

procedure TfrmMain.pbMapPaintBuffer(Sender: TObject);
var
  x, y: Integer;
  mapx, mapy: Integer;
  MapLeft, MapTop: Integer;
  MapCell: TMapCell;
  StaticBlock: TStaticBlock;
  i: Integer;
  RelX, RelY: Byte;
  HighestZ: Shortint;
  Map: TMapReader;
  Static: TStaticReader;
  Width, Height: Word;
  Position: String;
  TextY: Integer;
  Color: TColor;
begin
  MapLeft := - (pbMap.Width div 2);
  MapTop := - (pbMap.Height div 2);

  if MapCenterMap > 4 then
    exit;

  Width := Maps[MapCenterMap].getWidth * 8;
  Height := Maps[MapCenterMap].getHeight * 8;

  Map := Maps[MapCenterMap];
  Static := Statics[MapCenterMap];

  pbMap.Buffer.Clear(clBlack32);
  for x := 0 to pbMap.Width do
  begin
    for y := 0 to pbMap.Height do
    begin
      mapx := Integer(MapCenterX) + x + MapLeft;
      mapy := Integer(MapCenterY) + y + MapTop;

      if (mapx >= Width) or (mapy >= Height) then begin
        pbMap.Buffer.PixelS[x, y] := clBlack32;
        continue;
      end;

      if (mapx >= 0) and (mapy >= 0) then begin
        Map.ReadCell(mapx, mapy, MapCell);
        pbMap.Buffer.PixelS[x, y] := RadarCol.GetLandColor(MapCell.Id);

        RelX := mapx mod 8;
        RelY := mapy mod 8;
        HighestZ := MapCell.Height;
        Static.ReadBlock(mapx div 8, mapy div 8, StaticBlock);
        for i := 0 to Length(StaticBlock) - 1 do begin
          if (StaticBlock[i].X = RelX) and (StaticBlock[i].Y = RelY)
            and (StaticBlock[i].Z >= HighestZ) then begin
            if StaticBlock[i].Hue = 0 then
              pbMap.Buffer.PixelS[x, y] :=
                RadarCol.GetItemColor(StaticBlock[i].Id)
            else begin
              // A little more complicated
              //pbMap.Buffer.PixelS[x, y] := Color32(Color15to24(StaticBlock[i].Hue));
              Color := RadarCol.GetItemColor(StaticBlock[i].Id);
              Color := Hues.translateColor(StaticBlock[i].Hue, Color);
              pbMap.Buffer.PixelS[x, y] := Color;
            end;

            HighestZ := StaticBlock[i].Z;
          end;
        end;

        if (Cardinal(mapx) = MapCenterX) and
          (Cardinal(mapy) = MapCenterY) then begin
          MapCenterZ := HighestZ;
          pbMap.Buffer.PixelS[x, y] := clWhite32;
          continue;
        end;        
      end else begin
        pbMap.Buffer.PixelS[x, y] := clGray32;
      end;
    end;
  end;

  // Show the current location
  Position := Format('%u,%u,%d,%u', [MapCenterX, MapCenterY,
    MapCenterZ, MapCenterMap]);
  TextY := pbMap.Height - 2 - pbMap.Buffer.TextHeight(Position);
  pbMap.Buffer.Font.Color := clWhite;
  pbMap.Buffer.Textout(2, TextY, Position);
end;

procedure TfrmMain.pbMapMouseDown(Sender: TObject; Button: TMouseButton;
  Shift: TShiftState; X, Y: Integer);
var
  RelX, RelY: Integer;
  Width, Height: Cardinal;
begin
  if MapCenterMap > 4 then
    exit;

  if Button <> mbLeft then
    exit;

  Width := Maps[MapCenterMap].getWidth * 8;
  Height := Maps[MapCenterMap].getHeight * 8;

  RelX := X - pbMap.Width div 2;
  RelY := y - pbMap.Height div 2;

  if Integer(MapCenterX) + RelX >= 0 then begin
    MapCenterX := Integer(MapCenterX) + RelX;
  end else begin
    MapCenterX := 0;
  end;

  if Integer(MapCenterY) + RelY >= 0 then begin
    MapCenterY := Integer(MapCenterY) + RelY;
  end else begin
    MapCenterY := 0;
  end;

  if MapCenterX >= Width then
    MapCenterX := Width - 1;

  if MapCenterY >= Height then
    MapCenterY := Height - 1;

  pbMap.Invalidate;
end;

procedure TfrmMain.vtLocCategoriesExpanding(Sender: TBaseVirtualTree;
  Node: PVirtualNode; var Allowed: Boolean);
var
	MyData: PNode;
    RootQuery, CountQuery: TSQLiteTable;
    TreeNode: PVirtualNode;
begin
	// Children Already Added
    if ( Sender.ChildCount[ Node ] > 0 ) or not ( vsHasChildren in Node.States ) then
    	exit;

	// Add the Children
    MyData := Sender.GetNodeData( Node );

	try
        RootQuery := TSQLiteTable.Create( SQLiteDb, Format( 'SELECT id,name FROM locationcategories WHERE type = 0 AND parent = %u ORDER BY name ASC;', [ MyData.Id ] ) );

        while not RootQuery.EOF do
        begin
	        TreeNode := vtLocCategories.AddChild( Node );
            MyData := vtLocCategories.GetNodeData( TreeNode );
            MyData.Id := StrToInt( RootQuery.Fields[0] );
            MyData.Name := UTF8Decode(RootQuery.Fields[1]);
            TreeNode.States := [ vsInitialized ] + TreeNode.States;

            // Check for Children
            CountQuery := SQLiteDb.GetTable( Format( 'SELECT COUNT(*) FROM locationcategories WHERE type = 0 AND parent = %u', [ MyData.Id ] ) );

			if StrToInt( CountQuery.Fields[0] ) > 0 then
            	TreeNode.States := [ vsHasChildren ] + TreeNode.States;

            CountQuery.Free;

            RootQuery.Next;
        end;

        RootQuery.Free;
    except
    	on E: Exception do
        begin
	    	Application.MessageBox( PChar( E.Message ), 'Error', MB_OK+MB_ICONERROR );
	        PostQuitMessage( 0 );
	        Exit;
        end;
    end;
end;


procedure TfrmMain.vtLocCategoriesChange(Sender: TBaseVirtualTree;
  Node: PVirtualNode);
var
  MyData: PNode;
  LocationData: PLocationNode;
  Query: TSQLiteTable;
  TreeNode: PVirtualNode;
begin
 

  vtLocItems.Clear;
  if Node = nil then
      Exit;
  MyData := Sender.GetNodeData(Node);
  try
    Query := TSQLiteTable.Create( SQLiteDb, Format( 'SELECT id,name,posx,posy,posz,posmap,location FROM locations WHERE parent = %u ORDER BY name;', [ MyData.Id ] ) );
    while not Query.EOF do
    begin
      TreeNode := vtLocItems.AddChild( nil );
      LocationData := vtLocItems.GetNodeData(TreeNode);

      LocationData.Id := StrToInt( Query.Fields[0] );
      LocationData.Name := UTF8Decode(Query.Fields[1]);
      LocationData.PosX := StrToInt(Query.Fields[2]);
      LocationData.PosY := StrToInt(Query.Fields[3]);
      LocationData.PosZ := StrToInt(Query.Fields[4]);
      LocationData.PosMap := StrToInt(Query.Fields[5]);
      LocationData.LocId := Query.Fields[6];
      Query.Next;
    end;
    Query.Free;
    vtLocItems.SortTree(vtLocItems.Header.SortColumn, vtLocItems.Header.SortDirection);
  except
      on E: Exception do
      begin
        Application.MessageBox( PChar( 'Error retrieving ChildCount: ' + E.Message ), 'Error', MB_OK+MB_ICONERROR );
        PostQuitMessage( 0 );
        Exit;
      end;
  end;
end;

procedure TfrmMain.vtLocItemsGetText(Sender: TBaseVirtualTree;
  Node: PVirtualNode; Column: TColumnIndex; TextType: TVSTTextType;
  var CellText: WideString);
var
        MyData: PLocationNode;
begin
  myData := Sender.GetNodeData( Node );

        case Column of
                0:
        begin
                CellText := MyData.Name;
        end;
    end;

end;

procedure TfrmMain.vtLocItemsCompareNodes(Sender: TBaseVirtualTree; Node1,
  Node2: PVirtualNode; Column: TColumnIndex; var Result: Integer);
var
	MyData1, MyData2: PLocationNode;
begin
	MyData1 := Sender.GetNodeData( Node1 );
	MyData2 := Sender.GetNodeData( Node2 );
  Result := CompareStr( MyData1.Name, MyData2.Name );
end;

procedure TfrmMain.vtLocItemsHeaderClick(Sender: TVTHeader;
  Column: TColumnIndex; Button: TMouseButton; Shift: TShiftState; X,
  Y: Integer);
begin
  // When changing we keep our sort direction
  if Sender.SortDirection = sdAscending then
    Sender.SortDirection := sdDescending
  else
    Sender.SortDirection := sdAscending;
end;

procedure TfrmMain.vtLocItemsChange(Sender: TBaseVirtualTree;
  Node: PVirtualNode);
var
  LocationData: PLocationNode;
begin
  // Display a Preview Item
  if Node <> nil then begin
    LocationData := Sender.GetNodeData(Node);
    MapCenterX := LocationData.PosX;
    MapCenterY := LocationData.PosY;
    MapCenterZ := LocationData.PosZ;
    MapCenterMap := LocationData.PosMap;
    pbMap.Invalidate;
  end;
end;

procedure TfrmMain.Go1Click(Sender: TObject);
begin
  SendCommand(Format('go %u,%u,%d,%u',
    [MapCenterX, MapCenterY, MapCenterZ, MapCenterMap]));
end;

procedure TfrmMain.btnSendClick(Sender: TObject);
begin
  SendCommand(Format('send %u,%u,%d,%u',
    [MapCenterX, MapCenterY, MapCenterZ, MapCenterMap]));
end;

procedure TfrmMain.btnCmdAllmoveClick(Sender: TObject);
begin
  SendCommand('allmove');
end;

procedure TfrmMain.btnCmdAllshowClick(Sender: TObject);
begin
  SendCommand('allshow');
end;

procedure TfrmMain.btnCmdInvisibleClick(Sender: TObject);
begin
  SendCommand('invis');
end;

procedure TfrmMain.btnCmdStaffClick(Sender: TObject);
begin
  SendCommand('staff');
end;

procedure TfrmMain.btnCmdInvulClick(Sender: TObject);
begin
  SendCommand('invul');
end;

procedure TfrmMain.btnCmdShutdownClick(Sender: TObject);
begin
  if Application.MessageBox('Do you really want to close the Wolfpack server?',
  'Shutdown', MB_YESNO+MB_ICONQUESTION) = IDYES then
    SendCommand('shutdown');
end;

procedure TfrmMain.btnCmdReloadAllClick(Sender: TObject);
begin
  SendCommand('reload all');
end;

procedure TfrmMain.btnCmdReloadConfigClick(Sender: TObject);
begin
  SendCommand('reload configuration');
end;

procedure TfrmMain.btnCmdReloadAccountsClick(Sender: TObject);
begin
  SendCommand('reload accounts');
end;

procedure TfrmMain.btnCmdReloadPythonClick(Sender: TObject);
begin
  SendCommand('reload python');
end;

procedure TfrmMain.btnCmdReloadScriptsClick(Sender: TObject);
begin
  SendCommand('reload scripts');
end;

procedure TfrmMain.btnCmdDyeClick(Sender: TObject);
begin
  SendCommand(Format('dye %u', [StrToIntDef(eItemHue.Text, 0)]), True);
end;

procedure TfrmMain.pbHuePreviewPaintBuffer(Sender: TObject);
var
  i: Integer;
  Width: Integer;
  Hue: Word;
begin
  Hue := StrToIntDef(eItemHue.Text, 0);

  if Hue = 0 then begin
    pbHuePreview.Buffer.Clear(clGray32);
    exit;
  end;

  Width := Ceil(pbHuePreview.Width / 32.0);
  for i := 0 to 31 do
  begin
    pbHuePreview.Buffer.FillRect(i * Width, 0, (i + 1) * Width,
      pbHuePreview.Height, Hues.getColor(Hue, i));
  end;
end;

procedure TfrmMain.eItemHueChange(Sender: TObject);
begin
  pbHuePreview.Invalidate;
end;

procedure TfrmMain.btnChangeItemHueClick(Sender: TObject);
begin
  frmChooseHue.ShowModal;

  if frmChooseHue.ModalResult = mrOk then
  begin
    eItemHue.Text := '0x' + IntToHex(frmChooseHue.ColorId, 0);
    pbHuePreview.Invalidate;
  end;
end;

procedure TfrmMain.btnCmdDupeClick(Sender: TObject);
begin
  SendCommand('dupe', True);
end;

procedure TfrmMain.btnCmdInfoClick(Sender: TObject);
begin
  SendCommand('info', True);
end;

procedure TfrmMain.btnCmdRemoveClick(Sender: TObject);
begin
  SendCommand('remove', True);
end;

procedure TfrmMain.btnCmdLockClick(Sender: TObject);
begin
  SendCommand('lock', True);
end;

procedure TfrmMain.btnCmdNukeClick(Sender: TObject);
begin
  SendCommand('nuke', True);
end;

procedure TfrmMain.btnCmdItemSetClick(Sender: TObject);
begin
  SendCommand(Format('set %s %s', [cbItemProperty.Text, eItemPropValue.Text]), True);
end;

procedure TfrmMain.btnCmdItemShowClick(Sender: TObject);
begin
  SendCommand(Format('show %s', [cbItemProperty.Text]), True);
end;

procedure TfrmMain.cbItemPropertyChange(Sender: TObject);
begin
  btnCmdItemSet.Enabled := Length(cbItemProperty.Text) <> 0;
  btnCmdItemShow.Enabled := Length(cbItemProperty.Text) <> 0;
end;

procedure TfrmMain.btnCmdItemTileColorClick(Sender: TObject);
begin
  SendCommand('tilecolor ' + eItemHue.Text, true);
end;

procedure TfrmMain.btnTagInfoClick(Sender: TObject);
begin
  SendCommand('taginfo', True);
end;

procedure TfrmMain.eTagNameChange(Sender: TObject);
begin
  btnTagSet.Enabled := Length(eTagName.Text) > 0;
  btnTagShow.Enabled := btnTagSet.Enabled;
  btnTagDelete.Enabled := btnTagSet.Enabled;
end;

procedure TfrmMain.btnTagSetClick(Sender: TObject);
var
  TagType: String;
begin
  TagType := 'string';
  if cbTagType.ItemIndex = 1 then
    TagType := 'int'
  else if cbTagType.ItemIndex = 2 then
    TagType := 'float';

  if AnsiStrPos(PAnsiChar(eTagName.Text), ' ') <> nil then begin
    Application.MessageBox('Tag names may not contain spaces.', 'Error',
      MB_ICONERROR+MB_OK);
    exit;
  end;

  SendCommand(format('settag %s %s %s',
  [eTagName.Text, TagType, eTagValue.Text]), True);
end;

procedure TfrmMain.btnTagShowClick(Sender: TObject);
begin
  if AnsiStrPos(PAnsiChar(eTagName.Text), ' ') <> nil then begin
    Application.MessageBox('Tag names may not contain spaces.', 'Error',
      MB_ICONERROR+MB_OK);
    exit;
  end;

  SendCommand(format('gettag %s', [eTagName.Text]), True);
end;

procedure TfrmMain.btnTagDeleteClick(Sender: TObject);
begin
  if AnsiStrPos(PAnsiChar(eTagName.Text), ' ') <> nil then begin
    Application.MessageBox('Tag names may not contain spaces.', 'Error',
      MB_ICONERROR+MB_OK);
    exit;
  end;

  SendCommand(format('deltag %s', [eTagName.Text]), True);
end;

procedure TfrmMain.btnCmdSendOtherClick(Sender: TObject);
begin
  SendCommand('send', True);
end;

procedure TfrmMain.Button2Click(Sender: TObject);
begin
  SendCommand('tele', True);
end;

procedure TfrmMain.btnCmdMoveClick(Sender: TObject);
begin
  SendCommand(format('move %d,%d,%d',
  [StrToIntDef(eMoveX.Text, 0), StrToIntDef(eMoveY.Text, 0),
  StrToIntDef(eMoveZ.Text, 0)]), True);
end;

procedure TfrmMain.btnCmdMoveUpClick(Sender: TObject);
begin
  SendCommand('move 0,0,1', True);
end;

procedure TfrmMain.btnCmdMoveDownClick(Sender: TObject);
begin
  SendCommand('move 0,0,-1', True);
end;

procedure TfrmMain.btnTileItemClick(Sender: TObject);
var
	NodeData: PItemNode;
  Node: PVirtualNode;
  Ids: String;
begin
  Node := vtItems.RootNode.FirstChild;
  Ids := '';

  while Node <> nil do
  begin
    if vsSelected in Node.States then
    begin
      NodeData := vtItems.GetNodeData(Node);
      if Length(Ids) > 0 then
        Ids := Ids + ',' + NodeData.AddId
      else
        Ids := NodeData.AddId;
    end;
    Node := Node.NextSibling;
  end;

  SendCommand(format('tile %d %s',
  [StrToIntDef(eZLevel.Text, 0), Ids]), True);
end;

procedure TfrmMain.btnCmdShowEventlistClick(Sender: TObject);
begin
  SendCommand('show scriptlist', True);
end;

procedure TfrmMain.eEventNameChange(Sender: TObject);
begin
  btnCmdAddEvent.Enabled := Length(cbEventName.Text) > 0;
  btnCmdRemoveEvent.Enabled := Length(cbEventName.Text) > 0;
end;

procedure TfrmMain.btnCmdAddEventClick(Sender: TObject);
var
  i: Integer;
begin
  SendCommand(format('addscript %s', [cbEventName.Text]), True);

  // Add the text to the history if it's not already in there
  for i := 0 to cbEventName.Items.Count - 1 do
  begin
    if cbEventName.Items[i] = cbEventName.Text then
      exit;
  end;

  cbEventName.Items.Add(cbEventName.Text);
end;

procedure TfrmMain.btnCmdRemoveEventClick(Sender: TObject);
var
  i: Integer;
begin
  SendCommand(format('removescript %s', [cbEventName.Text]), True);

  // Add the text to the history if it's not already in there
  for i := 0 to cbEventName.Items.Count - 1 do
  begin
    if cbEventName.Items[i] = cbEventName.Text then
      exit;
  end;

  cbEventName.Items.Add(cbEventName.Text);  
end;

procedure TfrmMain.btnCmdWhereClick(Sender: TObject);
begin
  SendCommand('where');
end;

procedure TfrmMain.btnCmdServertimeClick(Sender: TObject);
begin
  SendCommand('servertime');
end;

procedure TfrmMain.vtNpcCategoriesExpanding(Sender: TBaseVirtualTree;
  Node: PVirtualNode; var Allowed: Boolean);
var
	MyData: PNode;
    RootQuery, CountQuery: TSQLiteTable;
    TreeNode: PVirtualNode;
begin
	// Children Already Added
    if ( Sender.ChildCount[ Node ] > 0 ) or not ( vsHasChildren in Node.States ) then
    	exit;

	// Add the Children
    MyData := Sender.GetNodeData( Node );

	try
        RootQuery := TSQLiteTable.Create( SQLiteDb, Format( 'SELECT id,name FROM npccategories WHERE type = 0 AND parent = %u ORDER BY name ASC;', [ MyData.Id ] ) );

        while not RootQuery.EOF do
        begin
	        TreeNode := vtNpcCategories.AddChild( Node );
            MyData := vtNpcCategories.GetNodeData( TreeNode );
            MyData.Id := StrToInt( RootQuery.Fields[0] );
            MyData.Name := UTF8Decode(RootQuery.Fields[1]);
            TreeNode.States := [ vsInitialized ] + TreeNode.States;

            // Check for Children
            CountQuery := SQLiteDb.GetTable( Format( 'SELECT COUNT(*) FROM npccategories WHERE type = 0 AND parent = %u', [ MyData.Id ] ) );

			if StrToInt( CountQuery.Fields[0] ) > 0 then
            	TreeNode.States := [ vsHasChildren ] + TreeNode.States;

            CountQuery.Free;

            RootQuery.Next;
        end;

        RootQuery.Free;
    except
    	on E: Exception do
      begin
        Application.MessageBox( PChar( E.Message ), 'Error', MB_OK+MB_ICONERROR );
        PostQuitMessage( 0 );
        Exit;
      end;
    end;
end;

procedure TfrmMain.vtNpcCategoriesChange(Sender: TBaseVirtualTree;
  Node: PVirtualNode);
var
	MyData: PNode;
  ItemData: PNpcNode;
	Query, SubQuery: TSQLiteTable;
  TreeNode: PVirtualNode;
  i: Integer;
begin
    vtNpcs.Clear;
    btnAddNpc.Enabled := False;
    btnNpcSpawn.Enabled := False;

    if Node = nil then
        Exit;

    MyData := Sender.GetNodeData(Node);
    try
      Query := TSQLiteTable.Create( SQLiteDb, Format( 'SELECT id,name,bodyid,addid,skin FROM npcs WHERE parent = %u ORDER BY name;', [ MyData.Id ] ) );
      while not Query.EOF do
      begin
        TreeNode := vtNpcs.AddChild( nil );
        ItemData := vtNpcs.GetNodeData( TreeNode );

        ItemData.Id := StrToInt( Query.Fields[0] );
        ItemData.Name := UTF8Decode(Query.Fields[1]);
        ItemData.BodyId := StrToInt( Query.Fields[2] );
        ItemData.AddId := Query.Fields[3];
        ItemData.Skin := StrToIntDef( Query.Fields[4], 0);

        SubQuery := TSQLiteTable.Create(SQLiteDb, Format('SELECT layer,artid,color FROM npcequipment WHERE id = %u ORDER BY layer DESC', [ ItemData.Id ] ) );

        while not SubQuery.Eof do
        begin
          i := Length(ItemData.Equipment);
          SetLength(ItemData.Equipment, i + 1);
          ItemData.Equipment[i].Layer := StrToInt( SubQuery.Fields[0] );
          ItemData.Equipment[i].Anim := StrToInt( SubQuery.Fields[1] );
          ItemData.Equipment[i].Color := StrToInt( SubQuery.Fields[2] );
          SubQuery.Next;
        end;

        SubQuery.Free;

        Query.Next;
      end;
      Query.Free;
      vtNpcs.SortTree(vtNpcs.Header.SortColumn, vtNpcs.Header.SortDirection);
    except
    	on E: Exception do
        begin
	    	Application.MessageBox( PChar( 'Error retrieving ChildCount: ' + E.Message ), 'Error', MB_OK+MB_ICONERROR );
	        PostQuitMessage( 0 );
	        Exit;
        end;
    end;
end;

procedure TfrmMain.vtNpcsCompareNodes(Sender: TBaseVirtualTree; Node1,
  Node2: PVirtualNode; Column: TColumnIndex; var Result: Integer);
var
	MyData1, MyData2: PNpcNode;
begin
	MyData1 := Sender.GetNodeData( Node1 );
	MyData2 := Sender.GetNodeData( Node2 );

    case Column of
        0: Result := CompareStr( MyData1.Name, MyData2.Name );
        1: Result := CompareStr( MyData1.AddId, MyData2.AddId );
    end;

end;

procedure TfrmMain.vtNpcsGetText(Sender: TBaseVirtualTree;
  Node: PVirtualNode; Column: TColumnIndex; TextType: TVSTTextType;
  var CellText: WideString);
var
	MyData: PNpcNode;
begin
	MyData := Sender.GetNodeData( Node );

    case Column of
		    0: CellText := MyData.Name;
        1: CellText := MyData.AddId;
    end;
end;

procedure TfrmMain.btnRemoveNpcClick(Sender: TObject);
begin
    SendCommand('remove', true);
end;

procedure TfrmMain.btnAddNpcClick(Sender: TObject);
var
	NodeData: PNpcNode;
  Node: PVirtualNode;
begin
  Node := vtNpcs.RootNode.FirstChild;
  while Node <> nil do
  begin
    if vsSelected in Node.States then
    begin
      NodeData := vtNpcs.GetNodeData(Node);
      SendCommand('add ' + NodeData.AddId, true);
      exit;
    end;
    Node := Node.NextSibling;
  end;
end;

procedure TfrmMain.vtNpcsChange(Sender: TBaseVirtualTree;
  Node: PVirtualNode);
var
	MyItemData: PNpcNode;
  frames: TAnimation;
  i, j: Integer;

  const LayerOrder: Array[0..21] of Byte = ( 20, 5, 4, 3, 24, 23, 13, 19, 17, 22, 12, 14, 8, 7, 1, 2, 10, 16, 11, 18, 6, 21 );
begin
  animtimer.Enabled := false;

  for i := 0 to Length(npcPreview) - 1 do
    npcPreview[i].Free;

  SetLength(npcPreview, 0);

  // Display a Preview Item
  if Node <> nil then
  begin
    MyItemData := Sender.GetNodeData(Node);

    frames := Anim.GetFrames(MyItemData.BodyId, 1, MyItemData.Skin);
    
    if frames <> nil then begin
      SetLength(npcPreview, 1);
      npcPreview[0] := frames;

      // Add Equipment
      for j := 0 to Length(LayerOrder) - 1 do begin
        for i := 0 to Length(MyItemData.Equipment) - 1 do begin
          if MyItemData.Equipment[i].Layer = LayerOrder[j] then begin
            frames := Anim.GetFrames(MyItemData.Equipment[i].Anim, 1, MyItemData.Equipment[i].Color);
            if frames <> nil then begin
              SetLength(npcPreview, Length(npcPreview) + 1);
              npcPreview[Length(npcPreview) - 1] := frames;
            end;
          end;
        end;
      end;

      npcPreviewFrame := 0;
      pbNpcPreview.Invalidate;
      animtimer.Interval := npcPreview[0].frameDelay;
      animtimer.Enabled := true;
    end;

  end else begin
    pbNpcPreview.Invalidate;
  end;

  btnAddNpc.Enabled := False;
  btnNpcSpawn.Enabled := False;

  // See if there is some selected node
  Node := vtNpcs.RootNode.FirstChild;
  while Node <> nil do
  begin
    if vsSelected in Node.States then
    begin
      btnAddNpc.Enabled := True;
      btnNpcSpawn.Enabled := True;
      exit;
    end;
    Node := Node.NextSibling;
  end;
end;

procedure TfrmMain.vtNpcsHeaderClick(Sender: TVTHeader;
  Column: TColumnIndex; Button: TMouseButton; Shift: TShiftState; X,
  Y: Integer);
begin
	// When changing we keep our sort direction
    if Sender.SortColumn = Column then
    begin
		if Sender.SortDirection = sdAscending then
        	Sender.SortDirection := sdDescending
        else
            Sender.SortDirection := sdAscending;
        exit;
    end;

  	Sender.SortColumn := Column;
    Sender.SortDirection := sdAscending;
end;

procedure TFrmMain.pixelCombine(F: TColor32; var B: TColor32; M: TColor32);
begin

end;

procedure TfrmMain.pbNpcPreviewPaintBuffer(Sender: TObject);
var
  pb: TPaintBox32;
  totalHeight, totalWidth, i, xpos, ypos: Integer;
  frame: PAnimFrame;
begin
  if not (Sender is TPaintBox32) then
    exit;

  pb := Sender as TPaintBox32;
  pb.Buffer.Clear(clGray32);

  // Draw Centered
  if (npcPreview <> nil) and (length(npcPreview) > 0) then begin
    // Measure Total Height and Width
    totalHeight := 0;
    totalWidth := 0;

    for i := 0 to Length(npcPreview) - 1 do begin
      totalHeight := Max(npcPreview[i].getTotalHeight, totalHeight);
      totalWidth := Max(npcPreview[i].getTotalWidth, totalWidth);
    end;

    xpos := (pb.ClientWidth div 2);
    ypos := (pb.ClientHeight div 2) + (totalHeight div 2);

    for i := 0 to Length(npcPreview) - 1 do begin
      frame := npcPreview[i].getFrame(npcPreviewFrame);

      if frame <> nil then begin
        pb.Buffer.Draw(xpos - frame.CenterX, ypos - frame.image.height - frame.CenterY, frame.Image);
      end;
    end;
  end;
end;

procedure TfrmMain.animtimerTimer(Sender: TObject);
begin
  if (npcPreview <> nil) and (npcPreview[0].frameCount > 0) then begin
    npcPreviewFrame := (npcPreviewFrame + 1) mod npcPreview[0].frameCount;
    pbNpcPreview.Invalidate;
  end;
end;

procedure TfrmMain.BitFilesClick(Sender: TObject);
Var
   prgr : TfrmConfig;
begin
  Prgr := TfrmConfig.Create(self);
  Prgr.FormStyle := fsStayOnTop;
  prgr.Position :=  poScreenCenter;
  Prgr.Show;
end;

procedure TfrmMain.loadMultis;
var
  Query: TSQLiteTable;
  Node: PVirtualNode;
  Data: PMultiNode;
begin
  try
    Query := TSQLiteTable.Create(SQLiteDb, 'SELECT id,name,addid,multiid FROM multis;');

    while not Query.EOF do begin
      Node := vtMultis.AddChild(nil);
      Data := vtMultis.GetNodeData(Node);

      Data.Id := StrToInt(Query.Fields[0]);
      Data.Name := UTF8Decode(Query.Fields[1]);
      Data.AddId := Query.Fields[2];
      Data.MultiId := StrToInt(Query.Fields[0]);

      Query.Next;
    end;

    Query.Free;
    vtMultis.SortTree(vtMultis.Header.SortColumn, vtMultis.Header.SortDirection);
  except
    	on E: Exception do
	    	Application.MessageBox(PChar(E.Message), 'Error', MB_OK+MB_ICONERROR ); 
  end;
end;

procedure TfrmMain.vtMultisGetText(Sender: TBaseVirtualTree;
  Node: PVirtualNode; Column: TColumnIndex; TextType: TVSTTextType;
  var CellText: WideString);
var
	MyData: PMultiNode;
begin
	MyData := Sender.GetNodeData( Node );

    case Column of
		    0: CellText := MyData.Name;
        1: CellText := MyData.AddId;
    end;
end;

procedure TfrmMain.vtMultisCompareNodes(Sender: TBaseVirtualTree; Node1,
  Node2: PVirtualNode; Column: TColumnIndex; var Result: Integer);
var
	MyData1, MyData2: PMultiNode;
begin
	MyData1 := Sender.GetNodeData( Node1 );
	MyData2 := Sender.GetNodeData( Node2 );

    case Column of
        0: Result := CompareStr( MyData1.Name, MyData2.Name );
        1: Result := CompareStr( MyData1.AddId, MyData2.AddId );
    end;
end;

procedure TfrmMain.vtMultisHeaderClick(Sender: TVTHeader;
  Column: TColumnIndex; Button: TMouseButton; Shift: TShiftState; X,
  Y: Integer);
begin
	// When changing we keep our sort direction
    if Sender.SortColumn = Column then
    begin
		if Sender.SortDirection = sdAscending then
        	Sender.SortDirection := sdDescending
        else
            Sender.SortDirection := sdAscending;
        exit;
    end;

  	Sender.SortColumn := Column;
    Sender.SortDirection := sdAscending;
end;

procedure TfrmMain.vtMultisChange(Sender: TBaseVirtualTree;
  Node: PVirtualNode);
begin
  if Sender.SelectedCount < 1 then begin
    btnAddMulti.Enabled := False;
    btnPreviewMulti.Enabled := False;
  end else begin
    btnAddMulti.Enabled := True;
    btnPreviewMulti.Enabled := True;
  end;
end;

procedure TfrmMain.btnPreviewMultiClick(Sender: TObject);
begin
  Application.MessageBox('Not yet implemented', 'Error', MB_OK+MB_ICONERROR);
end;

procedure TfrmMain.btnAddMultiClick(Sender: TObject);
var
	NodeData: PMultiNode;
  Node: PVirtualNode;
begin
  Node := vtMultis.RootNode.FirstChild;
  while Node <> nil do
  begin
    if vsSelected in Node.States then
    begin
      NodeData := vtMultis.GetNodeData(Node);
      SendCommand('add ' + NodeData.AddId, true);
      exit;
    end;
    Node := Node.NextSibling;
  end;
end;

procedure TfrmMain.Felucca1Click(Sender: TObject);
begin
  MapCenterX := 0;
  MapCenterY := 0;
  MapCenterZ := 0;
  MapCenterMap := 0;
  Felucca1.Checked := True;
  Trammel1.Checked := False;
  Ilshenar1.Checked := False;
  Malas1.Checked := False;
  SamuraiEmpire1.Checked := False;
  pbMap.Invalidate;
end;

procedure TfrmMain.Trammel1Click(Sender: TObject);
begin
  MapCenterX := 0;
  MapCenterY := 0;
  MapCenterZ := 0;
  MapCenterMap := 1;
  Felucca1.Checked := False;
  Trammel1.Checked := True;
  Ilshenar1.Checked := False;
  Malas1.Checked := False;
  SamuraiEmpire1.Checked := False;
  pbMap.Invalidate;
end;

procedure TfrmMain.Ilshenar1Click(Sender: TObject);
begin
  MapCenterX := 0;
  MapCenterY := 0;
  MapCenterZ := 0;
  MapCenterMap := 2;
  Felucca1.Checked := False;
  Trammel1.Checked := False;
  Ilshenar1.Checked := True;
  Malas1.Checked := False;
  SamuraiEmpire1.Checked := False;
  pbMap.Invalidate;
end;

procedure TfrmMain.Malas1Click(Sender: TObject);
begin
  MapCenterX := 0;
  MapCenterY := 0;
  MapCenterZ := 0;
  MapCenterMap := 3;
  Felucca1.Checked := False;
  Trammel1.Checked := False;
  Ilshenar1.Checked := False;
  Malas1.Checked := True;
  SamuraiEmpire1.Checked := False;
  pbMap.Invalidate;
end;

procedure TfrmMain.Center1Click(Sender: TObject);
begin
  frmCenter.Show;
  frmCenter.X.Text := IntToStr(MapCenterX);
  frmCenter.Y.Text := IntToStr(MapCenterY);
  frmCenter.Map.ItemIndex := MapCenterMap;
end;

procedure TfrmMain.SamuraiEmpire1Click(Sender: TObject);
begin
  MapCenterX := 0;
  MapCenterY := 0;
  MapCenterZ := 0;
  MapCenterMap := 4;
  Felucca1.Checked := False;
  Trammel1.Checked := False;
  Ilshenar1.Checked := False;
  Malas1.Checked := False;
  SamuraiEmpire1.Checked := True;
  pbMap.Invalidate;
end;

procedure TfrmMain.btnStaticItemClick(Sender: TObject);
var
	NodeData: PItemNode;
  Node: PVirtualNode;
begin
  Node := vtItems.RootNode.FirstChild;
  while Node <> nil do
  begin
    if vsSelected in Node.States then
    begin
      NodeData := vtItems.GetNodeData(Node);
      SendCommand('static ' + NodeData.AddId, true);
      exit;
    end;
    Node := Node.NextSibling;
  end;

end;

procedure TfrmMain.btnMapOverviewClick(Sender: TObject);
begin
  if not frmOverview.Generating then begin
    frmOverview.showMap(MapCenterMap);
    frmOverview.Show;
  end;
end;

function IsInParent(Control: TWinControl): Boolean;
begin
  if Control is TCustomForm then
    Result := True
  else if Control.Parent <> nil then
    Result := IsInParent(Control.Parent)
  else
    Result := False;
end;

procedure TfrmMain.ontopTimerTimer(Sender: TObject);
begin
  try
  // If one of the forms is active, skip this code
    if frmMain.Active or frmCenter.Active or frmOverview.Active then
      exit;

    if IsInParent(Screen.ActiveControl) then
      exit;

    if cbAlwaysOnTop.Checked then begin
      if frmMain.Visible then begin
        SetWindowPos(frmMain.Handle, HWND_TOPMOST, 0, 0, 0, 0, 3 or SWP_NOACTIVATE);
      end;

      if frmCenter.Visible then begin
        SetWindowPos(frmCenter.Handle, HWND_TOPMOST, 0, 0, 0, 0, 3 or SWP_NOACTIVATE);
      end;

      if frmOverview.Visible then begin
        SetWindowPos(frmOverview.Handle, HWND_TOPMOST, 0, 0, 0, 0, 3 or SWP_NOACTIVATE);
      end;
    end;
  except
        showmessage('error');
  end;
end;

procedure TfrmMain.btnNpcSpawnClick(Sender: TObject);
var
	NodeData: PNpcNode;
  Node: PVirtualNode;
begin
  Node := vtNpcs.RootNode.FirstChild;
  while Node <> nil do
  begin
    if vsSelected in Node.States then
    begin
      NodeData := vtNpcs.GetNodeData(Node);
      SendCommand(Format('spawn %s %s %s %s', [NodeData.AddId, eSpawnRadius.Text, eSpawnMin.Text, eSpawnMax.Text]), True);
      exit;
    end;
    Node := Node.NextSibling;
  end;
end;

procedure TfrmMain.eSpawnRadiusExit(Sender: TObject);
begin
  Config.setString('SpawnRadius', eSpawnRadius.Text);
  Config.Save;
end;

procedure TfrmMain.eSpawnMinExit(Sender: TObject);
begin
  Config.setString('SpawnMin', eSpawnMin.Text);
  Config.Save;
end;

procedure TfrmMain.eSpawnMaxExit(Sender: TObject);
begin
  Config.setString('SpawnMax', eSpawnMax.Text);
  Config.Save;
end;

procedure TfrmMain.btnRegionsClick(Sender: TObject);
begin
  frmRegions.Show;
end;

procedure TfrmMain.Button1Click(Sender: TObject);
begin
  frmPatternColor.Show;
end;

procedure TFrmMain.addBuildButton(Id: String; ItemId: Word; Text: String);
var
  BitBtn: TBitBtn;
begin
  BitBtn := TBitBtn.Create(sbBuild);
  BitBtn.SetBounds(BuildCurrentX, BuildCurrentY, 70, 120);
//  BitBtn.Caption := Text;
  BitBtn.Glyph.Assign(Art.GetTile(ItemId));
  //BitBtn.Layout := blGlyphTop;
  BitBtn.Hint := id;
  BitBtn.OnClick := clickBuildButton;
  sbBuild.InsertControl(BitBtn);

  Inc(BuildCurrentX, 70 + 8);

  // If the next button wouldn't fit anymore, go to the next row
  if BuildCurrentX + 70 + 8 > sbBuild.ClientWidth then begin
    BuildCurrentX := 8;
    Inc(BuildCurrentY, 120 + 8);
  end;
end;

procedure TfrmMain.sbBuildResize(Sender: TObject);
begin
  rebuildBuildMenu;
end;

procedure TfrmMain.clickBuildButton(Sender: TObject);
var
  BitBtn: TBitBtn;
begin
  if not (Sender is TBitBtn) then
    exit;

  BitBtn := Sender as TBitBtn;
  SendCommand(Format('build %s %d', [BitBtn.Hint, StrToIntDef(cbBuildZ.Text, 0)]), True);      
end;

procedure TfrmMain.sbBuildClick(Sender: TObject);
begin
  sbBuild.SetFocus;
end;

procedure TfrmMain.sbBuildMouseWheelDown(Sender: TObject;
  Shift: TShiftState; MousePos: TPoint; var Handled: Boolean);
begin
  SendMessage(sbBuild.Handle, WM_VSCROLL, SB_LINEDOWN, 0);
  Handled := True;
end;

procedure TfrmMain.sbBuildMouseWheelUp(Sender: TObject; Shift: TShiftState;
  MousePos: TPoint; var Handled: Boolean);
begin
  SendMessage(sbBuild.Handle, WM_VSCROLL, SB_LINEUP, 0);
  Handled := True;
end;

procedure TfrmMain.reorderCustomButtons;
var
  currentx, currenty, i: Integer;
begin
  currentx := 8;
  currenty := 8;

  for i := 0 to tsCustom.ControlCount - 1 do begin
    tsCustom.Controls[i].Left := currentx;
    tsCustom.Controls[i].Top := currenty;
    Inc(currenty, tsCustom.Controls[i].Height + 8);

    if CurrentY + tsCustom.Controls[i].Height > tsCustom.ClientHeight then begin
      CurrentY := 8;
      inc(CurrentX, tsCustom.Controls[i].Width + 8);
    end;
  end;
end;

procedure TfrmMain.loadCustomButtons;
var
  count, i: Integer;
  command: String;
  button: TButton;
begin
  count := Config.getInt('CustomButtonCount', 0);

  // Load the custom buttons
  for i := 0 to count - 1 do begin
    command := Config.getString(Format('CustomButton%u', [i]), '');

    button := TButton.Create(tsCustom);
    button.SetBounds(0, 0, 129, 25);
    button.Caption := command;
    button.OnClick := customButtonClick;
    button.OnMouseDown := customButtonMouseDown;
    button.PopupMenu := btnCustomPopup;
    button.Parent := tsCustom;
  end;

  // If no button at all was loaded, create an empty one to make
  // adding buttons possible. (RightClick -> Add Button)
  if Count = 0 then begin
    button := TButton.Create(tsCustom);
    button.SetBounds(0, 0, 129, 25);
    button.Caption := '';
    button.OnClick := customButtonClick;
    button.OnMouseDown := customButtonMouseDown;
    button.PopupMenu := btnCustomPopup;
    button.Parent := tsCustom;
  end;

  reorderCustomButtons;
end;

procedure TfrmMain.saveCustomButtons;
var
  count, i: Integer;
  button: TButton;
begin
  count := 0;

  // Account for all custom buttons
  for i := 0 to tsCustom.ControlCount - 1 do begin
    button := tsCustom.Controls[i] as TButton;
    if button <> nil then begin
      Config.setString(Format('CustomButton%u', [i]), Button.Caption);
      Inc(Count);
    end;
  end;

  Config.setInt('CustomButtonCount', count);
end;

procedure TfrmMain.customButtonClick(Sender: TObject);
var
  button: TButton;
begin
  button := sender as TButton;

  if button <> nil then begin
    // If it's empty, show the change gump instead
    if button.caption = '' then begin
      changeCustomButton(button);
    end else begin
      SendCommand(button.caption, True);
    end;
  end;
end;

procedure TfrmMain.customButtonMouseDown(Sender: TObject;
  Button: TMouseButton; Shift: TShiftState; X, Y: Integer);
var
  i, index: Integer;
begin
  if Button = mbRight then begin
    index := -1;

    // Find the index of this button in the custom ctrl
    for i := 0 to tsCustom.ControlCount - 1 do begin
      if tsCustom.Controls[i] = Sender then begin
        index := i;
        break;
      end;
    end;

    btnCustomPopup.Tag := index;
  end;

  inherited;
end;

procedure TfrmMain.Remove1Click(Sender: TObject);
var
  index: Integer;
begin
  index := btnCustomPopup.Tag;

  if (index <> -1) and (index < tsCustom.ControlCount) then begin
    tsCustom.Controls[index].Free;
    btnCustomPopup.Tag := -1;
    reorderCustomButtons;    
    saveCustomButtons;
  end;
end;

procedure TFrmMain.changeCustomButton(Button: TButton; NewButton: Boolean = False);
var
  value: String;
begin
  value := Button.Caption;
  // Query the new caption from the user
  if InputQuery('Change Custom Button', 'Please input the command to associate with this button:', value) then
  begin
    Button.Caption := value;
    saveCustomButtons;
  end;
end;

procedure TfrmMain.Change1Click(Sender: TObject);
var
  button: TButton;
  index: Integer;
begin
  index := btnCustomPopup.Tag;

  if (index <> -1) and (index < tsCustom.ControlCount) then begin
    button := tsCustom.Controls[index] as TButton;
    if button <> nil then
      changeCustomButton(button, False);
  end;
end;

procedure TfrmMain.AddButton1Click(Sender: TObject);
var
  button: TButton;
begin
    button := TButton.Create(tsCustom);
    button.SetBounds(0, 0, 129, 25);
    button.Caption := '';
    button.OnClick := customButtonClick;
    button.OnMouseDown := customButtonMouseDown;
    button.PopupMenu := btnCustomPopup;
    button.Parent := tsCustom;

    reorderCustomButtons;
    saveCustomButtons;
end;

procedure TfrmMain.tsCustomResize(Sender: TObject);
begin
  reorderCustomButtons;
end;

end.

