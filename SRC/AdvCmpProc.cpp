/****************************************************************************
 * AdvCmpProc.cpp
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
#include "AdvCmpProc.hpp"

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
 * Возвращает начало файла, без префиксов "\\?\"
 ****************************************************************************/
int GetPosToName(wchar_t *FileName)
{
	if (FileName[0]==L'\\' && FileName[1]==L'\\' && FileName[2]==L'?')
	{
		if (FileName[5]==L':')
			return 4;
		else if (FileName[5]==L'N')
			return 7;
	}
	return 0;
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


AdvCmpProc::AdvCmpProc()
{
	hScreen=Info.SaveScreen(0,0,-1,-1);
	hConInp=CreateFileW(L"CONIN$", GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
	bStartMsg=true;

	FList.F=NULL;
	FList.iCount=0;

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
	if (Opt.Substr) free(Opt.Substr); // определен из диалога опций

	Info.FileFilterControl(Opt.hCustomFilter,FFCTL_FREEFILEFILTER,0,0);  // определен из диалога опций
	Info.FileFilterControl(LPanel.hFilter,FFCTL_FREEFILEFILTER,0,0);
	Info.FileFilterControl(RPanel.hFilter,FFCTL_FREEFILEFILTER,0,0);

	CloseHandle(hConInp);
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
 *************************** ShowMessage functions **************************
 ****************************************************************************/

/****************************************************************************
 * Проверка на Esc. Возвращает true, если пользователь нажал Esc
 ****************************************************************************/
bool AdvCmpProc::CheckForEsc(void)
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
 * Усекает начало длинных имен файлов (или дополняет короткие имена)
 * для правильного показа в сообщении сравнения
 ****************************************************************************/
void AdvCmpProc::TrunCopy(wchar_t *Dest, const wchar_t *Src, bool bDir, const wchar_t *Msg)
{
	string strSrc(Src);
	int iLen=0;
	if (bDir)
	{
		FSF.sprintf(Dest,Msg,FSF.TruncStr(strSrc.get(),WinInfo.TruncLen-wcslen(Msg)+2));
		iLen=wcslen(Dest);
	}
	else
		iLen=wcslen(wcscpy(Dest,FSF.TruncStr(strSrc.get(),WinInfo.TruncLen)));

	if (iLen<WinInfo.TruncLen)
	{
		wmemset(&Dest[iLen],L' ',WinInfo.TruncLen-iLen);
		Dest[WinInfo.TruncLen]=L'\0';
	}
}


/****************************************************************************
 * Рисует строку-прогресс
 ****************************************************************************/
void AdvCmpProc::ProgressLine(wchar_t *Dest, unsigned __int64 nCurrent, unsigned __int64 nTotal)
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
 * Показывает сообщение о сравнении двух файлов
 ****************************************************************************/
void AdvCmpProc::ShowMessage(const wchar_t *Dir1, const wchar_t *Name1, const wchar_t *Dir2, const wchar_t *Name2, bool bRedraw)
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
	TrunCopy(TruncDir1, Dir1, true, GetMsg(MComparing));
	TrunCopy(TruncName1, Name1, false, L"");
	TrunCopy(TruncDir2, Dir2, true, GetMsg(MComparingWith));
	TrunCopy(TruncName2, Name2, false, L"");

	wchar_t LDiff[100], RDiff[100], DiffOut[MAX_PATH];
	FSF.sprintf(Buf,GetMsg(MComparingDiffN),itoaa(CmpInfo.LDiff,LDiff),itoaa(CmpInfo.RDiff,RDiff));
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


	Info.Message(&MainGuid,bStartMsg?FMSG_LEFTALIGN:FMSG_LEFTALIGN|FMSG_KEEPBACKGROUND,0,Opt.TotalProcess?MsgItems2:MsgItems1,
																		Opt.TotalProcess?sizeof(MsgItems2)/sizeof(MsgItems2[0]):sizeof(MsgItems1)/sizeof(MsgItems1[0]),0);
	bStartMsg=false;
}



/****************************************************************************
 ************************** CompareFiles functions **************************
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
	if (OnlyInfo && bBrokenByEsc)
		return true;

	string strPathMask(Dir);
	strPathMask+=L"\\*";
	WIN32_FIND_DATA wfdFindData;
	HANDLE hFind;
	bool ret=true;

	if (!OnlyInfo) // заполянем DirList
	{
		pList->Dir=(wchar_t*)malloc((wcslen(Dir)+1)*sizeof(wchar_t));
		if (pList->Dir) wcscpy(pList->Dir,Dir);
		pList->PPI=0;
		pList->ItemsNumber=0;
	}

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
				strPathMask=Dir;
				if (strPathMask.length()>0 && strPathMask[(size_t)(strPathMask.length()-1)]!=L'\\') strPathMask+=L"\\";
				strPathMask+=wfdFindData.cFileName;
				ret=GetDirList(strPathMask,ScanDepth+1,OnlyInfo,0);
			}
			else
			{
				if (OnlyInfo)
				{
					CmpInfo.Count+=1;
					CmpInfo.CountSize+=((unsigned __int64)wfdFindData.nFileSizeHigh << 32) | wfdFindData.nFileSizeLow;
					ShowMessage(L"*",L"*",L"*",L"*",false);
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

			string strLFullDir(LDir), strRFullDir(RDir);
			if ((strLFullDir.length()>0 && strLFullDir[(size_t)(strLFullDir.length()-1)]!=L'\\') || !strLFullDir.length()) strLFullDir+=L"\\";
			if ((strRFullDir.length()>0 && strRFullDir[(size_t)(strRFullDir.length()-1)]!=L'\\') || !strRFullDir.length()) strRFullDir+=L"\\";
			strLFullDir+=pLPPI->FileName;
			strRFullDir+=pRPPI->FileName;

			// Составим списки элементов в подкаталогах
			struct DirList LList, RList;
			bool bEqual = true;

			if (!(bLPanelPlug || bRPanelPlug))
			{
				if (!GetDirList(strLFullDir,ScanDepth,false,&LList) || !GetDirList(strRFullDir,ScanDepth,false,&RList))
				{
					bBrokenByEsc=true; // То ли юзер прервал, то ли ошибка чтения
					bEqual=false; // Остановим сравнение
					bOpenFail=true;
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

				if (bLPanelPlug && !bBrokenByEsc && !Info.GetPluginDirList(&MainGuid,LPanel.hPanel,pLPPI->FileName,&LList.PPI,&LList.ItemsNumber))
				{
					bBrokenByEsc=true;
					bEqual=false;
					bOpenFail=true;
				}

				if (bRPanelPlug && !bBrokenByEsc && !Info.GetPluginDirList(&MainGuid,RPanel.hPanel,pRPPI->FileName,&RList.PPI,&RList.ItemsNumber))
				{
					bBrokenByEsc=true;
					bEqual=false;
					bOpenFail=true;
				}

				if (!bLPanelPlug && !bBrokenByEsc && !Info.GetDirList(strLFullDir.get()+GetPosToName(strLFullDir.get()),&LList.PPI,&LList.ItemsNumber))
				{
					bBrokenByEsc=true;
					bEqual=false;
					bOpenFail=true;
				}

				if (!bRPanelPlug && !bBrokenByEsc && !Info.GetDirList(strRFullDir.get()+GetPosToName(strRFullDir.get()),&RList.PPI,&RList.ItemsNumber))
				{
					bBrokenByEsc=true;
					bEqual=false;
					bOpenFail=true;
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

		string strLFileName(LPanel.bTMP?FSF.PointToName(pLPPI->FileName):pLPPI->FileName);
		string strRFileName(RPanel.bTMP?FSF.PointToName(pRPPI->FileName):pRPPI->FileName);

		// регистр имен
		if (Opt.CmpCase &&
				Strncmp(Opt.SkipSubstr?CutSubstr(strLFileName,Opt.Substr):strLFileName.get(),
								Opt.SkipSubstr?CutSubstr(strRFileName,Opt.Substr):strRFileName.get())
			 )
		{
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

			string strLFullFileName(LDir), strRFullFileName(RDir);
			if ((strLFullFileName.length()>0 && strLFullFileName[(size_t)(strLFullFileName.length()-1)]!=L'\\') || !strLFullFileName.length()) strLFullFileName+=L"\\";
			if ((strRFullFileName.length()>0 && strRFullFileName[(size_t)(strRFullFileName.length()-1)]!=L'\\') || !strRFullFileName.length()) strRFullFileName+=L"\\";
			strLFullFileName+=pLPPI->FileName;
			strRFullFileName+=pRPPI->FileName;

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

			if (!LPanel.bARC)
			{
				if ((hLFile=CreateFileW(strLFullFileName, GENERIC_READ, FILE_SHARE_READ, 0,
                                 OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, 0)) == INVALID_HANDLE_VALUE)
				{
					CmpInfo.ProcSize+=CmpInfo.CurCountSize;
					bOpenFail=true;
					return false;
				}
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
					bOpenFail=true;
					return false;
				}
				RAccess=pRPPI->LastAccessTime;
			}

			//---------------------------------------------------------------------------

			ShowMessage(LDir,pLPPI->FileName,RDir,pRPPI->FileName,true);

			DWORD LReadSize=1, RReadSize=1;
			DWORD LBufPos=1, RBufPos=1;     // позиция в Opt.Buf
			const DWORD ReadBlock=65536;
			__int64 LFilePos=0, RFilePos=0;  // позиция в файле
			bool bEqual=true;

			{
				char *LPtr=Opt.Buf[0]+LBufPos, *RPtr=Opt.Buf[1]+RBufPos;
				bool bLExpectNewLine=false, bRExpectNewLine=false;
				SHFILEINFO shinfo;
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

					ShowMessage(LDir,pLPPI->FileName,RDir,pRPPI->FileName,false);

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
					if ( !Opt.Ignore ||
								( SHGetFileInfoW(strLFullFileName,0,&shinfo,sizeof(shinfo),SHGFI_EXETYPE) ||
									SHGetFileInfoW(strRFullFileName,0,&shinfo,sizeof(shinfo),SHGFI_EXETYPE) )
							)
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
						{
							if (Opt.Cache && !(LPanel.bARC || RPanel.bARC))
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
							break;
						}
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
			}
			CloseHandle(hLFile);
			CloseHandle(hRFile);

			if ((hLFile=CreateFileW(strLFullFileName, FILE_WRITE_ATTRIBUTES, FILE_SHARE_READ | FILE_SHARE_WRITE, 0,
                               OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, 0)) != INVALID_HANDLE_VALUE)
			{
				SetFileTime(hLFile,0,&LAccess,0);
				CloseHandle(hLFile);
			}

			if ((hRFile=CreateFileW(strRFullFileName, FILE_WRITE_ATTRIBUTES, FILE_SHARE_READ | FILE_SHARE_WRITE, 0,
                               OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, 0)) != INVALID_HANDLE_VALUE)
			{
				SetFileTime(hRFile,0,&RAccess,0);
				CloseHandle(hRFile);
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
 *************************** CompareDirs functions **************************
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
//DebugMsg((wchar_t *)pList->PPI[i].FileName);
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
//DebugMsg(L"FAR",(wchar_t *)pList->PPI[i].FileName);

					if (Opt.ProcessSubfolders==2 && !CheckScanDepth(pList->PPI[i].FileName, Opt.MaxScanDepth))
						continue;
				}

				// плагин + панель || панель + плагин (элемент с плагина)
				else if ((bLPanelPlug && !bRPanelPlug && bLeftPanel) || (!bLPanelPlug && bRPanelPlug && !bLeftPanel))
				{
//					string srtFileName(pList->PPI[i].FileName);
					// вырежем всё до первого слеша, так как сама папка нам не нужна
//					wcscpy((wchar_t*)pList->PPI[i].FileName,CutSubstr(srtFileName,L"^(.*?)\\\\"));
//DebugMsg(L"ARC",(wchar_t *)pList->PPI[i].FileName);

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
	ShowMessage(pLList->Dir,L"*",pRList->Dir,L"*",true);

	// соберем информацию, сколько элементов будем сравнивать и их размер
	if (ScanDepth==0 && Opt.TotalProcess)
	{
		if (!(LPanel.PInfo.Flags&PFLAGS_PLUGIN))
		{
			if (!Opt.ProcessSelected)
				GetDirList(pLList->Dir,ScanDepth,true);
			else
			{
				string strDir;
				for (int i=0; !bBrokenByEsc && i<pLList->ItemsNumber; i++)
				{
					if (pLList->PPI[i].Flags&PPIF_SELECTED)
					{
						if (pLList->PPI[i].FileAttributes&FILE_ATTRIBUTE_DIRECTORY)
						{
							strDir=pLList->Dir;
							if (strDir.length()>0 && strDir[(size_t)(strDir.length()-1)]!=L'\\') strDir+=L"\\";
							strDir+=pLList->PPI[i].FileName;
							GetDirList(strDir,ScanDepth,true);
						}
						else
						{
							CmpInfo.Count+=1;
							CmpInfo.CountSize+=(unsigned __int64)pLList->PPI[i].FileSize;
						}
					}
				}
			}
		}

		if (bBrokenByEsc)
			return true;

		if (!(RPanel.PInfo.Flags&PFLAGS_PLUGIN))
		{
			if (!Opt.ProcessSelected)
				GetDirList(pRList->Dir,ScanDepth,true);
			else
			{
				string strDir;
				for (int i=0; !bBrokenByEsc && i<pRList->ItemsNumber; i++)
				{
					if (pRList->PPI[i].Flags&PPIF_SELECTED)
					{
						if (pRList->PPI[i].FileAttributes&FILE_ATTRIBUTE_DIRECTORY)
						{
							strDir=pRList->Dir;
							if (strDir.length()>0 && strDir[(size_t)(strDir.length()-1)]!=L'\\') strDir+=L"\\";
							strDir+=pRList->PPI[i].FileName;
							GetDirList(strDir,ScanDepth,true);
						}
						else
						{
							CmpInfo.Count+=1;
							CmpInfo.CountSize+=(unsigned __int64)pRList->PPI[i].FileSize;
						}
					}
				}
			}
		}

		if (bBrokenByEsc)
			return true;
	}

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

	// экспресс-сравнение вложенного каталога
	if (ScanDepth && !Opt.Dialog && !Opt.IgnoreMissing && LII.iCount!=RII.iCount)
		return false;

	int i, j;

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
				}
				else
				{
					i++;
				}
				CmpInfo.Proc++;
				// добавим элемент в диалог результатов
				if (Opt.Dialog)
					BuildFileList(pLList->Dir,LII.pPPI[i-1],pRList->Dir,NULL,dwFlag);
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
				}
				else
				{
					j++;
				}
				CmpInfo.Proc++;
				// добавим элемент в диалог результатов
				if (Opt.Dialog)
					BuildFileList(pLList->Dir,NULL,pRList->Dir,RII.pPPI[j-1],dwFlag);
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

	if (pLPPI && pRPPI && Opt.CmpTime && dwFlag==RCIF_DIFFER)
	{
		__int64 Delta=(((__int64)pLPPI->LastWriteTime.dwHighDateTime << 32) | pLPPI->LastWriteTime.dwLowDateTime) -
									(((__int64)pRPPI->LastWriteTime.dwHighDateTime << 32) | pRPPI->LastWriteTime.dwLowDateTime);
		if (Delta>0) dwFlag=RCIF_LNEW;
		else if (Delta<0) dwFlag=RCIF_RNEW;
	}
	// два каталога - особый случай!
	if (pLPPI && pRPPI && (pLPPI->FileAttributes&FILE_ATTRIBUTE_DIRECTORY) && (pRPPI->FileAttributes&FILE_ATTRIBUTE_DIRECTORY))
		dwFlag|=RCIF_DIR;

	FList.F[FList.iCount].dwFlags=dwFlag;
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
		FList.F[FList.iCount].dwAttributes=pLPPI->FileAttributes;
		FList.F[FList.iCount].ftLLastWriteTime.dwLowDateTime=pLPPI->LastWriteTime.dwLowDateTime;
		FList.F[FList.iCount].ftLLastWriteTime.dwHighDateTime=pLPPI->LastWriteTime.dwHighDateTime;
		FList.F[FList.iCount].nLFileSize=pLPPI->FileSize;
	}
	if (pRPPI)
	{
		if (!(FList.F[FList.iCount].FileName))
		{
			FList.F[FList.iCount].FileName=(wchar_t*)malloc((wcslen(pRPPI->FileName)+1)*sizeof(wchar_t));
			if (FList.F[FList.iCount].FileName) wcscpy(FList.F[FList.iCount].FileName,pRPPI->FileName);
		}
		FList.F[FList.iCount].dwAttributes=pRPPI->FileAttributes;
		FList.F[FList.iCount].ftRLastWriteTime.dwLowDateTime=pRPPI->LastWriteTime.dwLowDateTime;
		FList.F[FList.iCount].ftRLastWriteTime.dwHighDateTime=pRPPI->LastWriteTime.dwHighDateTime;
		FList.F[FList.iCount].nRFileSize=pRPPI->FileSize;
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
		if (Item1->dwAttributes&FILE_ATTRIBUTE_DIRECTORY)
		{
			if (!(Item2->dwAttributes&FILE_ATTRIBUTE_DIRECTORY))
				return -1;
		}
		else
		{
			if (Item2->dwAttributes&FILE_ATTRIBUTE_DIRECTORY)
				return 1;
		}
		cmp=FSF.LStricmp(Item1->FileName,Item2->FileName);
	}
	return cmp;
}

/***************************************************************************
 * Изменение/обновление листа файлов в диалоге
 ***************************************************************************/
bool UpdateFarList(HANDLE hDlg, FileList *pFileList)
{
	// запросим информацию
	FarListInfo ListInfo;
	Info.SendDlgMessage(hDlg,DM_LISTINFO,0,&ListInfo);

	if (ListInfo.ItemsNumber)
		Info.SendDlgMessage(hDlg,DM_LISTDELETE,0,0);

	if (!pFileList->iCount)
		return true;

	FSF.qsort(pFileList->F,pFileList->iCount,sizeof(pFileList->F[0]),SortList);

	// каталог на панели
	string strLPanelDir, strRPanelDir;
	{
		int size=Info.PanelControl(LPanel.hPanel,FCTL_GETPANELDIR,0,0);
		strLPanelDir.get(size);
		Info.PanelControl(LPanel.hPanel,FCTL_GETPANELDIR,size,strLPanelDir.get());
		strLPanelDir.updsize();

		size=Info.PanelControl(RPanel.hPanel,FCTL_GETPANELDIR,0,0);
		strRPanelDir.get(size);
		Info.PanelControl(RPanel.hPanel,FCTL_GETPANELDIR,size,strRPanelDir.get());
		strRPanelDir.updsize();
	}

	int Index=0, Items=0, Equal=0, Diff=0, LDiff=0, RDiff=0;
	wchar_t buf[65536];

	for (int i=0; i<pFileList->iCount; i++)
	{
		File *cur=&pFileList->F[i];

		if (cur->dwFlags&RCIF_DIR)
			continue;

		wchar_t LTime[18]={0}, RTime[18]={0};
		SYSTEMTIME ModificTime;
		FILETIME local;
		if (cur->ftLLastWriteTime.dwLowDateTime || cur->ftLLastWriteTime.dwHighDateTime)  // есть элемент слева
		{
			FileTimeToLocalFileTime(&(cur->ftLLastWriteTime), &local);
			FileTimeToSystemTime(&local, &ModificTime);
			FSF.sprintf(LTime, L"%02d.%02d.%02d %02d:%02d:%02d",ModificTime.wDay,ModificTime.wMonth,ModificTime.wYear%100,ModificTime.wHour,ModificTime.wMinute,ModificTime.wSecond);
		}
		if (cur->ftRLastWriteTime.dwLowDateTime || cur->ftRLastWriteTime.dwHighDateTime)  // есть элемент справа
		{
			FileTimeToLocalFileTime(&(cur->ftRLastWriteTime), &local);
			FileTimeToSystemTime(&local, &ModificTime);
			FSF.sprintf(RTime, L"%02d.%02d.%02d %02d:%02d:%02d",ModificTime.wDay,ModificTime.wMonth,ModificTime.wYear%100,ModificTime.wHour,ModificTime.wMinute,ModificTime.wSecond);
		}

		wchar_t LSize[65]={0}, RSize[65]={0};
		const int nSIZE=32; // размер ячейки для FSF.sprintf(%*.*)

		if (!(cur->dwAttributes&FILE_ATTRIBUTE_DIRECTORY))
		{
			if (LTime[0]) cur->nLFileSize>99999999999?FSF.itoa64(cur->nLFileSize,LSize,10):itoaa(cur->nLFileSize,LSize);
			if (RTime[0]) cur->nRFileSize>99999999999?FSF.itoa64(cur->nRFileSize,RSize,10):itoaa(cur->nRFileSize,RSize);
		}
		else
		{
			if (LTime[0]) strcentr(LSize,GetMsg(MFolder),nSIZE,L' ');
			if (RTime[0]) strcentr(RSize,GetMsg(MFolder),nSIZE,L' ');
		}

		wchar_t Mark;
		switch (cur->dwFlags)
		{
			case RCIF_EQUAL:
				Mark=L'='; Equal++; break;
			case RCIF_DIFFER:
				Mark=0x2260; Diff++; break;
			case RCIF_LNEW:
				Mark=0x2192; LDiff++; break;
			case RCIF_RNEW:
				Mark=0x2190; RDiff++; break;
			default:
				Mark=L' '; break;
		}

		// виртуальная папка
		bool bAddVirtDir=false;
		string strVirtDir;

		// если попали сразу в подкаталог... добавим виртуальную папку в начало
		if (!Index && FSF.LStricmp(cur->LDir+GetPosToName(cur->LDir),strLPanelDir.get()))
		{
			strVirtDir=cur->LDir+(GetPosToName(cur->LDir)+strLPanelDir.length());
			if (strVirtDir.length()>0 && strVirtDir[(size_t)(strVirtDir.length()-1)]!=L'\\') strVirtDir+=L"\\";
			struct FarListItem Item;
			Item.Flags=0x2b;
			Item.Flags|=LIF_CHECKED;
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
		else
		{
			for (int j=i+1; j<pFileList->iCount; j++)
			{
				File *next=&pFileList->F[j];

				if (next->dwFlags&RCIF_DIR)
					continue;

				if (FSF.LStricmp(cur->LDir,next->LDir))
				{
					bAddVirtDir=true;
					strVirtDir=next->LDir+(GetPosToName(next->LDir)+strLPanelDir.length());
					if (strVirtDir.length()>0 && strVirtDir[(size_t)(strVirtDir.length()-1)]!=L'\\') strVirtDir+=L"\\";
				}
				break;
			}
		}

		if (cur->dwAttributes&FILE_ATTRIBUTE_DIRECTORY)
		{
			string strDir;
			if (LTime[0])
				strDir=cur->LDir+(GetPosToName(cur->LDir)+strLPanelDir.length());
			if (RTime[0])
				strDir=cur->RDir+(GetPosToName(cur->RDir)+strRPanelDir.length());
			strDir+=L"\\";
			strDir+=cur->FileName;
			if (strDir.length()>0 && strDir[(size_t)(strDir.length()-1)]!=L'\\') strDir+=L"\\";

			FSF.sprintf(buf, L"%*.*s%c%*.*s%c%c%c%s",nSIZE,nSIZE,LSize,0x2551,nSIZE,nSIZE,RSize,0x2551,Mark,0x2502,strDir.get());
		}
		else
			FSF.sprintf(buf, L"%*.*s%c%*.*s%c%*.*s%c%*.*s%c%c%c%s",14,14,LSize,0x2502,17,17,LTime,0x2551,17,17,RTime,0x2502,14,14,RSize,0x2551,Mark,0x2502,cur->FileName);

		struct FarListItem Item;
		Item.Flags=0;
		if (cur->dwAttributes&FILE_ATTRIBUTE_DIRECTORY)
		{
			Item.Flags=0x2b; 
			Item.Flags|=LIF_CHECKED;
		}
		else if (cur->dwFlags==RCIF_EQUAL) 
			Item.Flags|=LIF_GRAYED;
		Item.Text=buf;
		Item.Reserved[0]=Item.Reserved[1]=Item.Reserved[2]=0;
		struct FarList List;
		List.ItemsNumber=1;
		List.Items=&Item;

		if (Info.SendDlgMessage(hDlg,DM_LISTADD,0,&List))
		{
			Items++;
			struct FarListItemData Data;
			Data.Index=Index++;
			Data.DataSize=sizeof(*cur);
			Data.Data=cur;
			Info.SendDlgMessage(hDlg,DM_LISTSETDATA,0,&Data);
		}

		if (bAddVirtDir)
		{
			strcentr(LSize,GetMsg(MFolder),nSIZE,L' ');
			FSF.sprintf(buf, L"%*.*s%c%*.*s%c%c%c%s",nSIZE,nSIZE,LSize,0x2551,nSIZE,nSIZE,LSize,0x2551,L' ',0x2502,strVirtDir.get());
			Item.Flags=0x2b; 
			Item.Flags|=LIF_CHECKED;
			Item.Text=buf;
			List.Items=&Item;
			Info.SendDlgMessage(hDlg,DM_LISTADD,0,&List);
			Index++;
		}
	}

	wchar_t Title[MAX_PATH];
	wchar_t Bottom[MAX_PATH];
	FarListTitles ListTitle;
	ListTitle.Title=Title;
	ListTitle.TitleLen=sizeof(Title);
	ListTitle.Bottom=Bottom;
	ListTitle.BottomLen=sizeof(Bottom);;
	Info.SendDlgMessage(hDlg,DM_LISTGETTITLES,0,&ListTitle);
	FSF.sprintf(Bottom,GetMsg(MListBottom),Items,Equal,Diff,LDiff,RDiff);
	ListTitle.Bottom=Bottom;
	Info.SendDlgMessage(hDlg,DM_LISTSETTITLES,0,&ListTitle);

	FarListPos ListPos;
	ListPos.SelectPos=0/*ListInfo.SelectPos*/;
	ListPos.TopPos=-1/*ListInfo.TopPos*/;
	Info.SendDlgMessage(hDlg,DM_LISTSETCURPOS,0,&ListPos);

	Info.SendDlgMessage(hDlg,DM_LISTSETMOUSEREACTION,0,(void*)LMRT_NEVER);
	return true;
}


INT_PTR WINAPI ShowCmpDialogProc(HANDLE hDlg,int Msg,int Param1,void *Param2)
{
	FileList *pFileList=(FileList *)Info.SendDlgMessage(hDlg,DM_GETDLGDATA,0,0);
	switch(Msg)
	{
		case DN_INITDIALOG:
			UpdateFarList(hDlg,pFileList);
			break;

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
			Info.SendDlgMessage(hDlg,DM_LISTSETMOUSEREACTION,0,(void*)LMRT_NEVER);
			Info.SendDlgMessage(hDlg,DM_ENABLEREDRAW,true,0);
			return true;
		}

		case DN_CTLCOLORDLGLIST:
			if (Param1==0)
			{
				static BYTE ColorDlgList[15];
				BYTE ColorIndex[]=
				{
					COL_PANELTEXT,
					COL_PANELBOX,
					COL_PANELBOX,
					COL_PANELTEXT,
					COL_PANELSELECTEDTEXT,
					COL_PANELBOX,
					COL_PANELCURSOR,
					COL_PANELSELECTEDCURSOR,
					COL_PANELSCROLLBAR,
					COL_PANELHIGHLIGHTTEXT,
					COL_PANELSELECTEDTEXT,
					COL_PANELSELECTEDCURSOR,
					COL_PANELSELECTEDTEXT,
					COL_PANELHIGHLIGHTTEXT,
					COL_PANELCURSOR
				};
				for (int i=0; i<sizeof(ColorDlgList); i++)
					ColorDlgList[i]=Info.AdvControl(&MainGuid,ACTL_GETCOLOR,ColorIndex[i],0);

				FarListColors *flc=(FarListColors *)Param2;
				flc->Flags=flc->Reserved=0;
				flc->ColorCount=sizeof(ColorDlgList);
				flc->Colors=ColorDlgList;
				return true;
			}

		case DN_CONTROLINPUT:
		{
			const INPUT_RECORD* record=(const INPUT_RECORD *)Param2;
			if (record->EventType==MOUSE_EVENT)
				if (Param1==0 && record->Event.MouseEvent.dwButtonState==FROM_LEFT_1ST_BUTTON_PRESSED && record->Event.MouseEvent.dwEventFlags==DOUBLE_CLICK)
					goto GOTOCMPFILE;
			if (record->EventType==KEY_EVENT)
			{
				long Key=FSF.FarInputRecordToKey(record);

				if (Key==KEY_ENTER || Key==KEY_NUMENTER)
				{
GOTOCMPFILE:
					int Pos=Info.SendDlgMessage(hDlg,DM_LISTGETCURPOS,0,0);
					File *cur=(File *)Info.SendDlgMessage(hDlg,DM_LISTGETDATA,0,(void *)Pos);
					if (cur)
					{
						if ((LPanel.PInfo.Flags&PFLAGS_PLUGIN) || (RPanel.PInfo.Flags&PFLAGS_PLUGIN) ||
								(cur->dwAttributes&FILE_ATTRIBUTE_DIRECTORY) ||
								!(cur->ftLLastWriteTime.dwLowDateTime || cur->ftLLastWriteTime.dwHighDateTime) ||
								!(cur->ftRLastWriteTime.dwLowDateTime || cur->ftRLastWriteTime.dwHighDateTime) )
						{
							MessageBeep(MB_OK);
							return true;
						}
						string strLFullFileName=cur->LDir;
						if (strLFullFileName.length()>0 && strLFullFileName[(size_t)(strLFullFileName.length()-1)]!=L'\\') strLFullFileName+=L"\\";
						strLFullFileName+=cur->FileName;
						string strRFullFileName=cur->RDir;
						if (strRFullFileName.length()>0 && strRFullFileName[(size_t)(strRFullFileName.length()-1)]!=L'\\') strRFullFileName+=L"\\";
						strRFullFileName+=cur->FileName;

						if (pCompareFiles)
						{
							pCompareFiles(strLFullFileName.get(),strRFullFileName.get(),0);
						}
						else
						{
							bool bFindProg=false;
							WIN32_FIND_DATA wfdFindData;
							HANDLE hFind;
							wchar_t DiffProgram[MAX_PATH];
							ExpandEnvironmentStringsW(L"%ProgramFiles%",DiffProgram,(sizeof(DiffProgram)/sizeof(DiffProgram[0])));
							wcscat(DiffProgram,L"\\WinMerge\\WinMergeU.exe");
							if ((hFind=FindFirstFileW(DiffProgram,&wfdFindData)) != INVALID_HANDLE_VALUE)
							{
								bFindProg=true;
								FindClose(hFind);
								STARTUPINFO si;
								PROCESS_INFORMATION pi;
								memset(&si, 0, sizeof(si));
								si.cb = sizeof(si);
								wchar_t Command[32768];
								FSF.sprintf(Command, L"\"%s\" -e \"%s\" \"%s\"", DiffProgram,strLFullFileName.get()+GetPosToName(strLFullFileName.get()),strRFullFileName.get()+GetPosToName(strRFullFileName.get()));
								CreateProcess(0,Command,0,0,false,0,0,0,&si,&pi);
							}
							if (!bFindProg)
								MessageBeep(MB_ICONASTERISK);
						}
					}
					return true;
				}
				else if (Key==(KEY_CTRL|KEY_PGUP) || Key==(KEY_RCTRL|KEY_PGUP))
				{
					int Pos=Info.SendDlgMessage(hDlg,DM_LISTGETCURPOS,0,0);
					File *cur=(File *)Info.SendDlgMessage(hDlg,DM_LISTGETDATA,0,(void *)Pos);
					if (cur)
					{
						if ((LPanel.PInfo.Flags&PFLAGS_PLUGIN) || (RPanel.PInfo.Flags&PFLAGS_PLUGIN))
						{
							MessageBeep(MB_OK);
							return true;
						}

						// каталог на панели
						string strLPanelDir, strRPanelDir;
						{
							int size=Info.PanelControl(LPanel.hPanel,FCTL_GETPANELDIR,0,0);
							strLPanelDir.get(size);
							Info.PanelControl(LPanel.hPanel,FCTL_GETPANELDIR,size,strLPanelDir.get());
							strLPanelDir.updsize();

							size=Info.PanelControl(RPanel.hPanel,FCTL_GETPANELDIR,0,0);
							strRPanelDir.get(size);
							Info.PanelControl(RPanel.hPanel,FCTL_GETPANELDIR,size,strRPanelDir.get());
							strRPanelDir.updsize();
						}

						PanelRedrawInfo LRInfo={0,0}, RRInfo={0,0};
						bool bSetLDir=false, bSetLFile=false;
						bool bSetRDir=false, bSetRFile=false;

						if (FSF.LStricmp(strLPanelDir.get(),cur->LDir+GetPosToName(cur->LDir)))
						{
							bSetLDir=Info.PanelControl(LPanel.hPanel,FCTL_SETPANELDIR,0,(cur->LDir+GetPosToName(cur->LDir)));
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
												Info.PanelControl(LPanel.hPanel,FCTL_SETSELECTION,i,(void*)(pFileList->F[j].dwFlags&RCIF_DIFFER));
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

						if (FSF.LStricmp(strRPanelDir.get(),cur->RDir+GetPosToName(cur->RDir)))
						{
							bSetRDir=Info.PanelControl(RPanel.hPanel,FCTL_SETPANELDIR,0,(cur->RDir+GetPosToName(cur->RDir)));
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
												Info.PanelControl(RPanel.hPanel,FCTL_SETSELECTION,i,(void*)(pFileList->F[j].dwFlags&RCIF_DIFFER));
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
	}
	else
		strTmp=pLList->Dir+GetPosToName(pLList->Dir);
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
	}
	else
		strTmp=pRList->Dir+GetPosToName(pRList->Dir);
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

	HANDLE hDlg=Info.DialogInit(&MainGuid,&CmpDlgGuid,0,0,WinInfo.Con.Right,WinInfo.Con.Bottom-WinInfo.Con.Top,L"Contents",DialogItems,
															sizeof(DialogItems)/sizeof(DialogItems[0]),0,FDLG_SMALLDIALOG|FDLG_KEEPCONSOLETITLE,ShowCmpDialogProc,&FList);
	if (hDlg != INVALID_HANDLE_VALUE)
	{
		Info.DialogRun(hDlg);
		Info.DialogFree(hDlg);
	}
	if (buf) free(buf);
	return 1;
}
