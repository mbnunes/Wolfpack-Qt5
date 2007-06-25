unit Configuration;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, StdCtrls, Buttons;

type
  TFrmConfig = class(TForm)
    ScrollBox1: TScrollBox;
    SBTiledatamul: TSpeedButton;
    Label1: TLabel;
    Label2: TLabel;
    SBArtidx: TSpeedButton;
    Label3: TLabel;
    SBArtMul: TSpeedButton;
    Label4: TLabel;
    sbanimidx: TSpeedButton;
    Label13: TLabel;
    SBAnim5mul: TSpeedButton;
    Label17: TLabel;
    SBHues: TSpeedButton;
    Label10: TLabel;
    Label11: TLabel;
    Label12: TLabel;
    Label14: TLabel;
    Label15: TLabel;
    Label16: TLabel;
    Label5: TLabel;
    Label6: TLabel;
    Label7: TLabel;
    Label8: TLabel;
    Label9: TLabel;
    SBAnim4Idx: TSpeedButton;
    SBAnim4mul: TSpeedButton;
    SBAnim5Idx: TSpeedButton;
    SBBodyconv: TSpeedButton;
    SBBody: TSpeedButton;
    SBRadarcol: TSpeedButton;
    SbAnimmul: TSpeedButton;
    SBAnim2idx: TSpeedButton;
    SbAnim2mul: TSpeedButton;
    sbanim3idx: TSpeedButton;
    SBanim3mul: TSpeedButton;
    ETiledata: TEdit;
    EArtIdx: TEdit;
    EArt: TEdit;
    EAnimIdx: TEdit;
    EAnim5mul: TEdit;
    EHues: TEdit;
    EAnim4Idx: TEdit;
    EAnim4mul: TEdit;
    EAnim5Idx: TEdit;
    EBodyConv: TEdit;
    EBody: TEdit;
    ERadarcol: TEdit;
    EAnimMul: TEdit;
    EAnim2Idx: TEdit;
    EAnim2mul: TEdit;
    EAnim3Idx: TEdit;
    EAnim3mul: TEdit;
    GroupBox1: TGroupBox;
    Label18: TLabel;
    Emap0: TEdit;
    SBMap0: TSpeedButton;
    Label19: TLabel;
    Estaidx0: TEdit;
    SBStaidx0: TSpeedButton;
    Label20: TLabel;
    Estatics0: TEdit;
    SBStatics0: TSpeedButton;
    Label21: TLabel;
    Emap2: TEdit;
    SBMap2: TSpeedButton;
    Label22: TLabel;
    Estaidx2: TEdit;
    SBStaidx2: TSpeedButton;
    Label23: TLabel;
    Estatics2: TEdit;
    SBStatics2: TSpeedButton;
    Label24: TLabel;
    Emap3: TEdit;
    SBMap3: TSpeedButton;
    Label25: TLabel;
    Estaidx3: TEdit;
    SBStaidx3: TSpeedButton;
    Label26: TLabel;
    Estatics3: TEdit;
    SBStatics3: TSpeedButton;
    Label27: TLabel;
    Emap4: TEdit;
    SBMap4: TSpeedButton;
    Label28: TLabel;
    Estaidx4: TEdit;
    SBStaidx4: TSpeedButton;
    Label29: TLabel;
    Estatics4: TEdit;
    SBStatics4: TSpeedButton;
    OD: TOpenDialog;
    GroupBox2: TGroupBox;
    EMap0x: TEdit;
    Label30: TLabel;
    Label31: TLabel;
    EMap0Y: TEdit;
    Label32: TLabel;
    EMap2x: TEdit;
    Label33: TLabel;
    EMap2y: TEdit;
    Label34: TLabel;
    EMap3x: TEdit;
    Label35: TLabel;
    EMap3y: TEdit;
    EMap4y: TEdit;
    Label36: TLabel;
    EMap4x: TEdit;
    Label37: TLabel;
    Button1: TButton;
    Cancel: TButton;
    procedure FormActivate(Sender: TObject);
    procedure FormClose(Sender: TObject; var Action: TCloseAction);
    procedure FormCreate(Sender: TObject);
    procedure Button1Click(Sender: TObject);
    procedure SBTiledatamulClick(Sender: TObject);
    procedure SBArtidxClick(Sender: TObject);
    procedure SBArtMulClick(Sender: TObject);
    procedure sbanimidxClick(Sender: TObject);
    procedure SbAnimmulClick(Sender: TObject);
    procedure SBAnim2idxClick(Sender: TObject);
    procedure SbAnim2mulClick(Sender: TObject);
    procedure sbanim3idxClick(Sender: TObject);
    procedure SBanim3mulClick(Sender: TObject);
    procedure SBAnim4IdxClick(Sender: TObject);
    procedure SBAnim4mulClick(Sender: TObject);
    procedure SBAnim5IdxClick(Sender: TObject);
    procedure SBAnim5mulClick(Sender: TObject);
    procedure SBBodyconvClick(Sender: TObject);
    procedure SBBodyClick(Sender: TObject);
    procedure SBRadarcolClick(Sender: TObject);
    procedure SBHuesClick(Sender: TObject);
    procedure SBMap0Click(Sender: TObject);
    procedure SBMap2Click(Sender: TObject);
    procedure SBMap3Click(Sender: TObject);
    procedure SBMap4Click(Sender: TObject);
    procedure SBStaidx0Click(Sender: TObject);
    procedure SBStaidx2Click(Sender: TObject);
    procedure SBStaidx3Click(Sender: TObject);
    procedure SBStaidx4Click(Sender: TObject);
    procedure SBStatics0Click(Sender: TObject);
    procedure SBStatics2Click(Sender: TObject);
    procedure SBStatics3Click(Sender: TObject);
    procedure SBStatics4Click(Sender: TObject);
    procedure CancelClick(Sender: TObject);
  private
    { Private declarations }
  public
    { Public declarations }

  end;

var
  FrmConfig: TFrmConfig;
  uopath: string;
  actived,canceled : boolean;


implementation

uses UOUtilities, Main;

{$R *.dfm}

function IsNumber(pcString: PChar): Boolean;
begin
  Result := false;
  while pcString^ <> #0 do // 0 indicates the end of a PChar string
  begin
    if not (pcString^ in ['0'..'9']) then
    begin
      result := true;
      break;
    end;
    Inc(pcString);
  end;
end;

procedure TFrmConfig.Button1Click(Sender: TObject);
var
  er: boolean;
begin
  er := true;
  if (EMap0x.Text = '0') or (EMap0y.Text = '0') or
    (EMap2x.Text = '0') or (EMap2y.Text = '0') or
    (EMap3x.Text = '0') or (EMap3y.Text = '0') or
    (EMap4x.Text = '0') or (EMap4y.Text = '0') then
    begin
      showmessage('The Map Size cannot be ZERO!');
      er := false;
    end;

  if er then
    if (IsNumber(pchar(EMap0x.Text))) or (IsNumber(pchar(EMap0y.Text))) or
    (IsNumber(pchar(EMap2x.Text))) or (IsNumber(pchar(EMap2y.Text))) or
    (IsNumber(pchar(EMap3x.Text))) or (IsNumber(pchar(EMap3y.Text))) or
    (IsNumber(pchar(EMap4x.Text))) or (IsNumber(pchar(EMap4y.Text))) then
    begin
      showmessage('The Map Size is NOT valid! It must be a number.');
      er := false;
    end;
  if er then
    if (strToint(EMap0x.Text) mod 8 <> 0 ) or (strtoint(EMap0y.Text) mod 8 <> 0) or
    (strToint(EMap2x.Text) mod 8 <> 0) or (strToint(EMap2y.Text) mod 8 <> 0) or
    (strToint(EMap3x.Text) mod 8 <> 0) or (strToint(EMap3y.Text) mod 8 <> 0) or
    (strToint(EMap4x.Text) mod 8 <> 0) or (strToint(EMap4y.Text) mod 8 <> 0) then
    begin
      showmessage('The Map Size is NOT valid! It must be divisible by 8.');
      er := false;
    end;
  
  if er then
     close;
end;

procedure TFrmConfig.CancelClick(Sender: TObject);
begin
  canceled := true;
  close;
end;

procedure TFrmConfig.FormActivate(Sender: TObject);
var
   i,c: integer;
   file_ : string;
begin
  if not actived then
  begin
    for I := FILESMIN to FILESMAX do
      begin
          file_ := Config.getString(UOFILES[i] +' Path',UOPath + UOFILES[i]);
          if (not fileexists(file_)) and (UOREQ[i]) then
          begin
            for c := 0 to self.ComponentCount -1 do
              if self.Components[c] is Tedit then
                  if self.Components[c].Tag = i then
                  begin
                    (self.Components[c] as tedit).Color := clRed;
                    (self.Components[c] as tedit).Text := file_;
                    break;
                  end;
          end
          else
          begin
            for c := 0 to self.ComponentCount -1 do
              if self.Components[c] is Tedit then
                  if self.Components[c].Tag = (i+1) then
                  begin
                    (self.Components[c] as tedit).Color := clWindow;
                    (self.Components[c] as tedit).Text := file_;
//                    break;
                  end;
          end;
      end;
    end;

    EMap0x.Text :=  intTOstr(Config.getInt('Map 0 Width', 896) * 8);
    EMap0y.Text :=  intTOstr(Config.getInt('Map 0 Height', 512) * 8);
    EMap2x.Text :=  intTOstr(Config.getInt('Map 2 Width', 288) * 8);
    EMap2y.Text :=  intTOstr(Config.getInt('Map 2 Height', 200) * 8);
    EMap3x.Text :=  intTOstr(Config.getInt('Map 3 Width', 320) * 8);
    EMap3y.Text :=  intTOstr(Config.getInt('Map 3 Height', 256) * 8);
    EMap4x.Text :=  intTOstr(Config.getInt('Map 4 Width', 181) * 8);
    EMap4y.Text :=  intTOstr(Config.getInt('Map 4 Height', 181) * 8);

    actived := true;
end;

procedure TFrmConfig.FormClose(Sender: TObject; var Action: TCloseAction);
var
  i,c : integer;
  texto : string;
begin
  if not canceled then
  begin
    for I := FILESMIN to FILESMAX do
      begin
        for c := 0 to self.ComponentCount -1 do
          if self.Components[c] is Tedit then
            if self.Components[c].Tag = (i+1) then
              begin
                config.setString(UOFILES[i] +' Path',(self.Components[c] as tedit).text);
              end;
    end;

    Config.SetInt('Map 0 Width', strTOINt(EMap0x.Text) div 8);
    Config.setInt('Map 0 Height', strTOINt(EMap0y.Text) div 8);
    Config.SetInt('Map 2 Width', strTOINt(EMap2x.Text) div 8);
    Config.SetInt('Map 2 Height', strTOINt(EMap2y.Text) div 8);
    Config.SetInt('Map 3 Width', strTOINt(EMap3x.Text) div 8);
    Config.SetInt('Map 3 Height', strTOINt(EMap3y.Text) div 8);
    Config.SetInt('Map 4 Width', strTOINt(EMap4x.Text) div 8);
    Config.SetInt('Map 4 Height', strTOINt(EMap4y.Text) div 8);
  end;


  action := cafree;
end;

procedure TFrmConfig.FormCreate(Sender: TObject);
begin
  actived := false;
  canceled := false;
end;

procedure TFrmConfig.sbanimidxClick(Sender: TObject);
begin
  od.InitialDir := UOpath;
  od.Filter := 'anim.idx';
  od.DefaultExt := '.idx';
  if od.Execute then
     eanimidx.Text := od.FileName;
end;

procedure TFrmConfig.SbAnimmulClick(Sender: TObject);
begin
  od.InitialDir := UOpath;
  od.Filter := 'anim.mul';
  od.DefaultExt := '.mul';
  if od.Execute then
     eanimmul.Text := od.FileName;
end;

procedure TFrmConfig.SBArtidxClick(Sender: TObject);
begin
  od.InitialDir := UOpath;
  od.Filter := 'artidx.mul';
  od.DefaultExt := '.mul';
  if od.Execute then
     eartidx.Text := od.FileName;
end;

procedure TFrmConfig.SBArtMulClick(Sender: TObject);
begin
  od.InitialDir := UOpath;
  od.Filter := 'art.mul';
  od.DefaultExt := '.mul';
  if od.Execute then
     eart.Text := od.FileName;
end;

procedure TFrmConfig.SBTiledatamulClick(Sender: TObject);
begin
  od.InitialDir := UOpath;
  od.Filter := 'tiledata.mul';
  od.DefaultExt := '.mul';
  if od.Execute then
     etiledata.Text := od.FileName;
end;

procedure TFrmConfig.SBHuesClick(Sender: TObject);
begin
  od.InitialDir := UOpath;
  od.Filter := 'hues.mul';
  od.DefaultExt := '.mul';
  if od.Execute then
     ehues.Text := od.FileName;
end;

procedure TFrmConfig.SBMap0Click(Sender: TObject);
begin
  od.InitialDir := UOpath;
  od.Filter := 'map0.mul';
  od.DefaultExt := '.mul';
  if od.Execute then
     emap0.Text := od.FileName;
end;

procedure TFrmConfig.SBMap2Click(Sender: TObject);
begin
  od.InitialDir := UOpath;
  od.Filter := 'map2.mul';
  od.DefaultExt := '.mul';
  if od.Execute then
     emap2.Text := od.FileName;
end;

procedure TFrmConfig.SBMap3Click(Sender: TObject);
begin
  od.InitialDir := UOpath;
  od.Filter := 'map3.mul';
  od.DefaultExt := '.mul';
  if od.Execute then
     emap3.Text := od.FileName;
end;

procedure TFrmConfig.SBMap4Click(Sender: TObject);
begin
  od.InitialDir := UOpath;
  od.Filter := 'map4.mul';
  od.DefaultExt := '.mul';
  if od.Execute then
     emap4.Text := od.FileName;
end;

procedure TFrmConfig.SBRadarcolClick(Sender: TObject);
begin
  od.InitialDir := UOpath;
  od.Filter := 'radarcol.mul';
  od.DefaultExt := '.mul';
  if od.Execute then
     eradarcol.Text := od.FileName;
end;

procedure TFrmConfig.SBStaidx0Click(Sender: TObject);
begin
  od.InitialDir := UOpath;
  od.Filter := 'staidx0.mul';
  od.DefaultExt := '.mul';
  if od.Execute then
     estaidx0.Text := od.FileName;
end;

procedure TFrmConfig.SBStaidx2Click(Sender: TObject);
begin
  od.InitialDir := UOpath;
  od.Filter := 'staidx2.mul';
  od.DefaultExt := '.mul';
  if od.Execute then
     estaidx2.Text := od.FileName;
end;

procedure TFrmConfig.SBStaidx3Click(Sender: TObject);
begin
  od.InitialDir := UOpath;
  od.Filter := 'staidx3.mul';
  od.DefaultExt := '.mul';
  if od.Execute then
     estaidx3.Text := od.FileName;
end;

procedure TFrmConfig.SBStaidx4Click(Sender: TObject);
begin
  od.InitialDir := UOpath;
  od.Filter := 'staidx4.mul';
  od.DefaultExt := '.mul';
  if od.Execute then
     estaidx4.Text := od.FileName;
end;

procedure TFrmConfig.SBStatics0Click(Sender: TObject);
begin
  od.InitialDir := UOpath;
  od.Filter := 'statics0.mul';
  od.DefaultExt := '.mul';
  if od.Execute then
     estatics0.Text := od.FileName;
end;

procedure TFrmConfig.SBStatics2Click(Sender: TObject);
begin
  od.InitialDir := UOpath;
  od.Filter := 'statics2.mul';
  od.DefaultExt := '.mul';
  if od.Execute then
     estatics2.Text := od.FileName;
end;

procedure TFrmConfig.SBStatics3Click(Sender: TObject);
begin
  od.InitialDir := UOpath;
  od.Filter := 'statics3.mul';
  od.DefaultExt := '.mul';
  if od.Execute then
     estatics3.Text := od.FileName;
end;

procedure TFrmConfig.SBStatics4Click(Sender: TObject);
begin
  od.InitialDir := UOpath;
  od.Filter := 'statics4.mul';
  od.DefaultExt := '.mul';
  if od.Execute then
     estatics4.Text := od.FileName;
end;

procedure TFrmConfig.SBBodyconvClick(Sender: TObject);
begin
  od.InitialDir := UOpath;
  od.Filter := 'bodyconv.def';
  od.DefaultExt := '.def';
  if od.Execute then
     ebodyconv.Text := od.FileName;
end;

procedure TFrmConfig.SBBodyClick(Sender: TObject);
begin
  od.InitialDir := UOpath;
  od.Filter := 'body.def';
  od.DefaultExt := '.def';
  if od.Execute then
     ebody.Text := od.FileName;
end;

procedure TFrmConfig.SBAnim5IdxClick(Sender: TObject);
begin
  od.InitialDir := UOpath;
  od.Filter := 'anim5.idx';
  od.DefaultExt := '.idx';
  if od.Execute then
     eanim5idx.Text := od.FileName;
end;

procedure TFrmConfig.SBAnim5mulClick(Sender: TObject);
begin
  od.InitialDir := UOpath;
  od.Filter := 'anim5.mul';
  od.DefaultExt := '.mul';
  if od.Execute then
     eanim5mul.Text := od.FileName;
end;

procedure TFrmConfig.SBAnim4mulClick(Sender: TObject);
begin
  od.InitialDir := UOpath;
  od.Filter := 'anim4.mul';
  od.DefaultExt := '.mul';
  if od.Execute then
     eanim4mul.Text := od.FileName;
end;

procedure TFrmConfig.SBAnim4IdxClick(Sender: TObject);
begin
  od.InitialDir := UOpath;
  od.Filter := 'anim4.idx';
  od.DefaultExt := '.idx';
  if od.Execute then
     eanim4idx.Text := od.FileName;
end;

procedure TFrmConfig.sbanim3idxClick(Sender: TObject);
begin
  od.InitialDir := UOpath;
  od.Filter := 'anim3.idx';
  od.DefaultExt := '.idx';
  if od.Execute then
     eanim3idx.Text := od.FileName;
end;

procedure TFrmConfig.SBanim3mulClick(Sender: TObject);
begin
  od.InitialDir := UOpath;
  od.Filter := 'anim3.mul';
  od.DefaultExt := '.mul';
  if od.Execute then
     etiledata.Text := od.FileName;
end;

procedure TFrmConfig.SBAnim2idxClick(Sender: TObject);
begin
  od.InitialDir := UOpath;
  od.Filter := 'anim2.idx';
  od.DefaultExt := '.idx';
  if od.Execute then
     eanim2idx.Text := od.FileName;
end;

procedure TFrmConfig.SbAnim2mulClick(Sender: TObject);
begin
  od.InitialDir := UOpath;
  od.Filter := 'anim2.mul';
  od.DefaultExt := '.mul';
  if od.Execute then
     eanim2mul.Text := od.FileName;
end;

initialization
  uopath := getUoPath;
  

end.
