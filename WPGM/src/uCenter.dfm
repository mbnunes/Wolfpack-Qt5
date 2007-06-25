object frmCenter: TfrmCenter
  Left = 536
  Top = 411
  BorderStyle = bsToolWindow
  Caption = 'Center Map'
  ClientHeight = 101
  ClientWidth = 140
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  PixelsPerInch = 96
  TextHeight = 13
  object Label1: TLabel
    Left = 66
    Top = 42
    Width = 6
    Height = 20
    Caption = ','
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -16
    Font.Name = 'MS Sans Serif'
    Font.Style = [fsBold]
    ParentFont = False
  end
  object Button1: TButton
    Left = 8
    Top = 70
    Width = 75
    Height = 22
    Caption = '&Center'
    TabOrder = 0
    OnClick = Button1Click
  end
  object Map: TComboBox
    Left = 8
    Top = 8
    Width = 121
    Height = 21
    Style = csDropDownList
    ItemHeight = 13
    ItemIndex = 0
    TabOrder = 1
    Text = 'Felucca'
    Items.Strings = (
      'Felucca'
      'Trammel'
      'Ilshenar'
      'Malas')
  end
  object X: TEdit
    Left = 8
    Top = 40
    Width = 57
    Height = 21
    TabOrder = 2
    Text = 'X'
  end
  object Y: TEdit
    Left = 72
    Top = 40
    Width = 57
    Height = 21
    TabOrder = 3
    Text = 'Y'
  end
end
