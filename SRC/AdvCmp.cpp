/****************************************************************************
 * AdvCmp.cpp
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

#include "AdvCmp.hpp"
#include "AdvCmpDlgOpt.hpp"
#include "AdvCmpProc.hpp"

/****************************************************************************
 * Копии стандартных структур FAR
 ****************************************************************************/
struct PluginStartupInfo Info;
struct FarStandardFunctions FSF;

/****************************************************************************
 * Набор переменных
 ****************************************************************************/
struct Options Opt;                 //Текущие настройки плагина
struct CacheCmp Cache;              //Кеш сравнения "по содержимому"
struct FarPanelInfo LPanel,RPanel;
struct TotalCmpInfo CmpInfo;
struct FarWindowsInfo WinInfo;
bool bBrokenByEsc;
bool bGflLoaded=false;
HMODULE GflHandle=NULL;
HANDLE hConInp=INVALID_HANDLE_VALUE;


/****************************************************************************
 * Обёртка сервисной функции FAR: получение строки из .lng-файла
 ****************************************************************************/
const wchar_t *GetMsg(int MsgId)
{
	return Info.GetMsg(&MainGuid,MsgId);
}

/****************************************************************************
 * Показ предупреждения-ошибки с заголовком и одной строчкой
 ****************************************************************************/
void ErrorMsg(DWORD Title, DWORD Body)
{
	const wchar_t *MsgItems[]={ GetMsg(Title), GetMsg(Body), GetMsg(MOK) };
	Info.Message(&MainGuid,&ErrorMsgGuid,FMSG_WARNING,0,MsgItems,3,1);
}

/****************************************************************************
 * Показ предупреждения "Yes-No" с заголовком и одной строчкой
 ****************************************************************************/
bool YesNoMsg(DWORD Title, DWORD Body)
{
	const wchar_t *MsgItems[]={ GetMsg(Title), GetMsg(Body) };
	return (!Info.Message(&MainGuid,&YesNoMsgGuid,FMSG_WARNING|FMSG_MB_YESNO,0,MsgItems,2,0));
}

// Сообщение для отладки
int DebugMsg(wchar_t *msg, wchar_t *msg2, unsigned int i)
{
  wchar_t *MsgItems[] = {L"DebugMsg", L"", L"", L""};
  wchar_t buf[80]; FSF.itoa(i, buf,10);
  MsgItems[1] = msg2;
  MsgItems[2] = msg;
  MsgItems[3] = buf;
  return (!Info.Message(&MainGuid,&DebugMsgGuid,FMSG_WARNING|FMSG_MB_OKCANCEL,0,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]),2));
}


void FreeDirList(struct DirList *pList)
{
	if (pList->PPI)
	{
		for (int i=0; i<pList->ItemsNumber; i++)
			free((void*)pList->PPI[i].FileName);
		free(pList->PPI); pList->PPI=0;
	}
	free(pList->Dir); pList->Dir=0;
	pList->ItemsNumber=0;
}

/****************************************************************************
 * Динамическая загрузка необходимых dll
 ****************************************************************************/

///  VisComp.dll
PCOMPAREFILES pCompareFiles=NULL;

///  libgfl340.dll
PGFLLIBRARYINIT pGflLibraryInit=NULL;
PGFLENABLELZW pGflEnableLZW=NULL;
PGFLLIBRARYEXIT pGflLibraryExit=NULL;
PGFLLOADBITMAPW pGflLoadBitmapW=NULL;
PGFLGETNUMBEROFFORMAT pGflGetNumberOfFormat=NULL;
PGFLGETFORMATINFORMATIONBYINDEX pGflGetFormatInformationByIndex=NULL;
PGFLGETDEFAULTLOADPARAMS pGflGetDefaultLoadParams=NULL;
PGFLCHANGECOLORDEPTH pGflChangeColorDepth=NULL;
PGFLROTATE pGflRotate=NULL;
PGFLRESIZE pGflResize=NULL;
PGFLFREEBITMAP pGflFreeBitmap=NULL;
PGFLFREEFILEINFORMATION pGflFreeFileInformation=NULL;


bool FindFile(wchar_t *Dir, wchar_t *Pattern, string &strFileName)
{
	string strPathMask(Dir);
	strPathMask+=L"\\*";
	WIN32_FIND_DATA wfdFindData;
	HANDLE hFind;
	bool ret=false;

	if ((hFind=FindFirstFileW(strPathMask,&wfdFindData)) != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (wfdFindData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
			{
				if ((wfdFindData.cFileName[0]==L'.' && !wfdFindData.cFileName[1]) || (wfdFindData.cFileName[1]==L'.' && !wfdFindData.cFileName[2]))
					continue;
				strPathMask=Dir;
				if (strPathMask.length()>0 && strPathMask[(size_t)(strPathMask.length()-1)]!=L'\\') strPathMask+=L"\\";
				strPathMask+=wfdFindData.cFileName;
				if (FindFile(strPathMask.get(),Pattern,strFileName))
				{
					ret=true;
					break;
				}
			}
			else
			{
				if (!FSF.LStricmp(wfdFindData.cFileName,Pattern))
				{
					strFileName=Dir;
					if (strFileName.length()>0 && strFileName[(size_t)(strFileName.length()-1)]!=L'\\') strFileName+=L"\\";
					strFileName+=wfdFindData.cFileName;
					ret=true;
					break;
				}
			}
		} while (FindNextFile(hFind,&wfdFindData));
		FindClose(hFind);
	}
	return ret;
}

bool LoadVisComp(wchar_t *PlugPath)
{
	if (pCompareFiles) return true;

	string strPatchVisComp;
	if (FindFile(PlugPath,L"VisComp.dll",strPatchVisComp) &&
			Info.PluginsControl(INVALID_HANDLE_VALUE,PCTL_FORCEDLOADPLUGIN,PLT_PATH,strPatchVisComp.get()))
	{
		pCompareFiles=(PCOMPAREFILES)GetProcAddress(GetModuleHandleW(L"VisComp.dll"),"CompareFiles");
		return true;
	}
	return false;
}

bool UnLoadGfl()
{
	if (bGflLoaded)
	{
		pGflLibraryExit();
		if (FreeLibrary(GflHandle))
			bGflLoaded=false;
		else
			return false;
	}
	return true;
}

bool LoadGfl(wchar_t *PlugPath)
{
	if (bGflLoaded) return true;

	string strPatchGfl;
	if (FindFile(PlugPath,L"libgfl340.dll",strPatchGfl))
	{
		if (!(GflHandle=LoadLibrary(strPatchGfl)))
			return false;
		if (!pGflLibraryInit)
			pGflLibraryInit=(PGFLLIBRARYINIT)GetProcAddress(GflHandle,"gflLibraryInit");
		if (!pGflEnableLZW)
			pGflEnableLZW=(PGFLENABLELZW)GetProcAddress(GflHandle,"gflEnableLZW");
		if (!pGflLibraryExit)
			pGflLibraryExit=(PGFLLIBRARYEXIT)GetProcAddress(GflHandle,"gflLibraryExit");
		if (!pGflLoadBitmapW)
			pGflLoadBitmapW=(PGFLLOADBITMAPW)GetProcAddress(GflHandle,"gflLoadBitmapW");
		if (!pGflGetNumberOfFormat)
			pGflGetNumberOfFormat=(PGFLGETNUMBEROFFORMAT)GetProcAddress(GflHandle,"gflGetNumberOfFormat");
		if (!pGflGetFormatInformationByIndex)
			pGflGetFormatInformationByIndex=(PGFLGETFORMATINFORMATIONBYINDEX)GetProcAddress(GflHandle,"gflGetFormatInformationByIndex");
		if (!pGflGetDefaultLoadParams)
			pGflGetDefaultLoadParams=(PGFLGETDEFAULTLOADPARAMS)GetProcAddress(GflHandle,"gflGetDefaultLoadParams");
		if (!pGflChangeColorDepth)
			pGflChangeColorDepth=(PGFLCHANGECOLORDEPTH)GetProcAddress(GflHandle,"gflChangeColorDepth");
		if (!pGflRotate)
			pGflRotate=(PGFLROTATE)GetProcAddress(GflHandle,"gflRotate");
		if (!pGflResize)
			pGflResize=(PGFLRESIZE)GetProcAddress(GflHandle,"gflResize");
		if (!pGflFreeBitmap)
			pGflFreeBitmap=(PGFLFREEBITMAP)GetProcAddress(GflHandle,"gflFreeBitmap");
		if (!pGflFreeFileInformation)
			pGflFreeFileInformation=(PGFLFREEFILEINFORMATION)GetProcAddress(GflHandle,"gflFreeFileInformation");

		if (!pGflLibraryInit || !pGflEnableLZW || !pGflLibraryExit || !pGflLoadBitmapW || !pGflGetNumberOfFormat || !pGflGetFormatInformationByIndex ||
				!pGflGetDefaultLoadParams || !pGflChangeColorDepth || !pGflRotate || !pGflResize || !pGflFreeBitmap || !pGflFreeFileInformation)
			return false;

		bGflLoaded=true;

		if (pGflLibraryInit()!=GFL_NO_ERROR)
			UnLoadGfl();
		if (bGflLoaded)
			pGflEnableLZW(GFL_TRUE);
	}
	return bGflLoaded;
}


/****************************************************************************
 ***************************** Exported functions ***************************
 ****************************************************************************/


/****************************************************************************
 * Эти функции плагина FAR вызывает в первую очередь
 ****************************************************************************/
void WINAPI GetGlobalInfoW(struct GlobalInfo *Info)
{
	Info->StructSize=sizeof(GlobalInfo);
	Info->MinFarVersion=FARMANAGERVERSION;
	Info->Version=MAKEFARVERSION(3,0,0,34,VS_RC);
	Info->Guid=MainGuid;
	Info->Title=L"Advanced compare 2";
	Info->Description=L"Advanced compare 2 plugin for Far Manager v3.0";
	Info->Author=L"Alexey Samlyukov";
}

// заполним структуру PluginStartupInfo и сделаем ряд полезных действий...
void WINAPI SetStartupInfoW(const struct PluginStartupInfo *Info)
{
	::Info = *Info;
	if (Info->StructSize >= sizeof(PluginStartupInfo))
	{
		FSF = *Info->FSF;
		::Info.FSF = &FSF;

		// обнулим кэш (туда будем помещать результаты сравнения)
		memset(&Cache,0,sizeof(Cache));

		wchar_t PlugPath[MAX_PATH];
		ExpandEnvironmentStringsW(L"%FARHOME%\\Plugins",PlugPath,(sizeof(PlugPath)/sizeof(PlugPath[0])));
//		LoadVisComp(PlugPath);
		LoadGfl(PlugPath);
	}
}


/****************************************************************************
 * Эту функцию плагина FAR вызывает во вторую очередь - заполним PluginInfo, т.е.
 * скажем FARу какие пункты добавить в "Plugin commands" и "Plugins configuration".
 ****************************************************************************/
void WINAPI GetPluginInfoW(struct PluginInfo *Info)
{
	static const wchar_t *PluginMenuStrings[1];
	PluginMenuStrings[0] = GetMsg(MCompareTitle);

	Info->StructSize=sizeof(PluginInfo);
	Info->PluginMenu.Guids=&MenuGuid;
	Info->PluginMenu.Strings=PluginMenuStrings;
	Info->PluginMenu.Count=sizeof(PluginMenuStrings)/sizeof(PluginMenuStrings[0]);
}


/****************************************************************************
 * Основная функция плагина. FAR её вызывает, когда пользователь зовёт плагин
 ****************************************************************************/
HANDLE WINAPI OpenW(const struct OpenInfo *OInfo)
{
	HANDLE hPanel = INVALID_HANDLE_VALUE;
	struct PanelInfo PInfo; 
	PInfo.StructSize=sizeof(PanelInfo);

	// Если не удалось запросить информацию об активной панели...
	if (!Info.PanelControl(PANEL_ACTIVE,FCTL_GETPANELINFO,0,&PInfo))
		return hPanel;
	if (PInfo.Flags & PFLAGS_PANELLEFT)
	{
		LPanel.PInfo.StructSize=sizeof(PanelInfo);
		Info.PanelControl(PANEL_ACTIVE,FCTL_GETPANELINFO,0,&LPanel.PInfo);
		LPanel.hPanel=PANEL_ACTIVE;
	}
	else
	{
		RPanel.PInfo.StructSize=sizeof(PanelInfo);
		Info.PanelControl(PANEL_ACTIVE,FCTL_GETPANELINFO,0,&RPanel.PInfo);
		RPanel.hPanel=PANEL_ACTIVE;
	}
	// Если не удалось запросить информацию об пассивной панели...
	if (!Info.PanelControl(PANEL_PASSIVE,FCTL_GETPANELINFO,0,&PInfo))
		return hPanel;
	if (PInfo.Flags & PFLAGS_PANELLEFT)
	{
		LPanel.PInfo.StructSize=sizeof(PanelInfo);
		Info.PanelControl(PANEL_PASSIVE,FCTL_GETPANELINFO,0,&LPanel.PInfo);
		LPanel.hPanel=PANEL_PASSIVE;
	}
	else
	{
		RPanel.PInfo.StructSize=sizeof(PanelInfo);
		Info.PanelControl(PANEL_PASSIVE,FCTL_GETPANELINFO,0,&RPanel.PInfo);
		RPanel.hPanel=PANEL_PASSIVE;
	}

	// Если панели нефайловые...
	if (LPanel.PInfo.PanelType != PTYPE_FILEPANEL || RPanel.PInfo.PanelType != PTYPE_FILEPANEL)
	{
		ErrorMsg(MCompareTitle, MFilePanelsRequired);
		return hPanel;
	}

	LPanel.bTMP=LPanel.bARC=LPanel.bCurFile=LPanel.bDir=false;
	RPanel.bTMP=RPanel.bARC=RPanel.bCurFile=RPanel.bDir=false;
	struct DirList LList, RList;

	if (LPanel.PInfo.ItemsNumber)
	{
		LList.ItemsNumber=LPanel.PInfo.ItemsNumber;
		LList.PPI=(PluginPanelItem*)malloc(LList.ItemsNumber*sizeof(PluginPanelItem));
		if (LList.PPI)
		{
			FarGetPluginPanelItem FGPPI;
			for (int i=0; i<LList.ItemsNumber; i++)
			{
				FGPPI.Size=0; FGPPI.Item=0;
				FGPPI.Item=(PluginPanelItem*)malloc(FGPPI.Size=Info.PanelControl(LPanel.hPanel,FCTL_GETPANELITEM,i,&FGPPI));
				if (FGPPI.Item)
				{
					Info.PanelControl(LPanel.hPanel,FCTL_GETPANELITEM,i,&FGPPI);
					LList.PPI[i]=*(FGPPI.Item);
					LList.PPI[i].FileName=(wchar_t*)malloc((wcslen(FGPPI.Item->FileName)+1)*sizeof(wchar_t));
					if (LList.PPI[i].FileName) wcscpy((wchar_t*)LList.PPI[i].FileName,FGPPI.Item->FileName);
					{
						if (!LPanel.bARC && (LPanel.PInfo.Flags&PFLAGS_PLUGIN) && FGPPI.Item->CRC32)
							LPanel.bARC=true;
						if (!LPanel.bTMP && ((LPanel.PInfo.Flags&PFLAGS_PLUGIN) && (LPanel.PInfo.Flags&PFLAGS_REALNAMES)) && wcspbrk(FGPPI.Item->FileName,L":\\/"))
							LPanel.bTMP=true;
						if (i==LPanel.PInfo.CurrentItem && !(FGPPI.Item->FileAttributes&FILE_ATTRIBUTE_DIRECTORY))
							LPanel.bCurFile=true;
						if (!LPanel.bDir && !LPanel.bTMP && (FGPPI.Item->FileAttributes&FILE_ATTRIBUTE_DIRECTORY) && !(FGPPI.Item->FileName[1]==L'.' && !FGPPI.Item->FileName[2]))
							LPanel.bDir=true;
					}
					free(FGPPI.Item);
				}
			}
		}
		if (!LPanel.bARC)
			LPanel.bARC=LPanel.PInfo.Flags&PFLAGS_USECRC32;
	}
	else
	{
		LList.ItemsNumber=0;
		LList.PPI=NULL;
	}
	{
		int size=Info.PanelControl(LPanel.hPanel,FCTL_GETPANELDIRECTORY,0,0);
		if (size)
		{
			FarPanelDirectory *buf=(FarPanelDirectory*)malloc(size);
			if (buf)
			{
				Info.PanelControl(LPanel.hPanel,FCTL_GETPANELDIRECTORY,size,buf);
				wcscpy(LPanel.Dir,buf->Name);
				if (!(LPanel.PInfo.Flags&PFLAGS_PLUGIN))
				{
					size=FSF.ConvertPath(CPM_NATIVE,buf->Name,0,0);
					LList.Dir=(wchar_t*)malloc(size*sizeof(wchar_t));
					if (LList.Dir) FSF.ConvertPath(CPM_NATIVE,buf->Name,LList.Dir,size);
				}
				else
				{
					LList.Dir=(wchar_t*)malloc((wcslen(buf->Name)+1)*sizeof(wchar_t));
					if (LList.Dir) wcscpy(LList.Dir,buf->Name);
				}
				free(buf);
			}
		}
	}

	if (RPanel.PInfo.ItemsNumber)
	{
		RList.ItemsNumber=RPanel.PInfo.ItemsNumber;
		RList.PPI=(PluginPanelItem*)malloc(RList.ItemsNumber*sizeof(PluginPanelItem));
		if (RList.PPI)
		{
			FarGetPluginPanelItem FGPPI;
			for (int i=0; i<RList.ItemsNumber; i++)
			{
				FGPPI.Size=0; FGPPI.Item=0;
				FGPPI.Item=(PluginPanelItem*)malloc(FGPPI.Size=Info.PanelControl(RPanel.hPanel,FCTL_GETPANELITEM,i,&FGPPI));
				if (FGPPI.Item)
				{
					Info.PanelControl(RPanel.hPanel,FCTL_GETPANELITEM,i,&FGPPI);
					RList.PPI[i]=*(FGPPI.Item);
					RList.PPI[i].FileName=(wchar_t*)malloc((wcslen(FGPPI.Item->FileName)+1)*sizeof(wchar_t));
					if (RList.PPI[i].FileName) wcscpy((wchar_t*)RList.PPI[i].FileName,FGPPI.Item->FileName);
					{
						if (!RPanel.bARC && (RPanel.PInfo.Flags&PFLAGS_PLUGIN) && FGPPI.Item->CRC32)
							RPanel.bARC=true;
						if (!RPanel.bTMP && ((RPanel.PInfo.Flags&PFLAGS_PLUGIN) && (RPanel.PInfo.Flags&PFLAGS_REALNAMES)) && wcspbrk(FGPPI.Item->FileName,L":\\/"))
							RPanel.bTMP=true;
						if (i==RPanel.PInfo.CurrentItem && !(FGPPI.Item->FileAttributes&FILE_ATTRIBUTE_DIRECTORY))
							RPanel.bCurFile=true;
						if (!RPanel.bDir && !RPanel.bTMP && (FGPPI.Item->FileAttributes&FILE_ATTRIBUTE_DIRECTORY) && !(FGPPI.Item->FileName[1]==L'.' && !FGPPI.Item->FileName[2]))
							RPanel.bDir=true;
					}
					free(FGPPI.Item);
				}
			}
		}
		if (!RPanel.bARC)
			RPanel.bARC=RPanel.PInfo.Flags&PFLAGS_USECRC32;
	}
	else
	{
		RList.ItemsNumber=0;
		RList.PPI=NULL;
	}
	{
		int size=Info.PanelControl(RPanel.hPanel,FCTL_GETPANELDIRECTORY,0,0);
		if (size)
		{
			FarPanelDirectory *buf=(FarPanelDirectory*)malloc(size);
			if (buf)
			{
				Info.PanelControl(RPanel.hPanel,FCTL_GETPANELDIRECTORY,size,buf);
				wcscpy(RPanel.Dir,buf->Name);
				if (!(RPanel.PInfo.Flags&PFLAGS_PLUGIN))
				{
					size=FSF.ConvertPath(CPM_NATIVE,buf->Name,0,0);
					RList.Dir=(wchar_t*)malloc(size*sizeof(wchar_t));
					if (RList.Dir) FSF.ConvertPath(CPM_NATIVE,buf->Name,RList.Dir,size);
				}
				else
				{
					RList.Dir=(wchar_t*)malloc((wcslen(buf->Name)+1)*sizeof(wchar_t));
					if (RList.Dir) wcscpy(RList.Dir,buf->Name);
				}
				free(buf);
			}
		}
	}

	WinInfo.hFarWindow=(HWND)Info.AdvControl(&MainGuid,ACTL_GETFARHWND,0,0);
	GetClientRect(WinInfo.hFarWindow,&WinInfo.Win);
	if (Info.AdvControl(&MainGuid,ACTL_GETFARRECT,0,&WinInfo.Con))
	{
		WinInfo.TruncLen=WinInfo.Con.Right-WinInfo.Con.Left-20+1;
		if (WinInfo.TruncLen>MAX_PATH-2) WinInfo.TruncLen=MAX_PATH-2;
	}
	else
	{
		WinInfo.Con.Left=0;
		WinInfo.Con.Top=0;
		WinInfo.Con.Right=79;
		WinInfo.Con.Bottom=24;
		WinInfo.TruncLen=60;
	}

	memset(&CmpInfo,0,sizeof(CmpInfo));
	bBrokenByEsc=false;

	class AdvCmpDlgOpt AdvCmpOpt;
	int ret=AdvCmpOpt.ShowOptDialog();

	if (ret==42 || ret==43) // DlgOK || DlgUNDERCURSOR
	{
		DWORD dwTicks=GetTickCount();
		// откроем, для проверок на Esc
		hConInp=CreateFileW(L"CONIN$", GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);

		class AdvCmpProc AdvCmp;

		if (ret==42)
		{
			bool bDifferenceNotFound=AdvCmp.CompareDirs(&LList,&RList,true,0);

			if (hConInp!=INVALID_HANDLE_VALUE) CloseHandle(hConInp);

			// Отмечаем файлы и перерисовываем панели. Если нужно показываем сообщение...
			if (!bBrokenByEsc)
			{
				{
					Info.PanelControl(LPanel.hPanel,FCTL_BEGINSELECTION,0,0);
					Info.PanelControl(RPanel.hPanel,FCTL_BEGINSELECTION,0,0);

					for (int i=0; i<LList.ItemsNumber; i++)
						Info.PanelControl(LPanel.hPanel,FCTL_SETSELECTION,i,(void*)(LList.PPI[i].Flags&PPIF_SELECTED));
					for (int i=0; i<RList.ItemsNumber; i++)
						Info.PanelControl(RPanel.hPanel,FCTL_SETSELECTION,i,(void*)(RList.PPI[i].Flags&PPIF_SELECTED));

					Info.PanelControl(LPanel.hPanel,FCTL_ENDSELECTION,0,0);
					Info.PanelControl(LPanel.hPanel,FCTL_REDRAWPANEL,0,0);
					Info.PanelControl(RPanel.hPanel,FCTL_ENDSELECTION,0,0);
					Info.PanelControl(RPanel.hPanel,FCTL_REDRAWPANEL,0,0);
				}

				if (Opt.Sound && (GetTickCount()-dwTicks > 30000)) MessageBeep(MB_ICONASTERISK);
				Info.AdvControl(&MainGuid,ACTL_PROGRESSNOTIFY,0,0);
				if (CmpInfo.Errors && Opt.ShowMsg) ErrorMsg(MOpenErrorTitle,MOpenErrorBody);
				if (bDifferenceNotFound && Opt.ShowMsg)
				{
					const wchar_t *MsgItems[] = { GetMsg(MNoDiffTitle), GetMsg(MNoDiffBody), GetMsg(MOK) };
					Info.Message(&MainGuid,&NoDiffMsgGuid,0,0,MsgItems,sizeof(MsgItems) / sizeof(MsgItems[0]),1);
				}
				else if (!bDifferenceNotFound && Opt.Dialog)
					AdvCmp.ShowCmpDialog(&LList,&RList);
			}
		}
		else
		{
			if (hConInp!=INVALID_HANDLE_VALUE) CloseHandle(hConInp);
			AdvCmp.CompareCurFile(&LList,&RList);
		}
	}

	FreeDirList(&LList);
	FreeDirList(&RList);

	return hPanel;
}

/****************************************************************************
 * Эту функцию FAR вызывает перед выгрузкой плагина
 ****************************************************************************/
void WINAPI ExitFARW(const struct ExitInfo *Info)
{
	//Освободим память в случае выгрузки плагина
	if (Cache.RCI)
		free(Cache.RCI);
	Cache.RCI=0;
	Cache.ItemsNumber=0;

	UnLoadGfl();
}
