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
	Opt.DupDelRecycleBin=(Opt.DupDel && (Info.AdvControl(&MainGuid,ACTL_GETSYSTEMSETTINGS,0,0) & FSS_DELETETORECYCLEBIN));

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
	wchar_t buf[65536];
//	wchar_t Time[20];
	wchar_t Size[10];
	wchar_t MetaData[80];

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
		string strPath(GetPosToName(cur->Dir));
		FSF.TruncPathStr(strPath.get(),40);
		strPath.updsize();
		if ((cur->dwFlags&RCIF_PIC) || (cur->dwFlags&RCIF_PICERR && cur->PicWidth && cur->PicHeight))
		{
			wchar_t w[20], h[20];
			FSF.FormatFileSize(cur->PicWidth,7,FFFS_COMMAS,w,8);
			FSF.FormatFileSize(cur->PicHeight,7,FFFS_COMMAS,h,8);
			FSF.sprintf(MetaData,L"%s x %s",w,h);
		}
		else if (cur->dwFlags&RCIF_MUSIC)
		{
			FSF.sprintf(MetaData,L"%02u:%02u %03uKb/s %-20.20s %-20.20s",cur->MusicTime/60,cur->MusicTime%60,cur->MusicBitrate,(cur->dwFlags&RCIF_MUSICART)?cur->MusicArtist:L"",(cur->dwFlags&RCIF_MUSICTIT)?cur->MusicTitle:L"");
		}
		else
		{
			GetStrFileTime(&cur->ftLastWriteTime,MetaData,true);
		}

		FSF.sprintf(buf, L"%*d%c%*.*s%c%-*.*s%c%-*.*s%c%s",digits_count,cur->nGroup,0x2502,7,7,Size,0x2551,60,60,MetaData,0x2551,40,40,strPath.get(),0x2551,cur->FileName);

		struct FarListItem Item;
		Item.Flags=0;
		if (cur->dwFlags&RCIF_USERDEL)
		{
			Item.Flags|=(LIF_CHECKED|LIF_GRAYED);
			pFileList->Del++;
		}
		Item.Text=buf;
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
		ListPos.SelectPos=0; //ListInfo.SelectPos;
		ListPos.TopPos=-1/*ListInfo.TopPos*/;
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
			dupFile *cur=(tmp && *tmp)?*tmp:NULL;
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
				dupFile *cur=(tmp && *tmp)?*tmp:NULL;
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
						dupFile *cur=(tmp && *tmp)?*tmp:NULL;
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
						dupFile *cur=(tmp && *tmp)?*tmp:NULL;
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
				}
				else if (IsCtrl(record))
				{
					if (vk==VK_CONTROL)
					{
						int Pos=Info.SendDlgMessage(hDlg,DM_LISTGETCURPOS,0,0);
						cmpFile **tmp=(cmpFile **)Info.SendDlgMessage(hDlg,DM_LISTGETDATA,0,(void *)Pos);
						cmpFile *cur=(tmp && *tmp)?*tmp:NULL;
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
					else if (vk==VK_PRIOR)
					{
						int Pos=Info.SendDlgMessage(hDlg,DM_LISTGETCURPOS,0,0);
						cmpFile **tmp=(cmpFile **)Info.SendDlgMessage(hDlg,DM_LISTGETDATA,0,(void *)Pos);
						cmpFile *cur=(tmp && *tmp)?*tmp:NULL;
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
*/
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
		Info.DialogRun(hDlg);
		Info.DialogFree(hDlg);
	}

/*
	if (Opt.Mode==MODE_SYNC && Opt.Sync==2)
		Synchronize(&cFList);
*/
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
				ppi.FileSize=((__int64)wfdFindData.nFileSizeHigh << 32) | wfdFindData.nFileSizeLow;
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

int AdvCmpProc::GetMp3(dupFile *cur)
{
	int ret=0;

	if (cur->dwFlags&RCIF_MUSIC)
		return 1;

	if (CheckForEsc())
		return ret;

	HSTREAM stream;

	if (FSF.ProcessName(L"*.mp3",cur->FileName,0,PN_CMPNAME))
	{
		string strFullFileName;
		GetFullFileName(strFullFileName,cur->Dir,cur->FileName);

		if (stream=pBASS_StreamCreateFile(FALSE,strFullFileName.get(),0,0,BASS_STREAM_DECODE|BASS_UNICODE))
		{
			BASS_CHANNELINFO info;
			if (pBASS_ChannelGetInfo(stream,&info) && info.ctype==BASS_CTYPE_STREAM_MP3)
				cur->dwFlags|=RCIF_MUSIC;
		}
	}

	if (!(cur->dwFlags&RCIF_MUSIC))
	{
		if (stream) pBASS_StreamFree(stream);
		return ret;
	}

	ShowDupMsg(GetPosToName(cur->Dir),cur->FileName,true);

	cur->MusicBitrate=(DWORD)(pBASS_StreamGetFilePosition(stream,BASS_FILEPOS_END)/
														(125*pBASS_ChannelBytes2Seconds(stream,pBASS_ChannelGetLength(stream,BASS_POS_BYTE)))+0.5); // bitrate (Kbps)
	cur->MusicTime=pBASS_ChannelBytes2Seconds(stream,pBASS_ChannelGetLength(stream,BASS_POS_BYTE));
	TAG_ID3 *id3=(TAG_ID3*)pBASS_ChannelGetTags(stream,BASS_TAG_ID3);
	if (id3)
	{
		int len=lstrlenA(id3->artist);
		if (len)
		{
			cur->MusicArtist=(wchar_t*)malloc((len+1)*sizeof(wchar_t));
			if (cur->MusicArtist)
			{
				MultiByteToWideChar(CP_ACP,0,id3->artist,-1,cur->MusicArtist,len+1);
				cur->dwFlags|=RCIF_MUSICART;
			}
		}

		len=lstrlenA(id3->title);
		if (len)
		{
			cur->MusicTitle=(wchar_t*)malloc((len+1)*sizeof(wchar_t));
			if (cur->MusicTitle)
			{
				MultiByteToWideChar(CP_ACP,0,id3->title,-1,cur->MusicTitle,len+1);
				cur->dwFlags|=RCIF_MUSICTIT;
			}
		}
	}
/*
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

		if (!cur->MusicArtist && Ptr!=lenName)
		{
			cur->MusicArtist=(wchar_t*)malloc((Ptr+1)*sizeof(wchar_t));
			if (cur->MusicArtist)
			{
				bool bNum=true;
				for (int i=0,j=0; Name[j] && j<Ptr; j++)
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
			}
		}
		if (!cur->MusicTitle)
		{
			if (Ptr!=lenName)
			{
				cur->MusicTitle=(wchar_t*)malloc((lenName-Ptr)*sizeof(wchar_t));
				if (cur->MusicTitle)
					for (int i=0,j=Ptr+3; j<lenName; i++,j++)
						cur->MusicTitle[i]=Name[j];
			}
			else
			{
				cur->MusicTitle=(wchar_t*)malloc((lenName+1)*sizeof(wchar_t));
				if (cur->MusicTitle)
					for (int i=0; Name[i] && i<lenName; i++)
						cur->MusicTitle[i]=Name[i];
			}
		}
	}
*/
	pBASS_StreamFree(stream);

	return ret=1;
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

int AdvCmpProc::Duplicate(const struct DirList *pList)
{
	int ret=0;

	if (!Opt.DupName && !Opt.DupSize && !Opt.DupContents)
		goto END;

	bBrokenByEsc=false;
	bStartMsg=true;
	bAskLReadOnly=bAskRReadOnly=Info.AdvControl(&MainGuid,ACTL_GETCONFIRMATIONS,0,0) & FCS_OVERWRITEDELETEROFILES;
	bAskDel=Info.AdvControl(&MainGuid,ACTL_GETCONFIRMATIONS,0,0) & FCS_DELETE;
	bSkipLReadOnly=bSkipRReadOnly=false;
	DWORD dwTicks=GetTickCount();

	ret=ScanDir(pList->Dir,0);

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
						if (FSF.LStricmp(src->FileName,cur->FileName))
							continue;   // разные, сразу переходим к следующему
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

					if (!Opt.DupPic && !Opt.DupMusic && src->nFileSize != cur->nFileSize)
						continue;   // разные, сразу переходим к следующему

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
							if (nDiff>Opt.DupMusicDuration)
								continue;
						}
						if (Opt.DupMusicArtist)
						{
							if (!src->MusicArtist || !cur->MusicArtist)
								continue;

							unsigned LenS=wcslen(src->MusicArtist);
							unsigned LenC=wcslen(cur->MusicArtist);
							bool srcbig=(LenS>LenC); //исходная строка больше - ищем в исходной, и наоборот
							int r=1;
							for (int i=0;i<(srcbig?LenS:LenC);i++)
							{
								r=(srcbig?FSF.LStrnicmp(src->MusicArtist+i,cur->MusicArtist,LenC):FSF.LStrnicmp(cur->MusicArtist+i,src->MusicArtist,LenS));
								if (r==0)
									break;
							}
							if (r)
								continue;
						}
						if (Opt.DupMusicTitle)
						{
							if (!src->MusicTitle || !cur->MusicTitle)
								continue;

							unsigned LenS=wcslen(src->MusicTitle);
							unsigned LenC=wcslen(cur->MusicTitle);
							bool srcbig=(LenS>LenC); //исходная строка больше - ищем в исходной, и наоборот
							int r=1;
							for (int i=0;i<(srcbig?LenS:LenC);i++)
							{
								r=(srcbig?FSF.LStrnicmp(src->MusicTitle+i,cur->MusicTitle,LenC):FSF.LStrnicmp(cur->MusicTitle+i,src->MusicTitle,LenS));
								if (r==0)
									break;
							}
							if (r)
								continue;
						}
					}
					else if (cur->nFileSize)
					{
						if (!src->dwCRC || !cur->dwCRC || src->dwCRC!=cur->dwCRC)
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
