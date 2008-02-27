object frmMain: TfrmMain
  Left = 343
  Top = 473
  Caption = 'Wolfpack - GM Tool'
  ClientHeight = 184
  ClientWidth = 604
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  Position = poDesktopCenter
  ShowHint = True
  OnClose = FormClose
  OnCreate = FormCreate
  OnDestroy = FormDestroy
  PixelsPerInch = 96
  TextHeight = 13
  object Pages: TPageControl
    Left = 0
    Top = 21
    Width = 604
    Height = 163
    ActivePage = TabSheet1
    Align = alClient
    RaggedRight = True
    TabOrder = 0
    object tsCommands: TTabSheet
      Caption = 'Commands'
      ImageIndex = 3
      object GroupBox1: TGroupBox
        Left = 8
        Top = 8
        Width = 185
        Height = 121
        Caption = 'Toggle Flags'
        TabOrder = 0
        object btnCmdAllmove: TButton
          Left = 8
          Top = 24
          Width = 75
          Height = 25
          Hint = 'Toggle moving immovable objects.'
          Caption = 'Allmove'
          TabOrder = 0
          OnClick = btnCmdAllmoveClick
        end
        object btnCmdAllshow: TButton
          Left = 8
          Top = 56
          Width = 75
          Height = 25
          Hint = 'Toggle showing logged out characters.'
          Caption = 'Allshow'
          TabOrder = 1
          OnClick = btnCmdAllshowClick
        end
        object btnCmdInvisible: TButton
          Left = 8
          Top = 88
          Width = 75
          Height = 25
          Hint = 'Toggle the invisible status.'
          Caption = 'Invisible'
          TabOrder = 2
          OnClick = btnCmdInvisibleClick
        end
        object btnCmdStaff: TButton
          Left = 96
          Top = 56
          Width = 75
          Height = 25
          Hint = 'Toggle the GM mode flag.'
          Caption = 'Staff Flag'
          TabOrder = 3
          OnClick = btnCmdStaffClick
        end
        object btnCmdInvul: TButton
          Left = 96
          Top = 24
          Width = 75
          Height = 25
          Hint = 'Toggle invulnerability.'
          Caption = 'Invulnerable'
          TabOrder = 4
          OnClick = btnCmdInvulClick
        end
        object btnCmdPageNotify: TButton
          Left = 96
          Top = 88
          Width = 75
          Height = 25
          Hint = 'Be notified about incoming pages.'
          Caption = 'Page Notify'
          Enabled = False
          TabOrder = 5
          OnClick = btnCmdStaffClick
        end
      end
      object GroupBox2: TGroupBox
        Left = 200
        Top = 8
        Width = 233
        Height = 121
        Caption = 'System Commands'
        TabOrder = 1
        object btnCmdShutdown: TButton
          Left = 8
          Top = 24
          Width = 105
          Height = 25
          Hint = 'Close the server.'
          Caption = 'Shutdown'
          TabOrder = 0
          OnClick = btnCmdShutdownClick
        end
        object btnCmdReloadPython: TButton
          Left = 8
          Top = 56
          Width = 105
          Height = 25
          Hint = 'Reload the python scripts.'
          Caption = 'Reload Python'
          TabOrder = 1
          OnClick = btnCmdReloadPythonClick
        end
        object btnCmdReloadScripts: TButton
          Left = 8
          Top = 88
          Width = 105
          Height = 25
          Hint = 'Reload the definitions and python scripts.'
          Caption = 'Reload Definitions'
          TabOrder = 2
          OnClick = btnCmdReloadScriptsClick
        end
        object btnCmdReloadAccounts: TButton
          Left = 120
          Top = 56
          Width = 105
          Height = 25
          Hint = 'Reload the accounts.'
          Caption = 'Reload Accounts'
          TabOrder = 3
          OnClick = btnCmdReloadAccountsClick
        end
        object btnCmdReloadAll: TButton
          Left = 120
          Top = 88
          Width = 105
          Height = 25
          Hint = 'Reload everything.'
          Caption = 'Reload All'
          TabOrder = 4
          OnClick = btnCmdReloadAllClick
        end
        object btnCmdServertime: TButton
          Left = 120
          Top = 24
          Width = 105
          Height = 25
          Hint = 'Show the current servertime.'
          Caption = 'Show Servertime'
          TabOrder = 5
          OnClick = btnCmdServertimeClick
        end
      end
      object Button1: TButton
        Left = 440
        Top = 16
        Width = 97
        Height = 25
        Caption = 'Pattern Tilecolor'
        TabOrder = 2
        OnClick = Button1Click
      end
    end
    object tsItems: TTabSheet
      Caption = 'Items'
      ImageIndex = -1
      DesignSize = (
        596
        135)
      object Label7: TLabel
        Left = 162
        Top = 109
        Width = 39
        Height = 13
        Anchors = [akLeft, akBottom]
        Caption = 'Z Level:'
        ExplicitTop = 118
      end
      object pnlItems: TPanel
        Left = 0
        Top = 0
        Width = 594
        Height = 103
        Anchors = [akLeft, akTop, akRight, akBottom]
        BevelOuter = bvNone
        TabOrder = 0
        object spItems: TSplitter
          Left = 201
          Top = 0
          Height = 103
          ExplicitHeight = 112
        end
        object Splitter3: TSplitter
          Left = 477
          Top = 0
          Height = 103
          Align = alRight
          ExplicitLeft = 480
        end
        object vtCategories: TVirtualStringTree
          Left = 0
          Top = 0
          Width = 201
          Height = 103
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
          OnChange = vtCategoriesChange
          OnExpanding = vtCategoriesExpanding
          OnGetText = vtCategoriesGetText
          Columns = <>
        end
        object vtItems: TVirtualStringTree
          Left = 204
          Top = 0
          Width = 273
          Height = 103
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
          TreeOptions.SelectionOptions = [toDisableDrawSelection, toFullRowSelect, toMultiSelect]
          OnChange = vtItemsChange
          OnCompareNodes = vtItemsCompareNodes
          OnDblClick = btnAddItemClick
          OnGetText = vtItemsGetText
          OnHeaderClick = vtItemsHeaderClick
          Columns = <
            item
              Position = 0
              Width = 197
              WideText = 'Name'
            end
            item
              Position = 1
              WideText = 'ID'
            end>
        end
        object imgItemPreview: TImage32
          Left = 480
          Top = 0
          Width = 114
          Height = 103
          Align = alRight
          BitmapAlign = baCenter
          Scale = 1.000000000000000000
          ScaleMode = smNormal
          TabOrder = 2
        end
      end
      object btnAddItem: TButton
        Left = 0
        Top = 105
        Width = 75
        Height = 21
        Anchors = [akLeft, akBottom]
        Caption = 'Add'
        Enabled = False
        TabOrder = 1
        OnClick = btnAddItemClick
      end
      object btnRemoveItem: TButton
        Left = 80
        Top = 105
        Width = 75
        Height = 21
        Anchors = [akLeft, akBottom]
        Caption = 'Remove'
        TabOrder = 2
        OnClick = btnRemoveItemClick
      end
      object btnTileItem: TButton
        Left = 249
        Top = 105
        Width = 75
        Height = 21
        Hint = 'Fill a rectangle with items at the given z level.'
        Anchors = [akLeft, akBottom]
        Caption = 'Tile'
        Enabled = False
        TabOrder = 4
        OnClick = btnTileItemClick
      end
      object eZLevel: TEdit
        Left = 208
        Top = 105
        Width = 35
        Height = 21
        Anchors = [akLeft, akBottom]
        TabOrder = 3
        Text = '0'
      end
      object btnCmdItemNuke: TButton
        Left = 329
        Top = 105
        Width = 75
        Height = 21
        Hint = 'Nuke a rectangle of items.'
        Anchors = [akLeft, akBottom]
        Caption = 'Nuke'
        TabOrder = 5
        OnClick = btnCmdNukeClick
      end
      object cbStatic: TCheckBox
        Left = 413
        Top = 107
        Width = 49
        Height = 17
        Anchors = [akLeft, akBottom]
        Caption = 'Static'
        TabOrder = 6
      end
    end
    object tsItemManip: TTabSheet
      Caption = 'Item Manipulation'
      ImageIndex = 4
      object GroupBox3: TGroupBox
        Left = 8
        Top = 8
        Width = 145
        Height = 121
        Caption = 'Change Color'
        TabOrder = 0
        object eItemHue: TEdit
          Left = 8
          Top = 24
          Width = 97
          Height = 21
          TabOrder = 0
          Text = '0'
          OnChange = eItemHueChange
        end
        object btnChangeItemHue: TButton
          Left = 109
          Top = 23
          Width = 28
          Height = 23
          Caption = '...'
          TabOrder = 1
          OnClick = btnChangeItemHueClick
        end
        object btnCmdDye: TButton
          Left = 8
          Top = 88
          Width = 57
          Height = 25
          Caption = 'Set'
          TabOrder = 2
          OnClick = btnCmdDyeClick
        end
        object pbHuePreview: TPaintBox32
          Left = 8
          Top = 56
          Width = 128
          Height = 25
          TabOrder = 3
          OnPaintBuffer = pbHuePreviewPaintBuffer
        end
        object btnCmdItemTileColor: TButton
          Left = 72
          Top = 88
          Width = 67
          Height = 25
          Hint = 'Tiles this color over an area of items.'
          Caption = 'Tile'
          TabOrder = 4
          OnClick = btnCmdItemTileColorClick
        end
      end
      object GroupBox4: TGroupBox
        Left = 160
        Top = 8
        Width = 185
        Height = 121
        Caption = 'Commands'
        TabOrder = 1
        object btnCmdDupe: TButton
          Left = 8
          Top = 24
          Width = 75
          Height = 25
          Hint = 'Create an exact duplicate of the item.'
          Caption = 'Dupe'
          TabOrder = 0
          OnClick = btnCmdDupeClick
        end
        object btnCmdInfo: TButton
          Left = 8
          Top = 56
          Width = 75
          Height = 25
          Hint = 'Show an info dialog for the item.'
          Caption = 'Info'
          TabOrder = 1
          OnClick = btnCmdInfoClick
        end
        object btnCmdRemove: TButton
          Left = 8
          Top = 88
          Width = 75
          Height = 25
          Hint = 'Remove the item.'
          Caption = 'Remove'
          TabOrder = 2
          OnClick = btnCmdRemoveClick
        end
        object btnCmdLock: TButton
          Left = 96
          Top = 24
          Width = 75
          Height = 25
          Hint = 'Attach a lock to the item.'
          Caption = 'Lock'
          TabOrder = 3
          OnClick = btnCmdLockClick
        end
        object btnCmdNuke: TButton
          Left = 96
          Top = 56
          Width = 75
          Height = 25
          Hint = 'Wipe an area from its items.'
          Caption = 'Nuke'
          TabOrder = 4
          OnClick = btnCmdNukeClick
        end
        object btnCmdMoveUp: TBitBtn
          Left = 96
          Top = 88
          Width = 33
          Height = 25
          Hint = 'Move the item up by 1 point of z.'
          TabOrder = 5
          OnClick = btnCmdMoveUpClick
          Glyph.Data = {
            32010000424D3201000000000000360000002800000009000000090000000100
            180000000000FC000000120B0000120B00000000000000000000FFFFFFFFFFFF
            FFFFFF000000000000000000FFFFFFFFFFFFFFFFFF00FFFFFFFFFFFFFFFFFF00
            0000000000000000FFFFFFFFFFFFFFFFFF00FFFFFFFFFFFFFFFFFF0000000000
            00000000FFFFFFFFFFFFFFFFFF00FFFFFFFFFFFFFFFFFF000000000000000000
            FFFFFFFFFFFFFFFFFF0000000000000000000000000000000000000000000000
            000000000000FFFFFF000000000000000000000000000000000000000000FFFF
            FF00FFFFFFFFFFFF000000000000000000000000000000FFFFFFFFFFFF00FFFF
            FFFFFFFFFFFFFF000000000000000000FFFFFFFFFFFFFFFFFF00FFFFFFFFFFFF
            FFFFFFFFFFFF000000FFFFFFFFFFFFFFFFFFFFFFFF00}
        end
        object btnCmdMoveDown: TBitBtn
          Left = 136
          Top = 88
          Width = 33
          Height = 25
          Hint = 'Move the item down by 1 point of z.'
          TabOrder = 6
          OnClick = btnCmdMoveDownClick
          Glyph.Data = {
            32010000424D3201000000000000360000002800000009000000090000000100
            180000000000FC000000120B0000120B00000000000000000000FFFFFFFFFFFF
            FFFFFFFFFFFF000000FFFFFFFFFFFFFFFFFFFFFFFF00FFFFFFFFFFFFFFFFFF00
            0000000000000000FFFFFFFFFFFFFFFFFF00FFFFFFFFFFFF0000000000000000
            00000000000000FFFFFFFFFFFF00FFFFFF000000000000000000000000000000
            000000000000FFFFFF0000000000000000000000000000000000000000000000
            000000000000FFFFFFFFFFFFFFFFFF000000000000000000FFFFFFFFFFFFFFFF
            FF00FFFFFFFFFFFFFFFFFF000000000000000000FFFFFFFFFFFFFFFFFF00FFFF
            FFFFFFFFFFFFFF000000000000000000FFFFFFFFFFFFFFFFFF00FFFFFFFFFFFF
            FFFFFF000000000000000000FFFFFFFFFFFFFFFFFF00}
        end
      end
      object GroupBox5: TGroupBox
        Left = 352
        Top = 8
        Width = 177
        Height = 121
        Caption = 'Properties'
        TabOrder = 2
        object Label1: TLabel
          Left = 10
          Top = 60
          Width = 30
          Height = 13
          Caption = 'Value:'
        end
        object cbItemProperty: TComboBox
          Left = 8
          Top = 24
          Width = 153
          Height = 21
          ItemHeight = 13
          TabOrder = 0
          OnChange = cbItemPropertyChange
          Items.Strings = (
            'amount'
            'baseid'
            'color'
            'decay'
            'health'
            'id'
            'movable'
            'maxhealth'
            'name'
            'newbie'
            'owner'
            'ownervisible'
            'pos'
            'secured'
            'serial'
            'spawnregion'
            'totalweight'
            'type'
            'visible')
        end
        object eItemPropValue: TEdit
          Left = 56
          Top = 56
          Width = 105
          Height = 21
          TabOrder = 1
        end
        object btnCmdItemSet: TButton
          Left = 8
          Top = 88
          Width = 75
          Height = 25
          Hint = 'Set the value of the selected property'
          Caption = 'Set'
          Enabled = False
          TabOrder = 2
          OnClick = btnCmdItemSetClick
        end
        object btnCmdItemShow: TButton
          Left = 88
          Top = 88
          Width = 75
          Height = 25
          Hint = 'Show the value of the selected property.'
          Caption = 'Show'
          Enabled = False
          TabOrder = 3
          OnClick = btnCmdItemShowClick
        end
      end
    end
    object tsNpcs: TTabSheet
      Caption = 'NPCs'
      ImageIndex = 6
      DesignSize = (
        596
        135)
      object Label8: TLabel
        Left = 242
        Top = 109
        Width = 36
        Height = 13
        Anchors = [akLeft, akBottom]
        Caption = 'Radius:'
        ExplicitTop = 118
      end
      object Label9: TLabel
        Left = 330
        Top = 109
        Width = 46
        Height = 13
        Anchors = [akLeft, akBottom]
        Caption = 'Min. Time'
        ExplicitTop = 118
      end
      object Label10: TLabel
        Left = 426
        Top = 109
        Width = 49
        Height = 13
        Anchors = [akLeft, akBottom]
        Caption = 'Max. Time'
        ExplicitTop = 118
      end
      object pnNpcs: TPanel
        Left = 0
        Top = 0
        Width = 594
        Height = 103
        Anchors = [akLeft, akTop, akRight, akBottom]
        BevelOuter = bvNone
        TabOrder = 0
        object Splitter1: TSplitter
          Left = 201
          Top = 0
          Height = 103
          ExplicitLeft = 204
        end
        object Splitter2: TSplitter
          Left = 478
          Top = 0
          Height = 103
          Align = alRight
          ExplicitLeft = 481
        end
        object vtNpcCategories: TVirtualStringTree
          Left = 0
          Top = 0
          Width = 201
          Height = 103
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
          OnGetText = vtCategoriesGetText
          Columns = <>
        end
        object vtNpcs: TVirtualStringTree
          Left = 204
          Top = 0
          Width = 274
          Height = 103
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
          OnDblClick = btnAddNpcClick
          OnGetText = vtNpcsGetText
          OnHeaderClick = vtNpcsHeaderClick
          Columns = <
            item
              Position = 0
              Width = 197
              WideText = 'Name'
            end
            item
              Position = 1
              WideText = 'ID'
            end>
        end
        object pbNpcPreview: TPaintBox32
          Left = 481
          Top = 0
          Width = 113
          Height = 103
          Align = alRight
          TabOrder = 2
          OnPaintBuffer = pbNpcPreviewPaintBuffer
        end
      end
      object btnAddNpc: TButton
        Left = 0
        Top = 105
        Width = 75
        Height = 21
        Anchors = [akLeft, akBottom]
        Caption = 'Add'
        Enabled = False
        TabOrder = 1
        OnClick = btnAddNpcClick
      end
      object btnRemoveNpc: TButton
        Left = 79
        Top = 105
        Width = 75
        Height = 21
        Anchors = [akLeft, akBottom]
        Caption = 'Remove'
        TabOrder = 2
        OnClick = btnRemoveNpcClick
      end
      object eSpawnRadius: TEdit
        Left = 288
        Top = 105
        Width = 35
        Height = 21
        Anchors = [akLeft, akBottom]
        TabOrder = 3
        Text = '0'
        OnExit = eSpawnRadiusExit
      end
      object eSpawnMin: TEdit
        Left = 384
        Top = 105
        Width = 35
        Height = 21
        Anchors = [akLeft, akBottom]
        TabOrder = 4
        Text = '0'
        OnExit = eSpawnMinExit
      end
      object eSpawnMax: TEdit
        Left = 480
        Top = 105
        Width = 35
        Height = 21
        Anchors = [akLeft, akBottom]
        TabOrder = 5
        Text = '0'
        OnExit = eSpawnMaxExit
      end
      object btnNpcSpawn: TButton
        Left = 159
        Top = 105
        Width = 75
        Height = 21
        Anchors = [akLeft, akBottom]
        Caption = 'Spawn'
        Enabled = False
        TabOrder = 6
        OnClick = btnNpcSpawnClick
      end
    end
    object TabSheet1: TTabSheet
      Caption = 'Multis'
      ImageIndex = 7
      DesignSize = (
        596
        135)
      object Panel1: TPanel
        Left = 0
        Top = 0
        Width = 594
        Height = 103
        Anchors = [akLeft, akTop, akRight, akBottom]
        BevelOuter = bvNone
        TabOrder = 0
        object vtMultis: TVirtualStringTree
          Left = 0
          Top = 0
          Width = 594
          Height = 103
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
          TabOrder = 0
          TreeOptions.AutoOptions = [toAutoDropExpand, toAutoScrollOnExpand, toAutoSort, toAutoTristateTracking, toAutoDeleteMovedNodes, toAutoFreeOnCollapse]
          TreeOptions.PaintOptions = [toHideFocusRect, toShowButtons, toThemeAware, toUseBlendedImages]
          TreeOptions.SelectionOptions = [toDisableDrawSelection, toFullRowSelect]
          OnChange = vtMultisChange
          OnCompareNodes = vtMultisCompareNodes
          OnDblClick = btnAddMultiClick
          OnGetText = vtMultisGetText
          OnHeaderClick = vtMultisHeaderClick
          Columns = <
            item
              Position = 0
              Width = 197
              WideText = 'Name'
            end
            item
              Position = 1
              WideText = 'ID'
            end>
        end
      end
      object btnAddMulti: TButton
        Left = 0
        Top = 106
        Width = 75
        Height = 21
        Anchors = [akLeft, akBottom]
        Caption = 'Add'
        Enabled = False
        TabOrder = 1
        OnClick = btnAddMultiClick
      end
      object btnRemoveMulti: TButton
        Left = 79
        Top = 106
        Width = 75
        Height = 21
        Anchors = [akLeft, akBottom]
        Caption = 'Remove'
        TabOrder = 2
      end
      object btnPreviewMulti: TButton
        Left = 158
        Top = 106
        Width = 75
        Height = 21
        Anchors = [akLeft, akBottom]
        Caption = 'Preview'
        Enabled = False
        TabOrder = 3
        OnClick = btnPreviewMultiClick
      end
    end
    object tsTags: TTabSheet
      Caption = 'Object Manipulation'
      ImageIndex = 5
      object GroupBox6: TGroupBox
        Left = 8
        Top = 8
        Width = 233
        Height = 121
        Caption = 'Manage Tags'
        TabOrder = 0
        object Label2: TLabel
          Left = 10
          Top = 60
          Width = 30
          Height = 13
          Caption = 'Value:'
        end
        object Label3: TLabel
          Left = 10
          Top = 28
          Width = 31
          Height = 13
          Caption = 'Name:'
        end
        object eTagValue: TEdit
          Left = 56
          Top = 56
          Width = 97
          Height = 21
          Hint = 'The new tag value if you use set.'
          TabOrder = 0
        end
        object btnTagSet: TButton
          Left = 8
          Top = 88
          Width = 49
          Height = 25
          Caption = 'Set'
          Enabled = False
          TabOrder = 1
          OnClick = btnTagSetClick
        end
        object eTagName: TEdit
          Left = 56
          Top = 24
          Width = 169
          Height = 21
          Hint = 'The name of the tag you want to set.'
          TabOrder = 2
          OnChange = eTagNameChange
        end
        object btnTagShow: TButton
          Left = 64
          Top = 88
          Width = 49
          Height = 25
          Caption = 'Show'
          Enabled = False
          TabOrder = 3
          OnClick = btnTagShowClick
        end
        object btnTagDelete: TButton
          Left = 120
          Top = 88
          Width = 49
          Height = 25
          Caption = 'Delete'
          Enabled = False
          TabOrder = 4
          OnClick = btnTagDeleteClick
        end
        object cbTagType: TComboBox
          Left = 160
          Top = 56
          Width = 65
          Height = 21
          Hint = 'The type of the tag value.'
          Style = csDropDownList
          ItemHeight = 13
          ItemIndex = 0
          TabOrder = 5
          Text = 'String'
          Items.Strings = (
            'String'
            'Integer'
            'Float')
        end
        object btnTagInfo: TButton
          Left = 176
          Top = 88
          Width = 49
          Height = 25
          Caption = 'Info'
          TabOrder = 6
          OnClick = btnTagInfoClick
        end
      end
      object GroupBox7: TGroupBox
        Left = 248
        Top = 8
        Width = 137
        Height = 121
        Caption = 'Move Relatively'
        TabOrder = 1
        object Label4: TLabel
          Left = 16
          Top = 18
          Width = 29
          Height = 13
          Caption = 'X Axis'
        end
        object Label5: TLabel
          Left = 16
          Top = 42
          Width = 29
          Height = 13
          Caption = 'Y Axis'
        end
        object Label6: TLabel
          Left = 16
          Top = 66
          Width = 29
          Height = 13
          Caption = 'Z Axis'
        end
        object eMoveX: TEdit
          Left = 80
          Top = 16
          Width = 41
          Height = 21
          TabOrder = 0
          Text = '0'
        end
        object eMoveY: TEdit
          Left = 80
          Top = 40
          Width = 41
          Height = 21
          TabOrder = 1
          Text = '0'
        end
        object eMoveZ: TEdit
          Left = 80
          Top = 64
          Width = 41
          Height = 21
          TabOrder = 2
          Text = '0'
        end
        object btnCmdMove: TButton
          Left = 8
          Top = 88
          Width = 65
          Height = 25
          Caption = 'Move'
          TabOrder = 3
          OnClick = btnCmdMoveClick
        end
      end
      object GroupBox8: TGroupBox
        Left = 392
        Top = 8
        Width = 137
        Height = 121
        Caption = 'Manage Scripts'
        TabOrder = 2
        object btnCmdAddEvent: TButton
          Left = 8
          Top = 56
          Width = 57
          Height = 25
          Caption = 'Add'
          Enabled = False
          TabOrder = 1
          OnClick = btnCmdAddEventClick
        end
        object btnCmdRemoveEvent: TButton
          Left = 72
          Top = 56
          Width = 57
          Height = 25
          Caption = 'Remove'
          Enabled = False
          TabOrder = 2
          OnClick = btnCmdRemoveEventClick
        end
        object btnCmdShowEventlist: TButton
          Left = 8
          Top = 88
          Width = 121
          Height = 25
          Caption = 'Show Eventlist'
          TabOrder = 3
          OnClick = btnCmdShowEventlistClick
        end
        object cbEventName: TComboBox
          Left = 8
          Top = 24
          Width = 121
          Height = 21
          ItemHeight = 13
          TabOrder = 0
          OnChange = eEventNameChange
        end
      end
      object VirtualStringTree1: TVirtualStringTree
        Left = 540
        Top = 88
        Width = 37
        Height = 25
        Header.AutoSizeIndex = 0
        Header.Font.Charset = DEFAULT_CHARSET
        Header.Font.Color = clWindowText
        Header.Font.Height = -11
        Header.Font.Name = 'Tahoma'
        Header.Font.Style = []
        Header.MainColumn = -1
        Header.Options = [hoColumnResize, hoDrag]
        TabOrder = 3
        Columns = <>
      end
    end
    object TravelMap: TTabSheet
      Caption = 'Travel'
      ImageIndex = 2
      DesignSize = (
        596
        135)
      object pnlTravel: TPanel
        Left = 0
        Top = 0
        Width = 594
        Height = 103
        Anchors = [akLeft, akTop, akRight, akBottom]
        BevelOuter = bvNone
        TabOrder = 0
        object spTravel: TSplitter
          Left = 201
          Top = 0
          Height = 103
          ExplicitHeight = 112
        end
        object spMap: TSplitter
          Left = 441
          Top = 0
          Height = 103
          Align = alRight
          ExplicitHeight = 112
        end
        object vtLocCategories: TVirtualStringTree
          Left = 0
          Top = 0
          Width = 201
          Height = 103
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
          OnChange = vtLocCategoriesChange
          OnExpanding = vtLocCategoriesExpanding
          OnGetText = vtCategoriesGetText
          Columns = <>
        end
        object vtLocItems: TVirtualStringTree
          Left = 204
          Top = 0
          Width = 237
          Height = 103
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
          OnChange = vtLocItemsChange
          OnCompareNodes = vtLocItemsCompareNodes
          OnDblClick = Go1Click
          OnGetText = vtLocItemsGetText
          OnHeaderClick = vtLocItemsHeaderClick
          Columns = <
            item
              Position = 0
              Width = 197
              WideText = 'Name'
            end>
        end
        object pbMap: TPaintBox32
          Left = 444
          Top = 0
          Width = 150
          Height = 103
          Align = alRight
          PopupMenu = pmTravel
          TabOrder = 2
          OnMouseDown = pbMapMouseDown
          OnPaintBuffer = pbMapPaintBuffer
        end
      end
      object btnTravel: TButton
        Left = 0
        Top = 105
        Width = 75
        Height = 21
        Anchors = [akLeft, akBottom]
        Caption = 'Travel'
        TabOrder = 1
        OnClick = Go1Click
      end
      object btnSend: TButton
        Left = 80
        Top = 105
        Width = 75
        Height = 21
        Anchors = [akLeft, akBottom]
        Caption = 'Send'
        TabOrder = 2
        OnClick = btnSendClick
      end
      object Button2: TButton
        Left = 160
        Top = 105
        Width = 89
        Height = 21
        Anchors = [akLeft, akBottom]
        Caption = 'Teleport Self'
        TabOrder = 3
        OnClick = Button2Click
      end
      object btnCmdSendOther: TButton
        Left = 254
        Top = 105
        Width = 89
        Height = 21
        Anchors = [akLeft, akBottom]
        Caption = 'Teleport Other'
        TabOrder = 4
        OnClick = btnCmdSendOtherClick
      end
      object btnCmdWhere: TButton
        Left = 348
        Top = 105
        Width = 75
        Height = 21
        Hint = 'Show your current position.'
        Anchors = [akLeft, akBottom]
        Caption = 'Where'
        TabOrder = 5
        OnClick = btnCmdWhereClick
      end
      object btnMapOverview: TButton
        Left = 428
        Top = 105
        Width = 75
        Height = 21
        Hint = 'Show your current position.'
        Anchors = [akLeft, akBottom]
        Caption = 'Overview'
        TabOrder = 6
        OnClick = btnMapOverviewClick
      end
    end
    object tsSettings: TTabSheet
      Caption = 'Settings'
      ImageIndex = 1
      object gbSettingsGeneral: TGroupBox
        Left = 8
        Top = 8
        Width = 241
        Height = 97
        Caption = 'General Settings'
        TabOrder = 0
        object lblCommandPrefix: TLabel
          Left = 45
          Top = 64
          Width = 76
          Height = 13
          Caption = 'Command Prefix'
        end
        object cbAlwaysOnTop: TCheckBox
          Left = 16
          Top = 24
          Width = 97
          Height = 17
          Caption = 'Always on top'
          TabOrder = 0
          OnClick = cbAlwaysOnTopClick
        end
        object eCommandPrefix: TEdit
          Left = 15
          Top = 60
          Width = 25
          Height = 21
          TabOrder = 1
        end
        object btnSave: TButton
          Left = 152
          Top = 56
          Width = 75
          Height = 25
          Caption = '&Save'
          TabOrder = 2
          OnClick = btnSaveClick
        end
      end
      object btnRegions: TButton
        Left = 264
        Top = 16
        Width = 75
        Height = 25
        Caption = 'Regions'
        TabOrder = 1
        OnClick = btnRegionsClick
      end
      object BitFiles: TBitBtn
        Left = 264
        Top = 51
        Width = 75
        Height = 26
        Caption = 'Files Config.'
        TabOrder = 2
        OnClick = BitFilesClick
      end
    end
    object TabSheet2: TTabSheet
      Caption = 'Build'
      ImageIndex = 8
      DesignSize = (
        596
        135)
      object GroupBox9: TGroupBox
        Left = 0
        Top = 0
        Width = 89
        Height = 128
        Anchors = [akLeft, akTop, akBottom]
        Caption = 'Settings'
        TabOrder = 0
        object Label11: TLabel
          Left = 8
          Top = 18
          Width = 10
          Height = 16
          Caption = 'Z'
          Font.Charset = DEFAULT_CHARSET
          Font.Color = clWindowText
          Font.Height = -13
          Font.Name = 'MS Sans Serif'
          Font.Style = [fsBold]
          ParentFont = False
        end
        object cbBuildZ: TComboBox
          Left = 23
          Top = 16
          Width = 58
          Height = 21
          ItemHeight = 13
          TabOrder = 0
          Text = '0'
          Items.Strings = (
            '0'
            '5'
            '7'
            '10'
            '15'
            '17'
            '20'
            '25'
            '27'
            '30'
            '35'
            '37'
            '40')
        end
      end
      object gbBuild: TGroupBox
        Left = 96
        Top = 0
        Width = 489
        Height = 128
        Anchors = [akLeft, akTop, akRight, akBottom]
        Caption = 'Build'
        TabOrder = 1
        DesignSize = (
          489
          128)
        object sbBuild: TScrollBox
          Left = 8
          Top = 16
          Width = 473
          Height = 104
          HorzScrollBar.Smooth = True
          HorzScrollBar.Tracking = True
          VertScrollBar.Smooth = True
          VertScrollBar.Tracking = True
          Anchors = [akLeft, akTop, akRight, akBottom]
          BevelOuter = bvNone
          BevelKind = bkSoft
          TabOrder = 0
          OnClick = sbBuildClick
          OnMouseWheelDown = sbBuildMouseWheelDown
          OnMouseWheelUp = sbBuildMouseWheelUp
          OnResize = sbBuildResize
        end
      end
    end
    object tsCustom: TTabSheet
      Caption = 'Custom'
      ImageIndex = 9
      OnResize = tsCustomResize
    end
  end
  object TbQuick: TToolBar
    Left = 0
    Top = 0
    Width = 604
    Height = 21
    ButtonHeight = 21
    ButtonWidth = 83
    Customizable = True
    ShowCaptions = True
    TabOrder = 1
    object ToolButton1: TToolButton
      Left = 0
      Top = 0
      Caption = 'Last Hue'
      ImageIndex = 0
    end
    object ToolButton2: TToolButton
      Left = 83
      Top = 0
      Caption = 'Last Destination'
      ImageIndex = 1
    end
    object ToolButton3: TToolButton
      Left = 166
      Top = 0
      Caption = 'Last Command'
      ImageIndex = 2
    end
    object ToolButton4: TToolButton
      Left = 249
      Top = 0
      Caption = 'Last NPC'
      ImageIndex = 3
    end
    object ToolButton5: TToolButton
      Left = 332
      Top = 0
      Caption = 'Last Multi'
      ImageIndex = 4
    end
    object ToolButton6: TToolButton
      Left = 415
      Top = 0
      Caption = 'Tele'
      ImageIndex = 5
    end
    object ToolButton7: TToolButton
      Left = 498
      Top = 0
      Caption = 'Save'
      ImageIndex = 6
    end
  end
  object XPManifest1: TXPManifest
    Left = 572
    Top = 28
  end
  object pmTravel: TPopupMenu
    Left = 572
    Top = 25
    object Go1: TMenuItem
      Caption = '&Go'
      OnClick = Go1Click
    end
    object Send1: TMenuItem
      Caption = '&Send'
      OnClick = btnSendClick
    end
    object Center1: TMenuItem
      Caption = '&Center'
      OnClick = Center1Click
    end
    object N1: TMenuItem
      Caption = '-'
    end
    object Felucca1: TMenuItem
      Caption = 'Felucca'
      Checked = True
      GroupIndex = 1
      RadioItem = True
      OnClick = Felucca1Click
    end
    object Trammel1: TMenuItem
      Caption = 'Trammel'
      GroupIndex = 1
      RadioItem = True
      OnClick = Trammel1Click
    end
    object Ilshenar1: TMenuItem
      Caption = 'Ilshenar'
      GroupIndex = 1
      RadioItem = True
      OnClick = Ilshenar1Click
    end
    object Malas1: TMenuItem
      Caption = 'Malas'
      GroupIndex = 1
      RadioItem = True
      OnClick = Malas1Click
    end
    object SamuraiEmpire1: TMenuItem
      Caption = 'Samurai Empire'
      GroupIndex = 1
      OnClick = SamuraiEmpire1Click
    end
  end
  object animtimer: TTimer
    Enabled = False
    Interval = 100
    OnTimer = animtimerTimer
    Left = 572
    Top = 24
  end
  object ontopTimer: TTimer
    Enabled = False
    Interval = 100
    OnTimer = ontopTimerTimer
    Left = 576
    Top = 56
  end
  object btnCustomPopup: TPopupMenu
    Left = 572
    Top = 56
    object AddButton1: TMenuItem
      Caption = '&Add Button'
      OnClick = AddButton1Click
    end
    object Change1: TMenuItem
      Caption = '&Change'
      OnClick = Change1Click
    end
    object Remove1: TMenuItem
      Caption = '&Remove'
      OnClick = Remove1Click
    end
  end
end
