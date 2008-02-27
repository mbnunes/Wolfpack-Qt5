object frmRegionW1: TfrmRegionW1
  Left = 370
  Top = 183
  Caption = 'Region Wizard'
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
    Left = 56
    Top = 72
    Width = 116
    Height = 13
    Caption = 'Select your region type:'
  end
  object RGRegionType: TRadioGroup
    Left = 176
    Top = 108
    Width = 113
    Height = 81
    Items.Strings = (
      'Regions'
      'SpawnRegions')
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
  object Button2: TButton
    Left = 361
    Top = 277
    Width = 75
    Height = 25
    Caption = '&Cancel'
    TabOrder = 2
    OnClick = Button2Click
  end
end
