unit RegionWizard3Spawn;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, StdCtrls, Buttons;

type
  TFrmRegionW3S = class(TForm)
    CheckBox1: TCheckBox;
    Label1: TLabel;
    EMaxnpcamount: TEdit;
    Label2: TLabel;
    Edelaymin: TEdit;
    Edelaymax: TEdit;
    Label3: TLabel;
    Enpcspercycle: TEdit;
    Label4: TLabel;
    Label5: TLabel;
    Button3: TButton;
    Button1: TButton;
    Button2: TButton;
    ListBox1: TListBox;
    Edit1: TEdit;
    SpeedButton1: TSpeedButton;
    Label6: TLabel;
    SpeedButton2: TSpeedButton;
    procedure Button1Click(Sender: TObject);
    procedure SpeedButton1Click(Sender: TObject);
    procedure SpeedButton2Click(Sender: TObject);
  private
    { Private declarations }
  public
    { Public declarations }
    regionName : string;
  end;

var
  FrmRegionW3S: TFrmRegionW3S;

implementation

uses RegionWizard4Spawn;

{$R *.dfm}

procedure TFrmRegionW3S.Button1Click(Sender: TObject);
begin
  frmregionw4s.group := Tstringlist.Create;
  frmregionw4s.group.AddStrings(listbox1.Items);
  frmregionw4s.active := CheckBox1.Checked;
  frmregionw4s.npcpercycle := strtoint(Enpcspercycle.Text);
  frmregionw4s.maxnpcamount := strtoint(EMaxnpcamount.Text);
  frmregionw4s.delaymin := strtoint(Edelaymin.text);
  frmregionw4s.delaymax := strtoint(Edelaymax.text);
  frmregionw4s.regionName := regionName;
  frmregionw4s.Show;
  close;
end;

procedure TFrmRegionW3S.SpeedButton1Click(Sender: TObject);
begin
  if length(trim(edit1.Text)) <> 0 then
    listbox1.Items.Add(edit1.text);
  
end;

procedure TFrmRegionW3S.SpeedButton2Click(Sender: TObject);
begin
  if listbox1.ItemIndex <> -1 then
    listbox1.DeleteSelected;
end;

end.
