/****************************************************************************
 * AdvCmpProc.cpp
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

#pragma hdrstop
#include "AdvCmpProc.hpp"


/****************************************************************************
 *
 *                    Разные оччччень полезные функции :-)
 *
 ****************************************************************************/

/****************************************************************************
 * Поиск и вырезание Substr в именах файлов.
 ****************************************************************************/
wchar_t *CutSubstr(string &strSrc, wchar_t *Substr)
{
	if (!Substr) return strSrc.get();
	int len=wcslen(Substr);
	if (!len) return strSrc.get();

	int lenSrc=strSrc.length();
	const wchar_t *src=strSrc.get();
	string strBuf;
	// делаем замену
	{
		HANDLE re;
		int start_offset=0;
		if (!Info.RegExpControl(0,RECTL_CREATE,0,&re)) return false;

		string Search=L"/";
		if (len>0 && Substr[0]==L'/') 
			Search+=Substr+1;
		else Search+=Substr;
		if (Search.length()>0 && Search[(size_t)(Search.length()-1)]!=L'/')
			Search+=L"/i";
		if (Info.RegExpControl(re,RECTL_COMPILE,0,Search.get()))
		{
			int brackets=Info.RegExpControl(re,RECTL_BRACKETSCOUNT,0,0);
			if (!brackets) { Info.RegExpControl(re,RECTL_FREE,0,0); return false; }
			RegExpMatch *match=(RegExpMatch*)malloc(brackets*sizeof(RegExpMatch));

			for (;;)
			{
				RegExpSearch search= { src,start_offset,lenSrc,match,brackets,0 };

				if (Info.RegExpControl(re,RECTL_SEARCHEX,0,&search))
				{
					// копируем ДО паттерна
					for (int i=start_offset; i<match[0].start; i++)
						strBuf+=src[i];

					start_offset=match[0].end;

					if (match[0].start==match[0].end || start_offset>=lenSrc)
						break;
				}
				else
					break;
			}
			free(match);
			Info.RegExpControl(re,RECTL_FREE,0,0);
		}
		// копируем всё то что не вошло в паттерн
		for (int i=start_offset; i<lenSrc; i++)
			strBuf+=src[i];
		if (!FSF.Trim(strBuf.get())) return strSrc.get();
		strSrc=strBuf.get();
	}
	return strSrc.get();
}

/****************************************************************************
 * Центрирование строки и заполнение символом заполнителем
 ****************************************************************************/
void strcentr(wchar_t *Dest, const wchar_t *Src, int len, wchar_t sym)
{
	int iLen, iLen2;
	iLen=wcslen(wcscpy(Dest,Src));
	if (iLen<len)
	{
		iLen2=(len-iLen)/2;
		wmemmove(Dest+iLen2,Dest,iLen);
		wmemset(Dest,sym,iLen2);
		wmemset(Dest+iLen2+iLen,sym,len-iLen2-iLen);
		Dest[len]=L'\0';
	}
}

/****************************************************************************
 * Преобразует int в wchar_t поразрядно: из 1234567890 в "1 234 567 890"
 ****************************************************************************/
wchar_t* itoaa(__int64 num, wchar_t *buf)
{
	wchar_t tmp[100];
	FSF.itoa64(num,tmp,10);
	int digits_count=0;
	wchar_t *t=tmp;
	while (*t++)
		digits_count++;
	wchar_t *p=buf+digits_count+(digits_count-1) / 3;
	digits_count=0;
	*p--=L'\0';
	t--;      //заметь, требуется дополнительное смещение!
	while (p!=buf)
	{
		*p--=*--t;
		if ((++digits_count) % 3 == 0)
			*p-- = L' ';
	}
	*p=*--t;

	return buf;
}

/****************************************************************************
 * Рисует строку-прогресс
 ****************************************************************************/
void ProgressLine(wchar_t *Dest, unsigned __int64 nCurrent, unsigned __int64 nTotal)
{
	int n=0, len=WinInfo.TruncLen-4;
	if (nTotal>0) n=nCurrent*len / nTotal;
	if (n>len) n=len;
	wchar_t *Buf=(wchar_t*)malloc(WinInfo.TruncLen*sizeof(wchar_t));
	if (Buf)
	{
		wmemset(Buf,0x00002588,n);
		wmemset(&Buf[n],0x00002591,len-n);
		Buf[len]=L'\0';
		FSF.sprintf(Dest,L"%s%3d%%",Buf,nTotal?(nCurrent*100 / nTotal):0);
		free(Buf);
	}
	else
		*Dest=0;
}

/****************************************************************************
 * Возвращает смещение начала файла, т.е. без префиксов "\\?\"
 ****************************************************************************/
wchar_t *GetPosToName(const wchar_t *FileName)
{
	if (FileName && FileName[0]==L'\\' && FileName[1]==L'\\' && FileName[2]==L'?')
	{
		if (FileName[5]==L':')
			return (wchar_t *)&FileName[4];
		else if (FileName[5]==L'N')
			return (wchar_t *)&FileName[7];
	}
	return (wchar_t *)FileName;
}

/****************************************************************************
 * Возвращает полное имя файла, и опционально без префиксов "\\?\"
 ****************************************************************************/
void GetFullFileName(string &strFullFileName, const wchar_t *Dir, const wchar_t *FileName, bool bNative)
{
	if (Dir) strFullFileName=bNative?Dir:GetPosToName(Dir);
	if ((strFullFileName.length()>0 && strFullFileName[(size_t)(strFullFileName.length()-1)]!=L'\\') || ((LPanel.bTMP||RPanel.bTMP)? false: !strFullFileName.length())) strFullFileName+=L"\\";
	strFullFileName+=FileName;
}

/****************************************************************************
 * Возвращает строку с временем файла
 ****************************************************************************/
wchar_t *GetStrFileTime(FILETIME *LastWriteTime, wchar_t *Time, bool FullYear=true)
{
	SYSTEMTIME ModificTime;
	FILETIME LocalTime;
	FileTimeToLocalFileTime(LastWriteTime,&LocalTime);
	FileTimeToSystemTime(&LocalTime,&ModificTime);
	// для Time достаточно [20] !!!
	if (Time)
		FSF.sprintf(Time, FullYear?L"%02d.%02d.%04d %02d:%02d:%02d":L"%02d.%02d.%02d %02d:%02d:%02d",ModificTime.wDay,ModificTime.wMonth,FullYear?ModificTime.wYear:ModificTime.wYear%100,ModificTime.wHour,ModificTime.wMinute,ModificTime.wSecond);
	return Time;
}

/****************************************************************************
 * Проверка на Esc. Возвращает true, если пользователь нажал Esc
 ****************************************************************************/
bool CheckForEsc(void)
{
	if (hConInp == INVALID_HANDLE_VALUE)
		return false;

	static DWORD dwTicks;
	DWORD dwNewTicks = GetTickCount();
	if (dwNewTicks - dwTicks < 500)
		return false;
	dwTicks = dwNewTicks;

	INPUT_RECORD rec;
	DWORD ReadCount;
	while (PeekConsoleInput(hConInp, &rec, 1, &ReadCount) && ReadCount)
	{
		ReadConsoleInput(hConInp, &rec, 1, &ReadCount);
		if ( rec.EventType == KEY_EVENT && rec.Event.KeyEvent.wVirtualKeyCode == VK_ESCAPE && rec.Event.KeyEvent.bKeyDown )
		{
			// Опциональное подтверждение прерывания по Esc
			if (Info.AdvControl(&MainGuid, ACTL_GETCONFIRMATIONS,0,0) & FCS_INTERRUPTOPERATION)
			{
				if (YesNoMsg(MEscTitle, MEscBody))
					return bBrokenByEsc = true;
			}
			else
				return bBrokenByEsc = true;
		}
	}
	return false;
}

/****************************************************************************
 * Усекает начало длинных имен файлов (или дополняет короткие имена)
 * для правильного показа в сообщении сравнения
 ****************************************************************************/
void TruncCopy(wchar_t *Dest, const wchar_t *Src, int TruncLen, const wchar_t *FormatMsg=NULL)
{
	string strSrc(Src);
	int iLen=0;
	if (FormatMsg) // чего-нибудь допишем...
	{
		FSF.sprintf(Dest,FormatMsg,FSF.TruncPathStr(strSrc.get(),TruncLen-wcslen(FormatMsg)+2));
		iLen=wcslen(Dest);
	}
	else // иначе, тупо скопируем имя
		iLen=wcslen(wcscpy(Dest,FSF.TruncPathStr(strSrc.get(),TruncLen)));

	if (iLen<TruncLen) // для красивости дополним пробелами
	{
		wmemset(&Dest[iLen],L' ',TruncLen-iLen);
		Dest[TruncLen]=L'\0';
	}
}


AdvCmpProc::AdvCmpProc()
{
	hScreen=Info.SaveScreen(0,0,-1,-1);
	bStartMsg=true;

	FList.F=NULL;
	FList.iCount=0;
	FList.Items=0;
	FList.Select=0;
	FList.Identical=0;
	FList.Different=0;
	FList.LNew=0;
	FList.RNew=0;
	FList.bShowSelect=true;
	FList.bShowIdentical=true;
	FList.bShowDifferent=true;
	FList.bShowLNew=true;
	FList.bShowRNew=true;
	FList.bClearUserFlags=false;

	Opt.BufSize=65536<<4;
	Opt.Buf[0]=NULL;
	Opt.Buf[1]=NULL;

	// создадим буферы сравнения
	if (Opt.CmpContents)
	{
		Opt.Buf[0]=(char*)malloc(Opt.BufSize*sizeof(char));
		Opt.Buf[1]=(char*)malloc(Opt.BufSize*sizeof(char));
	}

	LPanel.hFilter=RPanel.hFilter=INVALID_HANDLE_VALUE;
	Info.FileFilterControl(LPanel.hPanel,FFCTL_CREATEFILEFILTER,FFT_PANEL,&LPanel.hFilter);
	Info.FileFilterControl(RPanel.hPanel,FFCTL_CREATEFILEFILTER,FFT_PANEL,&RPanel.hFilter);
	Info.FileFilterControl(LPanel.hFilter,FFCTL_STARTINGTOFILTER,0,0);
	Info.FileFilterControl(RPanel.hFilter,FFCTL_STARTINGTOFILTER,0,0);

	// На время сравнения изменим заголовок консоли ФАРа...
	TitleSaved=GetFarTitle(strFarTitle);
	SetConsoleTitle(GetMsg(MComparingFiles));
}

AdvCmpProc::~AdvCmpProc()
{
	if (Opt.Buf[0]) free(Opt.Buf[0]);
	if (Opt.Buf[1]) free(Opt.Buf[1]);

	Info.FileFilterControl(LPanel.hFilter,FFCTL_FREEFILEFILTER,0,0);
	Info.FileFilterControl(RPanel.hFilter,FFCTL_FREEFILEFILTER,0,0);

	Info.RestoreScreen(hScreen);
	// Восстановим заголовок консоли ФАРа...
	if (TitleSaved) SetConsoleTitle(strFarTitle);

	for (int i=0; i<FList.iCount; i++)
	{
		if (FList.F[i].FileName) free(FList.F[i].FileName);
		if (FList.F[i].LDir) free(FList.F[i].LDir);
		if (FList.F[i].RDir) free(FList.F[i].RDir);
	}
	if (FList.F) free (FList.F); FList.F=NULL;
	FList.iCount=0;
}


/****************************************************************************
 *
 *                          SHOWMESSAGE FUNCTIONS
 *
 ****************************************************************************/


/****************************************************************************
 * Получить заголовок консоли ФАРа
 ****************************************************************************/
bool AdvCmpProc::GetFarTitle(string &strTitle)
{
	DWORD dwSize=0;
	DWORD dwBufferSize=MAX_PATH;
	wchar_t *lpwszTitle=NULL;
	do
	{
		dwBufferSize <<= 1;
		lpwszTitle = (wchar_t*)realloc(lpwszTitle, dwBufferSize*sizeof(wchar_t));
		dwSize = GetConsoleTitle(lpwszTitle, dwBufferSize);
	}
	while (!dwSize && GetLastError() == ERROR_SUCCESS);

	if (dwSize)
		strTitle=lpwszTitle;
	free(lpwszTitle);
	return dwSize;
}

/****************************************************************************
 * Показывает сообщение о сравнении двух файлов
 ****************************************************************************/
void AdvCmpProc::ShowCmpMsg(const wchar_t *Dir1, const wchar_t *Name1, const wchar_t *Dir2, const wchar_t *Name2, bool bRedraw)
{
	// Для перерисовки не чаще 3-х раз в 1 сек.
	if (!bRedraw)
	{
		static DWORD dwTicks;
		DWORD dwNewTicks = GetTickCount();
		if (dwNewTicks - dwTicks < 350)
			return;
		dwTicks = dwNewTicks;
	}

	wchar_t Buf[MAX_PATH], ItemsOut[MAX_PATH];

	wchar_t TruncDir1[MAX_PATH], TruncDir2[MAX_PATH], TruncName1[MAX_PATH], TruncName2[MAX_PATH];
	TruncCopy(TruncDir1, GetPosToName(Dir1), WinInfo.TruncLen, GetMsg(MComparing));
	TruncCopy(TruncName1, Name1, WinInfo.TruncLen);
	TruncCopy(TruncDir2, GetPosToName(Dir2), WinInfo.TruncLen, GetMsg(MComparingWith));
	TruncCopy(TruncName2, Name2, WinInfo.TruncLen);

	wchar_t LDiff[100], RDiff[100], Errors[100], DiffOut[MAX_PATH];
	FSF.sprintf(Buf,GetMsg(MComparingDiffN),itoaa(CmpInfo.LDiff,LDiff),itoaa(CmpInfo.RDiff,RDiff),itoaa(CmpInfo.Errors,Errors));
	strcentr(DiffOut,Buf,WinInfo.TruncLen,0x00002500);

	wchar_t ProgressLineCur[MAX_PATH], ProgressLineTotal[MAX_PATH];
	if (!Opt.CmpContents || bStartMsg)
		wcscpy(ProgressLineCur,GetMsg(MWait));
	else
		ProgressLine(ProgressLineCur,CmpInfo.CurProcSize,CmpInfo.CurCountSize);

	if (Opt.TotalProcess)
	{
		FSF.sprintf(Buf,GetMsg(MComparingFiles2),CmpInfo.CountSize && !((LPanel.PInfo.Flags&PFLAGS_PLUGIN) || (RPanel.PInfo.Flags&PFLAGS_PLUGIN))?(CmpInfo.ProcSize*100/CmpInfo.CountSize):0);
		SetConsoleTitle(Buf);

		wchar_t Count[100], CountSize[100];
		FSF.sprintf(Buf,GetMsg(MComparingN),itoaa(CmpInfo.CountSize,CountSize),itoaa(CmpInfo.Count,Count));
		strcentr(ItemsOut,Buf,WinInfo.TruncLen,0x00002500);

		if ((LPanel.PInfo.Flags&PFLAGS_PLUGIN) || (RPanel.PInfo.Flags&PFLAGS_PLUGIN))
			wcscpy(ProgressLineTotal,GetMsg(MWait));
		else
			ProgressLine(ProgressLineTotal,CmpInfo.ProcSize,CmpInfo.CountSize);
	}
	strcentr(Buf,L"",WinInfo.TruncLen,0x00002500);  // просто сепаратор

	const wchar_t *MsgItems1[] = {
		GetMsg(MCompareTitle),
		TruncDir1,TruncName1,DiffOut,ProgressLineCur,Buf,TruncDir2,TruncName2
	};

	const wchar_t *MsgItems2[] = {
		GetMsg(MCompareTitle),
		TruncDir1,TruncName1,DiffOut,ProgressLineCur,Buf,ProgressLineTotal,ItemsOut,TruncDir2,TruncName2
	};


	Info.Message(&MainGuid,&CmpMsgGuid,bStartMsg?FMSG_LEFTALIGN:FMSG_LEFTALIGN|FMSG_KEEPBACKGROUND,0,Opt.TotalProcess?MsgItems2:MsgItems1,
																		Opt.TotalProcess?sizeof(MsgItems2)/sizeof(MsgItems2[0]):sizeof(MsgItems1)/sizeof(MsgItems1[0]),0);
	bStartMsg=false;
}



/****************************************************************************
 *
 *                          COMPAREFILES FUNCTIONS
 *
 ****************************************************************************/

void AdvCmpProc::WFD2PPI(WIN32_FIND_DATA &wfd, PluginPanelItem &ppi)
{
	ppi.FileAttributes=wfd.dwFileAttributes;
	ppi.LastAccessTime=wfd.ftLastAccessTime;
	ppi.LastWriteTime=wfd.ftLastWriteTime;
	ppi.FileSize=((__int64)wfd.nFileSizeHigh << 32) | wfd.nFileSizeLow;
	ppi.FileName=(wchar_t*)malloc((wcslen(wfd.cFileName)+1)*sizeof(wchar_t));
	if (ppi.FileName) wcscpy((wchar_t*)ppi.FileName,wfd.cFileName);
}

/****************************************************************************
 * Замена сервисной функции Info.GetDirList(). В отличие от оной возвращает
 * список файлов только в каталоге Dir, без подкаталогов.
 * Умеет собирать информацию об элементах на заданную глубину.
 ****************************************************************************/
int AdvCmpProc::GetDirList(const wchar_t *Dir, int ScanDepth, bool OnlyInfo, struct DirList *pList)
{
	bool ret=true;

	if (OnlyInfo && bBrokenByEsc)
		return ret;

	string strPathMask(Dir);

	if (Opt.ScanSymlink)
	{
		DWORD Attrib=GetFileAttributesW(Dir);
		if (Attrib!=INVALID_FILE_ATTRIBUTES && (Attrib&FILE_ATTRIBUTE_REPARSE_POINT))
		{
			// получим реальный путь
			size_t size=FSF.ConvertPath(CPM_REAL,Dir,0,0);
			wchar_t *buf=strPathMask.get(size); 
			FSF.ConvertPath(CPM_REAL,Dir,buf,size);
			strPathMask.updsize();

			// проверка на рекурсию - узнаем, может мы уже отсюда пришли
			wchar_t RealPrevDir[32768];
			wcscpy(RealPrevDir,Dir);
			(wchar_t)*(FSF.PointToName(RealPrevDir)) = 0;
			FSF.ConvertPath(CPM_REAL,RealPrevDir,RealPrevDir,32768);

			if (!FSF.LStricmp(strPathMask.get(),RealPrevDir)) // да, уже были тут!
				ret=false;
		}
	}

	if (!OnlyInfo) // заполняем DirList
	{
		pList->Dir=(wchar_t*)malloc((wcslen(Dir)+1)*sizeof(wchar_t));
		if (pList->Dir) wcscpy(pList->Dir,Dir);
		pList->PPI=0;
		pList->ItemsNumber=0;
	}

	if (Opt.ScanSymlink && !ret) // выходим
		return true;

	strPathMask+=L"\\*";
	WIN32_FIND_DATA wfdFindData;
	HANDLE hFind;

	if ((hFind=FindFirstFileW(strPathMask,&wfdFindData)) != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (OnlyInfo && CheckForEsc())
			{
				ret=true;
				break;
			}

			if (!Opt.ProcessHidden && (wfdFindData.dwFileAttributes&FILE_ATTRIBUTE_HIDDEN))
				continue;
			if ((wfdFindData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) &&
						((wfdFindData.cFileName[0]==L'.' && !wfdFindData.cFileName[1]) || (wfdFindData.cFileName[0]==L'.' && wfdFindData.cFileName[1]==L'.' && !wfdFindData.cFileName[2])))
				continue;
			if (OnlyInfo && (wfdFindData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY))
			{
				if (Opt.ProcessSubfolders==2 && Opt.MaxScanDepth<ScanDepth+1) // не глубже заданного уровня!
					break;
				if (!Opt.ProcessSubfolders)
					continue;
				GetFullFileName(strPathMask,Dir,wfdFindData.cFileName);
				ret=GetDirList(strPathMask,ScanDepth+1,OnlyInfo,0);
			}
			else
			{
				if (OnlyInfo)
				{
					CmpInfo.Count+=1;
					CmpInfo.CountSize+=((unsigned __int64)wfdFindData.nFileSizeHigh << 32) | wfdFindData.nFileSizeLow;
					ShowCmpMsg(L"*",L"*",L"*",L"*",false);
				}
				else
				{
					struct PluginPanelItem *pPPI=(PluginPanelItem *)realloc(pList->PPI,(pList->ItemsNumber+1)*sizeof(PluginPanelItem));
					if (!pPPI)
					{
						ErrorMsg(MNoMemTitle, MNoMemBody);
						// !!! возможно тут требуется обнулить элементы и их кол-во
						ret=false;
						break;
					}
					pList->PPI=pPPI;
					WFD2PPI(wfdFindData,pList->PPI[pList->ItemsNumber++]);
				}
			}
		} while (FindNextFile(hFind,&wfdFindData));
		FindClose(hFind);
	}
	else
		CmpInfo.Errors++;

	return ret;
}

/****************************************************************************
 * Замена сервисной функции Info.FreeDirList().
 ****************************************************************************/
void AdvCmpProc::FreeDirList(struct DirList *pList)
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
 * Перемещение указателя в файле для нужд Opt.ContentsPercent
 ****************************************************************************/
bool AdvCmpProc::mySetFilePointer(HANDLE hf, __int64 distance, DWORD MoveMethod)
{
	bool bSet = true;
	LARGE_INTEGER li;
	li.QuadPart = distance;
	li.LowPart = SetFilePointer(hf, li.LowPart, &li.HighPart, MoveMethod);
	if (li.LowPart == 0xFFFFFFFF && GetLastError() != NO_ERROR)
		bSet = false;
	return bSet;
}

/****************************************************************************
 * CRC32 со стандартным полиномом 0xEDB88320.
 ****************************************************************************/
DWORD AdvCmpProc::ProcessCRC(void *pData, register int iLen, DWORD FileCRC)
{
	register unsigned char *pdata = (unsigned char *)pData;
	register DWORD crc = FileCRC;
	static unsigned TableCRC[256];
	if (!TableCRC[1])
	{ // Инициализация CRC32 таблицы
		unsigned i, j, r;
		for (i = 0; i < 256; i++)
		{
			for (r = i, j = 8; j; j--)
				r = r & 1 ? (r >> 1) ^ 0xEDB88320 : r >> 1;
			TableCRC[i] = r;
		}
	}
	while (iLen--)
	{
		crc = TableCRC[(unsigned char)crc ^ *pdata++] ^ crc >> 8;
		crc ^= 0xD202EF8D;
	}
	return crc;
}

/****************************************************************************
 * Результат предыдущего сравнения "по содержимому".
 ****************************************************************************/
int AdvCmpProc::GetCacheResult(DWORD FullFileName1, DWORD FullFileName2, DWORD64 WriteTime1, DWORD64 WriteTime2)
{
	for (int i=0; i<Cache.ItemsNumber; i++)
	{
		if ( ((FullFileName1==Cache.RCI[i].dwFullFileName[0] && FullFileName2==Cache.RCI[i].dwFullFileName[1]) &&
					(WriteTime1==Cache.RCI[i].dwWriteTime[0] && WriteTime2==Cache.RCI[i].dwWriteTime[1]))
				|| ((FullFileName1==Cache.RCI[i].dwFullFileName[1] && FullFileName2==Cache.RCI[i].dwFullFileName[0]) &&
					(WriteTime1==Cache.RCI[i].dwWriteTime[1] && WriteTime2==Cache.RCI[i].dwWriteTime[0])) )
		{
			return (int)Cache.RCI[i].dwFlags;
		}
	}
	return 0;  // 0 - результат не определен, т.к. элемент не найден
}

/****************************************************************************
 * Сохранение результата сравнения "по содержимому".
 ****************************************************************************/
bool AdvCmpProc::SetCacheResult(DWORD FullFileName1, DWORD FullFileName2, DWORD64 WriteTime1, DWORD64 WriteTime2, DWORD dwFlag)
{
	bool bFoundItem=false;
	for (int i=0; i<Cache.ItemsNumber; i++)
	{
		if ( ((FullFileName1==Cache.RCI[i].dwFullFileName[0] && FullFileName2==Cache.RCI[i].dwFullFileName[1]) &&
					(WriteTime1==Cache.RCI[i].dwWriteTime[0] && WriteTime2==Cache.RCI[i].dwWriteTime[1]))
				|| ((FullFileName1==Cache.RCI[i].dwFullFileName[1] && FullFileName2==Cache.RCI[i].dwFullFileName[0]) &&
					(WriteTime1==Cache.RCI[i].dwWriteTime[1] && WriteTime2==Cache.RCI[i].dwWriteTime[0])) )
		{
			bFoundItem=true;
			Cache.RCI[i].dwFlags=dwFlag; // сделаем "тупо" :-)
			break;
		}
	}

	if (!bFoundItem)
	{
		struct ResultCmpItem *pRCI=(struct ResultCmpItem *)realloc(Cache.RCI,(Cache.ItemsNumber+1)*sizeof(ResultCmpItem));
		if (pRCI)
		{
			Cache.RCI=pRCI;
			struct ResultCmpItem *CurItem=&Cache.RCI[Cache.ItemsNumber++];
			CurItem->dwFullFileName[0]  = FullFileName1;
			CurItem->dwFullFileName[1]  = FullFileName2;
			CurItem->dwWriteTime[0] = WriteTime1;
			CurItem->dwWriteTime[1] = WriteTime2;
			CurItem->dwFlags=dwFlag;
		}
		else
		{
			ErrorMsg(MNoMemTitle, MNoMemBody);
			free(Cache.RCI);
			Cache.RCI=0; Cache.ItemsNumber=0;
			return false;
		}
	}
	return true;
}

/****************************************************************************
 * Сравнение атрибутов и прочего для двух одноимённых элементов (файлов или
 * подкаталогов).
 * Возвращает true, если они совпадают.
 ****************************************************************************/
bool AdvCmpProc::CompareFiles(const wchar_t *LDir, const PluginPanelItem *pLPPI, const wchar_t *RDir, const PluginPanelItem *pRPPI, int ScanDepth)
{
	if (pLPPI->FileAttributes & FILE_ATTRIBUTE_DIRECTORY)
	{
		// Здесь сравниваем два подкаталога
		if (Opt.ProcessSubfolders)
		{
			if (Opt.ProcessSubfolders==2 && Opt.MaxScanDepth<ScanDepth+1)
				return true;

			bool bLPanelPlug=(LPanel.PInfo.Flags&PFLAGS_PLUGIN), bRPanelPlug=(RPanel.PInfo.Flags&PFLAGS_PLUGIN);

			if (ScanDepth>0 && (bLPanelPlug || bRPanelPlug))
				return true;

			string strLFullDir, strRFullDir;
			GetFullFileName(strLFullDir,LDir,pLPPI->FileName);
			GetFullFileName(strRFullDir,RDir,pRPPI->FileName);

			// Составим списки элементов в подкаталогах
			struct DirList LList, RList;
			bool bEqual = true;

			if (!(bLPanelPlug || bRPanelPlug))
			{
				if (!GetDirList(strLFullDir,ScanDepth,false,&LList) || !GetDirList(strRFullDir,ScanDepth,false,&RList))
				{
					bBrokenByEsc=true; // То ли юзер прервал, то ли ошибка чтения
					bEqual=false; // Остановим сравнение
				}
			}
			else if (bLPanelPlug || bRPanelPlug)
			{
				LList.Dir=(wchar_t*)malloc((wcslen(LDir)+1)*sizeof(wchar_t));
				if (LList.Dir) wcscpy(LList.Dir,LDir);
//				DebugMsg(L"LList.Dir",LList.Dir);
//				DebugMsg(L"pLPPI->FileName",(wchar_t*)pLPPI->FileName);
				RList.Dir=(wchar_t*)malloc((wcslen(RDir)+1)*sizeof(wchar_t));
				if (RList.Dir) wcscpy(RList.Dir,RDir);
//				DebugMsg(L"RList.Dir",RList.Dir);
//				DebugMsg(L"pRPPI->FileName",(wchar_t*)pRPPI->FileName);

				if (bLPanelPlug && !bBrokenByEsc && !Info.GetPluginDirList(&MainGuid,LPanel.hPanel,pLPPI->FileName,&LList.PPI,(size_t*)&LList.ItemsNumber))
				{
					bBrokenByEsc=true;
					bEqual=false;
				}

				if (bRPanelPlug && !bBrokenByEsc && !Info.GetPluginDirList(&MainGuid,RPanel.hPanel,pRPPI->FileName,&RList.PPI,(size_t*)&RList.ItemsNumber))
				{
					bBrokenByEsc=true;
					bEqual=false;
				}

				if (!bLPanelPlug && !bBrokenByEsc && !Info.GetDirList(GetPosToName(strLFullDir.get()),&LList.PPI,(size_t*)&LList.ItemsNumber))
				{
					bBrokenByEsc=true;
					bEqual=false;
				}

				if (!bRPanelPlug && !bBrokenByEsc && !Info.GetDirList(GetPosToName(strRFullDir.get()),&RList.PPI,(size_t*)&RList.ItemsNumber))
				{
					bBrokenByEsc=true;
					bEqual=false;
				}
//				for (int i=0; bEqual && !LPanel.bARC && i<LList.ItemsNumber; i++)
//					DebugMsg(L"FAR",(wchar_t*)LList.PPI[i].FileName,i);
//				for (int i=0; bEqual && !RPanel.bARC && i<RList.ItemsNumber; i++)
//					DebugMsg(L"FAR",(wchar_t*)RList.PPI[i].FileName,i);

//				for (int i=0; bEqual && LPanel.bARC && i<LList.ItemsNumber; i++)
//					DebugMsg(L"ARC",(wchar_t*)LList.PPI[i].FileName,i);
//				for (int i=0; bEqual && RPanel.bARC && i<RList.ItemsNumber; i++)
//					DebugMsg(L"ARC",(wchar_t*)RList.PPI[i].FileName,i);
			}

			if (bEqual)
				bEqual=CompareDirs(&LList,&RList,Opt.Dialog,ScanDepth+1);  // Opt.Dialog==1 то всё сравним в подкаталоге, для показа в диалоге

			if (!(bLPanelPlug || bRPanelPlug))
			{
				FreeDirList(&LList);
				FreeDirList(&RList);
			}
			else if (bLPanelPlug || bRPanelPlug)
			{
				free(LList.Dir);
				free(RList.Dir);
				if (bLPanelPlug) Info.FreePluginDirList(LList.PPI,LList.ItemsNumber);
				if (bRPanelPlug) Info.FreePluginDirList(RList.PPI,RList.ItemsNumber);
				if (!bLPanelPlug) Info.FreeDirList(LList.PPI,LList.ItemsNumber);
				if (!bRPanelPlug) Info.FreeDirList(RList.PPI,RList.ItemsNumber);
			}

			return bEqual;
		}
	}
	else
	// Здесь сравниваем два файла
	{
		CmpInfo.CurCountSize=pLPPI->FileSize+pRPPI->FileSize;
		CmpInfo.CurProcSize=0;

		// покажем "работу" на прогрессе :)
		if (!Opt.CmpContents)  // содержимое - особый случай...
			CmpInfo.ProcSize+=CmpInfo.CurCountSize;

		// регистр имен
		if (Opt.CmpCase)
		{
			string strLFileName(LPanel.bTMP?FSF.PointToName(pLPPI->FileName):pLPPI->FileName);
			string strRFileName(RPanel.bTMP?FSF.PointToName(pRPPI->FileName):pRPPI->FileName);

			if (Strncmp(Opt.SkipSubstr?CutSubstr(strLFileName,Opt.Substr):strLFileName.get(),
									Opt.SkipSubstr?CutSubstr(strRFileName,Opt.Substr):strRFileName.get()))
				return false;
		}
		//===========================================================================
		// размер
		if (Opt.CmpSize && (pLPPI->FileSize != pRPPI->FileSize))
		{
			return false;
		}
		//===========================================================================
		// время
		if (Opt.CmpTime)
		{
			if (Opt.Seconds || Opt.IgnoreTimeZone)
			{
				union {
					__int64 num;
					struct {
						DWORD lo;
						DWORD hi;
					} hilo;
				} Precision, Difference, TimeDelta, temp;

				Precision.hilo.hi = 0;
				Precision.hilo.lo = (Opt.Seconds && Opt.LowPrecisionTime) ? 20000000 : 0; //2s or 0s
				Difference.num = __int64(9000000000); //15m

				FILETIME LLastWriteTime=pLPPI->LastWriteTime, RLastWriteTime=pRPPI->LastWriteTime;

				if (Opt.Seconds && !Opt.LowPrecisionTime)
				{
					SYSTEMTIME Time;
					FileTimeToSystemTime(&LLastWriteTime, &Time);
					Time.wSecond=Time.wMilliseconds=0;
					SystemTimeToFileTime(&Time,&LLastWriteTime);

					FileTimeToSystemTime(&RLastWriteTime, &Time);
					Time.wSecond=Time.wMilliseconds=0;
					SystemTimeToFileTime(&Time,&RLastWriteTime);
				}

				if (LLastWriteTime.dwHighDateTime > RLastWriteTime.dwHighDateTime)
				{
					TimeDelta.hilo.hi=LLastWriteTime.dwHighDateTime - RLastWriteTime.dwHighDateTime;
					TimeDelta.hilo.lo=LLastWriteTime.dwLowDateTime - RLastWriteTime.dwLowDateTime;
					if (TimeDelta.hilo.lo > LLastWriteTime.dwLowDateTime)
						--TimeDelta.hilo.hi;
				}
				else
				{
					if (LLastWriteTime.dwHighDateTime == RLastWriteTime.dwHighDateTime)
					{
						TimeDelta.hilo.hi=0;
						TimeDelta.hilo.lo=max(RLastWriteTime.dwLowDateTime,LLastWriteTime.dwLowDateTime)-
															min(RLastWriteTime.dwLowDateTime,LLastWriteTime.dwLowDateTime);
					}
					else
					{
						TimeDelta.hilo.hi=RLastWriteTime.dwHighDateTime - LLastWriteTime.dwHighDateTime;
						TimeDelta.hilo.lo=RLastWriteTime.dwLowDateTime - LLastWriteTime.dwLowDateTime;
						if (TimeDelta.hilo.lo > RLastWriteTime.dwLowDateTime)
							--TimeDelta.hilo.hi;
					}
				}

				//игнорировать различия не больше чем 26 часов.
				if (Opt.IgnoreTimeZone)
				{
					int counter = 0;
					while (TimeDelta.hilo.hi > Difference.hilo.hi && counter<=26*4)
					{
						temp.hilo.lo = TimeDelta.hilo.lo - Difference.hilo.lo;
						temp.hilo.hi = TimeDelta.hilo.hi - Difference.hilo.hi;
						if (temp.hilo.lo > TimeDelta.hilo.lo)
							--temp.hilo.hi;
						TimeDelta.hilo.lo = temp.hilo.lo;
						TimeDelta.hilo.hi = temp.hilo.hi;
						++counter;
					}
					if (counter<=26*4 && TimeDelta.hilo.hi == Difference.hilo.hi)
					{
						TimeDelta.hilo.hi = 0;
						TimeDelta.hilo.lo = max(TimeDelta.hilo.lo,Difference.hilo.lo) - min(TimeDelta.hilo.lo,Difference.hilo.lo);
					}
				}

				if ( Precision.hilo.hi < TimeDelta.hilo.hi ||
						(Precision.hilo.hi == TimeDelta.hilo.hi && Precision.hilo.lo < TimeDelta.hilo.lo))
				{
					return false;
				}
			}
			else if (pLPPI->LastWriteTime.dwLowDateTime != pRPPI->LastWriteTime.dwLowDateTime ||
							 pLPPI->LastWriteTime.dwHighDateTime != pRPPI->LastWriteTime.dwHighDateTime)
			{
				return false;
			}
		}
		//===========================================================================
		// содержимое
		if (Opt.CmpContents)
		{

			// экспресс-сравнение: сравним размер файлов
			if (!Opt.Ignore && (pLPPI->FileSize != pRPPI->FileSize))
			{
				CmpInfo.ProcSize+=CmpInfo.CurCountSize;
				return false;
			}

			// экспресс-сравнение: время совпало - скажем "одинаковые"
			if ( Opt.OnlyTimeDiff &&
					 (pLPPI->LastWriteTime.dwLowDateTime == pRPPI->LastWriteTime.dwLowDateTime &&
						pLPPI->LastWriteTime.dwHighDateTime == pRPPI->LastWriteTime.dwHighDateTime)
					)
			{
				CmpInfo.ProcSize+=CmpInfo.CurCountSize;
				return true;
			}

			// сравним 2-е архивные панели
			if (LPanel.bARC && RPanel.bARC)
			{
				CmpInfo.ProcSize+=CmpInfo.CurCountSize;
//        wchar_t buf[40];
//        FSF.sprintf(buf, L"L - %X R- %X", pLPPI->CRC32,pRPPI->CRC32);
//        DebugMsg(buf, L"");
				if (pLPPI->CRC32 == pRPPI->CRC32) return true;
				else return false;
			}

			string strLFullFileName, strRFullFileName;
			GetFullFileName(strLFullFileName,LDir,pLPPI->FileName);
			GetFullFileName(strRFullFileName,RDir,pRPPI->FileName);

			// получим NativeDir - "\\?\dir\"
			if (LPanel.bTMP || RPanel.bTMP)
			{
				size_t size=FSF.ConvertPath(CPM_NATIVE,LPanel.bTMP?strLFullFileName.get():strRFullFileName.get(),0,0);
				wchar_t *buf=(wchar_t*)malloc(size*sizeof(wchar_t));
				if (buf)
				{
					FSF.ConvertPath(CPM_NATIVE,LPanel.bTMP?strLFullFileName.get():strRFullFileName.get(),buf,size);
					(LPanel.bTMP?strLFullFileName:strRFullFileName)=buf;
					free(buf);
				}
			}

			// работа с кешем
			DWORD dwLFileName, dwRFileName;

			if (Opt.Cache && !Opt.Ignore && !(LPanel.bARC || RPanel.bARC))
			{
				dwLFileName=ProcessCRC((void *)strLFullFileName.get(),strLFullFileName.length()*2,0);
				dwRFileName=ProcessCRC((void *)strRFullFileName.get(),strRFullFileName.length()*2,0);

				// Используем кешированные данные
				if (!Opt.CacheIgnore )
				{
					int Result=GetCacheResult(dwLFileName,dwRFileName,
																		((__int64)pLPPI->LastWriteTime.dwHighDateTime << 32) | pLPPI->LastWriteTime.dwLowDateTime,
																		((__int64)pRPPI->LastWriteTime.dwHighDateTime << 32) | pRPPI->LastWriteTime.dwLowDateTime);
//              wchar_t buf[200];
//              FSF.sprintf(buf, L"GetCacheResult: L - %X R- %X", dwLFileName, dwRFileName);
//              DebugMsg(buf,(wchar_t*)pLPPI->FindData.lpwszFileName,Result?(Result==RCIF_EQUAL?RCIF_EQUAL:RCIF_DIFFER):0);

					if (Result == RCIF_EQUAL)
					{
						CmpInfo.ProcSize+=CmpInfo.CurCountSize;
						return true;
					}
					else if (Result == RCIF_DIFFER)
					{
						CmpInfo.ProcSize+=CmpInfo.CurCountSize;
						return false;
					}
				}
			}

			HANDLE hLFile, hRFile;
			FILETIME LAccess, RAccess;
			BY_HANDLE_FILE_INFORMATION LFileInfo, RFileInfo;
			bool bOkLFileInfo=false, bOkRFileInfo=false;

			if (!LPanel.bARC)
			{
				if ((hLFile=CreateFileW(strLFullFileName, GENERIC_READ, FILE_SHARE_READ, 0,
                                 OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, 0)) == INVALID_HANDLE_VALUE)
				{
					CmpInfo.ProcSize+=CmpInfo.CurCountSize;
					CmpInfo.Errors++;
					return false;
				}
				bOkLFileInfo=GetFileInformationByHandle(hLFile,&LFileInfo);
				// Сохраним время последнего доступа к файлу
				LAccess=pLPPI->LastAccessTime;
			}

			if (!RPanel.bARC)
			{
				if ((hRFile=CreateFileW(strRFullFileName, GENERIC_READ, FILE_SHARE_READ, 0,
                                 OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, 0)) == INVALID_HANDLE_VALUE)
				{
					if (hLFile) CloseHandle(hLFile);
					CmpInfo.ProcSize+=CmpInfo.CurCountSize;
					CmpInfo.Errors++;
					return false;
				}
				bOkRFileInfo=GetFileInformationByHandle(hRFile,&RFileInfo);
				RAccess=pRPPI->LastAccessTime;
			}

			//---------------------------------------------------------------------------

			ShowCmpMsg(LDir,pLPPI->FileName,RDir,pRPPI->FileName,true);

			// экспресс-сравнение: FileIndex совпали - скажем "одинаковые"
			if ( bOkLFileInfo && bOkRFileInfo &&
						LFileInfo.dwVolumeSerialNumber==RFileInfo.dwVolumeSerialNumber &&
						LFileInfo.nFileIndexHigh==RFileInfo.nFileIndexHigh && LFileInfo.nFileIndexLow==RFileInfo.nFileIndexLow )
			{
				CloseHandle(hLFile);
				CloseHandle(hRFile);
				CmpInfo.ProcSize+=CmpInfo.CurCountSize;
				return true;
			}

			DWORD LReadSize=1, RReadSize=1;
			DWORD LBufPos=1, RBufPos=1;     // позиция в Opt.Buf
			const DWORD ReadBlock=65536;
			__int64 LFilePos=0, RFilePos=0;  // позиция в файле
			bool bEqual=true;

			{
				char *LPtr=Opt.Buf[0]+LBufPos, *RPtr=Opt.Buf[1]+RBufPos;
				bool bLExpectNewLine=false, bRExpectNewLine=false;

				SHFILEINFO shinfo;
				bool bExe=false;
				if (!(LPanel.bARC || RPanel.bARC))
					bExe=(SHGetFileInfoW(strLFullFileName,0,&shinfo,sizeof(shinfo),SHGFI_EXETYPE) ||
								SHGetFileInfoW(strRFullFileName,0,&shinfo,sizeof(shinfo),SHGFI_EXETYPE));

				DWORD dwFileCRC=0;
				__int64 PartlyKbSize=(__int64)Opt.PartlyKbSize*1024;
				// частичное сравнение
				bool bPartlyFull=( Opt.Partly && !Opt.Ignore && !(LPanel.bARC || RPanel.bARC) &&
													(Opt.PartlyFull && pLPPI->FileSize > Opt.BufSize) );

				bool bPartlyKb=( Opt.Partly && !Opt.Ignore && !(LPanel.bARC || RPanel.bARC) &&
												(!Opt.PartlyFull && PartlyKbSize && pLPPI->FileSize > abs(PartlyKbSize)) );

				unsigned int BlockIndex=pLPPI->FileSize / Opt.BufSize;
				unsigned int LCurBlockIndex=0, RCurBlockIndex=0;

				// если с минусом, отсчитаем с конца файла
				bool bFromEnd=(bPartlyKb && abs(PartlyKbSize)!=PartlyKbSize);
				if (bFromEnd)
				{
					if (!mySetFilePointer(hLFile,PartlyKbSize,FILE_END) || !mySetFilePointer(hRFile,PartlyKbSize,FILE_END))
						bEqual=false;
				}

				while (1)
				{
					// частичное сравнение, пропускаем блоками по Opt.BufSize
					if (bPartlyFull)
					{
						if (!mySetFilePointer(hLFile,Opt.BufSize,FILE_CURRENT) || !mySetFilePointer(hRFile,Opt.BufSize,FILE_CURRENT))
						{
							bEqual = false;
							break;
						}
//						else 	DebugMsg(L"skip",L"",Opt.BufSize);
					}

					// читаем файл с активной панели
					if (!LPanel.bARC && LPtr >= Opt.Buf[0]+LBufPos)
					{
						LBufPos=0;
						LPtr=Opt.Buf[0];
						// читаем блоком Opt.BufSize
						while (LBufPos < Opt.BufSize)
						{
							if (CheckForEsc() || 
									!ReadFile(hLFile,Opt.Buf[0]+LBufPos,(!bPartlyKb || bFromEnd || LFilePos+ReadBlock<=PartlyKbSize)?ReadBlock:(PartlyKbSize-LFilePos),&LReadSize,0))
							{
								bEqual=false;
								break;
							}
							LBufPos+=LReadSize;
							LFilePos+=LReadSize;
//					DebugMsg(L"LReadSize",L"",LReadSize);
							CmpInfo.CurProcSize+=LReadSize;
							CmpInfo.ProcSize+=LReadSize;
							if (LReadSize < ReadBlock) break;
						}
					}
					if (!bEqual)
						break;

					// читаем файл с пассивной панели
					if (!RPanel.bARC && RPtr >= Opt.Buf[1]+RBufPos)
					{
						RBufPos=0;
						RPtr=Opt.Buf[1];
						// читаем блоком Opt.BufSize
						while (RBufPos < Opt.BufSize)
						{
							if (CheckForEsc() || 
									!ReadFile(hRFile,Opt.Buf[1]+RBufPos,(!bPartlyKb || bFromEnd || RFilePos+ReadBlock<=PartlyKbSize)?ReadBlock:(PartlyKbSize-RFilePos),&RReadSize,0))
							{
								bEqual=false;
								break;
							}
							RBufPos+=RReadSize;
							RFilePos+=RReadSize;
//					DebugMsg(L"RReadSize",L"",RReadSize);

							CmpInfo.CurProcSize+=RReadSize;
							CmpInfo.ProcSize+=RReadSize;
							if (RReadSize < ReadBlock) break;
						}
					}
					if (!bEqual)
						break;

					ShowCmpMsg(LDir,pLPPI->FileName,RDir,pRPPI->FileName,false);

					// сравниваем с архивом
					if (RPanel.bARC)
					{
						dwFileCRC=ProcessCRC(Opt.Buf[0],LBufPos,dwFileCRC);
						LPtr+=LBufPos;
						CmpInfo.CurProcSize+=LBufPos;
						CmpInfo.ProcSize+=LBufPos;
					}
					else if (LPanel.bARC)
					{
						dwFileCRC=ProcessCRC(Opt.Buf[1],RBufPos,dwFileCRC);
						RPtr+=RBufPos;
						CmpInfo.CurProcSize+=RBufPos;
						CmpInfo.ProcSize+=RBufPos;
					}

					if (LPanel.bARC || RPanel.bARC)
					{
						if ((RPanel.bARC && LBufPos != Opt.BufSize) || (LPanel.bARC && RBufPos != Opt.BufSize))
						{
							if (!LPanel.bARC && RPanel.bARC && dwFileCRC != pRPPI->CRC32)
								bEqual=false;
							else if (LPanel.bARC && !RPanel.bARC && dwFileCRC != pLPPI->CRC32)
								bEqual = false;
//              wchar_t buf[40];
//              FSF.sprintf(buf, L"L - %X R- %X", LPanel.bARC?pLPPI->CRC32:dwFileCRC, RPanel.bARC?pRPPI->CRC32:dwFileCRC);
//							if ((LPanel.bARC?pLPPI->CRC32:dwFileCRC)!=(RPanel.bARC?pRPPI->CRC32:dwFileCRC))
//              DebugMsg(buf,(wchar_t*)pLPPI->FindData.lpwszFileName,(LPanel.bARC?pLPPI->CRC32:dwFileCRC)!=(RPanel.bARC?pRPPI->CRC32:dwFileCRC));
							break;
						}
						else
							continue;
					}

					// обычное сравнение (фильтр отключен или файлы исполнимые)
					if (!Opt.Ignore || bExe)
					{
						if (memcmp(Opt.Buf[0], Opt.Buf[1], LBufPos))
						{
							bEqual=false;
							break;
						}
						LPtr += LBufPos;
						RPtr += RBufPos;

						// считали всё, выходим
						if (LBufPos != Opt.BufSize || RBufPos != Opt.BufSize)
							break;
					}
					else
					// фильтр включен
					{
						if (Opt.IgnoreTemplates == 0)      // '\n' & ' '
						{
							while (LPtr < Opt.Buf[0]+LBufPos && RPtr < Opt.Buf[1]+RBufPos && !IsWhiteSpace(*LPtr) && !IsWhiteSpace(*RPtr))
							{
								if (*LPtr != *RPtr)
								{
									bEqual=false;
									break;
								}
								++LPtr;
								++RPtr;
							}
							if (!bEqual)
								break;

							while (LPtr < Opt.Buf[0]+LBufPos && IsWhiteSpace(*LPtr))
								++LPtr;

							while (RPtr < Opt.Buf[1]+RBufPos && IsWhiteSpace(*RPtr))
								++RPtr;
						}
						else if (Opt.IgnoreTemplates == 1)  // '\n'
						{
							if (bLExpectNewLine)
							{
								bLExpectNewLine=false;
								if (LPtr < Opt.Buf[0]+LBufPos && *LPtr == '\n')
									++LPtr;
							}

							if (bRExpectNewLine)
							{
								bRExpectNewLine=false;
								if (RPtr < Opt.Buf[1]+RBufPos && *RPtr == '\n')
									++RPtr;
							}

							while (LPtr < Opt.Buf[0]+LBufPos && RPtr < Opt.Buf[1]+RBufPos && !IsNewLine(*LPtr) && !IsNewLine(*RPtr))
							{
								if (*LPtr != *RPtr)
								{
									bEqual = false;
									break;
								}
								++LPtr;
								++RPtr;
							}
							if (!bEqual)
								break;

							if (LPtr < Opt.Buf[0]+LBufPos && RPtr < Opt.Buf[1]+RBufPos && (!IsNewLine(*LPtr) || !IsNewLine(*RPtr)))
							{
								bEqual = false;
								break;
							}

							if (LPtr < Opt.Buf[0]+LBufPos && RPtr < Opt.Buf[1]+RBufPos)
							{
								if (*LPtr == '\r')
									bLExpectNewLine=true;

								if (*RPtr == '\r')
									bRExpectNewLine=true;

								++LPtr;
								++RPtr;
							}
						}
						else if (Opt.IgnoreTemplates == 2)  // ' '
						{
							while (LPtr < Opt.Buf[0]+LBufPos && RPtr < Opt.Buf[1]+RBufPos && !myIsSpace(*LPtr) && !myIsSpace(*RPtr))
							{
								if (*LPtr != *RPtr)
								{
									bEqual=false;
									break;
								}
								++LPtr;
								++RPtr;
							}
							if (!bEqual)
								break;

							while (LPtr < Opt.Buf[0]+LBufPos && myIsSpace(*LPtr))
								++LPtr;

							while (RPtr < Opt.Buf[1]+RBufPos && myIsSpace(*RPtr))
								++RPtr;
						}
						if (!LReadSize && RReadSize || LReadSize && !RReadSize)
						{
							bEqual = false;
							break;
						}
					}
					if (!LReadSize && !RReadSize)
						break;
				}

				// поместим в кэш результат
				if (Opt.Cache && !(LPanel.bARC || RPanel.bARC) && (!Opt.Ignore || bExe) && !Opt.Partly)
				{
					Opt.Cache=SetCacheResult(dwLFileName,dwRFileName,
																		((__int64)pLPPI->LastWriteTime.dwHighDateTime << 32) | pLPPI->LastWriteTime.dwLowDateTime,
																		((__int64)pRPPI->LastWriteTime.dwHighDateTime << 32) | pRPPI->LastWriteTime.dwLowDateTime,
																		bEqual?RCIF_EQUAL:RCIF_DIFFER);
//              DebugMsg(L"SetCacheResult",(wchar_t*)pLPPI->FindData.lpwszFileName,bEqual?RCIF_EQUAL:RCIF_DIFFER);
//              wchar_t buf[200];
//              FSF.sprintf(buf, L"SetCacheResult: L - %X R- %X", dwLFileName, dwRFileName);
//              DebugMsg(buf,(wchar_t*)pLPPI->FindData.lpwszFileName,bEqual?RCIF_EQUAL:RCIF_DIFFER);

				}
			}
			if (!(LPanel.bARC))
			{
				CloseHandle(hLFile);
				if ((hLFile=CreateFileW(strLFullFileName, FILE_WRITE_ATTRIBUTES, FILE_SHARE_READ | FILE_SHARE_WRITE, 0,
                                OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, 0)) != INVALID_HANDLE_VALUE)
				{
					SetFileTime(hLFile,0,&LAccess,0);
					CloseHandle(hLFile);
				}
			}
			if (!(RPanel.bARC))
			{
				CloseHandle(hRFile);
				if ((hRFile=CreateFileW(strRFullFileName, FILE_WRITE_ATTRIBUTES, FILE_SHARE_READ | FILE_SHARE_WRITE, 0,
                                OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, 0)) != INVALID_HANDLE_VALUE)
				{
					SetFileTime(hRFile,0,&RAccess,0);
					CloseHandle(hRFile);
				}
			}
			if (!bEqual)
			{
				CmpInfo.ProcSize+=CmpInfo.CurCountSize-CmpInfo.CurProcSize;
				return false;
			}
		}
	}
	return true;
}


/****************************************************************************
*
                         COMPAREDIRS FUNCTIONS
*
 ****************************************************************************/


/****************************************************************************
 * Функция сравнения имён файлов в двух структурах PluginPanelItem
 * для нужд qsort()
 ****************************************************************************/
int __cdecl PICompare(const void *el1, const void *el2)
{
	const PluginPanelItem *ppi1 = *(const PluginPanelItem **)el1, *ppi2 = *(const PluginPanelItem **)el2;

	if (ppi1->FileAttributes & FILE_ATTRIBUTE_DIRECTORY)
	{
		if (!(ppi2->FileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			return 1;
	}
	else
	{
		if (ppi2->FileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			return -1;
	}

	string strLFileName(LPanel.bTMP||RPanel.bTMP?FSF.PointToName(ppi1->FileName):ppi1->FileName);
	string strRFileName(LPanel.bTMP||RPanel.bTMP?FSF.PointToName(ppi2->FileName):ppi2->FileName);

	int i=FSF.LStricmp(Opt.SkipSubstr?CutSubstr(strLFileName,Opt.Substr):strLFileName.get(),
											Opt.SkipSubstr?CutSubstr(strRFileName,Opt.Substr):strRFileName.get());
//			DebugMsg(strLFileName.get(),L"PICompare-strLFileName",i);
//			DebugMsg(strRFileName.get(),L"PICompare-strRFileName",i);
  return i;
}

/****************************************************************************
 * Функция проверяет, входит ли файл из архива в заданную глубину вложенности
 ****************************************************************************/
bool AdvCmpProc::CheckScanDepth(const wchar_t *FileName, int ScanDepth)
{
	int i=0;
	while (*FileName++)
		if (*FileName==L'\\') i++;
	return  i<=ScanDepth;
}


/****************************************************************************
 * Построение сортированного списка элементов для быстрого сравнения
 ****************************************************************************/
bool AdvCmpProc::BuildItemsIndex(bool bLeftPanel,const struct DirList *pList,struct ItemsIndex *pIndex,int ScanDepth)
{
	pIndex->pPPI=0;
	pIndex->iCount=pList->ItemsNumber;

	if (!pIndex->iCount)
		return true;
	if (!(pIndex->pPPI=(PluginPanelItem **)malloc(pIndex->iCount * sizeof(pIndex->pPPI[0]))))
		return false;

	int j = 0;
	for (int i=pIndex->iCount-1; i>=0 && j<pIndex->iCount; i--)
	{
					// каталоги отсеиваем сразу... если надо
		if ( (Opt.ProcessSubfolders || !(pList->PPI[i].FileAttributes & FILE_ATTRIBUTE_DIRECTORY)) &&
					// выбираем только отмеченные элементы... если надо :)
				(!(Opt.ProcessSelected && ScanDepth==0) || (pList->PPI[i].Flags & PPIF_SELECTED))
			 )
		{
			if ( (pList->PPI[i].FileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
					 ((pList->PPI[i].FileName[0]==L'.' && !pList->PPI[i].FileName[1]) || 
					 (pList->PPI[i].FileName[0]==L'.' && pList->PPI[i].FileName[1]==L'.' && !pList->PPI[i].FileName[2]))
					)
				continue;
			if (!Opt.ProcessHidden && (pList->PPI[i].FileAttributes&FILE_ATTRIBUTE_HIDDEN))
				continue;
			if ( (bLeftPanel?LPanel.hFilter:RPanel.hFilter)!=INVALID_HANDLE_VALUE &&
					 !Info.FileFilterControl((bLeftPanel?LPanel.hFilter:RPanel.hFilter),FFCTL_ISFILEINFILTER,0,&pList->PPI[i]))
				continue;
			if (Opt.Filter && !Info.FileFilterControl(Opt.hCustomFilter,FFCTL_ISFILEINFILTER,0,&pList->PPI[i]))
				continue;

			if (ScanDepth && !(LPanel.bTMP || RPanel.bTMP))
			{
				bool bLPanelPlug=(LPanel.PInfo.Flags&PFLAGS_PLUGIN), bRPanelPlug=(RPanel.PInfo.Flags&PFLAGS_PLUGIN);

				// плагин + панель || панель + плагин (элемент с панели)
				if ((bLPanelPlug && !bRPanelPlug && !bLeftPanel) || (!bLPanelPlug && bRPanelPlug && bLeftPanel))
				{
					string srtFileName(pList->PPI[i].FileName);
					string strSubstr;
					wchar_t *p=pList->Dir+4;
					while (*p++) // для экранирования спецсимволов в регэкспах
					{
						if (*p==L'\\' || *p==L'[' || *p==L']' || *p==L'+' || *p==L'{' || *p==L'}')
							strSubstr+=L"\\";
						strSubstr+=*p;
					}
					// вырежем pList->Dir из имени файла, т.к. путь до текущей папки (и сама папка) нам не нужен
					wcscpy((wchar_t*)pList->PPI[i].FileName,CutSubstr(srtFileName,strSubstr.get())+2);

					if (Opt.ProcessSubfolders==2 && !CheckScanDepth(pList->PPI[i].FileName, Opt.MaxScanDepth))
						continue;
				}

				// плагин + панель || панель + плагин (элемент с плагина)
				else if ((bLPanelPlug && !bRPanelPlug && bLeftPanel) || (!bLPanelPlug && bRPanelPlug && !bLeftPanel))
				{
					if (Opt.ProcessSubfolders==2 && !CheckScanDepth(pList->PPI[i].FileName, Opt.MaxScanDepth))
						continue;
				}
			}

			pIndex->pPPI[j++]=&pList->PPI[i];
		}
	}

	if (pIndex->iCount=j)
	{
			FSF.qsort(pIndex->pPPI,j,sizeof(pIndex->pPPI[0]),PICompare);
	}
	else
	{
		free(pIndex->pPPI);
		pIndex->pPPI=0;
	}

	return true;
}

/****************************************************************************
 * Освобождение памяти
 ****************************************************************************/
void AdvCmpProc::FreeItemsIndex(struct ItemsIndex *pIndex)
{
	if (pIndex->pPPI)
		free(pIndex->pPPI);
	pIndex->pPPI=0;
	pIndex->iCount=0;
}


/****************************************************************************
 * Сравнение двух каталогов, описанных структурами AInfo и PInfo.
 * Возвращает true, если они совпадают.
 * Параметр bCompareAll определяет,
 * надо ли сравнивать все файлы и взводить PPIF_SELECTED (bCompareAll == true)
 * или просто вернуть false при первом несовпадении (bCompareAll == false).
 ****************************************************************************/
bool AdvCmpProc::CompareDirs(const struct DirList *pLList,const struct DirList *pRList,bool bCompareAll,int ScanDepth)
{
	// Стартуем с сообщением о сравнении
	ShowCmpMsg(pLList->Dir,L"*",pRList->Dir,L"*",true);

	// строим индексы элементов, для убыстрения сравнения
	struct ItemsIndex LII, RII;
	if (!BuildItemsIndex(true,pLList,&LII,ScanDepth) || !BuildItemsIndex(false,pRList,&RII,ScanDepth))
	{
		ErrorMsg(MNoMemTitle,MNoMemBody);
		bBrokenByEsc=true;
		FreeItemsIndex(&LII);
		FreeItemsIndex(&RII);
		return true;
	}

	int i=0, j=0;

	// соберем информацию, сколько элементов будем сравнивать и их размер
	if (ScanDepth==0 && Opt.TotalProcess)
	{
		while (i<LII.iCount && j<RII.iCount && !bBrokenByEsc)
		{
			switch (PICompare(&LII.pPPI[i], &RII.pPPI[j]))
			{
				case 0:
				{
					string strDir;
					if (LII.pPPI[i]->FileAttributes&FILE_ATTRIBUTE_DIRECTORY && !(LPanel.PInfo.Flags&PFLAGS_PLUGIN))
					{
						GetFullFileName(strDir,pLList->Dir,LII.pPPI[i]->FileName);
						GetDirList(strDir,ScanDepth,true);
					}
					else
					{
						CmpInfo.Count+=1;
						CmpInfo.CountSize+=(unsigned __int64)LII.pPPI[i]->FileSize;
					}

					if (!bBrokenByEsc)
					{
						if (RII.pPPI[j]->FileAttributes&FILE_ATTRIBUTE_DIRECTORY && !(RPanel.PInfo.Flags&PFLAGS_PLUGIN))
						{
							GetFullFileName(strDir,pRList->Dir,RII.pPPI[j]->FileName);
							GetDirList(strDir,ScanDepth,true);
						}
						else
						{
							CmpInfo.Count+=1;
							CmpInfo.CountSize+=(unsigned __int64)RII.pPPI[j]->FileSize;
						}
					}
					i++; j++;
					break;
				}
				case -1:
				{
					CmpInfo.Count+=1;
					CmpInfo.CountSize+=(unsigned __int64)LII.pPPI[i]->FileSize;
					i++;
					break;
				}
				case 1:
				{
					CmpInfo.Count+=1;
					CmpInfo.CountSize+=(unsigned __int64)RII.pPPI[j]->FileSize;
					j++;
					break;
				}
			}
		}
	}

	if (bBrokenByEsc)
	{
		FreeItemsIndex(&LII);
		FreeItemsIndex(&RII);
		return true;
	}

	// экспресс-сравнение вложенного каталога
	if (ScanDepth && !Opt.Dialog && !Opt.IgnoreMissing && LII.iCount!=RII.iCount)
	{
		FreeItemsIndex(&LII);
		FreeItemsIndex(&RII);
		return false;
	}

	// вначале снимем выделение на панелях
	if (ScanDepth==0)
	{
		for (i=0; i<pLList->ItemsNumber; i++)
			pLList->PPI[i].Flags &= ~PPIF_SELECTED;
		for (i=0; i<pRList->ItemsNumber; i++)
			pRList->PPI[i].Flags &= ~PPIF_SELECTED;
	}

	// начинаем сравнивать "наши" элементы...
	bool bDifferenceNotFound=true;
	i=0; j=0;
	DWORD dwFlag=0;

	while (i<LII.iCount && j<RII.iCount && (bDifferenceNotFound || bCompareAll) && !bBrokenByEsc)
	{
		// проверка на ESC
		const int iMaxCounter=256;
		static int iCounter=iMaxCounter;
		if (!--iCounter)
		{
			iCounter=iMaxCounter;
			if (CheckForEsc())
				break;
		}

		bool bNextItem;
		dwFlag=RCIF_DIFFER;

		switch (PICompare(&LII.pPPI[i], &RII.pPPI[j]))
		{

			/******************************************************************************/

			case 0: // Имена совпали - проверяем всё остальное
			{
//wchar_t buf[512];
//FSF.sprintf(buf,L"Left: %s Right: %s, %d + %d", LII.pPPI[i]->FindData.lpwszFileName, RII.pPPI[j]->FindData.lpwszFileName, i,j);
//			 DebugMsg(buf,L"case 0",bDifferenceNotFound);

				if (CompareFiles(pLList->Dir,LII.pPPI[i],pRList->Dir,RII.pPPI[j],ScanDepth))
				{// И остальное совпало
					i++; j++;
					dwFlag=RCIF_EQUAL;
				}
				else
				{
					bDifferenceNotFound=false;
					// узнаем, новый кто?
					if (Opt.SelectedNew)
					{
						__int64 Delta=(((__int64)LII.pPPI[i]->LastWriteTime.dwHighDateTime << 32) | LII.pPPI[i]->LastWriteTime.dwLowDateTime) -
													(((__int64)RII.pPPI[j]->LastWriteTime.dwHighDateTime << 32) | RII.pPPI[j]->LastWriteTime.dwLowDateTime);

						if (Delta>0)
						{
							LII.pPPI[i]->Flags |= PPIF_SELECTED;
						}
						else if (Delta<0)
						{
							RII.pPPI[j]->Flags |= PPIF_SELECTED;
						}
						else
						{
							LII.pPPI[i]->Flags |= PPIF_SELECTED;
							RII.pPPI[j]->Flags |= PPIF_SELECTED;
						}
					}
					else
					{
						LII.pPPI[i]->Flags |= PPIF_SELECTED;
						RII.pPPI[j]->Flags |= PPIF_SELECTED;
					}
					i++; j++;
					CmpInfo.LDiff++; CmpInfo.RDiff++;
					dwFlag=RCIF_DIFFER;
					if (Opt.ProcessTillFirstDiff && !bBrokenByEsc)
					{     // нужно ли продолжать сравнивать
						bCompareAll=(Opt.ShowMsg && !YesNoMsg(MFirstDiffTitle, MFirstDiffBody));
						Opt.ProcessTillFirstDiff=0;
					}
				}
				CmpInfo.Proc+=2;
				// добавим элемент в диалог результатов
				if (Opt.Dialog)
					BuildFileList(pLList->Dir,LII.pPPI[i-1],pRList->Dir,RII.pPPI[j-1],dwFlag);
				break;
			}

			/******************************************************************************/

			case -1: // Элемент LII.pPPI[i] не имеет одноимённых в RII.pPPI
			{
//wchar_t buf2[512];
//FSF.sprintf(buf2,L"Left: %s Right: %s", LII.pPPI[i]->FindData.lpwszFileName, RII.pPPI[j]->FindData.lpwszFileName);
//			 DebugMsg(buf2,L"case -1",bDifferenceNotFound);

				CmpContinueL:
				dwFlag=RCIF_DIFFER;
				if (!Opt.IgnoreMissing)
				{
					if (!LPanel.bTMP)
					{
						bNextItem=true;
						goto FoundDiffL;
					}
					else
					{ // ...но если с Темп-панели, то проверим с элементом RII.pPPI
						bNextItem=false;
						for (int k=0; k<RII.iCount; k++)
						{
							if (!PICompare(&LII.pPPI[i], &RII.pPPI[k]))
							{
								bNextItem = true;
								if (CompareFiles(pLList->Dir,LII.pPPI[i],pRList->Dir,RII.pPPI[k],ScanDepth))
								{
									i++;
									break;
								}
								else
								FoundDiffL:
								{
									bDifferenceNotFound=false;
									LII.pPPI[i]->Flags |= PPIF_SELECTED;
									dwFlag=RCIF_LNEW;
									i++; CmpInfo.LDiff++;
									if (LPanel.bTMP && k<RII.iCount)
									{
										RII.pPPI[k]->Flags |= PPIF_SELECTED;
										CmpInfo.RDiff++;
									}
									if (Opt.ProcessTillFirstDiff && !bBrokenByEsc)
									{
										bCompareAll=(Opt.ShowMsg && !YesNoMsg(MFirstDiffTitle, MFirstDiffBody));
										Opt.ProcessTillFirstDiff=0;
									}
									break;
								}
							}
						}
						if (!bNextItem)
						{
							bNextItem=true;
							goto FoundDiffL;
						}
					}
					// добавим элемент в диалог результатов
					if (Opt.Dialog)
						BuildFileList(pLList->Dir,LII.pPPI[i-1],pRList->Dir,NULL,dwFlag);
				}
				else
				{
					i++;
				}
				CmpInfo.Proc++;
				break;
			}

			/******************************************************************************/

			case 1: // Элемент RII.pPPI[j] не имеет одноимённых в LII.pPPI
			{
//wchar_t buf3[512];
//FSF.sprintf(buf3,L"Left: %s Right: %s", LII.pPPI[i]->FindData.lpwszFileName, RII.pPPI[j]->FindData.lpwszFileName);
//			 DebugMsg(buf3,L"case 1",bDifferenceNotFound);

				CmpContinueR:
				dwFlag=RCIF_DIFFER;
				if (!Opt.IgnoreMissing)
				{
					if (!RPanel.bTMP)
					{
						bNextItem=true;
						goto FoundDiffR;
					}
					else
					{ // ...но если с Темп-панели, то проверим с элементом LII.pPPI
						bNextItem=false;
						for (int k=0; k<LII.iCount; k++)
						{
							if (!PICompare(&LII.pPPI[k], &RII.pPPI[j]))
							{
								bNextItem = true;
								if (CompareFiles(pLList->Dir,LII.pPPI[k],pRList->Dir,RII.pPPI[j],ScanDepth))
								{
									j++;
									break;
								}
								else
								FoundDiffR:
								{
									bDifferenceNotFound=false;
									RII.pPPI[j]->Flags |= PPIF_SELECTED;
									dwFlag=RCIF_RNEW;
									j++; CmpInfo.RDiff++;
									if (RPanel.bTMP && k<LII.iCount)
									{
										LII.pPPI[k]->Flags |= PPIF_SELECTED;
										CmpInfo.LDiff++;
									}
									if (Opt.ProcessTillFirstDiff && !bBrokenByEsc)
									{
										bCompareAll=(Opt.ShowMsg && !YesNoMsg(MFirstDiffTitle, MFirstDiffBody));
										Opt.ProcessTillFirstDiff=0;
									}
									break;
								}
							}
						}
						if (!bNextItem)
						{
							bNextItem = true;
							goto FoundDiffR;
						}
					}
					// добавим элемент в диалог результатов
					if (Opt.Dialog)
						BuildFileList(pLList->Dir,NULL,pRList->Dir,RII.pPPI[j-1],dwFlag);
				}
				else
				{
					j++;
				}
				CmpInfo.Proc++;
				break;
			}
		}
	}

	if (!bBrokenByEsc)
	{
		// Собственно сравнение окончено. Пометим то, что осталось необработанным в массивах
		if (!Opt.IgnoreMissing && i<LII.iCount)
		{
			if (!LPanel.bTMP)
				bDifferenceNotFound=false;
			if (bCompareAll)
				goto CmpContinueL;
		}
		if (!Opt.IgnoreMissing && j<RII.iCount)
		{
			if (!RPanel.bTMP)
				bDifferenceNotFound=false;
			if (bCompareAll)
				goto CmpContinueR;
		}
	}
//			 DebugMsg(L"LII.iCount",L"",LII.iCount);
//			 DebugMsg(L"RII.iCount",L"",RII.iCount);

	FreeItemsIndex(&LII);
	FreeItemsIndex(&RII);

	return bDifferenceNotFound;
}



/***************************************************************************
 *
 *                       ДИАЛОГ РЕЗУЛЬТАТОВ
 *
 ***************************************************************************/


/****************************************************************************
 * Построение массива элементов, для диалога с результатами сравнения
 ****************************************************************************/
bool AdvCmpProc::BuildFileList(const wchar_t *LDir,const PluginPanelItem *pLPPI,const wchar_t *RDir,const PluginPanelItem *pRPPI,DWORD dwFlag)
{
	File *New=(File*)realloc(FList.F,(FList.iCount+1)*sizeof(File));

	if (!New)
		return false;

	FList.F=New;

	FList.F[FList.iCount].dwFlags=dwFlag;

	if (pLPPI && pRPPI && Opt.CmpTime && dwFlag==RCIF_DIFFER)
	{
		__int64 Delta=(((__int64)pLPPI->LastWriteTime.dwHighDateTime << 32) | pLPPI->LastWriteTime.dwLowDateTime) -
									(((__int64)pRPPI->LastWriteTime.dwHighDateTime << 32) | pRPPI->LastWriteTime.dwLowDateTime);
		if (Delta>0) FList.F[FList.iCount].dwFlags=RCIF_LNEW;
		else if (Delta<0) FList.F[FList.iCount].dwFlags=RCIF_RNEW;
	}

	if (pLPPI && !pRPPI)
		FList.F[FList.iCount].dwFlags|=RCIF_LUNIQ;
	else if (!pLPPI && pRPPI)
		FList.F[FList.iCount].dwFlags|=RCIF_RUNIQ;

	FList.F[FList.iCount].LDir=(wchar_t*)malloc((wcslen(LDir)+1)*sizeof(wchar_t));
	if (FList.F[FList.iCount].LDir)
		wcscpy(FList.F[FList.iCount].LDir,LDir);
	FList.F[FList.iCount].RDir=(wchar_t*)malloc((wcslen(RDir)+1)*sizeof(wchar_t));
	if (FList.F[FList.iCount].RDir)
		wcscpy(FList.F[FList.iCount].RDir,RDir);

	if (pLPPI)
	{
		FList.F[FList.iCount].FileName=(wchar_t*)malloc((wcslen(pLPPI->FileName)+1)*sizeof(wchar_t));
		if (FList.F[FList.iCount].FileName) wcscpy(FList.F[FList.iCount].FileName,pLPPI->FileName);
		FList.F[FList.iCount].ftLLastWriteTime.dwLowDateTime=pLPPI->LastWriteTime.dwLowDateTime;
		FList.F[FList.iCount].ftLLastWriteTime.dwHighDateTime=pLPPI->LastWriteTime.dwHighDateTime;
		FList.F[FList.iCount].nLFileSize=pLPPI->FileSize;
		FList.F[FList.iCount].dwLAttributes=pLPPI->FileAttributes;
	}
	else
	{
		FList.F[FList.iCount].ftLLastWriteTime.dwLowDateTime=0;
		FList.F[FList.iCount].ftLLastWriteTime.dwHighDateTime=0;
		FList.F[FList.iCount].nLFileSize=0;
		FList.F[FList.iCount].dwLAttributes=0;
	}
	if (pRPPI)
	{
		if (!(FList.F[FList.iCount].FileName))
		{
			FList.F[FList.iCount].FileName=(wchar_t*)malloc((wcslen(pRPPI->FileName)+1)*sizeof(wchar_t));
			if (FList.F[FList.iCount].FileName) wcscpy(FList.F[FList.iCount].FileName,pRPPI->FileName);
		}
		FList.F[FList.iCount].ftRLastWriteTime.dwLowDateTime=pRPPI->LastWriteTime.dwLowDateTime;
		FList.F[FList.iCount].ftRLastWriteTime.dwHighDateTime=pRPPI->LastWriteTime.dwHighDateTime;
		FList.F[FList.iCount].nRFileSize=pRPPI->FileSize;
		FList.F[FList.iCount].dwRAttributes=pRPPI->FileAttributes;
	}
	else
	{
		FList.F[FList.iCount].ftRLastWriteTime.dwLowDateTime=0;
		FList.F[FList.iCount].ftRLastWriteTime.dwHighDateTime=0;
		FList.F[FList.iCount].nRFileSize=0;
		FList.F[FList.iCount].dwRAttributes=0;
	}

	FList.iCount++;

	return true;
}

int __cdecl SortList(const void *el1, const void *el2)
{
	struct File *Item1=(struct File *)el1, *Item2=(struct File *)el2;

	int cmp=FSF.LStricmp(Item1->LDir,Item2->LDir);

	if (!cmp)
	{
		DWORD dwAttributes1=!(Item1->dwFlags&RCIF_LUNIQ)?Item1->dwRAttributes:Item1->dwLAttributes;
		DWORD dwAttributes2=!(Item2->dwFlags&RCIF_LUNIQ)?Item2->dwRAttributes:Item2->dwLAttributes;

		if (dwAttributes1&FILE_ATTRIBUTE_DIRECTORY)
		{
			if (!(dwAttributes2&FILE_ATTRIBUTE_DIRECTORY))
				return 1;
		}
		else
		{
			if (dwAttributes2&FILE_ATTRIBUTE_DIRECTORY)
				return -1;
		}
		cmp=FSF.LStricmp(Item1->FileName,Item2->FileName);
	}
	return cmp;
}

void MakeListItemText(wchar_t *buf, File *cur, wchar_t Mark)
{
	wchar_t LTime[20]={0}, RTime[20]={0};

	if (!(cur->dwFlags&RCIF_RUNIQ))  // есть элемент слева
		GetStrFileTime(&cur->ftLLastWriteTime,LTime,false);
	if (!(cur->dwFlags&RCIF_LUNIQ))  // есть элемент справа
		GetStrFileTime(&cur->ftRLastWriteTime,RTime,false);

	wchar_t LSize[65]={0}, RSize[65]={0};
	const int nSIZE=32; // размер ячейки для FSF.sprintf(%*.*)

	if (!(cur->dwLAttributes&FILE_ATTRIBUTE_DIRECTORY) && !(cur->dwRAttributes&FILE_ATTRIBUTE_DIRECTORY))
	{
		if (LTime[0]) cur->nLFileSize>99999999999?FSF.itoa64(cur->nLFileSize,LSize,10):itoaa(cur->nLFileSize,LSize);
		if (RTime[0]) cur->nRFileSize>99999999999?FSF.itoa64(cur->nRFileSize,RSize,10):itoaa(cur->nRFileSize,RSize);
	}
	else
	{
		if (LTime[0]) strcentr(LSize,GetMsg(MFolder),nSIZE,L' ');
		if (RTime[0]) strcentr(RSize,GetMsg(MFolder),nSIZE,L' ');
	}

	if (cur->dwLAttributes&FILE_ATTRIBUTE_DIRECTORY || cur->dwRAttributes&FILE_ATTRIBUTE_DIRECTORY)
	{
		string strDir;
			if (LTime[0])
		strDir=GetPosToName(cur->LDir)+wcslen(LPanel.Dir);
			if (RTime[0])
		strDir=GetPosToName(cur->RDir)+wcslen(RPanel.Dir);
		strDir+=L"\\";
		strDir+=cur->FileName;
		if (strDir.length()>0 && strDir[(size_t)(strDir.length()-1)]!=L'\\') strDir+=L"\\";

		FSF.sprintf(buf, L"%*.*s%c%*.*s%c%c%c%s",nSIZE,nSIZE,LSize,0x2551,nSIZE,nSIZE,RSize,0x2551,Mark,0x2502,strDir.get());
	}
	else
		FSF.sprintf(buf, L"%*.*s%c%*.*s%c%*.*s%c%*.*s%c%c%c%s",14,14,LSize,0x2502,17,17,LTime,0x2551,17,17,RTime,0x2502,14,14,RSize,0x2551,Mark,0x2502,cur->FileName);
}

void SetBottom(HANDLE hDlg, FileList *pFileList, wchar_t *CurDir=NULL)
{
	static wchar_t Title[MAX_PATH];
	static wchar_t Bottom[MAX_PATH];
	FarListTitles ListTitle;
	ListTitle.Title=Title;
	ListTitle.TitleSize=MAX_PATH;
	ListTitle.Bottom=Bottom;
	ListTitle.BottomSize=MAX_PATH;
	Info.SendDlgMessage(hDlg,DM_LISTGETTITLES,0,&ListTitle);
	if (CurDir) FSF.sprintf(Bottom,GetMsg(MListBottomCurDir),CurDir);
	else FSF.sprintf(Bottom,GetMsg(MListBottom),pFileList->Items,pFileList->Select,pFileList->bShowSelect?L' ':L'*',
								pFileList->Identical,pFileList->bShowIdentical?L' ':L'*',
								pFileList->Different,pFileList->bShowDifferent?L' ':L'*',
								pFileList->LNew,pFileList->bShowLNew?L' ':L'*',pFileList->RNew,pFileList->bShowRNew?L' ':L'*');
	ListTitle.Bottom=Bottom;
	Info.SendDlgMessage(hDlg,DM_LISTSETTITLES,0,&ListTitle);
}

/***************************************************************************
 * Изменение/обновление листа файлов в диалоге
 ***************************************************************************/
bool MakeFarList(HANDLE hDlg, FileList *pFileList, bool bSetCurPos=true, bool bSort=false)
{
	pFileList->Items=0;
	pFileList->Select=0;
	pFileList->Identical=0;
	pFileList->Different=0;
	pFileList->LNew=0;
	pFileList->RNew=0;

	// запросим информацию
	FarListInfo ListInfo;
	Info.SendDlgMessage(hDlg,DM_LISTINFO,0,&ListInfo);

	if (ListInfo.ItemsNumber)
		Info.SendDlgMessage(hDlg,DM_LISTDELETE,0,0);

	if (!pFileList->iCount)
		return true;

	// сортируем только при инициализации
	if (bSort)
		FSF.qsort(pFileList->F,pFileList->iCount,sizeof(pFileList->F[0]),SortList);

	int Index=0;
	wchar_t buf[65536];
	const int nSIZE=32; // размер ячейки для FSF.sprintf(%*.*)

	for (int i=0; i<pFileList->iCount; i++)
	{
		File *cur=&pFileList->F[i];

		if (pFileList->bClearUserFlags)
			cur->dwFlags&= ~RCIF_USER;

		wchar_t Mark=L' ';

		if ((cur->dwLAttributes&FILE_ATTRIBUTE_DIRECTORY) && (cur->dwRAttributes&FILE_ATTRIBUTE_DIRECTORY))
			continue;
		if ((cur->dwFlags&RCIF_USERSELECT) && !pFileList->bShowSelect)
			continue;
		if (cur->dwFlags&RCIF_EQUAL)
		{
			if (!pFileList->bShowIdentical)
				continue;
			Mark=L'='; pFileList->Identical++;
		}
		if (cur->dwFlags&RCIF_DIFFER)
		{
			if (!pFileList->bShowDifferent)
				continue;
			if (cur->dwFlags&RCIF_USERLNEW) Mark=0x25ba;
			else if (!Opt.SyncOnlyRight && (cur->dwFlags&RCIF_USERRNEW)) Mark=0x25c4;
			else Mark=0x2260;
			pFileList->Different++;
		}
		if (cur->dwFlags&RCIF_LNEW)
		{
			if (!pFileList->bShowLNew)
				continue;
			if (!Opt.SyncOnlyRight && (cur->dwFlags&RCIF_USERRNEW)) Mark=0x25c4;
			else if (!(cur->dwFlags&RCIF_USERNONE)) Mark=0x2192;
			pFileList->LNew++;
		}
		if (cur->dwFlags&RCIF_RNEW)
		{
			if (!pFileList->bShowRNew)
				continue;
			if (cur->dwFlags&RCIF_USERLNEW) Mark=0x25ba;
			else if (!Opt.SyncOnlyRight && !(cur->dwFlags&RCIF_USERNONE)) Mark=0x2190;
			else if (Opt.SyncOnlyRight)
			{
				if (cur->dwFlags&RCIF_RUNIQ) { cur->dwFlags|=RCIF_USERDEL; Mark=L'x'; }
				else cur->dwFlags|=RCIF_USERNONE;
			}
			if (!Opt.SyncOnlyRight || (Opt.SyncOnlyRight && (cur->dwFlags&RCIF_RUNIQ))) pFileList->RNew++;
		}

		// виртуальная папка
		bool bAddVirtDir=false;
		string strVirtDir;

		// если попали сразу в подкаталог... добавим виртуальную папку в начало
		if (!Index && FSF.LStricmp(GetPosToName(cur->LDir),LPanel.Dir))
		{
			strVirtDir=GetPosToName(cur->LDir)+wcslen(LPanel.Dir);
			if (strVirtDir.length()>0 && strVirtDir[(size_t)(strVirtDir.length()-1)]!=L'\\') strVirtDir+=L"\\";
			struct FarListItem Item;
			Item.Flags=LIF_DISABLE;
			wchar_t Size[65];
			strcentr(Size,GetMsg(MFolder),nSIZE,L' ');
			FSF.sprintf(buf, L"%*.*s%c%*.*s%c%c%c%s",nSIZE,nSIZE,Size,0x2551,nSIZE,nSIZE,Size,0x2551,L' ',0x2502,strVirtDir.get());
			Item.Text=buf;
			Item.Reserved[0]=Item.Reserved[1]=Item.Reserved[2]=0;
			struct FarList List;
			List.ItemsNumber=1;
			List.Items=&Item;
			Info.SendDlgMessage(hDlg,DM_LISTADD,0,&List);
			Index++;
		}

		{
			for (int j=i+1; j<pFileList->iCount; j++)
			{
				File *next=&pFileList->F[j];

				if (pFileList->bClearUserFlags)
					next->dwFlags&= ~RCIF_USER; // все равно же скидывать

				if ((next->dwLAttributes&FILE_ATTRIBUTE_DIRECTORY) && (next->dwRAttributes&FILE_ATTRIBUTE_DIRECTORY))
					continue;
				if ((next->dwFlags&RCIF_USERSELECT) && !pFileList->bShowSelect)
					continue;
				if ((next->dwFlags&RCIF_EQUAL) && !pFileList->bShowIdentical)
					continue;
				if ((next->dwFlags&RCIF_DIFFER) && !pFileList->bShowDifferent)
					continue;
				if ((next->dwFlags&RCIF_LNEW) && !pFileList->bShowLNew)
					continue;
				if ((next->dwFlags&RCIF_RNEW) && !pFileList->bShowRNew)
					continue;

				if (FSF.LStricmp(cur->LDir,next->LDir))
				{
					bAddVirtDir=true;
					strVirtDir=GetPosToName(next->LDir)+wcslen(LPanel.Dir);
					if (strVirtDir.length()>0 && strVirtDir[(size_t)(strVirtDir.length()-1)]!=L'\\') strVirtDir+=L"\\";
				}
				break;
			}
		}

		MakeListItemText(buf,cur,Mark);

		struct FarListItem Item;
		Item.Flags=0;
		if (cur->dwFlags&RCIF_EQUAL)
			Item.Flags|=LIF_GRAYED;
		if (cur->dwFlags&RCIF_USERSELECT)
		{
			Item.Flags|=LIF_CHECKED;
			pFileList->Select++;
		}
		Item.Text=buf;
		Item.Reserved[0]=Item.Reserved[1]=Item.Reserved[2]=0;
		struct FarList List;
		List.ItemsNumber=1;
		List.Items=&Item;

		// если удачно добавили элемент...
		if (Info.SendDlgMessage(hDlg,DM_LISTADD,0,&List))
		{
			pFileList->Items++;
			// ... то ассоциируем данные с элементом листа
			struct FarListItemData Data;
			Data.Index=Index++;
			Data.DataSize=sizeof(cur);
			Data.Data=&cur;
			Info.SendDlgMessage(hDlg,DM_LISTSETDATA,0,&Data);
		}

		if (bAddVirtDir)
		{
			wchar_t Size[65];
			strcentr(Size,GetMsg(MFolder),nSIZE,L' ');
			FSF.sprintf(buf, L"%*.*s%c%*.*s%c%c%c%s",nSIZE,nSIZE,Size,0x2551,nSIZE,nSIZE,Size,0x2551,L' ',0x2502,strVirtDir.get());
			Item.Flags=LIF_DISABLE;
			Item.Text=buf;
			List.Items=&Item;
			Info.SendDlgMessage(hDlg,DM_LISTADD,0,&List);
			Index++;
		}
	}

	SetBottom(hDlg,pFileList);

	if (bSetCurPos)
	{
		FarListPos ListPos;
		ListPos.SelectPos=0; //ListInfo.SelectPos;
		ListPos.TopPos=-1/*ListInfo.TopPos*/;
		Info.SendDlgMessage(hDlg,DM_LISTSETCURPOS,0,&ListPos);
	}
	return true;
}

int GetSyncOpt(FileList *pFileList)
{
	int ret=-1; // продолжаем редактировать список
	int ItemsLNew=0, ItemsRNew=0, ItemsRDel=0;

	for (int i=0; i<pFileList->iCount; i++)
	{
		File *cur=&pFileList->F[i];

		if ((cur->dwLAttributes&FILE_ATTRIBUTE_DIRECTORY) && (cur->dwRAttributes&FILE_ATTRIBUTE_DIRECTORY))
			continue;
		if ( ((cur->dwFlags&RCIF_USERSELECT) && !pFileList->bShowSelect) || ((cur->dwFlags&RCIF_DIFFER) && !pFileList->bShowDifferent) ||
				 ((cur->dwFlags&RCIF_LNEW) && !pFileList->bShowLNew) || ((cur->dwFlags&RCIF_RNEW) && !pFileList->bShowRNew) )
			continue;
		if ((cur->dwFlags&RCIF_EQUAL) || ((cur->dwFlags&RCIF_DIFFER) && !(cur->dwFlags&(RCIF_USERLNEW|RCIF_USERRNEW))) || (cur->dwFlags&RCIF_USERNONE))
			continue;
		if ((cur->dwFlags&RCIF_USERLNEW) || (cur->dwFlags&RCIF_LNEW))
		{
			ItemsLNew++;
			continue;
		}
		if (cur->dwFlags&RCIF_USERDEL)
		{
			ItemsRDel++;
			continue;
		}
		if (!Opt.SyncOnlyRight && ((cur->dwFlags&RCIF_USERRNEW) || (cur->dwFlags&RCIF_RNEW)))
		{
			ItemsRNew++;
		}
	}

	// нет элементов для синхронизации, выходим
	if (!ItemsLNew && !ItemsRNew && !ItemsRDel)
	{
		if (Opt.ShowMsg)
		{
			const wchar_t *MsgItems[] = { GetMsg(MSyncTitle), GetMsg(MNoSyncBody), GetMsg(MOK) };
			Info.Message(&MainGuid,&NoSyncMsgGuid,0,0,MsgItems,sizeof(MsgItems) / sizeof(MsgItems[0]),1);
		}
		return ret=1; //нет элементов
	}

	Opt.SyncLPanel=ItemsRNew, Opt.SyncRPanel=ItemsLNew, Opt.SyncRDel=ItemsRDel;

	wchar_t buf1[80], buf2[80], buf3[80];
	FSF.sprintf(buf1,GetMsg(MSyncLPanel),ItemsRNew);
	FSF.sprintf(buf2,GetMsg(MSyncRPanel),ItemsLNew);
	FSF.sprintf(buf3,GetMsg(MSyncRDel),ItemsRDel);

	struct FarDialogItem DialogItems[] = {
		//			Type	X1	Y1	X2	Y2				Selected	History	Mask	Flags	Data	MaxLen	UserParam
		/* 0*/{DI_DOUBLEBOX,  3, 1,60, 8,         0, 0, 0,             0, GetMsg(MSyncTitle),0,0},
		/* 1*/{DI_CHECKBOX,   5, 2, 0, 0, Opt.SyncRPanel, 0, 0,Opt.SyncRPanel?0:DIF_DISABLE,buf2,0,0},
		/* 2*/{DI_CHECKBOX,   5, 3, 0, 0, Opt.SyncLPanel, 0, 0,Opt.SyncLPanel?0:DIF_DISABLE,buf1,0,0},
		/* 3*/{DI_CHECKBOX,   5, 4, 0, 0, Opt.SyncRDel, 0, 0, Opt.SyncRDel?0:DIF_DISABLE,buf3,0,0},
		/* 4*/{DI_CHECKBOX,   8, 5, 0, 0,         0, 0, 0, Opt.SyncRDel?0:DIF_DISABLE,GetMsg(MSyncUseDelFilter),0,0},
		/* 5*/{DI_TEXT,      -1, 6, 0, 0,         0, 0, 0, DIF_SEPARATOR, L"",0,0},
		/* 6*/{DI_BUTTON,     0, 7, 0, 0,         0, 0, 0, DIF_DEFAULTBUTTON|DIF_CENTERGROUP, GetMsg(MOK),0,0},
		/* 7*/{DI_BUTTON,     0, 7, 0, 0,         0, 0, 0, DIF_CENTERGROUP, GetMsg(MSyncEdit),0,0},
		/* 8*/{DI_BUTTON,     0, 7, 0, 0,         0, 0, 0, DIF_CENTERGROUP, GetMsg(MCancel),0,0}
	};

	HANDLE hDlg=Info.DialogInit(&MainGuid, &OptSyncDlgGuid,-1,-1,64,10,L"DlgCmp",DialogItems,sizeof(DialogItems)/sizeof(DialogItems[0]),0,0,0,0);

	if (hDlg != INVALID_HANDLE_VALUE)
	{
		ret=Info.DialogRun(hDlg);
		if (ret==6)
		{
			Opt.SyncRPanel=Info.SendDlgMessage(hDlg,DM_GETCHECK,1,0);
			Opt.SyncLPanel=Info.SendDlgMessage(hDlg,DM_GETCHECK,2,0);
			Opt.SyncRDel=Info.SendDlgMessage(hDlg,DM_GETCHECK,3,0);
			Opt.SyncUseDelFilter=Info.SendDlgMessage(hDlg,DM_GETCHECK,4,0);
			ret=(Opt.SyncLPanel || Opt.SyncRPanel || Opt.SyncRDel)?2:1;   // синхронизируем, иначе - пропустим
		}
		else if (ret==7)
			ret=-1;
		else
		{
			bBrokenByEsc=true;
			ret=0; // отменили синхронизацию
		}
		Info.DialogFree(hDlg);
	}
	else
		ret=0;

	return ret;
}

bool bSetBottom=false;

INT_PTR WINAPI ShowCmpDialogProc(HANDLE hDlg,int Msg,int Param1,void *Param2)
{
	FileList *pFileList=(FileList *)Info.SendDlgMessage(hDlg,DM_GETDLGDATA,0,0);
	switch(Msg)
	{
		case DN_INITDIALOG:
			MakeFarList(hDlg,pFileList,true,true);
			Info.SendDlgMessage(hDlg, DM_SETMOUSEEVENTNOTIFY, 1, 0);
			break;

	/************************************************************************/

		case DN_RESIZECONSOLE:
		{
			COORD c=(*(COORD*)Param2);
			Info.SendDlgMessage(hDlg,DM_ENABLEREDRAW,false,0);
			Info.SendDlgMessage(hDlg,DM_RESIZEDIALOG,0,&c);
			WinInfo.Con.Right=c.X-1;
			WinInfo.Con.Bottom=c.Y-1;
			Info.SendDlgMessage(hDlg,DM_SETITEMPOSITION,0,&WinInfo.Con);
			c.X=c.Y=-1;
			Info.SendDlgMessage(hDlg,DM_MOVEDIALOG,true,&c);
			Info.SendDlgMessage(hDlg,DM_ENABLEREDRAW,true,0);
			return true;
		}

	/************************************************************************/

		case DN_CTLCOLORDLGLIST:
			if (Param1==0)
			{
				struct FarDialogItemColors *Colors=(FarDialogItemColors*)Param2;
				FarColor Color;
				int ColorIndex[15]=
				{
					COL_PANELTEXT,
					COL_PANELBOX,
					COL_PANELBOX,             // заголовки
					COL_PANELTEXT,            // элемент списка
					COL_PANELSELECTEDTEXT,
					COL_PANELBOX,
					COL_PANELCURSOR,          // под курсором
					COL_PANELSELECTEDCURSOR,
					COL_PANELSCROLLBAR,
					COL_PANELHIGHLIGHTTEXT,   // виртуальная папка
					COL_PANELSELECTEDTEXT,
					COL_PANELSELECTEDCURSOR,
					COL_PANELSELECTEDTEXT,
					COL_PANELHIGHLIGHTTEXT,   // одинаковые элементы
					COL_PANELCURSOR
				};
				for (int i=0; i<15; i++)
				{
					Info.AdvControl(&MainGuid,ACTL_GETCOLOR,ColorIndex[i],&Color);
					Colors->Colors[i] = Color;
					if (i==9) // виртуальная папка
					{
						int color=0x1F;
						Colors->Colors[i].Flags=FCF_FG_4BIT|FCF_BG_4BIT;
						Colors->Colors[i].ForegroundColor=color;
						Colors->Colors[i].BackgroundColor=color>>4;
					}
				}
				return true;
			}

	/************************************************************************/

		case DN_CLOSE:
			if (Opt.Sync && Param1==-1)
			{
				Opt.Sync=GetSyncOpt(pFileList);
				if (Opt.Sync==-1)
					return false;
				else
					return true;
			}
			break;

	/************************************************************************/

		case DN_INPUT:
		{
			const INPUT_RECORD* record=(const INPUT_RECORD *)Param2;

			if (record->EventType==MOUSE_EVENT)
				// отработаем щелчок мыши в поле Mark
				if (Opt.Sync && Param1==0 && record->Event.MouseEvent.dwButtonState==FROM_LEFT_1ST_BUTTON_PRESSED && record->Event.MouseEvent.dwEventFlags!=DOUBLE_CLICK)
				{
					SMALL_RECT dlgRect;
					Info.SendDlgMessage(hDlg, DM_GETDLGRECT, 0, &dlgRect);
					// щелкнули в LISTе
					if (record->Event.MouseEvent.dwMousePosition.X>dlgRect.Left && record->Event.MouseEvent.dwMousePosition.X<dlgRect.Right
					&& record->Event.MouseEvent.dwMousePosition.Y>dlgRect.Top && record->Event.MouseEvent.dwMousePosition.Y<dlgRect.Bottom)
					{
						FarListPos ListPos;
						Info.SendDlgMessage(hDlg, DM_LISTGETCURPOS, 0, &ListPos);
						int OldPos=ListPos.SelectPos;
						ListPos.SelectPos=ListPos.TopPos+(record->Event.MouseEvent.dwMousePosition.Y-1-dlgRect.Top);
						Info.SendDlgMessage(hDlg,DM_ENABLEREDRAW,false,0);
						int NewPos=Info.SendDlgMessage(hDlg, DM_LISTSETCURPOS, 0, &ListPos);
						if (NewPos!=ListPos.SelectPos)
						{
							ListPos.SelectPos=OldPos;
							Info.SendDlgMessage(hDlg, DM_LISTSETCURPOS, 0, &ListPos);
							MessageBeep(MB_OK);
						}
						Info.SendDlgMessage(hDlg,DM_ENABLEREDRAW,true,0);
						// вот оно, поле Mark
						if (NewPos==ListPos.SelectPos && record->Event.MouseEvent.dwMousePosition.X>=dlgRect.Left+68 && record->Event.MouseEvent.dwMousePosition.X<=dlgRect.Left+70)
						{
							goto GOTOCHANGEMARK;
							return false;
						}
					}
				}
				else if (Param1==0 && record->Event.MouseEvent.dwButtonState==FROM_LEFT_1ST_BUTTON_PRESSED && record->Event.MouseEvent.dwEventFlags==DOUBLE_CLICK)
				{
					SMALL_RECT dlgRect;
					Info.SendDlgMessage(hDlg, DM_GETDLGRECT, 0, &dlgRect);
					// не нужно поле Mark
					if (!Opt.Sync || record->Event.MouseEvent.dwMousePosition.X<dlgRect.Left+68 || record->Event.MouseEvent.dwMousePosition.X>dlgRect.Left+70)
					{
						goto GOTOCMPFILE;
						return false;
					}
				}
				// закроем диалог по клику в правом верхнем угу
				else if (Param1==0 && record->Event.MouseEvent.dwButtonState==FROM_LEFT_1ST_BUTTON_PRESSED)
				{
					SMALL_RECT dlgRect;
					Info.SendDlgMessage(hDlg, DM_GETDLGRECT, 0, &dlgRect);
					if (record->Event.MouseEvent.dwMousePosition.X==dlgRect.Right && record->Event.MouseEvent.dwMousePosition.Y==dlgRect.Top)
					{
						Info.SendDlgMessage(hDlg,DM_CLOSE,0,0);
						return false;
					}
				}

				return true;
		}

	/************************************************************************/

		case DN_CONTROLINPUT:
		{
			const INPUT_RECORD* record=(const INPUT_RECORD *)Param2;

			if (bSetBottom)
			{
				SetBottom(hDlg,pFileList);
				bSetBottom=false;
			}

			if (record->EventType==KEY_EVENT && record->Event.KeyEvent.bKeyDown)
			{
				WORD vk=record->Event.KeyEvent.wVirtualKeyCode;

				if (IsNone(record))
				{
#if 0
					if (vk==VK_F3)
					{
						wchar_t LName[4][66], RName[4][66];

						for (int i=0; i<4; i++)
						{
							LName[i][0]=0;
							RName[i][0]=0;
						}
						int Pos=Info.SendDlgMessage(hDlg,DM_LISTGETCURPOS,0,0);
						File **tmp=(File **)Info.SendDlgMessage(hDlg,DM_LISTGETDATA,0,(void *)Pos);
						File *cur=(tmp && *tmp)?*tmp:NULL;
						if (cur)
						{
							for (int j=0;j<=1;j++)
							{
								wchar_t *Name=GetPosToName((j==0)?cur->LDir:cur->RDir);
								int len=wcslen(Name);
								for (int i=0; i<4; i++, len-=65)
								{
									if (len<65)
									{
										lstrcpyn((j==0)?LName[i]:RName[i], Name+i*65, len+1);
										break;
									}
									else
										lstrcpyn((j==0)?LName[i]:RName[i], Name+i*65, 66);
								}
							}
						}
						struct FarDialogItem DialogItems[] = {
							//			Type	X1	Y1	X2	Y2	Selected	History	Mask	Flags	Data	MaxLen	UserParam	
							/* 0*/{DI_DOUBLEBOX,0, 0,70,14, 0, 0, 0,                  0, GetMsg(MCurDirName), 0,0},
							/* 1*/{DI_SINGLEBOX,2, 1,68, 6, 0, 0, 0,       DIF_LEFTTEXT, GetMsg(MLName), 0,0},
							/* 2*/{DI_TEXT,     3, 2, 0, 0, 0, 0, 0,       DIF_LEFTTEXT, LName[0],0,0},
							/* 3*/{DI_TEXT,     3, 3, 0, 0, 0, 0, 0,       DIF_LEFTTEXT, LName[1],0,0},
							/* 4*/{DI_TEXT,     3, 4, 0, 0, 0, 0, 0,       DIF_LEFTTEXT, LName[2],0,0},
							/* 5*/{DI_TEXT,     3, 5, 0, 0, 0, 0, 0,       DIF_LEFTTEXT, LName[3],0,0},
							/* 6*/{DI_SINGLEBOX,2, 7,68,12, 0, 0, 0,       DIF_LEFTTEXT, GetMsg(MRName), 0,0},
							/* 7*/{DI_TEXT,     3, 8, 0, 0, 0, 0, 0,       DIF_LEFTTEXT, RName[0],0,0},
							/* 8*/{DI_TEXT,     3, 9, 0, 0, 0, 0, 0,       DIF_LEFTTEXT, RName[1],0,0},
							/* 9*/{DI_TEXT,     3,10, 0, 0, 0, 0, 0,       DIF_LEFTTEXT, RName[2],0,0},
							/*10*/{DI_TEXT,     3,11, 0, 0, 0, 0, 0,       DIF_LEFTTEXT, RName[3],0,0},
							/*11*/{DI_BUTTON,   0,13, 0, 0, 0, 0, 0,DIF_CENTERGROUP|DIF_DEFAULTBUTTON, GetMsg(MOK),0,0},
						};
						HANDLE hDlg=Info.DialogInit(&MainGuid,&DlgNameGuid,-1,-1,71,15,L"DlgCmp", DialogItems,sizeof(DialogItems)/sizeof(DialogItems[0]),0,FDLG_SMALLDIALOG,0,0);
						if (hDlg != INVALID_HANDLE_VALUE)
						{
							Info.DialogRun(hDlg);
							Info.DialogFree(hDlg);
						}
						return true;
					}
					//-----
					else
#endif
					if (vk==VK_RETURN)
					{
GOTOCMPFILE:
						int Pos=Info.SendDlgMessage(hDlg,DM_LISTGETCURPOS,0,0);
						File **tmp=(File **)Info.SendDlgMessage(hDlg,DM_LISTGETDATA,0,(void *)Pos);
						File *cur=(tmp && *tmp)?*tmp:NULL;
						if (cur)
						{
							if ((LPanel.PInfo.Flags&PFLAGS_PLUGIN) || (RPanel.PInfo.Flags&PFLAGS_PLUGIN) ||
									(cur->dwLAttributes&FILE_ATTRIBUTE_DIRECTORY) || (cur->dwRAttributes&FILE_ATTRIBUTE_DIRECTORY) ||
									(cur->dwFlags&RCIF_LUNIQ) || (cur->dwFlags&RCIF_RUNIQ))
							{
								MessageBeep(MB_OK);
								return true;
							}
							string strLFullFileName, strRFullFileName;
							GetFullFileName(strLFullFileName,cur->LDir,cur->FileName);
							GetFullFileName(strRFullFileName,cur->RDir,cur->FileName);

							if (pCompareFiles)
							{
								pCompareFiles(strLFullFileName.get(),strRFullFileName.get(),0);
							}
							else
							{
								WIN32_FIND_DATA wfdFindData;
								HANDLE hFind;
								wchar_t DiffProgram[MAX_PATH];
								ExpandEnvironmentStringsW(Opt.WinMergePath,DiffProgram,(sizeof(DiffProgram)/sizeof(DiffProgram[0])));
								bool bFindDiffProg=((hFind=FindFirstFileW(DiffProgram,&wfdFindData)) != INVALID_HANDLE_VALUE);
								if (!bFindDiffProg)
								{
									ExpandEnvironmentStringsW(L"%ProgramFiles%\\WinMerge\\WinMergeU.exe",DiffProgram,(sizeof(DiffProgram)/sizeof(DiffProgram[0])));
									bFindDiffProg=((hFind=FindFirstFileW(DiffProgram,&wfdFindData)) != INVALID_HANDLE_VALUE);
								}
								if (bFindDiffProg)
								{
									FindClose(hFind);
									STARTUPINFO si;
									PROCESS_INFORMATION pi;
									memset(&si, 0, sizeof(si));
									si.cb = sizeof(si);
									wchar_t Command[32768];
									FSF.sprintf(Command, L"\"%s\" -e \"%s\" \"%s\"", DiffProgram,GetPosToName(strLFullFileName.get()),GetPosToName(strRFullFileName.get()));
									CreateProcess(0,Command,0,0,false,0,0,0,&si,&pi);
								}
								else
									MessageBeep(MB_ICONASTERISK);
							}
						}
						return true;
					}
					else if (vk==VK_INSERT)
					{
						struct FarListPos FLP;
						Info.SendDlgMessage(hDlg,DM_LISTGETCURPOS,0,&FLP);
						File **tmp=(File **)Info.SendDlgMessage(hDlg,DM_LISTGETDATA,0,(void *)FLP.SelectPos);
						File *cur=(tmp && *tmp)?*tmp:NULL;
						if (cur && pFileList->bShowSelect)
						{
							struct FarListGetItem FLGI;
							FLGI.ItemIndex=FLP.SelectPos;
							if (Info.SendDlgMessage(hDlg,DM_LISTGETITEM,0,&FLGI))
							{
								(FLGI.Item.Flags&LIF_CHECKED)?(FLGI.Item.Flags&= ~LIF_CHECKED):(FLGI.Item.Flags|=LIF_CHECKED);
								struct FarListUpdate FLU;
								FLU.Index=FLGI.ItemIndex;
								FLU.Item=FLGI.Item;
								if (Info.SendDlgMessage(hDlg,DM_LISTUPDATE,0,&FLU))
								{
									if (cur->dwFlags&RCIF_USERSELECT)
									{
										cur->dwFlags&=~RCIF_USERSELECT;
										pFileList->Select--;
									}
									else
									{
										cur->dwFlags|=RCIF_USERSELECT;
										pFileList->Select++;
									}
									FLP.SelectPos++;
									Info.SendDlgMessage(hDlg,DM_LISTSETCURPOS,0,&FLP);

									SetBottom(hDlg,pFileList);
									return true;
								}
							}
						}
						MessageBeep(MB_OK);
						return true;
					}
					else if (vk==VK_SPACE)
					{
GOTOCHANGEMARK:
						int Pos=Info.SendDlgMessage(hDlg,DM_LISTGETCURPOS,0,0);
						File **tmp=(File **)Info.SendDlgMessage(hDlg,DM_LISTGETDATA,0,(void *)Pos);
						File *cur=(tmp && *tmp)?*tmp:NULL;
						if (cur && !(cur->dwFlags&RCIF_EQUAL) && Opt.Sync)
						{
							struct FarListGetItem FLGI;
							FLGI.ItemIndex=Pos;
							if (Info.SendDlgMessage(hDlg,DM_LISTGETITEM,0,&FLGI))
							{
								__int64 Delta=0;
								if (!(cur->dwFlags&RCIF_LUNIQ) && !(cur->dwFlags&RCIF_RUNIQ))
									Delta=(((__int64)cur->ftLLastWriteTime.dwHighDateTime << 32) | cur->ftLLastWriteTime.dwLowDateTime) -
												(((__int64)cur->ftRLastWriteTime.dwHighDateTime << 32) | cur->ftRLastWriteTime.dwLowDateTime);

								if (cur->dwFlags&RCIF_DIFFER)
								{
									if (!(cur->dwFlags&RCIF_USERLNEW) && !(cur->dwFlags&RCIF_USERRNEW))
									{
										if (Delta>=0 || Opt.SyncOnlyRight) cur->dwFlags|=RCIF_USERLNEW;
										else if (!Opt.SyncOnlyRight) cur->dwFlags|=RCIF_USERRNEW;
									}
									else if ((Delta>=0 || Opt.SyncOnlyRight) && (cur->dwFlags&RCIF_USERLNEW))
									{
										cur->dwFlags&=~RCIF_USERLNEW;
										if (!Opt.SyncOnlyRight) cur->dwFlags|=RCIF_USERRNEW;
									}
									else if (Delta<0 && (cur->dwFlags&RCIF_USERRNEW))
									{
										cur->dwFlags|=RCIF_USERLNEW;
										cur->dwFlags&=~RCIF_USERRNEW;
									}
									else
									{
										cur->dwFlags&=~RCIF_USERLNEW;
										cur->dwFlags&=~RCIF_USERRNEW;
									}
								}
								else if (!(cur->dwFlags&RCIF_LUNIQ) && !(cur->dwFlags&RCIF_RUNIQ))
								{
									if (!(cur->dwFlags&RCIF_USERNONE) && !(cur->dwFlags&RCIF_USERLNEW) && !(cur->dwFlags&RCIF_USERRNEW))
									{
										cur->dwFlags|=RCIF_USERNONE;
										cur->dwFlags&=~RCIF_USERLNEW;
										cur->dwFlags&=~RCIF_USERRNEW;
									}
									else if (cur->dwFlags&RCIF_USERNONE)
									{
										cur->dwFlags&=~RCIF_USERNONE;
										if (cur->dwFlags&RCIF_LNEW)
										{
											if (!Opt.SyncOnlyRight) cur->dwFlags|=RCIF_USERRNEW;
										}
										else
										{
											cur->dwFlags|=RCIF_USERLNEW;
										}
									}
									else
									{
										if (Opt.SyncOnlyRight)
										{
											cur->dwFlags|=RCIF_USERNONE;
											cur->dwFlags&=~RCIF_USERLNEW;
										}
										else
										{
											cur->dwFlags&=~RCIF_USERNONE;
											cur->dwFlags&=~RCIF_USERLNEW;
											cur->dwFlags&=~RCIF_USERRNEW;
										}
									}
								}
								else
								{
									if (cur->dwFlags&RCIF_USERNONE)
									{
										cur->dwFlags&= ~RCIF_USERNONE;
										if (Opt.SyncOnlyRight && (cur->dwFlags&RCIF_RNEW)) cur->dwFlags|=RCIF_USERDEL;
									}
									else
									{
										cur->dwFlags|=RCIF_USERNONE;
										cur->dwFlags&=~RCIF_USERDEL;
									}
								}

								wchar_t buf[65536];
								wchar_t Mark=L' ';
								if (cur->dwFlags&RCIF_DIFFER)
								{
									if (cur->dwFlags&RCIF_USERLNEW) Mark=0x25ba;
									else if (!Opt.SyncOnlyRight && (cur->dwFlags&RCIF_USERRNEW)) Mark=0x25c4;
									else Mark=0x2260;
								}
								else if (cur->dwFlags&RCIF_LNEW)
								{
									if (!Opt.SyncOnlyRight && (cur->dwFlags&RCIF_USERRNEW)) Mark=0x25c4;
									else if (!(cur->dwFlags&RCIF_USERNONE)) Mark=0x2192;
								}
								else if (cur->dwFlags&RCIF_RNEW)
								{
									if (cur->dwFlags&RCIF_USERLNEW) Mark=0x25ba;
									else if (!Opt.SyncOnlyRight && !(cur->dwFlags&RCIF_USERNONE)) Mark=0x2190;
									else if (cur->dwFlags&RCIF_USERDEL) Mark=L'x';
								}
								MakeListItemText(buf,cur,Mark);
								struct FarListUpdate FLU;
								FLU.Index=FLGI.ItemIndex;
								FLU.Item=FLGI.Item;
								FLU.Item.Text=buf;
								if (Info.SendDlgMessage(hDlg,DM_LISTUPDATE,0,&FLU))
									return true;
							}
						}
						MessageBeep(MB_OK);
						return true;
					}
				}
				else if (IsCtrl(record))
				{
					if (vk==VK_CONTROL)
					{
						int Pos=Info.SendDlgMessage(hDlg,DM_LISTGETCURPOS,0,0);
						File **tmp=(File **)Info.SendDlgMessage(hDlg,DM_LISTGETDATA,0,(void *)Pos);
						File *cur=(tmp && *tmp)?*tmp:NULL;
						if (cur)
						{
							string strVirtDir=GetPosToName(cur->LDir)+wcslen(LPanel.Dir);
							if (strVirtDir.length()>0)
								FSF.TruncStr(strVirtDir.get(),WinInfo.TruncLen-wcslen(GetMsg(MListBottomCurDir)));
							else
							{
								MessageBeep(MB_OK);
								return true;
							}
							strVirtDir.updsize();
							SetBottom(hDlg,pFileList,strVirtDir.get());
							bSetBottom=true;
							return true;
						}
					}
					else if (vk==0x52) //VK_R
					{
						Info.SendDlgMessage(hDlg,DM_ENABLEREDRAW,false,0);
						pFileList->bShowSelect=true;
						pFileList->bShowIdentical=true;
						pFileList->bShowDifferent=true;
						pFileList->bShowLNew=true;
						pFileList->bShowRNew=true;
						pFileList->bClearUserFlags=true;
						MakeFarList(hDlg,pFileList);
						pFileList->bClearUserFlags=false; // восстановим!
						Info.SendDlgMessage(hDlg,DM_ENABLEREDRAW,true,0);
						return true;
					}
					else if (vk==VK_OEM_5 && !(pFileList->bShowSelect && pFileList->Select==0))
					{
						Info.SendDlgMessage(hDlg,DM_ENABLEREDRAW,false,0);
						pFileList->bShowSelect=(pFileList->bShowSelect?false:true);
						MakeFarList(hDlg,pFileList);
						Info.SendDlgMessage(hDlg,DM_ENABLEREDRAW,true,0);
						return true;
					}
					else if (vk==VK_OEM_PLUS && !(pFileList->bShowIdentical && pFileList->Identical==0))
					{
						Info.SendDlgMessage(hDlg,DM_ENABLEREDRAW,false,0);
						pFileList->bShowIdentical=(pFileList->bShowIdentical?false:true);
						MakeFarList(hDlg,pFileList);
						Info.SendDlgMessage(hDlg,DM_ENABLEREDRAW,true,0);
						return true;
					}
					else if (vk==VK_OEM_MINUS && !(pFileList->bShowDifferent && pFileList->Different==0))
					{
						Info.SendDlgMessage(hDlg,DM_ENABLEREDRAW,false,0);
						pFileList->bShowDifferent=(pFileList->bShowDifferent?false:true);
						MakeFarList(hDlg,pFileList);
						Info.SendDlgMessage(hDlg,DM_ENABLEREDRAW,true,0);
						return true;
					}
					else if (vk==VK_OEM_4 && !(pFileList->bShowLNew && pFileList->LNew==0))
					{
						Info.SendDlgMessage(hDlg,DM_ENABLEREDRAW,false,0);
						pFileList->bShowLNew=(pFileList->bShowLNew?false:true);
						MakeFarList(hDlg,pFileList);
						Info.SendDlgMessage(hDlg,DM_ENABLEREDRAW,true,0);
						return true;
					}
					else if (vk==VK_OEM_6 && !(pFileList->bShowRNew && pFileList->RNew==0))
					{
						Info.SendDlgMessage(hDlg,DM_ENABLEREDRAW,false,0);
						pFileList->bShowRNew=(pFileList->bShowRNew?false:true);
						MakeFarList(hDlg,pFileList);
						Info.SendDlgMessage(hDlg,DM_ENABLEREDRAW,true,0);
						return true;
					}
					else if (vk==VK_PRIOR)
					{
						int Pos=Info.SendDlgMessage(hDlg,DM_LISTGETCURPOS,0,0);
						File **tmp=(File **)Info.SendDlgMessage(hDlg,DM_LISTGETDATA,0,(void *)Pos);
						File *cur=(tmp && *tmp)?*tmp:NULL;
						if (cur)
						{
							if ((LPanel.PInfo.Flags&PFLAGS_PLUGIN) || (RPanel.PInfo.Flags&PFLAGS_PLUGIN))
							{
								MessageBeep(MB_OK);
								return true;
							}

							Opt.Sync=0; //скидываем!!!

							PanelRedrawInfo LRInfo={0,0}, RRInfo={0,0};
							bool bSetLDir=false, bSetLFile=false;
							bool bSetRDir=false, bSetRFile=false;
/*
						if (LPanel.PInfo.Flags&PFLAGS_PLUGIN)
						{
DebugMsg(strLPanelDir.get(),L"strLPanelDir.get()");
DebugMsg(cur->LDir,L"cur->LDir");
							strLPanelDir=cur->FileName;
							(wchar_t)*(FSF.PointToName(strLPanelDir.get())) = 0;
DebugMsg(strLPanelDir.get(),L"strLPanelDir.get()");
						}
*/
							if (FSF.LStricmp(LPanel.Dir,GetPosToName(cur->LDir)))
							{
								FarPanelDirectory dirInfo={sizeof(FarPanelDirectory),GetPosToName(cur->LDir),NULL,{0},NULL};
								bSetLDir=Info.PanelControl(LPanel.hPanel,FCTL_SETPANELDIRECTORY,0,&dirInfo);
								Info.PanelControl(LPanel.hPanel,FCTL_BEGINSELECTION,0,0);
							}
							{
								PanelInfo PInfo;
								PInfo.StructSize=sizeof(PanelInfo);
								Info.PanelControl(LPanel.hPanel,FCTL_GETPANELINFO,0,&PInfo);
								FarGetPluginPanelItem FGPPI;

								for (int i=0; i<PInfo.ItemsNumber; i++)
								{
									FGPPI.Size=0; FGPPI.Item=0;
									FGPPI.Item=(PluginPanelItem*)malloc(FGPPI.Size=Info.PanelControl(LPanel.hPanel,FCTL_GETPANELITEM,i,&FGPPI));
									if (FGPPI.Item)
									{
										Info.PanelControl(LPanel.hPanel,FCTL_GETPANELITEM,i,&FGPPI);
										if (!bSetLFile && cur->FileName && !FSF.LStricmp(cur->FileName,FGPPI.Item->FileName))
										{
											LRInfo.CurrentItem=i;
											bSetLFile=true;
										}
										if (bSetLDir)
										{
											for (int j=0; j<pFileList->iCount; j++)
											{
												if (FSF.LStricmp(pFileList->F[j].LDir,cur->LDir))
													continue;
												if (!FSF.LStricmp(pFileList->F[j].FileName,FGPPI.Item->FileName))
												{
													Info.PanelControl(LPanel.hPanel,FCTL_SETSELECTION,i,(void*)((pFileList->F[j].dwFlags&RCIF_DIFFER) || (pFileList->F[j].dwFlags&RCIF_LNEW) || (pFileList->F[j].dwFlags&RCIF_RNEW)));
													break;
												}
											}
										}
										free(FGPPI.Item);
									}
								}
								if (bSetLDir)
									Info.PanelControl(LPanel.hPanel,FCTL_ENDSELECTION,0,0);
							}

							if (FSF.LStricmp(RPanel.Dir,GetPosToName(cur->RDir)))
							{
								FarPanelDirectory dirInfo={sizeof(FarPanelDirectory),GetPosToName(cur->RDir),NULL,{0},NULL};
								bSetRDir=Info.PanelControl(RPanel.hPanel,FCTL_SETPANELDIRECTORY,0,&dirInfo);
								Info.PanelControl(RPanel.hPanel,FCTL_BEGINSELECTION,0,0);
							}
							{
								PanelInfo PInfo;
								PInfo.StructSize=sizeof(PanelInfo);
								Info.PanelControl(RPanel.hPanel,FCTL_GETPANELINFO,0,&PInfo);
								FarGetPluginPanelItem FGPPI;

								for (int i=0; i<PInfo.ItemsNumber; i++)
								{
									FGPPI.Size=0; FGPPI.Item=0;
									FGPPI.Item=(PluginPanelItem*)malloc(FGPPI.Size=Info.PanelControl(RPanel.hPanel,FCTL_GETPANELITEM,i,&FGPPI));
									if (FGPPI.Item)
									{
										Info.PanelControl(RPanel.hPanel,FCTL_GETPANELITEM,i,&FGPPI);
										if (!bSetRFile && cur->FileName && !FSF.LStricmp(cur->FileName,FGPPI.Item->FileName))
										{
											RRInfo.CurrentItem=i;
											bSetRFile=true;
										}
										if (bSetRDir)
										{
											for (int j=0; j<pFileList->iCount; j++)
											{
												if (FSF.LStricmp(pFileList->F[j].RDir,cur->RDir))
													continue;
												if (!FSF.LStricmp(pFileList->F[j].FileName,FGPPI.Item->FileName))
												{
													Info.PanelControl(RPanel.hPanel,FCTL_SETSELECTION,i,(void*)((pFileList->F[j].dwFlags&RCIF_DIFFER) || (pFileList->F[j].dwFlags&RCIF_LNEW) || (pFileList->F[j].dwFlags&RCIF_RNEW)));
													break;
												}
											}
										}
										free(FGPPI.Item);
									}
								}
								if (bSetRDir)
									Info.PanelControl(RPanel.hPanel,FCTL_ENDSELECTION,0,0);
							}
							Info.PanelControl(LPanel.hPanel,FCTL_REDRAWPANEL,0,&LRInfo);
							Info.PanelControl(RPanel.hPanel,FCTL_REDRAWPANEL,0,&RRInfo);
							Info.SendDlgMessage(hDlg,DM_CLOSE,0,0);
						}
						return true;
					}
				}
			}
		}

	}
	return Info.DefDlgProc(hDlg,Msg,Param1,Param2);
}


int AdvCmpProc::ShowCmpDialog(const struct DirList *pLList,const struct DirList *pRList)
{
	FarDialogItem DialogItems[1];
	memset(DialogItems,0,sizeof(DialogItems));

	DialogItems[0].Type=DI_LISTBOX;
	DialogItems[0].X1=0; DialogItems[0].X2=WinInfo.Con.Right;
	DialogItems[0].Y1=0; DialogItems[0].Y2=WinInfo.Con.Bottom-WinInfo.Con.Top;
	DialogItems[0].Flags=DIF_LISTNOCLOSE;

	int len=WinInfo.Con.Right/2-2;

	int size=Info.PanelControl(LPanel.hPanel,FCTL_GETPANELHOSTFILE,0,0);
	string strTmp;
	if (size>1)
	{
		strTmp.get(size);
		Info.PanelControl(LPanel.hPanel,FCTL_GETPANELHOSTFILE,size,strTmp.get());
		strTmp.updsize();
		if (pLList->Dir[0]) strTmp+=L'/';
		strTmp+=pLList->Dir;
	}
	else
		strTmp=GetPosToName(pLList->Dir);
	FSF.TruncPathStr(strTmp.get(),len-2);
	strTmp.updsize();
	string strTmp2=L" ";
	strTmp2+=strTmp+L" ";
	wchar_t buf1[MAX_PATH];
	strcentr(buf1,strTmp2.get(),len,0x00002550);

	size=Info.PanelControl(RPanel.hPanel,FCTL_GETPANELHOSTFILE,0,0);
	if (size>1)
	{
		strTmp.get(size);
		Info.PanelControl(RPanel.hPanel,FCTL_GETPANELHOSTFILE,size,strTmp.get());
		strTmp.updsize();
		if (pRList->Dir[0]) strTmp+=L'/';
		strTmp+=pRList->Dir;
	}
	else
		strTmp=GetPosToName(pRList->Dir);
	FSF.TruncPathStr(strTmp.get(),len-2);
	strTmp.updsize();
	strTmp2=L" ";
	strTmp2+=strTmp+L" ";
	wchar_t buf2[MAX_PATH];
	strcentr(buf2,strTmp2.get(),len,0x00002550);

	wchar_t *buf=(wchar_t*)malloc(WinInfo.Con.Right*sizeof(wchar_t));
	if (buf)
	{
		FSF.sprintf(buf, L"%s %s",buf1,buf2);
		DialogItems[0].Data=buf;
	}

	HANDLE hDlg=Info.DialogInit(&MainGuid,&CmpDlgGuid,0,0,WinInfo.Con.Right,WinInfo.Con.Bottom-WinInfo.Con.Top,L"DlgCmp",DialogItems,
															sizeof(DialogItems)/sizeof(DialogItems[0]),0,FDLG_SMALLDIALOG|FDLG_KEEPCONSOLETITLE,ShowCmpDialogProc,&FList);
	if (hDlg != INVALID_HANDLE_VALUE)
	{
		Info.DialogRun(hDlg);
		Info.DialogFree(hDlg);
	}
	if (buf) free(buf);

	if (Opt.Sync)
		Synchronize(&FList);

	return 1;
}

/***************************************************************************
 *
 *                               СИНХРОНИЗАЦИЯ
 *
 ***************************************************************************/

enum QueryResult
{
	QR_ABORT=-1,
	QR_OVERWRITE=0,
	QR_DELETE=0,
	QR_ALL=1,
	QR_SKIP=2,
	QR_SKIPALL=3
};

int AdvCmpProc::QueryOverwriteFile(const wchar_t *FileName, FILETIME *srcTime, FILETIME *destTime, __int64 srcSize, __int64 destSize, int direction, bool bReadOnlyType)
{
	wchar_t Warning[67], Name[67];
	wchar_t New[67], Existing[67];
	wchar_t Time[20], Size[20];
	const int LEN=66;

	strcentr(Warning,GetMsg(bReadOnlyType?MFileIsReadOnly:MFileAlreadyExists),LEN,L' ');
	TruncCopy(Name,GetPosToName(FileName),LEN);
	FSF.sprintf(New, L"%-30.30s %15.15s %19.19s",direction<0?GetMsg(MNewToL):GetMsg(MNewToR),itoaa(srcSize,Size), GetStrFileTime(srcTime,Time));
	FSF.sprintf(Existing, L"%-30.30s %15.15s %19.19s",GetMsg(MExisting),itoaa(destSize,Size),GetStrFileTime(destTime,Time));

	const wchar_t *MsgItems[]=
	{
		GetMsg(MWarning),
		Warning,
		Name,
		L"\1",
		New,
		Existing,
		GetMsg(MOverwrite), GetMsg(MAll), GetMsg(MSkip), GetMsg(MSkipAll), GetMsg(MCancel)
	};

	int ExitCode=Info.Message(&MainGuid,&QueryOverwriteMsgGuid,FMSG_WARNING|FMSG_LEFTALIGN,NULL,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]), 5);

	return (ExitCode<=QR_SKIPALL?ExitCode:QR_ABORT);
}

int AdvCmpProc::QueryDelete(const wchar_t *FileName, bool bIsDir, bool bReadOnlyType)
{
	const wchar_t *MsgItems[]=
	{
		GetMsg(MWarning),
		GetMsg(bReadOnlyType?MFileIsReadOnly:(bIsDir?MDelFolder:MDelFile)),
		GetPosToName(FileName),
		GetMsg(MAskDel),
		GetMsg(MDelete), GetMsg(MAll), GetMsg(MSkip), GetMsg(MSkipAll), GetMsg(MCancel)
	};

	int ExitCode=Info.Message(&MainGuid,&QueryDelMsgGuid,FMSG_WARNING,NULL,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]), 5);

	return (ExitCode<=QR_SKIPALL?ExitCode:QR_ABORT);
}

bool bStartSyncMsg=true;

void ShowSyncMsg(const wchar_t *Name1, const wchar_t *Name2, __int64 Progress, __int64 Max, bool bRedraw)
{
	// Для перерисовки не чаще 3-х раз в 1 сек.
	if (!bRedraw)
	{
		static DWORD dwTicks;
		DWORD dwNewTicks = GetTickCount();
		if (dwNewTicks - dwTicks < 350)
			return;
		dwTicks = dwNewTicks;
	}

	wchar_t TruncName1[MAX_PATH], TruncName2[MAX_PATH];
	TruncCopy(TruncName1,GetPosToName(Name1),WinInfo.TruncLen);
	TruncCopy(TruncName2,GetPosToName(Name2),WinInfo.TruncLen);

	wchar_t ProgressBar[MAX_PATH];
	ProgressLine(ProgressBar,Progress,Max);

	const wchar_t *MsgItems[] =
	{
		GetMsg(MSyncTitle),
		GetMsg(MCopying),
		TruncName1,
		GetMsg(MCopyingTo),
		TruncName2,
		L"\1",
		ProgressBar
	};

	Info.Message(&MainGuid,&SyncMsgGuid,bStartSyncMsg?FMSG_LEFTALIGN:FMSG_LEFTALIGN|FMSG_KEEPBACKGROUND,NULL,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]),0);
	bStartSyncMsg=false;
}

struct SynchronizeFileCopyCallbackData
{
	wchar_t *srcFileName;
	wchar_t *destFileName;
};

DWORD WINAPI SynchronizeFileCopyCallback(
	LARGE_INTEGER TotalFileSize,
	LARGE_INTEGER TotalBytesTransferred,
	LARGE_INTEGER StreamSize,
	LARGE_INTEGER StreamBytesTransferred,
	DWORD dwStreamNumber,
	DWORD dwCallbackReason,
	HANDLE hSourceFile,
	HANDLE hDestinationFile,
	LPVOID lpData
)
{
	__int64 progress=TotalBytesTransferred.QuadPart, max=TotalFileSize.QuadPart;

	if (lpData)
	{
		struct SynchronizeFileCopyCallbackData * data = (SynchronizeFileCopyCallbackData *)lpData;
		ShowSyncMsg(data->srcFileName,data->destFileName,progress,max,false);
	}
	else
		ShowSyncMsg(L"", L"", progress, max,false);

	if (CheckForEsc())
		return PROGRESS_CANCEL;
	else
		return PROGRESS_CONTINUE;
}

int AdvCmpProc::FileExists(const wchar_t *FileName, __int64 *pSize, FILETIME *pTime, DWORD *pAttrib, int CheckForFilter)
{
	int ret=0; // продолжим, но пропустим элемент
	WIN32_FIND_DATA wfdFindData;
	HANDLE hFind=FindFirstFileW(FileName,&wfdFindData);

	if (hFind != INVALID_HANDLE_VALUE)
	{
		FindClose(hFind);
		if (CheckForFilter)
		{
			if (!Opt.ProcessHidden && (wfdFindData.dwFileAttributes&FILE_ATTRIBUTE_HIDDEN))
				return ret;

			PluginPanelItem ppi;
			memset(&ppi,0,sizeof(ppi));
			ppi.FileAttributes=wfdFindData.dwFileAttributes;
			ppi.LastAccessTime=wfdFindData.ftLastAccessTime;
			ppi.LastWriteTime=wfdFindData.ftLastWriteTime;
			ppi.FileSize=((__int64)wfdFindData.nFileSizeHigh << 32) | wfdFindData.nFileSizeLow;
			ppi.FileName=FSF.PointToName(FileName);

			if ( (CheckForFilter>0?LPanel.hFilter:RPanel.hFilter)!=INVALID_HANDLE_VALUE &&
					 !Info.FileFilterControl((CheckForFilter>0?LPanel.hFilter:RPanel.hFilter),FFCTL_ISFILEINFILTER,0,&ppi))
				return ret;

			if (Opt.Filter && !Info.FileFilterControl(Opt.hCustomFilter,FFCTL_ISFILEINFILTER,0,&ppi))
				return ret;
		}
		*pSize=((__int64)wfdFindData.nFileSizeHigh << 32) | wfdFindData.nFileSizeLow;
		*pTime=wfdFindData.ftLastWriteTime;
		*pAttrib=wfdFindData.dwFileAttributes;
		ret=1; // ОК
	}

	return ret;
}

int AdvCmpProc::SyncFile(const wchar_t *srcFileName, const wchar_t *destFileName, int direction)
{
	if (bBrokenByEsc)
		return 0;

	int ret=1;

	// если сказали - "а мы не хотим туда копировать", то пропустим...
	if (((direction < 0) && !Opt.SyncLPanel) || ((direction > 0) && !Opt.SyncRPanel))
		return ret;

	__int64 srcSize=0, destSize=0;
	DWORD srcAttrib=0, destAttrib=0;
	FILETIME srcTime, destTime;

	if (srcFileName && destFileName && FileExists(srcFileName,&srcSize,&srcTime,&srcAttrib,direction))
	{
		ShowSyncMsg(srcFileName,destFileName,0,srcSize,true);
		int doCopy=1;

		if (FileExists(destFileName,&destSize,&destTime,&destAttrib,0))
		{
			// Overwrite confirmation
			if (((direction < 0) && bAskLOverwrite) || ((direction > 0) && bAskROverwrite))
			{
				switch(QueryOverwriteFile(destFileName,&srcTime,&destTime,srcSize,destSize,direction,false))
				{
					case QR_OVERWRITE:
						doCopy=1;
						break;
					case QR_ALL:
						doCopy=1;
						if (direction<0) bAskLOverwrite=false;
						else bAskROverwrite=false;
						break;
					case QR_SKIP:
						doCopy=0;
						break;
					case QR_SKIPALL:
						doCopy=0;
						if (direction<0) Opt.SyncLPanel=0;
						else Opt.SyncRPanel=0;
						break;
					default:
						doCopy=0;
						ret=0;
						bBrokenByEsc=true;
						break;
				}
			}

			// ReadOnly overwrite confirmation
			if (doCopy && (destAttrib & FILE_ATTRIBUTE_READONLY))
			{
				if (((direction < 0) && bSkipLReadOnly) || ((direction > 0) && bSkipRReadOnly))
				{
					doCopy=0;
				}
				else if (((direction < 0) && bAskLReadOnly) || ((direction > 0) && bAskRReadOnly))
				{
					switch(QueryOverwriteFile(destFileName,&srcTime,&destTime,srcSize,destSize,direction,true))
					{
						case QR_OVERWRITE:
							doCopy=1;
							break;
						case QR_ALL:
							doCopy=1;
							if (direction < 0) bAskLReadOnly=false;
							else bAskRReadOnly=false;
							break;
						case QR_SKIP:
							doCopy=0;
							break;
						case QR_SKIPALL:
							doCopy=0;
							if (direction < 0) bSkipLReadOnly=true;
							else bSkipRReadOnly=true;
							break;
						default:
							doCopy=0;
							ret=0;
							bBrokenByEsc=true;
							break;
					}
				}
			}
		}

		if (doCopy)
		{
			struct SynchronizeFileCopyCallbackData copyData;
			copyData.srcFileName=(wchar_t *)srcFileName;
			copyData.destFileName=(wchar_t *)destFileName;

RetryCopy:

			DWORD dwErr=0;
			SetLastError(dwErr);

			SetFileAttributesW(srcFileName,srcAttrib & ~(FILE_ATTRIBUTE_READONLY|FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_SYSTEM));
			SetFileAttributesW(destFileName,destAttrib & ~(FILE_ATTRIBUTE_READONLY|FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_SYSTEM));

			if (!CopyFileExW(srcFileName,destFileName,SynchronizeFileCopyCallback,&copyData,NULL,0))
			{
				dwErr=GetLastError();
				if (!dwErr) dwErr=E_UNEXPECTED;
				SetFileAttributesW(destFileName,destAttrib); // пробуем восстановить атрибуты
			}
			else
				SetFileAttributesW(destFileName,srcAttrib); // установим

			SetFileAttributesW(srcFileName,srcAttrib);  // восстановим атрибуты

			if (dwErr)
			{
				int nErrMsg=MFailedCopySrcFile;
				// Check, wich file failes?
				HANDLE hFile;
				// ERROR_SHARING_VIOLATION==32
				hFile=CreateFileW(srcFileName,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,0,NULL);
				if (hFile==INVALID_HANDLE_VALUE)
					nErrMsg=MFailedOpenSrcFile; // Source file failed
				else
				{
					CloseHandle(hFile);
					hFile=CreateFileW(destFileName,GENERIC_WRITE,FILE_SHARE_READ,0,OPEN_EXISTING,0,NULL);
					if (hFile==INVALID_HANDLE_VALUE)
					{
						if (GetLastError() == ERROR_FILE_NOT_FOUND)
							nErrMsg=MFailedCreateDstFile; // Failed to create destination file
						else
							nErrMsg=MFailedOpenDstFile; // Failed to open destination file
					}
					else
						CloseHandle(hFile);
				}

				const wchar_t *MsgItems[]=
				{
					GetMsg(MWarning),
					GetMsg(nErrMsg),
					GetPosToName(srcFileName),
					GetMsg(MFailedCopyDstFile),
					GetPosToName(destFileName),
					GetMsg(MRetry), GetMsg(MSkip), GetMsg(MCancel)
				};

				SetLastError(dwErr);
				int ExitCode= bBrokenByEsc ? 2/*MCancel*/ : Info.Message(&MainGuid,&FailedCopyMsgGuid,FMSG_WARNING|FMSG_ERRORTYPE,0,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]),3);

				if (!ExitCode)
					goto RetryCopy;
				else if (ExitCode != 1)
					ret=0;
			}
		}
	}
	return ret;
}

int AdvCmpProc::SyncRDelFile(const wchar_t *FileName)
{
	if (bBrokenByEsc)
		return 0;

	int ret=1;

	if (!Opt.SyncRDel)
		return ret;

	__int64 Size=0;
	DWORD Attrib=0;
	FILETIME Time;

	if (FileName && FileExists(FileName,&Size,&Time,&Attrib,Opt.SyncUseDelFilter?-1:0)) // -1, т.е. справа
	{
		int doDel=1;

		// Delete confirmation
		if (bAskRDel)
		{
			switch(QueryDelete(FileName,false,false))
			{
				case QR_DELETE:
					doDel=1;
					break;
				case QR_ALL:
					doDel=1;
					bAskRDel=false;
					break;
				case QR_SKIP:
					doDel=0;
					break;
				case QR_SKIPALL:
					doDel=0;
					Opt.SyncRDel=0;
					break;
				default:
					doDel=0;
					ret=0;
					bBrokenByEsc=true;
					break;
			}
		}

		// ReadOnly delete confirmation
		if (doDel && (Attrib & FILE_ATTRIBUTE_READONLY))
		{
			if (bSkipRReadOnly)
			{
				doDel=0;
			}
			else if (bAskRReadOnly)
			{
				switch(QueryDelete(FileName,false,true))
				{
					case QR_DELETE:
						doDel=1;
						break;
					case QR_ALL:
						doDel=1;
						bAskRReadOnly=false;
						break;
					case QR_SKIP:
						doDel=0;
						break;
					case QR_SKIPALL:
						doDel=0;
						bSkipRReadOnly=true;
						break;
					default:
						doDel=0;
						ret=0;
						bBrokenByEsc=true;
						break;
				}
			}
		}

		if (doDel)
		{
RetryDelFile:

			SetLastError(0);
			SetFileAttributesW(FileName,Attrib & ~(FILE_ATTRIBUTE_READONLY|FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_SYSTEM));

			if (!DeleteFileW(FileName))
			{
				const wchar_t *MsgItems[]=
				{
					GetMsg(MWarning),
					GetMsg(MFailedDelFile),
					GetPosToName(FileName),
					GetMsg(MRetry), GetMsg(MSkip), GetMsg(MCancel)
				};

				int ExitCode=bBrokenByEsc ? 2/*MCancel*/ : Info.Message(&MainGuid,&FailedDelFileMsgGuid,FMSG_WARNING|FMSG_ERRORTYPE,0,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]),3);

				if (!ExitCode)
					goto RetryDelFile;
				else if (ExitCode != 1)
				{
					SetFileAttributesW(FileName,Attrib); // пробуем восстановить атрибуты
					ret=0;
				}
			}
		}
	}
	return ret;
}


int AdvCmpProc::SyncDir(const wchar_t *srcDirName, const wchar_t *destDirName, int direction)
{
	if (bBrokenByEsc)
		return 0;

	if (!srcDirName || !*srcDirName || !destDirName || !*destDirName)
	{
		SetLastError(E_INVALIDARG);
		return 0;
	}

	__int64 Size=0;
	DWORD Attrib=0;
	FILETIME Time;

	// проверим на фильтр
	if (!FileExists(srcDirName,&Size,&Time,&Attrib,direction))
		return 1; // пропустим

RetryMkDir:

	int ret=1;
	WIN32_FIND_DATA wfdFindData;
	HANDLE hFind=FindFirstFileW(destDirName,&wfdFindData);

	if (hFind==INVALID_HANDLE_VALUE)
	{
		if (Attrib && CreateDirectoryW(destDirName,NULL))
			SetFileAttributesW(destDirName,Attrib);
		else
			ret=0;
	}
	else
	{
		FindClose(hFind);
		if (!(wfdFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			ret=0;
	}

	if (!ret)
	{
		const wchar_t *MsgItems[]=
		{
			GetMsg(MWarning),
			GetMsg(MCantCreateFolder),
			GetPosToName(destDirName),
			GetMsg(MRetry), GetMsg(MSkip), GetMsg(MCancel)
		};

		int ExitCode=Info.Message(&MainGuid,&CantCreateFolderMsgGuid,FMSG_WARNING|FMSG_ERRORTYPE,0,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]),3);

		if (!ExitCode)
			goto RetryMkDir;
		else if (ExitCode == 1)
			return 1; // значит продолжим, но не будем копировать эту папку и ее содержимое!
		return 0;
	}

	string strSrcDirMask(srcDirName);
	strSrcDirMask+=L"\\*";

	if ((hFind=FindFirstFileW(strSrcDirMask,&wfdFindData))!= INVALID_HANDLE_VALUE)
	{
		do
		{
			string strNewSrc, strNewDest;
			GetFullFileName(strNewSrc,srcDirName,wfdFindData.cFileName);
			GetFullFileName(strNewDest,destDirName,wfdFindData.cFileName);

			if (wfdFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				if (!(wfdFindData.cFileName[0]==L'.' && !wfdFindData.cFileName[1]) && !(wfdFindData.cFileName[0]==L'.' && wfdFindData.cFileName[1]==L'.' && !wfdFindData.cFileName[2]))
				{
					if (!SyncDir(strNewSrc,strNewDest,direction))
						ret=0;
				}
			}
			else
			{
				if (!SyncFile(strNewSrc,strNewDest,direction))
					ret=0;
			}
		}
		while (ret && FindNextFile(hFind,&wfdFindData));
		FindClose(hFind);
	}
	return ret;
}

int AdvCmpProc::SyncRDelDir(const wchar_t *DirName)
{
	if (bBrokenByEsc)
		return 0;

	int ret=1;

	if (!Opt.SyncRDel)
		return ret;

	if (!DirName || !*DirName)
	{
		SetLastError(E_INVALIDARG);
		return 0;
	}

	__int64 Size=0;
	DWORD Attrib=0;
	FILETIME Time;

	// проверим на фильтр
	if (!FileExists(DirName,&Size,&Time,&Attrib,Opt.SyncUseDelFilter?-1:0))
		return 1; // пропустим

RetryDelDir:

	WIN32_FIND_DATA wfdFindData;
	HANDLE hFind=FindFirstFileW(DirName,&wfdFindData);

	if (hFind==INVALID_HANDLE_VALUE)
	{
		return 1;  // не найден, считаем, что удалили
	}
	else
	{
		FindClose(hFind);
		if (!(wfdFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			ret=0;
	}

	if (ret)
	{
		int doDel=1;

		// Delete confirmation
		if (bAskRDel)
		{
			switch(QueryDelete(DirName,true,false))
			{
				case QR_DELETE:
					doDel=1;
					break;
				case QR_ALL:
					doDel=1;
					bAskRDel=false;
					break;
				case QR_SKIP:
					doDel=0;
					break;
				case QR_SKIPALL:
					doDel=0;
					Opt.SyncRDel=0;
					break;
				default:
					doDel=0;
					ret=0;
					bBrokenByEsc=true;
					break;
			}
		}

		// ReadOnly delete confirmation
		if (doDel && (wfdFindData.dwFileAttributes & FILE_ATTRIBUTE_READONLY))
		{
			if (bSkipRReadOnly)
			{
				doDel=0;
			}
			else if (bAskRReadOnly)
			{
				switch(QueryDelete(DirName,true,true))
				{
					case QR_DELETE:
						doDel=1;
						break;
					case QR_ALL:
						doDel=1;
						bAskRReadOnly=false;
						break;
					case QR_SKIP:
						doDel=0;
						break;
					case QR_SKIPALL:
						doDel=0;
						bSkipRReadOnly=true;
						break;
					default:
						doDel=0;
						ret=0;
						bBrokenByEsc=true;
						break;
				}
			}
		}

		if (doDel)
		{
			string strDirMask(DirName);
			strDirMask+=L"\\*";

			if ((hFind=FindFirstFileW(strDirMask,&wfdFindData)) != INVALID_HANDLE_VALUE)
			{
				do
				{
					string strNew;
					GetFullFileName(strNew,DirName,wfdFindData.cFileName);

					if (wfdFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
					{
						if (!(wfdFindData.cFileName[0]==L'.' && !wfdFindData.cFileName[1]) && !(wfdFindData.cFileName[0]==L'.' && wfdFindData.cFileName[1]==L'.' && !wfdFindData.cFileName[2]))
						{
							if (!SyncRDelDir(strNew))
								ret=0;
						}
					}
					else
					{
						if (!SyncRDelFile(strNew))
							ret=0;
					}
				}
				while (ret && FindNextFile(hFind,&wfdFindData));
				FindClose(hFind);
			}
		}

		if (ret && doDel && Opt.SyncRDel)
		{
			SetFileAttributesW(DirName,FILE_ATTRIBUTE_DIRECTORY);
			if (!RemoveDirectoryW(DirName))
				ret=0;
		}
	}

	if (!ret)
	{
		const wchar_t *MsgItems[]=
		{
			GetMsg(MWarning),
			GetMsg(MFailedDeleteFolder),
			GetPosToName(DirName),
			GetMsg(MRetry), GetMsg(MSkip), GetMsg(MCancel)
		};

		int ExitCode=bBrokenByEsc ? 2/*MCancel*/ : Info.Message(&MainGuid,&FailedDelFolderMsgGuid,FMSG_WARNING|FMSG_ERRORTYPE,0,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]),3);

		if (!ExitCode)
			goto RetryDelDir;
		else if (ExitCode == 1)
			ret=1;

		if (Attrib) SetFileAttributesW(DirName,Attrib); // пробуем восстановить
	}

	return ret;
}

int AdvCmpProc::Synchronize(FileList *pFileList)
{
	int ret=0;

	if (Opt.Sync==2) // есть элементы, синхронизируем
	{
		bBrokenByEsc=false;
		bStartSyncMsg=true;
		bAskLOverwrite=bAskROverwrite=Info.AdvControl(&MainGuid,ACTL_GETCONFIRMATIONS,0,0) & FCS_COPYOVERWRITE;
		bAskLReadOnly=bAskRReadOnly=Info.AdvControl(&MainGuid,ACTL_GETCONFIRMATIONS,0,0) & FCS_OVERWRITEDELETEROFILES;
		bAskRDel=Info.AdvControl(&MainGuid,ACTL_GETCONFIRMATIONS,0,0) & FCS_DELETE;
		bSkipLReadOnly=bSkipRReadOnly=false;

		hConInp=CreateFileW(L"CONIN$", GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
		DWORD dwTicks=GetTickCount();

		for (int i=0; i<pFileList->iCount; i++)
		{
			File *cur=&pFileList->F[i];
			int direction;
			if ((cur->dwLAttributes&FILE_ATTRIBUTE_DIRECTORY) && (cur->dwRAttributes&FILE_ATTRIBUTE_DIRECTORY))
				continue;
			if ( ((cur->dwFlags&RCIF_USERSELECT) && !pFileList->bShowSelect) || ((cur->dwFlags&RCIF_DIFFER) && !pFileList->bShowDifferent) ||
					 ((cur->dwFlags&RCIF_LNEW) && !pFileList->bShowLNew) || ((cur->dwFlags&RCIF_RNEW) && !pFileList->bShowRNew) )
				continue;
			if ((cur->dwFlags&RCIF_EQUAL) || ((cur->dwFlags&RCIF_DIFFER) && !(cur->dwFlags&(RCIF_USERLNEW|RCIF_USERRNEW))) || (cur->dwFlags&RCIF_USERNONE))
				continue;
			if ((cur->dwFlags&RCIF_USERLNEW) || (cur->dwFlags&RCIF_LNEW))
			{
				direction=1; // слева новый, значит копируем направо
				string strSrcName, strDestName;
				GetFullFileName(strSrcName,cur->LDir,cur->FileName);
				GetFullFileName(strDestName,cur->RDir,cur->FileName);

				if (cur->dwLAttributes&FILE_ATTRIBUTE_DIRECTORY)
				{
					if (!(ret=SyncDir(strSrcName,strDestName,direction)))
						break;
				}
				else
				{
					if (!(ret=SyncFile(strSrcName,strDestName,direction)))
						break;
				}
			}
			else if (cur->dwFlags&RCIF_USERDEL) // справа удаляем
			{
				string strName;
				GetFullFileName(strName,cur->RDir,cur->FileName);

				if (cur->dwRAttributes&FILE_ATTRIBUTE_DIRECTORY)
				{
					if (!(ret=SyncRDelDir(strName)))
						break;
				}
				else
				{
					if (!(ret=SyncRDelFile(strName)))
						break;
				}
			}
			else if ((cur->dwFlags&RCIF_USERRNEW) || (cur->dwFlags&RCIF_RNEW))
			{
				direction=-1; // справа новый, значит копируем налево
				string strSrcName, strDestName;
				GetFullFileName(strSrcName,cur->RDir,cur->FileName);
				GetFullFileName(strDestName,cur->LDir,cur->FileName);

				if (cur->dwRAttributes&FILE_ATTRIBUTE_DIRECTORY)
				{
					if (!(ret=SyncDir(strSrcName,strDestName,direction)))
						break;
				}
				else
				{
					if (!(ret=SyncFile(strSrcName,strDestName,direction)))
						break;
				}
			}
		}

		if (hConInp!=INVALID_HANDLE_VALUE) CloseHandle(hConInp);

		if (ret && !bBrokenByEsc)
		{
			if (Opt.Sound && (GetTickCount()-dwTicks > 30000)) MessageBeep(MB_ICONASTERISK);
			Info.AdvControl(&MainGuid,ACTL_PROGRESSNOTIFY,0,0);
		}

		// Кеш стал неактуальным, освободим
		if (Cache.RCI)
			free(Cache.RCI);
		Cache.RCI=0;
		Cache.ItemsNumber=0;
	}

	Info.PanelControl(LPanel.hPanel,FCTL_UPDATEPANEL,0,0);
	Info.PanelControl(RPanel.hPanel,FCTL_UPDATEPANEL,0,0);

	return ret;
}



/***************************************************************************
 *
 *                    ДИАЛОГ СРАВНЕНИЯ ТЕКУЩИХ ФАЙЛОВ
 *
 ***************************************************************************/


bool UpdateImage(PicData *data, bool CheckOnly=false)
{
	if (!data->DibData&&!data->Loaded)
	{
		//DrawImage
		{
			bool result=false;
			GFL_BITMAP *RawPicture=NULL;
			data->DibData=NULL;

			int dx=WinInfo.Win.right/(WinInfo.Con.Right-WinInfo.Con.Left);
			int dy=WinInfo.Win.bottom/(WinInfo.Con.Bottom-WinInfo.Con.Top);

			RECT DCRect;
			DCRect.left=dx*(data->DrawRect.left-WinInfo.Con.Left);
			DCRect.right=dx*(data->DrawRect.right+1-WinInfo.Con.Left);
			DCRect.top=dy*(data->DrawRect.top/*-WinInfo.Con.Top*/);          //костыль для запуска far.exe /w
			DCRect.bottom=dy*(data->DrawRect.bottom+1/*-WinInfo.Con.Top*/);  //костыль для запуска far.exe /w

			RECT RangedRect;

			GFL_LOAD_PARAMS load_params;
			pGflGetDefaultLoadParams(&load_params);
			load_params.Flags|=GFL_LOAD_SKIP_ALPHA;
			load_params.Flags|=GFL_LOAD_IGNORE_READ_ERROR;
			load_params.Origin=GFL_BOTTOM_LEFT;
			load_params.LinePadding=4;
			load_params.ImageWanted=data->Page-1;
			GFL_ERROR res=pGflLoadBitmapW(data->FileName,&RawPicture,&load_params,data->pic_info);
			if (res)
				RawPicture=NULL;
			if (RawPicture)
			{
				if (!pGflChangeColorDepth(RawPicture,NULL,GFL_MODE_TO_BGR,GFL_MODE_NO_DITHER) && !pGflRotate(RawPicture,NULL,data->Rotate,0))
				{
					{
						float asp_dst=(float)(DCRect.right-DCRect.left)/(float)(DCRect.bottom-DCRect.top);
						float asp_src=(float)RawPicture->Width/(float)RawPicture->Height;
						int dst_w, dst_h;

						if (asp_dst<asp_src)
						{
							dst_w=min(DCRect.right-DCRect.left,RawPicture->Width);
							dst_h=(int)(dst_w/asp_src);
						}
						else
						{
							dst_h=min(DCRect.bottom-DCRect.top,RawPicture->Height);
							dst_w=(int)(asp_src*dst_h);
						}
						RangedRect.left=DCRect.left;
						RangedRect.top=DCRect.top;
						RangedRect.right=dst_w;
						RangedRect.bottom=dst_h;
						RangedRect.left+=(DCRect.right-DCRect.left-RangedRect.right)/2;
						RangedRect.top+=(DCRect.bottom-DCRect.top-RangedRect.bottom)/2;
					}
					GFL_BITMAP *pic=NULL;
					pGflResize(RawPicture,&pic,RangedRect.right,RangedRect.bottom,GFL_RESIZE_BILINEAR,0);
					if (pic)
					{
						data->DibData=NULL;
						memset(data->BmpHeader,0,sizeof(BITMAPINFOHEADER));

						data->BmpHeader->biSize=sizeof(BITMAPINFOHEADER);
						data->BmpHeader->biWidth=pic->Width;
						data->BmpHeader->biHeight=pic->Height;
						data->BmpHeader->biPlanes=1;
						data->BmpHeader->biClrUsed=0;
						data->BmpHeader->biBitCount=24;
						data->BmpHeader->biCompression=BI_RGB;
						data->BmpHeader->biClrImportant=0;
						int bytes_per_line=(pic->Width*3+3)&-4;
						data->BmpHeader->biSizeImage=bytes_per_line*pic->Height;

						data->DibData=(unsigned char*)malloc(data->BmpHeader->biSizeImage);
						if (data->DibData)
							memcpy(data->DibData,pic->Data,data->BmpHeader->biSizeImage);

						pGflFreeBitmap(pic);
					}
				}
			}
			if (RawPicture&&data->DibData)
			{
				result=true;
				data->GDIRect=RangedRect;
			}
			if (RawPicture)
				pGflFreeBitmap(RawPicture);

			if (result)
			{
				data->Loaded=true;
				if ((!(data->FirstRun))&&(!CheckOnly))
					InvalidateRect(WinInfo.hFarWindow,NULL,TRUE);
			}
		}
	}
	if (!data->DibData||!data->Loaded)
		return false;
	if (CheckOnly)
		return true;
	HDC hDC=GetDC(WinInfo.hFarWindow);
	StretchDIBits(hDC,data->GDIRect.left,data->GDIRect.top,data->GDIRect.right,data->GDIRect.bottom,0,0,data->GDIRect.right,data->GDIRect.bottom,data->DibData,(BITMAPINFO *)data->BmpHeader,DIB_RGB_COLORS,SRCCOPY);
	ReleaseDC(WinInfo.hFarWindow,hDC);
	return true;
}

void FreeImage(PicData *data)
{
	if (data->DibData)
	{
		free(data->DibData);
		data->DibData=NULL;
		pGflFreeFileInformation(data->pic_info);
	}
}

void UpdateInfoText(HANDLE hDlg, PicData *data, bool left)
{
	wchar_t str[64], str2[4096];
	if (left)
	{
		FSF.sprintf(str,L"%d/%d  %d x %d",data->Page,data->pic_info->NumberOfImages,data->pic_info->Width,data->pic_info->Height);
		FSF.sprintf(str2,L"%*.*s",WinInfo.Con.Right/2-1,WinInfo.Con.Right/2-1,str);
	}
	else
		FSF.sprintf(str,L"%d x %d  %d/%d",data->pic_info->Width,data->pic_info->Height,data->Page,data->pic_info->NumberOfImages);
	Info.SendDlgMessage(hDlg,DM_SETTEXTPTR,left?3:4,(left?str2:str));
}


INT_PTR WINAPI ShowCmpCurDialogProc(HANDLE hDlg,int Msg,int Param1,void *Param2)
{
	CmpPicData *pPics=(CmpPicData *)Info.SendDlgMessage(hDlg,DM_GETDLGDATA,0,0);
	switch(Msg)
	{
		case DN_CTLCOLORDLGITEM:
			if (Param1!=0)
			{
				FarColor Color;
				struct FarDialogItemColors *Colors=(FarDialogItemColors*)Param2;
				Info.AdvControl(&MainGuid,ACTL_GETCOLOR,COL_PANELSELECTEDTITLE,&Color);
				Colors->Colors[0] = Color;
				Info.AdvControl(&MainGuid,ACTL_GETCOLOR,COL_PANELTEXT,&Color);
				Colors->Colors[1] = Colors->Colors[2] = Color;
			}
			break;

		case DN_DRAWDLGITEM:
			pPics->LPicData.Redraw=true;
			pPics->RPicData.Redraw=true;
			break;

		case DN_ENTERIDLE:
			if (pPics->LPicData.Redraw)
			{
				pPics->LPicData.Redraw=false;
				UpdateImage(&pPics->LPicData);
				if (pPics->LPicData.FirstRun)
				{
					pPics->LPicData.FirstRun=false;
					UpdateInfoText(hDlg,&pPics->LPicData,true);
					UpdateInfoText(hDlg,&pPics->RPicData,false);
				}
			}
			if (pPics->RPicData.Redraw)
			{
				pPics->RPicData.Redraw=false;
				UpdateImage(&pPics->RPicData);
				if (pPics->RPicData.FirstRun)
				{
					pPics->RPicData.FirstRun=false;
					UpdateInfoText(hDlg,&pPics->LPicData,true);
					UpdateInfoText(hDlg,&pPics->RPicData,false);
				}
			}
			break;

		case 0x3FFF:
			if (Param1)
			{
				UpdateImage(&pPics->LPicData);
				UpdateImage(&pPics->RPicData);
			}
			break;
/*
    case DN_GOTFOCUS:
      if(DlgParams->SelfKeys)
        Info.SendDlgMessage(hDlg,DM_SETFOCUS,2,0);
      break;
    case DN_GETDIALOGINFO:
      if(((DialogInfo*)(Param2))->StructSize != sizeof(DialogInfo))
        return FALSE;
      ((DialogInfo*)(Param2))->Id=DlgGUID;
      return TRUE;
    case DN_KEY:
      if(!DlgParams->SelfKeys)
      {
        if((Param2&(KEY_CTRL|KEY_ALT|KEY_SHIFT|KEY_RCTRL|KEY_RALT))==Param2) break;
        switch(Param2)
        {
          case KEY_CTRLR:
            UpdateImage(DlgParams);
            return TRUE;
          case KEY_CTRLD:
          case KEY_CTRLS:
          case KEY_CTRLE:
          {
            FreeImage(DlgParams);
            if(Param2==KEY_CTRLD) DlgParams->Rotate-=90;
            else if (Param2==KEY_CTRLS) DlgParams->Rotate+=90;
            else DlgParams->Rotate+=180;
            DlgParams->Loaded=false;
            UpdateImage(DlgParams);
            UpdateInfoText(hDlg,DlgParams);
            return TRUE;
          }
          case KEY_TAB:
            DlgParams->SelfKeys=true;
            Info.SendDlgMessage(hDlg,DM_SETFOCUS,2,0);
            return TRUE;
          case KEY_BS:
          case KEY_SPACE:
            if(DlgParams->ShowingIn==VIEWER)
              Param2=Param2==KEY_BS?KEY_SUBTRACT:KEY_ADD;
          default:
            if(DlgParams->ShowingIn==VIEWER && Param2==KEY_F3)
              Param2=KEY_ESC;
            if(DlgParams->ShowingIn==QUICKVIEW && Param2==KEY_DEL)
              Param2=KEY_F8;
            DlgParams->ResKey=Param2;
            Info.SendDlgMessage(hDlg,DM_CLOSE,-1,0);
            return TRUE;
        }
      }
      else
      {
        switch(Param2)
        {
          case KEY_TAB:
            DlgParams->SelfKeys=false;
            Info.SendDlgMessage(hDlg,DM_SETFOCUS,1,0);
            return TRUE;
          case KEY_ADD:
          case KEY_SUBTRACT:
            if(DlgParams->DibData)
            {
              int Pages=DlgParams->pic_info->NumberOfImages;
              FreeImage(DlgParams);
              DlgParams->Loaded=false;
              if(Param2==KEY_ADD) DlgParams->Page++;
              else DlgParams->Page--;
              if(DlgParams->Page<1) DlgParams->Page=Pages;
              if(DlgParams->Page>Pages) DlgParams->Page=1;
              UpdateImage(DlgParams);
              UpdateInfoText(hDlg,DlgParams);
            }
            return TRUE;
        }
      } 
      break;
*/
	}
	return Info.DefDlgProc(hDlg,Msg,Param1,Param2);
}


int AdvCmpProc::ShowCmpCurDialog(const PluginPanelItem *pLPPI,const PluginPanelItem *pRPPI)
{
	FarDialogItem DialogItems[9];
	memset(DialogItems,0,sizeof(DialogItems));

	DialogItems[0].Type=DI_USERCONTROL;
	DialogItems[0].X1=1; DialogItems[0].X2=WinInfo.Con.Right-1;
	DialogItems[0].Y1=1; DialogItems[0].Y2=WinInfo.Con.Bottom-WinInfo.Con.Top-3;

	DialogItems[1].Type=DI_DOUBLEBOX;
	DialogItems[1].X1=0; DialogItems[1].X2=WinInfo.Con.Right/2;
	DialogItems[1].Y1=0; DialogItems[1].Y2=WinInfo.Con.Bottom-WinInfo.Con.Top;
	DialogItems[1].Data=pLPPI->FileName;

	DialogItems[2].Type=DI_DOUBLEBOX;
	DialogItems[2].X1=WinInfo.Con.Right/2+1; DialogItems[2].X2=WinInfo.Con.Right;
	DialogItems[2].Y1=0; DialogItems[2].Y2=WinInfo.Con.Bottom-WinInfo.Con.Top;
	DialogItems[2].Data=pRPPI->FileName;
// инфо
	DialogItems[3].Type=DI_TEXT;
	DialogItems[3].X1=1; DialogItems[3].X2=WinInfo.Con.Right/2-1;
	DialogItems[3].Y1=WinInfo.Con.Bottom-WinInfo.Con.Top-3;

	DialogItems[4].Type=DI_TEXT;
	DialogItems[4].X1=WinInfo.Con.Right/2+2; DialogItems[4].X2=WinInfo.Con.Right-1;
	DialogItems[4].Y1=WinInfo.Con.Bottom-WinInfo.Con.Top-3;
// размер
	DialogItems[5].Type=DI_TEXT;
	DialogItems[5].X1=1; DialogItems[5].X2=WinInfo.Con.Right/2-1;
	DialogItems[5].Y1=WinInfo.Con.Bottom-WinInfo.Con.Top-2;

	DialogItems[6].Type=DI_TEXT;
	DialogItems[6].X1=WinInfo.Con.Right/2+2; DialogItems[6].X2=WinInfo.Con.Right-1;
	DialogItems[6].Y1=WinInfo.Con.Bottom-WinInfo.Con.Top-2;
// дата
	DialogItems[7].Type=DI_TEXT;
	DialogItems[7].X1=1; DialogItems[7].X2=WinInfo.Con.Right/2-1;
	DialogItems[7].Y1=WinInfo.Con.Bottom-WinInfo.Con.Top-1;

	DialogItems[8].Type=DI_TEXT;
	DialogItems[8].X1=WinInfo.Con.Right/2+2; DialogItems[8].X2=WinInfo.Con.Right-1;
	DialogItems[8].Y1=WinInfo.Con.Bottom-WinInfo.Con.Top-1;

	SYSTEMTIME LModificTime, RModificTime;
	FILETIME local;
	FileTimeToLocalFileTime(&(pLPPI->LastWriteTime), &local);
	FileTimeToSystemTime(&local, &LModificTime);
	FileTimeToLocalFileTime(&(pRPPI->LastWriteTime), &local);
	FileTimeToSystemTime(&local, &RModificTime);

	const wchar_t LAttributes[]=
	{
		pLPPI->FileAttributes&FILE_ATTRIBUTE_READONLY?L'R':L' ',
		pLPPI->FileAttributes&FILE_ATTRIBUTE_SYSTEM?L'S':L' ',
		pLPPI->FileAttributes&FILE_ATTRIBUTE_HIDDEN?L'H':L' ',
		pLPPI->FileAttributes&FILE_ATTRIBUTE_ARCHIVE?L'A':L' ',
		pLPPI->FileAttributes&FILE_ATTRIBUTE_REPARSE_POINT?L'L':pLPPI->FileAttributes&FILE_ATTRIBUTE_SPARSE_FILE?L'$':L' ',
		pLPPI->FileAttributes&FILE_ATTRIBUTE_COMPRESSED?L'C':pLPPI->FileAttributes&FILE_ATTRIBUTE_ENCRYPTED?L'E':L' ',
		pLPPI->FileAttributes&FILE_ATTRIBUTE_TEMPORARY?L'T':L' ',
		pLPPI->FileAttributes&FILE_ATTRIBUTE_NOT_CONTENT_INDEXED?L'I':L' ',
		pLPPI->FileAttributes&FILE_ATTRIBUTE_OFFLINE?L'O':L' ',
		pLPPI->FileAttributes&FILE_ATTRIBUTE_VIRTUAL?L'V':L' ',
		0
	};

	const wchar_t RAttributes[]=
	{
		pRPPI->FileAttributes&FILE_ATTRIBUTE_READONLY?L'R':L' ',
		pRPPI->FileAttributes&FILE_ATTRIBUTE_SYSTEM?L'S':L' ',
		pRPPI->FileAttributes&FILE_ATTRIBUTE_HIDDEN?L'H':L' ',
		pRPPI->FileAttributes&FILE_ATTRIBUTE_ARCHIVE?L'A':L' ',
		pRPPI->FileAttributes&FILE_ATTRIBUTE_REPARSE_POINT?L'L':pRPPI->FileAttributes&FILE_ATTRIBUTE_SPARSE_FILE?L'$':L' ',
		pRPPI->FileAttributes&FILE_ATTRIBUTE_COMPRESSED?L'C':pRPPI->FileAttributes&FILE_ATTRIBUTE_ENCRYPTED?L'E':L' ',
		pRPPI->FileAttributes&FILE_ATTRIBUTE_TEMPORARY?L'T':L' ',
		pRPPI->FileAttributes&FILE_ATTRIBUTE_NOT_CONTENT_INDEXED?L'I':L' ',
		pRPPI->FileAttributes&FILE_ATTRIBUTE_OFFLINE?L'O':L' ',
		pRPPI->FileAttributes&FILE_ATTRIBUTE_VIRTUAL?L'V':L' ',
		0
	};

	wchar_t Buf1[65], Buf2[65];
	string strBuf1, strBuf3;
	strBuf1.get(WinInfo.Con.Right/2);
	strBuf3.get(WinInfo.Con.Right/2);

	itoaa(pLPPI->FileSize,Buf1);
	FSF.sprintf(strBuf1.get(), L"%*.*s",DialogItems[5].X2,DialogItems[5].X2,Buf1);
	DialogItems[5].Data=strBuf1.get();

	itoaa(pRPPI->FileSize,Buf1);
	DialogItems[6].Data=Buf1;

	FSF.sprintf(Buf2, L"%-*.*s  %02d.%02d.%04d  %02d:%02d:%02d", 
							10,10,LAttributes,LModificTime.wDay,LModificTime.wMonth,LModificTime.wYear,LModificTime.wHour,LModificTime.wMinute,LModificTime.wSecond);
	FSF.sprintf(strBuf3.get(), L"%*.*s",DialogItems[7].X2,DialogItems[7].X2,Buf2);
	DialogItems[7].Data=strBuf3.get();

	FSF.sprintf(Buf2, L"%02d:%02d:%02d  %02d.%02d.%04d  %-*.*s", 
							RModificTime.wHour,RModificTime.wMinute,RModificTime.wSecond,RModificTime.wDay,RModificTime.wMonth,RModificTime.wYear,10,10,RAttributes);
	DialogItems[8].Data=Buf2;

	FarColor Color;
	Info.AdvControl(&MainGuid,ACTL_GETCOLOR,COL_PANELTEXT,&Color);

	unsigned int VBufSize=(WinInfo.Con.Right-WinInfo.Con.Left)*(WinInfo.Con.Bottom-WinInfo.Con.Top-2-1);
	FAR_CHAR_INFO *VirtualBuffer=(FAR_CHAR_INFO *)malloc(VBufSize*sizeof(FAR_CHAR_INFO));
	if (VirtualBuffer)
	{
		DialogItems[0].VBuf=VirtualBuffer;
		for(unsigned int i=0;i<VBufSize;i++)
		{
			VirtualBuffer[i].Char=L' ';
			VirtualBuffer[i].Attributes=Color;
		}

		HANDLE hDlg=Info.DialogInit(&MainGuid,&CurDlgGuid,0,0,WinInfo.Con.Right,WinInfo.Con.Bottom-WinInfo.Con.Top,NULL,DialogItems,
																sizeof(DialogItems)/sizeof(DialogItems[0]),0,FDLG_SMALLDIALOG|FDLG_NODRAWSHADOW,ShowCmpCurDialogProc,&CmpPic);
		if (hDlg != INVALID_HANDLE_VALUE)
		{
			Info.DialogRun(hDlg);
			Info.DialogFree(hDlg);
		}

		free(VirtualBuffer);
	}

	return true;
}

bool AdvCmpProc::CompareCurFile(const struct DirList *pLList,const struct DirList *pRList)
{
	string strLFullFileName, strRFullFileName;
	GetFullFileName(strLFullFileName,pLList->Dir,pLList->PPI[LPanel.PInfo.CurrentItem].FileName);
	GetFullFileName(strRFullFileName,pRList->Dir,pRList->PPI[RPanel.PInfo.CurrentItem].FileName);

	bool bImage=false;
	if (bGflLoaded)
	{
		CmpPic.LPicData.FileName=strLFullFileName.get();
		CmpPic.RPicData.FileName=strRFullFileName.get();
		CmpPic.LPicData.DrawRect.left=1;
		CmpPic.RPicData.DrawRect.left=WinInfo.Con.Right/2+2;
		CmpPic.LPicData.DrawRect.top=CmpPic.RPicData.DrawRect.top=1;
		CmpPic.LPicData.DrawRect.right=WinInfo.Con.Right/2-1;
		CmpPic.RPicData.DrawRect.right=WinInfo.Con.Right-1;
		CmpPic.LPicData.DrawRect.bottom=CmpPic.RPicData.DrawRect.bottom=WinInfo.Con.Bottom-WinInfo.Con.Top-2-2;
		CmpPic.LPicData.FirstRun=CmpPic.RPicData.FirstRun=true;
		CmpPic.LPicData.Redraw=CmpPic.RPicData.Redraw=false;
		CmpPic.LPicData.Loaded=CmpPic.RPicData.Loaded=false;
		BITMAPINFOHEADER BmpHeader1, BmpHeader2;
		CmpPic.LPicData.BmpHeader=&BmpHeader1;
		CmpPic.RPicData.BmpHeader=&BmpHeader2;
		CmpPic.LPicData.DibData=CmpPic.RPicData.DibData=NULL;
		GFL_FILE_INFORMATION pic_info1, pic_info2;
		CmpPic.LPicData.pic_info=&pic_info1;
		CmpPic.RPicData.pic_info=&pic_info2;
		CmpPic.LPicData.Page=CmpPic.RPicData.Page=1;
		CmpPic.LPicData.Rotate=CmpPic.RPicData.Rotate=0;

		bImage=(UpdateImage(&CmpPic.LPicData,true) && UpdateImage(&CmpPic.RPicData,true));
	}

	WIN32_FIND_DATA wfdFindData;
	HANDLE hFind;
	wchar_t DiffProgram[MAX_PATH];
	ExpandEnvironmentStringsW(Opt.WinMergePath,DiffProgram,(sizeof(DiffProgram)/sizeof(DiffProgram[0])));
	bool bFindDiffProg=((hFind=FindFirstFileW(DiffProgram,&wfdFindData)) != INVALID_HANDLE_VALUE);
	if (!bFindDiffProg)
	{
		ExpandEnvironmentStringsW(L"%ProgramFiles%\\WinMerge\\WinMergeU.exe",DiffProgram,(sizeof(DiffProgram)/sizeof(DiffProgram[0])));
		bFindDiffProg=((hFind=FindFirstFileW(DiffProgram,&wfdFindData)) != INVALID_HANDLE_VALUE);
	}
	if (bFindDiffProg) FindClose(hFind);

	struct FarMenuItem MenuItems[4];
	memset(MenuItems,0,sizeof(MenuItems));
	MenuItems[0].Text=GetMsg(MDefault);
	MenuItems[1].Text=GetMsg(MWinMerge);
	MenuItems[2].Text=GetMsg(MPictures);
	MenuItems[3].Text=GetMsg(MVisCmp);
	MenuItems[3].Flags|=MIF_GRAYED;
	if (!bFindDiffProg) MenuItems[1].Flags|=MIF_GRAYED;
	if (!bImage) MenuItems[2].Flags|=MIF_GRAYED;
	if (bImage) MenuItems[2].Flags|=MIF_SELECTED;
	else if (bFindDiffProg) MenuItems[1].Flags|=MIF_SELECTED;
	int MenuCode=Info.Menu(&MainGuid,&CmpMethodMenuGuid,-1,-1,0,FMENU_AUTOHIGHLIGHT|FMENU_WRAPMODE,L"Method",NULL,L"Contents",NULL,NULL,MenuItems,sizeof(MenuItems)/sizeof(MenuItems[0]));

	if (MenuCode==0)
	{
		bool bDifferenceNotFound;
		Opt.TotalProcess=0;
		if (Opt.CmpCase || Opt.CmpSize || Opt.CmpTime || Opt.CmpContents)
			bDifferenceNotFound=CompareFiles(pLList->Dir,&pLList->PPI[LPanel.PInfo.CurrentItem],pRList->Dir,&pRList->PPI[RPanel.PInfo.CurrentItem],0);
		else
			bDifferenceNotFound=!FSF.LStricmp(pLList->PPI[LPanel.PInfo.CurrentItem].FileName,pRList->PPI[RPanel.PInfo.CurrentItem].FileName);
		const wchar_t *MsgItems[]=
		{
			GetMsg(bDifferenceNotFound?MNoDiffTitle:MFirstDiffTitle),
			GetPosToName(strLFullFileName.get()),
			GetPosToName(strRFullFileName.get()),
			GetMsg(MOK)
		};
		Info.Message(&MainGuid,&CompareCurFileMsgGuid,bDifferenceNotFound?0:FMSG_WARNING,0,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]),1);
	}
	else if (MenuCode==1 && bFindDiffProg)
	{
		STARTUPINFO si;
		PROCESS_INFORMATION pi;
		memset(&si, 0, sizeof(si));
		si.cb = sizeof(si);
		wchar_t Command[32768];
		FSF.sprintf(Command, L"\"%s\" -e \"%s\" \"%s\"", DiffProgram,GetPosToName(strLFullFileName.get()),GetPosToName(strRFullFileName.get()));
		CreateProcess(0,Command,0,0,false,0,0,0,&si,&pi);
	}
	else if (MenuCode==2 && bImage)
		ShowCmpCurDialog(&pLList->PPI[LPanel.PInfo.CurrentItem],&pRList->PPI[RPanel.PInfo.CurrentItem]);
	else if (MenuCode==3 && pCompareFiles)
		pCompareFiles(strLFullFileName.get(),strRFullFileName.get(),0);

	FreeImage(&CmpPic.LPicData);
	FreeImage(&CmpPic.RPicData);

	return true;
}