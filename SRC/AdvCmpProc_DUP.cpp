/****************************************************************************
 * AdvCmpProc_DUP.cpp
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
 *                              ДУБЛИКАТЫ
 *
 ***************************************************************************/


/****************************************************************************
 * Построение массива элементов, для диалога дубликатов
 ****************************************************************************/
bool AdvCmpProc::MakeFileList(const wchar_t *Dir,const PluginPanelItem *pPPI)
{
	for (int i=0; i<dFList.iCount; i++)
	{
		if (!FSF.LStricmp(dFList.F[i].Dir,Dir))
			if (!FSF.LStricmp(dFList.F[i].FileName,pPPI->FileName))
				return true;
	}

	dupFile *New=(dupFile*)realloc(dFList.F,(dFList.iCount+1)*sizeof(dupFile));
	if (!New)
	{
		ErrorMsg(MNoMemTitle, MNoMemBody);
		return false;
	}
	dFList.F=New;

	dFList.F[dFList.iCount].FileName=(wchar_t*)malloc((wcslen(pPPI->FileName)+1)*sizeof(wchar_t));
	if (dFList.F[dFList.iCount].FileName) wcscpy(dFList.F[dFList.iCount].FileName,pPPI->FileName);
	dFList.F[dFList.iCount].Dir=(wchar_t*)malloc((wcslen(Dir)+1)*sizeof(wchar_t));
	if (dFList.F[dFList.iCount].Dir) wcscpy(dFList.F[dFList.iCount].Dir,Dir);
	dFList.F[dFList.iCount].ftLastWriteTime.dwLowDateTime=pPPI->LastWriteTime.dwLowDateTime;
	dFList.F[dFList.iCount].ftLastWriteTime.dwHighDateTime=pPPI->LastWriteTime.dwHighDateTime;
	dFList.F[dFList.iCount].nFileSize=pPPI->FileSize;
	dFList.F[dFList.iCount].dwAttributes=pPPI->FileAttributes;

	dFList.iCount++;
	return true;
}

/***************************************************************************
 * Узнаем опции синхронизации
 ***************************************************************************/
int GetDupOpt(dupFileList *pFileList)
{
	int ret=QR_EDIT; // продолжаем редактировать список
	int ItemsDel=0;

	for (int i=0; i<pFileList->iCount; i++)
	{
		dupFile *cur=&pFileList->F[i];

		if (cur->dwFlags&RCIF_USERDEL)
			ItemsDel++;
	}
/*
	// нет элементов, выходим
	if (!ItemsDel)
	{
		if (Opt.ShowMsg)
		{
			const wchar_t *MsgItems[] = { GetMsg(MDupTitle), GetMsg(MNoDupBody), GetMsg(MOK) };
			Info.Message(&MainGuid,&NoDupMsgGuid,0,0,MsgItems,sizeof(MsgItems) / sizeof(MsgItems[0]),1);
		}
		return ret=QR_SKIP; //нет элементов
	}
*/
	Opt.DupDel=ItemsDel;
	Opt.DupDelRecycleBin=(Opt.DupDel && GetFarSetting(FSSF_SYSTEM,L"DeleteToRecycleBin"));

	wchar_t buf[80];
	FSF.sprintf(buf,GetMsg(MDupDel),ItemsDel);

	struct FarDialogItem DialogItems[] = {
		//			Type	X1	Y1	X2	Y2		              Selected	History	Mask	Flags	Data	MaxLen	UserParam
		/* 0*/{DI_DOUBLEBOX,  3, 1,60, 6,                    0, 0, 0,             0, GetMsg(MDupTitle),0,0},
		/* 1*/{DI_CHECKBOX,   5, 2, 0, 0,           Opt.DupDel, 0, 0,Opt.DupDel?DIF_FOCUS:DIF_DISABLE,buf,0,0},
		/* 2*/{DI_CHECKBOX,   8, 3, 0, 0, Opt.DupDelRecycleBin, 0, 0, Opt.DupDel?0:DIF_DISABLE,GetMsg(MDupDelRecycleBin),0,0},
		/* 3*/{DI_TEXT,      -1, 4, 0, 0,                    0, 0, 0, DIF_SEPARATOR, L"",0,0},
		/* 4*/{DI_BUTTON,     0, 5, 0, 0,                    0, 0, 0, DIF_DEFAULTBUTTON|DIF_CENTERGROUP, GetMsg(MOK),0,0},
		/* 5*/{DI_BUTTON,     0, 5, 0, 0,                    0, 0, 0, DIF_CENTERGROUP, GetMsg(MDupEdit),0,0},
		/* 6*/{DI_BUTTON,     0, 5, 0, 0,                    0, 0, 0, DIF_CENTERGROUP, GetMsg(MCancel),0,0}
	};

	HANDLE hDlg=Info.DialogInit(&MainGuid, &OptSyncDlgGuid,-1,-1,64,8,L"DlgDup",DialogItems,sizeof(DialogItems)/sizeof(DialogItems[0]),0,0,0,0);

	if (hDlg != INVALID_HANDLE_VALUE)
	{
		ret=Info.DialogRun(hDlg);
		if (ret==4)
		{
			Opt.DupDel=Info.SendDlgMessage(hDlg,DM_GETCHECK,1,0);
			Opt.DupDelRecycleBin=Info.SendDlgMessage(hDlg,DM_GETCHECK,2,0);
			ret=Opt.DupDel?QR_ALL:QR_SKIP;   // удаляем дубли, иначе - пропустим
		}
		else if (ret==5)
			ret=QR_EDIT;
		else
		{
			bBrokenByEsc=true;
			ret=QR_ABORT; // отменили
		}
		Info.DialogFree(hDlg);
	}
	else
		ret=QR_ABORT;

	return ret;
}


int __cdecl dupSortListByName(const void *el1, const void *el2)
{
	struct dupFile *Item1=(struct dupFile *)el1, *Item2=(struct dupFile *)el2;
	return FSF.LStricmp(Item1->FileName,Item2->FileName);
}

int __cdecl dupSortListByGroup(const void *el1, const void *el2)
{
	struct dupFile *Item1=(struct dupFile *)el1, *Item2=(struct dupFile *)el2;
	return Item2->nGroup-Item1->nGroup;
}

int __cdecl dupSortListByGroupEx(const void *el1, const void *el2)
{
	struct dupFile *Item1=(struct dupFile *)el1, *Item2=(struct dupFile *)el2;
	int cmp=Item1->nGroup-Item2->nGroup;
	if (!cmp)
		cmp=FSF.LStricmp(Item1->FileName,Item2->FileName);
	if (!cmp)
		cmp=FSF.LStricmp(Item1->Dir,Item2->Dir);
	return cmp;
}

/***************************************************************************
 * Изменение строки статуса в листе
***************************************************************************/

void SetBottom(HANDLE hDlg, dupFileList *pFileList, dupFile *curItem)
{
	if (curItem)
	{
		static wchar_t Title[MAX_PATH];
		static wchar_t Bottom[MAX_PATH];
		FarListTitles ListTitle;
		ListTitle.Title=Title;
		ListTitle.TitleSize=MAX_PATH;
		ListTitle.Bottom=Bottom;
		ListTitle.BottomSize=MAX_PATH;
		Info.SendDlgMessage(hDlg,DM_LISTGETTITLES,0,&ListTitle);

		string strDir(GetPosToName(curItem->Dir));
		FSF.TruncPathStr(strDir.get(),WinInfo.TruncLen);
		strDir.updsize();
		wcscpy(Title,strDir.get());

		wchar_t Time[20];
		wchar_t Size[65];
		GetStrFileTime(&curItem->ftLastWriteTime,Time,false);
		itoaa(curItem->nFileSize,Size);
		FSF.sprintf(Bottom,GetMsg(MDupListBottom),14,14,Size,Time,pFileList->GroupCount,pFileList->iCount,pFileList->Del);

		Info.SendDlgMessage(hDlg,DM_LISTSETTITLES,0,&ListTitle);
	}
}

/***************************************************************************
 * Изменение/обновление листа файлов в диалоге
 ***************************************************************************/
bool MakeDupFarList(HANDLE hDlg, dupFileList *pFileList, bool bSetCurPos, bool bSort)
{
	// запросим информацию
	FarListInfo ListInfo;
	Info.SendDlgMessage(hDlg,DM_LISTINFO,0,&ListInfo);

	if (ListInfo.ItemsNumber)
		Info.SendDlgMessage(hDlg,DM_LISTDELETE,0,0);

	if (!pFileList->iCount)
		return true;

	// сортируем только при инициализации
	if (bSort)
		FSF.qsort(pFileList->F,pFileList->iCount,sizeof(pFileList->F[0]),dupSortListByGroupEx);

	int Index=0;
	string strBuf;
	strBuf.get(32768);
	wchar_t Size[10], Time[20], w[64], h[64];
	wchar_t MetaData[160];

	int digits_count=1, m=10;
	while (m <= pFileList->GroupCount)
	{
		m *= 10;
		digits_count++;
	}

	for (int i=0; i<pFileList->iCount; i++)
	{
		dupFile *cur=&pFileList->F[i];

		FSF.FormatFileSize(cur->nFileSize,7,FFFS_FLOATSIZE|FFFS_SHOWBYTESINDEX|FFFS_ECONOMIC,Size,8);

		if (Opt.DupListSmall)
		{
			FSF.sprintf(strBuf.get(), L"%*d%c%-7.7s%c%s",digits_count,cur->nGroup,0x2502,Size,0x2551,cur->FileName);
		}
		else
		{
			string strPath(GetPosToName(cur->Dir));

			if ((cur->dwFlags&RCIF_PIC) || (cur->dwFlags&RCIF_PICERR && cur->PicWidth && cur->PicHeight))
			{
				FSF.itoa(cur->PicWidth,w,10);
				FSF.itoa(cur->PicHeight,h,10);
				FSF.TruncPathStr(strPath.get(),88);
				strPath.updsize();
				FSF.sprintf(MetaData,L"%-88.88s%c%5.5s x %-5.5s",strPath.get(),0x2551,w,h);
			}
			else if (cur->dwFlags&RCIF_MUSIC)
			{
				FSF.TruncPathStr(strPath.get(),30);
				strPath.updsize();
				FSF.sprintf(MetaData,L"%-30.30s%c%-30.30s %-30.30s %02u:%02u %03u",strPath.get(),0x2551,(cur->dwFlags&RCIF_MUSICART)?cur->MusicArtist:L"",(cur->dwFlags&RCIF_MUSICTIT)?cur->MusicTitle:L"",cur->MusicTime/60,cur->MusicTime%60,cur->MusicBitrate);
			}
			else
			{
				GetStrFileTime(&cur->ftLastWriteTime,Time,true);
				FSF.TruncPathStr(strPath.get(),82);
				strPath.updsize();
				FSF.sprintf(MetaData,L"%-82.82s%c%-19.19s",strPath.get(),0x2551,Time);
			}
			FSF.sprintf(strBuf.get(), L"%*d%c%-102.102s%c%-7.7s%c%s",digits_count,cur->nGroup,0x2502,MetaData,0x2551,Size,0x2551,cur->FileName);
		}
		strBuf.updsize();

		struct FarListItem Item;
		Item.Flags=0;
		if (cur->dwFlags&RCIF_USERDEL)
		{
			Item.Flags|=(LIF_CHECKED|LIF_GRAYED);
			pFileList->Del++;
		}
		Item.Text=strBuf.get();
		Item.Reserved[0]=Item.Reserved[1]=Item.Reserved[2]=0;
		struct FarList List;
		List.ItemsNumber=1;
		List.Items=&Item;

		// если удачно добавили элемент...
		if (Info.SendDlgMessage(hDlg,DM_LISTADD,0,&List))
		{
			// ... то ассоциируем данные с элементом листа
			struct FarListItemData Data;
			Data.Index=Index++;
			Data.DataSize=sizeof(cur);
			Data.Data=&cur;
			Info.SendDlgMessage(hDlg,DM_LISTSETDATA,0,&Data);
		}

		// сепаратор
		int j=i+1;
		if (j<pFileList->iCount && cur->nGroup!=pFileList->F[j].nGroup)
		{
			Item.Flags=LIF_SEPARATOR;
			Item.Text=NULL;
			List.Items=&Item;
			Info.SendDlgMessage(hDlg,DM_LISTADD,0,&List);
			Index++;
		}
	}

	SetBottom(hDlg,pFileList,pFileList->iCount?&pFileList->F[0]:NULL);

	if (bSetCurPos)
	{
		FarListPos ListPos;
		ListPos.SelectPos=ListInfo.SelectPos;
		ListPos.TopPos=ListInfo.TopPos;
		Info.SendDlgMessage(hDlg,DM_LISTSETCURPOS,0,&ListPos);
	}
	return true;
}


/***************************************************************************
 * Обработчик диалога
 ***************************************************************************/
INT_PTR WINAPI ShowDupDialogProc(HANDLE hDlg,int Msg,int Param1,void *Param2)
{
	dupFileList *pFileList=(dupFileList *)Info.SendDlgMessage(hDlg,DM_GETDLGDATA,0,0);
	switch(Msg)
	{
		case DN_INITDIALOG:
			MakeDupFarList(hDlg,pFileList,true,true);
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
			int Pos=Info.SendDlgMessage(hDlg,DM_LISTGETCURPOS,0,0);
			dupFile **tmp=(dupFile **)Info.SendDlgMessage(hDlg,DM_LISTGETDATA,0,(void *)Pos);
			dupFile *cur=tmp?*tmp:NULL;
			if (cur) SetBottom(hDlg,pFileList,cur);
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
				}
				return true;
			}
			break;

	/************************************************************************/

		case DN_LISTCHANGE:
			if (Param1==0)
			{
				dupFile **tmp=(dupFile **)Info.SendDlgMessage(hDlg,DM_LISTGETDATA,0,(void *)Param2);
				dupFile *cur=tmp?*tmp:NULL;
				if (cur)
				{
					SetBottom(hDlg,pFileList,cur);
					return true;
				}
			}
			break;

	/************************************************************************/

		case DN_CLOSE:
			if (Opt.Mode==MODE_DUP && Param1==-1)
			{
				Opt.Dup=GetDupOpt(pFileList);
				if (Opt.Dup==QR_EDIT)
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
				if (Param1==0 && record->Event.MouseEvent.dwButtonState==RIGHTMOST_BUTTON_PRESSED)
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
						if (NewPos==ListPos.SelectPos)
						{
							dupFile **tmp=(dupFile **)Info.SendDlgMessage(hDlg,DM_LISTGETDATA,0,(void *)ListPos.SelectPos);
							dupFile *cur=(tmp && *tmp)?*tmp:NULL;
							if (cur)
							{
								struct FarListGetItem FLGI;
								FLGI.ItemIndex=ListPos.SelectPos;
								if (Info.SendDlgMessage(hDlg,DM_LISTGETITEM,0,&FLGI))
								{
									(FLGI.Item.Flags&(LIF_CHECKED|LIF_GRAYED))?(FLGI.Item.Flags&= ~(LIF_CHECKED|LIF_GRAYED)):(FLGI.Item.Flags|=(LIF_CHECKED|LIF_GRAYED));
									struct FarListUpdate FLU;
									FLU.Index=FLGI.ItemIndex;
									FLU.Item=FLGI.Item;
									if (Info.SendDlgMessage(hDlg,DM_LISTUPDATE,0,&FLU))
									{
										if (cur->dwFlags&RCIF_USERDEL)
										{
											cur->dwFlags&=~RCIF_USERDEL;
											pFileList->Del--;
										}
										else
										{
											cur->dwFlags|=RCIF_USERDEL;
											pFileList->Del++;
										}
										SetBottom(hDlg,pFileList,cur);
										return false;
									}
								}
							}
						}
					}
				}
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
/*
					{
GOTOCMPFILE:
						int Pos=Info.SendDlgMessage(hDlg,DM_LISTGETCURPOS,0,0);
						cmpFile **tmp=(cmpFile **)Info.SendDlgMessage(hDlg,DM_LISTGETDATA,0,(void *)Pos);
						cmpFile *cur=(tmp && *tmp)?*tmp:NULL;
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
					else 
*/
					if (vk==VK_F3)
					{
						int Pos=Info.SendDlgMessage(hDlg,DM_LISTGETCURPOS,0,0);
						dupFile **tmp=(dupFile **)Info.SendDlgMessage(hDlg,DM_LISTGETDATA,0,(void *)Pos);
						dupFile *cur=tmp?*tmp:NULL;
						if (cur)
						{
							string strFullFileName;
							GetFullFileName(strFullFileName,cur->Dir,cur->FileName);
							if (Info.Viewer(GetPosToName(strFullFileName.get()),NULL,0,0,-1,-1,VF_DISABLEHISTORY,CP_AUTODETECT))
							{
								SetBottom(hDlg,pFileList,cur); // обходим баг фара: почему-то строка функциональных клавиш не прячется автоматом!
								return true;
							}
						}
						else
							MessageBeep(MB_OK);
						return true;
					}
					else if (vk==VK_INSERT)
					{
						struct FarListPos FLP;
						Info.SendDlgMessage(hDlg,DM_LISTGETCURPOS,0,&FLP);
						dupFile **tmp=(dupFile **)Info.SendDlgMessage(hDlg,DM_LISTGETDATA,0,(void *)FLP.SelectPos);
						dupFile *cur=tmp?*tmp:NULL;
						if (cur)
						{
							struct FarListGetItem FLGI;
							FLGI.ItemIndex=FLP.SelectPos;
							if (Info.SendDlgMessage(hDlg,DM_LISTGETITEM,0,&FLGI))
							{
								(FLGI.Item.Flags&(LIF_CHECKED|LIF_GRAYED))?(FLGI.Item.Flags&= ~(LIF_CHECKED|LIF_GRAYED)):(FLGI.Item.Flags|=(LIF_CHECKED|LIF_GRAYED));
								struct FarListUpdate FLU;
								FLU.Index=FLGI.ItemIndex;
								FLU.Item=FLGI.Item;
								if (Info.SendDlgMessage(hDlg,DM_LISTUPDATE,0,&FLU))
								{
									if (cur->dwFlags&RCIF_USERDEL)
									{
										cur->dwFlags&=~RCIF_USERDEL;
										pFileList->Del--;
									}
									else
									{
										cur->dwFlags|=RCIF_USERDEL;
										pFileList->Del++;
									}
									FLP.SelectPos++;
									if ((FLP.SelectPos-1)==Info.SendDlgMessage(hDlg,DM_LISTSETCURPOS,0,&FLP))
										SetBottom(hDlg,pFileList,cur);

									return true;
								}
							}
						}
						MessageBeep(MB_OK);
						return true;
					}
/*
					else if (vk==VK_SPACE)
					{
GOTOCHANGEMARK:
						int Pos=Info.SendDlgMessage(hDlg,DM_LISTGETCURPOS,0,0);
						cmpFile **tmp=(cmpFile **)Info.SendDlgMessage(hDlg,DM_LISTGETDATA,0,(void *)Pos);
						cmpFile *cur=(tmp && *tmp)?*tmp:NULL;
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
*/
				}
				else if (IsCtrl(record))
				{
					if (vk==0x31) //VK_1
					{
						Info.SendDlgMessage(hDlg,DM_ENABLEREDRAW,false,0);
						Opt.DupListSmall ? Opt.DupListSmall=0 : Opt.DupListSmall=1;
						MakeDupFarList(hDlg,pFileList,true,false);
						Info.SendDlgMessage(hDlg,DM_ENABLEREDRAW,true,0);
						return true;
					}
					else if (vk==VK_PRIOR)
					{
						int Pos=Info.SendDlgMessage(hDlg,DM_LISTGETCURPOS,0,0);
						dupFile **tmp=(dupFile **)Info.SendDlgMessage(hDlg,DM_LISTGETDATA,0,(void *)Pos);
						dupFile *cur=tmp?*tmp:NULL;
						if (cur)
						{
							Opt.Mode=MODE_CMP; //скидываем!!!

							PanelRedrawInfo RInfo={0,0};
							bool bLeft=(LPanel.PInfo.Flags&PFLAGS_FOCUS?true:false);
							if (FSF.LStricmp(bLeft?LPanel.Dir:RPanel.Dir,GetPosToName(cur->Dir)))
							{
								FarPanelDirectory dirInfo={sizeof(FarPanelDirectory),GetPosToName(cur->Dir),NULL,{0},NULL};
								Info.PanelControl(bLeft?LPanel.hPanel:RPanel.hPanel,FCTL_SETPANELDIRECTORY,0,&dirInfo);
							}
							PanelInfo PInfo;
							PInfo.StructSize=sizeof(PanelInfo);
							Info.PanelControl(bLeft?LPanel.hPanel:RPanel.hPanel,FCTL_GETPANELINFO,0,&PInfo);
							FarGetPluginPanelItem FGPPI;

							for (unsigned i=0; i<PInfo.ItemsNumber; i++)
							{
								FGPPI.Size=0; FGPPI.Item=0;
								FGPPI.Item=(PluginPanelItem*)malloc(FGPPI.Size=Info.PanelControl(bLeft?LPanel.hPanel:RPanel.hPanel,FCTL_GETPANELITEM,i,&FGPPI));
								if (FGPPI.Item)
								{
									Info.PanelControl(bLeft?LPanel.hPanel:RPanel.hPanel,FCTL_GETPANELITEM,i,&FGPPI);
									if (!FSF.LStricmp(cur->FileName,FGPPI.Item->FileName))
									{
										RInfo.CurrentItem=i;
										free(FGPPI.Item);
										break;
									}
									free(FGPPI.Item);
								}
							}
							Info.PanelControl(bLeft?LPanel.hPanel:RPanel.hPanel,FCTL_REDRAWPANEL,0,&RInfo);
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
 * Диалог результатов сравнения файлов
 ***************************************************************************/
int AdvCmpProc::ShowDupDialog()
{
	FarDialogItem DialogItems[1];
	memset(DialogItems,0,sizeof(DialogItems));

	DialogItems[0].Type=DI_LISTBOX;
	DialogItems[0].X1=0; DialogItems[0].X2=WinInfo.Con.Right;
	DialogItems[0].Y1=0; DialogItems[0].Y2=WinInfo.Con.Bottom-WinInfo.Con.Top;
	DialogItems[0].Flags=DIF_LISTNOCLOSE;

	HANDLE hDlg=Info.DialogInit(&MainGuid,&DupDlgGuid,0,0,WinInfo.Con.Right,WinInfo.Con.Bottom-WinInfo.Con.Top,L"DlgDup",DialogItems,
															sizeof(DialogItems)/sizeof(DialogItems[0]),0,FDLG_SMALLDIALOG|FDLG_KEEPCONSOLETITLE,ShowDupDialogProc,&dFList);
	if (hDlg != INVALID_HANDLE_VALUE)
	{
		FarSettingsCreate settings={sizeof(FarSettingsCreate),MainGuid,INVALID_HANDLE_VALUE};
		Opt.DupListSmall=0;
		if (Info.SettingsControl(INVALID_HANDLE_VALUE,SCTL_CREATE,0,&settings))
		{
			int Root=0; // корень ключа
			FarSettingsItem item={Root,L"DupListSmall",FST_QWORD};
			if (Info.SettingsControl(settings.Handle,SCTL_GET,0,&item))
				Opt.DupListSmall=(int)item.Number;
			Info.SettingsControl(settings.Handle,SCTL_FREE,0,0);
		}

		Info.DialogRun(hDlg);

		if (Info.SettingsControl(INVALID_HANDLE_VALUE,SCTL_CREATE,0,&settings))
		{
			int Root=0; // корень ключа
			FarSettingsItem item={Root,L"DupListSmall",FST_QWORD};
			item.Number=Opt.DupListSmall;
			Info.SettingsControl(settings.Handle,SCTL_SET,0,&item);
			Info.SettingsControl(settings.Handle,SCTL_FREE,0,0);
		}
		Info.DialogFree(hDlg);
	}

	return 1;
}


int AdvCmpProc::ScanDir(const wchar_t *DirName, int ScanDepth)
{
	bool ret=true;

	if (bBrokenByEsc)
		return ret;

	WIN32_FIND_DATA wfdFindData;
	HANDLE hFind;
	string strDirMask(DirName);
	strDirMask+=L"\\*";

	if ((hFind=FindFirstFileW(strDirMask,&wfdFindData))!= INVALID_HANDLE_VALUE)
	{
		do
		{
			if (CheckForEsc())
			{
				ret=true;
				break;
			}

			if (!Opt.ProcessHidden && (wfdFindData.dwFileAttributes&FILE_ATTRIBUTE_HIDDEN))
				continue;
			if ((wfdFindData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) &&
						((wfdFindData.cFileName[0]==L'.' && !wfdFindData.cFileName[1]) || (wfdFindData.cFileName[0]==L'.' && wfdFindData.cFileName[1]==L'.' && !wfdFindData.cFileName[2])))
				continue;

			if (wfdFindData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
			{
				if (Opt.Subfolders==2 && Opt.MaxScanDepth<ScanDepth+1) // не глубже заданного уровня!
					break;
				if (!Opt.Subfolders)
					continue;
				GetFullFileName(strDirMask,DirName,wfdFindData.cFileName);
				ret=ScanDir(strDirMask,ScanDepth+1);
			}
			else
			{
				PluginPanelItem ppi;
				memset(&ppi,0,sizeof(ppi));
				ppi.FileAttributes=wfdFindData.dwFileAttributes;
				ppi.LastAccessTime=wfdFindData.ftLastAccessTime;
				ppi.LastWriteTime=wfdFindData.ftLastWriteTime;
				ppi.CreationTime=wfdFindData.ftCreationTime;
				ppi.FileSize=((unsigned __int64)wfdFindData.nFileSizeHigh << 32) | wfdFindData.nFileSizeLow;
				ppi.FileName=wfdFindData.cFileName;

				if (Opt.Filter && !Info.FileFilterControl(Opt.hCustomFilter,FFCTL_ISFILEINFILTER,0,&ppi))
					continue;

				CmpInfo.Count+=1;
				CmpInfo.CountSize+=ppi.FileSize;
				ShowDupMsg((LPanel.PInfo.Flags&PFLAGS_FOCUS?LPanel.Dir:RPanel.Dir),L"*",false);
				ret=MakeFileList(DirName,&ppi);
			}
		}
		while (ret && FindNextFile(hFind,&wfdFindData));
		FindClose(hFind);
	}
	else
		CmpInfo.Errors++;

	return ret;
}

DWORD AdvCmpProc::GetCRC(const dupFile *cur)
{
	string strFullFileName;
	GetFullFileName(strFullFileName,cur->Dir,cur->FileName);

	HANDLE hFile;
	if ((hFile=CreateFileW(strFullFileName, GENERIC_READ, FILE_SHARE_READ, 0,
                           OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, 0)) == INVALID_HANDLE_VALUE)
	{
		CmpInfo.ProcSize+=cur->nFileSize;
		CmpInfo.Errors++;
		return 0;
	}

	ShowDupMsg(GetPosToName(cur->Dir),cur->FileName,true);

	const DWORD ReadBlock=65536;
	char *Buf=(char*)malloc(ReadBlock*sizeof(char));
	DWORD ReadSize=0;
	bool ret=true;

	DWORD dwFileCRC=0;

	while (1)
	{
		if (CheckForEsc() || !ReadFile(hFile,Buf,ReadBlock,&ReadSize,0))
		{
			ret=false;
			break;
		}
		CmpInfo.CurProcSize+=ReadSize;
		CmpInfo.ProcSize+=ReadSize;
		dwFileCRC=ProcessCRC(Buf,ReadBlock,dwFileCRC);

		ShowDupMsg(GetPosToName(cur->Dir),cur->FileName,false);

		if (ReadSize<ReadBlock)
			break;
	}

	free(Buf);
	CloseHandle(hFile);

	if (!ret)
		return 0;

	return dwFileCRC;
}

unsigned int GetSyncSafeInt(unsigned char *b)
{
	return (b[0]<<21) | (b[1]<<14) | (b[2]<<7) | (b[3]);
}

unsigned int GetNonSyncSafeInt23(unsigned char *b)
{
	return (b[0]<<24) | (b[1]<<16) | (b[2]<<8) | (b[3]);
}

unsigned int GetNonSyncSafeInt22(unsigned char *b)
{
	return (b[0]<<16) | (b[1]<<8) | (b[2]);
}

bool ID3_ReadFrameText(char bType, char *pTextRaw, int nTextRawSize, wchar_t *pszBuf, int nBufSize)
{
	if ((bType == 3) || (bType != 1 && bType != 2 && nTextRawSize>= 3 && pTextRaw[0] == 0xef && pTextRaw[1] == 0xbb && pTextRaw[2] == 0xbf))
	{
		/* UTF8 */
		if (nTextRawSize>= 3 && pTextRaw[0] == 0xef && pTextRaw[1] == 0xbb && pTextRaw[2] == 0xbf)
		{
			pTextRaw += 3;
			nTextRawSize -= 3;
		}
		MultiByteToWideChar(CP_UTF8, 0, pTextRaw, nTextRawSize, pszBuf, nBufSize);
	}
	else if (bType == 1 || bType == 2)
	{
		/* UTF16 LE*/
		if (nTextRawSize>= 2 && pTextRaw[0] == 0xff && pTextRaw[1] == 0xfe)
		{
			pTextRaw += 2;
			nTextRawSize -= 2;
			bType = 1;
		} 
		/* UTF16 BE*/
		else if (nTextRawSize>= 2 && pTextRaw[0] == 0xfe && pTextRaw[1] == 0xff)
		{
			pTextRaw += 2;
			nTextRawSize -= 2;
			bType = 2;
		}
		if (bType == 2)
		{
			char *pa = (char *)malloc(nTextRawSize);
			if (pa)
			{
				int l = nTextRawSize >> 1;
				for (int i=0; i<l; i++)
				{
					pa[i * 2 + 0] = pTextRaw[i * 2 + 1];
					pa[i * 2 + 1] = pTextRaw[i * 2 + 0];
				}
				ID3_ReadFrameText(1, pa, nTextRawSize, pszBuf, nBufSize);
				free(pa);
				return true;
			}
			return false;
		}
#define XMIN(x,y) ((x>y)?y:x)
		memcpy(pszBuf, pTextRaw, XMIN((nTextRawSize >> 1), nBufSize) * sizeof(wchar_t));
	}
	else
	{
		MultiByteToWideChar(CP_ACP, 0, pTextRaw, nTextRawSize, pszBuf, nBufSize);
	}
	FSF.RTrim(pszBuf);
	return true;
}

char *Unsync(char *pSrc, unsigned nSrcLen, unsigned nDstLmt, unsigned *pnDstLen)
{
	char *pUnsync=(char *)malloc(nSrcLen?nSrcLen:nDstLmt);
	if (pUnsync)
	{
		unsigned i=0, j=0;
		for ( ; (nSrcLen?i<nSrcLen:j<nDstLmt); i++)
		{
			char b=pSrc[i];
			pUnsync[j++]=b; 
			if (b==0xff && pSrc[i+1] == 0) i++;
		}
		*pnDstLen=j;
	}
	return pUnsync;
}

bool ID3V23_ReadFrame(int nFlag, char *pRaw, unsigned nFrameSize, wchar_t *pszBuf, int nBufSize)
{
	bool bRet=false;
	unsigned nRawSize=nFrameSize;
	if (nFlag&1)
	{
		nRawSize -= 4;
		pRaw += 4;
	}
	if (nFlag&2)
	{
		unsigned nUnsynced;
		char *pUnsync=Unsync(pRaw+1, nRawSize-1, 0, &nUnsynced);
		if (pUnsync)
		{
			bRet=ID3_ReadFrameText(pRaw[0], pUnsync, nUnsynced, pszBuf, nBufSize);
			free(pUnsync);
		}
	}
	else
	{
		bRet=ID3_ReadFrameText(pRaw[0], pRaw+1, nRawSize-1, pszBuf, nBufSize);
	}
	return bRet;
}


int AdvCmpProc::GetMp3(dupFile *cur)
{
	int ret=0;

	if (cur->dwFlags&RCIF_MUSIC)
		return 1;

	if (CheckForEsc())
		return ret;

	HSTREAM stream=NULL;

	if (FSF.ProcessName(L"*.mp3",cur->FileName,0,PN_CMPNAME))
	{
		string strFullFileName;
		GetFullFileName(strFullFileName,cur->Dir,cur->FileName);
		BASS_CHANNELINFO info;
		if (stream=pBASS_StreamCreateFile(FALSE,strFullFileName.get(),0,0,BASS_STREAM_DECODE|BASS_UNICODE))
			if (pBASS_ChannelGetInfo(stream,&info))
				if (info.ctype==BASS_CTYPE_STREAM_MP3 || info.ctype==BASS_CTYPE_STREAM_MP2)
					cur->dwFlags|=RCIF_MUSIC;
	}

	if (!(cur->dwFlags&RCIF_MUSIC))
	{
		ret=0;
		goto END;
	}

	const unsigned int nSize=256;
	cur->MusicArtist=(wchar_t*)malloc(nSize*sizeof(wchar_t));
	cur->MusicTitle=(wchar_t*)malloc(nSize*sizeof(wchar_t));

	if (!cur->MusicArtist || !cur->MusicTitle)
	{
		ret=0;
		goto END;
	}

	ShowDupMsg(GetPosToName(cur->Dir),cur->FileName,true);

	cur->MusicBitrate=(DWORD)(pBASS_StreamGetFilePosition(stream,BASS_FILEPOS_END)/
														(125*pBASS_ChannelBytes2Seconds(stream,pBASS_ChannelGetLength(stream,BASS_POS_BYTE)))+0.5); // bitrate (Kbps)
	cur->MusicTime=(DWORD)pBASS_ChannelBytes2Seconds(stream,pBASS_ChannelGetLength(stream,BASS_POS_BYTE));

	char *p=(char *)pBASS_ChannelGetTags(stream,BASS_TAG_ID3V2);
	if (p && p[0]=='I' && p[1]=='D' && p[2]=='3')
	{
		unsigned nVersion = ((*(p + 3)) << 8) | (*(p + 4));
		unsigned nFlag = *(p + 5);
		unsigned nTagSize = GetSyncSafeInt((unsigned char*)p + 6);
		char szFrameID[5];
		unsigned nFrameSize;
		unsigned nTotal=0;
		char *pUnsync=NULL;

		if ((nFlag&0x80) && (nVersion<=0x300))
			p = pUnsync = Unsync(p+10,0,nTagSize,&nTagSize);
		else
			p += 10;
		if (p)
		{
			if (nVersion>=0x300)
			{
				if (nFlag & 0x40)
					nTotal += (nVersion==0x300) ? (4+GetNonSyncSafeInt23((unsigned char*)p+nTotal)) : GetSyncSafeInt((unsigned char*)p+nTotal);

				while(nTotal<nTagSize)
				{
					int nFrameFlag=0;
					// Ver.2.3
					lstrcpynA(szFrameID,(p+nTotal),5);
					if (nVersion== 0x300)
						nFrameSize=GetNonSyncSafeInt23((unsigned char*)p+nTotal+4);
					else
						nFrameSize=GetSyncSafeInt((unsigned char*)p+nTotal+4);
					int nLenID=lstrlenA(szFrameID);
					if (nLenID>0 && nLenID<4 && nTotal>4)
					{
						/* broken tag */
						lstrcpynA(szFrameID,(p+nTotal-4+nLenID),5);
						if (lstrlenA(szFrameID)==4)
						{
							nTotal += -4+nLenID;
							continue;
						}
					}
					if (nLenID!=4)
						break;
					if (nVersion!=0x300)
						nFrameFlag = (p[nTotal+9] & 3) | ((nFlag & 0x80) ? 2 : 0);

					if(!lstrcmpA(szFrameID, "TPE1"))
						ID3V23_ReadFrame(nFrameFlag, p+nTotal+10,nFrameSize,cur->MusicArtist,nSize);
					else if(!lstrcmpA(szFrameID, "TIT2"))
						ID3V23_ReadFrame(nFrameFlag, p+nTotal+10,nFrameSize,cur->MusicTitle,nSize);
					nTotal += nFrameSize+10;
				}
			}
			else
			{
				while(nTotal<nTagSize)
				{
					int nFrameFlag=0;
					// Ver.2.2
					lstrcpynA(szFrameID,(p+nTotal),4);
					nFrameSize=GetNonSyncSafeInt22((unsigned char*)p+nTotal+3);
					if (lstrlenA(szFrameID)!=3)
						break;

					if(!lstrcmpA(szFrameID, "TP1"))
						ID3V23_ReadFrame(nFrameFlag, p+nTotal+6,nFrameSize,cur->MusicArtist,nSize);
					else if(!lstrcmpA(szFrameID, "TT2"))
						ID3V23_ReadFrame(nFrameFlag, p+nTotal+6, nFrameSize,cur->MusicTitle,nSize);
					nTotal += nFrameSize+6;
				}
			}
		}
		if (pUnsync) free(pUnsync);
	}
	if (*cur->MusicArtist)
	{
//DebugMsg(L"cur->MusicArtist-2",cur->MusicArtist);
		ret=1;
		cur->dwFlags|=RCIF_MUSICART;
	}
	if (*cur->MusicTitle)
	{
//DebugMsg(L"cur->MusicTitle-2",cur->MusicTitle);
		ret=1;
		cur->dwFlags|=RCIF_MUSICTIT;
	}

	TAG_ID3 *id3=(TAG_ID3*)pBASS_ChannelGetTags(stream,BASS_TAG_ID3);
	if (id3)
	{
		int nFrameSize=30;
		if (!(cur->dwFlags&RCIF_MUSICART))
		{
			while (nFrameSize>0 && id3->artist[nFrameSize-1] == 0x20) nFrameSize--;
			MultiByteToWideChar(CP_ACP,0,id3->artist,nFrameSize,cur->MusicArtist,nSize);
			FSF.RTrim(cur->MusicArtist);
			if (*cur->MusicArtist)
			{
//DebugMsg(L"cur->MusicArtist-1",cur->MusicArtist);
				ret=1;
				cur->dwFlags|=RCIF_MUSICART;
			}
		}
		nFrameSize=30;
		if (!(cur->dwFlags&RCIF_MUSICTIT))
		{
			while (nFrameSize>0 && id3->title[nFrameSize-1] == 0x20) nFrameSize--;
			MultiByteToWideChar(CP_ACP,0,id3->title,nFrameSize,cur->MusicTitle,nSize);
			FSF.RTrim(cur->MusicTitle);
			if (*cur->MusicTitle)
			{
//DebugMsg(L"cur->MusicTitle-1",cur->MusicTitle);
				ret=1;
				cur->dwFlags|=RCIF_MUSICTIT;
			}
		}
	}

	if (!(cur->dwFlags&RCIF_MUSICART) || !(cur->dwFlags&RCIF_MUSICTIT))
	{
		wchar_t *Name=cur->FileName;
		int lenName=wcslen(Name)-4; //за минусом расширения
		int Ptr=lenName;
		for (int i=0; i<lenName; i++)
		{
			if (!Strncmp(Name+i, L" - ", 3) || !Strncmp(Name+i, L"_-_", 3))
			{
				if (i>0 && FSF.LIsAlphanum((wchar_t)Name[i-1]))
				{
					Ptr=i;
					break;
				}
			}
		}

		if (!(cur->dwFlags&RCIF_MUSICART) && Ptr!=lenName)
		{
			bool bNum=true;
			for (int i=0,j=0; Name[j] && j<Ptr && i<nSize; j++)
			{
				if (bNum)
				{
					if (Name[j]>=L'0' && Name[j]<=L'9')
						continue;
					else
						bNum=false;
				}
				cur->MusicArtist[i++]=Name[j];
			}
			//!!! память обнулена - cur->MusicArtist[i]=0 не делаем !!!
			FSF.RTrim(cur->MusicArtist);
			FSF.LTrim(cur->MusicArtist);
			if (*cur->MusicArtist)
			{
//DebugMsg(L"cur->MusicArtist-0",cur->MusicArtist);
				ret=1;
			}
		}

		if (!(cur->dwFlags&RCIF_MUSICTIT))
		{
			if (Ptr!=lenName)
			{
				for (int i=0,j=Ptr+3; j<lenName && i<nSize; i++,j++)
					cur->MusicTitle[i]=Name[j];
			}
			else
			{
				for (int i=0; Name[i] && i<lenName && i<nSize; i++)
					cur->MusicTitle[i]=Name[i];
			}
			//!!! память обнулена, cur->MusicTitle[i]=0 не делаем !!!
			FSF.RTrim(cur->MusicTitle);
			FSF.LTrim(cur->MusicTitle);
			if (*cur->MusicTitle)
			{
//DebugMsg(L"cur->MusicTitle-0",cur->MusicTitle);
				ret=1;
			}
		}
	}

END:
	if (stream) pBASS_StreamFree(stream);

	return ret;
}


int AdvCmpProc::GetPic(dupFile *cur)
{
	int ret=0;

	if (cur->dwFlags&RCIF_PIC)
		return 1;

	if (CheckForEsc())
		return ret;

	if (FSF.ProcessName(L"*.jpg,*.jpeg,*.jpe,*.jfif,*.jiff,*.jif,*.j,*.jng,*.jff",cur->FileName,0,PN_CMPNAMELIST))
		cur->dwFlags|=(RCIF_PICJPG|RCIF_PIC);
	else if (FSF.ProcessName(L"*.bmp,*.dib,*.rle",cur->FileName,0,PN_CMPNAMELIST))
		cur->dwFlags|=(RCIF_PICBMP|RCIF_PIC);
	else if (FSF.ProcessName(L"*.gif",cur->FileName,0,PN_CMPNAME))
		cur->dwFlags|=(RCIF_PICGIF|RCIF_PIC);
	else if (FSF.ProcessName(L"*.png",cur->FileName,0,PN_CMPNAME))
		cur->dwFlags|=(RCIF_PICPNG|RCIF_PIC);
	else if (FSF.ProcessName(L"*.tif,*.tiff",cur->FileName,0,PN_CMPNAMELIST))
		cur->dwFlags|=(RCIF_PICTIF|RCIF_PIC);
	else if (FSF.ProcessName(L"*.ico,*.icon,*.icn",cur->FileName,0,PN_CMPNAMELIST))
		cur->dwFlags|=(RCIF_PICICO|RCIF_PIC);
	else
		return ret;

	ShowDupMsg(GetPosToName(cur->Dir),cur->FileName,true);

	string strFullFileName;
	GetFullFileName(strFullFileName,cur->Dir,cur->FileName);

	GFL_BITMAP *pBitmap=NULL;
	GFL_LOAD_PARAMS load_params;

	pGflGetDefaultLoadParams(&load_params);
	load_params.Flags|=GFL_LOAD_SKIP_ALPHA;
	load_params.Origin=GFL_BOTTOM_LEFT;
	load_params.LinePadding=4;

	GFL_ERROR res=pGflLoadBitmapW(strFullFileName.get(),&pBitmap,&load_params,NULL);
	if (res)
	{
		cur->dwFlags|=RCIF_PICERR;
		pBitmap=NULL;
		load_params.Flags|=GFL_LOAD_IGNORE_READ_ERROR;
		res=pGflLoadBitmapW(strFullFileName.get(),&pBitmap,&load_params,NULL);
		if (res)
		{
			pBitmap=NULL;
			CmpInfo.Errors++;
			return ret;
		}
	}

	if (pBitmap)
	{
		if (pBitmap->Width<=0 || pBitmap->Height<=0)
		{
			cur->dwFlags|=RCIF_PICERR;
			pGflFreeBitmap(pBitmap);
			CmpInfo.Errors++;
			return ret;
		}
		cur->PicWidth=pBitmap->Width;
		cur->PicHeight=pBitmap->Height;
		cur->PicRatio=(cur->PicWidth>cur->PicHeight?(cur->PicHeight*PIXELS_SIZE/cur->PicWidth-PIXELS_SIZE):(cur->PicWidth*PIXELS_SIZE/cur->PicHeight-PIXELS_SIZE));
		cur->PicPix=(unsigned char*)malloc(PIXELS_SIZE*PIXELS_SIZE*sizeof(unsigned char));
		if (cur->PicPix)
		{
			for (int i_x=0; i_x<PIXELS_SIZE; i_x++)
			{
				int b_x=i_x*pBitmap->Width/PIXELS_SIZE;
				for (int i_y=0; i_y<PIXELS_SIZE; i_y++)
				{
					int b_y=i_y*pBitmap->Height/PIXELS_SIZE;
					GFL_COLOR Color;
					if (!pGflGetColorAt(pBitmap,b_x,b_y,&Color))
					{
						float fGray=(Color.Red+Color.Green+Color.Blue)/3+0.5;
						cur->PicPix[i_x*PIXELS_SIZE + i_y]=(unsigned char)fGray;
					}
				}
			}
		}
		else
		{
			cur->dwFlags|=RCIF_PICERR;
			pGflFreeBitmap(pBitmap);
			CmpInfo.Errors++;
			return ret;
		}
		pGflFreeBitmap(pBitmap);
		ret=1;
	}

	return ret;
}

bool CmpNameEx(const wchar_t *FileName1, const wchar_t *FileName2)
{
	bool ret=false;
	// определим указатель на расширение файла
	wchar_t *p1=(wchar_t *)FileName1, *p2=(wchar_t *)FileName2;
	while (*p1++) ;
	wchar_t *pEnd1=p1-1;
	while (--p1 != FileName1 && *p1 != L'.') ;
	while (*p2++) ;
	wchar_t *pEnd2=p2-1;
	while (--p2 != FileName2 && *p2 != L'.') ;

	int lenName1=(*p1== L'.'?p1-FileName1:pEnd1-FileName1);
	int lenName2=(*p2== L'.'?p2-FileName2:pEnd2-FileName2);
	// если есть только расширение
	if (!lenName1 && !lenName2) return true;
	if (!lenName1 || !lenName2) return false;
	// если нет расширения
	if (*p1 != L'.') p1=pEnd1;
	if (*p2 != L'.') p2=pEnd2;

	wchar_t *Name1=(wchar_t *)malloc((lenName1+1)*sizeof(wchar_t));
	wchar_t *Name2=(wchar_t *)malloc((lenName2+1)*sizeof(wchar_t));

	if (Name1 && Name2)
	{
		wchar_t *s1=(wchar_t *)FileName1, *s2=(wchar_t *)FileName2;
		wchar_t *ss1=Name1, *ss2=Name2;
		while (s1!=p1)
		{
			if (FSF.LIsAlpha(*s1)) *ss1++ = *s1++;
			else s1++;
		}
		while (s2!=p2)
		{
			if (FSF.LIsAlpha(*s2)) *ss2++ = *s2++;
			else s2++;
		}
		unsigned Len1=ss1-Name1;
		unsigned Len2=ss2-Name2;
		if (Len1<=Len2 && Len1>(Len2>>1))
		{
			for (unsigned l=0;l<Len2;l++)
			{
				if (!FSF.LStrnicmp(Name2+l,Name1,Len1))
				{
					ret=true;
					break;
				}
			}
		}
	}
	if (Name1) free(Name1);
	if (Name2) free(Name2);

	return ret;
}

int AdvCmpProc::Duplicate(const struct DirList *pList)
{
	int ret=0;

	if (!Opt.DupName && !Opt.DupSize && !Opt.DupContents)
		goto END;

	bBrokenByEsc=false;
	bStartMsg=true;
	bAskLReadOnly=bAskRReadOnly=GetFarSetting(FSSF_CONFIRMATIONS,L"RO")?true:false;
	bAskDel=GetFarSetting(FSSF_CONFIRMATIONS,L"Delete")?true:false;
	bSkipLReadOnly=bSkipRReadOnly=false;
	DWORD dwTicks=GetTickCount();

	if (Opt.ProcessSelected)
	{
		for (int i=0; i<pList->ItemsNumber; i++)
		{
			if (pList->PPI[i].Flags&PPIF_SELECTED)
			{
				if (pList->PPI[i].FileAttributes&FILE_ATTRIBUTE_DIRECTORY)
				{
					string strDir;
					GetFullFileName(strDir,pList->Dir,pList->PPI[i].FileName);
					ret=ScanDir(strDir.get(),0);
				}
				else
				{
					if (Opt.Filter && !Info.FileFilterControl(Opt.hCustomFilter,FFCTL_ISFILEINFILTER,0,&pList->PPI[i]))
						continue;

					CmpInfo.Count+=1;
					CmpInfo.CountSize+=pList->PPI[i].FileSize;
					ShowDupMsg((LPanel.PInfo.Flags&PFLAGS_FOCUS?LPanel.Dir:RPanel.Dir),L"*",false);
					ret=MakeFileList(pList->Dir,&pList->PPI[i]);
				}
				if (!ret) break;
			}
		}
	}
	else
	{
		if (!(ret=ScanDir(pList->Dir,0)))
			goto END;

		wchar_t **ListPath=NULL;
		int NumPath=GetArgv(Opt.DupPath, &ListPath);
		if (NumPath)
		{
			for (int i=0; i<NumPath; i++)
			{
				if (FSF.LStricmp(GetPosToName(pList->Dir),GetPosToName(ListPath[i])))
				{
					int size=FSF.ConvertPath(CPM_NATIVE,ListPath[i],0,0);
					wchar_t *Dir=(wchar_t*)malloc(size*sizeof(wchar_t));
					if (Dir)
					{
						FSF.ConvertPath(CPM_NATIVE,ListPath[i],Dir,size);
						if (GetFileAttributesW(Dir)!=INVALID_FILE_ATTRIBUTES)
							ret=ScanDir(Dir,0);
						free(Dir);
					}
				}
				if (!ret) break;
			}
		}
		for (int i=0; i<NumPath; i++)
			free(ListPath[i]);
		free(ListPath);
	}

	if (!ret)
		goto END;

	FSF.qsort(dFList.F,dFList.iCount,sizeof(dFList.F[0]),dupSortListByName);

	// ищем дубли, первый проход
	if (Opt.DupName || Opt.DupSize || (Opt.DupContents && !Opt.DupPic && !Opt.DupMusic))
	{
		for (int i=0, IndexGroup=1; i<dFList.iCount && !bBrokenByEsc; i++)
		{
			ShowDupMsg(GetPosToName(pList->Dir),L"*",false);
			if (CheckForEsc())
				break;
			bool bSetGroupCount=false;

			for (int j=0; j<dFList.iCount && !bBrokenByEsc; j++)
			{
				if (j==i)
					continue;
				dupFile *src=&dFList.F[i];
				dupFile *cur=&dFList.F[j];
				if (!cur->nGroup)
				{
					if (Opt.DupName)
					{
						if (Opt.DupName==2)
						{
							if (!CmpNameEx(src->FileName,cur->FileName))
								continue;   // разные, сразу переходим к следующему
						}
						else
						{
							if (FSF.LStricmp(src->FileName,cur->FileName))
								continue;   // разные, сразу переходим к следующему
						}
					}
					if (Opt.DupSize || (Opt.DupContents && !Opt.DupPic && !Opt.DupMusic))
					{
						if (src->nFileSize!=cur->nFileSize)
							continue;   // разные, сразу переходим к следующему
					}
					bSetGroupCount=true;
					src->nGroup=IndexGroup;
					src->dwFlags|=RCIF_EQUAL;
					cur->nGroup=IndexGroup;
					cur->dwFlags|=RCIF_EQUAL;
				}
			}
			if (bSetGroupCount) // есть группа
			{
				dFList.GroupCount=IndexGroup;
				IndexGroup++;
			}
			if (!Opt.DupContents)  // там свой индикатор!
			{
				CmpInfo.ProcSize+=dFList.F[i].nFileSize;
				CmpInfo.Proc=i+1;
			}
		}
	}

	if (Opt.DupContents)
	{
		// собираем инф-цию
		for (int i=0; i<dFList.iCount && !bBrokenByEsc; i++)
		{
			if (CheckForEsc())
				break;
			dupFile *cur=&dFList.F[i];
			cur->nGroup=0; // скинем, проверять будем флаг RCIF_EQUAL

			ShowDupMsg(GetPosToName(cur->Dir),cur->FileName,false);

			if ((Opt.DupName || Opt.DupSize || (Opt.DupContents && !Opt.DupPic && !Opt.DupMusic)) && !(cur->dwFlags&RCIF_EQUAL))
			{
				CmpInfo.ProcSize+=cur->nFileSize;
				CmpInfo.Proc=i+1;
				continue;   // обрабатывали, переходим к следующему
			}
			if (cur->nFileSize==0)
			{
				CmpInfo.Proc=i+1;
				continue;   // пустой, переходим к следующему
			}
			if (Opt.DupPic && bGflLoaded && GetPic(cur))
			{
				CmpInfo.ProcSize+=cur->nFileSize;
				CmpInfo.Proc=i+1;
				continue;   // обработали, переходим к следующему
			}
			if (Opt.DupMusic && bBASSLoaded && GetMp3(cur))
			{
				CmpInfo.ProcSize+=cur->nFileSize;
				CmpInfo.Proc=i+1;
				continue;   // обработали, переходим к следующему
			}
			if (!cur->dwCRC)
			{
				cur->dwCRC=GetCRC(cur);
				CmpInfo.Proc=i+1;
				continue;   // обработали, переходим к следующему
			}
			CmpInfo.ProcSize+=cur->nFileSize;
			CmpInfo.Proc=i+1;
		}

		// обрабатываем, ищем дубли
		for (int i=0, IndexGroup=1; i<dFList.iCount && !bBrokenByEsc; i++)
		{
			if (CheckForEsc())
				break;
			bool bSetGroupCount=false;

			for (int j=0; j<dFList.iCount && !bBrokenByEsc; j++)
			{
				if (j==i)
					continue;
				dupFile *src=&dFList.F[i];
				dupFile *cur=&dFList.F[j];

				ShowDupMsg(GetPosToName(src->Dir),src->FileName,false);

				if (!cur->nGroup)
				{
					if ((Opt.DupName || Opt.DupSize || (Opt.DupContents && !Opt.DupPic && !Opt.DupMusic)) && !(cur->dwFlags&RCIF_EQUAL))
						continue;   // обрабатывали -они разные, переходим к следующему

					if (Opt.DupPic && (src->dwFlags&RCIF_PIC) && (cur->dwFlags&RCIF_PIC))
					{
						if (Opt.DupPicSize)
						{
							if (src->PicWidth!=cur->PicWidth || src->PicHeight!=cur->PicHeight)
								continue;   // разные, сразу переходим к следующему
						}
						if (Opt.DupPicFmt)
						{
							if (!((src->dwFlags&RCIF_PICJPG && cur->dwFlags&RCIF_PICJPG) ||
										(src->dwFlags&RCIF_PICBMP && cur->dwFlags&RCIF_PICBMP) ||
										(src->dwFlags&RCIF_PICGIF && cur->dwFlags&RCIF_PICGIF) ||
										(src->dwFlags&RCIF_PICPNG && cur->dwFlags&RCIF_PICPNG) ||
										(src->dwFlags&RCIF_PICTIF && cur->dwFlags&RCIF_PICTIF) ||
										(src->dwFlags&RCIF_PICTIF && cur->dwFlags&RCIF_PICICO)) )
							{
								continue;
							}
						}
						// сравнивалка картинок
						// Yermalayeu Ihar, Minsk, Belarus, 2002-2009
						{
							int nCmpDiff   = Opt.DupPicDiff*8;
							int nMaxFastSum = PIXELS_SIZE*nCmpDiff*nCmpDiff;
							int nMaxSlowSum = PIXELS_SIZE*PIXELS_SIZE*nCmpDiff*nCmpDiff;
							int nRatioDiff  = cur->PicRatio-src->PicRatio;
							int nMaxRatDiff = Opt.DupPicDiff;

							if (nRatioDiff>nMaxRatDiff || nRatioDiff<-nMaxRatDiff)
								continue;

							int nDiff, nPix, nSum = 0;
							unsigned char *pCPix = cur->PicPix,
														*pSPix = src->PicPix;

							for (int i_f=0; i_f<PIXELS_SIZE; i_f+=4)
							{
								nPix=i_f*PIXELS_SIZE + i_f;
								nDiff = pCPix[nPix]-pSPix[nPix];
								nSum += nDiff*nDiff;
								nPix = i_f*PIXELS_SIZE + PIXELS_SIZE - i_f;
								nDiff = pCPix[nPix] - pSPix[nPix];
								nSum += nDiff*nDiff;
							}
							if (nSum>nMaxFastSum)
								continue;

							for (int i_f = PIXELS_SIZE*PIXELS_SIZE - 1; i_f >= 0; i_f--)
							{
								nDiff = pCPix[i_f] - pSPix[i_f];
								nSum += nDiff*nDiff;
							}
							if (nSum>nMaxSlowSum)
								continue;
						}
					}
					else if (Opt.DupMusic && (src->dwFlags&RCIF_MUSIC) && (cur->dwFlags&RCIF_MUSIC))
					{
						if (Opt.DupMusicDuration)
						{
							int nDiff=(src->MusicTime>=cur->MusicTime?src->MusicTime-cur->MusicTime:cur->MusicTime-src->MusicTime);
							if (nDiff>Opt.DupMusicDurationSec)
								continue;
						}
						if (Opt.DupMusicArtist)
						{
							if (!src->MusicArtist || !(*src->MusicArtist) || !cur->MusicArtist || !(*cur->MusicArtist))
								continue;

							if (Opt.DupMusicArtist==2)
							{
								unsigned LenS=wcslen(src->MusicArtist);
								unsigned LenC=wcslen(cur->MusicArtist);
								int r=1;
								if (LenS<=LenC)
								{
									for (unsigned l=0;l<LenC;l++)
									{
										r=FSF.LStrnicmp(cur->MusicArtist+l,src->MusicArtist,LenS);
										if (r==0)
											break;
									}
								}
								if (r)
									continue;
							}
							else
							{
								if (FSF.LStricmp(cur->MusicArtist,src->MusicArtist))
									continue;
							}
						}
						if (Opt.DupMusicTitle)
						{
							if (!src->MusicTitle || !(*src->MusicTitle) || !cur->MusicTitle || !(*cur->MusicTitle))
								continue;

							if (Opt.DupMusicTitle==2)
							{
								unsigned LenS=wcslen(src->MusicTitle);
								unsigned LenC=wcslen(cur->MusicTitle);
								int r=1;
								if (LenS<=LenC)
								{
									for (unsigned l=0;l<LenC;l++)
									{
										r=FSF.LStrnicmp(cur->MusicTitle+l,src->MusicTitle,LenS);
										if (r==0)
											break;
									}
								}
								if (r)
									continue;
							}
							else
							{
								if (FSF.LStricmp(cur->MusicTitle,src->MusicTitle))
									continue;
							}
						}
					}
					else
					{
						if (src->nFileSize!=cur->nFileSize)
							continue;
						if (src->nFileSize && cur->nFileSize && (!src->dwCRC || !cur->dwCRC || src->dwCRC!=cur->dwCRC))
							continue;
					}
					bSetGroupCount=true;
					src->nGroup=IndexGroup;
					cur->nGroup=IndexGroup;
				}
			}
			if (bSetGroupCount)
			{
				dFList.GroupCount=IndexGroup;
				IndexGroup++;
			}
		}
	}

	if (bBrokenByEsc)
		goto END;

	// освободимся от уникальных
	FSF.qsort(dFList.F,dFList.iCount,sizeof(dFList.F[0]),dupSortListByGroup);
	int Index;
	for (Index=dFList.iCount-1; Index>=0 && !dFList.F[Index].nGroup; Index--)
	{
		free(dFList.F[Index].FileName);
		dFList.F[Index].FileName=NULL;
		free(dFList.F[Index].Dir);
		dFList.F[Index].Dir=NULL;
		free(dFList.F[Index].PicPix);
		dFList.F[Index].PicPix=NULL;
		free(dFList.F[Index].MusicArtist);
		dFList.F[Index].MusicArtist=NULL;
		free(dFList.F[Index].MusicTitle);
		dFList.F[Index].MusicTitle=NULL;
	}

	dFList.F=(dupFile*)realloc(dFList.F,(dFList.iCount=Index+1)*sizeof(dupFile));

	if (hConInp!=INVALID_HANDLE_VALUE) CloseHandle(hConInp);
	Info.PanelControl(LPanel.hPanel,FCTL_REDRAWPANEL,0,0);
	Info.PanelControl(RPanel.hPanel,FCTL_REDRAWPANEL,0,0);

	ShowDupDialog();

	CmpInfo.Proc=0;
	CmpInfo.ProcSize=0;
	if (Opt.Dup==QR_ALL)
	{
		for (int i=0; i<dFList.iCount; i++)
		{
			dupFile *cur=&dFList.F[i];
			if (cur->dwFlags&RCIF_USERDEL)
			{
				string strName;
				GetFullFileName(strName,cur->Dir,cur->FileName);
				if (!(ret=DelFile(strName.get())))
					break;
			}
		}

		if (ret && !bBrokenByEsc)
		{
			if (Opt.Sound && (GetTickCount()-dwTicks > 30000)) MessageBeep(MB_ICONASTERISK);
			Info.AdvControl(&MainGuid,ACTL_PROGRESSNOTIFY,0,0);
		}
		if (!bBrokenByEsc)
		{
			if (CmpInfo.Errors && Opt.ShowMsg) ErrorMsg(MOpenErrorTitle,MOpenErrorBody);
			if (Opt.ShowMsg)
			{
				wchar_t buf[80], size[10];
				FSF.FormatFileSize(CmpInfo.ProcSize,7,FFFS_FLOATSIZE|FFFS_SHOWBYTESINDEX|FFFS_ECONOMIC,size,8);
				FSF.sprintf(buf,GetMsg(MDupDelItems), size, CmpInfo.Proc);
				const wchar_t *MsgItems[] = { GetMsg(MDupTitle), buf, GetMsg(MOK) };
				Info.Message(&MainGuid,&NoDiffMsgGuid,0,0,MsgItems,sizeof(MsgItems) / sizeof(MsgItems[0]),1);
			}
		}
	}
	if (Opt.Dup==QR_ALL || Opt.Dup==QR_SKIP)
	{
		Info.PanelControl(LPanel.hPanel,FCTL_UPDATEPANEL,0,0);
		Info.PanelControl(RPanel.hPanel,FCTL_UPDATEPANEL,0,0);
	}

END:
	if (hConInp!=INVALID_HANDLE_VALUE) CloseHandle(hConInp);
	return ret;
}
