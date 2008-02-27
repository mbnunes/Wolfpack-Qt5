unit RegionWizard2Region;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, StdCtrls;

type
  TfrmRegionW2R = class(TForm)
    Button1: TButton;
    Button2: TButton;
    Label1: TLabel;
    ERegionName: TEdit;
    Button3: TButton;
    procedure Button2Click(Sender: TObject);
    procedure Button1Click(Sender: TObject);
    procedure Button3Click(Sender: TObject);
  private
    { Private declarations }
  public
    { Public declarations }
  end;

var
  frmRegionW2R: TfrmRegionW2R;

implementation

uses RegionWizard3Region, RegionWizard, Spawnregions;

{$R *.dfm}

procedure TfrmRegionW2R.Button1Click(Sender: TObject);
begin
  if length(ERegionName.Text) = 0 then
    showmessage('You need a name for you Region!')
  else
  begin
    FrmRegionW3R.regionName :=  ERegionName.Text;
    FrmRegionW3R.show;
    close;
  end;

end;

procedure TfrmRegionW2R.Button2Click(Sender: TObject);
begin
  close;
end;

procedure TfrmRegionW2R.Button3Click(Sender: TObject);
begin
  frmRegionW1.show;
  close;
end;

end.
