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
    Left = 161
    Top = 488
    Width = 96
    Height = 13
    Anchors = [akLeft, akBottom]
    Caption = 'Screen Coordinates:'
    ExplicitTop = 468
  end
  object Label2: TLabel
    Left = 417
    Top = 488
    Width = 83
    Height = 13
    Anchors = [akLeft, akBottom]
    Caption = 'Map Coordinates:'
    ExplicitTop = 468
  end
  object lblScreen: TLabel
    Left = 264
    Top = 488
    Width = 3
    Height = 13
    Anchors = [akLeft, akBottom]
    ExplicitTop = 468
  end
  object lblMap: TLabel
    Left = 512
    Top = 488
    Width = 3
    Height = 13
    Anchors = [akLeft, akBottom]
    ExplicitTop = 468
  end
  object Label3: TLabel
    Left = 624
    Top = 264
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
  end
  object lblSelected: TLabel
    Left = 624
    Top = 283
    Width = 3
    Height = 13
    Anchors = [akTop, akRight]
  end
  object cbMap: TComboBox
    Left = 8
    Top = 484
    Width = 145
    Height = 21
    Style = csDropDownList
    Anchors = [akLeft, akBottom]
    ItemHeight = 13
    ItemIndex = 0
    TabOrder = 0
    Text = 'Felucca'
    Items.Strings = (
      'Felucca'
      'Trammel'
      'Ilshenar'
      'Malas'
      'Tokuno Islands')
    ExplicitTop = 464
  end
  object pbMap: TPaintBox32
    Left = 3
    Top = 28
    Width = 597
    Height = 433
    Anchors = [akLeft, akTop, akRight, akBottom]
    TabOrder = 1
    OnMouseDown = pbMapMouseDown
    OnMouseMove = pbMapMouseMove
    OnMouseUp = pbMapMouseUp
    OnPaintBuffer = pbMapPaintBuffer
  end
  object sbX: TScrollBar
    Left = 4
    Top = 460
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
    Top = 28
    Width = 17
    Height = 432
    Anchors = [akTop, akRight, akBottom]
    Kind = sbVertical
    LargeChange = 50
    PageSize = 0
    TabOrder = 3
    OnScroll = sbYScroll
  end
  object lbRects: TListBox
    Left = 624
    Top = 28
    Width = 121
    Height = 198
    Anchors = [akTop, akRight]
    ItemHeight = 13
    TabOrder = 4
    OnDblClick = lbRectsDblClick
    OnKeyDown = lbRectsKeyDown
  end
  object btnAdd: TButton
    Left = 624
    Top = 232
    Width = 57
    Height = 25
    Anchors = [akTop, akRight]
    Caption = 'Add'
    Enabled = False
    TabOrder = 5
    OnClick = btnAddClick
  end
  object btnRemove: TButton
    Left = 688
    Top = 232
    Width = 57
    Height = 25
    Anchors = [akTop, akRight]
    Caption = 'Remove'
    TabOrder = 6
    OnClick = btnRemoveClick
  end
  object Button1: TButton
    Left = 624
    Top = 304
    Width = 121
    Height = 25
    Anchors = [akTop, akRight]
    Caption = 'Copy as Spawnregion'
    TabOrder = 7
    Visible = False
    OnClick = Button1Click
  end
  object Button2: TButton
    Left = 624
    Top = 336
    Width = 121
    Height = 25
    Anchors = [akTop, akRight]
    Caption = 'Copy as Region'
    TabOrder = 8
    Visible = False
    OnClick = Button2Click
  end
  object Button3: TButton
    Left = 624
    Top = 368
    Width = 121
    Height = 25
    Anchors = [akTop, akRight]
    Caption = 'Paste Spawnregion'
    TabOrder = 9
    OnClick = Button3Click
  end
  object Button4: TButton
    Left = 624
    Top = 400
    Width = 121
    Height = 25
    Anchors = [akTop, akRight]
    Caption = 'Paste Region'
    TabOrder = 10
    OnClick = Button4Click
  end
  object btnZoomOut: TButton
    Left = 624
    Top = 452
    Width = 41
    Height = 25
    Anchors = [akRight, akBottom]
    Caption = '-'
    TabOrder = 11
    OnClick = btnZoomOutClick
  end
  object btnZoomIn: TButton
    Left = 672
    Top = 452
    Width = 41
    Height = 25
    Anchors = [akRight, akBottom]
    Caption = '+'
    TabOrder = 12
    OnClick = btnZoomInClick
  end
  object btnRedraw: TButton
    Left = 624
    Top = 484
    Width = 89
    Height = 25
    Anchors = [akRight, akBottom]
    Caption = 'Redraw'
    TabOrder = 13
    OnClick = btnRedrawClick
  end
  object ToolBar1: TToolBar
    Left = 0
    Top = 0
    Width = 756
    Height = 22
    ButtonHeight = 19
    ButtonWidth = 79
    Caption = 'ToolBar1'
    List = True
    AllowTextButtons = True
    TabOrder = 14
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
      ImageIndex = 1
      Style = tbsTextButton
      OnClick = ToolButton2Click
    end
    object ToolButton3: TToolButton
      Left = 146
      Top = 0
      Caption = 'Export Region'
      ImageIndex = 2
      Style = tbsTextButton
      OnClick = ToolButton3Click
    end
  end
  object CDSNpc: TClientDataSet
    Active = True
    Aggregates = <>
    Params = <>
    Left = 568
    Top = 4
    Data = {
      7C0000009619E0BD0100000018000000060000000000030000007C0002496402
      0049000000010005574944544802000200FF000A57616E646572547970650200
      49000000010005574944544802000200FF000278310400010000000000027931
      0400010000000000027832040001000000000002793204000100000000000000}
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
  end
end
