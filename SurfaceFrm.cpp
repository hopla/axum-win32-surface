//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "SurfaceFrm.h"
#include "MambaNetConnectionFrm.h"
#include "SurfaceSelectFrm.h"
#define MBN_VARARG
#include "mbn.h"
#include "Axum4FBPFrm.h"
#include "AxumCRMFrm.h"
#include "AxumMeterFrm.h"

#include <stdio.h>
#include <dos.h>

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"

TSurfaceForm *SurfaceForm;

//---------------------------------------------------------------------------
__fastcall TSurfaceForm::TSurfaceForm(TComponent* Owner)
   : TForm(Owner)
{
  int cnt;

  for (cnt=0; cnt<16; cnt++) {
    SurfaceNodes[cnt].MambaNetForm = NULL;
    SurfaceNodes[cnt].ConfigurationCopied = NULL;
    SurfaceNodes[cnt].FromAddr = 0;
  }

  StayOnTop = false;
}
//---------------------------------------------------------------------------

void __fastcall TSurfaceForm::FormClose(TObject *Sender,
      TCloseAction &Action)
{
  node_info *tempNodeInfo;
  surface_info *tempSurfaceInfo;
  TMambaNetForm *tempMambaNetForm;
  int cnt=0;

  for (cnt=0; cnt<16; cnt++) {
    if (SurfaceNodes[cnt].MambaNetForm != NULL)
    {
      tempMambaNetForm = SurfaceNodes[cnt].MambaNetForm;
      SurfaceNodes[cnt].MambaNetForm = NULL;
      delete tempMambaNetForm;
    }
  }

  while (surfaces != NULL) {
    while (surfaces->nodes != NULL) {
      tempNodeInfo = surfaces->nodes;
      surfaces->nodes = surfaces->nodes->next;
      delete tempNodeInfo;
    }
    tempSurfaceInfo = surfaces;
    surfaces = surfaces->next;
    delete tempSurfaceInfo;
  }

  Action = caFree;
}
//---------------------------------------------------------------------------

bool __fastcall TSurfaceForm::CopyConfiguration(unsigned short ToManID, unsigned short ToProductID, unsigned short ToID, unsigned int FromAddr, unsigned char FirmwareMajor)
{
  bool Copied = false;
  char Query[1024];
  unsigned int MambaNetAddress;
  PGresult *res;
  char tempText[32];

  sprintf(Query, "SELECT addr FROM addresses WHERE (id).man=%d AND (id).prod=%d AND (id).id=%d", ToManID, ToProductID, ToID);
  res = PQexecParams(sql_conn, Query, 0, NULL, NULL, NULL, NULL, 0);
  if ((res != NULL) && (PQntuples(res) == 1))
  { //Copy configuration.
    strcpy(tempText, PQgetvalue(res, 0, 0));
    sscanf(tempText, "%d", &MambaNetAddress);

    sprintf(Query, "DELETE FROM node_config WHERE addr = %d AND firm_major", MambaNetAddress, FirmwareMajor);
    res = PQexecParams(sql_conn, Query, 0, NULL, NULL, NULL, NULL, 0);
    sprintf(Query, "DELETE FROM defaults WHERE addr = %d AND firm_major", MambaNetAddress, FirmwareMajor);
    res = PQexecParams(sql_conn, Query, 0, NULL, NULL, NULL, NULL, 0);

    sprintf(Query, "INSERT INTO node_config (addr, object, func, firm_major) SELECT %d, object, func, firm_major FROM node_config WHERE addr = %d AND firm_major=%d", MambaNetAddress, FromAddr, FirmwareMajor);
    res = PQexecParams(sql_conn, Query, 0, NULL, NULL, NULL, NULL, 0);
    sprintf(Query, "INSERT INTO defaults (addr, object, data, firm_major) SELECT %d, object, data, firm_major FROM defaults WHERE addr = %d AND firm_major=%d", MambaNetAddress, FromAddr, FirmwareMajor);
    res = PQexecParams(sql_conn, Query, 0, NULL, NULL, NULL, NULL, 0);
    Copied = true;
  }
  PQclear(res);
  
  return Copied;
}


surface_node *GetSurfaceNode(struct mbn_handler *mbn)
{
  int cnt;
  surface_node *SurfaceNode = NULL;

  for (cnt=0; cnt<16; cnt++)
  {
    TMambaNetForm *Form = (TMambaNetForm *)SurfaceForm->SurfaceNodes[cnt].MambaNetForm;
    if (Form != NULL)
    {
      if (Form->mbn == mbn)
      {
        SurfaceNode = &SurfaceForm->SurfaceNodes[cnt];
      }
    }
  }
  return SurfaceNode;
}


void mError(struct mbn_handler *mbn, int code, char *msg)
{
}

void mOnlineStatus(struct mbn_handler *mbn, unsigned long addr, char valid)
{
  surface_node *SurfaceNode = GetSurfaceNode(mbn);
  if ((SurfaceNode != NULL) && (SurfaceNode->MambaNetForm != NULL))
  {
    SurfaceNode->MambaNetForm->MambaNetOnlineStatus(addr, valid);

    if (!SurfaceNode->ConfigurationCopied)
    {
      SurfaceNode->ConfigurationCopied = SurfaceForm->CopyConfiguration(SurfaceNode->MambaNetForm->thisnode.ManufacturerID,
                                                                         SurfaceNode->MambaNetForm->thisnode.ProductID,
                                                                         SurfaceNode->MambaNetForm->thisnode.UniqueIDPerProduct,
                                                                         SurfaceNode->FromAddr,
                                                                         SurfaceNode->MambaNetForm->thisnode.FirmwareMajorRevision);

    }
  }
}

int mSetActuatorData(struct mbn_handler *mbn, unsigned short object, union mbn_data data)
{
  surface_node *SurfaceNode = GetSurfaceNode(mbn);
  if ((SurfaceNode != NULL) && (SurfaceNode->MambaNetForm != NULL))
  {
    SurfaceNode->MambaNetForm->MambaNetSetActuatorData(object, data);
  }

  return 0;
}




void __fastcall TSurfaceForm::ConnecttoAXUM1Click(TObject *Sender)
{
  char err[MBN_ERRSIZE];
  struct mbn_interface *itf;
  char dbstr[256];
  PGresult *res;
  char Query[2048];
  char Parent[32] = "";
  char SurfaceText[16][1024];
  int cnt, cntRow, cntSurfaceNode;
  surface_info *WalkSurfaceInfo;
  node_info *WalkNodeInfo;
  char tempText[32];

  MambaNetConnectionForm = new TMambaNetConnectionForm(Application);

  if (MambaNetConnectionForm->ShowModal() !=  mrOk)
    return;

  if (MambaNetConnectionForm->URLEdit->Text == "")
  {
    ShowMessage("No URL given");
  }
  strcpy(url, MambaNetConnectionForm->URLEdit->Text.c_str());
  delete MambaNetConnectionForm;

  sprintf(dbstr, "hostaddr='%s' dbname='axum' user='axum'", url);
  sql_conn = PQconnectdb(dbstr);
  if (PQstatus(sql_conn) != CONNECTION_OK) {
    PQfinish(sql_conn);
    return;
  }
  sprintf(Query, "SELECT a.addr, a.name, a.active, a.parent, (a.id).man, (a.id).prod, (a.id).id, a.firm_major,                                                                                                                        \
    (SELECT COUNT(*) FROM templates t WHERE t.man_id = (a.id).man AND t.prod_id = (a.id).prod AND t.firm_major = a.firm_major) AS objects,                                                                                  \
    (SELECT number FROM templates t WHERE t.man_id = (a.id).man AND t.prod_id = (a.id).prod AND t.firm_major = a.firm_major AND t.description = 'Slot number') AS slot_obj,                                               \
    (SELECT name FROM addresses b WHERE (b.id).man = (a.parent).man AND (b.id).prod = (a.parent).prod AND (b.id).id = (a.parent).id) AS parent_name,                                                                        \
    (SELECT COUNT(*) FROM node_config n WHERE a.addr = n.addr AND a.firm_major = n.firm_major) AS config_cnt,                                                                                                               \
    (SELECT COUNT(*) FROM defaults d WHERE a.addr = d.addr AND a.firm_major = d.firm_major) AS default_cnt,                                                                                                                 \
    (SELECT COUNT(*) FROM predefined_node_config p WHERE (a.id).man = p.man_id AND (a.id).prod = p.prod_id AND a.firm_major = p.firm_major) AS predefined_cfg_cnt,                                                          \
    (SELECT COUNT(*) FROM predefined_node_defaults d WHERE (a.id).man = d.man_id AND (a.id).prod = d.prod_id AND a.firm_major = d.firm_major) AS predefined_dflt_cnt                                                        \
    FROM slot_config s                                                                                                                                                                                                      \
    RIGHT JOIN addresses a ON a.addr = s.addr WHERE s.addr IS NULL AND ((a.parent).man != 1 OR (a.parent).prod != 12) AND NOT ((a.id).man=(a.parent).man AND (a.id).prod=(a.parent).prod AND (a.id).id=(a.parent).id) AND (a.parent).man = 1 AND (a.parent).prod = 25 \
    ORDER BY NULLIF((a.parent).man, 0), (a.parent).prod, (a.parent).id, NOT a.active, (a.id).man, (a.id).prod, (a.id).id");

  res = PQexecParams(sql_conn, Query, 0, NULL, NULL, NULL, NULL, 0);
  if ((res == NULL) || (PQntuples(res) == 0)) {
    ShowMessage("DB query no result");
    //exit
  }

  surfaces = WalkSurfaceInfo = new surface_info;
  memset(WalkSurfaceInfo, 0, sizeof(surface_info));
  strcpy(WalkSurfaceInfo->name, PQgetvalue(res, 0, 10));

  strcpy(Parent, PQgetvalue(res, 0, 3));

  for (cntRow=0; cntRow<PQntuples(res); cntRow++)
  {
    if (strcmp(Parent, PQgetvalue(res, cntRow, 3)) != 0)
    {
      WalkSurfaceInfo->next = new surface_info;
      memset(WalkSurfaceInfo->next, 0, sizeof(surface_info));
      strcpy(WalkSurfaceInfo->next->name, PQgetvalue(res, 0, 10));
      WalkSurfaceInfo = WalkSurfaceInfo->next;
      strcpy(Parent, PQgetvalue(res, cntRow, 3));
    }

    if (WalkSurfaceInfo->nodes == NULL)
    {
      WalkSurfaceInfo->nodes = WalkNodeInfo = new node_info;
    }
    else
    {
      WalkNodeInfo->next = new node_info;
      WalkNodeInfo = WalkNodeInfo->next;
    }
    memset(WalkNodeInfo, 0, sizeof(node_info));


    strcpy(tempText, PQgetvalue(res, cntRow, 0));
    sscanf(tempText, "%d", &WalkNodeInfo->addr);
    strcpy(WalkNodeInfo->name, PQgetvalue(res, cntRow, 1));
    strcpy(tempText, PQgetvalue(res, cntRow, 4));
    sscanf(tempText, "%d", &WalkNodeInfo->man_id);
    strcpy(tempText, PQgetvalue(res, cntRow, 5));
    sscanf(tempText, "%d", &WalkNodeInfo->prod_id);
    strcpy(tempText, PQgetvalue(res, cntRow, 6));
    sscanf(tempText, "%d", &WalkNodeInfo->id);
    strcpy(tempText, PQgetvalue(res, cntRow, 7));
    sscanf(tempText, "%d", &WalkNodeInfo->firm_major);
  }

  SurfaceSelectForm = new TSurfaceSelectForm(Application);
  WalkSurfaceInfo = surfaces;
  while (WalkSurfaceInfo != NULL)
  {
    TTreeNode *SurfaceNode = SurfaceSelectForm->TreeView1->Items->AddObject(NULL, WalkSurfaceInfo->name, WalkSurfaceInfo);
    WalkNodeInfo = WalkSurfaceInfo->nodes;
    while (WalkNodeInfo != NULL)
    {
      SurfaceSelectForm->TreeView1->Items->AddChildObject(SurfaceNode, WalkNodeInfo->name, WalkNodeInfo);
      WalkNodeInfo = WalkNodeInfo->next;
    }
    WalkSurfaceInfo = WalkSurfaceInfo->next;
  }

  if (SurfaceSelectForm->ShowModal() ==  mrOk)
  {
    cntSurfaceNode = 0;
    if (SurfaceSelectForm->TreeView1->Selected->Parent == NULL)
    { //Surface node
      if((itf = mbnUDPOpen(url, "34848", NULL, err)) == NULL)
      {
        ShowMessage(err);
        return;
      }

      thisnode.MambaNetAddr = 0;
      thisnode.Services = 0;
      sprintf(thisnode.Description, "AXUM Surface software");
      strcpy(thisnode.Name, "Surface software");
      thisnode.ManufacturerID = 1;
      thisnode.ProductID = 1001;
      thisnode.UniqueIDPerProduct = 1;
      thisnode.HardwareMajorRevision = 0;
      thisnode.HardwareMinorRevision = 0;
      thisnode.FirmwareMajorRevision = 0;
      thisnode.FirmwareMinorRevision = 1;
      thisnode.FPGAFirmwareMajorRevision = 0;
      thisnode.FPGAFirmwareMinorRevision = 0;
      thisnode.NumberOfObjects = 0;
      thisnode.DefaultEngineAddr = 0;
      thisnode.HardwareParent[0] = thisnode.ManufacturerID;
      thisnode.HardwareParent[1] = thisnode.ProductID;
      thisnode.HardwareParent[2] = thisnode.UniqueIDPerProduct;
      thisnode.ServiceRequest = 0;

      mbn = mbnInit(&thisnode, NULL, itf, err);

      mbnStartInterface(itf, err);

      WalkSurfaceInfo = (surface_info *)SurfaceSelectForm->TreeView1->Selected->Data;
      WalkNodeInfo = WalkSurfaceInfo->nodes;
      while (WalkNodeInfo != NULL)
      {
        if (WalkNodeInfo->man_id == 1)
        {
          switch (WalkNodeInfo->prod_id)
          {
            case 7:
            {
              unsigned int MambaNetAddress;
              form_node_info node_info;
              strcpy(node_info.name, WalkNodeInfo->name);
              node_info.id = WalkNodeInfo->id;
              node_info.parent.man_id = 1;
              node_info.parent.prod_id = 1001;
              node_info.parent.id = 1;

              SurfaceNodes[cntSurfaceNode].FromAddr = WalkNodeInfo->addr;
              SurfaceNodes[cntSurfaceNode].ConfigurationCopied = CopyConfiguration(1, 1000, WalkNodeInfo->id, WalkNodeInfo->addr, WalkNodeInfo->firm_major);
              SurfaceNodes[cntSurfaceNode].MambaNetForm = new TAxum4FBPForm(this, url, &node_info);
            }
            break;
            case 8:
            {
              unsigned int MambaNetAddress;
              form_node_info node_info;
              strcpy(node_info.name, WalkNodeInfo->name);
              node_info.id = WalkNodeInfo->id;
              node_info.parent.man_id = 1;
              node_info.parent.prod_id = 1001;
              node_info.parent.id = 1;

              SurfaceNodes[cntSurfaceNode].FromAddr = WalkNodeInfo->addr;
              SurfaceNodes[cntSurfaceNode].ConfigurationCopied = CopyConfiguration(1, 1002, WalkNodeInfo->id, WalkNodeInfo->addr, WalkNodeInfo->firm_major);
              SurfaceNodes[cntSurfaceNode].MambaNetForm = new TAxumCRMForm(this, url, &node_info);
            }
            break;
            case 26:
            {
              unsigned int MambaNetAddress;
              form_node_info node_info;
              strcpy(node_info.name, WalkNodeInfo->name);
              node_info.id = WalkNodeInfo->id;
              node_info.parent.man_id = 1;
              node_info.parent.prod_id = 1001;
              node_info.parent.id = 1;

              SurfaceNodes[cntSurfaceNode].FromAddr = WalkNodeInfo->addr;
              SurfaceNodes[cntSurfaceNode].ConfigurationCopied = CopyConfiguration(1, 1003, WalkNodeInfo->id, WalkNodeInfo->addr, WalkNodeInfo->firm_major);
              SurfaceNodes[cntSurfaceNode].MambaNetForm = new TAxumMeterForm(this, url, &node_info);
            }
            break;
          }
          if (SurfaceNodes[cntSurfaceNode].MambaNetForm != NULL)
          {
            SurfaceNodes[cntSurfaceNode].MambaNetForm->Show();
            cntSurfaceNode++;
          }
        }
        WalkNodeInfo = WalkNodeInfo->next;
      }
    }
    else
    {
      WalkNodeInfo = (node_info *)SurfaceSelectForm->TreeView1->Selected->Data;
      if (WalkNodeInfo != NULL)
      {
        if (WalkNodeInfo->man_id == 1)
        {
          switch (WalkNodeInfo->prod_id)
          {
            case 7:
            {
              unsigned int MambaNetAddress;
              form_node_info node_info;
              strcpy(node_info.name, WalkNodeInfo->name);
              node_info.id = WalkNodeInfo->id;
              node_info.parent.man_id = 1;
              node_info.parent.prod_id = 1001;
              node_info.parent.id = 1;

              SurfaceNodes[cntSurfaceNode].FromAddr = WalkNodeInfo->addr;
              SurfaceNodes[cntSurfaceNode].ConfigurationCopied = CopyConfiguration(1, 1000, WalkNodeInfo->id, WalkNodeInfo->addr, WalkNodeInfo->firm_major);
              SurfaceNodes[cntSurfaceNode].MambaNetForm = new TAxum4FBPForm(this, url, &node_info);
            }
            break;
            case 8:
            {
              unsigned int MambaNetAddress;
              form_node_info node_info;
              strcpy(node_info.name, WalkNodeInfo->name);
              node_info.id = WalkNodeInfo->id;
              node_info.parent.man_id = 1;
              node_info.parent.prod_id = 1001;
              node_info.parent.id = 1;

              SurfaceNodes[cntSurfaceNode].FromAddr = WalkNodeInfo->addr;
              SurfaceNodes[cntSurfaceNode].ConfigurationCopied = CopyConfiguration(1, 1002, WalkNodeInfo->id, WalkNodeInfo->addr, WalkNodeInfo->firm_major);
              SurfaceNodes[cntSurfaceNode].MambaNetForm = new TAxumCRMForm(this, url, &node_info);
            }
            break;
            case 26:
            {
              unsigned int MambaNetAddress;
              form_node_info node_info;
              strcpy(node_info.name, WalkNodeInfo->name);
              node_info.id = WalkNodeInfo->id;
              node_info.parent.man_id = 1;
              node_info.parent.prod_id = 1001;
              node_info.parent.id = 1;

              SurfaceNodes[cntSurfaceNode].FromAddr = WalkNodeInfo->addr;
              SurfaceNodes[cntSurfaceNode].ConfigurationCopied = CopyConfiguration(1, 1003, WalkNodeInfo->id, WalkNodeInfo->addr, WalkNodeInfo->firm_major);
              SurfaceNodes[cntSurfaceNode].MambaNetForm = new TAxumMeterForm(this, url, &node_info);
            }
            break;
          }
          if (SurfaceNodes[cntSurfaceNode].MambaNetForm != NULL)
          {
            SurfaceNodes[cntSurfaceNode].MambaNetForm->Show();
            cntSurfaceNode++;
          }
        }
      }
    }
  }
  ReorderSurfaceNodes();

  for (cnt=0; cnt<16; cnt++)
  {
    if (SurfaceNodes[cnt].MambaNetForm != NULL)
    {
      SurfaceNodes[cnt].MambaNetForm->StartCommunication();
    }
  }
}
//---------------------------------------------------------------------------

void __fastcall TSurfaceForm::ReorderSurfaceNodes()
{
  int cnt;
  int XPos = 0;

  for (cnt=0; cnt<16; cnt++)
  {
    if (SurfaceNodes[cnt].MambaNetForm != NULL)
    {
      SurfaceNodes[cnt].MambaNetForm->Top = 0;
      SurfaceNodes[cnt].MambaNetForm->Height = ClientHeight-4;
      SurfaceNodes[cnt].MambaNetForm->Left = XPos;
      XPos+=SurfaceNodes[cnt].MambaNetForm->Width;
    }
  }
}
//---------------------------------------------------------------------------




void __fastcall TSurfaceForm::Reorder1Click(TObject *Sender)
{
  ReorderSurfaceNodes();
}
//---------------------------------------------------------------------------

void __fastcall TSurfaceForm::AlwaysOnTopMenuItemClick(TObject *Sender)
{
  StayOnTop = !StayOnTop;
  if (StayOnTop)
  {
    SetWindowPos(Handle, HWND_TOPMOST, 0,0,0,0, SWP_NOACTIVATE+SWP_NOMOVE+SWP_NOSIZE);
  }
  else
  {
    SetWindowPos(Handle, HWND_NOTOPMOST, 0,0,0,0, SWP_NOACTIVATE+SWP_NOMOVE+SWP_NOSIZE);
  }
  AlwaysOnTopMenuItem->Checked = StayOnTop;
}
//---------------------------------------------------------------------------


