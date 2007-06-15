program ImgView_Layers_Ex;

{%File '..\..\GR32.inc'}

uses
  QForms,
  MainUnit in 'MainUnit.pas' {MainForm},
  NewImageUnit in 'NewImageUnit.pas' {NewImageForm},
  RGBALoaderUnit in 'RGBALoaderUnit.pas' {RGBALoaderForm};

{$R *.res}

begin
  Application.Initialize;
  Application.CreateForm(TMainForm, MainForm);
  Application.CreateForm(TNewImageForm, NewImageForm);
  Application.CreateForm(TRGBALoaderForm, RGBALoaderForm);
  Application.Run;
end.
