/****************************************************************************
 * AdvCmpProc_SYNC.cpp
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
 *                               СИНХРОНИЗАЦИЯ
 *
 ***************************************************************************/

/***************************************************************************
 * Узнаем опции синхронизации
 ***************************************************************************/
int GetSyncOpt(cmpFileList *pFileList)
{
	int ret=QR_EDIT; // продолжаем редактировать список
	int ItemsLNew=0, ItemsRNew=0, ItemsRDel=0;

	for (int i=0; i<pFileList->iCount; i++)
	{
		cmpFile *cur=&pFileList->F[i];

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
		return ret=QR_SKIP; //нет элементов
	}

	Opt.SyncLPanel=ItemsRNew, Opt.SyncRPanel=ItemsLNew, Opt.SyncDel=ItemsRDel;

	wchar_t buf1[80], buf2[80], buf3[80];
	FSF.sprintf(buf1,GetMsg(MSyncLPanel),ItemsRNew);
	FSF.sprintf(buf2,GetMsg(MSyncRPanel),ItemsLNew);
	FSF.sprintf(buf3,GetMsg(MSyncDel),ItemsRDel);

	struct FarDialogItem DialogItems[] = {
		//			Type	X1	Y1	X2	Y2				Selected	History	Mask	Flags	Data	MaxLen	UserParam
		/* 0*/{DI_DOUBLEBOX,  3, 1,60, 8,         0, 0, 0,             0, GetMsg(MSyncTitle),0,0},
		/* 1*/{DI_CHECKBOX,   5, 2, 0, 0, Opt.SyncRPanel, 0, 0,Opt.SyncRPanel?DIF_FOCUS:DIF_DISABLE,buf2,0,0},
		/* 2*/{DI_CHECKBOX,   5, 3, 0, 0, Opt.SyncLPanel, 0, 0,Opt.SyncLPanel?0:DIF_DISABLE,buf1,0,0},
		/* 3*/{DI_CHECKBOX,   5, 4, 0, 0, Opt.SyncDel, 0, 0, Opt.SyncDel?0:DIF_DISABLE,buf3,0,0},
		/* 4*/{DI_CHECKBOX,   8, 5, 0, 0,         0, 0, 0, Opt.SyncDel?0:DIF_DISABLE,GetMsg(MSyncUseDelFilter),0,0},
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
			Opt.SyncDel=Info.SendDlgMessage(hDlg,DM_GETCHECK,3,0);
			Opt.SyncUseDelFilter=Info.SendDlgMessage(hDlg,DM_GETCHECK,4,0);
			ret=(Opt.SyncLPanel || Opt.SyncRPanel || Opt.SyncDel)?QR_ALL:QR_SKIP;   // синхронизируем, иначе - пропустим
		}
		else if (ret==7)
			ret=QR_EDIT;
		else
		{
			bBrokenByEsc=true;
			ret=QR_ABORT; // отменили синхронизацию
		}
		Info.DialogFree(hDlg);
	}
	else
		ret=QR_ABORT;

	return ret;
}

/***************************************************************************
 * Запрос на перезапись файлов
 ***************************************************************************/
int AdvCmpProc::QueryOverwriteFile(const wchar_t *FileName, FILETIME *srcTime, FILETIME *destTime, unsigned __int64 srcSize, unsigned __int64 destSize, int direction, bool bReadOnlyType)
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

/***************************************************************************
 * Запрос на удаление файлов
 ***************************************************************************/
int AdvCmpProc::QueryDelete(const wchar_t *FileName, bool bIsDir, bool bReadOnlyType)
{
	const wchar_t *MsgItems[]=
	{
		GetMsg(MWarning),
		GetMsg(bReadOnlyType ? MFileIsReadOnly : ((Opt.Mode==MODE_DUP && Opt.DupDelRecycleBin)?(bIsDir?MDelFolderRecycleBin:MDelFileRecycleBin):(bIsDir?MDelFolder:MDelFile))),
		GetPosToName(FileName),
		GetMsg(MAskDel),
		GetMsg(MDelete), GetMsg(MAll), GetMsg(MSkip), GetMsg(MSkipAll), GetMsg(MCancel)
	};

	int ExitCode=Info.Message(&MainGuid,&QueryDelMsgGuid,FMSG_WARNING,NULL,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]), 5);

	return (ExitCode<=QR_SKIPALL?ExitCode:QR_ABORT);
}

/***************************************************************************
 * Диалог-прогресс
 ***************************************************************************/
void ShowSyncMsg(const wchar_t *Name1, const wchar_t *Name2, unsigned __int64 Progress, unsigned __int64 Max, bool bRedraw)
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

	Info.Message(&MainGuid,&SyncMsgGuid,bStartMsg?FMSG_LEFTALIGN:FMSG_LEFTALIGN|FMSG_KEEPBACKGROUND,NULL,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]),0);
	bStartMsg=false;
}

struct SynchronizeFileCopyCallbackData
{
	wchar_t *srcFileName;
	wchar_t *destFileName;
};

DWORD WINAPI SynchronizeFileCopyCallback( LARGE_INTEGER TotalFileSize, LARGE_INTEGER TotalBytesTransferred, LARGE_INTEGER StreamSize, LARGE_INTEGER StreamBytesTransferred,
                                          DWORD dwStreamNumber, DWORD dwCallbackReason, HANDLE hSourceFile, HANDLE hDestinationFile, LPVOID lpData )
{
	unsigned __int64 progress=TotalBytesTransferred.QuadPart, max=TotalFileSize.QuadPart;

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

/***************************************************************************
 * Проверка на наличие файла
 ***************************************************************************/
int AdvCmpProc::FileExists(const wchar_t *FileName, unsigned __int64 *pSize, FILETIME *pTime, DWORD *pAttrib, int CheckForFilter)
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
			ppi.FileSize=((unsigned __int64)wfdFindData.nFileSizeHigh << 32) | wfdFindData.nFileSizeLow;
			ppi.FileName=FSF.PointToName(FileName);

			if ( (CheckForFilter>0?LPanel.hFilter:RPanel.hFilter)!=INVALID_HANDLE_VALUE &&
					 !Info.FileFilterControl((CheckForFilter>0?LPanel.hFilter:RPanel.hFilter),FFCTL_ISFILEINFILTER,0,&ppi))
				return ret;

			if (Opt.Filter && !Info.FileFilterControl(Opt.hCustomFilter,FFCTL_ISFILEINFILTER,0,&ppi))
				return ret;
		}
		*pSize=((unsigned __int64)wfdFindData.nFileSizeHigh << 32) | wfdFindData.nFileSizeLow;
		*pTime=wfdFindData.ftLastWriteTime;
		*pAttrib=wfdFindData.dwFileAttributes;
		ret=1; // ОК
	}

	return ret;
}

/***************************************************************************
 * Синхронизация файлов (копирование)
 ***************************************************************************/
int AdvCmpProc::SyncFile(const wchar_t *srcFileName, const wchar_t *destFileName, int direction)
{
	if (bBrokenByEsc)
		return 0;

	int ret=1;

	// если сказали - "а мы не хотим туда копировать", то пропустим...
	if (((direction < 0) && !Opt.SyncLPanel) || ((direction > 0) && !Opt.SyncRPanel))
		return ret;

	unsigned __int64 srcSize=0, destSize=0;
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

/***************************************************************************
 * Синхронизация файлов (удаление)
 * Теперь оно же и при удалении дубликатов
 ***************************************************************************/
int AdvCmpProc::DelFile(const wchar_t *FileName)
{
	if (bBrokenByEsc)
		return 0;

	int ret=1;

	if ((Opt.Mode==MODE_SYNC && !Opt.SyncDel) || (Opt.Mode==MODE_DUP && !Opt.DupDel))
		return ret;

	unsigned __int64 Size=0;
	DWORD Attrib=0;
	FILETIME Time;

	if (FileName && FileExists(FileName,&Size,&Time,&Attrib,(Opt.Mode==MODE_SYNC && Opt.SyncUseDelFilter?-1:0))) // -1, т.е. справа
	{
		int doDel=1;

		// Delete confirmation
		if (bAskDel)
		{
			switch(QueryDelete(FileName,false,false))
			{
				case QR_DELETE:
					doDel=1;
					break;
				case QR_ALL:
					doDel=1;
					bAskDel=false;
					break;
				case QR_SKIP:
					doDel=0;
					break;
				case QR_SKIPALL:
					doDel=0;
					Opt.Mode==MODE_SYNC?Opt.SyncDel=0:Opt.DupDel=0;
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
			if (bSkipRReadOnly)  // для синхронизации это правая панель, а для дубликатов будем просто юзать переменную :)
			{
				doDel=0;
			}
			else if (bAskRReadOnly)  // для синхронизации это правая панель, а для дубликатов будем просто юзать переменную :)
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

			const wchar_t *MsgItems[]=
			{
				GetMsg(MWarning),
				GetMsg((Opt.Mode==MODE_DUP && Opt.DupDelRecycleBin)?MFailedDelFileRecycleBin:MFailedDelFile),
				GetPosToName(FileName),
				GetMsg(MRetry), GetMsg(MSkip), GetMsg(MCancel)
			};

RetryDelFile:

			SetLastError(0);

			if (Opt.Mode==MODE_DUP && Opt.DupDelRecycleBin)
			{
				SHFILEOPSTRUCT shs;
				memset(&shs,0,sizeof(shs));
				int len=wcslen(GetPosToName(FileName));
				wchar_t *lpwszName=(wchar_t*)malloc((len+2)*sizeof(wchar_t));
				if (lpwszName)
				{
					wcscpy(lpwszName,GetPosToName(FileName));
					lpwszName[len+1]=0;
					shs.pFrom=lpwszName;
					shs.wFunc=FO_DELETE;
					shs.fFlags=FOF_ALLOWUNDO|FOF_SILENT|FOF_NOCONFIRMATION;
					DWORD Result=SHFileOperation(&shs);
					free(lpwszName);

					if (Result || shs.fAnyOperationsAborted)
					{
						int ExitCode= bBrokenByEsc ? 2/*MCancel*/ : Info.Message(&MainGuid,&FailedDelFileMsgGuid,FMSG_WARNING|FMSG_ERRORTYPE,0,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]),3);

						if (!ExitCode)
								goto RetryDelFile;
						else if (ExitCode != 1)
							ret=0;
					}
					else
					{
						CmpInfo.Proc+=1;
						CmpInfo.ProcSize+=Size;
					}
				}
				else
					ret=0;
			}
			else
			{
				SetFileAttributesW(FileName,Attrib & ~(FILE_ATTRIBUTE_READONLY|FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_SYSTEM));

				if (!DeleteFileW(FileName))
				{
					int ExitCode=bBrokenByEsc ? 2/*MCancel*/ : Info.Message(&MainGuid,&FailedDelFileMsgGuid,FMSG_WARNING|FMSG_ERRORTYPE,0,MsgItems,sizeof(MsgItems)/sizeof(MsgItems[0]),3);

					if (!ExitCode)
						goto RetryDelFile;
					else if (ExitCode != 1)
					{
						SetFileAttributesW(FileName,Attrib); // пробуем восстановить атрибуты
						ret=0;
					}
				}
				else
				{
					CmpInfo.Proc+=1;
					CmpInfo.ProcSize+=Size;
				}
			}
		}
	}
	return ret;
}

/***************************************************************************
 * Синхронизация каталогов (копирование)
 ***************************************************************************/
int AdvCmpProc::SyncDir(const wchar_t *srcDirName, const wchar_t *destDirName, int direction)
{
	if (bBrokenByEsc)
		return 0;

	if (!srcDirName || !*srcDirName || !destDirName || !*destDirName)
	{
		SetLastError(E_INVALIDARG);
		return 0;
	}

	unsigned __int64 Size=0;
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

/***************************************************************************
 * Синхронизация каталогов (удаление)
 ***************************************************************************/
int AdvCmpProc::DelDir(const wchar_t *DirName)
{
	if (bBrokenByEsc)
		return 0;

	int ret=1;

	if (!Opt.SyncDel)
		return ret;

	if (!DirName || !*DirName)
	{
		SetLastError(E_INVALIDARG);
		return 0;
	}

	unsigned __int64 Size=0;
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
		if (bAskDel)
		{
			switch(QueryDelete(DirName,true,false))
			{
				case QR_DELETE:
					doDel=1;
					break;
				case QR_ALL:
					doDel=1;
					bAskDel=false;
					break;
				case QR_SKIP:
					doDel=0;
					break;
				case QR_SKIPALL:
					doDel=0;
					Opt.SyncDel=0;
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
							if (!DelDir(strNew))
								ret=0;
						}
					}
					else
					{
						if (!DelFile(strNew))
							ret=0;
					}
				}
				while (ret && FindNextFile(hFind,&wfdFindData));
				FindClose(hFind);
			}
		}

		if (ret && doDel && Opt.SyncDel)
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

/***************************************************************************
 * Основания функция синхронизации
 ***************************************************************************/
int AdvCmpProc::Synchronize()
{
	int ret=0;

	if (Opt.Sync==QR_ALL) // есть элементы, синхронизируем
	{
		bBrokenByEsc=false;
		bStartMsg=true;
		bAskLOverwrite=bAskROverwrite=GetFarSetting(FSSF_CONFIRMATIONS,L"Copy")?true:false;
		bAskLReadOnly=bAskRReadOnly=GetFarSetting(FSSF_CONFIRMATIONS,L"RO")?true:false;
		bAskDel=GetFarSetting(FSSF_CONFIRMATIONS,L"Delete")?true:false;
		bSkipLReadOnly=bSkipRReadOnly=false;

		hConInp=CreateFileW(L"CONIN$", GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
		DWORD dwTicks=GetTickCount();
		cmpFileList *pFileList=&cFList;

		for (int i=0; i<pFileList->iCount; i++)
		{
			cmpFile *cur=&pFileList->F[i];
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
					if (!(ret=DelDir(strName)))
						break;
				}
				else
				{
					if (!(ret=DelFile(strName)))
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
