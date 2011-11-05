/****************************************************************************
 * AdvCmp.hpp
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

#include <wchar.h>
#include <initguid.h>
#include "plugin.hpp"
#include "farcolor.hpp"
#include "string.hpp"
#include "libgfl.h"
#include "AdvCmpLng.hpp"        // набор констант для извлечения строк из .lng файла


/// ВАЖНО! используем данные функции, чтоб дополнительно не обнулять память
#define malloc(size) HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,size)
#define free(ptr) ((ptr)?HeapFree(GetProcessHeap(),0,ptr):0)
#define realloc(ptr,size) ((size)?((ptr)?HeapReAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,ptr,size):HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,size)):(HeapFree(GetProcessHeap(),0,ptr),(void *)0))
#ifdef __cplusplus
inline void * __cdecl operator new(size_t size) { return malloc(size); }
inline void __cdecl operator delete(void *block) { free(block); }
#endif

/// Подмена strncmp() (или strcmp() при n=-1)
inline int __cdecl Strncmp(const wchar_t *s1, const wchar_t *s2, int n=-1) { return CompareString(0,SORT_STRINGSORT,s1,n,s2,n)-2; }

///
#define ControlKeyAllMask (RIGHT_ALT_PRESSED|LEFT_ALT_PRESSED|RIGHT_CTRL_PRESSED|LEFT_CTRL_PRESSED|SHIFT_PRESSED)
#define ControlKeyAltMask (RIGHT_ALT_PRESSED|LEFT_ALT_PRESSED)
#define ControlKeyNonAltMask (RIGHT_CTRL_PRESSED|LEFT_CTRL_PRESSED|SHIFT_PRESSED)
#define ControlKeyCtrlMask (RIGHT_CTRL_PRESSED|LEFT_CTRL_PRESSED)
#define ControlKeyNonCtrlMask (RIGHT_ALT_PRESSED|LEFT_ALT_PRESSED|SHIFT_PRESSED)
#define IsShift(rec) (((rec)->Event.KeyEvent.dwControlKeyState&ControlKeyAllMask)==SHIFT_PRESSED)
#define IsAlt(rec) (((rec)->Event.KeyEvent.dwControlKeyState&ControlKeyAltMask)&&!((rec)->Event.KeyEvent.dwControlKeyState&ControlKeyNonAltMask))
#define IsCtrl(rec) (((rec)->Event.KeyEvent.dwControlKeyState&ControlKeyCtrlMask)&&!((rec)->Event.KeyEvent.dwControlKeyState&ControlKeyNonCtrlMask))
#define IsNone(rec) (((rec)->Event.KeyEvent.dwControlKeyState&ControlKeyAllMask)==0)


/****************************************************************************
 * GUID
 ****************************************************************************/

// {ED0C4BD8-D2F0-4b6e-A19F-B0B0137C9B0C}
DEFINE_GUID(MainGuid,0xed0c4bd8, 0xd2f0, 0x4b6e, 0xa1, 0x9f, 0xb0, 0xb0, 0x13, 0x7c, 0x9b, 0xc);
// {CF6DF7E7-060C-4dd7-80D8-69E20F96EA38}
DEFINE_GUID(MenuGuid,0xcf6df7e7, 0x60c, 0x4dd7, 0x80, 0xd8, 0x69, 0xe2, 0xf, 0x96, 0xea, 0x38);
// {81184FEB-5A2C-4e7d-8C3F-D266B00E3CF6}
DEFINE_GUID(OptDlgGuid,0x81184feb, 0x5a2c, 0x4e7d, 0x8c, 0x3f, 0xd2, 0x66, 0xb0, 0xe, 0x3c, 0xf6);
// {50E6209D-3D39-42ad-B64A-CA960927A249}
DEFINE_GUID(CmpDlgGuid,0x50e6209d, 0x3d39, 0x42ad, 0xb6, 0x4a, 0xca, 0x96, 0x9, 0x27, 0xa2, 0x49);
// {8FC5FB22-E223-41ef-9EB6-A77D9302D462}
DEFINE_GUID(CurDlgGuid,0x8fc5fb22, 0xe223, 0x41ef, 0x9e, 0xb6, 0xa7, 0x7d, 0x93, 0x2, 0xd4, 0x62);
// {543FD7B0-0B51-4F81-819A-449C503EEEC2}
DEFINE_GUID(OptSyncDlgGuid,0x543fd7b0, 0xb51, 0x4f81, 0x81, 0x9a, 0x44, 0x9c, 0x50, 0x3e, 0xee, 0xc2);
// {965FC640-C950-47AC-A17C-9D05129D3C1C}
DEFINE_GUID(ErrorMsgGuid,0x965fc640, 0xc950, 0x47ac, 0xa1, 0x7c, 0x9d, 0x5, 0x12, 0x9d, 0x3c, 0x1c);
// {363D47D7-F723-48CF-9A31-7F5EB4475362}
DEFINE_GUID(YesNoMsgGuid,0x363d47d7, 0xf723, 0x48cf, 0x9a, 0x31, 0x7f, 0x5e, 0xb4, 0x47, 0x53, 0x62);
// {E2C773EB-F711-4F70-B551-92833045A30A}
DEFINE_GUID(DebugMsgGuid,0xe2c773eb, 0xf711, 0x4f70, 0xb5, 0x51, 0x92, 0x83, 0x30, 0x45, 0xa3, 0xa);
// {000A11B8-83DA-40B5-BE76-8B3E4975F3F3}
DEFINE_GUID(NoDiffMsgGuid,0xa11b8, 0x83da, 0x40b5, 0xbe, 0x76, 0x8b, 0x3e, 0x49, 0x75, 0xf3, 0xf3);
// {7E5BD905-1CA7-41E1-89D0-367043E94544}
DEFINE_GUID(ClearCacheMsgGuid,0x7e5bd905, 0x1ca7, 0x41e1, 0x89, 0xd0, 0x36, 0x70, 0x43, 0xe9, 0x45, 0x44);
// {D51A604E-593A-418A-996E-B3BC08EF0FD4}
DEFINE_GUID(CmpMsgGuid,0xd51a604e, 0x593a, 0x418a, 0x99, 0x6e, 0xb3, 0xbc, 0x8, 0xef, 0xf, 0xd4);
// {1D64872F-93E0-498E-AAE6-73E94B297864}
DEFINE_GUID(NoSyncMsgGuid,0x1d64872f, 0x93e0, 0x498e, 0xaa, 0xe6, 0x73, 0xe9, 0x4b, 0x29, 0x78, 0x64);
// {89404D9E-CD7C-4F7F-A02F-A74BDC34095A}
DEFINE_GUID(QueryOverwriteMsgGuid,0x89404d9e, 0xcd7c, 0x4f7f, 0xa0, 0x2f, 0xa7, 0x4b, 0xdc, 0x34, 0x9, 0x5a);
// {EB98752F-E7C4-41D8-AD86-75692151638E}
DEFINE_GUID(QueryDelMsgGuid,0xeb98752f, 0xe7c4, 0x41d8, 0xad, 0x86, 0x75, 0x69, 0x21, 0x51, 0x63, 0x8e);
// {1AB836F3-6252-4F95-B8EB-A38DD8063678}
DEFINE_GUID(SyncMsgGuid,0x1ab836f3, 0x6252, 0x4f95, 0xb8, 0xeb, 0xa3, 0x8d, 0xd8, 0x6, 0x36, 0x78);
// {124B2C5E-C9E8-4E99-AFB5-B93D8C9509B3}
DEFINE_GUID(FailedCopyMsgGuid,0x124b2c5e, 0xc9e8, 0x4e99, 0xaf, 0xb5, 0xb9, 0x3d, 0x8c, 0x95, 0x9, 0xb3);
// {961E4702-C8A1-49F2-8F96-36F2DCD91431}
DEFINE_GUID(FailedDelFileMsgGuid,0x961e4702, 0xc8a1, 0x49f2, 0x8f, 0x96, 0x36, 0xf2, 0xdc, 0xd9, 0x14, 0x31);
// {12C3FDA1-D6C4-4168-8522-D1C02D131395}
DEFINE_GUID(CantCreateFolderMsgGuid,0x12c3fda1, 0xd6c4, 0x4168, 0x85, 0x22, 0xd1, 0xc0, 0x2d, 0x13, 0x13, 0x95);
// {21364334-8293-42C8-922D-24CE103BF65F}
DEFINE_GUID(FailedDelFolderMsgGuid,0x21364334, 0x8293, 0x42c8, 0x92, 0x2d, 0x24, 0xce, 0x10, 0x3b, 0xf6, 0x5f);
// {89258BF3-3651-4791-91DF-E407D063F3C8}
DEFINE_GUID(CmpMethodMenuGuid,0x89258bf3, 0x3651, 0x4791, 0x91, 0xdf, 0xe4, 0x7, 0xd0, 0x63, 0xf3, 0xc8);
// {F36D329D-0C58-4015-96AF-9BC33D5309CE}
DEFINE_GUID(CompareCurFileMsgGuid,0xf36d329d, 0xc58, 0x4015, 0x96, 0xaf, 0x9b, 0xc3, 0x3d, 0x53, 0x9, 0xce);
// {30F9985D-EEB9-4588-B4DB-349E920B7513}
DEFINE_GUID(DlgNameGuid,0x30f9985d, 0xeeb9, 0x4588, 0xb4, 0xdb, 0x34, 0x9e, 0x92, 0xb, 0x75, 0x13);


/****************************************************************************
 * Копии стандартных структур FAR
 ****************************************************************************/
extern struct PluginStartupInfo Info;
extern struct FarStandardFunctions FSF;


/****************************************************************************
 * Текущие настройки плагина
 ****************************************************************************/
extern struct Options {
	int CmpCase,
			CmpSize,
			CmpTime,
			Seconds,
			LowPrecisionTime,
			IgnoreTimeZone,
			CmpContents,
			OnlyTimeDiff,
			Partly,
			PartlyFull,
			PartlyKbSize,
			Ignore,
			IgnoreTemplates,
			ProcessSubfolders,
			MaxScanDepth,
			ScanSymlink,
			Filter,
			ProcessSelected,
			SkipSubstr,
			IgnoreMissing,
			ProcessTillFirstDiff,
			SelectedNew,
			Cache,
			CacheIgnore,
			ShowMsg,
			Sound,
			TotalProcess,
			Dialog,
			Sync,
			SyncOnlyRight,
			SyncLPanel,
			SyncRPanel,
			SyncRDel,
			SyncUseDelFilter,
			ProcessHidden,
			BufSize;
	char *Buf[2];
	wchar_t *Substr;
	HANDLE hCustomFilter;
} Opt;

extern HANDLE hConInp;     // хэндл консол. ввода
extern bool bBrokenByEsc;  //прекратить/прекратили операцию сравнения?
extern bool bGflLoaded;    // libgfl340.dll загружена?

// информация о панели
extern struct FarPanelInfo {
	struct PanelInfo PInfo;
	HANDLE hPanel;
	HANDLE hFilter;         // подцепим фильтр с панели
	bool bTMP;              // Tmp-панель? обрабатывается особо!
	bool bARC;              // панель-архив? обрабатывается особо!
	bool bCurFile;          // под курсором файл?
	bool bDir;              // на панели есть каталоги?
	wchar_t Dir[32768];
} LPanel, RPanel;

// информация об окне Фара
extern struct FarWindowsInfo {
	HWND hFarWindow;        // описатель окна фара
	SMALL_RECT Con;         // координаты консоли (символы - {0,0,79,24})
	RECT Win;               // координаты окна (точки - {0,0,1280,896})
	int TruncLen;           // оптимальная длина сообщения-сравнения
} WinInfo;

// информация
extern struct TotalCmpInfo {
	unsigned int Count;            // кол-во сравниваемых элементов
	unsigned __int64 CountSize;    // их размер
	unsigned __int64 CurCountSize; // размер сравниваемой пары
	unsigned int Proc;             // кол-во обработанных элементов
	unsigned __int64 ProcSize;     // размер обработанного
	unsigned __int64 CurProcSize;  // размер обработанного из сравниваемой пары
	unsigned int LDiff;            // кол-во отличающихся на левой панели
	unsigned int RDiff;            // кол-во отличающихся на правой панели
	unsigned int Errors;           // кол-во, не смогли открыть каталогов/файлов
} CmpInfo;

// элементы для сравнения
struct DirList {
	wchar_t *Dir;           // каталог
	PluginPanelItem *PPI;   // элементы
	int ItemsNumber;        // кол-во
};

// флаги результата сравнения
enum ResultCmpItemFlag {
	RCIF_NONE   = 0,
	RCIF_EQUAL  = 0x1,      // одинаковые   |=|
	RCIF_DIFFER = 0x2,      // разные       |?|
	RCIF_LNEW   = 0x4,      // слева новый  |>|
	RCIF_RNEW   = 0x8,      // справа новый |<|

	// пользовательские флаги, для итогового диалога:
	RCIF_USER       = 0x1F0,
	RCIF_USERSELECT = 0x010, // элемент выделен
	RCIF_USERNONE   = 0x020, // скинут флаг отличия
	RCIF_USERLNEW   = 0x040, // установлен флаг "слева новый"
	RCIF_USERRNEW   = 0x080, // установлен флаг "справа новый"
	RCIF_USERDEL    = 0x100, // удалить файл на правой панели

	// дополнительные флаги, для итогового диалога:
	RCIF_LUNIQ  = 0x400,    // слева уникальный
	RCIF_RUNIQ  = 0x800,    // справа уникальный
};


/****************************************************************************
 * Кеш сравнения "по содержимому"
 ****************************************************************************/
// результат сравнения 2-х элементов
struct ResultCmpItem {
	DWORD   dwFullFileName[2];
	DWORD64 dwWriteTime[2];
	DWORD   dwFlags;
};

// кеш
extern struct CacheCmp {
	ResultCmpItem *RCI;
	int ItemsNumber;
} Cache;

const wchar_t *GetMsg(int MsgId);
void ErrorMsg(DWORD Title, DWORD Body);
bool YesNoMsg(DWORD Title, DWORD Body);
int DebugMsg(wchar_t *msg, wchar_t *msg2 = L" ", unsigned int i = 1000);
wchar_t* itoaa(__int64 num, wchar_t *buf);
wchar_t *GetPosToName(const wchar_t *FileName);
void GetFullFileName(string &strFullFileName, const wchar_t *Dir, const wchar_t *FileName, bool bNative=true);



/****************************************************************************
 *  VisComp.dll
 ****************************************************************************/
typedef int (WINAPI *PCOMPAREFILES)(wchar_t *FileName1, wchar_t *FileName2, DWORD Options);

extern PCOMPAREFILES pCompareFiles;

/****************************************************************************
 *  libgfl340.dll
 ****************************************************************************/
typedef GFL_ERROR		(WINAPI *PGFLLIBRARYINIT)(void);
typedef void				(WINAPI *PGFLENABLELZW)(GFL_BOOL);
typedef void				(WINAPI *PGFLLIBRARYEXIT)(void);
typedef GFL_ERROR		(WINAPI *PGFLLOADBITMAPW)(const wchar_t* filename, GFL_BITMAP** bitmap, const GFL_LOAD_PARAMS* params, GFL_FILE_INFORMATION* info);
typedef GFL_INT32		(WINAPI *PGFLGETNUMBEROFFORMAT)(void);
typedef GFL_ERROR		(WINAPI *PGFLGETFORMATINFORMATIONBYINDEX)(GFL_INT32 index, GFL_FORMAT_INFORMATION* info); 
typedef void				(WINAPI *PGFLGETDEFAULTLOADPARAMS)(GFL_LOAD_PARAMS *);
typedef GFL_ERROR		(WINAPI *PGFLCHANGECOLORDEPTH)(GFL_BITMAP* src, GFL_BITMAP** dst, GFL_MODE mode, GFL_MODE_PARAMS params);
typedef GFL_ERROR		(WINAPI *PGFLROTATE)(GFL_BITMAP* src, GFL_BITMAP** dst, GFL_INT32 angle, const GFL_COLOR *background_color); 
typedef GFL_ERROR		(WINAPI *PGFLRESIZE)(GFL_BITMAP *, GFL_BITMAP **, GFL_INT32, GFL_INT32, GFL_UINT32, GFL_UINT32);
typedef void				(WINAPI *PGFLFREEBITMAP)(GFL_BITMAP *);
typedef void				(WINAPI *PGFLFREEFILEINFORMATION)(GFL_FILE_INFORMATION* info);

extern PGFLLIBRARYINIT pGflLibraryInit;
extern PGFLENABLELZW pGflEnableLZW;
extern PGFLLIBRARYEXIT pGflLibraryExit;
extern PGFLLOADBITMAPW pGflLoadBitmapW;
extern PGFLGETNUMBEROFFORMAT pGflGetNumberOfFormat;
extern PGFLGETFORMATINFORMATIONBYINDEX pGflGetFormatInformationByIndex;
extern PGFLGETDEFAULTLOADPARAMS pGflGetDefaultLoadParams;
extern PGFLCHANGECOLORDEPTH pGflChangeColorDepth;
extern PGFLROTATE pGflRotate;
extern PGFLRESIZE pGflResize;
extern PGFLFREEBITMAP pGflFreeBitmap;
extern PGFLFREEFILEINFORMATION pGflFreeFileInformation;
