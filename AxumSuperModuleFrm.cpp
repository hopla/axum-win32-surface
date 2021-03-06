//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "AxumSuperModuleFrm.h"
#include "SurfaceFrm.h"
#include "DSPToolbox.h"

#include <stdio.h>
#include <dos.h>

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "AnalogClock"
#pragma link "MeterPanel"
#pragma link "GradientFill"
#pragma link "ExtendedPanel"
#pragma link "VUMeter"
#pragma link "BitmapButton"
#pragma link "EQPanel"
#pragma link "PhaseMeter"
#pragma link "FaderPanel"
#pragma link "MambaNetFrm"
#pragma link "Knob"
#pragma link "PanoramaPanel"
#pragma link "ATImage"
#pragma resource "*.dfm"

TAxumSuperModuleForm *AxumSuperModuleForm;

extern void mError(struct mbn_handler *mbn, int code, char *msg);
extern void mOnlineStatus(struct mbn_handler *mbn, unsigned long addr, char valid);
extern int mSetActuatorData(struct mbn_handler *mbn, unsigned short object, union mbn_data data);

//---------------------------------------------------------------------------
__fastcall TAxumSuperModuleForm::TAxumSuperModuleForm(TComponent* Owner, char *url, char *port, char TCP, form_node_info *node_info)
   : TMambaNetForm(Owner)
{
  char err[MBN_ERRSIZE];
  struct mbn_interface *itf;
  PGresult *res;
  char *Query;
  char SurfaceText[16][1024];
  int cnt, cntSwitch;
  struct mbn_object objects[200];
  int cntObject = 0;
  char obj_desc[32];
  int cntBand, cntBuss;

  BackgroundATImage = AxumSuperModuleBackgroundATImage;

  Caption = node_info->name;

  MambaNetAddress = 0x00000000;
  Valid = 0;

  PhaseData = 0;
  LeftMeterData = -50;
  RightMeterData = -50;

  DisplayFontSize =96;
  SmallSwitchFontSize = 96;
  MiddleSwitchFontSize = 96;
  LargeSwitchFontSize = 96;
  KnobFontSize = 96;
  LabelFontSize = 96;
  LowCutFontSize = 96;

  EQWindow = new TEQWindowDialog(this);

  if (TCP)
  {
    if((itf = mbnTCPOpen(url, port, NULL, NULL, err)) == NULL)
    {
      SurfaceForm->StatusBar->Panels->Items[1]->Text = err;
      return;
    }
  }
  else
  {
    if((itf = mbnUDPOpen(url, port, NULL, err)) == NULL)
    {
      SurfaceForm->StatusBar->Panels->Items[1]->Text = err;
      return;
    }
  }

  thisnode.MambaNetAddr = 0;
  thisnode.Services = 0;
  sprintf(thisnode.Description, "Axum-SuperModule Software node");
  strcpy(thisnode.Name, node_info->name);
  thisnode.ManufacturerID = 1;
  thisnode.ProductID = 1004;
  thisnode.UniqueIDPerProduct = node_info->id;
  thisnode.HardwareMajorRevision = 0;
  thisnode.HardwareMinorRevision = 0;
  thisnode.FirmwareMajorRevision = 1;
  thisnode.FirmwareMinorRevision = 0;
  thisnode.FPGAFirmwareMajorRevision = 0;
  thisnode.FPGAFirmwareMinorRevision = 0;
  thisnode.NumberOfObjects = 0;
  thisnode.DefaultEngineAddr = 0;
  thisnode.HardwareParent[0] = node_info->parent.man_id;
  thisnode.HardwareParent[1] = node_info->parent.prod_id;
  thisnode.HardwareParent[2] = node_info->parent.id;
  thisnode.ServiceRequest = 0;

  sprintf(obj_desc, "Module select");
  objects[cntObject++] = MBN_OBJ(obj_desc,
                                 MBN_DATATYPE_SINT, 1, 1, -128, 127, 0,
                                 MBN_DATATYPE_OCTETS, 8, 0, 127, 0, "");

  sprintf(obj_desc, "Source select");
  objects[cntObject++] = MBN_OBJ(obj_desc,
                                 MBN_DATATYPE_SINT, 1, 1, -128, 127, 0,
                                 MBN_DATATYPE_OCTETS, 8, 0, 127, 0, "");

  sprintf(obj_desc, "Phantom");
  objects[cntObject++] = MBN_OBJ(obj_desc,
                                 MBN_DATATYPE_STATE, 1, 1, 0, 1, 0,
                                 MBN_DATATYPE_STATE, 1, 0, 1, 0, 0);

  sprintf(obj_desc, "PAD");
  objects[cntObject++] = MBN_OBJ(obj_desc,
                                 MBN_DATATYPE_STATE, 1, 1, 0, 1, 0,
                                 MBN_DATATYPE_STATE, 1, 0, 1, 0, 0);

  sprintf(obj_desc, "Source gain");
  objects[cntObject++] = MBN_OBJ(obj_desc,
                                 MBN_DATATYPE_UINT, 1, 2, 0, 1023, 0,
                                 MBN_DATATYPE_UINT, 2, 0, 1023, 0, 0);

  sprintf(obj_desc, "Source gain reset");
  objects[cntObject++] = MBN_OBJ(obj_desc,
                                 MBN_DATATYPE_STATE, 1, 1, 0, 1, 0,
                                 MBN_DATATYPE_NODATA);

  sprintf(obj_desc, "Source gain info");
  objects[cntObject++] = MBN_OBJ(obj_desc,
                                 MBN_DATATYPE_NODATA,
                                 MBN_DATATYPE_OCTETS, 8, 0, 127, 0, "");

  for (cntSwitch=0; cntSwitch<4; cntSwitch++)
  {
    sprintf(obj_desc, "Module preset %dA", cntSwitch+1);
    objects[cntObject++] = MBN_OBJ(obj_desc,
                                   MBN_DATATYPE_STATE, 1, 1, 0, 1, 0,
                                   MBN_DATATYPE_STATE, 1, 0, 1, 0, 0);

    sprintf(obj_desc, "Module preset %dB", cntSwitch+1);
    objects[cntObject++] = MBN_OBJ(obj_desc,
                                   MBN_DATATYPE_STATE, 1, 1, 0, 1, 0,
                                   MBN_DATATYPE_STATE, 1, 0, 1, 0, 0);
  }

  sprintf(obj_desc, "Phase");
  objects[cntObject++] = MBN_OBJ(obj_desc,
                                 MBN_DATATYPE_STATE, 1, 1, 0, 1, 0,
                                 MBN_DATATYPE_STATE, 1, 0, 1, 0, 0);

  sprintf(obj_desc, "Mono");
  objects[cntObject++] = MBN_OBJ(obj_desc,
                                 MBN_DATATYPE_STATE, 1, 1, 0, 1, 0,
                                 MBN_DATATYPE_STATE, 1, 0, 1, 0, 0);

  sprintf(obj_desc, "DSP gain");
  objects[cntObject++] = MBN_OBJ(obj_desc,
                                 MBN_DATATYPE_UINT, 1, 2, 0, 1023, 0,
                                 MBN_DATATYPE_UINT, 2, 0, 1023, 0, 0);

  sprintf(obj_desc, "DSP gain reset");
  objects[cntObject++] = MBN_OBJ(obj_desc,
                                 MBN_DATATYPE_STATE, 1, 1, 0, 1, 0,
                                 MBN_DATATYPE_NODATA);

  sprintf(obj_desc, "DSP gain info");
  objects[cntObject++] = MBN_OBJ(obj_desc,
                                 MBN_DATATYPE_NODATA,
                                 MBN_DATATYPE_OCTETS, 8, 0, 127, 0, "");

  sprintf(obj_desc, "Insert");
  objects[cntObject++] = MBN_OBJ(obj_desc,
                                 MBN_DATATYPE_STATE, 1, 1, 0, 1, 0,
                                 MBN_DATATYPE_STATE, 1, 0, 1, 0, 0);

  sprintf(obj_desc, "Low cut");
  objects[cntObject++] = MBN_OBJ(obj_desc,
                                 MBN_DATATYPE_STATE, 1, 1, 0, 1, 0,
                                 MBN_DATATYPE_STATE, 1, 0, 1, 0, 0);

  sprintf(obj_desc, "Low cut frequency");
  objects[cntObject++] = MBN_OBJ(obj_desc,
                                 MBN_DATATYPE_SINT, 1, 1, -128, 127, 0,
                                 MBN_DATATYPE_OCTETS, 8, 0, 127, 0, "");

  sprintf(obj_desc, "EQ");
  objects[cntObject++] = MBN_OBJ(obj_desc,
                                 MBN_DATATYPE_STATE, 1, 1, 0, 1, 0,
                                 MBN_DATATYPE_STATE, 1, 0, 1, 0, 0);

  for (cntBand=0; cntBand<6; cntBand++)
  {
    sprintf(obj_desc, "EQ%d level", cntBand+1);
    objects[cntObject++] = MBN_OBJ(obj_desc,
                                   MBN_DATATYPE_FLOAT, 1, 2, -18.0, 18.0, 0.0,
                                   MBN_DATATYPE_FLOAT, 2, -18.0, 18.0, 0.0, 0.0);

    sprintf(obj_desc, "EQ%d frequency", cntBand+1);
    objects[cntObject++] = MBN_OBJ(obj_desc,
                                   MBN_DATATYPE_UINT, 1, 2, 10, 20000, 1000,
                                   MBN_DATATYPE_UINT, 2, 10, 20000, 1000, 1000);

    sprintf(obj_desc, "EQ%d Q", cntBand+1);
    objects[cntObject++] = MBN_OBJ(obj_desc,
                                   MBN_DATATYPE_FLOAT, 1, 2, 0.1, 10.0, 1.0,
                                   MBN_DATATYPE_FLOAT, 2, 0.1, 10.0, 1.0, 1.0);

    sprintf(obj_desc, "EQ%d type", cntBand+1);
    objects[cntObject++] = MBN_OBJ(obj_desc,
                                   MBN_DATATYPE_STATE, 1, 2, 0, 7, 3,
                                   MBN_DATATYPE_STATE, 2, 0, 7, 3);

    sprintf(obj_desc, "EQ%d level reset", cntBand+1);
    objects[cntObject++] = MBN_OBJ(obj_desc,
                                   MBN_DATATYPE_STATE, 1, 1, 0, 1, 0,
                                   MBN_DATATYPE_NODATA);

    sprintf(obj_desc, "EQ%d frequency reset", cntBand+1);
    objects[cntObject++] = MBN_OBJ(obj_desc,
                                   MBN_DATATYPE_STATE, 1, 1, 0, 1, 0,
                                   MBN_DATATYPE_NODATA);

    sprintf(obj_desc, "EQ%d Q reset", cntBand+1);
    objects[cntObject++] = MBN_OBJ(obj_desc,
                                   MBN_DATATYPE_STATE, 1, 1, 0, 1, 0,
                                   MBN_DATATYPE_NODATA);
  }

  sprintf(obj_desc, "Voice processing");
  objects[cntObject++] = MBN_OBJ(obj_desc,
                                 MBN_DATATYPE_STATE, 1, 1, 0, 1, 0,
                                 MBN_DATATYPE_STATE, 1, 0, 1, 0, 0);

  sprintf(obj_desc, "D-Exp threshold");
  objects[cntObject++] = MBN_OBJ(obj_desc,
                                 MBN_DATATYPE_UINT, 1, 2, 0, 1023, 0,
                                 MBN_DATATYPE_UINT, 2, 0, 1023, 0, 0);

  sprintf(obj_desc, "D-Exp threshold reset");
  objects[cntObject++] = MBN_OBJ(obj_desc,
                                 MBN_DATATYPE_STATE, 1, 1, 0, 1, 0,
                                 MBN_DATATYPE_NODATA);

  sprintf(obj_desc, "D-Exp threshold info");
  objects[cntObject++] = MBN_OBJ(obj_desc,
                                 MBN_DATATYPE_NODATA,
                                 MBN_DATATYPE_OCTETS, 8, 0, 127, 0, "");

  sprintf(obj_desc, "AGC threshold");
  objects[cntObject++] = MBN_OBJ(obj_desc,
                                 MBN_DATATYPE_UINT, 1, 2, 0, 1023, 0,
                                 MBN_DATATYPE_UINT, 2, 0, 1023, 0, 0);

  sprintf(obj_desc, "AGC threshold reset");
  objects[cntObject++] = MBN_OBJ(obj_desc,
                                 MBN_DATATYPE_STATE, 1, 1, 0, 1, 0,
                                 MBN_DATATYPE_NODATA);

  sprintf(obj_desc, "AGC threshold info");
  objects[cntObject++] = MBN_OBJ(obj_desc,
                                 MBN_DATATYPE_NODATA,
                                 MBN_DATATYPE_OCTETS, 8, 0, 127, 0, "");

  sprintf(obj_desc, "AGC ratio");
  objects[cntObject++] = MBN_OBJ(obj_desc,
                                 MBN_DATATYPE_UINT, 1, 2, 0, 1023, 0,
                                 MBN_DATATYPE_UINT, 2, 0, 1023, 0, 0);

  sprintf(obj_desc, "AGC ratio reset");
  objects[cntObject++] = MBN_OBJ(obj_desc,
                                 MBN_DATATYPE_STATE, 1, 1, 0, 1, 0,
                                 MBN_DATATYPE_NODATA);

  sprintf(obj_desc, "AGC ratio info");
  objects[cntObject++] = MBN_OBJ(obj_desc,
                                 MBN_DATATYPE_NODATA,
                                 MBN_DATATYPE_OCTETS, 8, 0, 127, 0, "");

  sprintf(obj_desc, "Pan");
  objects[cntObject++] = MBN_OBJ(obj_desc,
                                 MBN_DATATYPE_UINT, 1, 2, 0, 1023, 512,
                                 MBN_DATATYPE_UINT, 2, 0, 1023, 512, 512);

  sprintf(obj_desc, "Pan reset");
  objects[cntObject++] = MBN_OBJ(obj_desc,
                                 MBN_DATATYPE_STATE, 1, 1, 0, 1, 0,
                                 MBN_DATATYPE_NODATA);

  sprintf(obj_desc, "On");
  objects[cntObject++] = MBN_OBJ(obj_desc,
                                 MBN_DATATYPE_STATE, 1, 1, 0, 1, 0,
                                 MBN_DATATYPE_STATE, 1, 0, 1, 0, 0);

  sprintf(obj_desc, "Fader");
  objects[cntObject++] = MBN_OBJ(obj_desc,
                                 MBN_DATATYPE_UINT, 1, 2, 0, 1023, 0,
                                 MBN_DATATYPE_UINT, 2, 0, 1023, 0, 0);

  sprintf(obj_desc, "Phase meter");
  objects[cntObject++] = MBN_OBJ(obj_desc,
                                 MBN_DATATYPE_NODATA,
                                 MBN_DATATYPE_FLOAT, 2, 0.0, 2.0, 0.0, 0.0);

  sprintf(obj_desc, "Left level meter");
  objects[cntObject++] = MBN_OBJ(obj_desc,
                                 MBN_DATATYPE_NODATA,
                                 MBN_DATATYPE_FLOAT, 2, -50.0, 10.0, 0.0, 0.0);

  sprintf(obj_desc, "Right level meter");
  objects[cntObject++] = MBN_OBJ(obj_desc,
                                 MBN_DATATYPE_NODATA,
                                 MBN_DATATYPE_FLOAT, 2, -50.0, 10.0, 0.0, 0.0);

  for (cntBuss=0; cntBuss<16; cntBuss++)
  {
    sprintf(obj_desc, "Buss %d/%d label", (cntBuss<<1)+1, (cntBuss<<1)+2);
    objects[cntObject++] = MBN_OBJ(obj_desc,
                                   MBN_DATATYPE_NODATA,
                                   MBN_DATATYPE_OCTETS, 16, 0, 127, 0, "");

    sprintf(obj_desc, "Buss %d/%d pan", (cntBuss<<1)+1, (cntBuss<<1)+2);
    objects[cntObject++] = MBN_OBJ(obj_desc,
                                   MBN_DATATYPE_UINT, 1, 2, 0, 1023, 512,
                                   MBN_DATATYPE_UINT, 2, 0, 1023, 512, 512);

    sprintf(obj_desc, "Buss %d/%d pan reset", (cntBuss<<1)+1, (cntBuss<<1)+2);
    objects[cntObject++] = MBN_OBJ(obj_desc,
                                   MBN_DATATYPE_STATE, 1, 1, 0, 1, 0,
                                   MBN_DATATYPE_NODATA);

    sprintf(obj_desc, "Buss %d/%d pre", (cntBuss<<1)+1, (cntBuss<<1)+2);
    objects[cntObject++] = MBN_OBJ(obj_desc,
                                   MBN_DATATYPE_STATE, 1, 1, 0, 1, 0,
                                   MBN_DATATYPE_STATE, 1, 0, 1, 0, 0);

    sprintf(obj_desc, "Buss %d/%d on", (cntBuss<<1)+1, (cntBuss<<1)+2);
    objects[cntObject++] = MBN_OBJ(obj_desc,
                                   MBN_DATATYPE_STATE, 1, 1, 0, 1, 0,
                                   MBN_DATATYPE_STATE, 1, 0, 1, 0, 0);

    sprintf(obj_desc, "Buss %d/%d level", (cntBuss<<1)+1, (cntBuss<<1)+2);
    objects[cntObject++] = MBN_OBJ(obj_desc,
                                   MBN_DATATYPE_UINT, 1, 2, 0, 1023, 0,
                                   MBN_DATATYPE_UINT, 2, 0, 1023, 0, 0);

    sprintf(obj_desc, "Buss %d/%d level reset", (cntBuss<<1)+1, (cntBuss<<1)+2);
    objects[cntObject++] = MBN_OBJ(obj_desc,
                                   MBN_DATATYPE_STATE, 1, 1, 0, 1, 0,
                                   MBN_DATATYPE_NODATA);

    sprintf(obj_desc, "Buss %d/%d level info", (cntBuss<<1)+1, (cntBuss<<1)+2);
    objects[cntObject++] = MBN_OBJ(obj_desc,
                                   MBN_DATATYPE_NODATA,
                                   MBN_DATATYPE_OCTETS, 8, 0, 127, 0, "");

  }

  thisnode.NumberOfObjects = cntObject;

  mbn = mbnInit(&thisnode, objects, itf, err);

  mbnSetErrorCallback(mbn, mError);
  mbnSetOnlineStatusCallback(mbn, mOnlineStatus);
  mbnSetSetActuatorDataCallback(mbn, mSetActuatorData);
}
//---------------------------------------------------------------------------

__fastcall TAxumSuperModuleForm::~TAxumSuperModuleForm()
{
  if (EQWindow)
    delete EQWindow;

  if (mbn != NULL)
    mbnFree(mbn);
}

int object_from_switch(int SwitchNr)
{
  int ObjectNr = 0;

  if ((SwitchNr>=0) && (SwitchNr<2))
  {
    ObjectNr=1026+SwitchNr;
  }
  else if ((SwitchNr>=2) && (SwitchNr<12))
  {
    ObjectNr=1031+(SwitchNr-2);
  }
  else if ((SwitchNr>=12) && (SwitchNr<14))
  {
    ObjectNr=1044+(SwitchNr-12);
  }
  else if (SwitchNr==14)
  {
    ObjectNr=1047;
  }
  else if (SwitchNr==15)
  {
    ObjectNr=1090;
  }
  else if (SwitchNr==16)
  {
    ObjectNr=1102;
  }
  else if ((SwitchNr>=17) && (SwitchNr<49))
  {
    int BussNr = (SwitchNr-17)/2;
    int FuncNr = (SwitchNr-17)%2;

    ObjectNr=1110+(8*BussNr)+FuncNr;
  }

  return ObjectNr;
}

int object_from_knob(int KnobNr)
{
  int ObjectNr = 0;
  int BussNr;

  if (KnobNr==0)
  {
    ObjectNr=1028;
  }
  else if (KnobNr==1)
  {
    ObjectNr=1041;
  }
  else if ((KnobNr>=2) && (KnobNr<5))
  {
    ObjectNr=1091+(KnobNr-2)*3;
  }
  else if ((KnobNr>=5) && (KnobNr<21))
  {
    BussNr = KnobNr-5;
    ObjectNr=1112+(8*BussNr);
  }

  return ObjectNr;
}

int object_from_pan(int PanNr)
{
  int ObjectNr = 0;
  int BussNr;

  if (PanNr==0)
  {
    ObjectNr=1100;
  }
  else if ((PanNr>=1) && (PanNr<17))
  {
    BussNr = PanNr-1;
    ObjectNr=1108+(8*BussNr);
  }

  return ObjectNr;
}

void __fastcall TAxumSuperModuleForm::SwitchMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
  int SwitchNr;
  int ObjectNr;
  mbn_data data;
  char tempText[32];

  if ((Shift.Contains(ssLeft)) && (Valid))
  {
    strcpy(tempText, ((TATImage *)Sender)->Name.c_str());
    sscanf(tempText, "Switch%dImage", &SwitchNr);
    SwitchNr--;

    ObjectNr = object_from_switch(SwitchNr);

    if (ObjectNr>=1024)
    {
      data.State = 1;
      mbnUpdateSensorData(mbn, ObjectNr, data);
    }
  }
}
//---------------------------------------------------------------------------

void __fastcall TAxumSuperModuleForm::SwitchMouseUp(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
  int SwitchNr;
  int ObjectNr;
  mbn_data data;
  char tempText[32];

  if ((Shift.Contains(ssLeft)) && (Valid))
  {
    strcpy(tempText, ((TATImage *)Sender)->Name.c_str());
    sscanf(tempText, "Switch%dImage", &SwitchNr);
    SwitchNr--;

    ObjectNr = object_from_switch(SwitchNr);

    if (ObjectNr>=1024)
    {
      data.State = 0;
      mbnUpdateSensorData(mbn, ObjectNr, data);
    }
  }
}
//---------------------------------------------------------------------------

TPicture *TAxumSuperModuleForm::GetSmallSwitchPicture(unsigned char Color)
{
  TPicture *SwitchPicture;

  switch (Color)
  {
    case 0:
    {
      SwitchPicture = SmallOffImage->Picture;
    }
    break;
    case 1:
    {
      SwitchPicture = SmallGreenImage->Picture;
    }
    break;
    case 2:
    {
      SwitchPicture = SmallRedImage->Picture;
    }
    break;
    case 3:
    {
      SwitchPicture = SmallYellowImage->Picture;
    }
    break;
  }

  return SwitchPicture;
}

TPicture *TAxumSuperModuleForm::GetLargeSwitchPicture(unsigned char Color)
{
  TPicture *SwitchPicture;

  switch (Color)
  {
    case 0:
    {
      SwitchPicture = LargeOffImage->Picture;
    }
    break;
    case 1:
    {
      SwitchPicture = LargeGreenImage->Picture;
    }
    break;
    case 2:
    {
      SwitchPicture = LargeRedImage->Picture;
    }
    break;
    case 3:
    {
      SwitchPicture = LargeYellowImage->Picture;
    }
    break;
  }

  return SwitchPicture;
}


void TAxumSuperModuleForm::MambaNetError(int code, char *msg) {
}

void TAxumSuperModuleForm::MambaNetOnlineStatus(unsigned long addr, char valid) {
  char CaptionString[128];

  MambaNetAddress = addr;
  Valid = valid;

  sprintf(CaptionString, "%s (0x%08X)", thisnode.Name, addr);
  Caption = CaptionString;

  MeterReleaseTimer->Enabled = valid;

}

int TAxumSuperModuleForm::MambaNetSetActuatorData(unsigned short object, union mbn_data data)
{
  char ObjectName[32];
  char OctetString[17];
  unsigned char DisplayNr, SwitchNr, BandNr, FuncNr, KnobNr, BussNr;

  if ((object >= 1024) && (object < 1026))
  {
    DisplayNr = (object-1024);

    sprintf(ObjectName, "Display%dLabel", DisplayNr+1);
    TLabel *DisplayLabel = (TLabel *)FindFormControl(ObjectName);
    if (DisplayLabel != NULL)
    {
      strncpy(OctetString, data.Octets, 8);
      OctetString[8]=0;
      trim(OctetString);
      DisplayLabel->Caption = OctetString;

//      MaxFontSize = MaximalFontSizeToLabelExtents(LowCutLabel, 80);
//      if (MaxFontSize<LowCutFontSize)
//      {
//      }
//      else
//      {
//        LowCutLabel->Font->Size = LowCutFontSize;
//      }
    }
  }
  else if ((object >= 1026) && (object<1028))
  {
    SwitchNr = object-1026;

    sprintf(ObjectName, "Switch%dImage", SwitchNr+1);
    TATImage *SwitchImage = (TATImage *)FindFormControl(ObjectName);
    if (SwitchImage!= NULL)
    {
      if (data.State)
      {
        SwitchImage->Picture = GetSmallSwitchPicture(2);
      }
      else
      {
        SwitchImage->Picture = GetSmallSwitchPicture(0);
      }
    }
  }
  else if (object == 1028)
  {
    Knob1->Position = data.UInt;
  }
  else if (object == 1030)
  {
    strncpy(OctetString, data.Octets, 8);
    OctetString[8]=0;
    trim(OctetString);

    Knob1Label->Caption = OctetString;
  }
  else if ((object >= 1031) && (object<1041))
  {
    SwitchNr = (object-1031)+2;

    sprintf(ObjectName, "Switch%dImage", SwitchNr+1);
    TATImage *SwitchImage = (TATImage *)FindFormControl(ObjectName);
    if (SwitchImage!= NULL)
    {
      if (data.State)
      {
        SwitchImage->Picture = GetSmallSwitchPicture(2);
      }
      else
      {
        SwitchImage->Picture = GetSmallSwitchPicture(0);
      }
    }
  }
  else if (object == 1041)
  {
    Knob2->Position = data.UInt;
  }
  else if (object == 1043)
  {
    strncpy(OctetString, data.Octets, 8);
    OctetString[8]=0;
    trim(OctetString);

    Knob2Label->Caption = OctetString;
  }
  else if ((object >= 1044) && (object<1046))
  {
    SwitchNr = (object-1044)+12;

    sprintf(ObjectName, "Switch%dImage", SwitchNr+1);
    TATImage *SwitchImage = (TATImage *)FindFormControl(ObjectName);
    if (SwitchImage!= NULL)
    {
      if (data.State)
      {
        SwitchImage->Picture = GetSmallSwitchPicture(2);
      }
      else
      {
        SwitchImage->Picture = GetSmallSwitchPicture(0);
      }
    }
  }
  else if (object == 1046)
  {
    strncpy(OctetString, data.Octets, 8);
    OctetString[8]=0;
    trim(OctetString);

    LowCutLabel->Caption = OctetString;

//    MaxFontSize = MaximalFontSizeToLabelExtents(LowCutLabel, 80);
//    if (MaxFontSize<LowCutFontSize)
//    {
//    }
//    else
//    {
//      LowCutLabel->Font->Size = LowCutFontSize;
//    }
  }
  else if (object == 1047)
  {
    SwitchNr = (object-1047)+14;

    sprintf(ObjectName, "Switch%dImage", SwitchNr+1);
    TATImage *SwitchImage = (TATImage *)FindFormControl(ObjectName);
    if (SwitchImage!= NULL)
    {
      if (data.State)
      {
        SwitchImage->Picture = GetSmallSwitchPicture(2);
      }
      else
      {
        SwitchImage->Picture = GetSmallSwitchPicture(0);
      }
    }

    if (data.State)
    {
      EQWindow->EQImage->Picture = EQWindow->EQOnImage->Picture;
    }
    else
    {
      EQWindow->EQImage->Picture = EQWindow->EQOffImage->Picture;
    }


    EQPanel->EQOn = data.State;
    EQWindow->EQWindow->EQOn = data.State;
  }
  else if ((object>=1048) && (object<1090))
  {
    BandNr = (object-1048)/7;
    FuncNr = (object-1048)%7;

    switch (FuncNr)
    {
      case 0:
      { //level
        sprintf(ObjectName, "EQ%dLevelLabel", BandNr+1);
        TLabel *LevelLabel = (TLabel *)EQWindow->FindFormControl(ObjectName);
        sprintf(ObjectName, "EQ%dLevelKnob", BandNr+1);
        TKnob *LevelKnob = (TKnob *)EQWindow->FindFormControl(ObjectName);

        if (LevelLabel != NULL)
        {
          char Value[32];
          sprintf(Value,"%1.1f dB", data.Float);
          LevelLabel->Caption = Value;
        }

        if (LevelKnob != NULL)
        {
          int Position = ((data.Float+18)*1023)/36;
          LevelKnob->Position = Position;
        }

        switch (BandNr)
        {
          case 0:
          {
            EQPanel->GainBand1 = data.Float;
            EQWindow->EQWindow->GainBand1 = data.Float;
          }
          break;
          case 1:
          {
            EQPanel->GainBand2 = data.Float;
            EQWindow->EQWindow->GainBand2 = data.Float;
          }
          break;
          case 2:
          {
            EQPanel->GainBand3 = data.Float;
            EQWindow->EQWindow->GainBand3 = data.Float;
          }
          break;
          case 3:
          {
            EQPanel->GainBand4 = data.Float;
            EQWindow->EQWindow->GainBand4 = data.Float;
          }
          break;
          case 4:
          {
            EQPanel->GainBand5 = data.Float;
            EQWindow->EQWindow->GainBand5 = data.Float;
          }
          break;
          case 5:
          {
            EQPanel->GainBand6 = data.Float;
            EQWindow->EQWindow->GainBand6 = data.Float;
          }
          break;
        }
      }
      break;
      case 1:
      { //frequency
        sprintf(ObjectName, "EQ%dFrequencyLabel", BandNr+1);
        TLabel *FrequencyLabel = (TLabel *)EQWindow->FindFormControl(ObjectName);
        sprintf(ObjectName, "EQ%dFrequencyKnob", BandNr+1);
        TKnob *FrequencyKnob = (TKnob *)EQWindow->FindFormControl(ObjectName);

        if (FrequencyLabel != NULL)
        {
          char Value[32];
          sprintf(Value,"%d Hz", data.UInt);
          FrequencyLabel->Caption = Value;
        }

        if (FrequencyKnob != NULL)
        {
          int Position = ((data.UInt-10)*1023)/19990;
          FrequencyKnob->Position = Position;
        }

        switch (BandNr)
        {
          case 0:
          {
            EQPanel->FrequencyBand1 = data.UInt;
            EQWindow->EQWindow->FrequencyBand1 = data.UInt;
          }
          break;
          case 1:
          {
            EQPanel->FrequencyBand2 = data.UInt;
            EQWindow->EQWindow->FrequencyBand2 = data.UInt;
          }
          break;
          case 2:
          {
            EQPanel->FrequencyBand3 = data.UInt;
            EQWindow->EQWindow->FrequencyBand3 = data.UInt;
          }
          break;
          case 3:
          {
            EQPanel->FrequencyBand4 = data.UInt;
            EQWindow->EQWindow->FrequencyBand4 = data.UInt;
          }
          break;
          case 4:
          {
            EQPanel->FrequencyBand5 = data.UInt;
            EQWindow->EQWindow->FrequencyBand5 = data.UInt;
          }
          break;
          case 5:
          {
            EQPanel->FrequencyBand6 = data.UInt;
            EQWindow->EQWindow->FrequencyBand6 = data.UInt;
          }
          break;
        }
      }
      break;
      case 2:
      { //bandwidth
        sprintf(ObjectName, "EQ%dBandwidthLabel", BandNr+1);
        TLabel *BandwidthLabel = (TLabel *)EQWindow->FindFormControl(ObjectName);
        sprintf(ObjectName, "EQ%dBandwidthKnob", BandNr+1);
        TKnob *BandwidthKnob = (TKnob *)EQWindow->FindFormControl(ObjectName);

        if (BandwidthLabel != NULL)
        {
          char Value[32];
          sprintf(Value,"%1.1f Q", data.Float);
          BandwidthLabel->Caption = Value;
        }

        if (BandwidthKnob != NULL)
        {
          int Position = ((data.Float-0.1)*1023)/9.9;
          BandwidthKnob->Position = Position;
        }

        switch (BandNr)
        {
          case 0:
          {
            EQPanel->BandwidthBand1 = data.Float;
            EQWindow->EQWindow->BandwidthBand1 = data.Float;
          }
          break;
          case 1:
          {
            EQPanel->BandwidthBand2 = data.Float;
            EQWindow->EQWindow->BandwidthBand2 = data.Float;
          }
          break;
          case 2:
          {
            EQPanel->BandwidthBand3 = data.Float;
            EQWindow->EQWindow->BandwidthBand3 = data.Float;
          }
          break;
          case 3:
          {
            EQPanel->BandwidthBand4 = data.Float;
            EQWindow->EQWindow->BandwidthBand4 = data.Float;
          }
          break;
          case 4:
          {
            EQPanel->BandwidthBand5 = data.Float;
            EQWindow->EQWindow->BandwidthBand5 = data.Float;
          }
          break;
          case 5:
          {
            EQPanel->BandwidthBand6 = data.Float;
            EQWindow->EQWindow->BandwidthBand6 = data.Float;
          }
          break;
        }
      }
      break;
      case 3:
      { //type
        int BandType = PEAKINGEQ;
        bool On = true;
        char TypeName[16] = "Off";

        switch (data.State)
        {
          case 0:
          {
            On = false;
            BandType = PEAKINGEQ;
            sprintf(TypeName, "Off");
          }
          break;
          case 1:
          {
            BandType = HPF;
            sprintf(TypeName, "HPF");
          }
          break;
          case 2:
          {
            BandType = LOWSHELF;
            sprintf(TypeName, "Low shelf");
          }
          break;
          case 3:
          {
            BandType = PEAKINGEQ;
            sprintf(TypeName, "Peaking");
          }
          break;
          case 4:
          {
            BandType = HIGHSHELF;
            sprintf(TypeName, "High shelf");
          }
          break;
          case 5:
          {
            BandType = LPF;
            sprintf(TypeName, "LPF");
          }
          break;
          case 6:
          {
            BandType = BPF;
            sprintf(TypeName, "BPF");
          }
          break;
          case 7:
          {
            BandType = NOTCH;
            sprintf(TypeName, "Notch");
          }
          break;
        }

        sprintf(ObjectName, "EQ%dTypeLabel", BandNr+1);
        TLabel *TypeLabel = (TLabel *)EQWindow->FindFormControl(ObjectName);

        if (TypeLabel != NULL)
        {
          TypeLabel->Caption = TypeName;
        }

        switch (BandNr)
        {
          case 0:
          {
            EQPanel->OnBand1 = On;
            EQPanel->TypeBand1 = BandType;
            EQWindow->EQWindow->OnBand1 = On;
            EQWindow->EQWindow->TypeBand1 = BandType;
          }
          break;
          case 1:
          {
            EQPanel->OnBand2 = On;
            EQPanel->TypeBand2 = BandType;
            EQWindow->EQWindow->OnBand2 = On;
            EQWindow->EQWindow->TypeBand2 = BandType;
          }
          break;
          case 2:
          {
            EQPanel->OnBand3 = On;
            EQPanel->TypeBand3 = BandType;
            EQWindow->EQWindow->OnBand3 = On;
            EQWindow->EQWindow->TypeBand3 = BandType;
          }
          break;
          case 3:
          {
            EQPanel->TypeBand4 = BandType;
            EQPanel->OnBand4 = On;
            EQWindow->EQWindow->OnBand4 = On;
            EQWindow->EQWindow->TypeBand4 = BandType;
          }
          break;
          case 4:
          {
            EQPanel->TypeBand5 = BandType;
            EQPanel->OnBand5 = On;
            EQWindow->EQWindow->OnBand5 = On;
            EQWindow->EQWindow->TypeBand5 = BandType;
          }
          break;
          case 5:
          {
            EQPanel->TypeBand6 = BandType;
            EQPanel->OnBand6 = On;
            EQWindow->EQWindow->OnBand6 = On;
            EQWindow->EQWindow->TypeBand6 = BandType;
          }
          break;
        }
      }
      break;
    }
  }
  else if (object == 1090)
  {
    SwitchNr = (object-1090)+15;

    sprintf(ObjectName, "Switch%dImage", SwitchNr+1);
    TATImage *SwitchImage = (TATImage *)FindFormControl(ObjectName);
    if (SwitchImage!= NULL)
    {
      if (data.State)
      {
        SwitchImage->Picture = GetSmallSwitchPicture(2);
      }
      else
      {
        SwitchImage->Picture = GetSmallSwitchPicture(0);
      }
    }
  }
  else if ((object >= 1091) && (object<1100))
  {
    KnobNr = ((object-1091)/3)+2;
    FuncNr = ((object-1091)%3);

    switch (FuncNr)
    {
      case 0:
      {
        sprintf(ObjectName, "Knob%d", KnobNr+1);
        TKnob *Knob = (TKnob *)FindFormControl(ObjectName);
        if (Knob != NULL)
        {
          Knob->Position = data.UInt;
        }
      }
      break;
      case 2:
      {
        sprintf(ObjectName, "Knob%dLabel", KnobNr+1);
        TLabel *DisplayLabel = (TLabel *)FindFormControl(ObjectName);
        if (DisplayLabel != NULL)
        {
          strncpy(OctetString, data.Octets, 8);
          OctetString[8]=0;
          trim(OctetString);

          DisplayLabel->Caption = OctetString;
        }
      }
      break;
    }
  }
  else if (object == 1100)
  {
    PanoramaPanel1->Position = data.UInt;
  }
  else if (object == 1102)
  {
    SwitchNr = (object-1102)+16;

    sprintf(ObjectName, "Switch%dImage", SwitchNr+1);
    TATImage *SwitchImage = (TATImage *)FindFormControl(ObjectName);
    if (SwitchImage!= NULL)
    {
      if (data.State)
      {
        SwitchImage->Picture = GetLargeSwitchPicture(2);
      }
      else
      {
        SwitchImage->Picture = GetLargeSwitchPicture(0);
      }
    }
  }
  else if (object == 1103)
  {
    FaderPanel->Position = data.UInt;
  }
  else if (object == 1104)
  {
    PhaseData = data.Float;
  }
  else if (object==1105)
  {
    LeftMeterData = data.Float+20;
    if (LeftMeterData>LeftMeterPanel->dBPosition)
    {
      LeftMeterPanel->dBPosition = LeftMeterData;
    }
  }
  else if (object==1106)
  {
    RightMeterData = data.Float+20;
    if (RightMeterData>RightMeterPanel->dBPosition)
    {
      RightMeterPanel->dBPosition = RightMeterData;
    }
  }
  else if ((object>=1107) && (object<1235))
  {
    BussNr = (object-1107)/8;
    FuncNr = (object-1107)%8;

    switch (FuncNr)
    {
      case 0:
      {
        sprintf(ObjectName, "Label%d", (BussNr+1)+6);
        TLabel *DisplayLabel = (TLabel *)FindFormControl(ObjectName);
        if (DisplayLabel != NULL)
        {
          strncpy(OctetString, data.Octets, 16);
          OctetString[16]=0;
          trim(OctetString);

          DisplayLabel->Caption = OctetString;
        }
      }
      break;
      case 1:
      {
        sprintf(ObjectName, "PanoramaPanel%d", (BussNr+1)+1);
        TPanoramaPanel *PanoramaPanel = (TPanoramaPanel *)FindFormControl(ObjectName);
        if (PanoramaPanel != NULL)
        {
          PanoramaPanel->Position = data.UInt;
        }
      }
      break;
      case 3:
      {
        SwitchNr = (BussNr*2)+17;

        sprintf(ObjectName, "Switch%dImage", SwitchNr+1);
        TATImage *SwitchImage = (TATImage *)FindFormControl(ObjectName);
        if (SwitchImage!= NULL)
        {
          if (data.State)
          {
            SwitchImage->Picture = GetSmallSwitchPicture(2);
          }
          else
          {
            SwitchImage->Picture = GetSmallSwitchPicture(0);
          }
        }
      }
      break;
      case 4:
      {
        SwitchNr = (BussNr*2)+18;

        sprintf(ObjectName, "Switch%dImage", SwitchNr+1);
        TATImage *SwitchImage = (TATImage *)FindFormControl(ObjectName);
        if (SwitchImage!= NULL)
        {
          if (data.State)
          {
            SwitchImage->Picture = GetSmallSwitchPicture(2);
          }
          else
          {
            SwitchImage->Picture = GetSmallSwitchPicture(0);
          }
        }
      }
      break;
      case 5:
      {
        sprintf(ObjectName, "Knob%d", (BussNr+1)+5);
        TKnob *Knob = (TKnob *)FindFormControl(ObjectName);
        if (Knob != NULL)
        {
          Knob->Position = data.UInt;
        }
      }
      break;
      case 7:
      {
        sprintf(ObjectName, "Knob%dLabel", (BussNr+1)+5);
        TLabel *DisplayLabel = (TLabel *)FindFormControl(ObjectName);
        if (DisplayLabel != NULL)
        {
          strncpy(OctetString, data.Octets, 8);
          OctetString[8]=0;
          trim(OctetString);

          DisplayLabel->Caption = OctetString;
        }
      }
      break;
    }
  }

  return 0;
}

void TAxumSuperModuleForm::CalculateFontSizes()
{
  int cntModule;
  int cntLine;
  char ObjectName[32];
  TLabel *DisplayLabel;
  TEdit *LCEdit;
  int cntSwitch;
  int MaxFontSize;
  int cntLabel;
  int cntKnob;

  //recalculate fontsizes
  DisplayFontSize =96;
  SmallSwitchFontSize = 96;
  MiddleSwitchFontSize = 96;
  LargeSwitchFontSize = 96;
  KnobFontSize = 96;
  LabelFontSize = 96;
  LowCutFontSize = 96;

  for (cntLine=0; cntLine<2; cntLine++)
  {
    sprintf(ObjectName, "Display%dLabel", cntLine+1);
    DisplayLabel = (TLabel *)FindFormControl(ObjectName);

    if (DisplayLabel != NULL)
    {
      DisplayLabel->Canvas->Font = DisplayLabel->Font;
      MaxFontSize = MaximalFontSizeToExtents(DisplayLabel->Canvas, DisplayLabel->Caption, DisplayLabel->Width, DisplayLabel->Height, 80);
      if (MaxFontSize < DisplayFontSize)
      {
        DisplayFontSize = MaxFontSize;
      }
    }
  }
  for (cntSwitch=0; cntSwitch<49; cntSwitch++)
  {
    sprintf(ObjectName, "Switch%dLabel", cntSwitch+1);
    DisplayLabel = (TLabel *)FindFormControl(ObjectName);
    if (DisplayLabel != NULL)
    {
      DisplayLabel->Canvas->Font = DisplayLabel->Font;
      MaxFontSize = MaximalFontSizeToExtents(DisplayLabel->Canvas, DisplayLabel->Caption, DisplayLabel->Width, DisplayLabel->Height, 70);

      if (cntSwitch==16)
      { //large switches
        if (MaxFontSize < LargeSwitchFontSize)
        {
          LargeSwitchFontSize = MaxFontSize;
        }
      }
      else if (((cntSwitch>=2) && (cntSwitch<10)) ||
               ((cntSwitch>=13) && (cntSwitch<16)))
      { //small switches
        if (MaxFontSize < SmallSwitchFontSize)
        {
          SmallSwitchFontSize = MaxFontSize;
        }
      }
      else
      { //middle switches
        if (MaxFontSize < MiddleSwitchFontSize)
        {
          MiddleSwitchFontSize = MaxFontSize;
        }
      }
    }
  }
  for (cntLabel=0; cntLabel<29; cntLabel++)
  {
    sprintf(ObjectName, "Label%d", cntLabel+1);
    DisplayLabel = (TLabel *)FindFormControl(ObjectName);

    if (DisplayLabel != NULL)
    {
      DisplayLabel->Canvas->Font = DisplayLabel->Font;
      MaxFontSize = MaximalFontSizeToExtents(DisplayLabel->Canvas, DisplayLabel->Caption, DisplayLabel->Width, DisplayLabel->Height, 80);
      if (MaxFontSize < LabelFontSize)
      {
        LabelFontSize = MaxFontSize;
      }
    }
  }
  for (cntKnob=0; cntKnob<1; cntKnob++)
  {
    sprintf(ObjectName, "Knob%dLabel", cntKnob+1);
    DisplayLabel = (TLabel *)FindFormControl(ObjectName);
    if (DisplayLabel != NULL)
    {
      DisplayLabel->Canvas->Font = DisplayLabel->Font;
      MaxFontSize = MaximalFontSizeToExtents(DisplayLabel->Canvas, DisplayLabel->Caption, DisplayLabel->Width, DisplayLabel->Height, 100);
      if (MaxFontSize < KnobFontSize)
      {
        KnobFontSize = MaxFontSize;
      }
    }
  }
  sprintf(ObjectName, "LowCutLabel");
  DisplayLabel = (TLabel *)FindFormControl(ObjectName);
  if (DisplayLabel != NULL)
  {
    DisplayLabel->Canvas->Font = DisplayLabel->Font;
    MaxFontSize = MaximalFontSizeToExtents(DisplayLabel->Canvas, DisplayLabel->Caption, DisplayLabel->Width, DisplayLabel->Height, 90);
    if (MaxFontSize < LowCutFontSize)
    {
      LowCutFontSize = MaxFontSize;
    }
  }


  for (cntLine=0; cntLine<2; cntLine++)
  {
    sprintf(ObjectName, "Display%dLabel", cntLine+1);
    DisplayLabel = (TLabel *)FindFormControl(ObjectName);

    if (DisplayLabel != NULL)
    {
      DisplayLabel->Font->Size = DisplayFontSize;
    }
  }
  for (cntSwitch=0; cntSwitch<49; cntSwitch++)
  {
    sprintf(ObjectName, "Switch%dLabel", cntSwitch+1);
    DisplayLabel = (TLabel *)FindFormControl(ObjectName);
    if (DisplayLabel != NULL)
    {
      if (cntSwitch==16)
      { //large switch
        DisplayLabel->Font->Size = LargeSwitchFontSize;
      }
      else if (((cntSwitch>=2) && (cntSwitch<10)) ||
               ((cntSwitch>=13) && (cntSwitch<16)))
      { //small switches
        DisplayLabel->Font->Size = SmallSwitchFontSize;
      }
      else
      { //middle switches
        DisplayLabel->Font->Size = MiddleSwitchFontSize;
      }
    }
  }
  for (cntLabel=0; cntLabel<29; cntLabel++)
  {
    sprintf(ObjectName, "Label%d", cntLabel+1);
    DisplayLabel = (TLabel *)FindFormControl(ObjectName);
    if (DisplayLabel != NULL)
    {
      DisplayLabel->Font->Size = LabelFontSize;
    }
  }
  for (cntKnob=0; cntKnob<21; cntKnob++)
  {
    sprintf(ObjectName, "Knob%dLabel", cntKnob+1);
    DisplayLabel = (TLabel *)FindFormControl(ObjectName);
    if (DisplayLabel != NULL)
    {
      DisplayLabel->Font->Size = KnobFontSize;
    }
  }
  sprintf(ObjectName, "LowCutLabel");
  DisplayLabel = (TLabel *)FindFormControl(ObjectName);
  if (DisplayLabel != NULL)
  {
    DisplayLabel->Font->Size = LowCutFontSize;
  }
}

void __fastcall TAxumSuperModuleForm::FormResize(TObject *Sender)
{
  int cntLine;
  int cntModule;
  int cntSwitch;
  int cntChar;
  char ObjectName[32];
  TLabel *DisplayLabel;

  TMambaNetForm::FormResize(this);

  EQPanel->AxisBorderWidth = ((float)EQPanel->Height/10)+0.5;

  float Size = EQPanel->Height/20;
  if (Size<1)
  {
    Size = 1;
  }
  else if (Size>3)
  {
    Size = 3;
  }
  EQPanel->AnchorSize = Size;

  CalculateFontSizes();
}
//---------------------------------------------------------------------------

void __fastcall TAxumSuperModuleForm::SwitchLabelMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
  int SwitchNr;
  int ObjectNr;
  mbn_data data;
  char tempText[32];

  if ((Shift.Contains(ssLeft)) && (Valid))
  {
    strcpy(tempText, ((TATImage *)Sender)->Name.c_str());
    sscanf(tempText, "Switch%dLabel", &SwitchNr);
    SwitchNr--;

    ObjectNr = object_from_switch(SwitchNr);

    if (ObjectNr>=1024)
    {
      data.State = 1;
      mbnUpdateSensorData(mbn, ObjectNr, data);
    }
  }
}
//---------------------------------------------------------------------------


void __fastcall TAxumSuperModuleForm::SwitchLabelMouseUp(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
  int SwitchNr;
  int ObjectNr;
  mbn_data data;
  char tempText[32];

  if ((Shift.Contains(ssLeft)) && (Valid))
  {
    strcpy(tempText, ((TATImage *)Sender)->Name.c_str());
    sscanf(tempText, "Switch%dLabel", &SwitchNr);
    SwitchNr--;

    ObjectNr = object_from_switch(SwitchNr);

    if (ObjectNr>=1024)
    {
      data.State = 0;
      mbnUpdateSensorData(mbn, ObjectNr, data);
    }
  }
}
//---------------------------------------------------------------------------

void TAxumSuperModuleForm::ConfigurationInformation(unsigned short object, char func_type, int func_seq, int func_nr, char *Label, char *Description)
{
  char ObjectName[32];
  int MaxFontSize;
  TLabel *DisplayLabel;
  TATImage *Image;
  TKnob *Knob;
  TPanoramaPanel *PanoramaPanel;
  int SwitchNr, DisplayNr, KnobNr, FuncNr, BussNr;

  if ((object >= 1024) && (object < 1026))
  {
    DisplayNr = (object-1024);

    sprintf(ObjectName, "Display%dLabel", DisplayNr+1);
    DisplayLabel = (TLabel *)FindFormControl(ObjectName);
    if (DisplayLabel != NULL)
    {
      DisplayLabel->Hint = Description;
      DisplayLabel->ShowHint = true;
    }
  }
  else if ((object >= 1026) && (object<1028))
  {
    SwitchNr = object-1026;

    sprintf(ObjectName, "Switch%dLabel", SwitchNr+1);
    DisplayLabel = (TLabel *)FindFormControl(ObjectName);
    if (DisplayLabel!= NULL)
    {
      DisplayLabel->Caption = Label;
      DisplayLabel->Hint = Description;
      DisplayLabel->ShowHint = true;
    }
  }
  else if (object == 1028)
  {
    Knob1->Hint = Description;
    Knob1->ShowHint = true;
  }
  else if (object == 1030)
  {
    Knob1Label->Hint = Description;
    Knob1Label->ShowHint = true;
  }
  else if ((object >= 1031) && (object<1041))
  {
    SwitchNr = (object-1031)+2;

    sprintf(ObjectName, "Switch%dLabel", SwitchNr+1);
    DisplayLabel = (TLabel *)FindFormControl(ObjectName);
    if (DisplayLabel!= NULL)
    {
      DisplayLabel->Caption = Label;
      DisplayLabel->Hint = Description;
      DisplayLabel->ShowHint = true;
    }
  }
  else if (object == 1041)
  {
    Knob2->Hint = Description;
    Knob2->ShowHint = true;
  }
  else if (object == 1043)
  {
    Knob2Label->Hint = Description;
    Knob2Label->ShowHint = true;
  }
  else if ((object >= 1044) && (object<1046))
  {
    SwitchNr = (object-1044)+12;

    sprintf(ObjectName, "Switch%dLabel", SwitchNr+1);
    DisplayLabel = (TLabel *)FindFormControl(ObjectName);
    if (DisplayLabel!= NULL)
    {
      DisplayLabel->Caption = Label;
      DisplayLabel->Hint = Description;
      DisplayLabel->ShowHint = true;
    }
  }
  else if (object == 1046)
  {
    LowCutLabel->Hint = Description;
    LowCutLabel->ShowHint = true;
  }
  else if (object == 1047)
  {
    SwitchNr = (object-1047)+14;

    sprintf(ObjectName, "Switch%dLabel", SwitchNr+1);
    DisplayLabel = (TLabel *)FindFormControl(ObjectName);
    if (DisplayLabel!= NULL)
    {
      DisplayLabel->Caption = Label;
      DisplayLabel->Hint = Description;
      DisplayLabel->ShowHint = true;
    }
  }
  else if ((object>=1048) && (object<1072))
  {
  }
  else if (object == 1072)
  {
    SwitchNr = (object-1072)+15;

    sprintf(ObjectName, "Switch%dLabel", SwitchNr+1);
    DisplayLabel = (TLabel *)FindFormControl(ObjectName);
    if (DisplayLabel!= NULL)
    {
      DisplayLabel->Caption = Label;
      DisplayLabel->Hint = Description;
      DisplayLabel->ShowHint = true;
    }
  }
  else if ((object >= 1073) && (object<1082))
  {
    KnobNr = ((object-1071)/3)+2;
    FuncNr = ((object-1071)%3);

    switch (FuncNr)
    {
      case 0:
      {
        sprintf(ObjectName, "Knob%d", KnobNr+1);
        Knob = (TKnob *)FindFormControl(ObjectName);
        if (Knob != NULL)
        {
          Knob->Hint = Description;
          Knob->ShowHint = true;
        }
      }
      break;
      case 2:
      {
        sprintf(ObjectName, "Knob%dLabel", KnobNr+1);
        DisplayLabel = (TLabel *)FindFormControl(ObjectName);
        if (DisplayLabel != NULL)
        {
          DisplayLabel->Hint = Description;
          DisplayLabel->ShowHint = true;
        }
      }
      break;
    }
  }
  else if (object == 1082)
  {
    PanoramaPanel1->Hint = Description;
    PanoramaPanel1->ShowHint = true;
  }
  else if (object == 1084)
  {
    SwitchNr = (object-1084)+16;

    sprintf(ObjectName, "Switch%dLabel", SwitchNr+1);
    DisplayLabel = (TLabel *)FindFormControl(ObjectName);
    if (DisplayLabel!= NULL)
    {
      DisplayLabel->Caption = Label;
      DisplayLabel->Hint = Description;
      DisplayLabel->ShowHint = true;
    }
  }
  else if (object == 1085)
  {
    FaderPanel->Hint = Description;
    FaderPanel->ShowHint = true;
  }
  else if (object == 1086)
  {
    PhaseMeter->Hint = Description;
    PhaseMeter->ShowHint;
  }
  else if (object==1087)
  {
    LeftMeterPanel->Hint = Description;
    LeftMeterPanel->ShowHint = true;
  }
  else if (object==1088)
  {
    RightMeterPanel->Hint = Description;
    RightMeterPanel->ShowHint = true;
  }
  else if ((object>=1089) && (object<1217))
  {
    BussNr = (object-1089)/8;
    FuncNr = (object-1089)%8;

    switch (FuncNr)
    {
      case 0:
      {
        sprintf(ObjectName, "Label%d", (BussNr+1)+6);
        DisplayLabel = (TLabel *)FindFormControl(ObjectName);
        if (DisplayLabel != NULL)
        {
          DisplayLabel->Hint = Description;
          DisplayLabel->ShowHint = true;
        }
      }
      break;
      case 1:
      {
        sprintf(ObjectName, "PanoramaPanel%d", (BussNr+1)+1);
        PanoramaPanel = (TPanoramaPanel *)FindFormControl(ObjectName);
        if (PanoramaPanel != NULL)
        {
          PanoramaPanel->Hint = Description;
          PanoramaPanel->ShowHint = Description;
        }
      }
      break;
      case 3:
      {
        SwitchNr = (BussNr*2)+17;

        sprintf(ObjectName, "Switch%dLabel", SwitchNr+1);
        DisplayLabel = (TLabel *)FindFormControl(ObjectName);
        if (DisplayLabel!= NULL)
        {
          DisplayLabel->Caption = Label;
          DisplayLabel->Hint = Description;
          DisplayLabel->ShowHint = true;
        }
      }
      break;
      case 4:
      {
        SwitchNr = (BussNr*2)+18;

        sprintf(ObjectName, "Switch%dLabel", SwitchNr+1);
        DisplayLabel = (TLabel *)FindFormControl(ObjectName);
        if (DisplayLabel!= NULL)
        {
          DisplayLabel->Caption = Label;
          DisplayLabel->Hint = Description;
          DisplayLabel->ShowHint = true;
        }
      }
      break;
      case 5:
      {
        sprintf(ObjectName, "Knob%d", (BussNr+1)+5);
        Knob = (TKnob *)FindFormControl(ObjectName);
        if (Knob != NULL)
        {
          Knob->Hint = Description;
          Knob->ShowHint = true;
        }
      }
      break;
      case 7:
      {
        sprintf(ObjectName, "Knob%dLabel", (BussNr+1)+7);
        DisplayLabel = (TLabel *)FindFormControl(ObjectName);
        if (DisplayLabel != NULL)
        {
          DisplayLabel->Hint = Description;
          DisplayLabel->ShowHint = true;
        }
      }
      break;
    }
  }
  /*
  if ((object>=1040) && (object<1072))
  {
    ModuleNr=(object-1040)%4;
    SwitchNr=(object-1040)/4;
    sprintf(ObjectName, "Label%d_%d", ModuleNr+1, SwitchNr+1);
    DisplayLabel = (TLabel *)FindFormControl(ObjectName);

    if (DisplayLabel!=NULL)
    {
      DisplayLabel->Caption = Label;
      DisplayLabel->Hint = Description;
      DisplayLabel->ShowHint = true;

      MaxFontSize = MaximalFontSizeToLabelExtents(DisplayLabel, 80);

      if ((SwitchNr>=0) && (SwitchNr<4))
      { //small switch
        if (MaxFontSize<SmallFontSize)
        {
        }
        else
        {
          DisplayLabel->Font->Size = SmallFontSize;
        }
      }
      else
      { //large switch
        if (MaxFontSize<LargeFontSize)
        {
        }
        else
        {
          DisplayLabel->Font->Size = LargeFontSize;
        }
      }
    }
  }
  if ((object>=1072) && (object<1104))
  {
    ModuleNr=(object-1072)%4;
    LedNr=(object-1072)/4;
    sprintf(ObjectName, "Led%d_%dLabel", ModuleNr+1, LedNr+1);
    TLabel *DisplayLabel = (TLabel *)FindFormControl(ObjectName);
    sprintf(ObjectName, "Led%d_%dImage", ModuleNr+1, LedNr+1);
    TATImage *Image = (TATImage *)FindFormControl(ObjectName);

    if ((DisplayLabel!=NULL) && (Image!=NULL))
    {
      DisplayLabel->Caption = Label;
      DisplayLabel->Hint = Description;
      DisplayLabel->ShowHint = true;
      Image->Hint = Description;
      Image->ShowHint = true;

      MaxFontSize = MaximalFontSizeToLabelExtents(DisplayLabel, 80);

      if (MaxFontSize<LedFontSize)
      {
      }
      else
      {
        DisplayLabel->Font->Size = LedFontSize;
      }
    }
  }*/
}

void TAxumSuperModuleForm::StartCommunication()
{
  char err[MBN_ERRSIZE];

  mbnStartInterface(mbn->itf, err);
}







void __fastcall TAxumSuperModuleForm::MeterReleaseTimerTimer(
      TObject *Sender)
{
   if (LeftMeterPanel->dBPosition>-50)
   {
      if (LeftMeterData < LeftMeterPanel->dBPosition)
      {
         LeftMeterPanel->dBPosition-=0.25;
      }
   }

   if (RightMeterPanel->dBPosition>-50)
   {
      if (RightMeterData < RightMeterPanel->dBPosition)
      {
         RightMeterPanel->dBPosition-=0.25;
      }
   }

#define PHASE_STEPSIZE 0.0075
   if (PhaseData > (PhaseMeter->Position+PHASE_STEPSIZE))
     PhaseMeter->Position += PHASE_STEPSIZE;
   else if (PhaseData < (PhaseMeter->Position-PHASE_STEPSIZE))
     PhaseMeter->Position -= PHASE_STEPSIZE;
}
//---------------------------------------------------------------------------


void __fastcall TAxumSuperModuleForm::DisplayUpDownClick(TObject *Sender,
      TUDBtnType Button)
{
  int DisplayNr;
  char tempText[32];
  int ObjectNr;

  if (Valid)
  {
    strcpy(tempText, ((TUpDown *)Sender)->Name.c_str());
    sscanf(tempText, "Display%dUpDown", &DisplayNr);
    DisplayNr--;

    ObjectNr = 1024+DisplayNr;
    union mbn_data data;

    if (Button == btNext)
    {
      data.SInt = 1;
    }
    else
    {
      data.SInt = -1;
    }
    mbnUpdateSensorData(mbn, ObjectNr, data);
  }
}
//---------------------------------------------------------------------------

void __fastcall TAxumSuperModuleForm::KnobMouseMove(TObject *Sender,
      TShiftState Shift, int X, int Y)
{
  int KnobNr;
  char tempText[32];
  int ObjectNr;
  union mbn_data data;

  if ((Shift.Contains(ssLeft)) && (Valid))
  {
    strcpy(tempText, ((TKnob *)Sender)->Name.c_str());
    sscanf(tempText, "Knob%d", &KnobNr);
    KnobNr--;

    ObjectNr = object_from_knob(KnobNr);
    if (ObjectNr>=1024)
    {
      data.UInt = ((TKnob *)Sender)->GetPositionFromXY(X,Y);
      mbnUpdateSensorData(mbn, ObjectNr, data);
    }
  }
}
//---------------------------------------------------------------------------

void __fastcall TAxumSuperModuleForm::PanoramaPanelMouseMove(
      TObject *Sender, TShiftState Shift, int X, int Y)
{
  int PanNr;
  char tempText[32];
  int ObjectNr;
  union mbn_data data;

  if ((Shift.Contains(ssLeft)) && (Valid))
  {
    strcpy(tempText, ((TPanoramaPanel *)Sender)->Name.c_str());
    sscanf(tempText, "PanoramaPanel%d", &PanNr);
    PanNr--;

    ObjectNr = object_from_pan(PanNr);
    if (ObjectNr>=1024)
    {
      TPanoramaPanel *Pan;
      data.UInt = ((TPanoramaPanel *)Sender)->GetPositionFromX(X);
      mbnUpdateSensorData(mbn, ObjectNr, data);
    }
  }
}
//---------------------------------------------------------------------------

void __fastcall TAxumSuperModuleForm::EQPanelMouseMove(TObject *Sender,
      TShiftState Shift, int X, int Y)
{
   if (Shift.Contains(ssLeft))
   {
      switch (DragEQ)
      {
         case Band1:
         {
            float Gain;
            int Frequency;
            int ObjectNr;
            union mbn_data data;

            EQPanel->GetGainFreqFromXY(&Gain, &Frequency, X, Y);

            if (EQPanel->GainBand1 != Gain)
            {
              ObjectNr = 1048;
              data.Float = Gain;
              mbnUpdateSensorData(mbn, ObjectNr, data);
              EQPanel->GainBand1 = Gain;
            }
            if (EQPanel->FrequencyBand1 != Frequency)
            {
              ObjectNr = 1049;
              data.UInt = Frequency;
              mbnUpdateSensorData(mbn, ObjectNr, data);
              EQPanel->FrequencyBand1 = Frequency;
            }
         }
         break;
         case Band2:
         {
            float Gain;
            int Frequency;
            int ObjectNr;
            union mbn_data data;

            EQPanel->GetGainFreqFromXY(&Gain, &Frequency, X, Y);

            if (EQPanel->GainBand2 != Gain)
            {
              ObjectNr = 1055;
              data.Float = Gain;
              mbnUpdateSensorData(mbn, ObjectNr, data);
              EQPanel->GainBand2 = Gain;
            }
            if (EQPanel->FrequencyBand2 != Frequency)
            {
              ObjectNr = 1056;
              data.UInt = Frequency;
              mbnUpdateSensorData(mbn, ObjectNr, data);
              EQPanel->FrequencyBand2 = Frequency;
            }
         }
         break;
         case Band3:
         {
            float Gain;
            int Frequency;
            int ObjectNr;
            union mbn_data data;

            EQPanel->GetGainFreqFromXY(&Gain, &Frequency, X, Y);

            if (EQPanel->GainBand3 != Gain)
            {
              ObjectNr = 1062;
              data.Float = Gain;
              mbnUpdateSensorData(mbn, ObjectNr, data);
              EQPanel->GainBand3 = Gain;
            }
            if (EQPanel->FrequencyBand3 != Frequency)
            {
              ObjectNr = 1063;
              data.UInt = Frequency;
              mbnUpdateSensorData(mbn, ObjectNr, data);
              EQPanel->FrequencyBand3 = Frequency;
            }
         }
         break;
         case Band4:
         {
            float Gain;
            int Frequency;
            int ObjectNr;
            union mbn_data data;

            EQPanel->GetGainFreqFromXY(&Gain, &Frequency, X, Y);

            if (EQPanel->GainBand4 != Gain)
            {
              ObjectNr = 1069;
              data.Float = Gain;
              mbnUpdateSensorData(mbn, ObjectNr, data);
              EQPanel->GainBand4 = Gain;
            }
            if (EQPanel->FrequencyBand4 != Frequency)
            {
              ObjectNr = 1070;
              data.UInt = Frequency;
              mbnUpdateSensorData(mbn, ObjectNr, data);
              EQPanel->FrequencyBand4 = Frequency;
            }
         }
         break;
         case Band5:
         {
            float Gain;
            int Frequency;
            int ObjectNr;
            union mbn_data data;

            EQPanel->GetGainFreqFromXY(&Gain, &Frequency, X, Y);

            if (EQPanel->GainBand5 != Gain)
            {
              ObjectNr = 1076;
              data.Float = Gain;
              mbnUpdateSensorData(mbn, ObjectNr, data);
              EQPanel->GainBand5 = Gain;
            }
            if (EQPanel->FrequencyBand5 != Frequency)
            {
              ObjectNr = 1077;
              data.UInt = Frequency;
              mbnUpdateSensorData(mbn, ObjectNr, data);
              EQPanel->FrequencyBand5 = Frequency;
            }
         }
         break;
         case Band6:
         {
            float Gain;
            int Frequency;
            int ObjectNr;
            union mbn_data data;

            EQPanel->GetGainFreqFromXY(&Gain, &Frequency, X, Y);

            if (EQPanel->GainBand6 != Gain)
            {
              ObjectNr = 1083;
              data.Float = Gain;
              mbnUpdateSensorData(mbn, ObjectNr, data);
              EQPanel->GainBand6 = Gain;
            }
            if (EQPanel->FrequencyBand6 != Frequency)
            {
              ObjectNr = 1084;
              data.UInt = Frequency;
              mbnUpdateSensorData(mbn, ObjectNr, data);
              EQPanel->FrequencyBand6 = Frequency;
            }
         }
         break;
      }
   }
}
//---------------------------------------------------------------------------

void __fastcall TAxumSuperModuleForm::EQPanelMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
   if (Shift.Contains(ssLeft))
   {
      DragEQ = EQPanel->GetAnchorFromXY(X,Y);
   }
   else
   {
      DragEQ = NoEQAnchor;
   }
}
//---------------------------------------------------------------------------

void __fastcall TAxumSuperModuleForm::EQPanelMouseUp(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
  DragEQ = NoEQAnchor;
}
//---------------------------------------------------------------------------

void __fastcall TAxumSuperModuleForm::LowCutUpDownClick(TObject *Sender,
      TUDBtnType Button)
{
  int ObjectNr;
  union mbn_data data;

  if (Valid)
  {
    ObjectNr = 1046;

    if (Button == btNext)
    {
      data.SInt = 1;
    }
    else
    {
      data.SInt = -1;
    }
    mbnUpdateSensorData(mbn, ObjectNr, data);
  }
}
//---------------------------------------------------------------------------

void __fastcall TAxumSuperModuleForm::FaderPanelMouseMove(TObject *Sender,
      TShiftState Shift, int X, int Y)
{
  int ObjectNr;
  union mbn_data data;

  if ((Shift.Contains(ssLeft)) && (Valid))
  {
    ObjectNr = 1103;
    data.UInt = FaderPanel->GetPositionFromY(Y);
    mbnUpdateSensorData(mbn, ObjectNr, data);
  }
}
//---------------------------------------------------------------------------

void __fastcall TAxumSuperModuleForm::KnobDblClick(TObject *Sender)
{
  int KnobNr;
  char tempText[32];
  int ObjectNr;
  union mbn_data data;

  if (Valid)
  {
    strcpy(tempText, ((TKnob *)Sender)->Name.c_str());
    sscanf(tempText, "Knob%d", &KnobNr);
    KnobNr--;

    ObjectNr = object_from_knob(KnobNr);
    ObjectNr++; //is the reset object.
    if (ObjectNr>=1024)
    {
      data.State = 1;
      mbnUpdateSensorData(mbn, ObjectNr, data);
    }
  }
}
//---------------------------------------------------------------------------

void __fastcall TAxumSuperModuleForm::PanoramaPanelDblClick(
      TObject *Sender)
{
  int PanNr;
  char tempText[32];
  int ObjectNr;
  union mbn_data data;

  if (Valid)
  {
    strcpy(tempText, ((TPanoramaPanel *)Sender)->Name.c_str());
    sscanf(tempText, "PanoramaPanel%d", &PanNr);
    PanNr--;

    ObjectNr = object_from_pan(PanNr);
    ObjectNr++; //is the reset object.
    if (ObjectNr>=1024)
    {
      data.State = 1;
      mbnUpdateSensorData(mbn, ObjectNr, data);
    }
  }
}
//---------------------------------------------------------------------------

void __fastcall TAxumSuperModuleForm::EQPanelDblClick(TObject *Sender)
{
  EQWindow->Show();
}
//---------------------------------------------------------------------------


void __fastcall TAxumSuperModuleForm::Image1Click(TObject *Sender)
{
  EQWindow->Show();
}
//---------------------------------------------------------------------------


