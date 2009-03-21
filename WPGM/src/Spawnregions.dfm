object frmRegions: TfrmRegions
  Left = 214
  Top = 135
  Caption = 'Regions'
  ClientHeight = 515
  ClientWidth = 756
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  Position = poDesktopCenter
  OnCreate = FormCreate
  OnDestroy = FormDestroy
  OnResize = FormResize
  OnShow = FormShow
  DesignSize = (
    756
    515)
  PixelsPerInch = 96
  TextHeight = 13
  object Label1: TLabel
    Left = 159
    Top = 494
    Width = 96
    Height = 13
    Anchors = [akLeft, akBottom]
    Caption = 'Screen Coordinates:'
  end
  object Label2: TLabel
    Left = 365
    Top = 494
    Width = 83
    Height = 13
    Anchors = [akLeft, akBottom]
    Caption = 'Map Coordinates:'
  end
  object lblScreen: TLabel
    Left = 261
    Top = 494
    Width = 3
    Height = 13
    Anchors = [akLeft, akBottom]
  end
  object lblMap: TLabel
    Left = 460
    Top = 494
    Width = 3
    Height = 13
    Anchors = [akLeft, akBottom]
  end
  object Label6: TLabel
    Left = 624
    Top = 42
    Width = 42
    Height = 13
    Anchors = [akTop, akRight]
    Caption = 'Regions:'
  end
  object Label7: TLabel
    Left = 624
    Top = 269
    Width = 57
    Height = 13
    Anchors = [akTop, akRight]
    Caption = 'Rectangles:'
  end
  object cbMap: TComboBox
    Left = 8
    Top = 488
    Width = 145
    Height = 21
    Style = csDropDownList
    Anchors = [akLeft, akBottom]
    ItemHeight = 13
    ItemIndex = 0
    TabOrder = 0
    Text = 'Felucca'
    OnChange = cbMapChange
    Items.Strings = (
      'Felucca'
      'Trammel'
      'Ilshenar'
      'Malas'
      'Tokuno Islands')
  end
  object pbMap: TPaintBox32
    Left = 3
    Top = 42
    Width = 597
    Height = 423
    Anchors = [akLeft, akTop, akRight, akBottom]
    TabOrder = 1
    OnMouseDown = pbMapMouseDown
    OnMouseMove = pbMapMouseMove
    OnMouseUp = pbMapMouseUp
    OnPaintBuffer = pbMapPaintBuffer
  end
  object sbX: TScrollBar
    Left = 3
    Top = 465
    Width = 597
    Height = 17
    Anchors = [akLeft, akRight, akBottom]
    LargeChange = 50
    PageSize = 0
    TabOrder = 2
    OnScroll = sbXScroll
  end
  object sbY: TScrollBar
    Left = 601
    Top = 42
    Width = 17
    Height = 423
    Anchors = [akTop, akRight, akBottom]
    Kind = sbVertical
    LargeChange = 50
    PageSize = 0
    TabOrder = 3
    OnScroll = sbYScroll
  end
  object lbRects: TListBox
    Left = 624
    Top = 284
    Width = 124
    Height = 160
    Anchors = [akTop, akRight]
    ItemHeight = 13
    TabOrder = 4
    OnDblClick = lbRectsDblClick
    OnKeyDown = lbRectsKeyDown
  end
  object btnAdd: TButton
    Left = 624
    Top = 444
    Width = 61
    Height = 25
    Anchors = [akTop, akRight]
    Caption = 'Add'
    Enabled = False
    TabOrder = 5
    OnClick = btnAddClick
  end
  object btnRemove: TButton
    Left = 688
    Top = 444
    Width = 60
    Height = 25
    Anchors = [akTop, akRight]
    Caption = 'Remove'
    TabOrder = 6
    OnClick = btnRemoveClick
  end
  object btnZoomOut: TButton
    Left = 624
    Top = 488
    Width = 30
    Height = 25
    Anchors = [akRight, akBottom]
    Caption = '-'
    TabOrder = 7
    OnClick = btnZoomOutClick
  end
  object btnZoomIn: TButton
    Left = 656
    Top = 488
    Width = 29
    Height = 25
    Anchors = [akRight, akBottom]
    Caption = '+'
    TabOrder = 8
    OnClick = btnZoomInClick
  end
  object btnRedraw: TButton
    Left = 688
    Top = 488
    Width = 60
    Height = 25
    Anchors = [akRight, akBottom]
    Caption = 'Redraw'
    TabOrder = 9
    OnClick = btnRedrawClick
  end
  object ToolBar1: TToolBar
    Left = 0
    Top = 0
    Width = 756
    Height = 22
    ButtonHeight = 19
    ButtonWidth = 104
    Caption = 'ToolBar1'
    List = True
    AllowTextButtons = True
    TabOrder = 10
    object ToolButton1: TToolButton
      Left = 0
      Top = 0
      AutoSize = True
      Caption = 'New Region'
      ImageIndex = 0
      Style = tbsTextButton
      OnClick = ToolButton1Click
    end
    object ToolButton2: TToolButton
      Left = 75
      Top = 0
      Caption = 'Edit Region'
      Enabled = False
      ImageIndex = 1
      Style = tbsTextButton
      OnClick = ToolButton2Click
    end
    object ToolButton3: TToolButton
      Left = 146
      Top = 0
      Caption = 'Export Region'
      Enabled = False
      ImageIndex = 2
      Style = tbsTextButton
      OnClick = ToolButton3Click
    end
    object ToolButton4: TToolButton
      Left = 229
      Top = 0
      Width = 8
      Caption = 'ToolButton4'
      ImageIndex = 3
      Style = tbsSeparator
    end
    object ToolButton5: TToolButton
      Left = 237
      Top = 0
      Caption = 'Paste Spawnregion'
      Enabled = False
      ImageIndex = 3
      Style = tbsTextButton
      OnClick = ToolButton5Click
    end
    object ToolButton6: TToolButton
      Left = 345
      Top = 0
      Caption = 'Paste Region'
      Enabled = False
      ImageIndex = 4
      Style = tbsTextButton
      OnClick = ToolButton6Click
    end
  end
  object Panel1: TPanel
    Left = 0
    Top = 23
    Width = 756
    Height = 17
    Anchors = [akLeft, akTop, akRight]
    BevelInner = bvLowered
    Color = clSilver
    ParentBackground = False
    TabOrder = 11
    DesignSize = (
      756
      17)
    object Label3: TLabel
      Left = 401
      Top = 1
      Width = 55
      Height = 13
      Anchors = [akTop, akRight]
      Caption = 'Selected:'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'MS Sans Serif'
      Font.Style = [fsBold]
      ParentFont = False
      Transparent = True
    end
    object Label5: TLabel
      Left = 103
      Top = 1
      Width = 3
      Height = 13
      Transparent = True
    end
    object Label4: TLabel
      Left = 2
      Top = 1
      Width = 97
      Height = 13
      Caption = 'Region/Spawn : '
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'MS Sans Serif'
      Font.Style = [fsBold]
      ParentFont = False
      Transparent = True
    end
    object lblSelected: TLabel
      Left = 462
      Top = 1
      Width = 3
      Height = 13
      Anchors = [akTop, akRight]
      Transparent = True
    end
  end
  object DBGrid1: TDBGrid
    Left = 624
    Top = 58
    Width = 124
    Height = 179
    Anchors = [akTop, akRight]
    DataSource = DSRegion
    TabOrder = 12
    TitleFont.Charset = DEFAULT_CHARSET
    TitleFont.Color = clWindowText
    TitleFont.Height = -11
    TitleFont.Name = 'MS Sans Serif'
    TitleFont.Style = []
    Columns = <
      item
        Expanded = False
        FieldName = 'Name'
        Title.Caption = 'Region'
        Width = 56
        Visible = True
      end
      item
        Expanded = False
        FieldName = 'type'
        PickList.Strings = (
          'R'
          'S')
        Title.Caption = 'Type'
        Width = 29
        Visible = True
      end>
  end
  object Button1: TButton
    Left = 624
    Top = 238
    Width = 124
    Height = 25
    Anchors = [akTop, akRight]
    Caption = 'Remove'
    Enabled = False
    TabOrder = 13
    OnClick = btnRemoveClick
  end
  object CDSRegion: TClientDataSet
    Active = True
    Aggregates = <>
    Params = <>
    Left = 524
    Data = {
      340100009619E0BD01000000180000000F000000000003000000340102496404
      00010000000000044E616D65020049000000010005574944544802000200FF00
      086D6964696C697374020049000000010005574944544802000200FF00077265
      736F726573020049000000010005574944544802000200FF0007736E6F776D69
      6E040001000000000007736E6F776D61780400010000000000077261696E6D69
      6E0400010000000000077261696E6D617804000100000000000A67756172646F
      776E6572020049000000010005574944544802000200FF000474797065020003
      0000000000066163746976650200030000000000066D61786E70630400010000
      0000000864656C61796D696E04000100000000000864656C61796D6178040001
      0000000000086E70636379636C6504000100000000000000}
    object CDSRegionId: TIntegerField
      AutoGenerateValue = arAutoInc
      FieldName = 'Id'
    end
    object CDSRegionName: TStringField
      FieldName = 'Name'
      Size = 255
    end
    object CDSRegionmidilist: TStringField
      FieldName = 'midilist'
      Size = 255
    end
    object CDSRegionresores: TStringField
      FieldName = 'resores'
      Size = 255
    end
    object CDSRegionsnowmin: TIntegerField
      FieldName = 'snowmin'
    end
    object CDSRegionsnowmax: TIntegerField
      FieldName = 'snowmax'
    end
    object CDSRegionrainmin: TIntegerField
      FieldName = 'rainmin'
    end
    object CDSRegionrainmax: TIntegerField
      FieldName = 'rainmax'
    end
    object CDSRegionguardowner: TStringField
      FieldName = 'guardowner'
      Size = 255
    end
    object CDSRegiontype: TBooleanField
      FieldName = 'type'
    end
    object CDSRegionactive: TBooleanField
      FieldName = 'active'
    end
    object CDSRegionmaxnpc: TIntegerField
      FieldName = 'maxnpc'
    end
    object CDSRegiondelaymin: TIntegerField
      FieldName = 'delaymin'
    end
    object CDSRegiondelaymax: TIntegerField
      FieldName = 'delaymax'
    end
    object CDSRegionnpccycle: TIntegerField
      FieldName = 'npccycle'
    end
  end
  object CDSNpc: TClientDataSet
    Active = True
    Aggregates = <>
    IndexFieldNames = 'RegionID'
    MasterFields = 'Id'
    MasterSource = DSRegion
    PacketRecords = 0
    Params = <>
    Left = 488
    Top = 60
    Data = {
      8D0000009619E0BD0100000018000000070000000000030000008D0002496402
      0049000000010005574944544802000200FF000A57616E646572547970650200
      49000000010005574944544802000200FF000278310400010000000000027931
      0400010000000000027832040001000000000002793204000100000000000852
      6567696F6E494404000100000000000000}
    object CDSNpcId: TStringField
      FieldName = 'Id'
      Size = 255
    end
    object CDSNpcWanderType: TStringField
      FieldName = 'WanderType'
      Size = 255
    end
    object CDSNpcx1: TIntegerField
      FieldName = 'x1'
    end
    object CDSNpcy1: TIntegerField
      FieldName = 'y1'
    end
    object CDSNpcx2: TIntegerField
      FieldName = 'x2'
    end
    object CDSNpcy2: TIntegerField
      FieldName = 'y2'
    end
    object CDSNpcRegionID: TIntegerField
      FieldName = 'RegionID'
    end
  end
  object CDSFlags: TClientDataSet
    Active = True
    Aggregates = <>
    IndexFieldNames = 'RegionId'
    MasterFields = 'Id'
    MasterSource = DSRegion
    PacketRecords = 0
    Params = <>
    Left = 524
    Top = 60
    Data = {
      440000009619E0BD010000001800000002000000000003000000440004466C61
      67020049000000010005574944544802000200FF0008526567696F6E49640400
      0100000000000000}
    object CDSFlagsFlag: TStringField
      FieldName = 'Flag'
      Size = 255
    end
    object CDSFlagsRegionId: TIntegerField
      FieldName = 'RegionId'
    end
  end
  object CDSGroup: TClientDataSet
    Active = True
    Aggregates = <>
    IndexFieldNames = 'RegionId'
    MasterFields = 'Id'
    MasterSource = DSRegion
    PacketRecords = 0
    Params = <>
    Left = 560
    Top = 60
    Data = {
      450000009619E0BD01000000180000000200000000000300000045000547726F
      7570020049000000010005574944544802000200FF0008526567696F6E496404
      000100000000000000}
    object CDSGroupGroup: TStringField
      FieldName = 'Group'
      Size = 255
    end
    object CDSGroupRegionId: TIntegerField
      FieldName = 'RegionId'
    end
  end
  object DSRegion: TDataSource
    AutoEdit = False
    DataSet = CDSRegion
    Left = 524
    Top = 28
  end
end
