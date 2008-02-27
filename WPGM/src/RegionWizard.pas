unit RegionWizard;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, StdCtrls, ExtCtrls;

type
  TfrmRegionW1 = class(TForm)
    Label1: TLabel;
    RGRegionType: TRadioGroup;
    Button1: TButton;
    Button2: TButton;
    procedure Button2Click(Sender: TObject);
    procedure Button1Click(Sender: TObject);
  private
    { Private declarations }
  public
    { Public declarations }
  end;

var
  frmRegionW1: TfrmRegionW1;

implementation

uses RegionWizard2Region, Spawnregions, RegionWizard2Spawn;

{$R *.dfm}

procedure TfrmRegionW1.Button1Click(Sender: TObject);
begin
  if RGregiontype.ItemIndex = -1 then
    showmessage('You must select an option to continue!')
   else if RGregiontype.ItemIndex = 0 then
        begin
          frmRegionW2R.Show;
          frmregions.regiontype := True;
          close;
        end
        else
        begin
          frmRegionW2S.Show;
          frmregions.regiontype := False;
          close;
        end;
      

end;

procedure TfrmRegionW1.Button2Click(Sender: TObject);
begin
  close;
end;

end.
