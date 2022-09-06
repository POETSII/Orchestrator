object WizardForm: TWizardForm
  Left = 627
  Top = 1359
  Width = 907
  Height = 461
  Caption = 'Wizards back passage'
  Color = clSkyBlue
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  PixelsPerInch = 96
  TextHeight = 13
  object Label1: TLabel
    Left = 24
    Top = 32
    Width = 100
    Height = 13
    Caption = 'Dummy launch count'
  end
  object LaunchCountEdit: TEdit
    Left = 24
    Top = 48
    Width = 89
    Height = 21
    ReadOnly = True
    TabOrder = 0
    Text = '0'
  end
  object LaunchCountUpDown: TUpDown
    Left = 113
    Top = 48
    Width = 15
    Height = 21
    Associate = LaunchCountEdit
    Min = 0
    Position = 0
    TabOrder = 1
    Wrap = False
    OnClick = LaunchCountUpDownClick
  end
  object DoneBtn: TButton
    Left = 816
    Top = 392
    Width = 73
    Height = 33
    Caption = 'Done'
    TabOrder = 2
    OnClick = DoneBtnClick
  end
end
