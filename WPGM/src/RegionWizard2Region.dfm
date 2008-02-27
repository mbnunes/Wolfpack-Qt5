object frmRegionW2R: TfrmRegionW2R
  Left = 388
  Top = 134
  Caption = 'Region Wizard - Region 1'
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
    Left = 100
    Top = 140
    Width = 103
    Height = 13
    Caption = 'Name for the Region:'
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
  object ERegionName: TEdit
    Left = 209
    Top = 137
    Width = 121
    Height = 21
    TabOrder = 2
  end
  object Button3: TButton
    Left = 184
    Top = 277
    Width = 75
    Height = 25
    Caption = '&Back'
    TabOrder = 3
    OnClick = Button3Click
  end
end
