object frmPatternColor: TfrmPatternColor
  Left = 513
  Top = 261
  BorderIcons = [biSystemMenu, biMinimize]
  BorderStyle = bsDialog
  Caption = 'Pattern Tilecolor'
  ClientHeight = 380
  ClientWidth = 415
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  PixelsPerInch = 96
  TextHeight = 13
  object Shape1: TShape
    Left = 168
    Top = 216
    Width = 65
    Height = 65
  end
  object Image1: TImage
    Left = 184
    Top = 240
    Width = 105
    Height = 105
  end
  object Image2: TImage
    Left = 200
    Top = 256
    Width = 105
    Height = 105
  end
  object RadioButton1: TRadioButton
    Left = 8
    Top = 8
    Width = 41
    Height = 17
    Caption = '2x2'
    Checked = True
    TabOrder = 0
    TabStop = True
  end
  object RadioButton2: TRadioButton
    Left = 104
    Top = 8
    Width = 41
    Height = 17
    Caption = '3x3'
    TabOrder = 1
  end
  object RadioButton3: TRadioButton
    Left = 192
    Top = 8
    Width = 41
    Height = 17
    Caption = '4x4'
    TabOrder = 2
  end
  object Button1: TButton
    Left = 8
    Top = 216
    Width = 75
    Height = 25
    Caption = '&Tile'
    TabOrder = 3
  end
  object ImgView321: TImgView32
    Left = 8
    Top = 32
    Width = 225
    Height = 169
    Scale = 1.000000000000000000
    ScrollBars.ShowHandleGrip = True
    ScrollBars.Style = rbsDefault
    SizeGrip = sgNone
    OverSize = 0
    TabOrder = 4
  end
  object Timer1: TTimer
    OnTimer = Timer1Timer
    Left = 120
    Top = 184
  end
end
