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
	MListBottomCurDir,

	MCurDirName,
	MLName,
	MRName,

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

	MDefault,
	MWinMerge,
	MVisCmp,
	MPictures,
};

