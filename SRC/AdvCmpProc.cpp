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
	if (nTotal>0) n=nCurrent * (unsigned __int64)len / nTotal;
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
wchar_t *GetStrFileTime(FILETIME *LastWriteTime, wchar_t *Time, bool FullYear)
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
					return (bBrokenByEsc=true);
			}
			else
				return (bBrokenByEsc=true);
		}
	}
	return false;
}

/****************************************************************************
 * Усекает начало длинных имен файлов (или дополняет короткие имена)
 * для правильного показа в сообщении сравнения
 ****************************************************************************/
void TruncCopy(wchar_t *Dest, const wchar_t *Src, int TruncLen, const wchar_t *FormatMsg)
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

int GetArgv(const wchar_t *cmd, wchar_t ***argv)
{
	int l=wcslen(cmd);
	// settings for arguments vectors
	int *pos = (int*) malloc(l * sizeof(*pos) * sizeof(wchar_t));
	int *len = (int*) malloc(l * sizeof(*pos) * sizeof(wchar_t));
	int  i, num=0;
	for (i=0; i<l;)
	{
		while ((cmd[i]==L',' || cmd[i]==L';') && i<l)
			i++;
		if (i>=l)
			break;
		// get argument
		pos[num]=i;
		while ((cmd[i]!=L',' || cmd[i]!=L';') && i<l)
			i++;
		len[num]=i-pos[num];
		num++;
	}

	if (num)
	{
		*argv=(wchar_t**)malloc(num * sizeof(**argv));
		for (i=0; i<num; i++)
		{
			(*argv)[i] = (wchar_t*)malloc((len[i] + 2) * sizeof(***argv));
			lstrcpyn((*argv)[i],cmd+pos[i],len[i]+1);
		}
	}
	free(pos);
	free(len);
	return num;
}




AdvCmpProc::AdvCmpProc()
{
	hScreen=Info.SaveScreen(0,0,-1,-1);
	bStartMsg=true;

	cFList.F=NULL;
	cFList.iCount=0;
	cFList.Items=0;
	cFList.Select=0;
	cFList.Identical=0;
	cFList.Different=0;
	cFList.LNew=0;
	cFList.RNew=0;
	cFList.bShowSelect=true;
	cFList.bShowIdentical=true;
	cFList.bShowDifferent=true;
	cFList.bShowLNew=true;
	cFList.bShowRNew=true;
	cFList.bClearUserFlags=false;
	cFList.Copy=0;
	cFList.bCopyNew=false;

	dFList.F=NULL;
	dFList.iCount=0;
	dFList.GroupCount=0;
	dFList.Del=0;

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

	if (hScreen) Info.RestoreScreen(hScreen);
	// Восстановим заголовок консоли ФАРа...
	if (TitleSaved) SetConsoleTitle(strFarTitle);

	for (int i=0; i<cFList.iCount; i++)
	{
		if (cFList.F[i].FileName) free(cFList.F[i].FileName);
		if (cFList.F[i].LDir) free(cFList.F[i].LDir);
		if (cFList.F[i].RDir) free(cFList.F[i].RDir);
	}
	if (cFList.F) free (cFList.F); cFList.F=NULL;
	cFList.iCount=0;

	for (int i=0; i<dFList.iCount; i++)
	{
		if (dFList.F[i].FileName) free(dFList.F[i].FileName);
		if (dFList.F[i].Dir) free(dFList.F[i].Dir);
		if (dFList.F[i].PicPix) free(dFList.F[i].PicPix);
		if (dFList.F[i].MusicArtist) free(dFList.F[i].MusicArtist);
		if (dFList.F[i].MusicTitle) free(dFList.F[i].MusicTitle);
	}
	if (dFList.F) free (dFList.F); dFList.F=NULL;
	dFList.iCount=0;
}

/****************************************************************************
 *
 *                    Разные оччччень полезные функции :-)
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
 *
 *                          COMPAREFILES FUNCTIONS
 *
 ****************************************************************************/


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
				if (Opt.Subfolders==2 && Opt.MaxScanDepth<ScanDepth+1) // не глубже заданного уровня!
					break;
				if (!Opt.Subfolders)
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
		if ( (Opt.Subfolders || !(pList->PPI[i].FileAttributes & FILE_ATTRIBUTE_DIRECTORY)) &&
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

					if (Opt.Subfolders==2 && !CheckScanDepth(pList->PPI[i].FileName, Opt.MaxScanDepth))
						continue;
				}

				// плагин + панель || панель + плагин (элемент с плагина)
				else if ((bLPanelPlug && !bRPanelPlug && bLeftPanel) || (!bLPanelPlug && bRPanelPlug && !bLeftPanel))
				{
					if (Opt.Subfolders==2 && !CheckScanDepth(pList->PPI[i].FileName, Opt.MaxScanDepth))
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
	for (int i=0; i<Cache.ItemsNumber; i++)
	{
		if ( ((FullFileName1==Cache.RCI[i].dwFullFileName[0] && FullFileName2==Cache.RCI[i].dwFullFileName[1]) &&
					(WriteTime1==Cache.RCI[i].dwWriteTime[0] && WriteTime2==Cache.RCI[i].dwWriteTime[1]))
				|| ((FullFileName1==Cache.RCI[i].dwFullFileName[1] && FullFileName2==Cache.RCI[i].dwFullFileName[0]) &&
					(WriteTime1==Cache.RCI[i].dwWriteTime[1] && WriteTime2==Cache.RCI[i].dwWriteTime[0])) )
		{
			Cache.RCI[i].dwFlags=dwFlag; // был такой, обновим. сделаем "тупо" :-)
			return true;
		}
	}

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
	return true;
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

	wchar_t LDiff[64], RDiff[64], Errors[64], DiffOut[MAX_PATH];
	FSF.sprintf(Buf,GetMsg(MComparingDiffN),itoaa(CmpInfo.LDiff,LDiff),itoaa(CmpInfo.RDiff,RDiff),itoaa(CmpInfo.Errors,Errors));
	strcentr(DiffOut,Buf,WinInfo.TruncLen,0x00002500);

	wchar_t ProgressLineCur[MAX_PATH], ProgressLineTotal[MAX_PATH];
	if (!Opt.CmpContents || bStartMsg)
		wcscpy(ProgressLineCur,GetMsg(MWait));
	else
		ProgressLine(ProgressLineCur,CmpInfo.CurProcSize,CmpInfo.CurCountSize);

	if (Opt.TotalProgress)
	{
		FSF.sprintf(Buf,GetMsg(MComparingFiles2),CmpInfo.CountSize && !((LPanel.PInfo.Flags&PFLAGS_PLUGIN) || (RPanel.PInfo.Flags&PFLAGS_PLUGIN))?(CmpInfo.ProcSize*100/CmpInfo.CountSize):0);
		SetConsoleTitle(Buf);

		wchar_t Count[64], CountSize[64];
		FSF.sprintf(Buf,GetMsg(MComparingN),itoaa(CmpInfo.CountSize,CountSize),itoaa(CmpInfo.Count,Count));
		strcentr(ItemsOut,Buf,WinInfo.TruncLen,0x00002500);

		if ((LPanel.PInfo.Flags&PFLAGS_PLUGIN) || (RPanel.PInfo.Flags&PFLAGS_PLUGIN))
			wcscpy(ProgressLineTotal,GetMsg(MWait));
		else
			ProgressLine(ProgressLineTotal,CmpInfo.ProcSize,CmpInfo.CountSize);
	}
	strcentr(Buf,L"",WinInfo.TruncLen,0x00002500);  // просто сепаратор

	const wchar_t *MsgItems1[] = {
		GetMsg(MCmpTitle),
		TruncDir1,TruncName1,DiffOut,ProgressLineCur,Buf,TruncDir2,TruncName2
	};

	const wchar_t *MsgItems2[] = {
		GetMsg(MCmpTitle),
		TruncDir1,TruncName1,DiffOut,ProgressLineCur,Buf,ProgressLineTotal,ItemsOut,TruncDir2,TruncName2
	};


	Info.Message(&MainGuid,&CmpMsgGuid,bStartMsg?FMSG_LEFTALIGN:FMSG_LEFTALIGN|FMSG_KEEPBACKGROUND,0,Opt.TotalProgress?MsgItems2:MsgItems1,
																		Opt.TotalProgress?sizeof(MsgItems2)/sizeof(MsgItems2[0]):sizeof(MsgItems1)/sizeof(MsgItems1[0]),0);
	bStartMsg=false;
}

/****************************************************************************
 * Показывает сообщение о сравнении двух файлов
 ****************************************************************************/
void AdvCmpProc::ShowDupMsg(const wchar_t *Dir, const wchar_t *Name, bool bRedraw)
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

	wchar_t TruncDir[MAX_PATH], TruncName[MAX_PATH];
	TruncCopy(TruncDir, GetPosToName(Dir), WinInfo.TruncLen, GetMsg(MComparing));
	TruncCopy(TruncName, Name, WinInfo.TruncLen);

	wchar_t CurProc[64], Errors[64], CurProcOut[MAX_PATH];
	FSF.sprintf(Buf,GetMsg(MDupCurProc),itoaa(CmpInfo.Proc,CurProc),itoaa(CmpInfo.Errors,Errors));
	strcentr(CurProcOut,Buf,WinInfo.TruncLen,0x00002500);

	wchar_t ProgressLineCur[MAX_PATH], ProgressLineTotal[MAX_PATH];
	if (!Opt.DupContents || bStartMsg)
		wcscpy(ProgressLineCur,GetMsg(MWait));
	else
		ProgressLine(ProgressLineCur,CmpInfo.CurProcSize,CmpInfo.CurCountSize);

	if (Opt.TotalProgress)
	{
		FSF.sprintf(Buf,GetMsg(MComparingFiles2),CmpInfo.CountSize && !((LPanel.PInfo.Flags&PFLAGS_PLUGIN) || (RPanel.PInfo.Flags&PFLAGS_PLUGIN))?(CmpInfo.ProcSize*100/CmpInfo.CountSize):0);
		SetConsoleTitle(Buf);

		wchar_t Count[64], CountSize[64];
		FSF.sprintf(Buf,GetMsg(MComparingN),itoaa(CmpInfo.CountSize,CountSize),itoaa(CmpInfo.Count,Count));
		strcentr(ItemsOut,Buf,WinInfo.TruncLen,0x00002500);

		if ((LPanel.PInfo.Flags&PFLAGS_PLUGIN) || (RPanel.PInfo.Flags&PFLAGS_PLUGIN))
			wcscpy(ProgressLineTotal,GetMsg(MWait));
		else
			ProgressLine(ProgressLineTotal,CmpInfo.ProcSize,CmpInfo.CountSize);
	}
	strcentr(Buf,L"",WinInfo.TruncLen,0x00002500);  // просто сепаратор

	const wchar_t *MsgItems1[] = {
		GetMsg(MCmpTitle),
		TruncDir,TruncName,CurProcOut,ProgressLineCur
	};

	const wchar_t *MsgItems2[] = {
		GetMsg(MCmpTitle),
		TruncDir,TruncName,CurProcOut,ProgressLineCur,Buf,ProgressLineTotal,ItemsOut
	};


	Info.Message(&MainGuid,&DupMsgGuid,bStartMsg?FMSG_LEFTALIGN:FMSG_LEFTALIGN|FMSG_KEEPBACKGROUND,0,Opt.TotalProgress?MsgItems2:MsgItems1,
																		Opt.TotalProgress?sizeof(MsgItems2)/sizeof(MsgItems2[0]):sizeof(MsgItems1)/sizeof(MsgItems1[0]),0);
	bStartMsg=false;
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
		if (Opt.Subfolders)
		{
			if (Opt.Subfolders==2 && Opt.MaxScanDepth<ScanDepth+1)
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
												(!Opt.PartlyFull && PartlyKbSize && pLPPI->FileSize > (unsigned __int64)abs(PartlyKbSize)) );

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
						while (LBufPos < (unsigned)Opt.BufSize)
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
						while (RBufPos < (unsigned)Opt.BufSize)
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
	if (ScanDepth==0 && Opt.TotalProgress)
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
					if (Opt.TillFirstDiff && !bBrokenByEsc)
					{     // нужно ли продолжать сравнивать
						bCompareAll=(Opt.ShowMsg && !YesNoMsg(MFirstDiffTitle, MFirstDiffBody));
						Opt.TillFirstDiff=0;
					}
				}
				CmpInfo.Proc+=2;
				// добавим элемент в диалог результатов
				if (Opt.Dialog)
					MakeFileList(pLList->Dir,LII.pPPI[i-1],pRList->Dir,RII.pPPI[j-1],dwFlag);
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
				if (!Opt.IgnoreMissing || (Opt.IgnoreMissing==2 && ScanDepth))
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
									if (Opt.TillFirstDiff && !bBrokenByEsc)
									{
										bCompareAll=(Opt.ShowMsg && !YesNoMsg(MFirstDiffTitle, MFirstDiffBody));
										Opt.TillFirstDiff=0;
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
						MakeFileList(pLList->Dir,LII.pPPI[i-1],pRList->Dir,NULL,dwFlag);
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
				if (!Opt.IgnoreMissing || (Opt.IgnoreMissing==2 && ScanDepth))
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
									if (Opt.TillFirstDiff && !bBrokenByEsc)
									{
										bCompareAll=(Opt.ShowMsg && !YesNoMsg(MFirstDiffTitle, MFirstDiffBody));
										Opt.TillFirstDiff=0;
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
						MakeFileList(pLList->Dir,NULL,pRList->Dir,RII.pPPI[j-1],dwFlag);
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
		if ((!Opt.IgnoreMissing || (Opt.IgnoreMissing==2 && ScanDepth)) && i<LII.iCount)
		{
			if (!LPanel.bTMP)
				bDifferenceNotFound=false;
			if (bCompareAll)
				goto CmpContinueL;
		}
		if ((!Opt.IgnoreMissing || (Opt.IgnoreMissing==2 && ScanDepth)) && j<RII.iCount)
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

#include "AdvCmpProc_CLIST.cpp"
#include "AdvCmpProc_SYNC.cpp"
#include "AdvCmpProc_DUP.cpp"


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
				if (!pGflChangeColorDepth(RawPicture,NULL,GFL_MODE_TO_BGR,GFL_MODE_NO_DITHER)/* && !pGflRotate(RawPicture,NULL,data->Rotate,0)*/)
				{
					{
						int dx=WinInfo.Win.right/(WinInfo.Con.Right-WinInfo.Con.Left);
						int dy=WinInfo.Win.bottom/(WinInfo.Con.Bottom-WinInfo.Con.Top);

						RECT DCRect;
						DCRect.left=dx*(data->DrawRect.left-WinInfo.Con.Left);
						DCRect.right=dx*(data->DrawRect.right+1-WinInfo.Con.Left);
						DCRect.top=dy*(data->DrawRect.top/*-WinInfo.Con.Top*/);          //костыль для запуска far.exe /w
						DCRect.bottom=dy*(data->DrawRect.bottom+1/*-WinInfo.Con.Top*/);  //костыль для запуска far.exe /w

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
					data->MemSize=((RawPicture->Width*3+3)&-4)*RawPicture->Height;
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
	cmpPicFile *pPics=(cmpPicFile *)Info.SendDlgMessage(hDlg,DM_GETDLGDATA,0,0);
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
			pPics->L.Redraw=true;
			pPics->R.Redraw=true;
			break;

		case DN_ENTERIDLE:
			if (pPics->L.Redraw)
			{
				pPics->L.Redraw=false;
				UpdateImage(&pPics->L);
				if (pPics->L.FirstRun)
				{
					pPics->L.FirstRun=false;
					UpdateInfoText(hDlg,&pPics->L,true);
					UpdateInfoText(hDlg,&pPics->R,false);
				}
			}
			if (pPics->R.Redraw)
			{
				pPics->R.Redraw=false;
				UpdateImage(&pPics->R);
				if (pPics->R.FirstRun)
				{
					pPics->R.FirstRun=false;
					UpdateInfoText(hDlg,&pPics->L,true);
					UpdateInfoText(hDlg,&pPics->R,false);
				}
			}
			break;

		case 0x3FFF:
			if (Param1)
			{
				UpdateImage(&pPics->L);
				UpdateImage(&pPics->R);
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
	string strBuf1, strBuf2, strBuf3;
	strBuf1.get(WinInfo.Con.Right/2);
	strBuf2.get(WinInfo.Con.Right/2);
	strBuf3.get(WinInfo.Con.Right/2);

	itoaa(pLPPI->FileSize,Buf1);
	itoaa(CmpPic.L.MemSize,Buf2);
	FSF.sprintf(strBuf2.get(), L"(%s) %s",Buf2,Buf1);
	strBuf2.updsize();
	FSF.sprintf(strBuf1.get(), L"%*.*s",DialogItems[5].X2,DialogItems[5].X2,strBuf2.get());
	strBuf1.updsize();
	DialogItems[5].Data=strBuf1.get();

	itoaa(pRPPI->FileSize,Buf1);
	itoaa(CmpPic.R.MemSize,Buf2);
	FSF.sprintf(strBuf2.get(), L"%s (%s)",Buf1,Buf2);
	strBuf2.updsize();
	DialogItems[6].Data=strBuf2.get();

	FSF.sprintf(Buf2, L"%-*.*s  %02d.%02d.%04d  %02d:%02d:%02d", 
							10,10,LAttributes,LModificTime.wDay,LModificTime.wMonth,LModificTime.wYear,LModificTime.wHour,LModificTime.wMinute,LModificTime.wSecond);
	FSF.sprintf(strBuf3.get(), L"%*.*s",DialogItems[7].X2,DialogItems[7].X2,Buf2);
	strBuf3.updsize();
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

bool AdvCmpProc::CompareCurFile(const wchar_t *LDir, const wchar_t *LFileName, const wchar_t *RDir, const wchar_t *RFileName, int Method)
{
	string strLFullFileName, strRFullFileName;
	GetFullFileName(strLFullFileName,LDir,LFileName);
	GetFullFileName(strRFullFileName,RDir,RFileName);

	PluginPanelItem LPPI, RPPI;
	memset(&LPPI,0,sizeof(PluginPanelItem));
	memset(&RPPI,0,sizeof(PluginPanelItem));
	if (!FileExists(strLFullFileName.get(),&LPPI.FileSize,&LPPI.LastWriteTime,&LPPI.FileAttributes,0) ||
			!FileExists(strRFullFileName.get(),&RPPI.FileSize,&RPPI.LastWriteTime,&RPPI.FileAttributes,0))
		return false;
	LPPI.FileName=LFileName;
	RPPI.FileName=RFileName;

	string strCommand;
	strCommand.get(32768);
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
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
		memset(&si, 0, sizeof(si));
		si.cb = sizeof(si);
		FSF.sprintf(strCommand.get(), L"\"%s\" -e \"%s\" \"%s\"", DiffProgram,GetPosToName(strLFullFileName.get()),GetPosToName(strRFullFileName.get()));
		strCommand.updsize();
	}

	if (Method) // перебираем всё
	{
		bool bImage=false;
		if (bGflLoaded)
		{
			CmpPic.L.FileName=strLFullFileName.get();
			CmpPic.R.FileName=strRFullFileName.get();
			CmpPic.L.DrawRect.left=1;
			CmpPic.R.DrawRect.left=WinInfo.Con.Right/2+2;
			CmpPic.L.DrawRect.top=CmpPic.R.DrawRect.top=1;
			CmpPic.L.DrawRect.right=WinInfo.Con.Right/2-1;
			CmpPic.R.DrawRect.right=WinInfo.Con.Right-1;
			CmpPic.L.DrawRect.bottom=CmpPic.R.DrawRect.bottom=WinInfo.Con.Bottom-WinInfo.Con.Top-2-2;
			CmpPic.L.FirstRun=CmpPic.R.FirstRun=true;
			CmpPic.L.Redraw=CmpPic.R.Redraw=false;
			CmpPic.L.Loaded=CmpPic.R.Loaded=false;
			BITMAPINFOHEADER BmpHeader1, BmpHeader2;
			CmpPic.L.BmpHeader=&BmpHeader1;
			CmpPic.R.BmpHeader=&BmpHeader2;
			CmpPic.L.DibData=CmpPic.R.DibData=NULL;
			GFL_FILE_INFORMATION pic_info1, pic_info2;
			CmpPic.L.pic_info=&pic_info1;
			CmpPic.R.pic_info=&pic_info2;
			CmpPic.L.Page=CmpPic.R.Page=1;
			CmpPic.L.Rotate=CmpPic.R.Rotate=0;

			bImage=(UpdateImage(&CmpPic.L,true) && UpdateImage(&CmpPic.R,true));
		}

		struct FarMenuItem MenuItems[4];
		memset(MenuItems,0,sizeof(MenuItems));
		MenuItems[0].Text=GetMsg(MDefault);
		MenuItems[1].Text=GetMsg(MWinMerge);
		MenuItems[2].Text=GetMsg(MPictures);
		MenuItems[3].Text=GetMsg(MVisCmp);
		if (!pCompareFiles) MenuItems[3].Flags|=MIF_GRAYED;
		if (!bFindDiffProg) MenuItems[1].Flags|=MIF_GRAYED;
		if (!bImage) MenuItems[2].Flags|=MIF_GRAYED;
		if (bImage) MenuItems[2].Flags|=MIF_SELECTED;
		else if (pCompareFiles) MenuItems[3].Flags|=MIF_SELECTED;
		int MenuCode=Info.Menu(&MainGuid,&CmpMethodMenuGuid,-1,-1,0,FMENU_AUTOHIGHLIGHT|FMENU_WRAPMODE,L"Method",NULL,L"Contents",NULL,NULL,MenuItems,sizeof(MenuItems)/sizeof(MenuItems[0]));

		if (MenuCode==0)
		{
			bool bDifferenceNotFound;
			Opt.TotalProgress=0;
			if (Opt.CmpCase || Opt.CmpSize || Opt.CmpTime || Opt.CmpContents)
			{
				bDifferenceNotFound=CompareFiles(LDir,&LPPI,RDir,&RPPI,0);
			}
			else
				bDifferenceNotFound=!FSF.LStricmp(LFileName,RFileName);

			Info.PanelControl(LPanel.hPanel,FCTL_REDRAWPANEL,0,0);
			Info.PanelControl(RPanel.hPanel,FCTL_REDRAWPANEL,0,0);

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
			if (CreateProcess(0,strCommand.get(),0,0,false,0,0,0,&si,&pi))
			{
				WaitForSingleObject(pi.hProcess,INFINITE);
				CloseHandle(pi.hProcess);
				CloseHandle(pi.hThread);
			}
		}
		else if (MenuCode==2 && bImage)
			ShowCmpCurDialog(&LPPI,&RPPI);
		else if (MenuCode==3 && pCompareFiles)
			pCompareFiles(strLFullFileName.get(),strRFullFileName.get(),0);

		FreeImage(&CmpPic.L);
		FreeImage(&CmpPic.R);
	}
	else if (bFindDiffProg) // WinMerge
	{
		if (CreateProcess(0,strCommand.get(),0,0,false,0,0,0,&si,&pi))
		{
			WaitForSingleObject(pi.hProcess,INFINITE);
			CloseHandle(pi.hProcess);
			CloseHandle(pi.hThread);
		}
	}

	return true;
}
