object frmRegions: TfrmRegions
  Left = 214
  Top = 135
  AutoScroll = False
  Caption = 'Regions'
  ClientHeight = 495
  ClientWidth = 756
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  OnCreate = FormCreate
  OnDestroy = FormDestroy
  OnResize = FormResize
  OnShow = FormShow
  DesignSize = (
    756
    495)
  PixelsPerInch = 96
  TextHeight = 13
  object Label1: TLabel
    Left = 161
    Top = 468
    Width = 96
    Height = 13
    Anchors = [akLeft, akBottom]
    Caption = 'Screen Coordinates:'
  end
  object Label2: TLabel
    Left = 417
    Top = 468
    Width = 83
    Height = 13
    Anchors = [akLeft, akBottom]
    Caption = 'Map Coordinates:'
  end
  object lblScreen: TLabel
    Left = 264
    Top = 468
    Width = 3
    Height = 13
    Anchors = [akLeft, akBottom]
  end
  object lblMap: TLabel
    Left = 512
    Top = 468
    Width = 3
    Height = 13
    Anchors = [akLeft, akBottom]
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
    Top = 280
    Width = 3
    Height = 13
    Anchors = [akTop, akRight]
  end
  object cbMap: TComboBox
    Left = 8
    Top = 464
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
  end
  object pbMap: TPaintBox32
    Left = 3
    Top = 8
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
    Top = 440
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
    Top = 8
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
    Top = 8
    Width = 121
    Height = 217
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
    Top = 432
    Width = 41
    Height = 25
    Anchors = [akRight, akBottom]
    Caption = '-'
    TabOrder = 11
    OnClick = btnZoomOutClick
  end
  object btnZoomIn: TButton
    Left = 672
    Top = 432
    Width = 41
    Height = 25
    Anchors = [akRight, akBottom]
    Caption = '+'
    TabOrder = 12
    OnClick = btnZoomInClick
  end
  object btnRedraw: TButton
    Left = 624
    Top = 464
    Width = 89
    Height = 25
    Anchors = [akRight, akBottom]
    Caption = 'Redraw'
    TabOrder = 13
    OnClick = btnRedrawClick
  end
end
