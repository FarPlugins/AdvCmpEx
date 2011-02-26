/****************************************************************************
 * AdvCmpLng.hpp
 *
 * Plugin module for Far Manager 3.0
 *
 * Copyright (c) 2006-2011 Alexey Samlyukov
 ****************************************************************************/

/****************************************************************************
 * Константы для извлечения строк из .lng файла
 ****************************************************************************/
enum {
	MCompareTitle = 0,

	MOK,
	MCancel,

	/**** основной диалог ****/

	MCompareBox,
	MCompareCase,
	MCompareSize,
	MCompareTime,
	MCompareLowPrecision,
	MCompareIgnoreTimeZone,
	MCompareContents,
	MCompareOnlyTimeDiff,
	MComparePartly,
	MComparePartlyFull,
	MComparePartlyKb,
	MCompareIgnore,
	// ---
	MCompareIgnoreAllWhitespace,
	MCompareIgnoreNewLines,
	MCompareIgnoreWhitespace,
	// ---
	MTitleOptions,
	MProcessSubfolders,
	MMaxScanDepth,
	MFilter,
	MFilterBotton,
	MProcessSelected,
	MCompareSkipSubstr,
	MIgnoreMissing,
	MProcessTillFirstDiff,
	MSelectedNew,
	MCache,
	MCacheIgnore,
	MCacheUse,
	MCacheClearBotton,
	MShowMsg,
	MSound,
	MTotalProcess,
	MPanel,
	MUnderCursorBotton,

	/**** процесс ****/

	MComparingFiles,
	MComparingFiles2,
	MComparing,
	MComparingWith,
	MComparingDiffN,
	MComparingN,
	MWait,

	/**** панель ****/

	MPanelTitle,
	MName,
	MSize,
	MTime,

	/**** сообщения ****/

	MNoDiffTitle,
	MNoDiffBody,

	MUnderCursor,
	MNoDiff,
	MDiff,
	MRunDiffProgram,

	MCmpPathTitle,
	MCmpPathBody,

	MFirstDiffTitle,
	MFirstDiffBody,

	MFilePanelsRequired,

	MNoMemTitle,
	MNoMemBody,

	MClearCacheTitle,
	MClearCacheItems,
	MClearCacheBody,

	MEscTitle,
	MEscBody,

	MOpenErrorTitle,
	MOpenErrorBody,

	MFolder
};
