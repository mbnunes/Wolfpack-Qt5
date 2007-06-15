unit uCenter;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, StdCtrls;

type
  TfrmCenter = class(TForm)
    Button1: TButton;
    Map: TComboBox;
    X: TEdit;
    Y: TEdit;
    Label1: TLabel;
    procedure Button1Click(Sender: TObject);
  private
    { Private declarations }
  public
    { Public declarations }
  end;

var
  frmCenter: TfrmCenter;

implementation

uses Main;

{$R *.dfm}

procedure TfrmCenter.Button1Click(Sender: TObject);
begin
  MapCenterX := StrToIntDef(X.Text, MapCenterX);
  MapCenterY := StrToIntDef(Y.Text, MapCenterY);
  MapCenterMap := Map.ItemIndex;
  frmMain.pbMap.Invalidate;
end;

end.
