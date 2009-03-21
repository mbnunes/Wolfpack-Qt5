object FrmRegionEditS: TFrmRegionEditS
  Left = 323
  Top = 125
  Caption = 'Spawn Region Edit'
  ClientHeight = 609
  ClientWidth = 441
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  Position = poDesktopCenter
  OnCreate = FormCreate
  PixelsPerInch = 96
  TextHeight = 13
  object Label1: TLabel
    Left = 62
    Top = 48
    Width = 96
    Height = 13
    Caption = 'Spanwregion Name:'
  end
  object Label2: TLabel
    Left = 62
    Top = 108
    Width = 85
    Height = 13
    Caption = 'Max Npc Amount:'
  end
  object Label3: TLabel
    Left = 62
    Top = 154
    Width = 31
    Height = 13
    Caption = 'Delay:'
  end
  object Label4: TLabel
    Left = 62
    Top = 216
    Width = 73
    Height = 13
    Caption = 'Npcs per cycle:'
  end
  object Label5: TLabel
    Left = 62
    Top = 171
    Width = 20
    Height = 13
    Caption = 'Min:'
  end
  object Label6: TLabel
    Left = 134
    Top = 171
    Width = 24
    Height = 13
    Caption = 'Max:'
  end
  object SpeedButton1: TSpeedButton
    Left = 337
    Top = 94
    Width = 23
    Height = 22
    Glyph.Data = {
      36030000424D3603000000000000360000002800000010000000100000000100
      18000000000000030000120B0000120B00000000000000000000FF00FF078DBE
      078DBE078DBE078DBE078DBE078DBE078DBE078DBE078DBE078DBE078DBE078D
      BE078DBEFF00FFFF00FF078DBE25A1D172C7E785D7FA66CDF965CDF965CDF965
      CDF965CDF865CDF965CDF866CEF939ADD8078DBEFF00FFFF00FF078DBE4CBCE7
      39A8D1A0E2FB6FD4FA6FD4F96ED4FA6FD4F96FD4FA6FD4FA6FD4FA6ED4F93EB1
      D984D7EB078DBEFF00FF078DBE72D6FA078DBEAEEAFC79DCFB79DCFB79DCFB79
      DCFB79DCFB7ADCFB79DCFA79DCFA44B5D9AEF1F9078DBEFF00FF078DBE79DDFB
      1899C79ADFF392E7FB84E4FB83E4FC83E4FC84E4FC83E4FC83E4FB84E5FC48B9
      DAB3F4F9078DBEFF00FF078DBE82E3FC43B7DC65C3E0ACF0FD8DEBFC8DEBFC8D
      EBFD8DEBFD8DEBFC8DEBFD0C85184CBBDAB6F7F96DCAE0078DBE078DBE8AEAFC
      77DCF3229CC6FDFFFFC8F7FEC9F7FEC9F7FEC9F7FEC8F7FE0C85183CBC5D0C85
      18DEF9FBD6F6F9078DBE078DBE93F0FE93F0FD1697C5078DBE078DBE078DBE07
      8DBE078DBE0C851852D97F62ED9741C4650C8518078DBE078DBE078DBE9BF5FE
      9AF6FE9AF6FE9BF5FD9BF6FE9AF6FE9BF5FE0C851846CE6C59E48858E18861EB
      9440C1650C8518FF00FF078DBEFEFEFEA0FBFFA0FBFEA0FBFEA1FAFEA1FBFE0C
      85180C85180C85180C851856E18447CD6E0C85180C85180C8518FF00FF078DBE
      FEFEFEA5FEFFA5FEFFA5FEFF078CB643B7DC43B7DC43B7DC0C85184EDD7936BA
      540C8518FF00FFFF00FFFF00FFFF00FF078DBE078DBE078DBE078DBEFF00FFFF
      00FFFF00FFFF00FF0C851840D0650C8518FF00FFFF00FFFF00FFFF00FFFF00FF
      FF00FFFF00FFFF00FFFF00FFFF00FFFF00FFFF00FF0C85182AB7432DBA490C85
      18FF00FFFF00FFFF00FFFF00FFFF00FFFF00FFFF00FFFF00FFFF00FFFF00FFFF
      00FFFF00FF0C851821B5380C8518FF00FFFF00FFFF00FFFF00FFFF00FFFF00FF
      FF00FFFF00FFFF00FFFF00FFFF00FF0C85180C85180C85180C8518FF00FFFF00
      FFFF00FFFF00FFFF00FFFF00FFFF00FFFF00FFFF00FFFF00FF0C85180C85180C
      85180C8518FF00FFFF00FFFF00FFFF00FFFF00FFFF00FFFF00FF}
    OnClick = SpeedButton1Click
  end
  object Label7: TLabel
    Left = 218
    Top = 76
    Width = 33
    Height = 13
    Caption = 'Group:'
  end
  object SpeedButton2: TSpeedButton
    Left = 364
    Top = 94
    Width = 23
    Height = 22
    Glyph.Data = {
      36030000424D3603000000000000360000002800000010000000100000000100
      18000000000000030000120B0000120B00000000000000000000FF00FFFF00FF
      FF00FFFF00FFFF00FF01079F0313A90418AE0419AE0313A90108A0FF00FFFF00
      FFFF00FFFF00FFFF00FFFF00FFFF00FFFF00FF01049D041CB10730C00734C407
      35C50735C50734C30731C1041FB301069EFF00FFFF00FFFF00FFFF00FFFF00FF
      0109A1052BC30735C70733C20732C20732C20732C20732C20733C30735C4062D
      BE020CA4FF00FFFF00FFFF00FF01049B052BCA0636D80431CD0027C4032EC107
      32C20732C20430C10027BF042FC10735C4072EBE01069EFF00FFFF00FF031ABA
      0537E70331DD123DD86480E01840CB002CC1022DC00F38C46580D91B43C7052F
      C10735C5051FB3FF00FF01049E0430E40436F1002AE45070E9FFFFFFB7C4F10D
      36CA042DC3A2B2E8FFFFFF6984DA0026BE0733C30731C10108A0020FAF0336FA
      0335F80232EE0A35E88CA2F2FFFFFFB4C2F1A9B8EDFFFFFFA7B7E9133AC4052F
      C10732C20734C40313AA0619BC1747FE093AFC0435F80131F0002BE891A5F4FF
      FFFFFFFFFFABBAEF062FC5022DC00732C20732C20736C50419AE0B1DBE4168FE
      1C49FC0335FB0031F90531F2A4B5F7FFFFFFFFFFFFB9C6F20D36D0002CC60732
      C20732C20736C50418AD0613B45B7CFC486CFD0133FB113CFBA1B4FEFFFFFFA4
      B6F892A7F5FFFFFFB6C4F21A41D3042FC80732C40734C30212A90003A04A6AF3
      8FA6FF1F46FB4C6FFCFFFFFFA7B8FE0733F6002AED8CA2F6FFFFFF627FE70028
      D00734CC0730C300069FFF00FF1A2FCB99AFFF8BA2FE214DFB4D71FC0E3DFB00
      30FB0031F70636F14C6EF1103CE30432DB0636D7041CB5FF00FFFF00FF0004A0
      415EECB8C7FF9CAFFD3A5CFC0A3AFB0335FB0335FB0133F9052FF20635EB0537
      E9052CCD00049CFF00FFFF00FFFF00FF0309A54260ECA9BBFFBDCAFF8EA5FE64
      83FD5073FC4A6EFD3961FD1444F9042CD70109A2FF00FFFF00FFFF00FFFF00FF
      FF00FF0004A01E32CD5876F6859EFE8BA3FF7994FE5376FC234AF0051EC50104
      9CFF00FFFF00FFFF00FFFF00FFFF00FFFF00FFFF00FFFF00FF0004A00917B610
      22C30D1FC20311B401059FFF00FFFF00FFFF00FFFF00FFFF00FF}
    OnClick = SpeedButton2Click
  end
  object GroupBox2: TGroupBox
    Left = 64
    Top = 536
    Width = 321
    Height = 57
    TabOrder = 0
    object BitBtn1: TBitBtn
      Left = 46
      Top = 20
      Width = 75
      Height = 25
      Caption = 'OK'
      TabOrder = 0
      OnClick = BitBtn1Click
    end
    object BitBtn2: TBitBtn
      Left = 202
      Top = 20
      Width = 75
      Height = 25
      Caption = 'Cancel'
      TabOrder = 1
      OnClick = BitBtn2Click
    end
  end
  object Panel1: TPanel
    Left = 12
    Top = 5
    Width = 413
    Height = 27
    Caption = 'Edit your Region '
    TabOrder = 1
  end
  object ERegionName: TEdit
    Left = 164
    Top = 45
    Width = 223
    Height = 21
    TabOrder = 2
  end
  object CheckBox1: TCheckBox
    Left = 61
    Top = 80
    Width = 97
    Height = 13
    Caption = 'Active'
    TabOrder = 3
  end
  object EMaxnpcamount: TEdit
    Left = 62
    Top = 127
    Width = 121
    Height = 21
    TabOrder = 4
    Text = '0'
  end
  object Edelaymin: TEdit
    Left = 62
    Top = 185
    Width = 53
    Height = 21
    TabOrder = 5
    Text = '0'
  end
  object Edelaymax: TEdit
    Left = 134
    Top = 185
    Width = 49
    Height = 21
    TabOrder = 6
    Text = '0'
  end
  object Enpcspercycle: TEdit
    Left = 62
    Top = 235
    Width = 121
    Height = 21
    TabOrder = 7
    Text = '0'
  end
  object ListBox1: TListBox
    Left = 218
    Top = 122
    Width = 169
    Height = 136
    ItemHeight = 13
    TabOrder = 8
  end
  object Edit1: TEdit
    Left = 218
    Top = 95
    Width = 113
    Height = 21
    TabOrder = 9
  end
  object Panel2: TPanel
    Left = -2
    Top = 264
    Width = 443
    Height = 125
    TabOrder = 10
    object Splitter1: TSplitter
      Left = 157
      Top = 1
      Height = 123
      ExplicitLeft = 202
      ExplicitHeight = 112
    end
    object Splitter2: TSplitter
      Left = 327
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
      Width = 167
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
      ExplicitWidth = 177
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
      Left = 330
      Top = 1
      Width = 112
      Height = 123
      Align = alRight
      TabOrder = 2
      OnPaintBuffer = pbNpcPreviewPaintBuffer
      ExplicitLeft = 340
    end
  end
  object GroupBox1: TGroupBox
    Left = 2
    Top = 394
    Width = 439
    Height = 50
    Caption = 'Npc Wander'
    TabOrder = 11
    object Label8: TLabel
      Left = 162
      Top = 22
      Width = 16
      Height = 13
      Caption = 'x1:'
    end
    object Label9: TLabel
      Left = 229
      Top = 22
      Width = 16
      Height = 13
      Caption = 'y1:'
    end
    object Label10: TLabel
      Left = 300
      Top = 22
      Width = 16
      Height = 13
      Caption = 'x2:'
    end
    object Label11: TLabel
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
    Left = 43
    Top = 450
    Width = 398
    Height = 80
    DataSource = DataSource1
    TabOrder = 12
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
    Left = 1
    Top = 450
    Width = 36
    Height = 39
    Caption = 'Add'
    TabOrder = 13
    OnClick = BaddClick
  end
  object bdel: TButton
    Left = 1
    Top = 491
    Width = 36
    Height = 39
    Caption = 'Del'
    TabOrder = 14
    OnClick = bdelClick
  end
  object DataSource1: TDataSource
    AutoEdit = False
    DataSet = frmRegions.CDSNpc
    Left = 42
    Top = 540
  end
  object animtimer: TTimer
    Enabled = False
    Interval = 100
    OnTimer = animtimerTimer
    Left = 572
    Top = 24
  end
end
