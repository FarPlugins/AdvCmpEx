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
	DlgCMP,         // 1
	DlgCMPCASE,     // 2
	DlgCMPSIZE,     // 3
	DlgCMPTIME,     // 4
	DlgPRECISION,   // 5
	DlgTIMEZONE,    // 6
	DlgCMPCONTENTS, // 7
	DlgDIFFTIME,    // 8
	DlgPARTLY,      // 9
	DlgPARTLYFULL,  //10
	DlgLPARTLYKB,   //11
	DlgEPARTLYKB,   //12
	DlgIGNORE,      //13
	DlgIGNORETEMPL, //14

	DlgSEP1,        //15
	DlgSUBFOLDER,   //16
	DlgLMAXDEPTH,   //17
	DlgEMAXDEPTH,   //18
	DlgFILTER,      //19
	DlgFILTERBOTTON,//20
	DlgSELECTED,    //21
	DlgLCMPSKIP,    //22
	DlgECMPSKIP,    //23
	DlgIGNORMISSING,//24
	DlgFIRSTDIFF,   //25
	DlgSELECTEDNEW, //26
	DlgCACHE,       //27
	DlgCACHEIGNORE, //28
	DlgCACHEUSE,    //29
	DlgCACHECLEAR,  //30
	DlgSHOWMSG,     //31
	DlgSOUND,       //32
	DlgTOTALPROCESS,//33

	DlgSEP2_DIALOG, //34
	DlgOK,          //35
	DlgUNDERCURSOR, //36
	DlgCANCEL,      //37
};

struct ParamStore
{
	int ID;
	wchar_t *RegName;
	int *Option;
} StoreOpt[] = {
	{DlgBORDER,      0,                      0},
	{DlgCMP,         0,                      0},

	{DlgCMPCASE,     L"CmpCase",             &Opt.CmpCase},
	{DlgCMPSIZE,     L"CmpSize",             &Opt.CmpSize},
	{DlgCMPTIME,     L"CmpTime",             &Opt.CmpTime},
	{DlgPRECISION,   L"LowPrecisionTime",    &Opt.LowPrecisionTime},
	{DlgTIMEZONE,    L"IgnoreTimeZone",      &Opt.IgnoreTimeZone},
	{DlgCMPCONTENTS, L"CmpContents",         &Opt.CmpContents},
	{DlgDIFFTIME,    L"OnlyTimeDiff",        &Opt.OnlyTimeDiff},
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
	{DlgFILTER,      L"Filter",              &Opt.Filter},
	{DlgFILTERBOTTON,0,                      0},
	{DlgSELECTED,    L"ProcessSelected",     &Opt.ProcessSelected},
	{DlgLCMPSKIP,    L"SkipSubstr",          &Opt.SkipSubstr},
	{DlgECMPSKIP,    0,                      0},
	{DlgIGNORMISSING,L"IgnoreMissing",       &Opt.IgnoreMissing},
	{DlgFIRSTDIFF,   L"ProcessTillFirstDiff",&Opt.ProcessTillFirstDiff},
	{DlgSELECTEDNEW, L"SelectedNew",         &Opt.SelectedNew},
	{DlgCACHE,       L"Cache",               &Opt.Cache},
	{DlgCACHEIGNORE, L"CacheIgnore",         &Opt.CacheIgnore},
	{DlgCACHEUSE,    0,                      0},
	{DlgCACHECLEAR,  0,                      0},
	{DlgSHOWMSG,     L"ShowMsg",             &Opt.ShowMsg},
	{DlgSOUND,       L"Sound",               &Opt.Sound},
	{DlgTOTALPROCESS,L"TotalProcess",        &Opt.TotalProcess},
	{DlgSEP2_DIALOG, L"Dialog",              &Opt.Dialog}
};


/****************************************************************************
 * Обработчик диалога для ShowOptDialog
 ****************************************************************************/

INT_PTR WINAPI AdvCmpDlgOpt::ShowOptDialogProcThunk(HANDLE hDlg, int Msg, int Param1, INT_PTR Param2)
{
	AdvCmpDlgOpt* Class=(AdvCmpDlgOpt*)Info.SendDlgMessage(hDlg,DM_GETDLGDATA,0,0);
	return Class->ShowOptDialogProc(hDlg,Msg,Param1,Param2);
}

INT_PTR WINAPI AdvCmpDlgOpt::ShowOptDialogProc(HANDLE hDlg, int Msg, int Param1, INT_PTR Param2)
{

	switch (Msg)
	{
		case DN_INITDIALOG:
			{
				bool CheckSelect=false;
				if (LPanel.PInfo.SelectedItemsNumber)
				{
					PluginPanelItem *PPI=(PluginPanelItem*)malloc(Info.Control(LPanel.hPanel,FCTL_GETSELECTEDPANELITEM,0,0));
					if (PPI)
					{
						Info.Control(LPanel.hPanel,FCTL_GETSELECTEDPANELITEM,0,(INT_PTR)PPI);
						if (PPI->Flags&PPIF_SELECTED) CheckSelect=true;
						free(PPI);
					}
				}
				if (!CheckSelect && RPanel.PInfo.SelectedItemsNumber)
				{
					PluginPanelItem *PPI=(PluginPanelItem*)malloc(Info.Control(RPanel.hPanel,FCTL_GETSELECTEDPANELITEM,0,0));
					if (PPI)
					{
						Info.Control(RPanel.hPanel,FCTL_GETSELECTEDPANELITEM,0,(INT_PTR)PPI);
						if (PPI->Flags&PPIF_SELECTED) CheckSelect=true;
						free(PPI);
					}
				}

				if (!CheckSelect)
				{
					Info.SendDlgMessage(hDlg,DM_SETCHECK,DlgSELECTED,BSTATE_UNCHECKED);
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgSELECTED,false);
				}
				//------------
				if (!Opt.CmpTime)
				{
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgPRECISION,false);
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgTIMEZONE,false);
				}
				//--------
				if ( !( ((LPanel.PInfo.Flags&PFLAGS_REALNAMES) && (RPanel.PInfo.Flags&PFLAGS_REALNAMES)) ||
								((LPanel.PInfo.Flags&PFLAGS_REALNAMES) && RPanel.bARC) ||
								((RPanel.PInfo.Flags&PFLAGS_REALNAMES) && LPanel.bARC) ||
								(LPanel.bARC && RPanel.bARC) ) )
				{
					Opt.CmpContents=0;
					Opt.Cache=0;
					Info.SendDlgMessage(hDlg,DM_SETCHECK,DlgCMPCONTENTS,BSTATE_UNCHECKED);
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgCMPCONTENTS,false);
				}
				if (Opt.CmpContents && (LPanel.bARC || RPanel.bARC))
				{
					Opt.Partly=0;
					Opt.Ignore=0;
					Opt.Cache=0;
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgPARTLY,false);
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgIGNORE,false);
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgCACHE,false);
				}
				//------------ порядок важен! идем из глубины опций наверх
				if (Opt.PartlyFull)
				{
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgEPARTLYKB,false);
					Info.SendDlgMessage(hDlg,DM_SETCHECK,DlgPARTLYFULL,BSTATE_CHECKED);
				}
				else
					Info.SendDlgMessage(hDlg,DM_SETCHECK,DlgLPARTLYKB,BSTATE_CHECKED);

				if (!Opt.Partly)
				{
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgPARTLYFULL,false);
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgLPARTLYKB,false);
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgEPARTLYKB,false);
				}
				if (!Opt.Ignore)
				{
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgIGNORETEMPL,false);
				}
				//------------порядок важен! идем из глубины опций наверх
				if (Opt.CacheIgnore)
					Info.SendDlgMessage(hDlg,DM_SETCHECK,DlgCACHEIGNORE,BSTATE_CHECKED);
				else
					Info.SendDlgMessage(hDlg,DM_SETCHECK,DlgCACHEUSE,BSTATE_CHECKED);

				if (!Opt.Cache)
				{
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgCACHEIGNORE,false);
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgCACHEUSE,false);
				}

				if (!Opt.CmpContents)
				{
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgDIFFTIME,false);
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgPARTLY,false);
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgPARTLYFULL,false);
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgLPARTLYKB,false);
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgEPARTLYKB,false);
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgIGNORE,false);
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgIGNORETEMPL,false);
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgCACHE,false);
				}
				//------------
				if ( LPanel.bTMP || RPanel.bTMP || 
						 !( ((LPanel.PInfo.Flags&PFLAGS_REALNAMES) && (RPanel.PInfo.Flags&PFLAGS_REALNAMES)) ||
								((LPanel.PInfo.Flags&PFLAGS_REALNAMES) && RPanel.bARC) ||
								((RPanel.PInfo.Flags&PFLAGS_REALNAMES) && LPanel.bARC) ||
								(LPanel.bARC && RPanel.bARC) ) )
				{
					Opt.ProcessSubfolders=0;
					Info.SendDlgMessage(hDlg,DM_SETCHECK,DlgSUBFOLDER,BSTATE_UNCHECKED);
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgSUBFOLDER,false);
				}
				if (Opt.ProcessSubfolders!=2)
				{
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgLMAXDEPTH,false);
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgEMAXDEPTH,false);
				}
				//------------
				if (!Opt.Filter)
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgFILTERBOTTON,false);
				//------------
				if (!Opt.SkipSubstr)
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgECMPSKIP,false);
				//-----------
				if (!Cache.ItemsNumber)
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgCACHECLEAR,false);
				//------------
				if (!LPanel.bCurFile || !RPanel.bCurFile)
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgUNDERCURSOR,false);

				// !! временно
//				Info.SendDlgMessage(hDlg,DM_ENABLE,DlgPANEL,false);

				// определим остальные опции...
				Opt.ProcessHidden=Info.AdvControl(&MainGuid,ACTL_GETPANELSETTINGS,0) & FPS_SHOWHIDDENANDSYSTEMFILES;
				Opt.hCustomFilter=INVALID_HANDLE_VALUE;

				return true;
			}

	/************************************************************************/

		case DN_DRAWDIALOG:
			{
				string sep=GetMsg(MDialog);
				if (Opt.Dialog) sep[(size_t)1]=0x221a;
				Info.SendDlgMessage(hDlg, DM_SETTEXTPTR, DlgSEP2_DIALOG, (INT_PTR)sep.get());
				return true;
			}

	/************************************************************************/

		case DN_BTNCLICK:
			if (Param1 == DlgCMPTIME)
			{
				if (Param2)
				{
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgPRECISION,true);
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgTIMEZONE,true);
				}
				else
				{
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgPRECISION,false);
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgTIMEZONE,false);
				}
			}
			//------------
			else if (Param1 == DlgCMPCONTENTS)
			{
				if (Param2)
				{
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgDIFFTIME,true);

					if (!(LPanel.bARC || RPanel.bARC))
						Info.SendDlgMessage(hDlg,DM_ENABLE,DlgPARTLY,true);
					if (Info.SendDlgMessage(hDlg,DM_GETCHECK,DlgPARTLY,0))
					{
						Info.SendDlgMessage(hDlg,DM_ENABLE,DlgPARTLYFULL,true);
						Info.SendDlgMessage(hDlg,DM_ENABLE,DlgLPARTLYKB,true);
						if (Info.SendDlgMessage(hDlg,DM_GETCHECK,DlgLPARTLYKB,0))
							Info.SendDlgMessage(hDlg,DM_ENABLE,DlgEPARTLYKB,true);
						else
							Info.SendDlgMessage(hDlg,DM_ENABLE,DlgEPARTLYKB,false);
					}
					else
					{
						Info.SendDlgMessage(hDlg,DM_ENABLE,DlgPARTLYFULL,false);
						Info.SendDlgMessage(hDlg,DM_ENABLE,DlgLPARTLYKB,false);
						Info.SendDlgMessage(hDlg,DM_ENABLE,DlgEPARTLYKB,false);
					}

					if (!(LPanel.bARC || RPanel.bARC))
						Info.SendDlgMessage(hDlg,DM_ENABLE,DlgIGNORE,true);
					if (Info.SendDlgMessage(hDlg,DM_GETCHECK,DlgIGNORE,0))
						Info.SendDlgMessage(hDlg,DM_ENABLE,DlgIGNORETEMPL,true);
					else
						Info.SendDlgMessage(hDlg,DM_ENABLE,DlgIGNORETEMPL,false);

					if (!(LPanel.bARC || RPanel.bARC))
						Info.SendDlgMessage(hDlg,DM_ENABLE,DlgCACHE,true);
					if (Info.SendDlgMessage(hDlg,DM_GETCHECK,DlgCACHE,0))
					{
						Info.SendDlgMessage(hDlg,DM_ENABLE,DlgCACHEIGNORE,true);
						Info.SendDlgMessage(hDlg,DM_ENABLE,DlgCACHEUSE,true);
					}
					else
					{
						Info.SendDlgMessage(hDlg,DM_ENABLE,DlgCACHEIGNORE,false);
						Info.SendDlgMessage(hDlg,DM_ENABLE,DlgCACHEUSE,false);
					}
				}
				else
				{
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgDIFFTIME,false);
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgPARTLY,false);
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgPARTLYFULL,false);
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgLPARTLYKB,false);
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgEPARTLYKB,false);
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgIGNORE,false);
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgIGNORETEMPL,false);
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgCACHE,false);
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgCACHEIGNORE,false);
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgCACHEUSE,false);
				}
			}
			//------------
			else if (Param1 == DlgPARTLY)
			{
				if (Param2)
				{
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgPARTLYFULL,true);
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgLPARTLYKB,true);
					if (Info.SendDlgMessage(hDlg,DM_GETCHECK,DlgLPARTLYKB,0))
						Info.SendDlgMessage(hDlg,DM_ENABLE,DlgEPARTLYKB,true);
					else
						Info.SendDlgMessage(hDlg,DM_ENABLE,DlgEPARTLYKB,false);
				}
				else
				{
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgPARTLYFULL,false);
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgLPARTLYKB,false);
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgEPARTLYKB,false);
				}
			}
			//------------
			else if (Param1 == DlgLPARTLYKB)
			{
				if (Param2)
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgEPARTLYKB,true);
				else
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgEPARTLYKB,false);
			}
			//------------
			else if (Param1 == DlgIGNORE)
			{
				if (Param2)
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgIGNORETEMPL,true);
				else
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgIGNORETEMPL,false);
			}
			//------------
			else if (Param1 == DlgSUBFOLDER)
			{
				if (Param2 == 2)
				{
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgLMAXDEPTH,true);
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgEMAXDEPTH,true);
				}
				else
				{
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgLMAXDEPTH,false);
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgEMAXDEPTH,false);
				}
			}
			//------------
			else if (Param1 == DlgFILTER)
			{
				if (Param2)
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgFILTERBOTTON,true);
				else
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgFILTERBOTTON,false);
			}
			//------------
			else if (Param1 == DlgLCMPSKIP)
			{
				if (Param2)
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgECMPSKIP,true);
				else
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgECMPSKIP,false);
			}
			//------------
			else if (Param1 == DlgCACHE)
			{
				if (Param2)
				{
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgCACHEIGNORE,true);
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgCACHEUSE,true);
				}
				else
				{
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgCACHEIGNORE,false);
					Info.SendDlgMessage(hDlg,DM_ENABLE,DlgCACHEUSE,false);
				}
			}
			break;


	/************************************************************************/

		case DN_CONTROLINPUT:
		{
			const INPUT_RECORD* record=(const INPUT_RECORD *)Param2;
			if (record->EventType==MOUSE_EVENT)
			{
				if (Param1==DlgSEP2_DIALOG && record->Event.MouseEvent.dwButtonState==FROM_LEFT_1ST_BUTTON_PRESSED)
				{
					if (Opt.Dialog) Opt.Dialog=0;
					else Opt.Dialog=1;
					Info.SendDlgMessage(hDlg, DM_REDRAW, 0, 0);
					return true;
				}
			}
			if (record->EventType==KEY_EVENT)
			{
				long Key=FSF.FarInputRecordToKey(record);

				if (Key == KEY_F4 && Info.SendDlgMessage(hDlg,DM_GETCHECK,DlgFILTER,0))
					Info.SendDlgMessage(hDlg,DM_CLOSE,DlgFILTERBOTTON,0);
				else if (Key == KEY_F8 && Info.SendDlgMessage(hDlg,DM_ENABLE,DlgCACHECLEAR,(INT_PTR)-1))
					Info.SendDlgMessage(hDlg,DM_CLOSE,DlgCACHECLEAR,0);
				else if (Key == KEY_F2 && Info.SendDlgMessage(hDlg,DM_ENABLE,DlgUNDERCURSOR,(INT_PTR)-1))
					Info.SendDlgMessage(hDlg,DM_CLOSE,DlgUNDERCURSOR,0);
				else if (Key == KEY_F3)
				{
					if (Opt.Dialog) Opt.Dialog=0;
					else Opt.Dialog=1;
					Info.SendDlgMessage(hDlg, DM_REDRAW, 0, 0);
					return true;
				}
			}
		}
		break;

	/************************************************************************/

		case DN_CLOSE:
			if (Param1 == DlgFILTERBOTTON)
			{
				if (Opt.hCustomFilter == INVALID_HANDLE_VALUE)
					Opt.Filter=Info.FileFilterControl(PANEL_NONE,FFCTL_CREATEFILEFILTER,FFT_CUSTOM,(INT_PTR)&Opt.hCustomFilter);
				if (Opt.hCustomFilter != INVALID_HANDLE_VALUE)
					Info.FileFilterControl(Opt.hCustomFilter,FFCTL_OPENFILTERSMENU,0,0);
				return false;
			}
			else if (Param1 == DlgCACHECLEAR)
			{
				if (Cache.RCI)
				{
					if (Info.AdvControl(&MainGuid,ACTL_GETCONFIRMATIONS,0) & FCS_DELETE)
					{
						wchar_t buf[100]; FSF.sprintf(buf,GetMsg(MClearCacheItems),Cache.ItemsNumber);
						const wchar_t *MsgItems[]={ GetMsg(MClearCacheTitle),buf,GetMsg(MClearCacheBody) };
						if (!Info.Message(&MainGuid, FMSG_WARNING|FMSG_MB_YESNO, 0, MsgItems, 3, 0))
							goto ClearCache;
					}
					else
					ClearCache:
					{
						free(Cache.RCI);
						Cache.RCI=0;
						Cache.ItemsNumber=0;
						Info.SendDlgMessage(hDlg,DM_ENABLE,DlgCACHECLEAR,false);
					}
				}
				return false;
			}
			else if (Param1 == DlgUNDERCURSOR)
			{
				return true;
			}
			else if (Param1 == DlgOK)
			{
				for (int i=DlgCMPCASE; i<(sizeof(StoreOpt)/sizeof(StoreOpt[0]))-1; i++)
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

				Opt.Substr=NULL;
				int len=Info.SendDlgMessage(hDlg,DM_GETTEXTPTR,DlgECMPSKIP,0);
				if (len)
				{
					Opt.Substr=(wchar_t*)realloc(Opt.Substr,(len+1)*sizeof(wchar_t));
					if (Opt.Substr)
						Info.SendDlgMessage(hDlg,DM_GETTEXTPTR,DlgECMPSKIP,(INT_PTR)Opt.Substr);
					else
						Opt.SkipSubstr=0;
				}
				else
				{
					Opt.SkipSubstr=0;
					free(Opt.Substr);
				}

				if (Opt.Filter && Opt.hCustomFilter == INVALID_HANDLE_VALUE)
					Opt.Filter=Info.FileFilterControl(PANEL_NONE,FFCTL_CREATEFILEFILTER,FFT_CUSTOM,(INT_PTR)&Opt.hCustomFilter);

				FarSettingsCreate settings={sizeof(FarSettingsCreate),MainGuid,INVALID_HANDLE_VALUE};
				if (Info.SettingsControl(INVALID_HANDLE_VALUE,SCTL_CREATE,0,(INT_PTR)&settings))
				{
					int Root=0; // корень ключа
					for (int i=DlgCMPCASE; i<sizeof(StoreOpt)/sizeof(StoreOpt[0]); i++)
					{
						if (StoreOpt[i].RegName && Info.SendDlgMessage(hDlg,DM_ENABLE,i,(INT_PTR)-1))
						{
							FarSettingsItem item={Root,StoreOpt[i].RegName,FST_QWORD};
							item.Number=*StoreOpt[i].Option;
							Info.SettingsControl(settings.Handle,SCTL_SET,0,(INT_PTR)&item);
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
	const unsigned int dW = 60;   // ширина
	const unsigned int dH = 25;   // высота

	struct FarDialogItem DialogItems[] = {
		//			Type	X1	Y1	X2	Y2		Selected	History					Mask															Flags	Data	MaxLen	UserParam
		/* 0*/{DI_DOUBLEBOX,  0, 0,  dW,  dH, 0, 0,                   0,                                0, GetMsg(MCompareTitle),0,0},
		/* 1*/{DI_TEXT,       2, 1,   0,   0, 0, 0,                   0,                                0, GetMsg(MCompareBox),0,0},
		/* 2*/{DI_CHECKBOX,   2, 2,   0,   0, 0, 0,                   0,                                0, GetMsg(MCompareCase),0,0},
		/* 3*/{DI_CHECKBOX,   2, 3,   0,   0, 1, 0,                   0,                                0, GetMsg(MCompareSize),0,0},
		/* 4*/{DI_CHECKBOX,   2, 4,   0,   0, 1, 0,                   0,                                0, GetMsg(MCompareTime),0,0},
		/* 5*/{DI_CHECKBOX,   6, 5,   0,   0, 0, 0,                   0,                                0, GetMsg(MCompareLowPrecision),0,0},
		/* 6*/{DI_CHECKBOX,   6, 6,   0,   0, 0, 0,                   0,                                0, GetMsg(MCompareIgnoreTimeZone),0,0},
		/* 7*/{DI_CHECKBOX,   2, 7,   0,   0, 0, 0,                   0,                                0, GetMsg(MCompareContents),0,0},
		/* 8*/{DI_CHECKBOX,   6, 8,   0,   0, 0, 0,                   0,                                0, GetMsg(MCompareOnlyTimeDiff),0,0},
		/* 9*/{DI_CHECKBOX,   6, 9,   0,   0, 0, 0,                   0,                                0, GetMsg(MComparePartly),0,0},
		/*10*/{DI_RADIOBUTTON,0, 9,  33,   0, 1, 0,                   0,                        DIF_GROUP, GetMsg(MComparePartlyFull),0,0},
		/*11*/{DI_RADIOBUTTON,35,9,  41,   0, 0, 0,                   0,                                0, GetMsg(MComparePartlyKb),0,0},
		/*12*/{DI_FIXEDIT,   43, 9,dW-3,   0, 0, 0,       L"##########",                     DIF_MASKEDIT, L"64",0,0},
		/*13*/{DI_CHECKBOX,   6,10,  22,   0, 0, 0,                   0,                                0, GetMsg(MCompareIgnore),0,0},
		/*14*/{DI_COMBOBOX,   0,10,dW-3,   0, 0, 0,                   0,DIF_LISTAUTOHIGHLIGHT|DIF_DROPDOWNLIST|DIF_LISTWRAPMODE, L"",0,0},

		/*15*/{DI_TEXT,      -1,11,   0,   0, 0, 0,                   0,                    DIF_SEPARATOR, GetMsg(MTitleOptions),0,0},
		/*16*/{DI_CHECKBOX,   2,12,   0,   0, 0, 0,                   0,                       DIF_3STATE, GetMsg(MProcessSubfolders),0,0},
		/*17*/{DI_TEXT,       0,12,   0,   0, 0, 0,                   0,                                0, GetMsg(MMaxScanDepth),0,0},
		/*18*/{DI_FIXEDIT,    0,12,   2,   0, 0, 0,              L"999",                     DIF_MASKEDIT, L"1",0,0},
		/*19*/{DI_CHECKBOX,   2,13,   0,   0, 0, 0,                   0,                                0, GetMsg(MFilter),0,0},
		/*20*/{DI_BUTTON,     0,13,dW-3,   0, 0, 0,                   0,                                0, GetMsg(MFilterBotton),0,0},
		/*21*/{DI_CHECKBOX,   2,14,   0,   0, 0, 0,                   0,                                0, GetMsg(MProcessSelected),0,0},
		/*22*/{DI_CHECKBOX,   2,15,   0,   0, 0, 0,                   0,                                0, GetMsg(MCompareSkipSubstr),0,0},
		/*23*/{DI_EDIT,       0,15,dW-3,   0, 0, L"AdvCmpSkipSubstr", 0,   DIF_USELASTHISTORY|DIF_HISTORY, L"",0,0},
		/*24*/{DI_CHECKBOX,   2,16,   0,   0, 0, 0,                   0,                                0, GetMsg(MIgnoreMissing),0,0},
		/*25*/{DI_CHECKBOX,   2,17,   0,   0, 1, 0,                   0,                                0, GetMsg(MProcessTillFirstDiff),0,0},
		/*26*/{DI_CHECKBOX,   2,18,   0,   0, 1, 0,                   0,                                0, GetMsg(MSelectedNew),0,0},
		/*27*/{DI_CHECKBOX,   2,19,   0,   0, 0, 0,                   0,                                0, GetMsg(MCache),0,0},
		/*28*/{DI_RADIOBUTTON,0,19,  27,   0, 0, 0,                   0,                        DIF_GROUP, GetMsg(MCacheIgnore),0,0},
		/*29*/{DI_RADIOBUTTON,28,19, 44,   0, 1, 0,                   0,                                0, GetMsg(MCacheUse),0,0},
		/*30*/{DI_BUTTON,     0,19,dW-3,   0, 0, 0,                   0,                                0, GetMsg(MCacheClearBotton),0,0},
		/*31*/{DI_CHECKBOX,   2,20,   0,   0, 1, 0,                   0,                                0, GetMsg(MShowMsg),0,0},
		/*32*/{DI_CHECKBOX,  28,20,   0,   0, 0, 0,                   0,                                0, GetMsg(MSound),0,0},
		/*33*/{DI_CHECKBOX,   2,21,   0,   0, 0, 0,                   0,                                0, GetMsg(MTotalProcess),0,0},

		/*34*/{DI_TEXT,      -1,22,   0,   0, 0, 0,                   0,                    DIF_SEPARATOR, L"",0,0},
		/*35*/{DI_BUTTON,     0,23,   0,   0, 0, 0,                   0,DIF_CENTERGROUP|DIF_DEFAULTBUTTON, GetMsg(MOK),0,0},
		/*36*/{DI_BUTTON,     0,23,   0,   0, 0, 0,                   0,                  DIF_CENTERGROUP, GetMsg(MUnderCursorBotton),0,0},
		/*37*/{DI_BUTTON,     0,23,   0,   0, 0, 0,                   0,                  DIF_CENTERGROUP, GetMsg(MCancel),0,0}
	};

	// динамические координаты для строк
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
	*StoreOpt[DlgSEP2_DIALOG].Option=0;

	FarSettingsCreate settings={sizeof(FarSettingsCreate),MainGuid,INVALID_HANDLE_VALUE};
	if (Info.SettingsControl(INVALID_HANDLE_VALUE,SCTL_CREATE,0,(INT_PTR)&settings))
	{
		int Root=0; // корень ключа
		for (int i=DlgCMPCASE; i<sizeof(StoreOpt)/sizeof(StoreOpt[0]); i++)
		{
			FarSettingsItem item={Root,StoreOpt[i].RegName,FST_QWORD};
			if (StoreOpt[i].RegName && Info.SettingsControl(settings.Handle,SCTL_GET,0,(INT_PTR)&item))
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
                                (INT_PTR) this );

	int ExitCode=-1;

	if (hDlg != INVALID_HANDLE_VALUE)
	{
		ExitCode=Info.DialogRun(hDlg);
		Info.DialogFree(hDlg);
	}
	return ExitCode;
}
