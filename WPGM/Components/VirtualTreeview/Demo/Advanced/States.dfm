object StateForm: TStateForm
  Left = 11
  Top = 11
  BorderStyle = bsToolWindow
  Caption = 'Watch Virtual Treeview at work:'
  ClientHeight = 512
  ClientWidth = 423
  Color = clBtnFace
  Font.Charset = ANSI_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Arial Narrow'
  Font.Style = []
  OldCreateOrder = False
  PixelsPerInch = 96
  TextHeight = 15
  object EnableCheckBox: TCheckBox
    Left = 8
    Top = 12
    Width = 141
    Height = 17
    Caption = 'Enable state tracking'
    TabOrder = 0
    OnClick = EnableCheckBoxClick
  end
  object GroupBox1: TGroupBox
    Left = 16
    Top = 36
    Width = 201
    Height = 85
    Caption = ' Changes: '
    TabOrder = 1
    object CheckBox1: TCheckBox
      Left = 8
      Top = 16
      Width = 140
      Height = 17
      Caption = 'Change pending'
      Enabled = False
      TabOrder = 0
    end
    object CheckBox3: TCheckBox
      Left = 8
      Top = 32
      Width = 140
      Height = 17
      Caption = 'Clear focus selection'
      Enabled = False
      TabOrder = 1
    end
    object CheckBox4: TCheckBox
      Left = 8
      Top = 48
      Width = 140
      Height = 17
      Caption = 'Clear pending'
      Enabled = False
      TabOrder = 2
    end
    object CheckBox32: TCheckBox
      Left = 8
      Top = 64
      Width = 140
      Height = 17
      Caption = 'Structure change pending'
      Enabled = False
      TabOrder = 3
    end
  end
  object GroupBox2: TGroupBox
    Left = 16
    Top = 216
    Width = 201
    Height = 149
    Caption = ' Mouse actions: '
    TabOrder = 2
    object CheckBox8: TCheckBox
      Left = 8
      Top = 112
      Width = 140
      Height = 17
      Caption = 'Draw selection pending'
      Enabled = False
      TabOrder = 0
    end
    object CheckBox9: TCheckBox
      Left = 8
      Top = 128
      Width = 140
      Height = 17
      Caption = 'Draw selecting'
      Enabled = False
      TabOrder = 1
    end
    object CheckBox19: TCheckBox
      Left = 8
      Top = 16
      Width = 140
      Height = 17
      Caption = 'Left mouse button down'
      Enabled = False
      TabOrder = 2
    end
    object CheckBox20: TCheckBox
      Left = 8
      Top = 32
      Width = 140
      Height = 17
      Caption = 'Mouse check pending'
      Enabled = False
      TabOrder = 3
    end
    object CheckBox21: TCheckBox
      Left = 8
      Top = 48
      Width = 153
      Height = 17
      Caption = 'Middle mouse button down'
      Enabled = False
      TabOrder = 4
    end
    object CheckBox27: TCheckBox
      Left = 8
      Top = 64
      Width = 140
      Height = 17
      Caption = 'Right mouse button down'
      Enabled = False
      TabOrder = 5
    end
    object CheckBox43: TCheckBox
      Left = 8
      Top = 80
      Width = 129
      Height = 17
      Caption = 'Mouse wheel panning'
      Enabled = False
      TabOrder = 6
    end
    object CheckBox44: TCheckBox
      Left = 8
      Top = 96
      Width = 129
      Height = 17
      Caption = 'Mouse wheel scrolling'
      Enabled = False
      TabOrder = 7
    end
  end
  object GroupBox3: TGroupBox
    Left = 228
    Top = 300
    Width = 185
    Height = 101
    Caption = ' Keyboard actions: '
    TabOrder = 3
    object CheckBox10: TCheckBox
      Left = 8
      Top = 32
      Width = 140
      Height = 17
      Caption = 'Editing'
      Enabled = False
      TabOrder = 0
    end
    object CheckBox11: TCheckBox
      Left = 8
      Top = 48
      Width = 140
      Height = 17
      Caption = 'Edit pending'
      Enabled = False
      TabOrder = 1
    end
    object CheckBox15: TCheckBox
      Left = 8
      Top = 80
      Width = 165
      Height = 17
      Caption = 'Incremental search in progress'
      Enabled = False
      TabOrder = 2
    end
    object CheckBox16: TCheckBox
      Left = 8
      Top = 64
      Width = 153
      Height = 17
      Caption = 'Incremental search pending'
      Enabled = False
      TabOrder = 3
    end
    object CheckBox18: TCheckBox
      Left = 8
      Top = 16
      Width = 140
      Height = 17
      Caption = 'Key check pending'
      Enabled = False
      TabOrder = 4
    end
  end
  object GroupBox4: TGroupBox
    Left = 228
    Top = 36
    Width = 185
    Height = 161
    Caption = ' Clipboard and drag'#39'n drop actions: '
    TabOrder = 4
    object CheckBox5: TCheckBox
      Left = 8
      Top = 96
      Width = 140
      Height = 17
      Caption = 'Clipboard flushing'
      Enabled = False
      TabOrder = 0
    end
    object CheckBox6: TCheckBox
      Left = 8
      Top = 112
      Width = 140
      Height = 17
      Caption = 'Clipboard copy pending'
      Enabled = False
      TabOrder = 1
    end
    object CheckBox7: TCheckBox
      Left = 8
      Top = 128
      Width = 140
      Height = 17
      Caption = 'Clipboard cut pending'
      Enabled = False
      TabOrder = 2
    end
    object CheckBox24: TCheckBox
      Left = 8
      Top = 80
      Width = 157
      Height = 17
      Caption = 'OLE drag'#39'n drop in progress'
      Enabled = False
      TabOrder = 3
    end
    object CheckBox25: TCheckBox
      Left = 8
      Top = 64
      Width = 140
      Height = 17
      Caption = 'OLE drag'#39'n drop pending'
      Enabled = False
      TabOrder = 4
    end
    object CheckBox37: TCheckBox
      Left = 8
      Top = 48
      Width = 161
      Height = 17
      Caption = 'VCL dd with app. drag object'
      Enabled = False
      TabOrder = 5
    end
    object CheckBox41: TCheckBox
      Left = 8
      Top = 16
      Width = 153
      Height = 17
      Caption = 'VCL drag'#39'n drop in progress'
      Enabled = False
      TabOrder = 6
    end
    object CheckBox42: TCheckBox
      Left = 8
      Top = 32
      Width = 140
      Height = 17
      Caption = 'VCL drag'#39'n drop pending'
      Enabled = False
      TabOrder = 7
    end
  end
  object GroupBox5: TGroupBox
    Left = 16
    Top = 124
    Width = 201
    Height = 85
    Caption = ' Tree cache: '
    TabOrder = 5
    object CheckBox31: TCheckBox
      Left = 8
      Top = 16
      Width = 185
      Height = 17
      Caption = 'Tree cache validation stop request'
      Enabled = False
      TabOrder = 0
    end
    object CheckBox36: TCheckBox
      Left = 8
      Top = 64
      Width = 140
      Height = 17
      Caption = 'Tree cache valid'
      Enabled = False
      TabOrder = 1
    end
    object CheckBox39: TCheckBox
      Left = 8
      Top = 32
      Width = 161
      Height = 17
      Caption = 'Tree cache is being validated'
      Enabled = False
      TabOrder = 2
    end
    object CheckBox40: TCheckBox
      Left = 8
      Top = 48
      Width = 161
      Height = 17
      Caption = 'Tree cache invalid or unused'
      Enabled = False
      TabOrder = 3
    end
  end
  object GroupBox6: TGroupBox
    Left = 228
    Top = 204
    Width = 185
    Height = 93
    Caption = ' Collapse/Expand/Scroll: '
    TabOrder = 6
    object CheckBox2: TCheckBox
      Left = 8
      Top = 16
      Width = 133
      Height = 17
      Caption = 'Full collapse in progress'
      Enabled = False
      TabOrder = 0
    end
    object CheckBox12: TCheckBox
      Left = 8
      Top = 32
      Width = 129
      Height = 17
      Caption = 'Full expand in progress'
      Enabled = False
      TabOrder = 1
    end
    object CheckBox28: TCheckBox
      Left = 8
      Top = 48
      Width = 61
      Height = 17
      Caption = 'Scrolling'
      Enabled = False
      TabOrder = 2
    end
    object CheckBox29: TCheckBox
      Left = 8
      Top = 64
      Width = 110
      Height = 17
      Caption = 'Auto scroll pending'
      Enabled = False
      TabOrder = 3
    end
  end
  object GroupBox7: TGroupBox
    Left = 228
    Top = 404
    Width = 185
    Height = 101
    Caption = ' Miscellanous: '
    TabOrder = 7
    object CheckBox13: TCheckBox
      Left = 8
      Top = 32
      Width = 165
      Height = 17
      Caption = 'Last hint window was from VT'
      Enabled = False
      TabOrder = 0
    end
    object CheckBox14: TCheckBox
      Left = 8
      Top = 64
      Width = 140
      Height = 17
      Caption = 'In animation'
      Enabled = False
      TabOrder = 1
    end
    object CheckBox17: TCheckBox
      Left = 8
      Top = 80
      Width = 140
      Height = 17
      Caption = 'Iterating'
      Enabled = False
      TabOrder = 2
    end
    object CheckBox23: TCheckBox
      Left = 8
      Top = 48
      Width = 140
      Height = 17
      Caption = 'Need root count update'
      Enabled = False
      TabOrder = 3
    end
    object CheckBox33: TCheckBox
      Left = 8
      Top = 16
      Width = 140
      Height = 17
      Caption = 'Synchronous mode active'
      Enabled = False
      TabOrder = 4
    end
  end
  object GroupBox8: TGroupBox
    Left = 16
    Top = 372
    Width = 201
    Height = 133
    Caption = ' Window related actions: '
    TabOrder = 8
    object CheckBox22: TCheckBox
      Left = 8
      Top = 112
      Width = 140
      Height = 17
      Caption = 'Default node height scale'
      Enabled = False
      TabOrder = 0
    end
    object CheckBox26: TCheckBox
      Left = 8
      Top = 64
      Width = 140
      Height = 17
      Caption = 'Tree painting'
      Enabled = False
      TabOrder = 1
    end
    object CheckBox30: TCheckBox
      Left = 8
      Top = 48
      Width = 140
      Height = 17
      Caption = 'Window resizing'
      Enabled = False
      TabOrder = 2
    end
    object CheckBox34: TCheckBox
      Left = 8
      Top = 80
      Width = 140
      Height = 17
      Caption = 'Tumb tracking (scrollbar)'
      Enabled = False
      TabOrder = 3
    end
    object CheckBox35: TCheckBox
      Left = 8
      Top = 96
      Width = 140
      Height = 17
      Caption = 'Updates locked'
      Enabled = False
      TabOrder = 4
    end
    object CheckBox38: TCheckBox
      Left = 8
      Top = 16
      Width = 185
      Height = 17
      Caption = 'Windows XP Theme support in use'
      Enabled = False
      TabOrder = 5
    end
    object CheckBox45: TCheckBox
      Left = 8
      Top = 32
      Width = 181
      Height = 17
      Caption = 'Treewindow is under construction'
      Enabled = False
      TabOrder = 6
    end
  end
end
