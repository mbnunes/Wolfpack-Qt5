unit uChooseHue;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, StdCtrls, GR32_Image, ComCtrls;

type
  TfrmChooseHue = class(TForm)
    pbHueSelection: TPaintBox32;
    btnOk: TButton;
    btnCancel: TButton;
    tbHue: TTrackBar;
    lblId: TLabel;
    lblName: TLabel;
    pbHuePreview: TPaintBox32;
    procedure pbHueSelectionPaintBuffer(Sender: TObject);
    procedure tbHueChange(Sender: TObject);
    procedure FormCreate(Sender: TObject);
    procedure pbHueSelectionMouseDown(Sender: TObject;
      Button: TMouseButton; Shift: TShiftState; X, Y: Integer);
    procedure pbHuePreviewPaintBuffer(Sender: TObject);
    procedure FormShow(Sender: TObject);
  private
    { Private declarations }
  public
    { Public declarations }
    ColorId: Word;
  end;

var
  frmChooseHue: TfrmChooseHue;

implementation

uses Main, GR32, Math;

{$R *.dfm}

procedure TfrmChooseHue.pbHueSelectionPaintBuffer(Sender: TObject);
var
  hue, xhue, yhue: Word;
begin
  for xhue := 0 to 46 do
    for yhue := 0 to 63 do begin
      hue := 1 + xhue * 64 + yhue; // First hue: 0x01

      if Hue <= $BB7 then
        // Fill a 6x6 picture
        pbHueSelection.Buffer.FillRect(xhue * 6, yhue * 5,
          xhue * 6 + 5, yhue * 5 + 4, Main.Hues.getColor(Hue, tbHue.Position));            
    end;
end;

procedure TfrmChooseHue.tbHueChange(Sender: TObject);
begin
  pbHueSelection.Invalidate;
end;

procedure TfrmChooseHue.FormCreate(Sender: TObject);
begin
    ColorId := 1;
    lblId.Caption := Format('Id: 0x%x', [ColorId]);
    lblName.Caption := Format('Name: %s', [Main.Hues.getHue(ColorId).Name]);
end;

procedure TfrmChooseHue.pbHueSelectionMouseDown(Sender: TObject;
  Button: TMouseButton; Shift: TShiftState; X, Y: Integer);
var
  xhue, yhue, hue: word;
begin
  xhue := x div 6;
  yhue := y div 5;

  // The black border doesnt trigger anything
  if x mod 6 = 5 then
    exit;

  if y mod 5 = 4 then
    exit;

  hue := 1 + xhue * 64 + yhue;

  if Main.Hues.getHue(hue) = nil then
    exit;

  ColorId := Hue;
  lblId.Caption := Format('Id: 0x%x', [ColorId]);
  lblName.Caption := Format('Name: %s', [Main.Hues.getHue(ColorId).Name + #0]);
  pbHuePreview.Invalidate;
end;

procedure TfrmChooseHue.pbHuePreviewPaintBuffer(Sender: TObject);
var
  i: Integer;
  Width: Integer;
  Hue: Word;
begin
  Hue := ColorId;

  if Hue = 0 then begin
    pbHuePreview.Buffer.Clear(clGray32);
    exit;
  end;

  Width := Ceil(pbHuePreview.Width / 32.0);
  for i := 0 to 31 do
  begin
    pbHuePreview.Buffer.FillRect(i * Width, 0, (i + 1) * Width,
      pbHuePreview.Height, Hues.getColor(Hue, i));
  end;
end;

procedure TfrmChooseHue.FormShow(Sender: TObject);
begin
  pbHuePreview.Invalidate;
end;

end.
