//////////////////////////////////////////////////
// Torment 1.0.1114
//////////////////////////////////////////////////
//
// Copyright © 2003 Dark Knight Software
// Programmed by Jason Zimmer
//
// Compiled in Visual Studio .NET 2003
//
// You are allowed to make changes to this code
// so long as all source and modified source
// along with any additional data is made
// available for public use.
//
// This License must also appear and be unchanged
// in all of the original source documents.
//
//////////////////////////////////////////////////
// COPYRIGHT NOTICE:
// 
//////////////////////////////////////////////////
// TorrenTres by Göth^Lorien
// $Id: FileSettingsPage.h,v 1.1 2004/03/31 20:39:09 deepb Exp $
//////////////////////////////////////////////////
//
// This is a BitTorrent clone made in C++ (VS .NET 2003) 
// uses ATL and MFC libraries, so it is intended to work 
// on MS Operating Systems only.
// 
// It is a derivative work of Torment (Jason Zimmer), 
// but heavily enhanced and aimed to people who can understand Spanish 
// (though international releases and arbitrary translations 
// by XML additions are expected).
// 
// Both binary and source code forms are available as of version 0.0.2. 
// They're both working and ready to distribution.
// 
// This is a Work In Progress as of now. Main features expected are:
// - 'sparse' file creation, 
// - report arbitrary IP to Tracker, 
// - automatic prioritization of downloads, 
// - selective downloading within multifile torrents, 
// - selective downloading, 
// - multitracker specification, 
// - client recognition, and some more. 
// Some of them could be working yet by now, others are on their way.
// 
// Code can be used anywhere as long as this copyright notices 
// are kept untouched in both binary and sourcecode forms.
// 
// END OF COPYRIGHT NOTICE
//////////////////////////////////////////////////

//////////////////////////////////////////////////
//
// FileSettingsPage.h
//
// Manages the File Settings Dialog
//

#pragma once
#include "afxwin.h"


// FileSettingsPage dialog

class FileSettingsPage : public CPropertyPage
{
	DECLARE_DYNAMIC(FileSettingsPage)

public:
	FileSettingsPage();
	virtual ~FileSettingsPage();

// Dialog Data
	enum { IDD = IDD_FILESETTINGS_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedAsktosaveCheck();
	afx_msg void OnBnClickedRemoveunderscoresCheck();
	afx_msg void OnBnClickedDefaultsavedirCheck();
	afx_msg void OnEnChangeDefaultsaveEdit();
	afx_msg void OnBnClickedDownloadalgorithmRarestRadio();
	afx_msg void OnBnClickedDownloadalgorithmRandomRadio();
	afx_msg void OnBnClickedDownloadalgorithmSequentialRadio();
	BOOL m_AskToSave;
	BOOL m_RemoveUnderscores;
	BOOL m_UseDefaultSaveDir;
	CString m_DefaultSaveDir;
	int m_DownloadAlgorithm;
	CEdit m_cDefaultSaveDir;
	CButton m_cDefaultSaveDirBrowse;
	afx_msg void OnBnClickedDefaultsaveButton();
	virtual BOOL OnApply();
	afx_msg void OnBnClickedCreatesubdirsCheck();
	BOOL m_CreateSubDirs;
	afx_msg void OnBnClickedAutoloadCheck();
	BOOL m_AutoLoad;
	CEdit m_cAutoLoadDir;
	CButton m_cAutoLoadDirBrowse;
	CString m_AutoLoadDir;
	afx_msg void OnEnChangeAutoloadEdit();
	afx_msg void OnBnClickedAutoloadButton();
	afx_msg void OnBnClickedHistoryCheck();
	afx_msg void OnEnChangeHistoryEdit();
	afx_msg void OnBnClickedHistoryButton();
	BOOL m_History;
	CString m_HistoryDir;
	CEdit m_cHistoryDir;
	CButton m_cHistoryDirBrowse;
};
