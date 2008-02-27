unit RegionWizard4Region;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, StdCtrls;

type
  TfrmRergionW4R = class(TForm)
    Button2: TButton;
    Button1: TButton;
    Eguardowner: TEdit;
    CheckBox1: TCheckBox;
    Label1: TLabel;
    Label2: TLabel;
    Emidilist: TEdit;
    Label3: TLabel;
    Erainchancemin: TEdit;
    Label4: TLabel;
    ESnowChancemin: TEdit;
    Label5: TLabel;
    EResores: TEdit;
    Button3: TButton;
    Erainchancemax: TEdit;
    ESnowChancemax: TEdit;
    procedure Button3Click(Sender: TObject);
    procedure Button1Click(Sender: TObject);
  private
    { Private declarations }
  public
    { Public declarations }
    flags : tstringlist;
    regionname: string;
  end;

var
  frmRergionW4R: TfrmRergionW4R;

implementation

uses RegionWizard3Region, Spawnregions;

{$R *.dfm}

procedure TfrmRergionW4R.Button1Click(Sender: TObject);
begin
  frmRegions.regionname := regionname;
  frmRegions.flags := tstringList.Create;
  frmRegions.flags.AddStrings(flags);
  flags.Destroy;
  frmRegions.rainchancemin := strTOint(erainchancemin.Text);
  frmRegions.rainchancemax := strTOint(erainchancemax.Text);
  frmRegions.snowchancemin := strTOint(esnowchancemin.Text);
  frmRegions.snowchancemax := strTOint(esnowchancemax.Text);
  frmRegions.guardowner := Eguardowner.Text;
  frmRegions.isguarded := false;
  frmRegions.isguarded := CheckBox1.Checked;
  frmregions.midlist := emidilist.Text;
  frmregions.resores := eresores.Text;
  ShowMessage('Now you can set areas of the to your region.');
  close;
end;

procedure TfrmRergionW4R.Button3Click(Sender: TObject);
begin
  FrmRegionW3R.Show;
  close;
end;

end.
