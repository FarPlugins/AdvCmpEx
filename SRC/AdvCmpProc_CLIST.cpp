/****************************************************************************
 * AdvCmpProc_CLIST.cpp
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


/***************************************************************************
 *
 *                       ƒ»јЋќ√ –≈«”Ћ№“ј“ќ¬ —–ј¬Ќ≈Ќ»я
 *
 ***************************************************************************/


/****************************************************************************
 * ѕостроение массива элементов, дл€ диалога с результатами сравнени€
 ****************************************************************************/
bool AdvCmpProc::MakeFileList(const wchar_t *LDir,const PluginPanelItem *pLPPI,const wchar_t *RDir,const PluginPanelItem *pRPPI,DWORD dwFlag)
{
	cmpFile *New=(cmpFile*)realloc(cFList.F,(cFList.iCount+1)*sizeof(cmpFile));
	if (!New)
	{
		ErrorMsg(MNoMemTitle, MNoMemBody);
		return false;
	}
	cFList.F=New;

	cFList.F[cFList.iCount].dwFlags=dwFlag;

	if (pLPPI && pRPPI && Opt.CmpTime && dwFlag==RCIF_DIFFER)
	{
		__int64 Delta=(((__int64)pLPPI->LastWriteTime.dwHighDateTime << 32) | pLPPI->LastWriteTime.dwLowDateTime) -
									(((__int64)pRPPI->LastWriteTime.dwHighDateTime << 32) | pRPPI->LastWriteTime.dwLowDateTime);
		if (Delta>0) cFList.F[cFList.iCount].dwFlags=RCIF_LNEW;
		else if (Delta<0) cFList.F[cFList.iCount].dwFlags=RCIF_RNEW;
	}

	if (pLPPI && !pRPPI)
		cFList.F[cFList.iCount].dwFlags|=RCIF_LUNIQ;
	else if (!pLPPI && pRPPI)
		cFList.F[cFList.iCount].dwFlags|=RCIF_RUNIQ;

	cFList.F[cFList.iCount].LDir=(wchar_t*)malloc((wcslen(LDir)+1)*sizeof(wchar_t));
	if (cFList.F[cFList.iCount].LDir)
		wcscpy(cFList.F[cFList.iCount].LDir,LDir);
	cFList.F[cFList.iCount].RDir=(wchar_t*)malloc((wcslen(RDir)+1)*sizeof(wchar_t));
	if (cFList.F[cFList.iCount].RDir)
		wcscpy(cFList.F[cFList.iCount].RDir,RDir);

	if (pLPPI)
	{
		cFList.F[cFList.iCount].FileName=(wchar_t*)malloc((wcslen(pLPPI->FileName)+1)*sizeof(wchar_t));
		if (cFList.F[cFList.iCount].FileName) wcscpy(cFList.F[cFList.iCount].FileName,pLPPI->FileName);
		cFList.F[cFList.iCount].ftLLastWriteTime.dwLowDateTime=pLPPI->LastWriteTime.dwLowDateTime;
		cFList.F[cFList.iCount].ftLLastWriteTime.dwHighDateTime=pLPPI->LastWriteTime.dwHighDateTime;
		cFList.F[cFList.iCount].nLFileSize=pLPPI->FileSize;
		cFList.F[cFList.iCount].dwLAttributes=pLPPI->FileAttributes;
	}
	else
	{
		cFList.F[cFList.iCount].ftLLastWriteTime.dwLowDateTime=0;
		cFList.F[cFList.iCount].ftLLastWriteTime.dwHighDateTime=0;
		cFList.F[cFList.iCount].nLFileSize=0;
		cFList.F[cFList.iCount].dwLAttributes=0;
	}
	if (pRPPI)
	{
		if (!(cFList.F[cFList.iCount].FileName))
		{
			cFList.F[cFList.iCount].FileName=(wchar_t*)malloc((wcslen(pRPPI->FileName)+1)*sizeof(wchar_t));
			if (cFList.F[cFList.iCount].FileName) wcscpy(cFList.F[cFList.iCount].FileName,pRPPI->FileName);
		}
		cFList.F[cFList.iCount].ftRLastWriteTime.dwLowDateTime=pRPPI->LastWriteTime.dwLowDateTime;
		cFList.F[cFList.iCount].ftRLastWriteTime.dwHighDateTime=pRPPI->LastWriteTime.dwHighDateTime;
		cFList.F[cFList.iCount].nRFileSize=pRPPI->FileSize;
		cFList.F[cFList.iCount].dwRAttributes=pRPPI->FileAttributes;
	}
	else
	{
		cFList.F[cFList.iCount].ftRLastWriteTime.dwLowDateTime=0;
		cFList.F[cFList.iCount].ftRLastWriteTime.dwHighDateTime=0;
		cFList.F[cFList.iCount].nRFileSize=0;
		cFList.F[cFList.iCount].dwRAttributes=0;
	}

	cFList.iCount++;

	return true;
}

int __cdecl cmpSortList(const void *el1, const void *el2)
{
	struct cmpFile *Item1=(struct cmpFile *)el1, *Item2=(struct cmpFile *)el2;

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

/***************************************************************************
 * —оздание текстовой метки дл€ элемента листа
 ***************************************************************************/
void MakeListItemText(wchar_t *buf, cmpFile *cur, wchar_t Mark)
{
	wchar_t LTime[20]={0}, RTime[20]={0};

	if (!(cur->dwFlags&RCIF_RUNIQ))  // есть элемент слева
		GetStrFileTime(&cur->ftLLastWriteTime,LTime,false);
	if (!(cur->dwFlags&RCIF_LUNIQ))  // есть элемент справа
		GetStrFileTime(&cur->ftRLastWriteTime,RTime,false);

	wchar_t LSize[65]={0}, RSize[65]={0};
	const int nSIZE=32; // размер €чейки дл€ FSF.sprintf(%*.*)

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

/***************************************************************************
 * »зменение строки статуса в листе
***************************************************************************/
void SetBottom(HANDLE hDlg, cmpFileList *pFileList, wchar_t *CurDir)
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
//	ListTitle.Bottom=Bottom;
	Info.SendDlgMessage(hDlg,DM_LISTSETTITLES,0,&ListTitle);
}

/***************************************************************************
 * »зменение/обновление листа файлов в диалоге
 ***************************************************************************/
bool MakeCmpFarList(HANDLE hDlg, cmpFileList *pFileList, bool bSetCurPos, bool bSort)
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
		FSF.qsort(pFileList->F,pFileList->iCount,sizeof(pFileList->F[0]),cmpSortList);

	int Index=0;
	wchar_t buf[65536];
	const int nSIZE=32; // размер €чейки дл€ FSF.sprintf(%*.*)

	for (int i=0; i<pFileList->iCount; i++)
	{
		cmpFile *cur=&pFileList->F[i];

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

			if (!pFileList->bClearUserFlags)
			{
				__int64 Delta=(((__int64)cur->ftLLastWriteTime.dwHighDateTime << 32) | cur->ftLLastWriteTime.dwLowDateTime) -
											(((__int64)cur->ftRLastWriteTime.dwHighDateTime << 32) | cur->ftRLastWriteTime.dwLowDateTime);

				if ((pFileList->Copy>0 && !pFileList->bCopyNew) || (pFileList->Copy>0 && pFileList->bCopyNew && Delta>0))
				{
					cur->dwFlags|=RCIF_USERLNEW;
					cur->dwFlags&=~RCIF_USERRNEW;
				}
				else if ((pFileList->Copy<0 && !pFileList->bCopyNew) || (pFileList->Copy<0 && pFileList->bCopyNew && Delta<0))
				{
					cur->dwFlags|=RCIF_USERRNEW;
					cur->dwFlags&=~RCIF_USERLNEW;
				}
				else
				{
					cur->dwFlags&=~RCIF_USERLNEW;
					cur->dwFlags&=~RCIF_USERRNEW;
				}
			}

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

		// виртуальна€ папка
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
				cmpFile *next=&pFileList->F[j];

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
		ListPos.SelectPos=ListInfo.SelectPos;
		ListPos.TopPos=ListInfo.TopPos;
		Info.SendDlgMessage(hDlg,DM_LISTSETCURPOS,0,&ListPos);
	}
	return true;
}

bool bSetBottom=false;

/***************************************************************************
 * ќбработчик диалога
 ***************************************************************************/
INT_PTR WINAPI ShowCmpDialogProc(HANDLE hDlg,int Msg,int Param1,void *Param2)
{
	cmpFileList *pFileList=(cmpFileList *)Info.SendDlgMessage(hDlg,DM_GETDLGDATA,0,0);
	switch(Msg)
	{
		case DN_INITDIALOG:
			MakeCmpFarList(hDlg,pFileList,true,true);
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
			WinInfo.TruncLen=WinInfo.Con.Right-WinInfo.Con.Left-20+1;
			if (WinInfo.TruncLen>MAX_PATH-2) WinInfo.TruncLen=MAX_PATH-2;
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
					COL_PANELHIGHLIGHTTEXT,   // виртуальна€ папка
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
					if (i==9) // виртуальна€ папка
					{
						int color=0x1F;
						Colors->Colors[i].Flags=FCF_FG_4BIT|FCF_BG_4BIT;
						Colors->Colors[i].ForegroundColor=color;
						Colors->Colors[i].BackgroundColor=color>>4;
					}
				}
				return true;
			}
			break;

	/************************************************************************/

		case DN_CLOSE:
			if (Opt.Mode==MODE_SYNC && Param1==-1)
			{
				Opt.Sync=GetSyncOpt(pFileList);
				if (Opt.Sync==QR_EDIT)
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
				if (Opt.Mode==MODE_SYNC && Param1==0 && record->Event.MouseEvent.dwButtonState==FROM_LEFT_1ST_BUTTON_PRESSED && record->Event.MouseEvent.dwEventFlags!=DOUBLE_CLICK)
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
					if (Opt.Mode==MODE_CMP || record->Event.MouseEvent.dwMousePosition.X<dlgRect.Left+68 || record->Event.MouseEvent.dwMousePosition.X>dlgRect.Left+70)
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
						INPUT_RECORD rec;
						if (FSF.FarNameToInputRecord(L"F10",&rec))
							Info.SendDlgMessage(hDlg,DM_KEY,1,&rec);
						else
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
						cmpFile **tmp=(cmpFile **)Info.SendDlgMessage(hDlg,DM_LISTGETDATA,0,(void *)Pos);
						cmpFile *cur=(tmp && *tmp)?*tmp:NULL;
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
					if (vk==VK_F3||vk==VK_F4)
					{
						int Pos=Info.SendDlgMessage(hDlg,DM_LISTGETCURPOS,0,0);
						cmpFile **tmp=(cmpFile **)Info.SendDlgMessage(hDlg,DM_LISTGETDATA,0,(void *)Pos);
						cmpFile *cur=(tmp && *tmp)?*tmp:NULL;
						if (cur)
						{
							string strFullFileName;
							if (vk==VK_F3 && !(cur->dwFlags&RCIF_RUNIQ) && !(cur->dwLAttributes&FILE_ATTRIBUTE_DIRECTORY))
								GetFullFileName(strFullFileName,cur->LDir,cur->FileName);
							else if (vk==VK_F4 && !(cur->dwFlags&RCIF_LUNIQ) && !(cur->dwRAttributes&FILE_ATTRIBUTE_DIRECTORY))
								GetFullFileName(strFullFileName,cur->RDir,cur->FileName);
							if (strFullFileName.length() && Info.Viewer(GetPosToName(strFullFileName.get()),NULL,0,0,-1,-1,VF_DISABLEHISTORY,CP_AUTODETECT))
								return true;
						}
						MessageBeep(MB_OK);
						return true;
					}
					else 
#endif
					if (vk==VK_RETURN)
					{
GOTOCMPFILE:
						int Pos=Info.SendDlgMessage(hDlg,DM_LISTGETCURPOS,0,0);
						cmpFile **tmp=(cmpFile **)Info.SendDlgMessage(hDlg,DM_LISTGETDATA,0,(void *)Pos);
						cmpFile *cur=tmp?*tmp:NULL;
						if (cur)
						{
							if ((LPanel.PInfo.Flags&PFLAGS_PLUGIN) || (RPanel.PInfo.Flags&PFLAGS_PLUGIN) ||
									(cur->dwLAttributes&FILE_ATTRIBUTE_DIRECTORY) || (cur->dwRAttributes&FILE_ATTRIBUTE_DIRECTORY) ||
									(cur->dwFlags&RCIF_LUNIQ) || (cur->dwFlags&RCIF_RUNIQ))
							{
								MessageBeep(MB_OK);
								return true;
							}
/*
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
									if (CreateProcess(0,Command,0,0,false,0,0,0,&si,&pi))
									{
										WaitForSingleObject(pi.hProcess,INFINITE);
										CloseHandle(pi.hProcess);
										CloseHandle(pi.hThread);
									}
								}
								else
									MessageBeep(MB_ICONASTERISK);
							}
*/

							if (!pFileList->AdvCmp->CompareCurFile(cur->LDir,cur->FileName,cur->RDir,cur->FileName,0))
								MessageBeep(MB_ICONASTERISK);
						}
						return true;
					}
					else if (vk==VK_INSERT)
					{
						struct FarListPos FLP;
						Info.SendDlgMessage(hDlg,DM_LISTGETCURPOS,0,&FLP);
						cmpFile **tmp=(cmpFile **)Info.SendDlgMessage(hDlg,DM_LISTGETDATA,0,(void *)FLP.SelectPos);
						cmpFile *cur=tmp?*tmp:NULL;
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
						cmpFile **tmp=(cmpFile **)Info.SendDlgMessage(hDlg,DM_LISTGETDATA,0,(void *)Pos);
						cmpFile *cur=tmp?*tmp:NULL;
						if (cur && !(cur->dwFlags&RCIF_EQUAL) && Opt.Mode==MODE_SYNC)
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
				else if (IsShift(record))
				{
					if (vk==VK_RETURN)
					{
						int Pos=Info.SendDlgMessage(hDlg,DM_LISTGETCURPOS,0,0);
						cmpFile **tmp=(cmpFile **)Info.SendDlgMessage(hDlg,DM_LISTGETDATA,0,(void *)Pos);
						cmpFile *cur=tmp?*tmp:NULL;
						if (cur)
						{
							if ((LPanel.PInfo.Flags&PFLAGS_PLUGIN) || (RPanel.PInfo.Flags&PFLAGS_PLUGIN) ||
									(cur->dwLAttributes&FILE_ATTRIBUTE_DIRECTORY) || (cur->dwRAttributes&FILE_ATTRIBUTE_DIRECTORY) ||
									(cur->dwFlags&RCIF_LUNIQ) || (cur->dwFlags&RCIF_RUNIQ))
							{
								MessageBeep(MB_OK);
								return true;
							}
							if (!pFileList->AdvCmp->CompareCurFile(cur->LDir,cur->FileName,cur->RDir,cur->FileName,1))
								MessageBeep(MB_ICONASTERISK);
						}
						return true;
					}
				}
				else if (IsCtrl(record))
				{
					if (vk==VK_CONTROL)
					{
						int Pos=Info.SendDlgMessage(hDlg,DM_LISTGETCURPOS,0,0);
						cmpFile **tmp=(cmpFile **)Info.SendDlgMessage(hDlg,DM_LISTGETDATA,0,(void *)Pos);
						cmpFile *cur=tmp?*tmp:NULL;
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
						MakeCmpFarList(hDlg,pFileList);
						pFileList->bClearUserFlags=false; // восстановим!
						Info.SendDlgMessage(hDlg,DM_ENABLEREDRAW,true,0);
						return true;
					}
					else if (vk==VK_OEM_5 && !(pFileList->bShowSelect && pFileList->Select==0))
					{
						Info.SendDlgMessage(hDlg,DM_ENABLEREDRAW,false,0);
						pFileList->bShowSelect=(pFileList->bShowSelect?false:true);
						MakeCmpFarList(hDlg,pFileList);
						Info.SendDlgMessage(hDlg,DM_ENABLEREDRAW,true,0);
						return true;
					}
					else if (vk==VK_OEM_PLUS && !(pFileList->bShowIdentical && pFileList->Identical==0))
					{
						Info.SendDlgMessage(hDlg,DM_ENABLEREDRAW,false,0);
						pFileList->bShowIdentical=(pFileList->bShowIdentical?false:true);
						MakeCmpFarList(hDlg,pFileList);
						Info.SendDlgMessage(hDlg,DM_ENABLEREDRAW,true,0);
						return true;
					}
					else if (vk==VK_OEM_MINUS && !(pFileList->bShowDifferent && pFileList->Different==0))
					{
						Info.SendDlgMessage(hDlg,DM_ENABLEREDRAW,false,0);
						pFileList->bShowDifferent=(pFileList->bShowDifferent?false:true);
						MakeCmpFarList(hDlg,pFileList);
						Info.SendDlgMessage(hDlg,DM_ENABLEREDRAW,true,0);
						return true;
					}
					else if (vk==VK_OEM_4 && !(pFileList->bShowLNew && pFileList->LNew==0))
					{
						Info.SendDlgMessage(hDlg,DM_ENABLEREDRAW,false,0);
						pFileList->bShowLNew=(pFileList->bShowLNew?false:true);
						MakeCmpFarList(hDlg,pFileList);
						Info.SendDlgMessage(hDlg,DM_ENABLEREDRAW,true,0);
						return true;
					}
					else if (vk==VK_OEM_6 && !(pFileList->bShowRNew && pFileList->RNew==0))
					{
						Info.SendDlgMessage(hDlg,DM_ENABLEREDRAW,false,0);
						pFileList->bShowRNew=(pFileList->bShowRNew?false:true);
						MakeCmpFarList(hDlg,pFileList);
						Info.SendDlgMessage(hDlg,DM_ENABLEREDRAW,true,0);
						return true;
					}
					else if (vk==VK_SPACE && Opt.Mode==MODE_SYNC)
					{
						pFileList->Copy=0;
						int ret=0;
						struct FarDialogItem DialogItems[] = {
						//			Type	X1	Y1	X2	Y2				Selected	History	Mask	Flags	Data	MaxLen	UserParam
						/* 0*/{DI_DOUBLEBOX,  3, 1,60, 7, 0, 0, 0,                   0, GetMsg(MSyncSelTitle),0,0},
						/* 1*/{DI_TEXT,       5, 2, 0, 0, 0, 0, 0,                   0, GetMsg(MSyncSelDiff),0,0},
						/* 2*/{DI_RADIOBUTTON,5, 3, 0, 0, 1, 0, 0, DIF_FOCUS|DIF_GROUP, GetMsg(MSyncSelDiffToRight),0,0},
						/* 3*/{DI_RADIOBUTTON,34,3, 0, 0, 0, 0, 0,                   0, GetMsg(MSyncSelDiffToLeft),0,0},
						/* 4*/{DI_CHECKBOX,   5, 4, 0, 0, 0, 0, 0,                   0, GetMsg(MSyncSelDiffNew),0,0},
						/* 5*/{DI_TEXT,       0, 5, 0, 0, 0, 0, 0,       DIF_SEPARATOR, 0,0,0},
						/* 6*/{DI_BUTTON,     0, 6, 0, 0, 0, 0, 0, DIF_DEFAULTBUTTON|DIF_CENTERGROUP, GetMsg(MOK),0,0},
						/* 7*/{DI_BUTTON,     0, 6, 0, 0, 0, 0, 0,     DIF_CENTERGROUP, GetMsg(MCancel),0,0}
						};

						HANDLE hDlgCur=Info.DialogInit(&MainGuid,&DlgSyncSel,-1,-1,64,9,L"DlgCmp", DialogItems,sizeof(DialogItems)/sizeof(DialogItems[0]),0,0,0,0);
						if (hDlgCur != INVALID_HANDLE_VALUE)
						{
							ret=Info.DialogRun(hDlgCur);
							if (ret==6)
							{
								pFileList->Copy=(Info.SendDlgMessage(hDlgCur,DM_GETCHECK,2,0)?1:-1);
								pFileList->bCopyNew=Info.SendDlgMessage(hDlgCur,DM_GETCHECK,4,0);
							}
							Info.DialogFree(hDlgCur);
						}
						if (ret==6)
						{
							Info.SendDlgMessage(hDlg,DM_ENABLEREDRAW,false,0);
							MakeCmpFarList(hDlg,pFileList,true,false);
							Info.SendDlgMessage(hDlg,DM_ENABLEREDRAW,true,0);
						}
						return true;
					}
					else if (vk==VK_PRIOR)
					{
						int Pos=Info.SendDlgMessage(hDlg,DM_LISTGETCURPOS,0,0);
						cmpFile **tmp=(cmpFile **)Info.SendDlgMessage(hDlg,DM_LISTGETDATA,0,(void *)Pos);
						cmpFile *cur=tmp?*tmp:NULL;
						if (cur)
						{
							if ((LPanel.PInfo.Flags&PFLAGS_PLUGIN) || (RPanel.PInfo.Flags&PFLAGS_PLUGIN))
							{
								MessageBeep(MB_OK);
								return true;
							}

							Opt.Mode=MODE_CMP; //скидываем!!!

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

/***************************************************************************
 * ƒиалог результатов сравнени€ файлов
 ***************************************************************************/
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
	cFList.AdvCmp=this;

	HANDLE hDlg=Info.DialogInit(&MainGuid,&CmpDlgGuid,0,0,WinInfo.Con.Right,WinInfo.Con.Bottom-WinInfo.Con.Top,L"DlgCmp",DialogItems,
															sizeof(DialogItems)/sizeof(DialogItems[0]),0,FDLG_SMALLDIALOG|FDLG_KEEPCONSOLETITLE,ShowCmpDialogProc,&cFList);
	if (hDlg != INVALID_HANDLE_VALUE)
	{
		Info.DialogRun(hDlg);
		Info.DialogFree(hDlg);
	}
	if (buf) free(buf);

	if (Opt.Mode==MODE_SYNC && (Opt.Sync==QR_ALL || Opt.Sync==QR_SKIP))
		Synchronize();

	return 1;
}
