unit RegionWizard3Region;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, StdCtrls;

type
  TFrmRegionW3R = class(TForm)
    Button1: TButton;
    Button2: TButton;
    Label1: TLabel;
    LBSelected: TListBox;
    LBAvaible: TListBox;
    Label2: TLabel;
    Label3: TLabel;
    Insert: TButton;
    Remove: TButton;
    Button3: TButton;
    procedure InsertClick(Sender: TObject);
    procedure RemoveClick(Sender: TObject);
    procedure Button2Click(Sender: TObject);
    procedure Button1Click(Sender: TObject);
    procedure Button3Click(Sender: TObject);
  private
    { Private declarations }
  public
    { Public declarations }
    regionName : string;
  end;

var
  FrmRegionW3R: TFrmRegionW3R;

implementation

uses RegionWizard4Region, RegionWizard2Region, Spawnregions;

{$R *.dfm}

procedure TFrmRegionW3R.Button1Click(Sender: TObject);
begin

  frmRergionW4R.regionName := regionName;
  frmRergionW4R.flags := TstringList.Create;
  frmRergionW4R.flags.AddStrings(lbselected.Items);
  frmRergionW4R.show;
  close;
end;

procedure TFrmRegionW3R.Button2Click(Sender: TObject);
begin
  close;
end;

procedure TFrmRegionW3R.Button3Click(Sender: TObject);
begin
  FrmRegionW2R.Show;
  close;
end;

procedure TFrmRegionW3R.InsertClick(Sender: TObject);
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

procedure TFrmRegionW3R.RemoveClick(Sender: TObject);
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
