object FrmRegionW4S: TFrmRegionW4S
  Left = 387
  Top = 257
  Caption = 'Region Wizard - Spawnregion - Npcs'
  ClientHeight = 310
  ClientWidth = 454
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  Position = poDesktopCenter
  OnClose = FormClose
  OnCreate = FormCreate
  PixelsPerInch = 96
  TextHeight = 13
  object Button2: TButton
    Left = 361
    Top = 277
    Width = 75
    Height = 25
    Caption = '&Cancel'
    TabOrder = 0
    OnClick = Button2Click
  end
  object Button1: TButton
    Left = 272
    Top = 277
    Width = 75
    Height = 25
    Caption = '&Next'
    TabOrder = 1
    OnClick = Button1Click
  end
  object Button3: TButton
    Left = 184
    Top = 277
    Width = 75
    Height = 25
    Caption = '&Back'
    TabOrder = 2
  end
  object Panel1: TPanel
    Left = 0
    Top = 4
    Width = 453
    Height = 125
    TabOrder = 3
    object Splitter1: TSplitter
      Left = 157
      Top = 1
      Height = 123
      ExplicitLeft = 202
      ExplicitHeight = 112
    end
    object Splitter2: TSplitter
      Left = 337
      Top = 1
      Height = 123
      Align = alRight
      ExplicitLeft = 336
    end
    object vtNpcCategories: TVirtualStringTree
      Left = 1
      Top = 1
      Width = 156
      Height = 123
      Align = alLeft
      Header.AutoSizeIndex = 0
      Header.Font.Charset = DEFAULT_CHARSET
      Header.Font.Color = clWindowText
      Header.Font.Height = -11
      Header.Font.Name = 'MS Sans Serif'
      Header.Font.Style = []
      Header.MainColumn = -1
      Header.Options = [hoColumnResize, hoDrag]
      IncrementalSearchStart = ssAlwaysStartOver
      ScrollBarOptions.AlwaysVisible = True
      TabOrder = 0
      TreeOptions.PaintOptions = [toShowButtons, toShowRoot, toShowTreeLines, toThemeAware, toUseBlendedImages]
      OnChange = vtNpcCategoriesChange
      OnExpanding = vtNpcCategoriesExpanding
      OnGetText = vtNpcCategoriesGetText
      Columns = <>
    end
    object vtNpcs: TVirtualStringTree
      Left = 160
      Top = 1
      Width = 177
      Height = 123
      Align = alClient
      Header.AutoSizeIndex = 0
      Header.Font.Charset = DEFAULT_CHARSET
      Header.Font.Color = clWindowText
      Header.Font.Height = -11
      Header.Font.Name = 'MS Sans Serif'
      Header.Font.Style = []
      Header.Options = [hoColumnResize, hoDrag, hoShowSortGlyphs, hoVisible]
      Header.SortColumn = 0
      ScrollBarOptions.AlwaysVisible = True
      ScrollBarOptions.ScrollBars = ssVertical
      TabOrder = 1
      TreeOptions.AutoOptions = [toAutoDropExpand, toAutoScrollOnExpand, toAutoSort, toAutoTristateTracking, toAutoDeleteMovedNodes, toAutoFreeOnCollapse]
      TreeOptions.PaintOptions = [toHideFocusRect, toShowButtons, toThemeAware, toUseBlendedImages]
      TreeOptions.SelectionOptions = [toDisableDrawSelection, toFullRowSelect]
      OnChange = vtNpcsChange
      OnCompareNodes = vtNpcsCompareNodes
      OnGetText = vtNpcsGetText
      OnHeaderClick = vtNpcsHeaderClick
      Columns = <
        item
          Position = 0
          Width = 100
          WideText = 'Name'
        end
        item
          Position = 1
          WideText = 'ID'
        end>
    end
    object pbNpcPreview: TPaintBox32
      Left = 340
      Top = 1
      Width = 112
      Height = 123
      Align = alRight
      TabOrder = 2
      OnPaintBuffer = pbNpcPreviewPaintBuffer
    end
  end
  object GroupBox1: TGroupBox
    Left = 2
    Top = 134
    Width = 451
    Height = 50
    Caption = 'Npc Wander'
    TabOrder = 4
    object Label1: TLabel
      Left = 162
      Top = 22
      Width = 16
      Height = 13
      Caption = 'x1:'
    end
    object Label2: TLabel
      Left = 229
      Top = 22
      Width = 16
      Height = 13
      Caption = 'y1:'
    end
    object Label3: TLabel
      Left = 300
      Top = 22
      Width = 16
      Height = 13
      Caption = 'x2:'
    end
    object Label4: TLabel
      Left = 369
      Top = 22
      Width = 16
      Height = 13
      Caption = 'y2:'
    end
    object ComboBox1: TComboBox
      Left = 23
      Top = 20
      Width = 133
      Height = 21
      ItemHeight = 13
      TabOrder = 0
      OnChange = ComboBox1Change
      Items.Strings = (
        'rectangle'
        'circle'
        'freely'
        'none')
    end
    object Ex1: TEdit
      Left = 179
      Top = 19
      Width = 48
      Height = 21
      TabOrder = 1
      Text = '0'
    end
    object Ey1: TEdit
      Left = 247
      Top = 19
      Width = 48
      Height = 21
      TabOrder = 2
      Text = '0'
    end
    object Ex2: TEdit
      Left = 315
      Top = 19
      Width = 48
      Height = 21
      TabOrder = 3
      Text = '0'
    end
    object Ey2: TEdit
      Left = 386
      Top = 19
      Width = 48
      Height = 21
      TabOrder = 4
      Text = '0'
    end
  end
  object DBGNpc: TDBGrid
    Left = 44
    Top = 190
    Width = 408
    Height = 80
    DataSource = DataSource1
    TabOrder = 5
    TitleFont.Charset = DEFAULT_CHARSET
    TitleFont.Color = clWindowText
    TitleFont.Height = -11
    TitleFont.Name = 'Tahoma'
    TitleFont.Style = []
    Columns = <
      item
        Expanded = False
        FieldName = 'Id'
        Width = 100
        Visible = True
      end
      item
        Expanded = False
        FieldName = 'WanderType'
        Width = 70
        Visible = True
      end
      item
        Expanded = False
        FieldName = 'x1'
        Width = 45
        Visible = True
      end
      item
        Expanded = False
        FieldName = 'x2'
        Width = 45
        Visible = True
      end
      item
        Expanded = False
        FieldName = 'y1'
        Width = 45
        Visible = True
      end
      item
        Expanded = False
        FieldName = 'y2'
        Width = 45
        Visible = True
      end>
  end
  object Badd: TButton
    Left = 2
    Top = 190
    Width = 36
    Height = 39
    Caption = 'Add'
    TabOrder = 6
    OnClick = BaddClick
  end
  object bdel: TButton
    Left = 2
    Top = 231
    Width = 36
    Height = 39
    Caption = 'Del'
    TabOrder = 7
  end
  object animtimer: TTimer
    Enabled = False
    Interval = 100
    OnTimer = animtimerTimer
    Left = 572
    Top = 24
  end
  object DataSource1: TDataSource
    AutoEdit = False
    DataSet = frmRegions.CDSNpc
    Left = 32
    Top = 280
  end
end
