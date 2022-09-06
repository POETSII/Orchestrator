object ChannelForm: TChannelForm
  Left = 779
  Top = 966
  Width = 440
  Height = 600
  Caption = 'Channel X'
  Color = clBtnFace
  Constraints.MaxHeight = 600
  Constraints.MaxWidth = 440
  Constraints.MinHeight = 600
  Constraints.MinWidth = 440
  DefaultMonitor = dmMainForm
  ParentFont = True
  FormStyle = fsMDIChild
  OldCreateOrder = False
  Position = poDefaultPosOnly
  Visible = True
  OnClose = FormClose
  PixelsPerInch = 96
  TextHeight = 13
  object ChannelPageCtl: TPageControl
    Left = 8
    Top = 8
    Width = 417
    Height = 497
    ActivePage = DeviceTabSheet
    TabIndex = 0
    TabOrder = 0
    object DeviceTabSheet: TTabSheet
      Caption = 'Device'
      object Label1: TLabel
        Left = 8
        Top = 64
        Width = 71
        Height = 13
        Caption = 'Hosting device'
      end
      object Label2: TLabel
        Left = 8
        Top = 248
        Width = 65
        Height = 13
        Caption = 'Acknowledge'
      end
      object Label7: TLabel
        Left = 8
        Top = 16
        Width = 3
        Height = 13
      end
      object Label8: TLabel
        Left = 8
        Top = 192
        Width = 94
        Height = 13
        Caption = 'Update interval (ms)'
      end
      object Label11: TLabel
        Left = 8
        Top = 16
        Width = 90
        Height = 13
        Caption = 'POETS application'
      end
      object Label12: TLabel
        Left = 8
        Top = 296
        Width = 56
        Height = 13
        Caption = 'Data log file'
      end
      object Label13: TLabel
        Left = 224
        Top = 112
        Width = 49
        Height = 13
        Alignment = taRightJustify
        AutoSize = False
        Caption = 'Box'
      end
      object Label14: TLabel
        Left = 224
        Top = 136
        Width = 49
        Height = 13
        Alignment = taRightJustify
        AutoSize = False
        Caption = 'Board'
      end
      object Label15: TLabel
        Left = 224
        Top = 160
        Width = 49
        Height = 13
        Alignment = taRightJustify
        AutoSize = False
        Caption = 'Mailbox'
      end
      object Label16: TLabel
        Left = 224
        Top = 184
        Width = 49
        Height = 13
        Alignment = taRightJustify
        AutoSize = False
        Caption = 'Core'
      end
      object Label17: TLabel
        Left = 224
        Top = 208
        Width = 49
        Height = 13
        Alignment = taRightJustify
        AutoSize = False
        Caption = 'Thread'
      end
      object Label18: TLabel
        Left = 224
        Top = 232
        Width = 49
        Height = 13
        Alignment = taRightJustify
        AutoSize = False
        Caption = 'Device'
      end
      object HostDeviceEdit: TEdit
        Left = 8
        Top = 80
        Width = 385
        Height = 21
        TabOrder = 0
      end
      object LabelEdit: TEdit
        Left = 8
        Top = 264
        Width = 385
        Height = 21
        TabOrder = 2
      end
      object DeviceStartBtn: TButton
        Left = 8
        Top = 408
        Width = 153
        Height = 49
        Caption = 'Start'
        TabOrder = 3
        OnClick = DeviceStartBtnClick
      end
      object DeviceStopBtn: TButton
        Left = 176
        Top = 408
        Width = 153
        Height = 49
        Caption = 'Stop'
        TabOrder = 4
        OnClick = DeviceStopBtnClick
      end
      object ApplicationEdit: TEdit
        Left = 8
        Top = 32
        Width = 385
        Height = 21
        TabOrder = 5
      end
      object IntervalEdit: TEdit
        Left = 8
        Top = 208
        Width = 81
        Height = 21
        ReadOnly = True
        TabOrder = 6
        Text = '1'
      end
      object IntervalUpDown: TUpDown
        Left = 89
        Top = 208
        Width = 16
        Height = 21
        Associate = IntervalEdit
        Min = 1
        Max = 1000
        Position = 1
        TabOrder = 7
        Wrap = False
        OnClick = IntervalUpDownClick
      end
      object GroupBox1: TGroupBox
        Left = 8
        Top = 104
        Width = 209
        Height = 73
        Caption = 'Instrumentation type'
        TabOrder = 8
      end
      object LogFileEdit: TEdit
        Left = 8
        Top = 312
        Width = 385
        Height = 21
        TabOrder = 9
      end
      object BoxEdit: TEdit
        Left = 280
        Top = 112
        Width = 113
        Height = 21
        ReadOnly = True
        TabOrder = 10
      end
      object BoardEdit: TEdit
        Left = 280
        Top = 136
        Width = 113
        Height = 21
        ReadOnly = True
        TabOrder = 11
      end
      object MailBoxEdit: TEdit
        Left = 280
        Top = 160
        Width = 113
        Height = 21
        ReadOnly = True
        TabOrder = 12
      end
      object CoreEdit: TEdit
        Left = 280
        Top = 184
        Width = 113
        Height = 21
        ReadOnly = True
        TabOrder = 13
      end
      object ThreadEdit: TEdit
        Left = 280
        Top = 208
        Width = 113
        Height = 21
        ReadOnly = True
        TabOrder = 14
      end
      object DeviceEdit: TEdit
        Left = 280
        Top = 232
        Width = 113
        Height = 21
        ReadOnly = True
        TabOrder = 15
      end
      object BrowseBtn: TButton
        Left = 312
        Top = 336
        Width = 81
        Height = 25
        Caption = 'Browse'
        TabOrder = 16
        OnClick = BrowseBtnClick
      end
      object POLPanel: TPanel
        Left = 112
        Top = 208
        Width = 25
        Height = 25
        Color = clAqua
        TabOrder = 17
      end
      object Panel1: TPanel
        Left = 16
        Top = 120
        Width = 97
        Height = 49
        BevelOuter = bvNone
        TabOrder = 18
        object SoftRadioBtn: TRadioButton
          Left = 0
          Top = 8
          Width = 81
          Height = 17
          Alignment = taLeftJustify
          Caption = 'Softswitch'
          Checked = True
          TabOrder = 0
          TabStop = True
        end
        object MothRadioBtn: TRadioButton
          Left = 0
          Top = 24
          Width = 81
          Height = 25
          Alignment = taLeftJustify
          Caption = 'Mothership'
          TabOrder = 1
        end
      end
      object Panel2: TPanel
        Left = 120
        Top = 120
        Width = 89
        Height = 49
        BevelOuter = bvNone
        TabOrder = 19
        object Type1Btn: TRadioButton
          Left = 16
          Top = 8
          Width = 57
          Height = 17
          Alignment = taLeftJustify
          Caption = 'Type 1'
          Checked = True
          TabOrder = 0
          TabStop = True
        end
        object Type2Btn: TRadioButton
          Left = 16
          Top = 24
          Width = 57
          Height = 25
          Alignment = taLeftJustify
          BiDiMode = bdLeftToRight
          Caption = 'Type 2'
          ParentBiDiMode = False
          TabOrder = 1
        end
      end
      object Local: TGroupBox
        Left = 8
        Top = 336
        Width = 121
        Height = 57
        Caption = 'Local'
        TabOrder = 20
        object LocalOverwrite: TRadioButton
          Left = 40
          Top = 8
          Width = 73
          Height = 25
          Alignment = taLeftJustify
          Caption = 'Overwrite'
          TabOrder = 0
        end
        object LocalAppend: TRadioButton
          Left = 40
          Top = 24
          Width = 73
          Height = 25
          Alignment = taLeftJustify
          Caption = 'Append'
          TabOrder = 1
        end
      end
      object LocalCheckBox: TCheckBox
        Left = 16
        Top = 352
        Width = 17
        Height = 17
        Alignment = taLeftJustify
        TabOrder = 21
      end
      object GroupBox2: TGroupBox
        Left = 136
        Top = 336
        Width = 121
        Height = 57
        Caption = 'Remote'
        TabOrder = 22
        object RemoteOverwrite: TRadioButton
          Left = 48
          Top = 8
          Width = 65
          Height = 25
          Alignment = taLeftJustify
          Caption = 'Overwrite'
          TabOrder = 1
        end
        object RemoteAppend: TRadioButton
          Left = 48
          Top = 24
          Width = 65
          Height = 25
          Alignment = taLeftJustify
          Caption = 'Append'
          TabOrder = 0
        end
      end
      object RemoteCheckBox: TCheckBox
        Left = 144
        Top = 352
        Width = 17
        Height = 17
        Alignment = taLeftJustify
        TabOrder = 1
      end
    end
    object InjeTabSheet: TTabSheet
      Caption = 'Inject'
      ImageIndex = 2
      object Label5: TLabel
        Left = 8
        Top = 16
        Width = 47
        Height = 13
        Caption = 'Command'
      end
      object Label6: TLabel
        Left = 8
        Top = 320
        Width = 65
        Height = 13
        Caption = 'Acknowledge'
      end
      object InjeSendBtn: TButton
        Left = 8
        Top = 408
        Width = 153
        Height = 49
        Caption = 'Send'
        TabOrder = 0
        OnClick = InjeSendBtnClick
      end
      object InjeCommandEdit: TEdit
        Left = 8
        Top = 32
        Width = 385
        Height = 21
        TabOrder = 1
      end
      object InjeAckEdit: TEdit
        Left = 8
        Top = 336
        Width = 385
        Height = 21
        TabOrder = 2
      end
    end
  end
end
