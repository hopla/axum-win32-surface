//---------------------------------------------------------------------------

#ifndef AxumSuperModuleFrmH
#define AxumSuperModuleFrmH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include "AnalogClock.h"
#include "MeterPanel.h"
#include <Graphics.hpp>
#include "GradientFill.h"
#include <Menus.hpp>
#include "ExtendedPanel.h"
#include "VUMeter.h"
#include "BitmapButton.h"
#include <ScktComp.hpp>
#include "EQPanel.h"
#include "PhaseMeter.h"
#include <jpeg.hpp>
#include "libpq-fe.h"
#include "FaderPanel.h"
#include "MambaNetFrm.h"
#include "Knob.h"
#include <ComCtrls.hpp>
#include "PanoramaPanel.h"
#include "ATImage.hpp"
#define MBN_VARARG
#include "mbn.h"
#include "axum_form.h"
#include "MambaNetFrm.h"
#include "EQWindowDlg.h"

//---------------------------------------------------------------------------
class TAxumSuperModuleForm : public TMambaNetForm
{
__published:    // IDE-managed Components
  TATImage *AxumSuperModuleBackgroundATImage;
  TATImage *SmallOffImage;
  TATImage *SmallRedImage;
  TATImage *SmallGreenImage;
  TATImage *SmallYellowImage;
  TATImage *LargeOffImage;
  TATImage *LargeRedImage;
  TATImage *LargeGreenImage;
  TATImage *LargeYellowImage;
  TATImage *Switch3Image;
  TLabel *Display1Label;
  TLabel *Display2Label;
  TATImage *LedOffImage;
  TATImage *LedGreenImage;
  TATImage *LedRedImage;
  TFaderPanel *FaderPanel;
  TLabel *Switch3Label;
  TEQPanel *EQPanel;
  TATImage *Switch4Image;
  TATImage *Switch5Image;
  TATImage *Switch6Image;
  TATImage *Switch7Image;
  TATImage *Switch8Image;
  TATImage *Switch9Image;
  TATImage *Switch10Image;
  TLabel *Switch4Label;
  TLabel *Switch5Label;
  TLabel *Switch6Label;
  TLabel *Switch7Label;
  TLabel *Switch8Label;
  TLabel *Switch9Label;
  TLabel *Switch10Label;
  TLabel *Switch1Label;
  TATImage *Switch1Image;
  TLabel *Switch2Label;
  TATImage *Switch2Image;
  TKnob *Knob1;
  TUpDown *Display2UpDown;
  TLabel *Label1;
  TLabel *Label2;
  TLabel *Switch11Label;
  TATImage *Switch11Image;
  TATImage *Switch12Image;
  TLabel *Switch12Label;
  TKnob *Knob2;
  TLabel *Switch14Label;
  TATImage *Switch14Image;
  TLabel *Label3;
  TUpDown *Display1UpDown;
  TLabel *Label4;
  TLabel *Label5;
  TKnob *Knob3;
  TKnob *Knob4;
  TKnob *Knob5;
  TATImage *Switch15Image;
  TLabel *Switch15Label;
  TATImage *Switch16Image;
  TLabel *Switch16Label;
  TMeterPanel *LeftMeterPanel;
  TMeterPanel *RightMeterPanel;
  TPhaseMeter *PhaseMeter;
  TLabel *Label6;
  TATImage *Switch13Image;
  TLabel *Switch13Label;
  TLabel *Label23;
  TLabel *Label24;
  TATImage *Switch17Image;
  TLabel *Switch17Label;
  TLabel *Label7;
  TPanoramaPanel *PanoramaPanel1;
  TLabel *Knob1Label;
  TLabel *Knob2Label;
  TLabel *Knob3Label;
  TLabel *Knob4Label;
  TLabel *Knob5Label;
  TPanoramaPanel *PanoramaPanel2;
  TATImage *Switch19Image;
  TLabel *Switch19Label;
  TKnob *Knob6;
  TATImage *Switch18Image;
  TLabel *Switch18Label;
  TLabel *Knob6Label;
  TLabel *Label8;
  TPanoramaPanel *PanoramaPanel3;
  TATImage *Switch21Image;
  TLabel *Switch21Label;
  TKnob *Knob7;
  TATImage *Switch20Image;
  TLabel *Switch20Label;
  TLabel *Knob7Label;
  TLabel *Label9;
  TPanoramaPanel *PanoramaPanel4;
  TATImage *Switch23Image;
  TLabel *Switch23Label;
  TKnob *Knob8;
  TATImage *Switch22Image;
  TLabel *Switch22Label;
  TLabel *Knob8Label;
  TLabel *Label10;
  TPanoramaPanel *PanoramaPanel5;
  TATImage *Switch25Image;
  TLabel *Switch25Label;
  TKnob *Knob9;
  TATImage *Switch24Image;
  TLabel *Switch24Label;
  TLabel *Knob9Label;
  TLabel *Label11;
  TPanoramaPanel *PanoramaPanel6;
  TATImage *Switch27Image;
  TLabel *Switch27Label;
  TKnob *Knob10;
  TATImage *Switch26Image;
  TLabel *Switch26Label;
  TLabel *Knob10Label;
  TLabel *Label12;
  TPanoramaPanel *PanoramaPanel7;
  TATImage *Switch29Image;
  TLabel *Switch29Label;
  TKnob *Knob11;
  TATImage *Switch28Image;
  TLabel *Switch28Label;
  TLabel *Knob11Label;
  TLabel *Label13;
  TPanoramaPanel *PanoramaPanel8;
  TATImage *Switch31Image;
  TLabel *Switch31Label;
  TKnob *Knob12;
  TATImage *Switch30Image;
  TLabel *Switch30Label;
  TLabel *Knob12Label;
  TLabel *Label14;
  TPanoramaPanel *PanoramaPanel9;
  TATImage *Switch33Image;
  TLabel *Switch33Label;
  TKnob *Knob13;
  TATImage *Switch32Image;
  TLabel *Switch32Label;
  TLabel *Knob13Label;
  TLabel *Label15;
  TPanoramaPanel *PanoramaPanel10;
  TATImage *Switch35Image;
  TLabel *Switch35Label;
  TKnob *Knob14;
  TATImage *Switch34Image;
  TLabel *Switch34Label;
  TLabel *Knob14Label;
  TLabel *Label16;
  TPanoramaPanel *PanoramaPanel11;
  TATImage *Switch37Image;
  TLabel *Switch37Label;
  TKnob *Knob15;
  TATImage *Switch36Image;
  TLabel *Switch36Label;
  TLabel *Knob15Label;
  TLabel *Label17;
  TPanoramaPanel *PanoramaPanel12;
  TATImage *Switch39Image;
  TLabel *Switch39Label;
  TKnob *Knob16;
  TATImage *Switch38Image;
  TLabel *Switch38Label;
  TLabel *Knob16Label;
  TLabel *Label18;
  TPanoramaPanel *PanoramaPanel13;
  TATImage *Switch41Image;
  TLabel *Switch41Label;
  TKnob *Knob17;
  TATImage *Switch40Image;
  TLabel *Switch40Label;
  TLabel *Knob17Label;
  TLabel *Label19;
  TPanoramaPanel *PanoramaPanel14;
  TATImage *Switch43Image;
  TLabel *Switch43Label;
  TKnob *Knob18;
  TATImage *Switch42Image;
  TLabel *Switch42Label;
  TLabel *Knob18Label;
  TLabel *Label20;
  TPanoramaPanel *PanoramaPanel15;
  TATImage *Switch45Image;
  TLabel *Switch45Label;
  TKnob *Knob19;
  TATImage *Switch44Image;
  TLabel *Switch44Label;
  TLabel *Knob19Label;
  TLabel *Label21;
  TPanoramaPanel *PanoramaPanel16;
  TATImage *Switch47Image;
  TLabel *Switch47Label;
  TKnob *Knob20;
  TATImage *Switch46Image;
  TLabel *Switch46Label;
  TLabel *Knob20Label;
  TLabel *Label22;
  TPanoramaPanel *PanoramaPanel17;
  TATImage *Switch49Image;
  TLabel *Switch49Label;
  TKnob *Knob21;
  TATImage *Switch48Image;
  TLabel *Switch48Label;
  TLabel *Knob21Label;
  TUpDown *LowCutUpDown;
  TLabel *LowCutLabel;
  TLabel *Label25;
  TLabel *Label26;
  TLabel *Label27;
  TLabel *Label28;
  TLabel *Label29;
  TTimer *MeterReleaseTimer;
  TATImage *Image1;
   void __fastcall SwitchMouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
   void __fastcall SwitchMouseUp(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
  void __fastcall FormResize(TObject *Sender);
  void __fastcall SwitchLabelMouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
  void __fastcall SwitchLabelMouseUp(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
  void __fastcall MeterReleaseTimerTimer(TObject *Sender);
  void __fastcall DisplayUpDownClick(TObject *Sender, TUDBtnType Button);
  void __fastcall KnobMouseMove(TObject *Sender, TShiftState Shift, int X,
          int Y);
  void __fastcall PanoramaPanelMouseMove(TObject *Sender,
          TShiftState Shift, int X, int Y);
  void __fastcall EQPanelMouseMove(TObject *Sender, TShiftState Shift,
          int X, int Y);
  void __fastcall EQPanelMouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
  void __fastcall EQPanelMouseUp(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
  void __fastcall LowCutUpDownClick(TObject *Sender, TUDBtnType Button);
  void __fastcall FaderPanelMouseMove(TObject *Sender, TShiftState Shift,
          int X, int Y);
  void __fastcall KnobDblClick(TObject *Sender);
  void __fastcall PanoramaPanelDblClick(TObject *Sender);
  void __fastcall EQPanelDblClick(TObject *Sender);
  void __fastcall Image1Click(TObject *Sender);
private:    // User declarations
  TEQWindowDialog *EQWindow;
  TPicture *GetSmallSwitchPicture(unsigned char Color);
  TPicture *GetLargeSwitchPicture(unsigned char Color);
  int DisplayFontSize;
  int SmallSwitchFontSize;
  int MiddleSwitchFontSize;
  int LargeSwitchFontSize;
  int KnobFontSize;
  int LabelFontSize;
  int LowCutFontSize;
  float PhaseData;
  float LeftMeterData;
  float RightMeterData;
  EQAnchor DragEQ;
public:     // User declarations
   __fastcall TAxumSuperModuleForm(TComponent* Owner, char *url, char *port, char TCP, form_node_info *node_info);
   __fastcall ~TAxumSuperModuleForm();
  void MambaNetError(int code, char *msg);
  void MambaNetOnlineStatus(unsigned long addr, char valid);
  int MambaNetSetActuatorData(unsigned short object, union mbn_data data);
  void ConfigurationInformation(unsigned short object, char func_type, int func_seq, int func_nr, char *Label, char *Description);
  void StartCommunication();
  void CalculateFontSizes();
};
//---------------------------------------------------------------------------
extern PACKAGE TAxumSuperModuleForm *AxumSuperModuleForm;
//---------------------------------------------------------------------------
#endif
