object FrmRegionW3R: TFrmRegionW3R
  Left = 406
  Top = 179
  Caption = 'Region Wizard - Region 2 - Flags'
  ClientHeight = 310
  ClientWidth = 454
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  FormStyle = fsStayOnTop
  OldCreateOrder = False
  PopupMode = pmExplicit
  PopupParent = frmRegions.Owner
  Position = poDesktopCenter
  PixelsPerInch = 96
  TextHeight = 13
  object Label1: TLabel
    Left = 24
    Top = 8
    Width = 134
    Height = 13
    Caption = 'Select flags for your region:'
  end
  object Label2: TLabel
    Left = 52
    Top = 29
    Width = 73
    Height = 13
    Caption = 'Selected Flags:'
  end
  object Label3: TLabel
    Left = 264
    Top = 29
    Width = 67
    Height = 13
    Caption = 'Avaible Flags:'
  end
  object Button1: TButton
    Left = 272
    Top = 277
    Width = 75
    Height = 25
    Caption = '&Next'
    TabOrder = 0
    OnClick = Button1Click
  end
  object Button2: TButton
    Left = 361
    Top = 277
    Width = 75
    Height = 25
    Caption = '&Cancel'
    TabOrder = 1
    OnClick = Button2Click
  end
  object LBSelected: TListBox
    Left = 52
    Top = 48
    Width = 137
    Height = 209
    ItemHeight = 13
    TabOrder = 2
  end
  object LBAvaible: TListBox
    Left = 264
    Top = 48
    Width = 137
    Height = 209
    ItemHeight = 13
    Items.Strings = (
      'antimagic'
      'cave'
      'escortregion'
      'guarded'
      'instalogout'
      'noagressivemagic'
      'nocriminalcombat'
      'nodecay'
      'noentermessage'
      'nogate'
      'noguardmessage'
      'nohousing'
      'nokillcount'
      'nomark'
      'nomusic'
      'norecallin'
      'norecallout'
      'noteleport'
      'recallshield'
      'safe')
    Sorted = True
    TabOrder = 3
  end
  object Insert: TButton
    Left = 212
    Top = 112
    Width = 25
    Height = 25
    Caption = '<'
    TabOrder = 4
    OnClick = InsertClick
  end
  object Remove: TButton
    Left = 212
    Top = 156
    Width = 25
    Height = 25
    Caption = '>'
    TabOrder = 5
    OnClick = RemoveClick
  end
  object Button3: TButton
    Left = 184
    Top = 277
    Width = 75
    Height = 25
    Caption = '&Back'
    TabOrder = 6
    OnClick = Button3Click
  end
end
