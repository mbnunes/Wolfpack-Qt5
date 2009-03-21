unit RegionEditR;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, StdCtrls, ExtCtrls, Buttons;

type
  TFrmRegionEdit = class(TForm)
    Label1: TLabel;
    ERegionName: TEdit;
    Label5: TLabel;
    Label6: TLabel;
    Label7: TLabel;
    Label8: TLabel;
    Label9: TLabel;
    Eguardowner: TEdit;
    Emidilist: TEdit;
    Erainchancemin: TEdit;
    ESnowChancemin: TEdit;
    EResores: TEdit;
    Erainchancemax: TEdit;
    ESnowChancemax: TEdit;
    GroupBox1: TGroupBox;
    Label3: TLabel;
    Label4: TLabel;
    LBSelected: TListBox;
    LBAvaible: TListBox;
    Insert: TButton;
    Remove: TButton;
    GroupBox2: TGroupBox;
    BitBtn1: TBitBtn;
    BitBtn2: TBitBtn;
    Panel1: TPanel;
    procedure InsertClick(Sender: TObject);
    procedure RemoveClick(Sender: TObject);
    procedure BitBtn1Click(Sender: TObject);
    procedure BitBtn2Click(Sender: TObject);
  private
    { Private declarations }
  public
    { Public declarations }
  end;

var
  FrmRegionEdit: TFrmRegionEdit;

implementation

uses Spawnregions;

{$R *.dfm}

procedure TFrmRegionEdit.BitBtn1Click(Sender: TObject);
begin
  frmRegions.regionname := Eregionname.Text;

  if frmRegions.flags = nil then
    frmRegions.flags := tstringList.Create
  else
    frmRegions.flags.Clear;

  frmRegions.flags.AddStrings(lbselected.Items);

  frmRegions.rainchancemin := strTOint(erainchancemin.Text);
  frmRegions.rainchancemax := strTOint(erainchancemax.Text);
  frmRegions.snowchancemin := strTOint(esnowchancemin.Text);
  frmRegions.snowchancemax := strTOint(esnowchancemax.Text);
  frmRegions.guardowner := Eguardowner.Text;
  frmregions.midlist := emidilist.Text;
  frmregions.resores := eresores.Text;
  close;
end;

procedure TFrmRegionEdit.BitBtn2Click(Sender: TObject);
begin
  close;
end;

procedure TFrmRegionEdit.InsertClick(Sender: TObject);
begin
if lbavaible.Items.Count > 0 then
  begin
    if lbavaible.ItemIndex = -1 then
      exit;

    lbselected.Items.Add(lbavaible.Items.Strings[lbavaible.ItemIndex]);
    lbselected.Refresh;
    lbavaible.Items.Delete(lbavaible.ItemIndex);
    lbavaible.Refresh;
  end
  else
    showmessage('There`re no more flags avaibles to add.');

end;

procedure TFrmRegionEdit.RemoveClick(Sender: TObject);
begin
  if lbselected.Items.Count > 0 then
  begin
    if lbselected.ItemIndex = -1 then
      exit;
    lbavaible.Items.Add(lbselected.Items.Strings[lbselected.ItemIndex]);
    lbselected.Items.Delete(lbselected.ItemIndex);
    lbselected.Refresh;
    lbavaible.Refresh;
  end
  else
    showmessage('There`re no more flags to remove.');
end;

end.
