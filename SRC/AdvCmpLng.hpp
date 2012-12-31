/****************************************************************************
 * AdvCmpLng.hpp
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

/****************************************************************************
 * Константы для извлечения строк из .lng файла
 ****************************************************************************/
enum {
	MCmpTitle = 0,

	MOK,
	MCancel,

	/**** основной диалог ****/

	MMode,
	// ---
	MModeCmp,
	MModeSync,
	MModeDup,
	// ---
	MCmpCase,
	MCmpSize,
	MCmpTime,
	MCmpSeconds,
	MCmpIgnoreSeconds,
	MCmpLowPrecision,
	MCmpIgnoreTimeZone,
	MCmpContents,
	MCmpOnlyTimeDiff,
	MCmpCache,
	MCmpCacheIgnore,
	MCmpCacheUse,
	MCmpCacheClearBotton,
	MCmpPartly,
	MCmpPartlyFull,
	MCmpPartlyKb,
	MCmpIgnore,
	// ---
	MCmpIgnoreAllWhitespace,
	MCmpIgnoreNewLines,
	MCmpIgnoreWhitespace,
	// ---
	MDupName,
	MDupSize,
	MDupContents,
	MDupPic,
	//---
	MDupPicDiffNone,
	MDupPicDiffSmall,
	MDupPicDiffNormal,
	MDupPicDiffBig,
	MDupPicDiffVeryBig,
	//---
	MDupPicSize,
	MDupPicFmt,
	MDupMusic,
	MDupMusicArtist,
	MDupMusicTitle,
	MDupMusicDuration,

	MTitleOptions,
	MSubfolders,
	MMaxScanDepth,
	MScanSymlink,
	MSkipSubstr,
	MStopDiffDup,
	MFilter,
	MFilterBotton,
	MProcessSelected,
	MSelectedNew,
	MSyncOnlyRight,
	MIgnoreMissing,
	MLightSync,
	MShowMsg,
	MSound,
	MDialog,
	MTotalProgress,
	MUnderCursorBotton,

	/**** процесс ****/

	MComparingFiles,
	MComparingFiles2,
	MComparing,
	MComparingWith,
	MComparingDiffN,
	MComparingN,
	MWait,

	MDupCurProc,

	/**** сообщения ****/

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
	MListBottomCurDir,

	MCurDirName,
	MLName,
	MRName,

	MSyncTitle,
	MNoSyncBody,

	MSyncLPanel,
	MSyncRPanel,
	MSyncDel,
	MSyncUseDelFilter,
	MSyncEdit,

	MSyncSelTitle,
	MSyncSelDiff,
	MSyncSelLNew,
	MSyncSelRNew,
	MSyncSelToSkip,
	MSyncSelToRight,
	MSyncSelToLeft,
	MSyncSelIfNew,
	MSyncSelToNon,
	MSyncSelToDel,

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
	MFailedDelFileRecycleBin,
	MFailedDeleteFolder,

	MDelFolder,
	MDelFolderRecycleBin,
	MDelFile,
	MDelFileRecycleBin,
	MAskDel,
	MDelete,

	MMethod,
	MDefault,
	MWinMerge,
	MVisCmp,
	MPictures,

	MDupListBottom,

	MDupTitle,
	MNoDupBody,

	MDupDel,
	MDupDelRecycleBin,
	MDupEdit,

	MDupDelItems,

	MDelLeft,
	MDelRight,
};

