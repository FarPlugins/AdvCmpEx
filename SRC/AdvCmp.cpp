/****************************************************************************
 * AdvCmp.cpp
 *
 * Plugin module for Far Manager 3.0
 *
 * Copyright (c) 2006-2012 Alexey Samlyukov
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
bool bStartMsg;
bool bGflLoaded=false;
bool bBASSLoaded=false;   // bass.dll загружена?
HMODULE GflHandle=NULL;
HMODULE BASSHandle=NULL;
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

__int64 GetFarSetting(FARSETTINGS_SUBFOLDERS Root,const wchar_t* Name)
{
	__int64 result=0;
	FarSettingsCreate settings={sizeof(FarSettingsCreate),FarGuid,INVALID_HANDLE_VALUE};
	HANDLE Settings=Info.SettingsControl(INVALID_HANDLE_VALUE,SCTL_CREATE,0,&settings)?settings.Handle:0;
	if (Settings)
	{
		FarSettingsItem item={Root,Name,FST_UNKNOWN,{0}};
		if(Info.SettingsControl(Settings,SCTL_GET,0,&item)&&FST_QWORD==item.Type)
		{
			result=item.Number;
		}
		Info.SettingsControl(Settings,SCTL_FREE,0,0);
	}
	return result;
}

void GetDirList(FarPanelInfo &CurPanel, DirList &CurList)
{
	if (CurPanel.PInfo.ItemsNumber)
	{
		CurList.ItemsNumber=CurPanel.PInfo.ItemsNumber;
		CurList.PPI=(PluginPanelItem*)malloc(CurList.ItemsNumber*sizeof(PluginPanelItem));
		if (CurList.PPI)
		{
			FarGetPluginPanelItem FGPPI;
			for (int i=0; i<CurList.ItemsNumber; i++)
			{
				FGPPI.Item=(PluginPanelItem*)malloc(FGPPI.Size=Info.PanelControl(CurPanel.hPanel,FCTL_GETPANELITEM,i,0));
				if (FGPPI.Item)
				{
					Info.PanelControl(CurPanel.hPanel,FCTL_GETPANELITEM,i,&FGPPI);
					PluginPanelItem &CurrentPluginItem=(CurList.PPI[i]);
					CurrentPluginItem.FileAttributes=FGPPI.Item->FileAttributes;
					CurrentPluginItem.LastAccessTime=FGPPI.Item->LastAccessTime;
					CurrentPluginItem.LastWriteTime=FGPPI.Item->LastWriteTime;
					CurrentPluginItem.FileSize=FGPPI.Item->FileSize;
					CurrentPluginItem.CRC32=FGPPI.Item->CRC32;
					CurrentPluginItem.Flags=FGPPI.Item->Flags;
					CurrentPluginItem.FileName=(wchar_t*)malloc((wcslen(FGPPI.Item->FileName)+1)*sizeof(wchar_t));
					if (CurrentPluginItem.FileName) wcscpy((wchar_t*)CurrentPluginItem.FileName,FGPPI.Item->FileName);

					if (!CurPanel.bARC && (CurPanel.PInfo.Flags&PFLAGS_PLUGIN) && FGPPI.Item->CRC32)
						CurPanel.bARC=true;
					if (!CurPanel.bTMP && ((CurPanel.PInfo.Flags&PFLAGS_PLUGIN) && (CurPanel.PInfo.Flags&PFLAGS_REALNAMES)) && wcspbrk(FGPPI.Item->FileName,L":\\/"))
						CurPanel.bTMP=true;
					if (i==CurPanel.PInfo.CurrentItem && !(FGPPI.Item->FileAttributes&FILE_ATTRIBUTE_DIRECTORY))
						CurPanel.bCurFile=true;
					if (!CurPanel.bDir && !CurPanel.bTMP && (FGPPI.Item->FileAttributes&FILE_ATTRIBUTE_DIRECTORY) && !(FGPPI.Item->FileName[1]==L'.' && !FGPPI.Item->FileName[2]))
						CurPanel.bDir=true;

					free(FGPPI.Item);
				}
			}
		}
		if (!CurPanel.bARC)
			CurPanel.bARC=(CurPanel.PInfo.Flags&PFLAGS_USECRC32?true:false);
	}
	else
	{
		CurList.ItemsNumber=0;
		CurList.PPI=NULL;
	}

	int size=Info.PanelControl(CurPanel.hPanel,FCTL_GETPANELDIRECTORY,0,0);
	if (size)
	{
		FarPanelDirectory *buf=(FarPanelDirectory*)malloc(size);
		if (buf)
		{
			buf->StructSize=sizeof(FarPanelDirectory);
			Info.PanelControl(CurPanel.hPanel,FCTL_GETPANELDIRECTORY,size,buf);
			wcscpy(CurPanel.Dir,buf->Name);
			if (!(CurPanel.PInfo.Flags&PFLAGS_PLUGIN))
			{
				size=FSF.ConvertPath(CPM_NATIVE,buf->Name,0,0);
				CurList.Dir=(wchar_t*)malloc(size*sizeof(wchar_t));
				if (CurList.Dir) FSF.ConvertPath(CPM_NATIVE,buf->Name,CurList.Dir,size);
			}
			else
			{
				CurList.Dir=(wchar_t*)malloc((wcslen(buf->Name)+1)*sizeof(wchar_t));
				if (CurList.Dir) wcscpy(CurList.Dir,buf->Name);
			}
			free(buf);
		}
	}
}

void FreeDirList(struct DirList *pList)
{
	if (pList->PPI)
	{
		for (int i=0; i<pList->ItemsNumber; i++)
			free((void*)pList->PPI[i].FileName);
		free(pList->PPI); pList->PPI=NULL;
	}
	free(pList->Dir); pList->Dir=NULL;
	pList->ItemsNumber=0;
}

/****************************************************************************
 * Динамическая загрузка необходимых dll
 ****************************************************************************/

///  VisComp.dll
PCOMPAREFILES pCompareFiles=NULL;

///  libgfl340.dll
extern PGFLGETVERSION pGflGetVersion=NULL;
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
PGFLGETCOLORAT pGflGetColorAt=NULL;

///  bass.dll
PBASS_GETVERSION pBASS_GetVersion=NULL;
PBASS_SETCONFIG pBASS_SetConfig=NULL;
PBASS_INIT pBASS_Init=NULL;
PBASS_FREE pBASS_Free=NULL;
PBASS_STREAMCREATEFILE pBASS_StreamCreateFile=NULL;
PBASS_STREAMFREE pBASS_StreamFree=NULL;
PBASS_CHANNELGETLENGTH pBASS_ChannelGetLength=NULL;
PBASS_CHANNELBYTES2SECONDS pBASS_ChannelBytes2Seconds=NULL;
PBASS_CHANNELGETINFO pBASS_ChannelGetInfo=NULL;
PBASS_CHANNELGETTAGS pBASS_ChannelGetTags=NULL;
PBASS_STREAMGETFILEPOSITION pBASS_StreamGetFilePosition=NULL;
PBASS_GETDEVICEINFO pBASS_GetDeviceInfo=NULL;


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
		HMODULE h=GetModuleHandleW(L"VisComp.dll");
		if (h)
		{
			pCompareFiles=(PCOMPAREFILES)GetProcAddress(h,"CompareFiles");
			return true;
		}
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
		if (!pGflGetVersion)
			pGflGetVersion=(PGFLGETVERSION)GetProcAddress(GflHandle,"gflGetVersion");
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
		if (!pGflGetColorAt)
			pGflGetColorAt=(PGFLGETCOLORAT)GetProcAddress(GflHandle,"gflGetColorAt");

		if (!pGflGetVersion || !pGflLibraryInit || !pGflEnableLZW || !pGflLibraryExit || !pGflLoadBitmapW || !pGflGetNumberOfFormat || !pGflGetFormatInformationByIndex ||
				!pGflGetDefaultLoadParams || !pGflChangeColorDepth || !pGflRotate || !pGflResize || !pGflFreeBitmap || !pGflFreeFileInformation || !pGflGetColorAt)
		{
			FreeLibrary(GflHandle);
			return false;
		}
		bGflLoaded=true;

		if (lstrcmpA(GFL_VERSION,pGflGetVersion())>0 || pGflLibraryInit()!=GFL_NO_ERROR)
			UnLoadGfl();
		if (bGflLoaded)
			pGflEnableLZW(GFL_TRUE);
	}
	return bGflLoaded;
}

bool UnLoadBASS()
{
	if (bBASSLoaded)
	{
		pBASS_Free();
		if (FreeLibrary(BASSHandle))
			bBASSLoaded=false;
		else
			return false;
	}
	return true;
}

bool LoadBASS(wchar_t *PlugPath)
{
	if (bBASSLoaded) return true;

	string strPatchBASS;
	if (FindFile(PlugPath,L"bass.dll",strPatchBASS))
	{
		if (!(BASSHandle=LoadLibrary(strPatchBASS)))
			return false;
		if (!pBASS_GetVersion)
			pBASS_GetVersion=(PBASS_GETVERSION)GetProcAddress(BASSHandle,"BASS_GetVersion");
		if (!pBASS_SetConfig)
			pBASS_SetConfig=(PBASS_SETCONFIG)GetProcAddress(BASSHandle,"BASS_SetConfig");
		if (!pBASS_Init)
			pBASS_Init=(PBASS_INIT)GetProcAddress(BASSHandle,"BASS_Init");
		if (!pBASS_Free)
			pBASS_Free=(PBASS_FREE)GetProcAddress(BASSHandle,"BASS_Free");
		if (!pBASS_StreamCreateFile)
			pBASS_StreamCreateFile=(PBASS_STREAMCREATEFILE)GetProcAddress(BASSHandle,"BASS_StreamCreateFile");
		if (!pBASS_StreamFree)
			pBASS_StreamFree=(PBASS_STREAMFREE)GetProcAddress(BASSHandle,"BASS_StreamFree");
		if (!pBASS_ChannelGetLength)
			pBASS_ChannelGetLength=(PBASS_CHANNELGETLENGTH)GetProcAddress(BASSHandle,"BASS_ChannelGetLength");
		if (!pBASS_ChannelBytes2Seconds)
			pBASS_ChannelBytes2Seconds=(PBASS_CHANNELBYTES2SECONDS)GetProcAddress(BASSHandle,"BASS_ChannelBytes2Seconds");
		if (!pBASS_ChannelGetInfo)
			pBASS_ChannelGetInfo=(PBASS_CHANNELGETINFO)GetProcAddress(BASSHandle,"BASS_ChannelGetInfo");
		if (!pBASS_ChannelGetTags)
			pBASS_ChannelGetTags=(PBASS_CHANNELGETTAGS)GetProcAddress(BASSHandle,"BASS_ChannelGetTags");
		if (!pBASS_StreamGetFilePosition)
			pBASS_StreamGetFilePosition=(PBASS_STREAMGETFILEPOSITION)GetProcAddress(BASSHandle,"BASS_StreamGetFilePosition");
		if (!pBASS_GetDeviceInfo)
			pBASS_GetDeviceInfo=(PBASS_GETDEVICEINFO)GetProcAddress(BASSHandle,"BASS_GetDeviceInfo");

		if (!pBASS_GetVersion || !pBASS_SetConfig || !pBASS_Init || !pBASS_Free || !pBASS_StreamCreateFile || !pBASS_StreamFree || !pBASS_GetDeviceInfo ||
				!pBASS_ChannelGetLength || !pBASS_ChannelBytes2Seconds || !pBASS_ChannelGetInfo || !pBASS_ChannelGetTags || !pBASS_StreamGetFilePosition)
		{
			FreeLibrary(BASSHandle);
			return false;
		}
		bBASSLoaded=true;

		if (HIWORD(pBASS_GetVersion())!=BASSVERSION)
			UnLoadBASS();
		if (bBASSLoaded)
		{
			if (!pBASS_Init(0,44100,0,0,NULL))
				UnLoadBASS();
		}
	}
	return bBASSLoaded;
}


/****************************************************************************
 ***************************** Exported functions ***************************
 ****************************************************************************/


/****************************************************************************
 * Эти функции плагина FAR вызывает в первую очередь
 ****************************************************************************/
void WINAPI GetGlobalInfoW(struct GlobalInfo *pInfo)
{
	pInfo->StructSize=sizeof(GlobalInfo);
	pInfo->MinFarVersion=FARMANAGERVERSION;
	pInfo->Version=MAKEFARVERSION(3,0,0,34,VS_RC);
	pInfo->Guid=MainGuid;
	pInfo->Title=L"Advanced compare 2";
	pInfo->Description=L"Advanced compare 2 plugin for Far Manager v3.0";
	pInfo->Author=L"Alexey Samlyukov";
}

// заполним структуру PluginStartupInfo и сделаем ряд полезных действий...
void WINAPI SetStartupInfoW(const struct PluginStartupInfo *pInfo)
{
	::Info = *pInfo;
	if (pInfo->StructSize >= sizeof(PluginStartupInfo))
	{
		FSF = *pInfo->FSF;
		::Info.FSF = &FSF;

		// обнулим кэш (туда будем помещать результаты сравнения)
		memset(&Cache,0,sizeof(Cache));

		wchar_t PlugPath[MAX_PATH];
		ExpandEnvironmentStringsW(L"%FARHOME%\\Plugins",PlugPath,(sizeof(PlugPath)/sizeof(PlugPath[0])));
		LoadVisComp(PlugPath);
		LoadGfl(PlugPath);
		LoadBASS(PlugPath);
	}
}


/****************************************************************************
 * Эту функцию плагина FAR вызывает во вторую очередь - заполним PluginInfo, т.е.
 * скажем FARу какие пункты добавить в "Plugin commands" и "Plugins configuration".
 ****************************************************************************/
void WINAPI GetPluginInfoW(struct PluginInfo *pInfo)
{
	static const wchar_t *PluginMenuStrings[1];
	PluginMenuStrings[0] = GetMsg(MCmpTitle);

	pInfo->StructSize=sizeof(PluginInfo);
	pInfo->PluginMenu.Guids=&MenuGuid;
	pInfo->PluginMenu.Strings=PluginMenuStrings;
	pInfo->PluginMenu.Count=sizeof(PluginMenuStrings)/sizeof(PluginMenuStrings[0]);
}


/****************************************************************************
 * Основная функция плагина. FAR её вызывает, когда пользователь зовёт плагин
 ****************************************************************************/
HANDLE WINAPI OpenW(const struct OpenInfo *OInfo)
{
	HANDLE hPanel = NULL;
	struct PanelInfo PInfo;
	PInfo.StructSize=sizeof(PanelInfo);

	// Если не удалось запросить информацию о активной панели...
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
	// Если не удалось запросить информацию о пассивной панели...
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
		ErrorMsg(MCmpTitle, MFilePanelsRequired);
		return hPanel;
	}

	LPanel.bTMP=LPanel.bARC=LPanel.bCurFile=LPanel.bDir=false;
	RPanel.bTMP=RPanel.bARC=RPanel.bCurFile=RPanel.bDir=false;
	struct DirList LList, RList;

	GetDirList(LPanel, LList);
	GetDirList(RPanel, RList);

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

	if (bBASSLoaded)
	{
		BASS_DEVICEINFO info;
		pBASS_GetDeviceInfo(0,&info);
		if (!(info.flags&BASS_DEVICE_INIT))
			if (!pBASS_Init(0,44100,0,0,NULL))
				UnLoadBASS();
	}

	class AdvCmpDlgOpt AdvCmpOpt;
	int ret=AdvCmpOpt.ShowOptDialog();

	if (ret==54 || ret==55) // DlgOK || DlgUNDERCURSOR
	{
		DWORD dwTicks=GetTickCount();
		// откроем, для проверок на Esc
		hConInp=CreateFileW(L"CONIN$", GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);

		class AdvCmpProc AdvCmp;
		AdvCmp.Init();

		if (ret==54)
		{
			if (Opt.Mode!=MODE_DUP)
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
				AdvCmp.Duplicate(LPanel.PInfo.Flags&PFLAGS_FOCUS?&LList:&RList);
			}
		}
		else
		{
			if (hConInp!=INVALID_HANDLE_VALUE) CloseHandle(hConInp);
			AdvCmp.CompareCurFile(LList.Dir,LList.PPI[LPanel.PInfo.CurrentItem].FileName,RList.Dir,RList.PPI[RPanel.PInfo.CurrentItem].FileName,1);
		}
		AdvCmp.Close();
	}

	// определены из диалога опций
	if (Opt.Substr) { free(Opt.Substr); Opt.Substr=NULL; }
	if (Opt.WinMergePath) { free(Opt.WinMergePath); Opt.WinMergePath=NULL; }
	if (Opt.DupPath) { free(Opt.DupPath); Opt.DupPath=NULL; }
	Info.FileFilterControl(Opt.hCustomFilter,FFCTL_FREEFILEFILTER,0,0);

	FreeDirList(&LList);
	FreeDirList(&RList);

	return hPanel;
}

/****************************************************************************
 * Эту функцию FAR вызывает перед выгрузкой плагина
 ****************************************************************************/
void WINAPI ExitFARW(const struct ExitInfo *pInfo)
{
	//Освободим память в случае выгрузки плагина
	if (Cache.RCI)
		free(Cache.RCI);
	Cache.RCI=NULL;
	Cache.ItemsNumber=0;

	UnLoadGfl();
	UnLoadBASS();
}
