/****************************************************************************
 * AdvCmpProc.hpp
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

// элемент для показа в диалоге результатов
struct File
{
	wchar_t *FileName;
	wchar_t *LDir;
	wchar_t *RDir;
	DWORD dwLAttributes;
	DWORD dwRAttributes;
	unsigned __int64 nLFileSize;
	unsigned __int64 nRFileSize;
	FILETIME ftLLastWriteTime;
	FILETIME ftRLastWriteTime;
	DWORD dwFlags;

	File()
	{
		FileName=NULL;
		LDir=NULL;
		RDir=NULL;
		dwLAttributes=0;
		dwRAttributes=0;
		nLFileSize=0;
		nRFileSize=0;
		ftLLastWriteTime.dwLowDateTime=0;
		ftLLastWriteTime.dwHighDateTime=0;
		ftRLastWriteTime.dwLowDateTime=0;
		ftRLastWriteTime.dwHighDateTime=0;
		dwFlags=RCIF_NONE;
	}
};

// массив элементов, для диалога с результатами сравнения
struct FileList {
	File *F;
	int iCount;
	// для строки статуса:
	int Items;
	int Select;
	int Identical;
	int Different;
	int LNew;
	int RNew;
	bool bShowSelect;
	bool bShowIdentical;
	bool bShowDifferent;
	bool bShowLNew;
	bool bShowRNew;
	bool bClearUserFlags;
};


// сама сравнивалка :)
class AdvCmpProc
{
		HANDLE hScreen;
		bool bStartMsg;    // старт сообщения? для нужд ShowMessage()

		bool TitleSaved;
		string strFarTitle;

		// отсортированный массив указателей на элементы DirList.PPI
		struct ItemsIndex {
			PluginPanelItem **pPPI; // элементы
			int iCount;             // кол-во
		};

		// массив элементов, для диалога с результатами сравнения
		struct FileList FList;
		// для синхронизации
		bool bAskLOverwrite;
		bool bAskROverwrite;
		bool bAskLReadOnly;
		bool bAskRReadOnly;
		bool bSkipLReadOnly;
		bool bSkipRReadOnly;

	private:
		bool GetFarTitle(string &strTitle);
		void ShowCmpMsg(const wchar_t *Dir1, const wchar_t *Name1, const wchar_t *Dir2, const wchar_t *Name2, bool bRedraw);

		void WFD2PPI(WIN32_FIND_DATA &wfd, PluginPanelItem &ppi);
		int GetDirList(const wchar_t *Dir, int ScanDepth, bool OnlyInfo, struct DirList *pList=0);
		void FreeDirList(struct DirList *pList);

		inline bool IsNewLine(int c) {return (c == '\r' || c == '\n');}
		inline bool myIsSpace(int c) {return (c == ' ' || c == '\t' || c == '\v' || c == '\f');}
		inline bool IsWhiteSpace(int c) {return (c == ' ' || c == '\t' || c == '\v' || c == '\f' || c == '\r' || c == '\n');}

		bool mySetFilePointer(HANDLE hf, __int64 distance, DWORD MoveMethod);
		DWORD ProcessCRC(void *pData, register int iLen, DWORD FileCRC);
		int GetCacheResult(DWORD FullFileName1, DWORD FullFileName2, DWORD64 WriteTime1, DWORD64 WriteTime2);
		bool SetCacheResult(DWORD FullFileName1, DWORD FullFileName2, DWORD64 WriteTime1, DWORD64 WriteTime2, DWORD dwFlag);
		bool CompareFiles(const wchar_t *LDir, const PluginPanelItem *pLPPI, const wchar_t *RDir, const PluginPanelItem *pRPPI, int ScanDepth);

		bool CheckScanDepth(const wchar_t *FileName, int ScanDepth);
		bool BuildItemsIndex(bool bLeftPanel,const struct DirList *pList,struct ItemsIndex *pIndex,int ScanDepth);
		void FreeItemsIndex(struct ItemsIndex *pIndex);
		bool BuildFileList(const wchar_t *LDir,const PluginPanelItem *pLPPI,const wchar_t *RDir,const PluginPanelItem *pRPPI,DWORD dwFlag);

		int QueryOverwriteFile(const wchar_t *FileName, FILETIME *srcTime, FILETIME *destTime, __int64 srcSize, __int64 destSize, int direction, bool bReadOnlyType);
		int FileExists(const wchar_t *FileName, __int64 *pSize, FILETIME *pTime, DWORD *pAttrib);
		int SyncFile(const wchar_t *srcFileName, const wchar_t *destFileName, int direction);
		int SyncDir(const wchar_t *srcDirName, const wchar_t *destDirName, int direction);
		int Synchronize(FileList *pFileList);

	public:
		AdvCmpProc();
		~AdvCmpProc();

		bool CompareDirs(const struct DirList *pLList,const struct DirList *pRList,bool bCompareAll,int ScanDepth);
		int ShowCmpDialog(const struct DirList *pLList,const struct DirList *pRList);
};
