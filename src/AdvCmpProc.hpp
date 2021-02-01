/****************************************************************************
 * AdvCmpProc.hpp
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

#pragma once
#include "AdvCmp.hpp"

// коды возврата для сообщений
enum QueryResult { QR_ABORT = -1, QR_OVERWRITE = 0, QR_DELETE = 0, QR_EDIT = 0, QR_ALL = 1, QR_SKIP = 2, QR_SKIPALL = 3 };

struct FileInfo
{
  wchar_t* FileName;
  wchar_t* Dir;
  DWORD dwAttributes;
  unsigned __int64 nFileSize;
  FILETIME ftLastWriteTime;

  FileInfo()
  {
    FileName = nullptr;
    Dir = nullptr;
    dwAttributes = 0;
    nFileSize = 0;
    ftLastWriteTime.dwLowDateTime = 0;
    ftLastWriteTime.dwHighDateTime = 0;
  }
};

// элемент для показа в диалоге результатов
struct cmpFile
{
  FileInfo L, R;
  DWORD dwFlags;
};

// массив элементов, для диалога с результатами сравнения
struct cmpFileList
{
  cmpFile* F;
  int iCount;
  // для строки статуса:
  int Items;
  int Select;
  int Identical;
  int Different;
  int LNew;
  int RNew;

  class AdvCmpProc* AdvCmp;
};

const int PIXELS_SIZE = 32;

// элемент для показа в диалоге результатов дублей
struct dupFile
{
  FileInfo fi;
  unsigned int nDupGroup;
  DWORD dwCRC;
  DWORD dwFlags;

  int PicWidth;
  int PicHeight;
  unsigned char* PicPix;  // массив выборочных пикселей

  wchar_t* MusicArtist;
  wchar_t* MusicTitle;
  DWORD MusicBitrate;
  DWORD MusicTime;  // продолжительность

  dupFile()
  {
    nDupGroup = 0;
    dwCRC = 0;
    dwFlags = RCIF_NONE;

    PicWidth = PicHeight = 0;
    PicPix = nullptr;

    MusicArtist = nullptr;
    MusicTitle = nullptr;
    MusicBitrate = 0;
    MusicTime = 0;
  }
};

// массив элементов - список дубликатов
struct dupFileList
{
  dupFile* F;
  int iCount;
  // для строки статуса
  int GroupItems;
  int Del;
};

// для показа рисунков
struct PicData
{
  wchar_t* FileName;
  RECT DrawRect;  //символы
  RECT GDIRect;   //точки
  bool Redraw;
  bool Loaded;
  bool FirstRun;
  BITMAPINFOHEADER* BmpHeader;
  unsigned char* DibData;
  GFL_FILE_INFORMATION* pic_info;
  int Page;
  int Rotate;
  DWORD MemSize;
};

struct cmpPicFile
{
  struct PicData L;
  struct PicData R;
};

// сама сравнивалка :)
class AdvCmpProc
{
  HANDLE hScreen;

  // отсортированный массив указателей на элементы DirList.PPI
  struct ItemsIndex
  {
    PluginPanelItem** pPPI;  // элементы
    int iCount;              // кол-во
  };

  // массив элементов, для диалога с результатами сравнения
  struct cmpFileList cFList;
  // массив дубликатов
  struct dupFileList dFList;
  // два рисунка
  struct cmpPicFile CmpPic;
  // для синхронизации
  bool bAskLOverwrite;
  bool bAskROverwrite;
  bool bAskLReadOnly;
  bool bAskRReadOnly;
  bool bAskDel;
  bool bSkipLReadOnly;
  bool bSkipRReadOnly;

 public:
  bool TitleSaved;
  string strFarTitle;

 private:
  // полезняшки
  bool GetFarTitle(string& strTitle);
  void WFD2PPI(WIN32_FIND_DATA& wfd, PluginPanelItem& ppi);
  static inline bool IsNewLine(int c)
  {
    return (c == '\r' || c == '\n');
  }
  static inline bool myIsSpace(int c)
  {
    return (c == ' ' || c == '\t' || c == '\v' || c == '\f');
  }
  static inline bool IsWhiteSpace(int c)
  {
    return (c == ' ' || c == '\t' || c == '\v' || c == '\f' || c == '\r' || c == '\n');
  }
  bool mySetFilePointer(HANDLE hf, unsigned __int64 distance, DWORD MoveMethod);
  DWORD ProcessCRC(void* pData, register int iLen, DWORD FileCRC);
  bool CheckScanDepth(const wchar_t* FileName, int ScanDepth);

  int GetDirList(const wchar_t* Dir, int ScanDepth, bool OnlyInfo, struct DirList* pList = nullptr);
  void FreeDirList(struct DirList* pList);
  bool BuildItemsIndex(bool bLeftPanel, const struct DirList* pList, struct ItemsIndex* pIndex, int ScanDepth);
  void FreeItemsIndex(struct ItemsIndex* pIndex);
  int GetCacheResult(DWORD FullFileName1, DWORD FullFileName2, DWORD64 WriteTime1, DWORD64 WriteTime2);
  bool SetCacheResult(DWORD FullFileName1, DWORD FullFileName2, DWORD64 WriteTime1, DWORD64 WriteTime2, DWORD dwFlag);
  void ShowCmpMsg(const wchar_t* Dir1, const wchar_t* Name1, const wchar_t* Dir2, const wchar_t* Name2, bool bRedraw);
  bool CompareFiles(const wchar_t* LDir, const PluginPanelItem* pLPPI, const wchar_t* RDir, const PluginPanelItem* pRPPI, int ScanDepth,
                    DWORD* dwFlag);
#if 0
		int ShowCmpCurDialog(const PluginPanelItem *pLPPI,const PluginPanelItem *pRPPI);
#endif
  // диалог результатов сравнения
  bool MakeFileList(const wchar_t* LDir, const PluginPanelItem* pLPPI, const wchar_t* RDir, const PluginPanelItem* pRPPI, DWORD dwFlag);
  // синхронизация
  int QueryOverwriteFile(const wchar_t* FileName, FILETIME* srcTime, FILETIME* destTime, unsigned __int64 srcSize, unsigned __int64 destSize,
                         int direction, bool bReadOnlyType);
  int QueryDelete(const wchar_t* FileName, bool bIsDir, bool bReadOnlyType);
  int FileExists(const wchar_t* FileName, WIN32_FIND_DATA& FindData, int CheckForFilter);
  int SyncFile(const wchar_t* srcFileName, const wchar_t* destFileName, int direction, DWORD dwFlag = 0);
  int DelFile(const wchar_t* FileName);
  int SyncDir(const wchar_t* srcDirName, const wchar_t* destDirName, int direction);
  int DelDir(const wchar_t* DirName);
  int Synchronize();
  // дубликаты
  void ShowDupMsg(const wchar_t* Dir, const wchar_t* Name, bool bRedraw);
  int ScanDir(const wchar_t* DirName, int ScanDepth);
  DWORD GetCRC(const dupFile* cur);
  int GetPic(dupFile* cur, int GetInfo);
  int GetMp3(dupFile* cur, int GetInfo);
  bool MakeFileList(const wchar_t* Dir, const PluginPanelItem* pPPI);
  int ShowDupDialog();

 public:
  AdvCmpProc() = default;
  ~AdvCmpProc() = default;

  void Init();
  void Close();

  bool CompareDirs(const struct DirList* pLList, const struct DirList* pRList, bool bCompareAll, int ScanDepth);
  bool CompareCurFile(const wchar_t* LDir, const wchar_t* LFileName, const wchar_t* RDir, const wchar_t* RFileName, int Method);
  int ShowCmpDialog(const struct DirList* pLList, const struct DirList* pRList);
  int Duplicate(const struct DirList* pList);
};

// диалог результатов сравнения
void MakeListItemText(wchar_t* buf, cmpFile* cur, wchar_t Mark);
void SetBottom(HANDLE hDlg, cmpFileList* pFileList, wchar_t* CurDir = nullptr);
bool MakeCmpFarList(HANDLE hDlg, cmpFileList* pFileList, bool bSetCurPos = true, bool bSort = false);
intptr_t WINAPI ShowCmpDialogProc(HANDLE hDlg, intptr_t Msg, intptr_t Param1, void* Param2);

// синхронизация
int GetSyncOpt(cmpFileList* pFileList);
void ShowSyncMsg(const wchar_t* Name1, const wchar_t* Name2, unsigned __int64 Progress, unsigned __int64 Max, bool bRedraw);

// полезняшки
wchar_t* CutSubstr(string& strSrc, wchar_t* Substr);
void strcentr(wchar_t* Dest, const wchar_t* Src, int len, wchar_t sym);
wchar_t* itoaa(__int64 num, wchar_t* buf);
void ProgressLine(wchar_t* Dest, unsigned __int64 nCurrent, unsigned __int64 nTotal);
wchar_t* GetPosToName(const wchar_t* FileName);
void GetFullFileName(string& strFullFileName, const wchar_t* Dir, const wchar_t* FileName, bool bNative = true);
wchar_t* GetStrFileTime(FILETIME* LastWriteTime, wchar_t* Time, bool FullYear = true);
bool CheckForEsc();
void TruncCopy(wchar_t* Dest, const wchar_t* Src, int TruncLen, const wchar_t* FormatMsg = nullptr);
int GetArgv(const wchar_t* cmd, wchar_t*** argv);
