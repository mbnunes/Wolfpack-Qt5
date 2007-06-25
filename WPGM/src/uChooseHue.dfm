object frmChooseHue: TfrmChooseHue
  Left = 364
  Top = 284
  BorderStyle = bsDialog
  Caption = 'Chose Hue'
  ClientHeight = 394
  ClientWidth = 404
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  OnCreate = FormCreate
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 13
  object lblId: TLabel
    Left = 8
    Top = 336
    Width = 12
    Height = 13
    Caption = 'Id:'
  end
  object lblName: TLabel
    Left = 80
    Top = 336
    Width = 31
    Height = 13
    Caption = 'Name:'
  end
  object pbHueSelection: TPaintBox32
    Left = 8
    Top = 8
    Width = 384
    Height = 320
    TabOrder = 0
    OnMouseDown = pbHueSelectionMouseDown
    OnPaintBuffer = pbHueSelectionPaintBuffer
  end
  object btnOk: TButton
    Left = 8
    Top = 360
    Width = 75
    Height = 25
    Caption = 'Ok'
    ModalResult = 1
    TabOrder = 1
  end
  object btnCancel: TButton
    Left = 88
    Top = 360
    Width = 75
    Height = 25
    Caption = 'Cancel'
    ModalResult = 2
    TabOrder = 2
  end
  object tbHue: TTrackBar
    Left = 169
    Top = 360
    Width = 224
    Height = 25
    Max = 31
    Frequency = 2
    Position = 31
    TabOrder = 3
    OnChange = tbHueChange
  end
  object pbHuePreview: TPaintBox32
    Left = 264
    Top = 333
    Width = 128
    Height = 18
    TabOrder = 4
    OnPaintBuffer = pbHuePreviewPaintBuffer
  end
end
