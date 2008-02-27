object frmRergionW4R: TfrmRergionW4R
  Left = 337
  Top = 190
  Caption = 'Region Wizard - Region 4 - Extra-Options'
  ClientHeight = 310
  ClientWidth = 484
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
    Left = 64
    Top = 127
    Width = 71
    Height = 13
    Caption = 'Guard Owner :'
  end
  object Label2: TLabel
    Left = 247
    Top = 36
    Width = 38
    Height = 13
    Caption = 'MidiList:'
  end
  object Label3: TLabel
    Left = 247
    Top = 96
    Width = 64
    Height = 13
    Caption = 'Rain Chance:'
  end
  object Label4: TLabel
    Left = 247
    Top = 149
    Width = 69
    Height = 13
    Caption = 'Snow Chance:'
  end
  object Label5: TLabel
    Left = 247
    Top = 207
    Width = 40
    Height = 13
    Caption = 'resores:'
  end
  object Button2: TButton
    Left = 361
    Top = 277
    Width = 75
    Height = 25
    Caption = '&Cancel'
    TabOrder = 0
  end
  object Button1: TButton
    Left = 272
    Top = 277
    Width = 75
    Height = 25
    Caption = '&Finish'
    TabOrder = 1
    OnClick = Button1Click
  end
  object Eguardowner: TEdit
    Left = 64
    Top = 146
    Width = 121
    Height = 21
    TabOrder = 2
  end
  object CheckBox1: TCheckBox
    Left = 64
    Top = 104
    Width = 65
    Height = 17
    Caption = 'Guards'
    TabOrder = 3
  end
  object Emidilist: TEdit
    Left = 247
    Top = 55
    Width = 121
    Height = 21
    TabOrder = 4
  end
  object Erainchancemin: TEdit
    Left = 247
    Top = 115
    Width = 58
    Height = 21
    TabOrder = 5
  end
  object ESnowChancemin: TEdit
    Left = 247
    Top = 168
    Width = 58
    Height = 21
    TabOrder = 6
  end
  object EResores: TEdit
    Left = 247
    Top = 226
    Width = 121
    Height = 21
    TabOrder = 7
  end
  object Button3: TButton
    Left = 184
    Top = 277
    Width = 75
    Height = 25
    Caption = '&Back'
    TabOrder = 8
    OnClick = Button3Click
  end
  object Erainchancemax: TEdit
    Left = 311
    Top = 115
    Width = 58
    Height = 21
    TabOrder = 9
  end
  object ESnowChancemax: TEdit
    Left = 311
    Top = 168
    Width = 58
    Height = 21
    TabOrder = 10
  end
end
