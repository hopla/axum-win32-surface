//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "MambaNetFrm.h"
#include "SurfaceFrm.h"

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
#pragma resource "*.dfm"

TMambaNetForm *MambaNetForm;

//---------------------------------------------------------------------------
__fastcall TMambaNetForm::TMambaNetForm(TComponent* Owner)
   : TForm(Owner)
{
  mbn = NULL;
}
//---------------------------------------------------------------------------

__fastcall TMambaNetForm::~TMambaNetForm()
{
}

void TMambaNetForm::MambaNetError(int code, char *msg)
{
}

void TMambaNetForm::MambaNetOnlineStatus(unsigned long addr, char valid)
{
}

int TMambaNetForm::MambaNetSetActuatorData(unsigned short object, union mbn_data data)
{
  return 0;
}

void __fastcall TMambaNetForm::FormActivate(TObject *Sender)
{
  FormResize(this);
}
//---------------------------------------------------------------------------

void __fastcall TMambaNetForm::FormCanResize(TObject *Sender,
      int &NewWidth, int &NewHeight, bool &Resize)
{
  float DifferenceHeight = Height-ClientHeight;
  float DifferenceWidth = Width-ClientWidth;
  float OriginalHeight = BackgroundATImage->Picture->Height;
  float OriginalWidth = BackgroundATImage->Picture->Width;

  float Ratio = OriginalWidth/OriginalHeight;

//  float CalculatedHeight = (((float)NewWidth-DifferenceWidth)/Ratio)+DifferenceHeight;
  float CalculatedWidth = (((float)NewHeight-DifferenceHeight)*Ratio)+DifferenceWidth;

  NewWidth = CalculatedWidth+0.5;
}
//---------------------------------------------------------------------------

void __fastcall TMambaNetForm::FormResize(TObject *Sender)
{
  float Factor = (float)ClientHeight/BackgroundATImage->Picture->Height;
  TControl *ChildControl;

  for (int cnt=0; cnt<ControlCount; cnt++)
  {
    ChildControl = Controls[cnt];
    ChildControl->Top = (Factor*ControlOriginalTop[cnt])+0.5;
    ChildControl->Left = (Factor*ControlOriginalLeft[cnt])+0.5;
    ChildControl->Height = (Factor*ControlOriginalHeight[cnt])+0.5;
    ChildControl->Width = (Factor*ControlOriginalWidth[cnt])+0.5;
  }
}
//---------------------------------------------------------------------------

void __fastcall TMambaNetForm::FormShow(TObject *Sender)
{
  TControl *ChildControl;

  for (int cnt=0; cnt<ControlCount; cnt++)
  {
    ChildControl = Controls[cnt];
    ControlOriginalTop[cnt] = ChildControl->Top;
    ControlOriginalLeft[cnt] = ChildControl->Left;
    ControlOriginalHeight[cnt] = ChildControl->Height;
    ControlOriginalWidth[cnt] = ChildControl->Width;
  }
}
//---------------------------------------------------------------------------

void TMambaNetForm::ConfigurationInformation(unsigned short object, char func_type, int func_seq, int func_nr, char *Label, char *Description)
{
}

void TMambaNetForm::StartCommunication()
{
}

int TMambaNetForm::MaximalFontSizeToExtents(TCanvas *Canvas, AnsiString Text, int Width, int Height, int Percent)
{
  int MaximumFontSize = 96;
  int HeightSize;
  int WidthSize;
  unsigned int cntChar;
  unsigned int cntCharLine = 0;
  char LabelText[32];
  char TextLines[32][32];
  char cntLine = 0;
  char LongestLine = 0;
  AnsiString SelectedLine;
  float PercentHeight;
  float PercentWidth;

  strcpy(LabelText, Text.c_str());

  for (cntChar=0; cntChar<strlen(LabelText); cntChar++)
  {
    TextLines[cntLine][cntCharLine++] = LabelText[cntChar];
    if (LabelText[cntChar] == '\n')
    {
      if (strlen(TextLines[LongestLine]) < cntCharLine)
      {
        LongestLine = cntLine;
      }
      TextLines[cntLine][cntCharLine] = 0;
      cntLine++;
      cntCharLine = 0;
    }
  }
  TextLines[cntLine][cntCharLine] = 0;
  PercentHeight = ((float)Percent/100)/(cntLine+1);
  PercentWidth = ((float)Percent/100);

  SelectedLine = TextLines[LongestLine];

  if (SelectedLine != "")
  {
//  DisplayLabel->Canvas->Font = DisplayLabel->Font;
    Canvas->Font->Size=1;
    TSize Size = Canvas->TextExtent(SelectedLine);
    while ((Size.cy<((int)(Height*PercentHeight)+0.5)))
    {
      Canvas->Font->Size++;
      Size = Canvas->TextExtent(SelectedLine);
    }
    HeightSize=Canvas->Font->Size;

    Canvas->Font->Size=1;
    Size = Canvas->TextExtent(SelectedLine);
    while ((Size.cx<((int)(Width*PercentWidth)+0.5)))
    {
      Canvas->Font->Size++;
      Size = Canvas->TextExtent(SelectedLine);
    }
    WidthSize=Canvas->Font->Size;

    if (WidthSize<HeightSize)
      MaximumFontSize = WidthSize;
    else
      MaximumFontSize = HeightSize;
  }
  return MaximumFontSize;
}

void TMambaNetForm::CalculateFontSizes()
{
}

void TMambaNetForm::WMSetActuatorData(TMessage & msg)
{
  set_actuator_data *actuator_data = (set_actuator_data *)msg.LParam;

  MambaNetSetActuatorData(actuator_data->object, actuator_data->data);
}

void TMambaNetForm::WMConfigInfo(TMessage &msg)
{
  config_info *info = (config_info *)msg.LParam;
  ConfigurationInformation(info->obj,
                           info->func_type,
                           info->func_seq,
                           info->func_func,
                           info->Label,
                           info->Desc);
}

void TMambaNetForm::WMResizeFonts(TMessage &msg)
{
  CalculateFontSizes();
}

void TMambaNetForm::WMMambaNetError(TMessage &msg)
{
  MambaNetError(msg.WParam, (char *)msg.LParam);
}

void TMambaNetForm::WMOnlineStatus(TMessage &msg)
{
  MambaNetOnlineStatus(msg.WParam, msg.LParam);
}

TControl *TMambaNetForm::FindFormControl(char *Name)
{
  TControl *FoundControl = NULL;

  for (int cnt=0; cnt<ControlCount; cnt++)
  {
    if (strcmp(Name, Controls[cnt]->Name.c_str()) == 0)
    {
      FoundControl = Controls[cnt];
    }
  }
  return FoundControl;
}

void TMambaNetForm::PrintLabels(TCanvas *Canvas, float *xMm, float *yMm, float xPixelPerMm, float yPixelPerMm, float PageWidth, float PageHeight)
{
}

bool TMambaNetForm::PrintLabelsAvailable()
{
  return false;
}





