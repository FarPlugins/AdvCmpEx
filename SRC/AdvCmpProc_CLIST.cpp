/****************************************************************************
 * AdvCmpProc_CLIST.cpp
 *
 * Plugin module for Far Manager 3.0
 *
 * Copyright (c) 2006 Alexey Samlyukov
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

	cFList.F[cFList.iCount].L.Dir=(wchar_t*)malloc((wcslen(LDir)+1)*sizeof(wchar_t));
	if (cFList.F[cFList.iCount].L.Dir)
		wcscpy(cFList.F[cFList.iCount].L.Dir,LDir);
	cFList.F[cFList.iCount].R.Dir=(wchar_t*)malloc((wcslen(RDir)+1)*sizeof(wchar_t));
	if (cFList.F[cFList.iCount].R.Dir)
		wcscpy(cFList.F[cFList.iCount].R.Dir,RDir);

	if (pLPPI)
	{
		cFList.F[cFList.iCount].L.FileName=(wchar_t*)malloc((wcslen(pLPPI->FileName)+1)*sizeof(wchar_t));
		if (cFList.F[cFList.iCount].L.FileName) wcscpy(cFList.F[cFList.iCount].L.FileName,pLPPI->FileName);
		cFList.F[cFList.iCount].L.ftLastWriteTime.dwLowDateTime=pLPPI->LastWriteTime.dwLowDateTime;
		cFList.F[cFList.iCount].L.ftLastWriteTime.dwHighDateTime=pLPPI->LastWriteTime.dwHighDateTime;
		cFList.F[cFList.iCount].L.nFileSize=pLPPI->FileSize;
		cFList.F[cFList.iCount].L.dwAttributes=pLPPI->FileAttributes;
	}
	if (pRPPI)
	{
		cFList.F[cFList.iCount].R.FileName=(wchar_t*)malloc((wcslen(pRPPI->FileName)+1)*sizeof(wchar_t));
		if (cFList.F[cFList.iCount].R.FileName) wcscpy(cFList.F[cFList.iCount].R.FileName,pRPPI->FileName);
		cFList.F[cFList.iCount].R.ftLastWriteTime.dwLowDateTime=pRPPI->LastWriteTime.dwLowDateTime;
		cFList.F[cFList.iCount].R.ftLastWriteTime.dwHighDateTime=pRPPI->LastWriteTime.dwHighDateTime;
		cFList.F[cFList.iCount].R.nFileSize=pRPPI->FileSize;
		cFList.F[cFList.iCount].R.dwAttributes=pRPPI->FileAttributes;
	}
	cFList.iCount++;

	return true;
}

int WINAPI cmpSortList(const void *el1, const void *el2, void * el3)
{
	struct cmpFile *Item1=(struct cmpFile *)el1, *Item2=(struct cmpFile *)el2;

	int cmp=FSF.LStricmp(Item1->L.Dir,Item2->L.Dir);

	if (!cmp)
	{
		DWORD dwAttributes1=!(Item1->dwFlags&RCIF_RUNIQ)?Item1->L.dwAttributes:Item1->R.dwAttributes;
		DWORD dwAttributes2=!(Item2->dwFlags&RCIF_RUNIQ)?Item2->L.dwAttributes:Item2->R.dwAttributes;

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
		wchar_t *FileName1=!(Item1->dwFlags&RCIF_RUNIQ)?Item1->L.FileName:Item1->R.FileName;
		wchar_t *FileName2=!(Item2->dwFlags&RCIF_RUNIQ)?Item2->L.FileName:Item2->R.FileName;

		cmp=FSF.LStricmp(FileName1,FileName2);
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
		GetStrFileTime(&cur->L.ftLastWriteTime,LTime,false);
	if (!(cur->dwFlags&RCIF_LUNIQ))  // есть элемент справа
		GetStrFileTime(&cur->R.ftLastWriteTime,RTime,false);

	wchar_t LSize[65]={0}, RSize[65]={0};

	if (!(cur->L.dwAttributes&FILE_ATTRIBUTE_DIRECTORY) && !(cur->R.dwAttributes&FILE_ATTRIBUTE_DIRECTORY))
	{
		if (LTime[0]) cur->L.nFileSize>99999999999?FSF.itoa64(cur->L.nFileSize,LSize,10):itoaa(cur->L.nFileSize,LSize);
		if (RTime[0]) cur->R.nFileSize>99999999999?FSF.itoa64(cur->R.nFileSize,RSize,10):itoaa(cur->R.nFileSize,RSize);
	}
	else
	{
		if (LTime[0]) strcentr(LSize,GetMsg(MFolder),14,L' ');
		if (RTime[0]) strcentr(RSize,GetMsg(MFolder),14,L' ');
	}

	string strName;
	if (cur->L.dwAttributes&FILE_ATTRIBUTE_DIRECTORY || cur->R.dwAttributes&FILE_ATTRIBUTE_DIRECTORY)
	{
		if (LTime[0]) strName=GetPosToName(cur->L.Dir)+wcslen(LPanel.Dir);
		if (RTime[0]) strName=GetPosToName(cur->R.Dir)+wcslen(RPanel.Dir);
		strName+=L"\\";
		strName+=(cur->L.FileName?cur->L.FileName:cur->R.FileName);
		if (strName.length()>0 && strName[(size_t)(strName.length()-1)]!=L'\\') strName+=L"\\";
	}
	else
	{
		strName=(cur->L.FileName?cur->L.FileName:cur->R.FileName);
		if (Opt.SkipSubstr) CutSubstr(strName,Opt.Substr);
	}
	FSF.sprintf(buf, L"%*.*s%c%*.*s%c%*.*s%c%*.*s%c%c%c%s",14,14,LSize,0x2502,17,17,LTime,0x2551,17,17,RTime,0x2502,14,14,RSize,0x2551,Mark,0x2502,strName.get());
}

/***************************************************************************
 * »зменение строки статуса в листе
***************************************************************************/
void SetBottom(HANDLE hDlg, cmpFileList *pFileList, wchar_t *CurDir)
{
	static wchar_t Title[MAX_PATH];
	static wchar_t Bottom[MAX_PATH];
	FarListTitles ListTitle={sizeof(FarListTitles)};
	ListTitle.Title=Title;
	ListTitle.TitleSize=MAX_PATH;
	ListTitle.Bottom=Bottom;
	ListTitle.BottomSize=MAX_PATH;
	Info.SendDlgMessage(hDlg,DM_LISTGETTITLES,0,&ListTitle);
	if (CurDir) FSF.sprintf(Bottom,GetMsg(MListBottomCurDir),CurDir);
	else FSF.sprintf(Bottom,GetMsg(MListBottom),pFileList->Items,pFileList->Select,Opt.ShowListSelect?L' ':L'*',
								pFileList->Identical,Opt.ShowListIdentical?L' ':L'*',
								pFileList->Different,Opt.ShowListDifferent?L' ':L'*',
								pFileList->LNew,Opt.ShowListLNew?L' ':L'*',pFileList->RNew,Opt.ShowListRNew?L' ':L'*');
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
	FarListInfo ListInfo={sizeof(FarListInfo)};
	Info.SendDlgMessage(hDlg,DM_LISTINFO,0,&ListInfo);

	if (ListInfo.ItemsNumber)
		Info.SendDlgMessage(hDlg,DM_LISTDELETE,0,0);

	if (!pFileList->iCount)
		return true;

	// сортируем только при инициализации
	if (bSort)
		FSF.qsort(pFileList->F,pFileList->iCount,sizeof(pFileList->F[0]),cmpSortList,NULL);

	int Index=0;
	string strBuf;
	strBuf.get(65536);
	const int nSIZE=32; // размер €чейки дл€ FSF.sprintf(%*.*)

	for (int i=0; i<pFileList->iCount; i++)
	{
		cmpFile *cur=&pFileList->F[i];

		if (Opt.SyncFlagClearUser)
			cur->dwFlags&= ~RCIF_USER;

		wchar_t Mark=L' ';

		if ((cur->L.dwAttributes&FILE_ATTRIBUTE_DIRECTORY) && (cur->R.dwAttributes&FILE_ATTRIBUTE_DIRECTORY))
			continue;
		if ((cur->dwFlags&RCIF_USERSELECT) && !Opt.ShowListSelect)
			continue;
		if (cur->dwFlags&RCIF_EQUAL)
		{
			if (!Opt.ShowListIdentical)
				continue;
			Mark=L'='; pFileList->Identical++;
		}
		if (cur->dwFlags&RCIF_DIFFER)
		{
			if (!Opt.ShowListDifferent)
				continue;

			__int64 Delta=(((__int64)cur->L.ftLastWriteTime.dwHighDateTime << 32) | cur->L.ftLastWriteTime.dwLowDateTime) -
										(((__int64)cur->R.ftLastWriteTime.dwHighDateTime << 32) | cur->R.ftLastWriteTime.dwLowDateTime);

			if (Opt.SyncOnlyRight)
			{
				if (Opt.SyncOnlyRight==1 || (Opt.SyncOnlyRight==2 && Delta>0)) cur->dwFlags|=RCIF_USERLNEW;
			}
#if 1
			// отметка элементов
			if (Opt.SyncFlagCopy)
			{
				if ((Opt.SyncFlagCopy==2 && !Opt.SyncFlagIfNew) || (Opt.SyncFlagCopy==2 && Opt.SyncFlagIfNew && Delta>0))
				{
					cur->dwFlags|=RCIF_USERLNEW;
					cur->dwFlags&=~RCIF_USERRNEW;
				}
				else if ((Opt.SyncFlagCopy==-2 && !Opt.SyncFlagIfNew) || (Opt.SyncFlagCopy==-2 && Opt.SyncFlagIfNew && Delta<0))
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
#endif
			if (cur->dwFlags&RCIF_USERLNEW) Mark=0x25ba;
			else if (!Opt.SyncOnlyRight && (cur->dwFlags&RCIF_USERRNEW)) Mark=0x25c4;
			else Mark=0x2260;
			pFileList->Different++;
		}
		if (cur->dwFlags&RCIF_LNEW)
		{
			if (!Opt.ShowListLNew)
				continue;
#if 1
			// отметка элементов
			if (Opt.SyncFlagLCopy<=0)
			{
				if (Opt.SyncFlagLCopy<0) // юзерское
				{
					if (!(cur->dwFlags&RCIF_LUNIQ) && !Opt.SyncOnlyRight)
					{
						cur->dwFlags&=~RCIF_USERNONE;
						cur->dwFlags|=RCIF_USERRNEW;
					}
					else
					{
						cur->dwFlags|=RCIF_USERNONE; //cur->dwFlags&=~RCIF_USERNONE;
						cur->dwFlags&=~RCIF_USERRNEW;
					}
				}
				else // скинем
				{
					cur->dwFlags|=RCIF_USERNONE;
					cur->dwFlags&=~RCIF_USERRNEW;
				}
			}
			else if (Opt.SyncFlagLCopy==2) // установим обычное
			{
				cur->dwFlags&=~RCIF_USERNONE;
				cur->dwFlags&=~RCIF_USERRNEW;
			}
#endif
			if (!Opt.SyncOnlyRight && (cur->dwFlags&RCIF_USERRNEW)) Mark=0x25c4;  //направо
			else if (!(cur->dwFlags&RCIF_USERNONE)) Mark=0x2192; //как обычно, налево
			pFileList->LNew++;
		}
		if (cur->dwFlags&RCIF_RNEW)
		{
			if (!Opt.ShowListRNew)
				continue;
			if (Opt.SyncOnlyRight)
			{
				if (cur->dwFlags&RCIF_RUNIQ)
				{
					if (Opt.SyncOnlyRight==1 && !(cur->dwFlags&RCIF_USERNONE)) cur->dwFlags|=RCIF_USERDEL;
					else if (Opt.SyncOnlyRight==2) cur->dwFlags|=RCIF_USERNONE;
				}
				else
				{
					if (Opt.SyncOnlyRight==1) cur->dwFlags|=RCIF_USERLNEW;
					else cur->dwFlags|=RCIF_USERNONE;
				}
			}
#if 1
			// отметка элементов
			if (Opt.SyncFlagRCopy>=0)
			{
				if (Opt.SyncFlagRCopy>0) // юзерское
				{
					if (!(cur->dwFlags&RCIF_RUNIQ))
					{
						cur->dwFlags&=~RCIF_USERNONE;
						cur->dwFlags|=RCIF_USERLNEW;
					}
					else
					{
						cur->dwFlags|=RCIF_USERNONE;
						cur->dwFlags&=~RCIF_USERLNEW;
						cur->dwFlags&=~RCIF_USERDEL;
/*
						cur->dwFlags&=~RCIF_USERNONE;
						cur->dwFlags|=RCIF_USERDEL;
*/
					}
				}
				else // скинем
				{
					cur->dwFlags|=RCIF_USERNONE;
					cur->dwFlags&=~RCIF_USERLNEW;
					cur->dwFlags&=~RCIF_USERDEL;
				}
			}
			else if (Opt.SyncFlagRCopy==-2) // установим обычное
			{
				cur->dwFlags&=~RCIF_USERNONE;
				cur->dwFlags&=~RCIF_USERLNEW;
				if (Opt.SyncOnlyRight && cur->dwFlags&RCIF_RUNIQ) cur->dwFlags|=RCIF_USERDEL;
			}
#endif
			if (cur->dwFlags&RCIF_USERLNEW) Mark=0x25ba; //налево
			else if (!Opt.SyncOnlyRight && !(cur->dwFlags&RCIF_USERNONE)) Mark=0x2190; //как обычно, направо
			else if (cur->dwFlags&RCIF_USERDEL) Mark=L'x';
			if (!Opt.SyncOnlyRight || (Opt.SyncOnlyRight && (cur->dwFlags&RCIF_RUNIQ))) pFileList->RNew++;
		}

		// виртуальна€ папка
		bool bAddVirtDir=false;
		string strVirtDir;

		// если попали сразу в подкаталог... добавим виртуальную папку в начало
		if (!Index && FSF.LStricmp(GetPosToName(cur->L.Dir),LPanel.Dir))
		{
			strVirtDir=GetPosToName(cur->L.Dir)+wcslen(LPanel.Dir);
			if (strVirtDir.length()>0 && strVirtDir[(size_t)(strVirtDir.length()-1)]!=L'\\') strVirtDir+=L"\\";
			struct FarListItem Item={};
			Item.Flags=LIF_DISABLE|LIF_CHECKED|0x2b;
			wchar_t Size[65];
			strcentr(Size,GetMsg(MFolder),nSIZE,L' ');
			FSF.sprintf(strBuf.get(), L"%*.*s%c%*.*s%c%c%c%s",nSIZE,nSIZE,Size,0x2551,nSIZE,nSIZE,Size,0x2551,L' ',0x2502,strVirtDir.get());
			strBuf.updsize();
			Item.Text=strBuf.get();
			struct FarList List={sizeof(FarList)};
			List.ItemsNumber=1;
			List.Items=&Item;
			Info.SendDlgMessage(hDlg,DM_LISTADD,0,&List);
			Index++;
		}

		{
			for (int j=i+1; j<pFileList->iCount; j++)
			{
				cmpFile *next=&pFileList->F[j];

				if (Opt.SyncFlagClearUser)
					next->dwFlags&= ~RCIF_USER; // все равно же скидывать

				if ((next->L.dwAttributes&FILE_ATTRIBUTE_DIRECTORY) && (next->R.dwAttributes&FILE_ATTRIBUTE_DIRECTORY))
					continue;
				if ((next->dwFlags&RCIF_USERSELECT) && !Opt.ShowListSelect)
					continue;
				if ((next->dwFlags&RCIF_EQUAL) && !Opt.ShowListIdentical)
					continue;
				if ((next->dwFlags&RCIF_DIFFER) && !Opt.ShowListDifferent)
					continue;
				if ((next->dwFlags&RCIF_LNEW) && !Opt.ShowListLNew)
					continue;
				if ((next->dwFlags&RCIF_RNEW) && !Opt.ShowListRNew)
					continue;

				if (FSF.LStricmp(cur->L.Dir,next->L.Dir))
				{
					bAddVirtDir=true;
					strVirtDir=GetPosToName(next->L.Dir)+wcslen(LPanel.Dir);
					if (strVirtDir.length()>0 && strVirtDir[(size_t)(strVirtDir.length()-1)]!=L'\\') strVirtDir+=L"\\";
				}
				break;
			}
		}

		MakeListItemText(strBuf.get(),cur,Mark);
		strBuf.updsize();

		struct FarListItem Item={};
		if (cur->dwFlags&RCIF_EQUAL)
			Item.Flags|=LIF_GRAYED;
		if (cur->dwFlags&RCIF_USERSELECT)
		{
			Item.Flags|=LIF_CHECKED;
			pFileList->Select++;
		}
		Item.Text=strBuf.get();
		struct FarList List={sizeof(FarList)};
		List.ItemsNumber=1;
		List.Items=&Item;

		// если удачно добавили элемент...
		if (Info.SendDlgMessage(hDlg,DM_LISTADD,0,&List))
		{
			pFileList->Items++;
			// ... то ассоциируем данные с элементом листа
			struct FarListItemData Data={sizeof(FarListItemData)};
			Data.Index=Index++;
			Data.DataSize=sizeof(cur);
			Data.Data=&cur;
			Info.SendDlgMessage(hDlg,DM_LISTSETDATA,0,&Data);
		}

		if (bAddVirtDir)
		{
			wchar_t Size[65];
			strcentr(Size,GetMsg(MFolder),nSIZE,L' ');
			FSF.sprintf(strBuf.get(), L"%*.*s%c%*.*s%c%c%c%s",nSIZE,nSIZE,Size,0x2551,nSIZE,nSIZE,Size,0x2551,L' ',0x2502,strVirtDir.get());
			strBuf.updsize();
			Item.Flags=LIF_DISABLE|LIF_CHECKED|0x2b;
			Item.Text=strBuf.get();
			List.Items=&Item;
			Info.SendDlgMessage(hDlg,DM_LISTADD,0,&List);
			Index++;
		}
	}

	SetBottom(hDlg,pFileList);

	if (bSetCurPos)
	{
		FarListPos ListPos={sizeof(FarListPos)};
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
intptr_t WINAPI ShowCmpDialogProc(HANDLE hDlg,intptr_t Msg,intptr_t Param1,void *Param2)
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
						FarListPos ListPos={sizeof(FarListPos)};
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
									(cur->L.dwAttributes&FILE_ATTRIBUTE_DIRECTORY) || (cur->R.dwAttributes&FILE_ATTRIBUTE_DIRECTORY) ||
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

							if (!pFileList->AdvCmp->CompareCurFile(cur->L.Dir,cur->L.FileName,cur->R.Dir,cur->R.FileName,0))
								MessageBeep(MB_ICONASTERISK);
						}
						return true;
					}
					else if (vk==VK_INSERT)
					{
						struct FarListPos FLP={sizeof(FarListPos)};
						Info.SendDlgMessage(hDlg,DM_LISTGETCURPOS,0,&FLP);
						cmpFile **tmp=(cmpFile **)Info.SendDlgMessage(hDlg,DM_LISTGETDATA,0,(void *)FLP.SelectPos);
						cmpFile *cur=tmp?*tmp:NULL;
						if (cur && Opt.ShowListSelect)
						{
							struct FarListGetItem FLGI={sizeof(FarListGetItem)};
							FLGI.ItemIndex=FLP.SelectPos;
							if (Info.SendDlgMessage(hDlg,DM_LISTGETITEM,0,&FLGI))
							{
								(FLGI.Item.Flags&LIF_CHECKED)?(FLGI.Item.Flags&= ~LIF_CHECKED):(FLGI.Item.Flags|=LIF_CHECKED);
								struct FarListUpdate FLU={sizeof(FarListUpdate)};
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
							struct FarListGetItem FLGI={sizeof(FarListGetItem)};
							FLGI.ItemIndex=Pos;
							if (Info.SendDlgMessage(hDlg,DM_LISTGETITEM,0,&FLGI))
							{
								__int64 Delta=0;
								if (!(cur->dwFlags&RCIF_LUNIQ) && !(cur->dwFlags&RCIF_RUNIQ))
									Delta=(((__int64)cur->L.ftLastWriteTime.dwHighDateTime << 32) | cur->L.ftLastWriteTime.dwLowDateTime) -
												(((__int64)cur->R.ftLastWriteTime.dwHighDateTime << 32) | cur->R.ftLastWriteTime.dwLowDateTime);

								if (cur->dwFlags&RCIF_DIFFER) // сравнивали без учета времени элементов
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
								else if (!(cur->dwFlags&RCIF_LUNIQ) && !(cur->dwFlags&RCIF_RUNIQ))  // сравнивали с учетом времени элементов
								{
									if (!(cur->dwFlags&RCIF_USERNONE) && !(cur->dwFlags&RCIF_USERLNEW) && !(cur->dwFlags&RCIF_USERRNEW))
									{
										cur->dwFlags|=RCIF_USERNONE;
//										cur->dwFlags&=~RCIF_USERLNEW;
//										cur->dwFlags&=~RCIF_USERRNEW;
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
//											cur->dwFlags&=~RCIF_USERNONE;
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

								string strBuf;
								strBuf.get(65536);
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
								MakeListItemText(strBuf.get(),cur,Mark);
								strBuf.updsize();
								struct FarListUpdate FLU={sizeof(FarListUpdate)};
								FLU.Index=FLGI.ItemIndex;
								FLU.Item=FLGI.Item;
								FLU.Item.Text=strBuf.get();
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
									(cur->L.dwAttributes&FILE_ATTRIBUTE_DIRECTORY) || (cur->R.dwAttributes&FILE_ATTRIBUTE_DIRECTORY) ||
									(cur->dwFlags&RCIF_LUNIQ) || (cur->dwFlags&RCIF_RUNIQ))
							{
								MessageBeep(MB_OK);
								return true;
							}
							if (!pFileList->AdvCmp->CompareCurFile(cur->L.Dir,cur->L.FileName,cur->R.Dir,cur->R.FileName,1))
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
							string strVirtDir=GetPosToName(cur->L.Dir)+wcslen(LPanel.Dir);
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
						Opt.ShowListSelect=true;
						Opt.ShowListIdentical=true;
						Opt.ShowListDifferent=true;
						Opt.ShowListLNew=true;
						Opt.ShowListRNew=true;
						Opt.SyncFlagClearUser=true;
						MakeCmpFarList(hDlg,pFileList);
						Opt.SyncFlagClearUser=false; // восстановим!
						Info.SendDlgMessage(hDlg,DM_ENABLEREDRAW,true,0);
						return true;
					}
					else if (vk==VK_OEM_5 && !(Opt.ShowListSelect && pFileList->Select==0))
					{
						Info.SendDlgMessage(hDlg,DM_ENABLEREDRAW,false,0);
						Opt.ShowListSelect=(Opt.ShowListSelect?false:true);
						MakeCmpFarList(hDlg,pFileList);
						Info.SendDlgMessage(hDlg,DM_ENABLEREDRAW,true,0);
						return true;
					}
					else if (vk==VK_OEM_PLUS && !(Opt.ShowListIdentical && pFileList->Identical==0))
					{
						Info.SendDlgMessage(hDlg,DM_ENABLEREDRAW,false,0);
						Opt.ShowListIdentical=(Opt.ShowListIdentical?false:true);
						MakeCmpFarList(hDlg,pFileList);
						Info.SendDlgMessage(hDlg,DM_ENABLEREDRAW,true,0);
						return true;
					}
					else if (vk==VK_OEM_MINUS && !(Opt.ShowListDifferent && pFileList->Different==0))
					{
						Info.SendDlgMessage(hDlg,DM_ENABLEREDRAW,false,0);
						Opt.ShowListDifferent=(Opt.ShowListDifferent?false:true);
						MakeCmpFarList(hDlg,pFileList);
						Info.SendDlgMessage(hDlg,DM_ENABLEREDRAW,true,0);
						return true;
					}
					else if (vk==VK_OEM_4 && !(Opt.ShowListLNew && pFileList->LNew==0))
					{
						Info.SendDlgMessage(hDlg,DM_ENABLEREDRAW,false,0);
						Opt.ShowListLNew=(Opt.ShowListLNew?false:true);
						MakeCmpFarList(hDlg,pFileList);
						Info.SendDlgMessage(hDlg,DM_ENABLEREDRAW,true,0);
						return true;
					}
					else if (vk==VK_OEM_6 && !(Opt.ShowListRNew && pFileList->RNew==0))
					{
						Info.SendDlgMessage(hDlg,DM_ENABLEREDRAW,false,0);
						Opt.ShowListRNew=(Opt.ShowListRNew?false:true);
						MakeCmpFarList(hDlg,pFileList);
						Info.SendDlgMessage(hDlg,DM_ENABLEREDRAW,true,0);
						return true;
					}
					else if (vk==VK_SPACE && Opt.Mode==MODE_SYNC)
					{
						int ret=0;
						struct FarDialogItem DialogItems[] = {
						//			Type	X1	Y1	X2	Y2				Selected	History	Mask	Flags	Data	MaxLen	UserParam
						/* 0*/{DI_DOUBLEBOX,  3, 1,60,11, 0, 0, 0,                   0, GetMsg(MSyncSelTitle),0,0},
						/* 1*/{DI_TEXT,       5, 2, 0, 0, 0, 0, 0,       DIF_SEPARATOR, GetMsg(MSyncSelDiff),0,0},
						/* 2*/{DI_RADIOBUTTON,5, 3, 0, 0,(Opt.SyncOnlyRight?0:1),0,0,DIF_FOCUS|DIF_GROUP, GetMsg(MSyncSelToSkip),0,0},
						/* 3*/{DI_RADIOBUTTON,22,3, 0, 0,(Opt.SyncOnlyRight?1:0),0,0,0, GetMsg(MSyncSelToRight),0,0},
						/* 4*/{DI_RADIOBUTTON,39,3, 0, 0, 0, 0, 0,                   0, GetMsg(MSyncSelToLeft),0,0},
						/* 5*/{DI_CHECKBOX,   5, 4, 0, 0, 0, 0, 0,                   0, GetMsg(MSyncSelIfNew),0,0},
						/* 6*/{DI_TEXT,       5, 5, 0, 0, 0, 0, 0,       DIF_SEPARATOR, GetMsg(MSyncSelLNew),0,0},
						/* 7*/{DI_RADIOBUTTON,5, 6, 0, 0, 1, 0, 0,           DIF_GROUP, GetMsg(MSyncSelToRight),0,0},
						/* 8*/{DI_RADIOBUTTON,22,6, 0, 0, 0, 0, 0,                   0, GetMsg(MSyncSelToLeft),0,0},
						/* 9*/{DI_RADIOBUTTON,39,6, 0, 0, 0, 0, 0,                   0, GetMsg(MSyncSelToNon),0,0},
						/*10*/{DI_TEXT,       5, 7, 0, 0, 0, 0, 0,       DIF_SEPARATOR, GetMsg(MSyncSelRNew),0,0},
						/*11*/{DI_RADIOBUTTON,5, 8, 0, 0,(Opt.SyncOnlyRight!=2?1:0),0,0,DIF_GROUP,GetMsg(Opt.SyncOnlyRight?MSyncSelToDel:MSyncSelToLeft),0,0},
						/*12*/{DI_RADIOBUTTON,22,8, 0, 0, 0, 0, 0,                   0, GetMsg(MSyncSelToRight),0,0},
						/*13*/{DI_RADIOBUTTON,39,8, 0, 0,(Opt.SyncOnlyRight!=2?0:1),0,0,0,GetMsg(MSyncSelToNon),0,0},
						/*14*/{DI_TEXT,      -1, 9, 0, 0, 0, 0, 0,       DIF_SEPARATOR, 0,0,0},
						/*15*/{DI_BUTTON,     0,10, 0, 0, 0, 0, 0, DIF_DEFAULTBUTTON|DIF_CENTERGROUP, GetMsg(MOK),0,0},
						/*16*/{DI_BUTTON,     0,10, 0, 0, 0, 0, 0,     DIF_CENTERGROUP, GetMsg(MCancel),0,0}
						};

						HANDLE hDlgCur=Info.DialogInit(&MainGuid,&DlgSyncSel,-1,-1,64,13,L"DlgCmp", DialogItems,sizeof(DialogItems)/sizeof(DialogItems[0]),0,0,0,0);
						if (hDlgCur != INVALID_HANDLE_VALUE)
						{
							ret=(int)Info.DialogRun(hDlgCur);
							if (ret==15)
							{
								Opt.SyncFlagCopy=(Info.SendDlgMessage(hDlgCur,DM_GETCHECK,2,0)?1:(Info.SendDlgMessage(hDlgCur,DM_GETCHECK,3,0)?2:-2));
								Opt.SyncFlagIfNew=Info.SendDlgMessage(hDlgCur,DM_GETCHECK,5,0);
								Opt.SyncFlagLCopy=(Info.SendDlgMessage(hDlgCur,DM_GETCHECK,7,0)?2:(Info.SendDlgMessage(hDlgCur,DM_GETCHECK,8,0)?-2:0));
								Opt.SyncFlagRCopy=(Info.SendDlgMessage(hDlgCur,DM_GETCHECK,11,0)?-2:(Info.SendDlgMessage(hDlgCur,DM_GETCHECK,12,0)?2:0));
							}
							Info.DialogFree(hDlgCur);
						}
						if (ret==15)
						{
							Info.SendDlgMessage(hDlg,DM_ENABLEREDRAW,false,0);
							MakeCmpFarList(hDlg,pFileList,true,false);
							Info.SendDlgMessage(hDlg,DM_ENABLEREDRAW,true,0);
							Opt.SyncFlagCopy=Opt.SyncFlagIfNew=0; Opt.SyncFlagLCopy=1; Opt.SyncFlagRCopy=-1;
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

							PanelRedrawInfo LRInfo={sizeof(PanelRedrawInfo),0,0}, RRInfo={sizeof(PanelRedrawInfo),0,0};
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
							if (FSF.LStricmp(LPanel.Dir,GetPosToName(cur->L.Dir)))
							{
								FarPanelDirectory dirInfo={sizeof(FarPanelDirectory),GetPosToName(cur->L.Dir),NULL,{0},NULL};
								bSetLDir=Info.PanelControl(LPanel.hPanel,FCTL_SETPANELDIRECTORY,0,&dirInfo);
								Info.PanelControl(LPanel.hPanel,FCTL_BEGINSELECTION,0,0);
							}

							{
								PanelInfo PInfo={sizeof(PanelInfo)};
								Info.PanelControl(LPanel.hPanel,FCTL_GETPANELINFO,0,&PInfo);

								for (unsigned i=0; i<PInfo.ItemsNumber; i++)
								{
									size_t size=Info.PanelControl(LPanel.hPanel,FCTL_GETPANELITEM,i,0);
									PluginPanelItem *PPI=(PluginPanelItem*)malloc(size);
									if (PPI)
									{
										FarGetPluginPanelItem FGPPI={sizeof(FarGetPluginPanelItem),size,PPI};
										Info.PanelControl(LPanel.hPanel,FCTL_GETPANELITEM,i,&FGPPI);
										if (!bSetLFile && cur->L.FileName && !FSF.LStricmp(cur->L.FileName,FGPPI.Item->FileName))
										{
											LRInfo.CurrentItem=i;
											bSetLFile=true;
										}
										// если изменили каталог, то отметим все отличи€ на новой напанели
										if (bSetLDir)
										{
											for (int j=0; j<pFileList->iCount; j++)
											{
												if (FSF.LStricmp(pFileList->F[j].L.Dir,cur->L.Dir))
													continue;
												if (pFileList->F[j].L.FileName && !FSF.LStricmp(pFileList->F[j].L.FileName,FGPPI.Item->FileName))
												{
													Info.PanelControl(LPanel.hPanel,FCTL_SETSELECTION,i,(void*)((pFileList->F[j].dwFlags&RCIF_DIFFER) || (pFileList->F[j].dwFlags&RCIF_LNEW) || (pFileList->F[j].dwFlags&RCIF_RNEW)));
													break;
												}
											}
										}
										free(PPI);
										if (!bSetLDir && bSetLFile)
											break;
									}
								}
								if (bSetLDir)
									Info.PanelControl(LPanel.hPanel,FCTL_ENDSELECTION,0,0);
							}

							if (FSF.LStricmp(RPanel.Dir,GetPosToName(cur->R.Dir)))
							{
								FarPanelDirectory dirInfo={sizeof(FarPanelDirectory),GetPosToName(cur->R.Dir),NULL,{0},NULL};
								bSetRDir=Info.PanelControl(RPanel.hPanel,FCTL_SETPANELDIRECTORY,0,&dirInfo);
								Info.PanelControl(RPanel.hPanel,FCTL_BEGINSELECTION,0,0);
							}
							{
								PanelInfo PInfo={sizeof(PanelInfo)};
								Info.PanelControl(RPanel.hPanel,FCTL_GETPANELINFO,0,&PInfo);

								for (unsigned i=0; i<PInfo.ItemsNumber; i++)
								{
									size_t size=Info.PanelControl(RPanel.hPanel,FCTL_GETPANELITEM,i,0);
									PluginPanelItem *PPI=(PluginPanelItem*)malloc(size);
									if (PPI)
									{
										FarGetPluginPanelItem FGPPI={sizeof(FarGetPluginPanelItem),size,PPI};
										Info.PanelControl(RPanel.hPanel,FCTL_GETPANELITEM,i,&FGPPI);
										if (!bSetRFile && cur->R.FileName && !FSF.LStricmp(cur->R.FileName,FGPPI.Item->FileName))
										{
											RRInfo.CurrentItem=i;
											bSetRFile=true;
										}
										if (bSetRDir)
										{
											for (int j=0; j<pFileList->iCount; j++)
											{
												if (FSF.LStricmp(pFileList->F[j].R.Dir,cur->R.Dir))
													continue;
												if (pFileList->F[j].R.FileName && !FSF.LStricmp(pFileList->F[j].R.FileName,FGPPI.Item->FileName))
												{
													Info.PanelControl(RPanel.hPanel,FCTL_SETSELECTION,i,(void*)((pFileList->F[j].dwFlags&RCIF_DIFFER) || (pFileList->F[j].dwFlags&RCIF_LNEW) || (pFileList->F[j].dwFlags&RCIF_RNEW)));
													break;
												}
											}
										}
										free(PPI);
										if (!bSetRDir && bSetRFile)
											break;
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
