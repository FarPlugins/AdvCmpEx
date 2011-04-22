/****************************************************************************
 * AdvCmpProcCur.hpp
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

#pragma once
#include "AdvCmp.hpp"

class AdvCmpProcCur
{
		string strLFullFileName;
		string strRFullFileName;

		wchar_t **PicExts;
		DWORD PicExtsNum;
		bool bImage;

		struct PicData {
			wchar_t *FileName;
			RECT DrawRect;  //символы
			RECT GDIRect;   //точки
			bool Redraw;
			bool Loaded;
			bool FirstRun;
			BITMAPINFOHEADER *BmpHeader;
			unsigned char *DibData;
			GFL_FILE_INFORMATION *pic_info;
			int Page;
			int Rotate;
		} LPicData, RPicData;

	private:
		void SetPicDefaultExtentions();
		bool CheckName(const wchar_t *FileName);
		void GetDIBFromBitmap(GFL_BITMAP *bitmap,BITMAPINFOHEADER *bitmap_info,unsigned char **data);
		RECT RangingPic(RECT DCRect,GFL_BITMAP *RawPicture);
		bool DrawImage(PicData *pData);
		bool UpdateImage(PicData *pData,bool CheckOnly=false);
		void FreeImage(PicData *pData);
		void UpdateInfoText(HANDLE hDlg, PicData *data, bool left);

		static INT_PTR WINAPI ShowCmpCurDialogProcThunk(HANDLE hDlg,int Msg,int Param1,void *Param2);
		INT_PTR WINAPI ShowCmpCurDialogProc(HANDLE hDlg,int Msg,int Param1,void *Param2);
		int ShowCmpCurDialog(const PluginPanelItem *pLPPI,const PluginPanelItem *pRPPI);

	public:
		AdvCmpProcCur() { }
		~AdvCmpProcCur() { }

		bool CompareCurFile(const struct DirList *pLList,const struct DirList *pRList);
};
