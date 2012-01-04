/****************************************************************************
 * AdvCmpDlgOpt.cpp
 *
 * Plugin module for Far Manager 3.0
 *
 * Copyright (c) 2006-2011 Alexey Samlyukov
 ****************************************************************************/
/*
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the authors may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#pragma hdrstop
#include "AdvCmpDlgOpt.hpp"

/****************************************************************************
 **************************** ShowOptDialog functions ***********************
 ****************************************************************************/

/****************************************************************************
 * ID-константы диалога
 ****************************************************************************/
enum {
	DlgBORDER = 0,  // 0
	DlgMODE,        // 1
	DlgCMP,         // 2
	DlgSYNC,        // 3
	DlgCMPCASE,     // 4
	DlgCMPSIZE,     // 5
	DlgCMPTIME,     // 6
	DlgSECONDS,     // 7
	DlgIGNORESEC,   // 8
	DlgPRECISION,   // 9
	DlgTIMEZONE,    //10
	DlgCMPCONTENTS, //11
	DlgDIFFTIME,    //12
	DlgCACHE,       //13
	DlgCACHEIGNORE, //14
	DlgCACHEUSE,    //15
	DlgCACHECLEAR,  //16
	DlgPARTLY,      //17
	DlgPARTLYFULL,  //18
	DlgLPARTLYKB,   //19
	DlgEPARTLYKB,   //20
	DlgIGNORE,      //21
	DlgIGNORETEMPL, //22

	DlgSEP1,        //23
	DlgSUBFOLDER,   //24
	DlgLMAXDEPTH,   //25
	DlgEMAXDEPTH,   //26
	DlgSCANSYMLINK, //27
	DlgFILTER,      //28
	DlgFILTERBOTTON,//29
	DlgFIRSTDIFF,   //30
	DlgLCMPSKIP,    //31
	DlgECMPSKIP,    //32
	DlgSELECTED,    //33
	DlgSELECTEDNEW, //34
	DlgONLYRIGHT,   //35
	DlgIGNORMISSING,//36
	DlgSHOWMSG,     //37
	DlgSOUND,       //38
	DlgDIALOG,      //39
	DlgTOTALPROCESS,//40

	DlgSEP2,        //41
	DlgOK,          //42
	DlgUNDERCURSOR, //43
	DlgCANCEL,      //44
};

struct ParamStore
{
	int ID;
	wchar_t *RegName;
	int *Option;
} StoreOpt[] = {
	{DlgBORDER,      0,                      0},
	{DlgMODE,        0,                      0},

	{DlgCMP,         0,                      0},
	{DlgSYNC,        L"Synchronize",         &Opt.Sync},
	{DlgCMPCASE,     L"CmpCase",             &Opt.CmpCase},
	{DlgCMPSIZE,     L"CmpSize",             &Opt.CmpSize},
	{DlgCMPTIME,     L"CmpTime",             &Opt.CmpTime},
	{DlgSECONDS,     L"Seconds",             &Opt.Seconds},
	{DlgIGNORESEC,   0,                      0},
	{DlgPRECISION,   L"LowPrecisionTime",    &Opt.LowPrecisionTime},
	{DlgTIMEZONE,    L"IgnoreTimeZone",      &Opt.IgnoreTimeZone},
	{DlgCMPCONTENTS, L"CmpContents",         &Opt.CmpContents},
	{DlgDIFFTIME,    L"OnlyTimeDiff",        &Opt.OnlyTimeDiff},
	{DlgCACHE,       L"Cache",               &Opt.Cache},
	{DlgCACHEIGNORE, L"CacheIgnore",         &Opt.CacheIgnore},
	{DlgCACHEUSE,    0,                      0},
	{DlgCACHECLEAR,  0,                      0},
	{DlgPARTLY,      L"Partly",              &Opt.Partly},
	{DlgPARTLYFULL,  L"PartlyFull",          &Opt.PartlyFull},
	{DlgLPARTLYKB,   0,                      0},
	{DlgEPARTLYKB,   L"PartlyKbSize",        &Opt.PartlyKbSize},
	{DlgIGNORE,      L"Ignore",              &Opt.Ignore},
	{DlgIGNORETEMPL, L"IgnoreTemplates",     &Opt.IgnoreTemplates},

	{DlgSEP1,        0,                      0},
	{DlgSUBFOLDER,   L"ProcessSubfolders",   &Opt.ProcessSubfolders},
	{DlgLMAXDEPTH,   0,                      0},
	{DlgEMAXDEPTH,   L"MaxScanDepth",        &Opt.MaxScanDepth},
	{DlgSCANSYMLINK, 0,                      0},
	{DlgFILTER,      L"Filter",              &Opt.Filter},
	{DlgFILTERBOTTON,0,                      0},
	{DlgFIRSTDIFF,   L"ProcessTillFirstDiff",&Opt.ProcessTillFirstDiff},
	{DlgLCMPSKIP,    L"SkipSubstr",          &Opt.SkipSubstr},
	{DlgECMPSKIP,    0,                      0},
	{DlgSELECTED,    L"ProcessSelected",     &Opt.ProcessSelected},
	{DlgSELECTEDNEW, L"SelectedNew",         &Opt.SelectedNew},
	{DlgONLYRIGHT,   L"SyncOnlyRight",       &Opt.SyncOnlyRight},
	{DlgIGNORMISSING,L"IgnoreMissing",       &Opt.IgnoreMissing},
	{DlgSHOWMSG,     L"ShowMsg",             &Opt.ShowMsg},
	{DlgSOUND,       L"Sound",               &Opt.Sound},
	{DlgDIALOG,      L"Dialog",              &Opt.Dialog},
	{DlgTOTALPROCESS,L"TotalProcess",        &Opt.TotalProcess}
};


/****************************************************************************
 * Обработчик диалога для ShowOptDialog
 ****************************************************************************/

INT_PTR WINAPI AdvCmpDlgOpt::ShowOptDialogProcThunk(HANDLE hDlg, int Msg, int Param1, void *Param2)
{
	AdvCmpDlgOpt* Class=(AdvCmpDlgOpt*)Info.SendDlgMessage(hDlg,DM_GETDLGDATA,0,0);
	return Class->ShowOptDialogProc(hDlg,Msg,Param1,Param2);
}

INT_PTR WINAPI AdvCmpDlgOpt::ShowOptDialogProc(HANDLE hDlg, int Msg, int Param1, void *Param2)
{

	switch (Msg)
	{
		case DN_INITDIALOG:
			{
				bool CheckSelect=false;
				if (LPanel.PInfo.SelectedItemsNumber)
				{
					FarGetPluginPanelItem FGPPI={0,0};
					FGPPI.Item=(PluginPanelItem*)malloc(FGPPI.Size=Info.PanelControl(LPanel.hPanel,FCTL_GETSELECTEDPANELITEM,0,&FGPPI));
					if (FGPPI.Item)
					{
						Info.PanelControl(LPanel.hPanel,FCTL_GETSELECTEDPANELITEM,0,&FGPPI);
						if (FGPPI.Item->Flags&PPIF_SELECTED) CheckSelect=true;
						free(FGPPI.Item);
					}
				}
				if (!CheckSelect && RPanel.PInfo.SelectedItemsNumber)
				{
					FarGetPluginPanelItem FGPPI={0,0};
					FGPPI.Item=(PluginPanelItem*)malloc(FGPPI.Size=Info.PanelControl(RPanel.hPanel,FCTL_GETSELECTEDPANELITEM,0,&FGPPI));
					if (FGPPI.Item)
					{
						Info.PanelControl(RPanel.hPanel,FCTL_GETSELECTEDPANELITEM,0,&FGPPI);
						if (FGPPI.Item->Flags&PPIF_SELECTED) CheckSelect=true;
						free(FGPPI.Item);
					}
				}
				if (!CheckSelect)
				{
					Info.SendDlgMessage(hDlg,DM_SETCHECK,DlgSELECTED,(void*)BSTATE_UNCHECKED);
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgSELECTED,(void*)false);
				}

				//-------------
				if ((LPanel.PInfo.Flags&PFLAGS_PLUGIN) || (RPanel.PInfo.Flags&PFLAGS_PLUGIN))
				{
					Opt.Sync=0;
					Info.SendDlgMessage(hDlg,DM_SETCHECK,DlgCMP,(void*)BSTATE_CHECKED);
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgCMP,(void*)false);
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgSYNC,(void*)false);
				}
				if (Opt.Sync)
				{
					Info.SendDlgMessage(hDlg,DM_SETCHECK,DlgSYNC,(void*)BSTATE_CHECKED);
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgLCMPSKIP,(void*)false);
					Info.SendDlgMessage(hDlg,DM_SETCHECK,DlgLCMPSKIP,(void*)BSTATE_UNCHECKED);
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgECMPSKIP,(void*)false);
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgFIRSTDIFF,(void*)false);
					Info.SendDlgMessage(hDlg,DM_SETCHECK,DlgFIRSTDIFF,(void*)BSTATE_UNCHECKED);
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgSELECTEDNEW,(void*)false);
					Info.SendDlgMessage(hDlg,DM_SETCHECK,DlgSELECTEDNEW,(void*)BSTATE_UNCHECKED);
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgDIALOG,(void*)false);
					Info.SendDlgMessage(hDlg,DM_SETCHECK,DlgDIALOG,(void*)BSTATE_CHECKED);
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgUNDERCURSOR,(void*)false);
				}
				else
					Info.SendDlgMessage(hDlg,DM_SETCHECK,DlgCMP,(void*)BSTATE_CHECKED);

				//------------порядок важен! идем из глубины опций наверх
				if (Opt.LowPrecisionTime)
					Info.SendDlgMessage(hDlg,DM_SETCHECK,DlgPRECISION,(void*)BSTATE_CHECKED);
				else
					Info.SendDlgMessage(hDlg,DM_SETCHECK,DlgIGNORESEC,(void*)BSTATE_CHECKED);
				if (!Opt.Seconds)
				{
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgIGNORESEC,(void*)false);
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgPRECISION,(void*)false);
				}
				if (!Opt.CmpTime)
				{
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgSECONDS,(void*)false);
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgIGNORESEC,(void*)false);
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgPRECISION,(void*)false);
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgTIMEZONE,(void*)false);
				}

				//--------
				if ( !( ((LPanel.PInfo.Flags&PFLAGS_REALNAMES) && (RPanel.PInfo.Flags&PFLAGS_REALNAMES)) ||
								((LPanel.PInfo.Flags&PFLAGS_REALNAMES) && RPanel.bARC) ||
								((RPanel.PInfo.Flags&PFLAGS_REALNAMES) && LPanel.bARC) ||
								(LPanel.bARC && RPanel.bARC) ) )
				{
					Opt.CmpContents=0;
					Opt.Cache=0;
					Info.SendDlgMessage(hDlg,DM_SETCHECK,DlgCMPCONTENTS,(void*)BSTATE_UNCHECKED);
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgCMPCONTENTS,(void*)false);
				}
				if (Opt.CmpContents && (LPanel.bARC || RPanel.bARC))
				{
					Opt.Partly=0;
					Opt.Ignore=0;
					Opt.Cache=0;
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgPARTLY,(void*)false);
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgIGNORE,(void*)false);
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgCACHE,(void*)false);
				}
				//------------ порядок важен! идем из глубины опций наверх
				if (Opt.PartlyFull)
				{
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgEPARTLYKB,(void*)false);
					Info.SendDlgMessage(hDlg,DM_SETCHECK,DlgPARTLYFULL,(void*)BSTATE_CHECKED);
				}
				else
					Info.SendDlgMessage(hDlg,DM_SETCHECK,DlgLPARTLYKB,(void*)BSTATE_CHECKED);

				if (!Opt.Partly)
				{
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgPARTLYFULL,(void*)false);
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgLPARTLYKB,(void*)false);
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgEPARTLYKB,(void*)false);
				}
				if (!Opt.Ignore)
				{
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgIGNORETEMPL,(void*)false);
				}
				//------------порядок важен! идем из глубины опций наверх
				if (Opt.CacheIgnore)
					Info.SendDlgMessage(hDlg,DM_SETCHECK,DlgCACHEIGNORE,(void*)BSTATE_CHECKED);
				else
					Info.SendDlgMessage(hDlg,DM_SETCHECK,DlgCACHEUSE,(void*)BSTATE_CHECKED);

				if (!Opt.Cache)
				{
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgCACHEIGNORE,(void*)false);
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgCACHEUSE,(void*)false);
				}

				if (!Opt.CmpContents)
				{
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgDIFFTIME,(void*)false);
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgPARTLY,(void*)false);
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgPARTLYFULL,(void*)false);
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgLPARTLYKB,(void*)false);
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgEPARTLYKB,(void*)false);
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgIGNORE,(void*)false);
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgIGNORETEMPL,(void*)false);
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgCACHE,(void*)false);
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgCACHEIGNORE,(void*)false);
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgCACHEUSE,(void*)false);
				}
				//-----------
				if (!Cache.ItemsNumber)
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgCACHECLEAR,(void*)false);
				//------------
				if (LPanel.bTMP || RPanel.bTMP || (!LPanel.bDir && !RPanel.bDir))
				{
					Opt.ProcessSubfolders=0;
					Info.SendDlgMessage(hDlg,DM_SETCHECK,DlgSUBFOLDER,(void*)BSTATE_UNCHECKED);
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgSUBFOLDER,(void*)false);
				}
				if (Opt.ProcessSubfolders!=2)
				{
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgLMAXDEPTH,(void*)false);
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgEMAXDEPTH,(void*)false);
				}
				//------
				Opt.ScanSymlink=Info.AdvControl(&MainGuid,ACTL_GETSYSTEMSETTINGS,0,0)&FSS_SCANSYMLINK;
				Info.SendDlgMessage(hDlg,DM_SETCHECK,DlgSCANSYMLINK,(void*)(Opt.ScanSymlink?BSTATE_CHECKED:BSTATE_UNCHECKED));
				if (!Opt.ProcessSubfolders || Opt.Sync || (LPanel.PInfo.Flags&PFLAGS_PLUGIN) || (RPanel.PInfo.Flags&PFLAGS_PLUGIN))
				{
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgSCANSYMLINK,(void*)false);
					Info.SendDlgMessage(hDlg,DM_SETCHECK,DlgSCANSYMLINK,(void*)BSTATE_UNCHECKED);
				}
				//------------
				if (!Opt.Filter)
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgFILTERBOTTON,(void*)false);
				//------------
				if (!Opt.SkipSubstr)
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgECMPSKIP,(void*)false);
				//------------
				if (LPanel.bTMP || RPanel.bTMP)
				{
					Info.SendDlgMessage(hDlg,DM_SETCHECK,DlgDIALOG,(void*)BSTATE_UNCHECKED);
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgDIALOG,(void*)false);
				}
				//------------
				if (!LPanel.bCurFile || !RPanel.bCurFile || Opt.Sync)
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgUNDERCURSOR,(void*)false);

				// определим остальные опции...
				Opt.ProcessHidden=Info.AdvControl(&MainGuid,ACTL_GETPANELSETTINGS,0,0) & FPS_SHOWHIDDENANDSYSTEMFILES;
				Opt.hCustomFilter=INVALID_HANDLE_VALUE;

				return true;
			}

	/************************************************************************/
/*
		case DN_DRAWDIALOG:
			{
				string sep=GetMsg(MDialog);
				if (Opt.Dialog) sep[(size_t)1]=0x221a;
				Info.SendDlgMessage(hDlg, DM_SETTEXTPTR, DlgSEP2_DIALOG, sep.get());
				return true;
			}
*/
	/************************************************************************/

		case DN_BTNCLICK:
			if (Param1 == DlgCMP)
			{
				if (Param2)
				{
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgSCANSYMLINK,(void*)(Info.SendDlgMessage(hDlg,DM_GETCHECK,DlgSUBFOLDER,0) && !((LPanel.PInfo.Flags&PFLAGS_PLUGIN) || (RPanel.PInfo.Flags&PFLAGS_PLUGIN))) );
					Info.SendDlgMessage(hDlg,DM_SETCHECK,DlgSCANSYMLINK,(void*)(Opt.ScanSymlink && Info.SendDlgMessage(hDlg,DM_GETCHECK,DlgSUBFOLDER,0)?BSTATE_CHECKED:BSTATE_UNCHECKED));
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgLCMPSKIP,(void*)true);
					Info.SendDlgMessage(hDlg,DM_SETCHECK,DlgLCMPSKIP,(void*)(Opt.SkipSubstr?BSTATE_CHECKED:BSTATE_UNCHECKED));
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgECMPSKIP,(void*)(Opt.SkipSubstr?true:false));
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgFIRSTDIFF,(void*)true);
					Info.SendDlgMessage(hDlg,DM_SETCHECK,DlgFIRSTDIFF,(void*)(Opt.ProcessTillFirstDiff?BSTATE_CHECKED:BSTATE_UNCHECKED));
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgSELECTEDNEW,(void*)true);
					Info.SendDlgMessage(hDlg,DM_SETCHECK,DlgSELECTEDNEW,(void*)(Opt.SelectedNew?BSTATE_CHECKED:BSTATE_UNCHECKED));
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgDIALOG,(void*)true);
					Info.SendDlgMessage(hDlg,DM_SETCHECK,DlgDIALOG,(void*)(Opt.Dialog?BSTATE_CHECKED:BSTATE_UNCHECKED));
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgUNDERCURSOR,(void*)(!LPanel.bCurFile || !RPanel.bCurFile?false:true));
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgONLYRIGHT,(void*)false);
					Info.SendDlgMessage(hDlg,DM_SETCHECK,DlgONLYRIGHT,(void*)BSTATE_UNCHECKED);
				}
			}
			//-------------
			else if (Param1 == DlgSYNC)
			{
				if (Param2)
				{
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgSCANSYMLINK,(void*)false);
					Info.SendDlgMessage(hDlg,DM_SETCHECK,DlgSCANSYMLINK,(void*)BSTATE_UNCHECKED);
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgLCMPSKIP,(void*)false);
					Info.SendDlgMessage(hDlg,DM_SETCHECK,DlgLCMPSKIP,(void*)BSTATE_UNCHECKED);
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgECMPSKIP,(void*)false);
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgFIRSTDIFF,(void*)false);
					Info.SendDlgMessage(hDlg,DM_SETCHECK,DlgFIRSTDIFF,(void*)BSTATE_UNCHECKED);
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgSELECTEDNEW,(void*)false);
					Info.SendDlgMessage(hDlg,DM_SETCHECK,DlgSELECTEDNEW,(void*)BSTATE_UNCHECKED);
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgDIALOG,(void*)false);
					Info.SendDlgMessage(hDlg,DM_SETCHECK,DlgDIALOG,(void*)BSTATE_CHECKED);
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgUNDERCURSOR,(void*)false);
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgONLYRIGHT,(void*)true);
					Info.SendDlgMessage(hDlg,DM_SETCHECK,DlgONLYRIGHT,(void*)(Opt.SyncOnlyRight?BSTATE_CHECKED:BSTATE_UNCHECKED));
				}
			}
			//-------------
			else if (Param1 == DlgCMPTIME)
			{
				if (Param2)
				{
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgSECONDS,(void*)true);
					if (Info.SendDlgMessage(hDlg,DM_GETCHECK,DlgSECONDS,0))
					{
						Info.SendDlgMessage(hDlg,DM_ENABLE,DlgIGNORESEC,(void*)true);
						Info.SendDlgMessage(hDlg,DM_ENABLE,DlgPRECISION,(void*)true);
					}
					else
					{
						Info.SendDlgMessage(hDlg,DM_ENABLE,DlgIGNORESEC,(void*)false);
						Info.SendDlgMessage(hDlg,DM_ENABLE,DlgPRECISION,(void*)false);
					}
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgTIMEZONE,(void*)true);
				}
				else
				{
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgSECONDS,(void*)false);
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgIGNORESEC,(void*)false);
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgPRECISION,(void*)false);
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgTIMEZONE,(void*)false);
				}
			}
			//------------
			else if (Param1 == DlgSECONDS)
			{
				if (Param2)
				{
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgIGNORESEC,(void*)true);
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgPRECISION,(void*)true);
				}
				else
				{
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgIGNORESEC,(void*)false);
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgPRECISION,(void*)false);
				}
			}
			//-----------
			else if (Param1 == DlgCMPCONTENTS)
			{
				if (Param2)
				{
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgDIFFTIME,(void*)true);

					if (!(LPanel.bARC || RPanel.bARC))
						Info.SendDlgMessage(hDlg,DM_ENABLE,DlgPARTLY,(void*)true);
					if (Info.SendDlgMessage(hDlg,DM_GETCHECK,DlgPARTLY,0))
					{
						Info.SendDlgMessage(hDlg,DM_ENABLE,DlgPARTLYFULL,(void*)true);
						Info.SendDlgMessage(hDlg,DM_ENABLE,DlgLPARTLYKB,(void*)true);
						if (Info.SendDlgMessage(hDlg,DM_GETCHECK,DlgLPARTLYKB,0))
							Info.SendDlgMessage(hDlg,DM_ENABLE,DlgEPARTLYKB,(void*)true);
						else
							Info.SendDlgMessage(hDlg,DM_ENABLE,DlgEPARTLYKB,(void*)false);
					}
					else
					{
						Info.SendDlgMessage(hDlg,DM_ENABLE,DlgPARTLYFULL,(void*)false);
						Info.SendDlgMessage(hDlg,DM_ENABLE,DlgLPARTLYKB,(void*)false);
						Info.SendDlgMessage(hDlg,DM_ENABLE,DlgEPARTLYKB,(void*)false);
					}

					if (!(LPanel.bARC || RPanel.bARC))
						Info.SendDlgMessage(hDlg,DM_ENABLE,DlgIGNORE,(void*)true);
					if (Info.SendDlgMessage(hDlg,DM_GETCHECK,DlgIGNORE,0))
						Info.SendDlgMessage(hDlg,DM_ENABLE,DlgIGNORETEMPL,(void*)true);
					else
						Info.SendDlgMessage(hDlg,DM_ENABLE,DlgIGNORETEMPL,(void*)false);

					if (!(LPanel.bARC || RPanel.bARC))
						Info.SendDlgMessage(hDlg,DM_ENABLE,DlgCACHE,(void*)true);
					if (Info.SendDlgMessage(hDlg,DM_GETCHECK,DlgCACHE,0))
					{
						Info.SendDlgMessage(hDlg,DM_ENABLE,DlgCACHEIGNORE,(void*)true);
						Info.SendDlgMessage(hDlg,DM_ENABLE,DlgCACHEUSE,(void*)true);
					}
					else
					{
						Info.SendDlgMessage(hDlg,DM_ENABLE,DlgCACHEIGNORE,(void*)false);
						Info.SendDlgMessage(hDlg,DM_ENABLE,DlgCACHEUSE,(void*)false);
					}
				}
				else
				{
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgDIFFTIME,(void*)false);
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgCACHE,(void*)false);
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgCACHEIGNORE,(void*)false);
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgCACHEUSE,(void*)false);
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgPARTLY,(void*)false);
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgPARTLYFULL,(void*)false);
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgLPARTLYKB,(void*)false);
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgEPARTLYKB,(void*)false);
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgIGNORE,(void*)false);
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgIGNORETEMPL,(void*)false);
				}
			}
			//------------
			else if (Param1 == DlgCACHE)
			{
				if (Param2)
				{
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgCACHEIGNORE,(void*)true);
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgCACHEUSE,(void*)true);
				}
				else
				{
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgCACHEIGNORE,(void*)false);
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgCACHEUSE,(void*)false);
				}
			}
			//------------
			else if (Param1 == DlgPARTLY)
			{
				if (Param2)
				{
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgPARTLYFULL,(void*)true);
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgLPARTLYKB,(void*)true);
					if (Info.SendDlgMessage(hDlg,DM_GETCHECK,DlgLPARTLYKB,0))
						Info.SendDlgMessage(hDlg,DM_ENABLE,DlgEPARTLYKB,(void*)true);
					else
						Info.SendDlgMessage(hDlg,DM_ENABLE,DlgEPARTLYKB,(void*)false);
				}
				else
				{
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgPARTLYFULL,(void*)false);
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgLPARTLYKB,(void*)false);
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgEPARTLYKB,(void*)false);
				}
			}
			//------------
			else if (Param1 == DlgLPARTLYKB)
			{
				if (Param2)
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgEPARTLYKB,(void*)true);
				else
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgEPARTLYKB,(void*)false);
			}
			//------------
			else if (Param1 == DlgIGNORE)
			{
				if (Param2)
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgIGNORETEMPL,(void*)true);
				else
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgIGNORETEMPL,(void*)false);
			}
			//------------
			else if (Param1 == DlgSUBFOLDER)
			{
				if ((int)Param2 == 1)
				{
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgLMAXDEPTH,(void*)false);
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgEMAXDEPTH,(void*)false);
					if (!((LPanel.PInfo.Flags&PFLAGS_PLUGIN) || (RPanel.PInfo.Flags&PFLAGS_PLUGIN)) && !Info.SendDlgMessage(hDlg,DM_GETCHECK,DlgSYNC,0))
					{
						Info.SendDlgMessage(hDlg,DM_ENABLE,DlgSCANSYMLINK,(void*)true);
						Info.SendDlgMessage(hDlg,DM_SETCHECK,DlgSCANSYMLINK,(void*)(Opt.ScanSymlink?BSTATE_CHECKED:BSTATE_UNCHECKED));
					}
					else
					{
						Info.SendDlgMessage(hDlg,DM_ENABLE,DlgSCANSYMLINK,(void*)false);
						Info.SendDlgMessage(hDlg,DM_SETCHECK,DlgSCANSYMLINK,(void*)BSTATE_UNCHECKED);
					}
				}
				else if ((int)Param2 == 2)
				{
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgLMAXDEPTH,(void*)true);
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgEMAXDEPTH,(void*)true);
					if (!((LPanel.PInfo.Flags&PFLAGS_PLUGIN) || (RPanel.PInfo.Flags&PFLAGS_PLUGIN)) && !Info.SendDlgMessage(hDlg,DM_GETCHECK,DlgSYNC,0))
					{
						Info.SendDlgMessage(hDlg,DM_ENABLE,DlgSCANSYMLINK,(void*)true);
						Info.SendDlgMessage(hDlg,DM_SETCHECK,DlgSCANSYMLINK,(void*)(Opt.ScanSymlink?BSTATE_CHECKED:BSTATE_UNCHECKED));
					}
					else
					{
						Info.SendDlgMessage(hDlg,DM_ENABLE,DlgSCANSYMLINK,(void*)false);
						Info.SendDlgMessage(hDlg,DM_SETCHECK,DlgSCANSYMLINK,(void*)BSTATE_UNCHECKED);
					}
				}
				else
				{
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgLMAXDEPTH,(void*)false);
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgEMAXDEPTH,(void*)false);
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgSCANSYMLINK,(void*)false);
					Info.SendDlgMessage(hDlg,DM_SETCHECK,DlgSCANSYMLINK,(void*)BSTATE_UNCHECKED);
				}
			}
			//------------
			else if (Param1 == DlgFILTER)
			{
				if (Param2)
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgFILTERBOTTON,(void*)true);
				else
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgFILTERBOTTON,(void*)false);
			}
			//------------
			else if (Param1 == DlgLCMPSKIP)
			{
				if (Param2)
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgECMPSKIP,(void*)true);
				else
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgECMPSKIP,(void*)false);
			}
			break;


	/************************************************************************/

		case DN_CONTROLINPUT:
		{
			const INPUT_RECORD* record=(const INPUT_RECORD *)Param2;

			if (record->EventType==KEY_EVENT && record->Event.KeyEvent.bKeyDown)
			{
				WORD vk=record->Event.KeyEvent.wVirtualKeyCode;
				if (IsNone(record))
				{
					if (vk == VK_F4 && Info.SendDlgMessage(hDlg,DM_GETCHECK,DlgFILTER,0))
						Info.SendDlgMessage(hDlg,DM_CLOSE,DlgFILTERBOTTON,0);
					else if (vk == VK_F8 && Info.SendDlgMessage(hDlg,DM_ENABLE,DlgCACHECLEAR,(void*)-1))
						Info.SendDlgMessage(hDlg,DM_CLOSE,DlgCACHECLEAR,0);
					else if (vk == VK_F2 && Info.SendDlgMessage(hDlg,DM_ENABLE,DlgUNDERCURSOR,(void*)-1))
						Info.SendDlgMessage(hDlg,DM_CLOSE,DlgUNDERCURSOR,0);
				}
			}
		}
		break;

	/************************************************************************/

		case DN_CLOSE:
			if (Param1 == DlgFILTERBOTTON)
			{
				if (Opt.hCustomFilter == INVALID_HANDLE_VALUE)
					Opt.Filter=Info.FileFilterControl(PANEL_NONE,FFCTL_CREATEFILEFILTER,FFT_CUSTOM,&Opt.hCustomFilter);
				if (Opt.hCustomFilter != INVALID_HANDLE_VALUE)
					Info.FileFilterControl(Opt.hCustomFilter,FFCTL_OPENFILTERSMENU,0,0);
				return false;
			}
			else if (Param1 == DlgCACHECLEAR)
			{
				if (Cache.RCI)
				{
					if (Info.AdvControl(&MainGuid,ACTL_GETCONFIRMATIONS,0,0) & FCS_DELETE)
					{
						wchar_t buf[100]; FSF.sprintf(buf,GetMsg(MClearCacheItems),Cache.ItemsNumber);
						const wchar_t *MsgItems[]={ GetMsg(MClearCacheTitle),buf,GetMsg(MClearCacheBody) };
						if (!Info.Message(&MainGuid,&ClearCacheMsgGuid,FMSG_WARNING|FMSG_MB_YESNO,0,MsgItems,3,0))
							goto ClearCache;
					}
					else
					ClearCache:
					{
						free(Cache.RCI);
						Cache.RCI=0;
						Cache.ItemsNumber=0;
						Info.SendDlgMessage(hDlg,DM_ENABLE,DlgCACHECLEAR,(void*)false);
					}
				}
				return false;
			}
			else if (Param1 == DlgUNDERCURSOR)
			{
				for (int i=DlgCMPCASE; i<DlgSEP1; i++)
				{
					if (StoreOpt[i].RegName)
					{
						if (i==DlgEPARTLYKB)
							*StoreOpt[i].Option=FSF.atoi((const wchar_t *)Info.SendDlgMessage(hDlg,DM_GETCONSTTEXTPTR,i,0));
						else if (i==DlgIGNORETEMPL)
							*StoreOpt[i].Option=Info.SendDlgMessage(hDlg,DM_LISTGETCURPOS,i,0);
						else
							*StoreOpt[i].Option=Info.SendDlgMessage(hDlg,DM_GETCHECK,i,0);
					}
				}
				FarSettingsCreate settings={sizeof(FarSettingsCreate),MainGuid,INVALID_HANDLE_VALUE};
				if (Info.SettingsControl(INVALID_HANDLE_VALUE,SCTL_CREATE,0,&settings))
				{
					int Root=0; // корень ключа
					for (int i=DlgCMPCASE; i<DlgSEP1; i++)
					{
						if (StoreOpt[i].RegName && Info.SendDlgMessage(hDlg,DM_ENABLE,i,(void*)-1))
						{
							FarSettingsItem item={Root,StoreOpt[i].RegName,FST_QWORD};
							item.Number=*StoreOpt[i].Option;
							Info.SettingsControl(settings.Handle,SCTL_SET,0,&item);
						}
					}
					Info.SettingsControl(settings.Handle,SCTL_FREE,0,0);
				}
				return true;
			}
			else if (Param1 == DlgOK)
			{
				for (int i=DlgSYNC; i<(sizeof(StoreOpt)/sizeof(StoreOpt[0])); i++)
				{
					if (StoreOpt[i].RegName)
					{
						if (i==DlgEPARTLYKB || i==DlgEMAXDEPTH)
							*StoreOpt[i].Option=FSF.atoi((const wchar_t *)Info.SendDlgMessage(hDlg,DM_GETCONSTTEXTPTR,i,0));
						else if (i==DlgIGNORETEMPL)
							*StoreOpt[i].Option=Info.SendDlgMessage(hDlg,DM_LISTGETCURPOS,i,0);
						else
							*StoreOpt[i].Option=Info.SendDlgMessage(hDlg,DM_GETCHECK,i,0);
					}
				}

				Opt.ScanSymlink=Info.SendDlgMessage(hDlg,DM_GETCHECK,DlgSCANSYMLINK,0);

				Opt.Substr=NULL;
				int len=Info.SendDlgMessage(hDlg,DM_GETTEXTPTR,DlgECMPSKIP,0);
				if (len)
				{
					Opt.Substr=(wchar_t*)malloc((len+1)*sizeof(wchar_t));
					if (Opt.Substr)
						Info.SendDlgMessage(hDlg,DM_GETTEXTPTR,DlgECMPSKIP,Opt.Substr);
					else
						Opt.SkipSubstr=0;
				}
				else
				{
					Opt.SkipSubstr=0;
					if (Opt.Substr) free(Opt.Substr);
				}

				if (Opt.Filter && Opt.hCustomFilter == INVALID_HANDLE_VALUE)
					Opt.Filter=Info.FileFilterControl(PANEL_NONE,FFCTL_CREATEFILEFILTER,FFT_CUSTOM,&Opt.hCustomFilter);

				FarSettingsCreate settings={sizeof(FarSettingsCreate),MainGuid,INVALID_HANDLE_VALUE};
				if (Info.SettingsControl(INVALID_HANDLE_VALUE,SCTL_CREATE,0,&settings))
				{
					int Root=0; // корень ключа
					for (int i=DlgSYNC; i<sizeof(StoreOpt)/sizeof(StoreOpt[0]); i++)
					{
						if (StoreOpt[i].RegName && Info.SendDlgMessage(hDlg,DM_ENABLE,i,(void*)-1))
						{
							FarSettingsItem item={Root,StoreOpt[i].RegName,FST_QWORD};
							item.Number=*StoreOpt[i].Option;
							Info.SettingsControl(settings.Handle,SCTL_SET,0,&item);
						}
					}
					Info.SettingsControl(settings.Handle,SCTL_FREE,0,0);
				}
				return true;
			}
			break;

	}
	return Info.DefDlgProc(hDlg, Msg, Param1, Param2);
}

/****************************************************************************
 * Читает настройки из реестра, показывает диалог с опциями сравнения,
 * заполняет структуру Opt, сохраняет (если надо) новые настройки в реестре,
 * возвращает true, если пользователь нажал OK
 ****************************************************************************/
int AdvCmpDlgOpt::ShowOptDialog()
{
	const unsigned int dW = 68;   // ширина
	const unsigned int dH = 25;   // высота

	struct FarDialogItem DialogItems[] = {
		//			Type	X1	Y1	X2	Y2		Selected	History					Mask															Flags	Data	MaxLen	UserParam
		/* 0*/{DI_DOUBLEBOX,  0, 0,  dW,  dH, 0, 0,                   0,                                0, GetMsg(MCompareTitle),0,0},
		/* 1*/{DI_TEXT,       2, 1,   0,   0, 0, 0,                   0,                                0, GetMsg(MMode),0,0},
		/* 2*/{DI_RADIOBUTTON,0, 1,  33,   0, 1, 0,                   0,                        DIF_GROUP, GetMsg(MCompare),0,0},
		/* 3*/{DI_RADIOBUTTON,35,1,   0,   0, 0, 0,                   0,                                0, GetMsg(MSynchronize),0,0},
		/* 4*/{DI_CHECKBOX,   2, 2,   0,   0, 0, 0,                   0,                                0, GetMsg(MCompareCase),0,0},
		/* 5*/{DI_CHECKBOX,   2, 3,   0,   0, 1, 0,                   0,                                0, GetMsg(MCompareSize),0,0},
		/* 6*/{DI_CHECKBOX,   2, 4,   0,   0, 1, 0,                   0,                                0, GetMsg(MCompareTime),0,0},
		/* 7*/{DI_CHECKBOX,   6, 5,   0,   0, 0, 0,                   0,                                0, GetMsg(MCompareSeconds),0,0},
		/* 8*/{DI_RADIOBUTTON,0, 5,  33,   0, 0, 0,                   0,                        DIF_GROUP, GetMsg(MCompareIgnoreSeconds),0,0},
		/* 9*/{DI_RADIOBUTTON,35,5,   0,   0, 1, 0,                   0,                                0, GetMsg(MCompareLowPrecision),0,0},
		/*10*/{DI_CHECKBOX,   6, 6,   0,   0, 0, 0,                   0,                                0, GetMsg(MCompareIgnoreTimeZone),0,0},
		/*11*/{DI_CHECKBOX,   2, 7,   0,   0, 0, 0,                   0,                                0, GetMsg(MCompareContents),0,0},
		/*12*/{DI_CHECKBOX,   6, 8,   0,   0, 0, 0,                   0,                                0, GetMsg(MCompareOnlyTimeDiff),0,0},
		/*13*/{DI_CHECKBOX,   6, 9,   0,   0, 0, 0,                   0,                                0, GetMsg(MCache),0,0},
		/*14*/{DI_RADIOBUTTON,0, 9,  33,   0, 0, 0,                   0,                        DIF_GROUP, GetMsg(MCacheIgnore),0,0},
		/*15*/{DI_RADIOBUTTON,35,9,  44,   0, 1, 0,                   0,                                0, GetMsg(MCacheUse),0,0},
		/*16*/{DI_BUTTON,     0, 9,dW-3,   0, 0, 0,                   0,                                0, GetMsg(MCacheClearBotton),0,0},
		/*17*/{DI_CHECKBOX,   6,10,   0,   0, 0, 0,                   0,                                0, GetMsg(MComparePartly),0,0},
		/*18*/{DI_RADIOBUTTON,0,10,  33,   0, 1, 0,                   0,                        DIF_GROUP, GetMsg(MComparePartlyFull),0,0},
		/*19*/{DI_RADIOBUTTON,35,10, 52,   0, 0, 0,                   0,                                0, GetMsg(MComparePartlyKb),0,0},
		/*20*/{DI_FIXEDIT,   54,10,dW-3,   0, 0, 0,       L"##########",                     DIF_MASKEDIT, L"64",0,0},
		/*21*/{DI_CHECKBOX,   6,11,  22,   0, 0, 0,                   0,                                0, GetMsg(MCompareIgnore),0,0},
		/*22*/{DI_COMBOBOX,   0,11,dW-3,   0, 0, 0,                   0,DIF_LISTAUTOHIGHLIGHT|DIF_DROPDOWNLIST|DIF_LISTWRAPMODE, L"",0,0},

		/*23*/{DI_TEXT,      -1,12,   0,   0, 0, 0,                   0,                    DIF_SEPARATOR, GetMsg(MTitleOptions),0,0},
		/*24*/{DI_CHECKBOX,   2,13,   0,   0, 0, 0,                   0,                       DIF_3STATE, GetMsg(MProcessSubfolders),0,0},
		/*25*/{DI_TEXT,       0,13,   0,   0, 0, 0,                   0,                                0, GetMsg(MMaxScanDepth),0,0},
		/*26*/{DI_FIXEDIT,    0,13,   2,   0, 0, 0,              L"999",                     DIF_MASKEDIT, L"10",0,0},
		/*27*/{DI_CHECKBOX,   2,14,   0,   0, 0, 0,                   0,                                0, GetMsg(MScanSymlink),0,0},
		/*28*/{DI_CHECKBOX,   2,15,   0,   0, 0, 0,                   0,                                0, GetMsg(MFilter),0,0},
		/*29*/{DI_BUTTON,     0,15,dW-3,   0, 0, 0,                   0,                                0, GetMsg(MFilterBotton),0,0},
		/*30*/{DI_CHECKBOX,   2,16,   0,   0, 1, 0,                   0,                                0, GetMsg(MProcessTillFirstDiff),0,0},
		/*31*/{DI_CHECKBOX,   2,17,   0,   0, 0, 0,                   0,                                0, GetMsg(MCompareSkipSubstr),0,0},
		/*32*/{DI_EDIT,       0,17,dW-3,   0, 0, L"AdvCmpSkipSubstr", 0,   DIF_USELASTHISTORY|DIF_HISTORY, L"",0,0},
		/*33*/{DI_CHECKBOX,   2,18,  33,   0, 0, 0,                   0,                                0, GetMsg(MProcessSelected),0,0},
		/*34*/{DI_CHECKBOX,  35,18,   0,   0, 0, 0,                   0,                                0, GetMsg(MSelectedNew),0,0},
		/*35*/{DI_CHECKBOX,   2,19,  33,   0, 0, 0,                   0,                                0, GetMsg(MSyncOnlyRight),0,0},
		/*36*/{DI_CHECKBOX,  35,19,   0,   0, 0, 0,                   0,                                0, GetMsg(MIgnoreMissing),0,0},
		/*37*/{DI_CHECKBOX,   2,20,   0,   0, 1, 0,                   0,                                0, GetMsg(MShowMsg),0,0},
		/*38*/{DI_CHECKBOX,  35,20,   0,   0, 0, 0,                   0,                                0, GetMsg(MSound),0,0},
		/*39*/{DI_CHECKBOX,   2,21,   0,   0, 0, 0,                   0,                                0, GetMsg(MDialog),0,0},
		/*40*/{DI_CHECKBOX,  35,21,   0,   0, 0, 0,                   0,                                0, GetMsg(MTotalProcess),0,0},

		/*41*/{DI_TEXT,      -1,22,   0,   0, 0, 0,                   0,                    DIF_SEPARATOR, L"",0,0},
		/*42*/{DI_BUTTON,     0,23,   0,   0, 0, 0,                   0,DIF_CENTERGROUP|DIF_DEFAULTBUTTON, GetMsg(MOK),0,0},
		/*43*/{DI_BUTTON,     0,23,   0,   0, 0, 0,                   0,                  DIF_CENTERGROUP, GetMsg(MUnderCursorBotton),0,0},
		/*44*/{DI_BUTTON,     0,23,   0,   0, 0, 0,                   0,                  DIF_CENTERGROUP, GetMsg(MCancel),0,0}
	};

	// динамические координаты для строк
	DialogItems[DlgCMP].X1 = DialogItems[DlgMODE].X1 + wcslen(DialogItems[DlgMODE].Data) + 1;
	DialogItems[DlgIGNORESEC].X1 = DialogItems[DlgSECONDS].X1 + wcslen(DialogItems[DlgSECONDS].Data) - (wcschr(DialogItems[DlgSECONDS].Data, L'&')?1:0) + 5;
	DialogItems[DlgECMPSKIP].X1 = DialogItems[DlgLCMPSKIP].X1 + wcslen(DialogItems[DlgLCMPSKIP].Data) - (wcschr(DialogItems[DlgLCMPSKIP].Data, L'&')?1:0) + 5;
	DialogItems[DlgLMAXDEPTH].X1 = DialogItems[DlgSUBFOLDER].X1 + wcslen(DialogItems[DlgSUBFOLDER].Data) - (wcschr(DialogItems[DlgSUBFOLDER].Data, L'&')?1:0) + 5;
	DialogItems[DlgLMAXDEPTH].X2 += DialogItems[DlgLMAXDEPTH].X1;
	DialogItems[DlgEMAXDEPTH].X1 = DialogItems[DlgLMAXDEPTH].X1 + wcslen(DialogItems[DlgLMAXDEPTH].Data) - (wcschr(DialogItems[DlgLMAXDEPTH].Data, L'&')?1:0) + 1;
	DialogItems[DlgEMAXDEPTH].X2 += DialogItems[DlgEMAXDEPTH].X1;
	DialogItems[DlgFILTERBOTTON].X1 = DialogItems[DlgFILTERBOTTON].X2 - wcslen(DialogItems[DlgFILTERBOTTON].Data) + (wcschr(DialogItems[DlgFILTERBOTTON].Data, L'&')?1:0) - 3;
	DialogItems[DlgCACHECLEAR].X1 = DialogItems[DlgCACHECLEAR].X2 - wcslen(DialogItems[DlgCACHECLEAR].Data) + (wcschr(DialogItems[DlgCACHECLEAR].Data, L'&')?1:0) - 3;
	DialogItems[DlgPARTLYFULL].X1 = DialogItems[DlgPARTLY].X1 + wcslen(DialogItems[DlgPARTLY].Data) - (wcschr(DialogItems[DlgPARTLY].Data, L'&')?1:0) + 5;
	DialogItems[DlgIGNORETEMPL].X1 = DialogItems[DlgIGNORE].X1 + wcslen(DialogItems[DlgIGNORE].Data) - (wcschr(DialogItems[DlgIGNORE].Data, L'&')?1:0) + 5;
	DialogItems[DlgCACHEIGNORE].X1 = DialogItems[DlgCACHE].X1 + wcslen(DialogItems[DlgCACHE].Data) - (wcschr(DialogItems[DlgCACHE].Data, L'&')?1:0) + 5;

	// расставим опции в диалоге
	FarSettingsCreate settings={sizeof(FarSettingsCreate),MainGuid,INVALID_HANDLE_VALUE};
	if (Info.SettingsControl(INVALID_HANDLE_VALUE,SCTL_CREATE,0,&settings))
	{
		int Root=0; // корень ключа
		for (int i=DlgSYNC; i<sizeof(StoreOpt)/sizeof(StoreOpt[0]); i++)
		{
			FarSettingsItem item={Root,StoreOpt[i].RegName,FST_QWORD};
			if (StoreOpt[i].RegName && Info.SettingsControl(settings.Handle,SCTL_GET,0,&item))
			{
				*StoreOpt[i].Option=(int)item.Number;
				if (i==DlgEPARTLYKB)
				{
					static wchar_t buf[20];
					FSF.itoa64(item.Number,buf,10);
					DialogItems[i].Data=buf;
				}
				else if (i==DlgEMAXDEPTH)
				{
					static wchar_t buf[5];
					FSF.itoa64(item.Number,buf,10);
					DialogItems[i].Data=buf;
				}
				else
					DialogItems[i].Selected=(int)item.Number;
			}
		}
		// узнаем пользовательский путь до WinMerge
		Opt.WinMergePath=NULL;
		FarSettingsItem item={Root,L"WinMergePath",FST_STRING};
		if (Info.SettingsControl(settings.Handle,SCTL_GET,0,&item))
		{
			Opt.WinMergePath=(wchar_t*)malloc((wcslen(item.String)+1)*sizeof(wchar_t));
			if (Opt.WinMergePath) wcscpy(Opt.WinMergePath,item.String);
		}
		Info.SettingsControl(settings.Handle,SCTL_FREE,0,0);
	}

	// комбинированный список с шаблонами фильтра по умолчанию
	FarListItem itemIgnoreTemplates[3];
	int n = sizeof(itemIgnoreTemplates) / sizeof(itemIgnoreTemplates[0]);
	for (int i=0; i<n; i++)
	{
		itemIgnoreTemplates[i].Flags = 0;
		if (i==Opt.IgnoreTemplates)
			itemIgnoreTemplates[i].Flags |= LIF_SELECTED;
		else
			itemIgnoreTemplates[i].Flags &= ~LIF_SELECTED;
		itemIgnoreTemplates[i].Text=GetMsg(MCompareIgnoreAllWhitespace+i);
	}
	FarList IgnoreTemplates = {n, itemIgnoreTemplates};
	DialogItems[DlgIGNORETEMPL].ListItems = &IgnoreTemplates;


	HANDLE hDlg=Info.DialogInit( &MainGuid, &OptDlgGuid,
                                -1, -1, dW, dH,
                                L"Contents",
                                DialogItems,
                                sizeof(DialogItems)/sizeof(DialogItems[0]),
                                0, FDLG_SMALLDIALOG,
                                ShowOptDialogProcThunk,
                                this );

	int ExitCode=-1;

	if (hDlg != INVALID_HANDLE_VALUE)
	{
		ExitCode=Info.DialogRun(hDlg);
		Info.DialogFree(hDlg);
	}
	return ExitCode;
}
