unit RegionEditS;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, ExtCtrls, StdCtrls, Buttons, DB, Grids, DBGrids, GR32_Image,
  VirtualTrees, GR32, SQLite3, SQLiteTable3, UOAnim, uConfig;

type
  TFrmRegionEditS = class(TForm)
    GroupBox2: TGroupBox;
    BitBtn1: TBitBtn;
    BitBtn2: TBitBtn;
    Panel1: TPanel;
    Label1: TLabel;
    ERegionName: TEdit;
    Label2: TLabel;
    Label3: TLabel;
    Label4: TLabel;
    Label5: TLabel;
    Label6: TLabel;
    SpeedButton1: TSpeedButton;
    Label7: TLabel;
    SpeedButton2: TSpeedButton;
    CheckBox1: TCheckBox;
    EMaxnpcamount: TEdit;
    Edelaymin: TEdit;
    Edelaymax: TEdit;
    Enpcspercycle: TEdit;
    ListBox1: TListBox;
    Edit1: TEdit;
    Panel2: TPanel;
    Splitter1: TSplitter;
    Splitter2: TSplitter;
    vtNpcCategories: TVirtualStringTree;
    vtNpcs: TVirtualStringTree;
    pbNpcPreview: TPaintBox32;
    GroupBox1: TGroupBox;
    Label8: TLabel;
    Label9: TLabel;
    Label10: TLabel;
    Label11: TLabel;
    ComboBox1: TComboBox;
    Ex1: TEdit;
    Ey1: TEdit;
    Ex2: TEdit;
    Ey2: TEdit;
    DBGNpc: TDBGrid;
    Badd: TButton;
    bdel: TButton;
    DataSource1: TDataSource;
    animtimer: TTimer;
    procedure ComboBox1Change(Sender: TObject);
     procedure vtNpcCategoriesChange(Sender: TBaseVirtualTree;
      Node: PVirtualNode);
    procedure vtNpcsChange(Sender: TBaseVirtualTree; Node: PVirtualNode);
    procedure vtNpcsCompareNodes(Sender: TBaseVirtualTree; Node1,
      Node2: PVirtualNode; Column: TColumnIndex; var Result: Integer);
    procedure vtNpcsGetText(Sender: TBaseVirtualTree; Node: PVirtualNode;
      Column: TColumnIndex; TextType: TVSTTextType; var CellText: WideString);
    procedure vtNpcsHeaderClick(Sender: TVTHeader; Column: TColumnIndex;
      Button: TMouseButton; Shift: TShiftState; X, Y: Integer);
    procedure pbNpcPreviewPaintBuffer(Sender: TObject);
    procedure FormCreate(Sender: TObject);
    procedure FormClose(Sender: TObject; var Action: TCloseAction);
    procedure vtNpcCategoriesExpanding(Sender: TBaseVirtualTree;
      Node: PVirtualNode; var Allowed: Boolean);
    procedure vtNpcCategoriesGetText(Sender: TBaseVirtualTree;
      Node: PVirtualNode; Column: TColumnIndex; TextType: TVSTTextType;
      var CellText: WideString);
    procedure animtimerTimer(Sender: TObject);
    procedure BaddClick(Sender: TObject);
    procedure SpeedButton1Click(Sender: TObject);
    procedure SpeedButton2Click(Sender: TObject);
    procedure bdelClick(Sender: TObject);
    procedure BitBtn1Click(Sender: TObject);
    procedure BitBtn2Click(Sender: TObject);
  private
    { Private declarations }
  public
    { Public declarations }
  end;



var
  FrmRegionEditS: TFrmRegionEditS;
  npcPreview: Array of TAnimation;
  npcPreviewFrame: Integer;
  selectedId : string;


implementation

uses Main,Math, Spawnregions;

{$R *.dfm}

procedure TFrmRegionEditS.animtimerTimer(Sender: TObject);
begin
  if (npcPreview <> nil) and (npcPreview[0].frameCount > 0) then begin
    npcPreviewFrame := (npcPreviewFrame + 1) mod npcPreview[0].frameCount;
    pbNpcPreview.Invalidate;
  end;
end;
  


procedure TFrmRegionEditS.BaddClick(Sender: TObject);
begin
  if combobox1.ItemIndex = -1 then
  begin
    showmessage('You must select a wandertype!');
    exit;
  end;

  frmRegions.CDSNpc.Append;
  frmregions.CDSNpcId.AsString := selectedId;
  frmregions.CDSNpcWanderType.AsString := combobox1.Items.Strings[combobox1.ItemIndex];
  frmregions.CDSNpcx1.AsString := ex1.Text;
  frmregions.CDSNpcy1.AsString := ey1.Text;
  frmregions.CDSNpcx2.AsString := ex2.Text;
  frmregions.CDSNpcy2.AsString := ey1.Text;
  frmregions.CDSNpc.Post;

end;

procedure TFrmRegionEditS.bdelClick(Sender: TObject);
begin
  if not frmRegions.CDSNpc.Eof then
    frmregions.cdsnpc.Delete;

end;

procedure TFrmRegionEditS.BitBtn1Click(Sender: TObject);
begin
  frmRegions.regionname := Eregionname.Text;

  if frmRegions.group = nil then
    frmRegions.group := tstringList.Create
  else
    frmRegions.group.Clear;

  frmRegions.group.AddStrings(listbox1.Items);


  frmregions.active := active;
  frmregions.maxnpcamount := strtoint(emaxnpcamount.Text);
  frmregions.delaymin := strtoint(edelaymin.Text);
  frmregions.delaymax := strtoint(edelaymax.Text);
  frmregions.npcspercycle := strtoint(enpcspercycle.Text);
  close;

end;

procedure TFrmRegionEditS.BitBtn2Click(Sender: TObject);
begin
  close;
end;

procedure TFrmRegionEditS.ComboBox1Change(Sender: TObject);
begin
    case combobox1.ItemIndex  of
      0: begin
            label8.Visible := true;
            label9.Visible := true;
            label10.Visible := true;
            label11.Visible := true;
            ex1.Visible := true;
            ey1.Visible := true;
            ex2.Visible := true;
            ey2.Visible := true;
         end;
      1: begin
            label8.Visible := true;
            label9.Visible := false;
            label10.Visible := false;
            label11.Visible := false;
            ex1.Visible := true;
            ey1.Visible := false;
            ex2.Visible := false;
            ey2.Visible := false;
         end;
      2: begin
            label8.Visible := false;
            label9.Visible := false;
            label10.Visible := false;
            label11.Visible := false;
            ex1.Visible := false;
            ey1.Visible := false;
            ex2.Visible := false;
            ey2.Visible := false;
          end;
      3: begin
            label8.Visible := false;
            label9.Visible := false;
            label10.Visible := false;
            label11.Visible := false;
            ex1.Visible := false;
            ey1.Visible := false;
            ex2.Visible := false;
            ey2.Visible := false;

         end;
    end;
end;

procedure TFrmRegionEditS.FormClose(Sender: TObject; var Action: TCloseAction);
begin
  cafree;
end;

procedure TFrmRegionEditS.FormCreate(Sender: TObject);
var
  Config: TConfig;
      RootQuery: TSQLiteTable;
    CountQuery: TSQLiteTable;
    Node: PNode;
    TreeNode: PVirtualNode;
begin
    frmRegions.CDSNpc.Open;

    npcPreview := nil;
    npcPreviewFrame := 0;
    Config := TConfig.Create;
    Config.Load(ExtractFilePath( Application.ExeName ) + 'config.ini');

    vtNpcCategories.NodeDataSize := sizeof(TNode);

    vtNpcs.NodeDataSize := sizeof( TNpcNode );


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

        //RootQuery.Free;

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

    except
    	on E: Exception do
        begin
	    	Application.MessageBox(PChar(E.Message), 'Error', MB_OK+MB_ICONERROR );
	        PostQuitMessage( 0 );
	        Exit;
        end;
    end;
 end;

procedure TFrmRegionEditS.pbNpcPreviewPaintBuffer(Sender: TObject);
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

procedure TFrmRegionEditS.SpeedButton1Click(Sender: TObject);
begin
  if length(trim(edit1.Text)) <> 0 then
    listbox1.Items.Add(edit1.text);
end;

procedure TFrmRegionEditS.SpeedButton2Click(Sender: TObject);
begin
    if listbox1.ItemIndex <> -1 then
    listbox1.DeleteSelected;
end;

procedure TFrmRegionEditS.vtNpcCategoriesChange(Sender: TBaseVirtualTree;
  Node: PVirtualNode);
var
	MyData: PNode;
  ItemData: PNpcNode;
	Query, SubQuery: TSQLiteTable;
  TreeNode: PVirtualNode;
  i: Integer;
begin
    vtNpcs.Clear;

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

procedure TFrmRegionEditS.vtNpcCategoriesExpanding(Sender: TBaseVirtualTree;
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


procedure TFrmRegionEditS.vtNpcCategoriesGetText(Sender: TBaseVirtualTree;
  Node: PVirtualNode; Column: TColumnIndex; TextType: TVSTTextType;
  var CellText: WideString);
begin
  if TextType = ttNormal then
  	CellText := PNode( Sender.GetNodeData( Node ) ).Name;
end;

procedure TFrmRegionEditS.vtNpcsChange(Sender: TBaseVirtualTree;
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
    selectedId := Myitemdata.AddId;

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


  // See if there is some selected node
  Node := vtNpcs.RootNode.FirstChild;
  while Node <> nil do
  begin
    if vsSelected in Node.States then
    begin
      exit;
    end;
    Node := Node.NextSibling;
  end;
end;

procedure TFrmRegionEditS.vtNpcsCompareNodes(Sender: TBaseVirtualTree; Node1,
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

procedure TFrmRegionEditS.vtNpcsGetText(Sender: TBaseVirtualTree;
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

procedure TFrmRegionEditS.vtNpcsHeaderClick(Sender: TVTHeader;
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



end.
