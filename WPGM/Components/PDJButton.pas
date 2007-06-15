
unit PDJButton;
{
*****************************
*         PDJTools          *
*****************************
}
// TPDJButton version 4.01
// Author Peric Djordje
// Freeware Component for D3,D4,D5,D6
// Copyright © 2000-2001 by PDJTools
// Birthday of Component 07.04.2001.
// Home Page: http://www.ptt.yu/korisnici/p/e/pericddn/
// E-mail: pericddn@ptt.yu
// If I' find any errors or rubbish in TPDJButton please send me Your suggest or Reclamation.

{$IFDEF VER100}
  {$DEFINE PDJ_D3}
{$ELSE}
  {$IFDEF VER120}
    {$DEFINE PDJ_D4}
  {$ELSE}
    {$DEFINE PDJ_D5Up}
  {$ENDIF}
{$ENDIF}

interface

uses
  Windows, Messages, SysUtils, Classes, Graphics, Controls, Forms, Dialogs
  ,MMSystem,ExtCtrls,Menus,Buttons,StdCtrls;

  {$R PDJBUTTON.res}


type
  TOnMouseOverEvent = procedure(Sender: TObject) of object;
  TOnMouseOutEvent = procedure(Sender: TObject) of object;
  TLayout=(blGlyphLeft,blGlyphBottom,blGlyphRight,blGlyphTop);
  TNumGlyphs = 1..4;
  TNumGlyphsHot = 1..4;
  TSpacing = 1..5;
  TMargin = -5..5;
  TMarkGap = 1..5;
  TPositionPopup = (puDown,puLeft,puRight);
  TStyle = (stFlat,stDefault,stNone,stRaised,stFlatDot,
  stRaisedDot,stFrameLowered,stFrameRaised);
  TKind = (skCustom, skOK, skCancel, skHelp, skYes, skNo, skClose,
    skAbort, skRetry, skIgnore,skAll,skSilly);
  TColorMarginDefault = (cmDefault,cmCustom);

  TPDJCustomButton = class(TCustomControl)
  private

  MainRect,
  Secenje,
  Duplikat,
  Ispravni,
  Gde,
  CaptRect,
  MarkRect,
  GdeMark:Trect;
  I,
  IG,
  Xpromtext,
  Ypromtext,
  Xveliki,
  Yveliki,
  Ymark,
  Xmark:integer;
  DC: THandle;
             {$IFDEF PDJ_D3}
  {$ELSE}
    BiDiFlags: Longint ;
    {$ENDIF}
    FRepeat,
    FStart:  TTimer;
    FMarkGap:TMarkGap;
    FSpacing:TSpacing;
    FMargin:TMargin;
    FStyle:TStyle;
    FNumGlyphs:TNumGlyphs;
    FNumGlyphsHot:TNumGlyphsHot;
    FKind:TKind;
    FVersion: string;
    FHintTwo:String;
    FPositionPopup:TPositionPopup;
    FPopupMenu: TPopupMenu;
    FOnMouseOver: TOnMouseOverEvent;
    FOnMouseOut: TOnMouseOutEvent;
    FGlyph,
    FIspravniGlyph,
    FGlyphHot,
    FMonoBmp,
    Fmark:TBitmap;
    FLayout:TLayout;
    FMouseInPos,
    FHotTrack,
    FPlayMusic,
    FActive,
    FCancel,
    FDefault,
    FMarkMenu,
    FShowHandCursor,
    Focused,
    FShowFocused,
    FRepeatFunction:Boolean;
    FColorMarginDefault:TColorMarginDefault;
    Fx,
    FRavno,
    FRavno1,
    Fslovo:integer;
    FBoja,
    FBojaHot,
    FHotTrackColor,
    FColorHighLight,
    FColorShadow,
    FColorHighLightPomocni,
    FColorShadowPomocni:TColor;

    function  GetRepeatInterval: integer;
    procedure SetRepeatInterval(const Value: integer);
    procedure SetStartInterval(const Value: integer);
    function  GetStartInterval: integer;
    procedure SetRepeatFunction(ARepeatFunction:boolean);
    function GetVersion: string;
    procedure SetVersion(const Value: string);
    function  GetHintTwo: string;
    procedure DrawGlyph;
    procedure DrawGlyphHot;
    procedure PozicijaGlyph;
    procedure DrawCaption;
    procedure PozijaCaption;
    procedure DrawMark;
    procedure PozicijaMark;
    procedure DrawFocus;
    procedure DrawCaptionEnabled;
    procedure SetShowHandCursor(AShowHandCursor:boolean);
    procedure SetKind(value:TKind);
    procedure SetColorMarginDefault(value:TColorMarginDefault);
    procedure SetShowFocused(AShowFocused: Boolean);
    procedure SetColorPomocni(index: integer; value: TColor);
    procedure SetHintTwo(AHintTwo: string);
    procedure SetMarkGap(value:TMarkGap);
    procedure SetDefault(Value: Boolean);
    procedure SetMarkMenu(AMarkMenu: Boolean);
    procedure SetPositionPopup(value:TPositionPopup);
    procedure SetPlayMusic(APlayMusic:boolean);
    procedure SetHotTrack(AHotTrack:Boolean);
    procedure SetSpacing(value:TSpacing);
    procedure SetMargin(value:TMargin);
    procedure SetStyle(value:TStyle);
    procedure SetNumGlyphs(value: TNumGlyphs);
    procedure SetNumGlyphsHot(value: TNumGlyphsHot);
    procedure SetGlyph(value: TBitmap);
    procedure SetGlyphHot(value: TBitmap);
    procedure SetHotTrackColor(AHotTrackColor: TColor);
    procedure SetLayout(Value:TLayout);
    procedure DrawDisabledBitmap(Canvas: TCanvas; x, y: Integer; bmp: TBitmap);
    procedure CMSysColorChange(var Message: TMessage); message CM_SYSCOLORCHANGE;
    procedure CMColorChanged(var Message: TMessage); message CM_COLORCHANGED;
    procedure CMMouseEnter(var AMsg: TMessage);message CM_MOUSEENTER;
    procedure CMMouseLeave(var AMsg: TMessage);message CM_MOUSELEAVE;
    procedure CmEnabledChanged(var Message: TWmNoParams); message CM_ENABLEDCHANGED;
    procedure CmParentColorChanged(var Message: TWMNoParams); message CM_PARENTCOLORCHANGED;
    procedure CmTextChanged(var Message: TWmNoParams); message CM_TEXTCHANGED;
    procedure CmVisibleChanged(var Message: TWmNoParams); message CM_VISIBLECHANGED;
    procedure CmParentFontChanged(var Message: TWMNoParams); message CM_FONTCHANGED;
    procedure SetPopupMenu(value: TPopupMenu);
    procedure CnCommand(var Message: TWMCommand); message CN_COMMAND;
    procedure CmDialogKey(var Message: TCMDialogKey); message CM_DIALOGKEY;
    procedure CmDialogChar(var Message: TCMDialogChar); message CM_DIALOGCHAR;
    procedure CmFocusChanged(var Message: TCMFocusChanged); message CM_FOCUSCHANGED;
    procedure WMLButtonDblClk (var Message: TWMLButtonDown); message WM_LBUTTONDBLCLK;

  protected
    procedure Click;override;
    procedure SetButtonStyle(ADefault: Boolean); virtual;
    procedure Music(song : pchar);
    procedure Paint;override;
    procedure MouseDown(Button: TMouseButton; Shift: TShiftState;
      X, Y: Integer); override;
    procedure MouseUp(Button: TMouseButton; Shift: TShiftState;
      X, Y: Integer); override;
    procedure Notification(AComponent: TComponent; Operation: TOperation);override;
    procedure DoEnter; override;
    procedure DoExit; override;
    procedure doRepeat(Sender: TObject);
    procedure doStart(Sender: TObject);
    property RepeatAction:boolean read FRepeatFunction write SetRepeatFunction default false;
    property RepeatInterval: integer read GetRepeatInterval write SetRepeatInterval;
    property RepeatStartInterval: integer read GetStartInterval write SetStartInterval;
    property ShowFocused:boolean read FShowFocused write SetShowFocused default True;
    property CursorHandCustom:boolean read FShowHandCursor write SetShowHandCursor default True;
    property ButtonKind:TKind read FKind write SetKind default skCustom;
    property ColorMarginLeftTop: TColor
      index 1 read FColorHighLightPomocni write SetColorPomocni default clBtnHighLight;
    property ColorMarginRightBottom: TColor
      index 2 read FColorShadowPomocni write SetColorPomocni default clBtnShadow;
    property ColorMarginStyle:TColorMarginDefault
      read FColorMarginDefault write SetColorMarginDefault default cmDefault;
    property Version: string read GetVersion write SetVersion;
    property PopupMenuMark:boolean read FMarkMenu write SetMarkMenu default False;
    property PopupMenuMarkGap:TMarkGap read FMarkGap write SetMarkGap default 5;
    property PositionPopup: TPositionPopup read FPositionPopup write SetPositionPopup default puDown;
    property PlayMusic: boolean read FPlayMusic write SetPlayMusic default False;
    property PopupMenu: TPopupMenu read FPopupMenu write SetPopupMenu;
    property HotTrackColor:TColor read FHotTrackColor write SetHotTrackColor default clBlue;
    property HotTrack:Boolean read FHotTrack write SetHotTrack default True;
    property Spacing:TSpacing read FSpacing write SetSpacing default 4;
    property SpacingLayout:TMargin read FMargin write SetMargin default 3;
    property OnMouseOver: TOnMouseOverEvent read FOnMouseOver write FOnMouseOver;
    property OnMouseOut: TOnMouseOutEvent read FOnMouseOut write FOnMouseOut;
    property NumGlyphs: TNumGlyphs read FNumGlyphs write SetNumGlyphs default 2;
    property NumGlyphsHot: TNumGlyphsHot read FNumGlyphsHot write SetNumGlyphsHot default 2;
    property Glyph: TBitmap read FGlyph write SetGlyph;
    property GlyphHot: TBitmap read FGlyphHot write SetGlyphHot;
    property Layout: TLayout read FLayout write SetLayout default blGlyphLeft;
    property ButtonStyle: TStyle read FStyle write SetStyle default stFlat;
    property Cancel: Boolean read FCancel write FCancel default False;
    property Default: Boolean read FDefault write SetDefault default False;
    property HintSecondLine:string read GetHintTwo  write SetHintTwo;

  public
    constructor Create(AOwner: TComponent); override;
    destructor Destroy; override;

    end;

    TPDJButton = class (TPDJCustomButton)

  published

    property RepeatAction;
    property RepeatInterval;
    property RepeatStartInterval;
    property ShowFocused;
    property CursorHandCustom;
    property ButtonKind;
    property ColorMarginLeftTop;
    property ColorMarginRightBottom;
    property ColorMarginStyle;
    property Version;
    property PopupMenuMark;
    property PopupMenuMarkGap;
    property PositionPopup;
    property PlayMusic;
    property PopupMenu;
    property HotTrackColor;
    property HotTrack;
    property Spacing;
    property SpacingLayout;
    property OnMouseOver;
    property OnMouseOut;
    property NumGlyphs;
    property NumGlyphsHot;
    property Glyph;
    property GlyphHot;
    property Layout;
    property ButtonStyle;
    property Cancel;
    property Default;
    property HintSecondLine;
    property Enabled;
    property Caption;
    property ParentFont;
    property Font;
    property OnClick;
    property OnDblClick;
    property OnDragDrop;
    property OnDragOver;
    property OnEndDrag;
    property OnMouseDown;
    property OnMouseUp;
    property OnMouseMove;
    property OnStartDrag;
    property ShowHint;
    property ParentColor;
    property Visible;
    property Color;
    property HelpContext;
    property TabStop;
    property OnKeyDown;
    property OnKeyPress;
    property OnKeyUp;
    property TabOrder;
    property OnEnter;
    property OnExit;
    property Align;
     {$IFDEF PDJ_D5up}
     property OnContextPopup;
      {$ENDIF}
  {$IFDEF PDJ_D3}
  {$ELSE}
    
    property Action;
    property Anchors;
    property Constraints;
    property BiDiMode;
    property ParentBiDiMode;
  {$ENDIF}
  end;

procedure Register;

implementation

procedure Register;
begin
  RegisterComponents('PDJ', [TPDJButton]);
end;

constructor TPDJCustomButton.Create(AOwner: TComponent);
begin
inherited Create(AOwner);
SetBounds(0, 0, 75, 25);
FColorHighLight := clBtnHighLight;
FColorShadow := clBtnShadow;
FColorHighLightPomocni := clBtnHighLight;
FColorShadowPomocni := clBtnShadow;
FMouseInPos:=false;
FLayout:=blGlyphLeft;
Fx:=0;
FNumGlyphs :=2;
FNumGlyphsHot :=2;
FVersion:='Version 4.01, Copyright © 2000-2001 by Peric, http://www.ptt.yu/korisnici/p/e/pericddn/, E-mail: pericddn@ptt.yu';
FRavno:=0;
Fravno1:=1;
FStyle:=stFlat;
FSpacing:=4;
FMargin:=3;
Fslovo:=2;
FHotTrack:=True;
FHotTrackColor:=clBlue;
FPlayMusic:=False;
FPositionPopup:=puDown;
FMarkMenu:=False;
FMarkGap:=5;
FKind:=skCustom;
FGlyph := TBitmap.Create;
FGlyphHot := TBitmap.Create;
FIspravniGlyph := TBitmap.Create;
FMonoBmp := TBitmap.Create;
FShowHandCursor :=True;
FShowFocused:=True;
FColorMarginDefault:=cmDefault;
FRepeat:=TTimer.Create(Self);
FRepeat.Enabled:=false;
FRepeat.Interval:=50;
FRepeat.OnTimer:=doRepeat;
FStart:=TTimer.Create(Self);
FStart.Enabled:=false;
FStart.Interval:=50;
FStart.OnTimer:=doStart;
FRepeatFunction:=False;
end;


procedure TPDJCustomButton.SetVersion(const Value: string);
begin
  FVersion:=FVersion;
end;

function TPDJCustomButton.GetVersion: string;
begin
  Result:=FVersion;
end;


destructor TPDJCustomButton.Destroy;
begin
FStart.Free;
FRepeat.Free;
FGlyph.Free;
FGlyphHot.Free;
FIspravniGlyph.Free;
FMonoBmp.free;
inherited Destroy;
end;

procedure TPDJCustomButton.SetRepeatFunction(ARepeatFunction:boolean);
begin
FRepeatFunction:=ARepeatFunction;
end;

procedure TPDJCustomButton.doRepeat(Sender: TObject);
begin
if FRepeatFunction then
 if assigned(OnClick) then OnClick(Self);
end;

procedure TPDJCustomButton.doStart(Sender: TObject);
begin
if FRepeatFunction then begin
 FStart.Enabled:=false;
 FRepeat.Enabled:=true;
end;
end;

function TPDJCustomButton.GetRepeatInterval: integer;
begin
 Result:=FRepeat.Interval;
end;

function TPDJCustomButton.GetStartInterval: integer;
begin
 Result:=FStart.Interval;
end;

procedure TPDJCustomButton.SetRepeatInterval(const Value: integer);
begin
 FRepeat.Interval:=Value;
end;

procedure TPDJCustomButton.SetStartInterval(const Value: integer);
begin
 FStart.Interval:=Value;
end;

procedure TPDJCustomButton.SetShowFocused(AShowFocused:boolean);
begin
FShowFocused:=AShowFocused;
end;

procedure TPDJCustomButton.SetShowHandCursor(AShowHandCursor:boolean);
begin
FShowHandCursor:=AShowHandCursor;
end;

procedure TPDJCustomButton.SetKind(value:TKind);
begin
  if value <> FKind then
  begin
    FKind := value;
    FGlyph.Free;
    FGlyphHot.Free;
    FIspravniGlyph.Free;
    FMonoBmp.free;
    FGlyph := TBitmap.Create;
    FGlyphHot := TBitmap.Create;
    FIspravniGlyph := TBitmap.Create;
    FMonoBmp := TBitmap.Create;

  end;

  case Fkind of
  skOK:
  begin
  FGlyph.Handle := LoadBitmap(hInstance, 'dagpdjb');
  FGlyphHot.Handle := LoadBitmap(hInstance, 'dapdjb');
  
  if ((csLoading in ComponentState) and (Caption <> '')) or
        (not (csLoading in ComponentState)) then caption:='OK';
  end;
  skCancel:
  begin
  FGlyph.Handle := LoadBitmap(hInstance, 'iksicgpdjb');
  FGlyphHot.Handle := LoadBitmap(hInstance, 'iksicpdjb');
    if ((csLoading in ComponentState) and (Caption <> '')) or
        (not (csLoading in ComponentState)) then caption:='Cancel';
  end;
   skHelp:
  begin
  FGlyph.Handle := LoadBitmap(hInstance, 'pomocgpdjb');
  FGlyphHot.Handle := LoadBitmap(hInstance, 'pomocpdjb');
      if ((csLoading in ComponentState) and (Caption <> '')) or
        (not (csLoading in ComponentState)) then caption:='&Help';
  end;
    skYes:
  begin
  FGlyph.Handle := LoadBitmap(hInstance, 'dagpdjb');
  FGlyphHot.Handle := LoadBitmap(hInstance, 'dapdjb');
        if ((csLoading in ComponentState) and (Caption <> '')) or
        (not (csLoading in ComponentState)) then caption:='&Yes';
  end;
      skNo:
  begin
  FGlyph.Handle := LoadBitmap(hInstance, 'negpdjb');
  FGlyphHot.Handle := LoadBitmap(hInstance, 'nepdjb');
       if ((csLoading in ComponentState) and (Caption <> '')) or
        (not (csLoading in ComponentState)) then caption:='&No';
  end;
       skClose:
  begin
  FGlyph.Handle := LoadBitmap(hInstance, 'exitgpdjb');
  FGlyphHot.Handle := LoadBitmap(hInstance, 'exitpdjb');
         if ((csLoading in ComponentState) and (Caption <> '')) or
        (not (csLoading in ComponentState)) then  caption:='&Close';
  end;
         skAbort:
  begin
  FGlyph.Handle := LoadBitmap(hInstance, 'iksicgpdjb');
  FGlyphHot.Handle := LoadBitmap(hInstance, 'iksicpdjb');
           if ((csLoading in ComponentState) and (Caption <> '')) or
        (not (csLoading in ComponentState)) then   caption:='Abort';
  end;
           skRetry:
  begin
  FGlyph.Handle := LoadBitmap(hInstance, 'Retrigpdjb');
  FGlyphHot.Handle := LoadBitmap(hInstance, 'Retripdjb');
          if ((csLoading in ComponentState) and (Caption <> '')) or
        (not (csLoading in ComponentState)) then caption:='&Retry';
  end;
             skIgnore:
  begin
  FGlyph.Handle := LoadBitmap(hInstance, 'ignogpdjb');
  FGlyphHot.Handle := LoadBitmap(hInstance, 'ignopdjb');
            if ((csLoading in ComponentState) and (Caption <> '')) or
        (not (csLoading in ComponentState)) then caption:='&Ignore';
  end;
               skAll:
  begin
  FGlyph.Handle := LoadBitmap(hInstance, 'svegpdjb');
  FGlyphHot.Handle := LoadBitmap(hInstance, 'svepdjb');
            if ((csLoading in ComponentState) and (Caption <> '')) or
        (not (csLoading in ComponentState)) then  caption:='&All';
  end;

                     skSilly:
  begin
  FGlyph.Handle := LoadBitmap(hInstance, 'glupigpdjb');
  FGlyphHot.Handle := LoadBitmap(hInstance, 'glupipdjb');

    if ((csLoading in ComponentState) and (Caption <> '')) or
        (not (csLoading in ComponentState)) then caption:='&Silly';
  end;
  end;
   if FKind<>skCustom then
    begin
    FNumGlyphs :=1;
    FNumGlyphsHot :=1;

    end;
  Invalidate;
  if (FKind=skCancel) or (FKind=skNo) then
  Cancel:=True else Cancel:=False;
   if (FKind=skOK) or (FKind=skYes) then
  FDefault:=True else FDefault:=False;
end;


procedure TPDJCustomButton.Click;
var
  Form: TCustomForm;
  Control: TWinControl;
  q: TPoint;
begin
////////VP////////// Fixed Vladimir Prieto  {19.02.2002}
    inherited Click;
  case FKind of
    skClose:
      begin
        Form := GetParentForm(Self);
        if Form <> nil then Form.Close;
      end;
    skHelp:
      begin
        Control := Self;
        while (Control <> nil) and (Control.HelpContext = 0) do
          Control := Control.Parent;
        if Control <> nil then Application.HelpContext(Control.HelpContext);
      end;
    skOk:
      begin
        Form := GetParentForm(Self);
        if Form <> nil then Form.ModalResult := mrOk;
      end;
    skCancel:
      begin
        Form := GetParentForm(Self);
        if Form <> nil then Form.ModalResult := mrCancel;
      end;
    skYes:
      begin
        Form := GetParentForm(Self);
        if Form <> nil then Form.ModalResult := mrYes;
      end;
    skNo:
      begin
        Form := GetParentForm(Self);
        if Form <> nil then Form.ModalResult := mrNo;
      end;
    skAbort:
      begin
        Form := GetParentForm(Self);
        if Form <> nil then Form.ModalResult := mrAbort;
      end;
    skIgnore:
      begin
        Form := GetParentForm(Self);
        if Form <> nil then Form.ModalResult := mrIgnore;
      end;
    skRetry:
      begin
        Form := GetParentForm(Self);
        if Form <> nil then Form.ModalResult := mrRetry;
      end;
    skAll:
      begin
        Form := GetParentForm(Self);
        if Form <> nil then Form.ModalResult := mrAll;
      end;
////////VP//////////

  end;

//******************************************************************************

{{fixed 28.10.2001.)}
Begin
if FPopupMenu = nil then exit;
if not enabled then exit;

if FRepeatFunction then FStart.Enabled:=true;
Fslovo:=0;
IG:=1;

DC := Canvas.Handle;
MainRect:=clientRect;

 case FStyle of stDefault,stFrameLowered,stFrameRaised:
Canvas.brush.color:=clBtnFace;
 stFlat,stNone,stRaised,stFlatDot,stRaisedDot:
 Canvas.brush.color:=color;
 end;

Frame3d(Canvas, MainRect, FColorShadow,FColorHighLight,Fravno1);
Canvas.FillRect(MainRect);
case FStyle of stFrameRaised:
begin
 inc(MainRect.left); inc(MainRect.top);dec(MainRect.right); dec(MainRect.bottom);
end;
end;
 case FStyle of stDefault,stFrameLowered,stFrameRaised:
DrawEdge(DC, MainRect, BDR_SUNKENOUTER,BF_BOTTOMRIGHT  or BF_TOPLEFT or BF_MIDDLE);
end;
i:=1;
if (FGlyphHot.empty)then DrawGlyph;
if (not FGlyphHot.empty)and (not FGlyph.empty) then
begin
if not enabled then
DrawGlyph else DrawGlyphHot;
end;
 case FStyle of stDefault,stFrameLowered,stFrameRaised:
DrawFocus;
end;
DrawCaption;
///if FPopupMenu = nil then exit;
  FPopupMenu.AutoPopup:= false;
    // Add 20.11.2001.
  FPopupMenu.PopupComponent:=TPDJButton(self);
  q.X:= 0;
  q.Y:= 0;
  q:= ClientToScreen(q);

Case FPositionPopup of
puDown:
begin
  FPopupMenu.Alignment:=paLeft;
  FPopupMenu.Popup(q.X - 1, q.Y + Height);
end;
puRight:
begin
  FPopupMenu.Alignment:=paLeft;
  FPopupMenu.Popup(q.X+width+1, q.Y );
end;
puLeft:
begin
  FPopupMenu.Alignment:=paRight;
  FPopupMenu.Popup(q.X-1, q.Y );
end;
end;


DC := Canvas.Handle;
MainRect:=clientRect;

 case FStyle of stDefault,stFrameLowered,stFrameRaised:
Canvas.brush.color:=clBtnFace;
 stFlat,stNone,stRaised,stFlatDot,stRaisedDot:
 Canvas.brush.color:=color;
 end;

Case FStyle of
stFlat,stNone,stFlatDot:
Frame3d(Canvas, MainRect, FColorShadow,FColorHighLight,0);
stDefault,stFrameLowered,stFrameRaised:
Frame3d(Canvas, MainRect,FColorHighLight ,FColorShadow,1);
stRaised,stRaisedDot:
Frame3d(Canvas, MainRect,FColorHighLight ,FColorShadow,2);
end;
Canvas.FillRect(MainRect);
case FStyle of stFrameRaised:
begin
 inc(MainRect.left); inc(MainRect.top);dec(MainRect.right); dec(MainRect.bottom);
end;
end;
 case FStyle of stDefault,stFrameLowered,stFrameRaised:
DrawEdge(DC, MainRect, BDR_RAISEDINNER, BF_BOTTOMRIGHT  or BF_TOPLEFT or BF_MIDDLE );
end;
IG:=0;
DrawGlyph;
 case FStyle of stDefault,stFrameLowered,stFrameRaised:
DrawFocus;
end;
Fslovo:=1;
DrawCaption;

if FRepeatFunction then begin
 FRepeat.Enabled:=false;
 FStart.Enabled:=false;
end;

i:=0;
end;
//******************************************************************************
end;

procedure TPDJCustomButton.SetMarkMenu(AMarkMenu: Boolean);
begin
FMarkMenu:=AMarkMenu;
Invalidate;
end;

procedure TPDJCustomButton.SetMarkGap(value: TMarkGap);
begin
  if value <> FMarkGap then
  begin
    FMarkGap := value;
    Invalidate;
  end;
end;

function TPDJCustomButton.GetHintTwo: string;
begin
     Result:=FHintTwo;
end;

procedure TPDJCustomButton.SetHintTwo(AHintTwo:String);
begin
FHintTwo:=AHintTwo;
if csDesigning in ComponentState then
 Exit;
  if hint<>'' then
  if FHintTwo<>'' then
hint:=hint+#13+FHintTwo else
hint:=hint;
end;


procedure TPDJCustomButton.SetButtonStyle(ADefault: Boolean);
const
  BS_MASK = $000F;
var
  Style: Word;
begin
  if HandleAllocated then
  begin
    if ADefault then Style := BS_DEFPUSHBUTTON else Style := BS_PUSHBUTTON;
    if GetWindowLong(Handle, GWL_STYLE) and BS_MASK <> Style then
      SendMessage(Handle, BM_SETSTYLE, Style, 1);
  end;
end;

procedure TPDJCustomButton.DoEnter;
begin
  inherited DoEnter;
  Case FStyle of stDefault,stFrameLowered,stFrameRaised:
   Focused := true;
end;
end;

procedure TPDJCustomButton.DoExit;
begin
  inherited DoExit;
   Case FStyle of stDefault,stFrameLowered,stFrameRaised:
   begin
   Focused := false;
 if ShowFocused then Invalidate;
  end;
  end;
end;

procedure TPDJCustomButton.SetDefault(Value: Boolean);
begin
Fdefault:=value;
end;

procedure TPDJCustomButton.CMFocusChanged(var Message: TCMFocusChanged);
begin

  with Message do
    if Sender is TPDJCustomButton then
    begin
      FActive := Sender = Self;
      end
    else
    begin
    FActive := FDefault;
    end;

      begin  SetButtonStyle(FActive);
  Case FStyle of stDefault,stFrameLowered,stFrameRaised:
  begin

    if focused then
    begin
    if FShowFocused then Invalidate;

    end;
    end;
    end;  
end; inherited;
end;

procedure TPDJCustomButton.DrawFocus;
begin
Case FStyle of stDefault,stFrameLowered,stFrameRaised:
  begin
if FShowFocused then begin
if  focused then
DrawFocusrect(Canvas.Handle,Rect(MainRect.left+3,MainRect.top+3,MainRect.Right-3,MainRect.bottom-3));

end ;
end;
end;
end;




procedure TPDJCustomButton.CNCommand(var Message: TWMCommand);
begin
  if Message.NotifyCode = BN_CLICKED then Click;
end;

procedure TPDJCustomButton.CMDialogKey(var Message: TCMDialogKey);

begin

  with Message do
    if (((CharCode = VK_RETURN) and FActive) or ((CharCode = VK_ESCAPE) and FCancel))
       and (KeyDataToShiftState(Message.KeyData) = []) and CanFocus then
    begin
       
       Click;


       Result := 1;

    end
    else
      inherited;
end;

procedure TPDJCustomButton.CMDialogChar(var Message: TCMDialogChar);
begin
  with Message do
    if IsAccel(CharCode, Caption) and CanFocus then
    begin
      Click;
      Result := 1;
    end
    else
      inherited;
end;


procedure TPDJCustomButton.Music(song : pchar);
var
  h: THandle;
  p: pointer;
begin
  h := FindResource(hInstance,song,'WAV');
  h := LoadResource(hInstance, h);
  p := LockResource(h);
  sndPlaySound(p,SND_MEMORY or SND_SYNC);
  UnLockResource(h);
  FreeResource(h);
 end;

procedure TPDJCustomButton.SetPopupMenu(value: TPopupMenu);
begin
  FPopupMenu := Value;
  if Value <> nil then
  begin
    Value.FreeNotification(Self);
   Invalidate;
  end;
end;

procedure TPDJCustomButton.Notification(AComponent: TComponent; Operation: TOperation);
begin
  inherited Notification(AComponent, Operation);
  if (Operation = opRemove) and (AComponent = FPopupMenu) then FPopupMenu := nil;
end;

procedure TPDJCustomButton.SetHotTrack(AHotTrack:boolean);
begin
FHotTrack:=AHotTrack;
end;

procedure TPDJCustomButton.SetPlayMusic(APlayMusic:boolean);
begin
FPlayMusic:=APlayMusic;
end;

procedure TPDJCustomButton.SetHotTrackColor(AHotTrackColor:TColor);
begin
FHotTrackColor:=AHotTrackColor;
end;

procedure TPDJCustomButton.SetSpacing(value:TSpacing);
begin
  if value <> FSpacing then
  begin
    FSpacing := value;
    Invalidate;
  end;
end;

procedure TPDJCustomButton.SetPositionPopup(value:TPositionPopup);
begin
  if value <> FPositionPopup then
  begin
    FPositionPopup := value;
  end;
end;


procedure TPDJCustomButton.SetMargin(value:TMargin);
begin
  if value <> FMargin then
  begin
    FMargin := value;
    Invalidate;
  end;
end;


procedure TPDJCustomButton.PozicijaGlyph;
begin
with canvas do
begin
if (caption='') and (not FGlyph.empty) then
begin

Xveliki:=(MainRect.left+((MainRect.right-MainRect.left)- (Ispravni.right-Ispravni.left)) div 2);
Yveliki:=(MainRect.top+((MainRect.bottom-MainRect.top)- (Ispravni.bottom-Ispravni.top)) div 2);
begin
if not FMarkMenu then
Gde:= Rect(Xveliki+IG, Yveliki+IG, Xveliki+FispravniGlyph.Width+IG, Yveliki+FispravniGlyph.Height+IG)
else
Gde:= Rect(Xveliki+IG-5, Yveliki+IG, Xveliki+FispravniGlyph.Width+IG-5, Yveliki+FispravniGlyph.Height+IG);
end;
end
else
begin
case FLayout of
blGlyphLeft:
begin

Xveliki:=((MainRect.left+((MainRect.right-MainRect.left)- ((Ispravni.right-Ispravni.left)+(TextWidth(Caption))))div 2))-FMargin;
Yveliki:=(MainRect.top+((MainRect.bottom-MainRect.top)- (Ispravni.bottom-Ispravni.top)) div 2);
Gde:= Rect(Xveliki+IG, Yveliki+IG, Xveliki+FispravniGlyph.Width+IG, Yveliki+FispravniGlyph.Height+IG);
end;
blGlyphBottom:
begin
Xveliki:=(MainRect.left+((MainRect.right-MainRect.left)- (Ispravni.right-Ispravni.left)) div 2);
Yveliki:=(MainRect.top+((MainRect.bottom-MainRect.top)- (Ispravni.bottom-Ispravni.top)) div 2)+ ((textheight(caption))div 2)+FMargin;
Gde:= Rect(Xveliki+IG, Yveliki+IG, Xveliki+FispravniGlyph.Width+IG, Yveliki+FispravniGlyph.Height+IG);
end;
blGlyphTop:
begin
Xveliki:=(MainRect.left+((MainRect.right-MainRect.left)- (Ispravni.right-Ispravni.left)) div 2);
Yveliki:=(MainRect.top+((MainRect.bottom-MainRect.top)- (Ispravni.bottom-Ispravni.top)) div 2)- ((textheight(caption))div 2)-FMargin;
Gde:= Rect(Xveliki+IG, Yveliki+IG, Xveliki+FispravniGlyph.Width+IG, Yveliki+FispravniGlyph.Height+IG);
end;
blGlyphRight:
begin
Xveliki:=((MainRect.left+((MainRect.right-MainRect.left)- ((Ispravni.right-Ispravni.left)-(TextWidth(Caption))))div 2))+FMargin;
Yveliki:=(MainRect.top+((MainRect.bottom-MainRect.top)- (Ispravni.bottom-Ispravni.top)) div 2);
Gde:= Rect(Xveliki+IG, Yveliki+IG, Xveliki+FispravniGlyph.Width+IG, Yveliki+FispravniGlyph.Height+IG);
end;
end;
end;
end;
end;

procedure TPDJCustomButton.PozicijaMark;
begin
with canvas do
begin
if (not FGlyph.empty) then
Ymark:=(Gde.top+((Gde.bottom-Gde.top)- (MarkRect.bottom-MarkRect.top)) div 2);
Xmark:=(Gde.left+FispravniGlyph.Width)+FMarkGap;
GdeMark:=(Rect(Xmark,Ymark,(MarkRect.Right-MarkRect.Left)+Xmark,(MarkRect.bottom-MarkRect.top)+Ymark));
end;
end;



procedure TPDJCustomButton.DrawMark;
var fmarkBoja:Tcolor;
begin
FMark := TBitmap.Create;

if Fglyph.Empty then Exit;
if not FMarkMenu then Exit;
if ((Flayout=blGlyphLeft) or (Flayout=blGlyphRight)) and (Caption>'') then Exit;

FMark.Handle := LoadBitmap(hInstance, 'Strela');

begin
if not enabled then                              
begin
MarkRect := Rect(0, 0, FMark.Width, FMark.Height);
Pozicijamark;
DrawDisabledBitmap(Canvas,GdeMark.left, GdeMark.top, FMark);

end
else
begin
FMarkboja:=FMark.Canvas.Pixels[0,FMark.Height-1];
MarkRect := Rect(0, 0, FMark.Width, FMark.Height);
Pozicijamark;
canvas.BrushCopy(GdeMark, FMark ,MarkRect, fMarkBoja);
Fmark.Free;
end;
end;
end;

procedure TPDJCustomButton.DrawGlyph;
begin
if Fglyph.Empty then exit;
Secenje:=Rect(0,0,FGlyph.Width div FNumGlyphs,Fglyph.Height);
if (FNumGlyphs=2) and (not enabled) then
Secenje:=Rect(FGlyph.Width div FNumGlyphs,0,FGlyph.Width,Fglyph.Height);

Duplikat:=Rect(0,0,FGlyph.Width div FNumGlyphs,Fglyph.Height);
FIspravniGlyph.width:=FGlyph.Width div FNumGlyphs;
FIspravniGlyph.height:=FGlyph.height;
FispravniGlyph.Canvas.Copyrect(Duplikat,FGlyph.canvas,Secenje);
if (not enabled) and (FNumGlyphs<>2) then
begin
Ispravni:=Rect(0,0,FIspravniGlyph.width,FIspravniGlyph.height);
PozicijaGlyph;
DrawDisabledBitmap(Canvas,gde.left, gde.top, FIspravniGlyph);
end
else
begin
Ispravni:=Rect(0,0,FIspravniGlyph.width,FIspravniGlyph.height);
PozicijaGlyph;
Fboja:=FispravniGlyph.Canvas.Pixels[0,FispravniGlyph.Height-1];
Canvas.BrushCopy(Gde,FispravniGlyph,Ispravni,FBoja);
end;
DrawMark;
end;


procedure TPDJCustomButton.DrawDisabledBitmap(Canvas: TCanvas; x, y: Integer; bmp: TBitmap);
begin
FMonoBmp.Assign(bmp);
FMonobmp.Canvas.Brush.Color := clBlack;
FMonobmp.Monochrome := True;
Canvas.Brush.Color := clBtnHighlight;
SetTextColor(Canvas.Handle, clBlack);
SetBkColor(Canvas.Handle, clWhite);
BitBlt(Canvas.Handle, x+1, y+1, bmp.Width, bmp.Height,
FMonobmp.Canvas.Handle, 0, 0, $00E20746);
Canvas.Brush.Color := clBtnShadow;
SetTextColor(Canvas.Handle, clBlack);
SetBkColor(Canvas.Handle, clWhite);
BitBlt(Canvas.Handle, x, y, bmp.Width, bmp.Height,
FMonobmp.Canvas.Handle, 0, 0, $00E20746);
end;

procedure TPDJCustomButton.DrawGlyphHot;

begin
if FglyphHot.Empty then exit;
Secenje:=Rect(0,0,FGlyphHot.Width div FNumGlyphsHot,FglyphHot.Height);
Duplikat:=Rect(0,0,FGlyphHot.Width div FNumGlyphsHot,FglyphHot.Height);
FIspravniGlyph.width:=FGlyphHot.Width div FNumGlyphsHot;
FIspravniGlyph.height:=FGlyphHot.height;
FispravniGlyph.Canvas.Copyrect(Duplikat,FGlyphHot.canvas,Secenje);
Ispravni:=Rect(0,0,FIspravniGlyph.width,FIspravniGlyph.height);
PozicijaGlyph;

FbojaHot:=FispravniGlyph.Canvas.Pixels[0,FispravniGlyph.Height-1];
Canvas.BrushCopy(Gde,FispravniGlyph,Ispravni,FBojaHot);
DrawMark;
end;


procedure TPDJCustomButton.pozijaCaption;
begin
with Canvas do
begin
if (caption<>'') and (FGlyph.empty) then
begin
xpromtext:=(MainRect.left+((MainRect.right-MainRect.left)-Textwidth(Caption)) div 2);
ypromtext:=(MainRect.top+((MainRect.bottom-MainRect.top)-TextHeight(Caption)) div 2);
CaptRect := Rect(Xpromtext+IG, Ypromtext+IG , Xpromtext+TextWidth(Caption)+IG, ypromtext+TextHeight(Caption)+IG);
end
else
begin
case FLayout of
blGlyphLeft:
begin
xpromtext:=gde.right+FSpacing;
ypromtext:=(gde.top+((gde.Bottom-gde.top)-textheight(caption)) div 2);
CaptRect := Rect(Xpromtext, Ypromtext , Xpromtext+TextWidth(Caption), ypromtext+TextHeight(Caption));
end;
blGlyphRight:
begin
xpromtext:= gde.left-textwidth(caption)-FSpacing;
ypromtext:=(gde.top+((gde.Bottom-gde.top)-textheight(caption)) div 2);
CaptRect := Rect(Xpromtext, Ypromtext , Xpromtext+TextWidth(Caption), ypromtext+TextHeight(Caption));
end;
blGlyphBottom:
begin
xpromtext:=gde.left-(Textwidth(Caption)div 2)+((gde.Right-gde.Left)div 2);
ypromtext:=gde.top-8-FSpacing-(TextHeight(Caption)div 2);
CaptRect := Rect(Xpromtext, Ypromtext , Xpromtext+TextWidth(Caption), ypromtext+TextHeight(Caption));
end;
blGlyphTop:
begin
xpromtext:=gde.left-(Textwidth(Caption)div 2)+((gde.Right-gde.Left)div 2);
ypromtext:=(gde.bottom+FSpacing);
CaptRect := Rect(Xpromtext, Ypromtext, Xpromtext+TextWidth(Caption), ypromtext+TextHeight(Caption));
end;
end;
end;
end;
end;


procedure TPDJCustomButton.DrawCaption;
var
Farbica:TColor;
begin
with Canvas do
begin

Font.Assign(Self.Font);



farbica:=font.color;
 if FHotTrack then
    begin
         if Fslovo=0 then font.color:=FHotTrackColor else font.color:=farbica;
    end;

Brush.Style := bsClear;

pozijaCaption;
begin
if not Enabled then DrawCaptionEnabled;
{$IFDEF PDJ_D3}
DrawText(Handle, PChar(Caption), Length(Caption), CaptRect, (DT_EXPANDTABS or DT_center ));
{$ELSE}
DrawText(Handle, PChar(Caption), Length(Caption), CaptRect, (DT_EXPANDTABS or DT_center or BiDiFlags ));
{$ENDIF}
end;

end;
end;



procedure TPDJCustomButton.DrawCaptionEnabled;
var ECaptRect:TRect;
begin
with canvas do
  begin
     Font := Self.Font;
     brush.style:=bsClear;
     {$IFDEF PDJ_D3}
     DrawText(Handle, PChar(Caption), Length(Caption), CaptRect, (DT_EXPANDTABS or DT_center ));
     {$ELSE}
     DrawText(Handle, PChar(Caption), Length(Caption), CaptRect, (DT_EXPANDTABS or DT_center or BiDiFlags ));
     {$ENDIF}
     font.Color :=clBtnHighlight;
     ECaptRect:=Rect(CaptRect.Left+1,CaptRect.top+1,CaptRect.Right+1,CaptRect.Bottom+1);
     {$IFDEF PDJ_D3}
     DrawText(Handle, PChar(Caption), Length(Caption), ECaptRect, (DT_EXPANDTABS or DT_center ));
     {$ELSE}
     DrawText(Handle, PChar(Caption), Length(Caption), ECaptRect, (DT_EXPANDTABS or DT_center or BiDiFlags));
     {$ENDIF}
     font.color :=clBtnShadow;
  end;
end;

procedure TPDJCustomButton.SetGlyph(value: TBitmap);
begin
  if value <> FGlyph then
  begin
    FGlyph.Assign(value);
    if not FGlyph.Empty then
   begin
  if Fglyph.width<Glyph.Height then
   begin
   FNumGlyphs:=1;
   Invalidate;
   end;
   if FGlyph.Width div FGlyph.Height<2 then
   begin
   FNumGlyphs:=1;
   Invalidate;
   end;

    if FGlyph.Width mod FGlyph.Height = 0 then
      begin
        FNumGlyphs := FGlyph.Width div FGlyph.Height;

        if FNumGlyphs > 4 then FNumGlyphs := 1;
        Invalidate
     end;
      end
     else
     Invalidate;

 end;
 FKind:=skCustom;
end;

procedure TPDJCustomButton.SetGlyphHot(value: TBitmap);
begin
  if value <> FGlyphHot then
  begin
    FGlyphHot.Assign(value);
    if not FGlyphHot.Empty then
   begin
   if FglyphHot.width<GlyphHot.Height then
   begin
   FNumGlyphsHot:=1;
   Invalidate;
   end;
   if FGlyphHot.Width div FGlyphHot.Height<2 then
   begin
   FNumGlyphsHot:=1;
   Invalidate;
   end;

    if FGlyphHot.Width mod FGlyphHot.Height = 0 then
      begin
        FNumGlyphsHot := FGlyphHot.Width div FGlyphHot.Height;
        if FNumGlyphsHot > 4 then FNumGlyphsHot := 1;
        Invalidate
     end;
      end
     else
     Invalidate;

 end;
 FKind:=skCustom;
end;

procedure TPDJCustomButton.SetColorMarginDefault(value:TColorMarginDefault);
begin
    FColorMarginDefault := value;
 if FColorMarginDefault=cmDefault then
 begin
     Case FStyle of

stFlat,stRaised,stFrameRaised,stFlatDot,stRaisedDot:
     begin
     FColorHighLight := clBtnHighLight;
     FColorShadow := clBtnShadow;
     FColorHighLightPomocni := FColorHighLight;
     FColorShadowPomocni := FColorShadow;
    end;
stDefault:
    begin
     FColorHighLight := clBtnHighLight;
     FColorShadow := clBlack;
     FColorHighLightPomocni := FColorHighLight;
     FColorShadowPomocni := clBlack;
    end;
stFrameLowered:
    begin
     FColorHighLight := clBtnShadow;
     FColorShadow :=  clBtnHighLight;
     FColorHighLightPomocni := FColorHighLight;
     FColorShadowPomocni := FColorShadow;
     end;
    end;

end;
invalidate;
end;

procedure TPDJCustomButton.SetColorPomocni(index: integer; value: TColor);
begin
case index of
1: if value <> FColorHighLightPomocni then
begin
FColorHighLightPomocni := value;
FColorHighLight :=FColorHighLightPomocni;
    Case FStyle of
stRaisedDot,stFlat,stFrameRaised,stFlatDot,stRaised,stDefault:
    begin
       Case FColorMarginDefault of cmDefault:
begin
    if FColorHighLightPomocni <> clBtnHighLight then
    FColorMarginDefault:=cmCustom;
    end;
    end;
    end;
stFrameLowered:
    begin
       Case FColorMarginDefault of cmDefault:
begin
    if FColorHighLightPomocni <> clBtnShadow then
    FColorMarginDefault:=cmCustom;
    end;
    end;
    end;
end;
Invalidate;
end;

2: if value <> FColorShadowPomocni then
begin
FColorShadowPomocni := value;
FColorShadow := FColorShadowPomocni;
Case FStyle of
stRaisedDot,stFlat,stFrameRaised,stFlatDot,stRaised:
begin
    Case FColorMarginDefault of cmDefault:
        begin
        if FColorShadowPomocni <> clBtnShadow then
        FColorMarginDefault:=cmCustom;
        end;
    end;
end;
stDefault:
begin
    Case FColorMarginDefault of cmDefault:
        begin
        if FColorShadowPomocni <> clBlack then
        FColorMarginDefault:=cmCustom;
        end;
    end;
end;
stFrameLowered:
begin
    Case FColorMarginDefault of cmDefault:
        begin
        if FColorShadowPomocni <> clBtnHighLight then
        FColorMarginDefault:=cmCustom;
        end;
    end;
end;
end;
Invalidate;
end;
end;
end;

procedure TPDJCustomButton.SetStyle(value: TStyle);
begin
  begin
    FStyle := value;
    Case FStyle of

stFlat:
    begin
    Fravno:=0;
    Fravno1:=1;
    Canvas.pen.Style:=psSolid;

   Case FColorMarginDefault of
    cmDefault:
     begin
     FColorHighLight := clBtnHighLight;
     FColorShadow := clBtnShadow;
     FColorHighLightPomocni := FColorHighLight;
     FColorShadowPomocni := FColorShadow;
     end;
    cmCustom:
     begin
     FColorHighLight := FColorHighLightPomocni;
     FColorShadow := FColorShadowPomocni;
     end;
     end;
    end;
stNone:
    begin
    Fravno:=0;
    Fravno1:=0;
    end;
stRaised:
    begin
    Fravno:=2;
    Fravno1:=2;
    Canvas.pen.Style:=psSolid;
       Case FColorMarginDefault of
    cmDefault:
     begin
     FColorHighLight := clBtnHighLight;
     FColorShadow := clBtnShadow;
     FColorHighLightPomocni := FColorHighLight;
     FColorShadowPomocni := FColorShadow;
     end;
    cmCustom:
     begin
     FColorHighLight := FColorHighLightPomocni;
     FColorShadow := FColorShadowPomocni;
     end;
     end;
    end;
stDefault:
    begin
    ParentColor:=False;
    Color:=clBtnface;
    Fravno:=1;
    Fravno1:=1;
    Canvas.pen.Style:=psSolid;
           Case FColorMarginDefault of
    cmDefault:
     begin
     FColorHighLight := clBtnHighLight;
     FColorShadow := clBlack;
     FColorHighLightPomocni := FColorHighLight;
     FColorShadowPomocni := clBlack;
     end;
    cmCustom:
     begin
     FColorHighLight := FColorHighLightPomocni;
     FColorShadow := FColorShadowPomocni;
     end;
     end;
    end;
stFrameRaised:
    begin
    ParentColor:=False;
    Color:=clBtnface;
    Fravno:=1;
    Fravno1:=1;
    Canvas.pen.Style:=psSolid;
        Case FColorMarginDefault of
    cmDefault:
     begin
     FColorHighLight := clBtnHighLight;
     FColorShadow := clBtnShadow;
     FColorHighLightPomocni := FColorHighLight;
     FColorShadowPomocni := FColorShadow;
     end;
    cmCustom:
     begin
     FColorHighLight := FColorHighLightPomocni;
     FColorShadow := FColorShadowPomocni;
     end;
     end;
    end;
stFrameLowered:
    begin
    ParentColor:=False;
    Color:=clBtnface;                                                
    Fravno:=1;
    Fravno1:=1;
    Canvas.pen.Style:=psSolid;
        Case FColorMarginDefault of
    cmDefault:
     begin
     FColorHighLight := clBtnShadow;
     FColorShadow :=  clBtnHighLight;
     FColorHighLightPomocni := FColorHighLight;
     FColorShadowPomocni := FColorShadow;
     end;
    cmCustom:
     begin
     FColorHighLight := FColorHighLightPomocni;
     FColorShadow := FColorShadowPomocni;
     end;
     end;
    end;

stFlatDot:
    begin
    Fravno:=0;
    Fravno1:=1;
    Canvas.pen.Style:=psDot;
       Case FColorMarginDefault of
    cmDefault:
     begin
     FColorHighLight := clBtnHighLight;
     FColorShadow := clBtnShadow;
     FColorHighLightPomocni := FColorHighLight;
     FColorShadowPomocni := FColorShadow;
     end;
    cmCustom:
     begin
     FColorHighLight := FColorHighLightPomocni;
     FColorShadow := FColorShadowPomocni;
     end;
     end;
    end;
stRaisedDot:
    begin
    Fravno:=2;
    Fravno1:=2;
    Canvas.pen.Style:=psDot;
       Case FColorMarginDefault of
    cmDefault:
     begin
     FColorHighLight := clBtnHighLight;
     FColorShadow := clBtnShadow;
     FColorHighLightPomocni := FColorHighLight;
     FColorShadowPomocni := FColorShadow;
     end;
    cmCustom:
     begin
     FColorHighLight := FColorHighLightPomocni;
     FColorShadow := FColorShadowPomocni;
     end;
     end;
    end;

end;

    Invalidate;
  end;
end;



procedure TPDJCustomButton.SetNumGlyphs(value: TNumGlyphs);
begin
  if value <> FNumGlyphs then
  begin
    FNumGlyphs := value;
    Invalidate;
  end;
end;

procedure TPDJCustomButton.SetNumGlyphsHot(value: TNumGlyphsHot);
begin
  if value <> FNumGlyphsHot then
  begin
    FNumGlyphsHot := value;
    Invalidate;
  end;
end;




procedure TPDJCustomButton.SetLayout(value: TLayout);
begin
  if value <> FLayout then FLayout := value;
Invalidate;
end;


procedure TPDJCustomButton.Paint;
begin
inherited Paint;
DC := Canvas.Handle;

if (FStyle=stFlat) or (FStyle=stFlatDot) then begin
FRavno:=0;
if csDesigning in ComponentState then
FRavno:=1;
end;
MainRect:=clientRect;
 case FStyle of stDefault,stFrameLowered,stFrameRaised:
Canvas.brush.color:=clBtnFace;
 stFlat,stNone,stRaised,stFlatDot,stRaisedDot:
 Canvas.brush.color:=color;
 end;


Frame3d(Canvas, MainRect, FColorHighLight,FColorShadow,FRavno);
Canvas.FillRect(MainRect);
case FStyle of stFrameRaised:
begin
 inc(MainRect.left); inc(MainRect.top);dec(MainRect.right); dec(MainRect.bottom);
end;
end;
 case FStyle of stDefault,stFrameLowered,stFrameRaised:
 begin
DrawEdge(DC, MainRect, BDR_RAISEDINNER, BF_BOTTOMRIGHT  or BF_TOPLEFT or BF_MIDDLE );
end;
end;

DrawGlyph;
 case FStyle of stDefault,stFrameLowered,stFrameRaised:
DrawFocus;
end;
DrawCaption;
end;

procedure TPDJCustomButton.CMMouseEnter(var AMsg: TMessage);

begin
if not Enabled then exit;
if Assigned(FOnMouseOver) then FOnMouseOver(Self);
FMouseInPos:=True;
IG:=0;
 case FStyle of stDefault,stFrameLowered,stFrameRaised:
 begin
if ((FPopupmenu<>nil) )  then setfocus;
end;
end;
if FShowHandCursor then begin
Screen.Cursors[1] := LoadCursor(hinstance,'HANDpdjb');
Cursor := 1; end;
Fslovo:=0;
if FPlayMusic then music('PREKO');
DC := Canvas.Handle;
MainRect:=clientRect;

 case FStyle of stDefault,stFrameLowered,stFrameRaised:
Canvas.brush.color:=clBtnFace;
 stFlat,stNone,stRaised,stFlatDot,stRaisedDot:
 Canvas.brush.color:=color;
 end;

    Case FStyle of
stFlat:
    begin
    Fravno:=0;
    Fravno1:=1;
    Canvas.pen.Style:=psSolid;

   Case FColorMarginDefault of
    cmDefault:
     begin
     FColorHighLight := clBtnHighLight;
     FColorShadow := clBtnShadow;
     FColorHighLightPomocni := FColorHighLight;
     FColorShadowPomocni := FColorShadow;
     end;
    cmCustom:
     begin
     FColorHighLight := FColorHighLightPomocni;
     FColorShadow := FColorShadowPomocni;
     end;
     end;
    end;
     end;

Frame3d(Canvas, MainRect, FColorHighLight,FColorShadow,Fravno1);
Canvas.FillRect(MainRect);
case FStyle of stFrameRaised:
begin
 inc(MainRect.left); inc(MainRect.top);dec(MainRect.right); dec(MainRect.bottom);
end;
end;
 case FStyle of stDefault,stFrameLowered,stFrameRaised:
DrawEdge(DC, MainRect, BDR_RAISEDINNER, BF_BOTTOMRIGHT  or BF_TOPLEFT or BF_MIDDLE );
end;
i:=0;
if (FGlyphHot.empty)then DrawGlyph;
if (not FGlyphHot.empty)and (not FGlyph.empty) then
begin
if not enabled then
DrawGlyph else DrawGlyphHot;
end;
 case FStyle of stDefault,stFrameLowered,stFrameRaised:
 DrawFocus;
end;
DrawCaption;

Fx:=0;
end;


procedure TPDJCustomButton.CMMouseLeave(var AMsg: TMessage);
begin

if not Enabled then exit;
if Assigned(FOnMouseOut) then FOnMouseOut(Self);
FMouseInpos:=False;
IG:=0;
if FShowHandCursor then begin
Screen.Cursors[0] := crDefault;
Cursor := 0; end;
if FRepeatFunction then begin
 FRepeat.Enabled:=false;
 FStart.Enabled:=false;
end;
Fslovo:=1;
DC := Canvas.Handle;
MainRect:=clientRect;

 case FStyle of stDefault,stFrameLowered,stFrameRaised:
Canvas.brush.color:=clBtnFace;
 stFlat,stNone,stRaised,stFlatDot,stRaisedDot:
 Canvas.brush.color:=color;
 end;

Frame3d(Canvas, MainRect,FColorHighLight ,FColorShadow,FRavno);
Canvas.FillRect(MainRect);
case FStyle of stFrameRaised:
begin
 inc(MainRect.left); inc(MainRect.top);dec(MainRect.right); dec(MainRect.bottom);
end;
end;
 case FStyle of stDefault,stFrameLowered,stFrameRaised:
DrawEdge(DC, MainRect, BDR_RAISEDINNER, BF_BOTTOMRIGHT  or BF_TOPLEFT or BF_MIDDLE );
end;
DrawGlyph;
 case FStyle of stDefault,stFrameLowered,stFrameRaised:
{if TabStop then} DrawFocus;
end;
DrawCaption;

Fx:=1;
i:=0;
end;


procedure TPDJCustomButton.MouseUp(Button: TMouseButton; Shift: TShiftState;
  X, Y: Integer);
begin
if Button<>mbleft then exit else begin

if not Enabled then exit;
if Fx=1 then exit;

if FRepeatFunction then begin
 FRepeat.Enabled:=false;
 FStart.Enabled:=false;
end;

IG:=0;
DC := Canvas.Handle;
MainRect:=clientRect;

 case FStyle of stDefault,stFrameLowered,stFrameRaised:
Canvas.brush.color:=clBtnFace;
 stFlat,stNone,stRaised,stFlatDot,stRaisedDot:
 Canvas.brush.color:=color;
 end;

Frame3d(Canvas, MainRect, FColorHighLight,FColorShadow,Fravno1);
Canvas.FillRect(MainRect);
case FStyle of stFrameRaised:
begin
 inc(MainRect.left); inc(MainRect.top);dec(MainRect.right); dec(MainRect.bottom);
end;
end;
 case FStyle of stDefault,stFrameLowered,stFrameRaised:

DrawEdge(DC, MainRect, BDR_RAISEDINNER, BF_BOTTOMRIGHT  or BF_TOPLEFT or BF_MIDDLE );

end;
i:=0;

if (FGlyphHot.empty)then DrawGlyph;

if (not FGlyphHot.empty)and (not FGlyph.empty) then
begin
if not enabled then
DrawGlyph else DrawGlyphHot;
end;
 case FStyle of stDefault,stFrameLowered,stFrameRaised:
DrawFocus;
end;
DrawCaption;
end;
 case FStyle of stDefault,stFrameLowered,stFrameRaised:
 begin
 SetFocus;
end;
end;
inherited MouseUp(Button, Shift, X, Y);
end;



procedure TPDJCustomButton.MouseDown(Button: TMouseButton; Shift: TShiftState;
  X, Y: Integer);
    var
  q: TPoint;
Begin
if Button<>mbleft then exit else begin
if not enabled then exit;

if FRepeatFunction then FStart.Enabled:=true;
Fslovo:=0;
IG:=1;

DC := Canvas.Handle;
MainRect:=clientRect;

 case FStyle of stDefault,stFrameLowered,stFrameRaised:
Canvas.brush.color:=clBtnFace;
 stFlat,stNone,stRaised,stFlatDot,stRaisedDot:
 Canvas.brush.color:=color;
 end;

Frame3d(Canvas, MainRect, FColorShadow,FColorHighLight,Fravno1);
Canvas.FillRect(MainRect);
case FStyle of stFrameRaised:
begin
 inc(MainRect.left); inc(MainRect.top);dec(MainRect.right); dec(MainRect.bottom);
end;
end;
 case FStyle of stDefault,stFrameLowered,stFrameRaised:
DrawEdge(DC, MainRect, BDR_SUNKENOUTER,BF_BOTTOMRIGHT  or BF_TOPLEFT or BF_MIDDLE);
end;
i:=1;
if (FGlyphHot.empty)then DrawGlyph;
if (not FGlyphHot.empty)and (not FGlyph.empty) then
begin
if not enabled then
DrawGlyph else DrawGlyphHot;
end;
 case FStyle of stDefault,stFrameLowered,stFrameRaised:
DrawFocus;
end;
DrawCaption;
if FPopupMenu = nil then exit;
  FPopupMenu.AutoPopup:= false;
  // Add 20.11.2001.
  FPopupMenu.PopupComponent:=TPDJButton(self);
  q.X:= 0;
  q.Y:= 0;
  q:= ClientToScreen(q);

Case FPositionPopup of
puDown:
begin
  FPopupMenu.Alignment:=paLeft;
  FPopupMenu.Popup(q.X - 1, q.Y + Height);
end;
puRight:
begin
  FPopupMenu.Alignment:=paLeft;
  FPopupMenu.Popup(q.X+width+1, q.Y );
end;
puLeft:
begin
  FPopupMenu.Alignment:=paRight;
  FPopupMenu.Popup(q.X-1, q.Y );
end;
end;


DC := Canvas.Handle;
MainRect:=clientRect;

 case FStyle of stDefault,stFrameLowered,stFrameRaised:
Canvas.brush.color:=clBtnFace;
 stFlat,stNone,stRaised,stFlatDot,stRaisedDot:
 Canvas.brush.color:=color;
 end;

Case FStyle of
stFlat,stNone,stFlatDot:
Frame3d(Canvas, MainRect, FColorShadow,FColorHighLight,0);
stDefault,stFrameLowered,stFrameRaised:
Frame3d(Canvas, MainRect,FColorHighLight ,FColorShadow,1);
stRaised,stRaisedDot:
Frame3d(Canvas, MainRect,FColorHighLight ,FColorShadow,2);
end;
Canvas.FillRect(MainRect);
case FStyle of stFrameRaised:
begin
 inc(MainRect.left); inc(MainRect.top);dec(MainRect.right); dec(MainRect.bottom);
end;
end;
 case FStyle of stDefault,stFrameLowered,stFrameRaised:
DrawEdge(DC, MainRect, BDR_RAISEDINNER, BF_BOTTOMRIGHT  or BF_TOPLEFT or BF_MIDDLE );
end;
IG:=0;
DrawGlyph;
 case FStyle of stDefault,stFrameLowered,stFrameRaised:
DrawFocus;
end;
Fslovo:=1;
DrawCaption;

if FRepeatFunction then begin
 FRepeat.Enabled:=false;
 FStart.Enabled:=false;
end;

i:=0;
end;

inherited MouseDown(Button, Shift, X, Y);
end;

procedure TPDJCustomButton.WMLButtonDblClk (var Message: TWMLButtonDown);
begin
  inherited;
  Click;
end;

procedure TPDJCustomButton.CmTextChanged(var Message: TWmNoParams);
begin
  inherited;
  Invalidate;
end;

procedure TPDJCustomButton.CmVisibleChanged(var Message: TWmNoParams);
begin
  inherited;
  Invalidate;
end;

procedure TPDJCustomButton.CmParentColorChanged(var Message: TWMNoParams);
begin
  inherited;
  case FStyle of stDefault,stFrameLowered,stFrameRaised: Color :=clBtnFace; end;
  if ParentColor then
  Invalidate;
end;

procedure TPDJCustomButton.CMColorChanged(var Message: TMessage);
begin
  inherited;
  case FStyle of stDefault,stFrameLowered,stFrameRaised: Color :=clBtnFace; end;
  if ParentColor then
  Invalidate;
end;

procedure TPDJCustomButton.CMSysColorChange(var Message: TMessage);
begin
  inherited;
   case FStyle of stDefault,stFrameLowered,stFrameRaised: Color :=clBtnFace; end;
  if ParentColor then
  Invalidate;
end;

procedure TPDJCustomButton.CmEnabledChanged(var Message: TWmNoParams);
begin
  inherited;
  if (not enabled) and (not (csDesigning in ComponentState)) then begin
   IG:=0;
   Fslovo:=1;
DC := Canvas.Handle;
MainRect:=clientRect;

 case FStyle of stDefault,stFrameLowered,stFrameRaised:
Canvas.brush.color:=clBtnFace;
 stFlat,stNone,stRaised,stFlatDot,stRaisedDot:
 Canvas.brush.color:=color;
 end;

Frame3d(Canvas, MainRect, FColorHighLight,FColorShadow,Fravno1);
Canvas.FillRect(MainRect);
case FStyle of stFrameRaised:
begin
 inc(MainRect.left); inc(MainRect.top);dec(MainRect.right); dec(MainRect.bottom);
end;
end;
 case FStyle of stDefault,stFrameLowered,stFrameRaised:
DrawEdge(DC, MainRect, BDR_RAISEDINNER, BF_BOTTOMRIGHT  or BF_TOPLEFT or BF_MIDDLE );
end;
i:=0;
if (FGlyphHot.empty)then DrawGlyph;
if (not FGlyphHot.empty)and (not FGlyph.empty) then
begin
if not enabled then
DrawGlyph else DrawGlyphHot;
end;
 case FStyle of stDefault,stFrameLowered,stFrameRaised:
DrawFocus;
end;
DrawCaption;
end;
  Invalidate;
end;

procedure TPDJCustomButton.CmParentFontChanged(var Message: TWMNoParams);
begin
  inherited;
  Canvas.Font.Assign(Self.Font);
  Invalidate;
end;




end.
