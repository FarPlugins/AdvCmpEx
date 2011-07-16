/****************************************************************************
 * AdvCmpLng.hpp
 *
 * Plugin module for Far Manager 3.0
 *
 * Copyright (c) 2006-2011 Alexey Samlyukov
 ****************************************************************************/

/****************************************************************************
 * ��������� ��� ���������� ����� �� .lng �����
 ****************************************************************************/
enum {
	MCompareTitle = 0,

	MOK,
	MCancel,

	/**** �������� ������ ****/

	MMode,
	MCompare,
	MSynchronize,
	MCompareCase,
	MCompareSize,
	MCompareTime,
	MCompareSeconds,
	MCompareIgnoreSeconds,
	MCompareLowPrecision,
	MCompareIgnoreTimeZone,
	MCompareContents,
	MCompareOnlyTimeDiff,
	MCache,
	MCacheIgnore,
	MCacheUse,
	MCacheClearBotton,
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
	MScanSymlink,
	MFilter,
	MFilterBotton,
	MProcessTillFirstDiff,
	MCompareSkipSubstr,
	MProcessSelected,
	MSelectedNew,
	MSyncOnlyRight,
	MIgnoreMissing,
	MShowMsg,
	MSound,
	MDialog,
	MTotalProcess,
	MUnderCursorBotton,

	/**** ������� ****/

	MComparingFiles,
	MComparingFiles2,
	MComparing,
	MComparingWith,
	MComparingDiffN,
	MComparingN,
	MWait,

	/**** ��������� ****/

	MNoDiffTitle,
	MNoDiffBody,

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

	MFolder,

	MListBottom,

	MSyncTitle,
	MNoSyncBody,

	MSyncLPanel,
	MSyncRPanel,
	MSyncRDel,
	MSyncUseDelFilter,
	MSyncEdit,

	MWarning,
	MFileIsReadOnly,
	MFileAlreadyExists,
	MNewToL,
	MNewToR,
	MExisting,
	MOverwrite,
	MAll,
	MSkip,
	MSkipAll,
	MRetry,

	MCopying,
	MCopyingTo,

	MCantCreateFolder,

	MFailedCopySrcFile,
	MFailedOpenSrcFile,
	MFailedCreateDstFile,
	MFailedOpenDstFile,
	MFailedCopyDstFile,

	MFailedDelFile,
	MFailedDeleteFolder,

	MDelFolder,
	MDelFile,
	MAskDel,
	MDelete,
};

