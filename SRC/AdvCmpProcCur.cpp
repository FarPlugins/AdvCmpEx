/****************************************************************************
 * AdvCmpProcCur.cpp
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
#include "AdvCmpProcCur.hpp"


void AdvCmpProcCur::SetPicDefaultExtentions()
{
	int number=pGflGetNumberOfFormat();
	GFL_FORMAT_INFORMATION finfo;
	for (int i=0;i<number;i++)
	{
		pGflGetFormatInformationByIndex(i,&finfo);
		for (DWORD j=0;j<finfo.NumberOfExtension;j++)
		{
			PicExts=(wchar_t **)(PicExts?realloc(PicExts,(PicExtsNum+1)*sizeof(*PicExts)):malloc((PicExtsNum+1)*sizeof(*PicExts)));
			int size = MultiByteToWideChar(CP_ACP,0,finfo.Extension[j],-1,0,0);
			PicExts[PicExtsNum]=(wchar_t *)malloc(size*sizeof(wchar_t));
			MultiByteToWideChar(CP_ACP,0,finfo.Extension[j],-1,PicExts[PicExtsNum],size-1);
			PicExtsNum++;
		}
	}
}

bool AdvCmpProcCur::CheckName(const wchar_t *FileName)
{
	int flen=wcslen(FileName);
	for (DWORD i=0;i<PicExtsNum&&PicExts;i++)
		if(PicExts[i]&&wcslen(PicExts[i])<flen)
			if(!FSF.LStricmp(FileName+flen-wcslen(PicExts[i]),PicExts[i])&&FileName[flen-wcslen(PicExts[i])-1]==L'.') return true;
	return false;
}

void AdvCmpProcCur::GetDIBFromBitmap(GFL_BITMAP *bitmap,BITMAPINFOHEADER *bitmap_info,unsigned char **data)
{
	int bytes_per_line;

	*data=NULL;
	memset(bitmap_info,0,sizeof(BITMAPINFOHEADER));

	bitmap_info->biSize=sizeof(BITMAPINFOHEADER);
	bitmap_info->biWidth=bitmap->Width;
	bitmap_info->biHeight=bitmap->Height;
	bitmap_info->biPlanes=1;

	bytes_per_line=(bitmap->Width*3+3)&-4;
	bitmap_info->biClrUsed=0;
	bitmap_info->biBitCount=24;
	bitmap_info->biCompression=BI_RGB;
	bitmap_info->biSizeImage=bytes_per_line*bitmap->Height;
	bitmap_info->biClrImportant=0;

	*data=(unsigned char*)malloc(bitmap_info->biSizeImage);

	if (*data)
		memcpy(*data,bitmap->Data,bitmap_info->biSizeImage);

	return;
}

RECT AdvCmpProcCur::RangingPic(RECT DCRect,GFL_BITMAP *RawPicture)
{
	float asp_dst=(float)(DCRect.right-DCRect.left)/(float)(DCRect.bottom-DCRect.top);
	float asp_src=(float)RawPicture->Width/(float)RawPicture->Height;

	int dst_w;
	int dst_h;

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

	RECT dest={DCRect.left,DCRect.top,dst_w,dst_h};
	return dest;
}


bool AdvCmpProcCur::DrawImage(PicData *data)
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
			GFL_BITMAP *pic=NULL;
			RangedRect=RangingPic(DCRect,RawPicture);
			RangedRect.left+=(DCRect.right-DCRect.left-RangedRect.right)/2;
			RangedRect.top+=(DCRect.bottom-DCRect.top-RangedRect.bottom)/2;
			pGflResize(RawPicture,&pic,RangedRect.right,RangedRect.bottom,GFL_RESIZE_BILINEAR,0);
			if (pic)
			{
				GetDIBFromBitmap(pic,data->BmpHeader,&data->DibData);
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
	return result;
}

bool AdvCmpProcCur::UpdateImage(PicData *data, bool CheckOnly)
{
	if (!data->DibData&&!data->Loaded)
	{
		if (DrawImage(data))
		{
			data->Loaded=true;
			if ((!(data->FirstRun))&&(!CheckOnly))
				InvalidateRect(WinInfo.hFarWindow,NULL,TRUE);
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

void AdvCmpProcCur::FreeImage(PicData *data)
{
	if (data->DibData)
	{
		free(data->DibData);
		data->DibData=NULL;
	}
	pGflFreeFileInformation(data->pic_info);
}

void AdvCmpProcCur::UpdateInfoText(HANDLE hDlg, PicData *data, bool left)
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

INT_PTR WINAPI AdvCmpProcCur::ShowCmpCurDialogProcThunk(HANDLE hDlg, int Msg, int Param1, void *Param2)
{
	AdvCmpProcCur* Class=(AdvCmpProcCur*)Info.SendDlgMessage(hDlg,DM_GETDLGDATA,0,0);
	return Class->ShowCmpCurDialogProc(hDlg,Msg,Param1,Param2);
}

INT_PTR WINAPI AdvCmpProcCur::ShowCmpCurDialogProc(HANDLE hDlg,int Msg,int Param1,void *Param2)
{
	switch(Msg)
	{
		case DN_CTLCOLORDLGITEM:
			if (Param1!=0)
				return (Info.AdvControl(&MainGuid,ACTL_GETCOLOR,COL_PANELTEXT,0)<<16)|(Info.AdvControl(&MainGuid,ACTL_GETCOLOR,COL_PANELTEXT,0)<<8)|(Info.AdvControl(&MainGuid,ACTL_GETCOLOR,COL_PANELSELECTEDTITLE,0));
			break;

		case DN_DRAWDLGITEM:
			LPicData.Redraw=true;
			RPicData.Redraw=true;
			break;
		case DN_ENTERIDLE:
			if (LPicData.Redraw)
			{
				LPicData.Redraw=false;
				UpdateImage(&LPicData);
				if (LPicData.FirstRun)
				{
					LPicData.FirstRun=false;
					UpdateInfoText(hDlg,&LPicData,true);
					UpdateInfoText(hDlg,&RPicData,false);
				}
			}
			if (RPicData.Redraw)
			{
				RPicData.Redraw=false;
				UpdateImage(&RPicData);
				if (RPicData.FirstRun)
				{
					RPicData.FirstRun=false;
					UpdateInfoText(hDlg,&LPicData,true);
					UpdateInfoText(hDlg,&RPicData,false);
				}
			}
			break;
		case 0x3FFF:
			if (Param1)
			{
				UpdateImage(&LPicData);
				UpdateImage(&RPicData);
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


int AdvCmpProcCur::ShowCmpCurDialog(const PluginPanelItem *pLPPI,const PluginPanelItem *pRPPI)
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

//	FSF.itoa64(pLPPI->FileSize,Buf1,10);
	itoaa(pLPPI->FileSize,Buf1);
	FSF.sprintf(strBuf1.get(), L"%*.*s",DialogItems[5].X2,DialogItems[5].X2,Buf1);
	DialogItems[5].Data=strBuf1.get();

//	FSF.itoa64(pRPPI->FileSize,Buf1,10);
	itoaa(pRPPI->FileSize,Buf1);
	DialogItems[6].Data=Buf1;

	FSF.sprintf(Buf2, L"%-*.*s  %02d.%02d.%04d  %02d:%02d:%02d", 
							10,10,LAttributes,LModificTime.wDay,LModificTime.wMonth,LModificTime.wYear,LModificTime.wHour,LModificTime.wMinute,LModificTime.wSecond);
	FSF.sprintf(strBuf3.get(), L"%*.*s",DialogItems[7].X2,DialogItems[7].X2,Buf2);
	DialogItems[7].Data=strBuf3.get();

	FSF.sprintf(Buf2, L"%02d:%02d:%02d  %02d.%02d.%04d  %-*.*s", 
							RModificTime.wHour,RModificTime.wMinute,RModificTime.wSecond,RModificTime.wDay,RModificTime.wMonth,RModificTime.wYear,10,10,RAttributes);
	DialogItems[8].Data=Buf2;

	int color=Info.AdvControl(&MainGuid,ACTL_GETCOLOR,COL_PANELTEXT,0);
	color=color&0xF0;
	color=color|(color>>4);

	UpdateImage(&LPicData,true);
	UpdateImage(&RPicData,true);

	unsigned int VBufSize=(WinInfo.Con.Right-WinInfo.Con.Left)*(WinInfo.Con.Bottom-WinInfo.Con.Top-2-1);
	CHAR_INFO *VirtualBuffer=(CHAR_INFO *)malloc(VBufSize*sizeof(CHAR_INFO));
	if (VirtualBuffer)
	{
		DialogItems[0].VBuf=VirtualBuffer;
		for(unsigned int i=0;i<VBufSize;i++)
		{
			VirtualBuffer[i].Char.UnicodeChar=L'.';
			VirtualBuffer[i].Attributes=color;
		}

		HANDLE hDlg=Info.DialogInit(&MainGuid,&CurDlgGuid,0,0,WinInfo.Con.Right,WinInfo.Con.Bottom-WinInfo.Con.Top,NULL,DialogItems,
																sizeof(DialogItems)/sizeof(DialogItems[0]),0,FDLG_SMALLDIALOG|FDLG_NODRAWSHADOW,ShowCmpCurDialogProcThunk,this);
		if (hDlg != INVALID_HANDLE_VALUE)
		{
			Info.DialogRun(hDlg);
			Info.DialogFree(hDlg);
		}

		free(VirtualBuffer);
	}

	FreeImage(&LPicData);
	FreeImage(&RPicData);

	return true;
}

bool AdvCmpProcCur::CompareCurFile(const struct DirList *pLList,const struct DirList *pRList)
{
	strLFullFileName=pLList->Dir;
	if (strLFullFileName.length()>0 && strLFullFileName[(size_t)(strLFullFileName.length()-1)]!=L'\\') strLFullFileName+=L"\\";
	strLFullFileName+=pLList->PPI[LPanel.PInfo.CurrentItem].FileName;
	strRFullFileName=pRList->Dir;
	if (strRFullFileName.length()>0 && strRFullFileName[(size_t)(strRFullFileName.length()-1)]!=L'\\') strRFullFileName+=L"\\";
	strRFullFileName+=pRList->PPI[RPanel.PInfo.CurrentItem].FileName;

	PicExts=NULL;
	PicExtsNum=0;
	if (bGflLoaded)
		SetPicDefaultExtentions();

	LPicData.FileName=strLFullFileName.get();
	LPicData.DrawRect.left=1;
	LPicData.DrawRect.top=1;
	LPicData.DrawRect.right=WinInfo.Con.Right/2-1;
	LPicData.DrawRect.bottom=WinInfo.Con.Bottom-WinInfo.Con.Top-2-2;
	LPicData.FirstRun=true;
	LPicData.Redraw=false;
	LPicData.Loaded=false;
	BITMAPINFOHEADER BmpHeader1;
	LPicData.BmpHeader=&BmpHeader1;
	LPicData.DibData=NULL;
	GFL_FILE_INFORMATION pic_info1;
	LPicData.pic_info=&pic_info1;
	LPicData.Page=1;
	LPicData.Rotate=0;

	RPicData.FileName=strRFullFileName.get();
	RPicData.DrawRect.left=WinInfo.Con.Right/2+2;
	RPicData.DrawRect.top=1;
	RPicData.DrawRect.right=WinInfo.Con.Right-1;
	RPicData.DrawRect.bottom=WinInfo.Con.Bottom-WinInfo.Con.Top-2-2;
	RPicData.FirstRun=true;
	RPicData.Redraw=false;
	RPicData.Loaded=false;
	BITMAPINFOHEADER BmpHeader2;
	RPicData.BmpHeader=&BmpHeader2;
	RPicData.DibData=NULL;
	GFL_FILE_INFORMATION pic_info2;
	RPicData.pic_info=&pic_info2;
	RPicData.Page=1;
	RPicData.Rotate=0;

	struct FarMenuItem MenuItems[3];
	memset(MenuItems,0,sizeof(MenuItems));
	MenuItems[0].Text=L"Texts (VisualCompare)";
	MenuItems[1].Text=L"Texts (WinMerge)";
	MenuItems[2].Text=L"Pictures";
	MenuItems[0].Flags|=MIF_SELECTED;
	int MenuCode=Info.Menu(&MainGuid,-1,-1,0,FMENU_AUTOHIGHLIGHT|FMENU_WRAPMODE,L"Method",NULL,L"Contents",NULL,NULL,MenuItems,sizeof(MenuItems)/sizeof(MenuItems[0]));

	WIN32_FIND_DATA wfdFindData;
	HANDLE hFind;
	wchar_t DiffProgram[MAX_PATH];
	ExpandEnvironmentStringsW(L"%ProgramFiles%",DiffProgram,(sizeof(DiffProgram)/sizeof(DiffProgram[0])));
	wcscat(DiffProgram,L"\\WinMerge\\WinMergeU.exe");

	if (MenuCode==0 && pCompareFiles)
		pCompareFiles(strLFullFileName.get(),strRFullFileName.get(),0);
	else if (MenuCode==1 && ((hFind=FindFirstFileW(DiffProgram,&wfdFindData)) != INVALID_HANDLE_VALUE))
	{
		FindClose(hFind);
		STARTUPINFO si;
		PROCESS_INFORMATION pi;
		memset(&si, 0, sizeof(si));
		si.cb = sizeof(si);
		wchar_t Command[32768];
		FSF.sprintf(Command, L"\"%s\" -e \"%s\" \"%s\"", DiffProgram,strLFullFileName.get()+4,strRFullFileName.get()+4);
		CreateProcess(0,Command,0,0,false,0,0,0,&si,&pi);
	}
	else if (MenuCode==2 && bGflLoaded && CheckName(strLFullFileName.get()) && CheckName(strRFullFileName.get()))
		ShowCmpCurDialog(&pLList->PPI[LPanel.PInfo.CurrentItem],&pRList->PPI[RPanel.PInfo.CurrentItem]);

	if (PicExts)
	{
		for (int i=0;i<PicExtsNum;i++)
			if (PicExts[i]) free(PicExts[i]);
		free(PicExts); PicExts=NULL;
		PicExtsNum=0;
	}

	return true;
}