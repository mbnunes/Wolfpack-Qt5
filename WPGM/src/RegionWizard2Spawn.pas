unit RegionWizard2Spawn;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, StdCtrls;

type
  TfrmRegionW2S = class(TForm)
    Button2: TButton;
    Button1: TButton;
    Button3: TButton;
    ERegionName: TEdit;
    Label1: TLabel;
    procedure Button1Click(Sender: TObject);
    procedure Button3Click(Sender: TObject);
  private
    { Private declarations }
  public
    { Public declarations }
  end;

var
  frmRegionW2S: TfrmRegionW2S;

implementation

uses RegionWizard, RegionWizard3Spawn;

{$R *.dfm}

procedure TfrmRegionW2S.Button1Click(Sender: TObject);
begin
  if length(ERegionName.Text) = 0 then
    showmessage('You need a name for you Region!')
  else
  begin
    FrmRegionW3S.regionName :=  ERegionName.Text;
    FrmRegionW3S.show;
    close;
  end;
end;

procedure TfrmRegionW2S.Button3Click(Sender: TObject);
begin
  frmRegionW1.show;
  close;
end;

end.
