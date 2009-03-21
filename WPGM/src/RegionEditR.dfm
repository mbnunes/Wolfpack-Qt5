object FrmRegionEdit: TFrmRegionEdit
  Left = 353
  Top = 124
  Caption = 'Region - Edit'
  ClientHeight = 609
  ClientWidth = 441
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  FormStyle = fsStayOnTop
  OldCreateOrder = False
  Position = poDesktopCenter
  PixelsPerInch = 96
  TextHeight = 13
  object Label1: TLabel
    Left = 16
    Top = 41
    Width = 67
    Height = 13
    Caption = 'Region Name:'
  end
  object Label5: TLabel
    Left = 16
    Top = 364
    Width = 71
    Height = 13
    Caption = 'Guard Owner :'
  end
  object Label6: TLabel
    Left = 220
    Top = 364
    Width = 38
    Height = 13
    Caption = 'MidiList:'
  end
  object Label7: TLabel
    Left = 16
    Top = 416
    Width = 64
    Height = 13
    Caption = 'Rain Chance:'
  end
  object Label8: TLabel
    Left = 223
    Top = 416
    Width = 69
    Height = 13
    Caption = 'Snow Chance:'
  end
  object Label9: TLabel
    Left = 16
    Top = 471
    Width = 40
    Height = 13
    Caption = 'resores:'
  end
  object ERegionName: TEdit
    Left = 89
    Top = 38
    Width = 336
    Height = 21
    TabOrder = 0
  end
  object Eguardowner: TEdit
    Left = 16
    Top = 383
    Width = 181
    Height = 21
    TabOrder = 1
  end
  object Emidilist: TEdit
    Left = 220
    Top = 383
    Width = 205
    Height = 21
    TabOrder = 2
  end
  object Erainchancemin: TEdit
    Left = 35
    Top = 435
    Width = 58
    Height = 21
    TabOrder = 3
  end
  object ESnowChancemin: TEdit
    Left = 252
    Top = 435
    Width = 58
    Height = 21
    TabOrder = 4
  end
  object EResores: TEdit
    Left = 16
    Top = 490
    Width = 409
    Height = 21
    TabOrder = 5
  end
  object Erainchancemax: TEdit
    Left = 139
    Top = 435
    Width = 58
    Height = 21
    TabOrder = 6
  end
  object ESnowChancemax: TEdit
    Left = 367
    Top = 435
    Width = 58
    Height = 21
    TabOrder = 7
  end
  object GroupBox1: TGroupBox
    Left = 16
    Top = 76
    Width = 409
    Height = 269
    Caption = 'Regions Flags'
    TabOrder = 8
    object Label3: TLabel
      Left = 24
      Top = 25
      Width = 73
      Height = 13
      Caption = 'Selected Flags:'
    end
    object Label4: TLabel
      Left = 236
      Top = 25
      Width = 67
      Height = 13
      Caption = 'Avaible Flags:'
    end
    object LBSelected: TListBox
      Left = 24
      Top = 44
      Width = 137
      Height = 209
      ItemHeight = 13
      TabOrder = 0
    end
    object LBAvaible: TListBox
      Left = 236
      Top = 44
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
      TabOrder = 1
    end
    object Insert: TButton
      Left = 184
      Top = 108
      Width = 25
      Height = 25
      Caption = '<'
      TabOrder = 2
      OnClick = InsertClick
    end
    object Remove: TButton
      Left = 184
      Top = 152
      Width = 25
      Height = 25
      Caption = '>'
      TabOrder = 3
      OnClick = RemoveClick
    end
  end
  object GroupBox2: TGroupBox
    Left = 64
    Top = 536
    Width = 321
    Height = 57
    TabOrder = 9
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
    TabOrder = 10
  end
end
