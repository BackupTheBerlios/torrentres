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
// $Id: WindowSettingsPage.h,v 1.1 2004/03/31 20:39:09 deepb Exp $
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
// WindowSettingsPage.h
//
// Manages the Windows Settings
//

#pragma once


// WindowSettingsPage dialog

class WindowSettingsPage : public CPropertyPage
{
	DECLARE_DYNAMIC(WindowSettingsPage)

public:
	WindowSettingsPage();
	virtual ~WindowSettingsPage();

// Dialog Data
	enum { IDD = IDD_WINDOWSETTINGS_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedMinimizetosystrayCheck();
	afx_msg void OnBnClickedStatusonloadCheck();
	BOOL m_MinimizeToSysTray;
	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();
	BOOL m_StatusOnLoad;
	BOOL m_StatusMinimizeToSysTray;
	afx_msg void OnBnClickedStatusminimizetosystrayCheck();
	BOOL m_DoubleBlockMap;
	afx_msg void OnBnClickedDoubleblockmapCheck();
};
