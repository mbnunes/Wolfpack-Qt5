object frmRegionW2S: TfrmRegionW2S
  Left = 371
  Top = 228
  Caption = 'Region Wizard - Spawnregion'
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
  PixelsPerInch = 96
  TextHeight = 13
  object Label1: TLabel
    Left = 71
    Top = 140
    Width = 132
    Height = 13
    Caption = 'Name for the Spawnregion:'
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
    OnClick = Button3Click
  end
  object ERegionName: TEdit
    Left = 209
    Top = 137
    Width = 121
    Height = 21
    TabOrder = 3
  end
end
