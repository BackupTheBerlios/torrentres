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
// $Id: ConnectionSettingsPage.h,v 1.1 2004/03/31 20:39:09 deepb Exp $
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
// ConnectionSettingsPage.h
//
// Manages Settings for the Connections
//

#pragma once
#include "afxwin.h"
#include "afxcmn.h"


// ConnectionSettingsPage dialog

class ConnectionSettingsPage : public CPropertyPage
{
	DECLARE_DYNAMIC(ConnectionSettingsPage)

public:
	ConnectionSettingsPage();
	virtual ~ConnectionSettingsPage();

// Dialog Data
	enum { IDD = IDD_CONNECTIONSETTINGS_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnEnChangeMaxuploadrateEdit();
	afx_msg void OnEnChangeMaxuploadsEdit();
	afx_msg void OnEnChangePortEdit();
	afx_msg void OnEnChangeMaxconnectionsEdit();
	afx_msg void OnBnClickedUnlimiteduploadrateCheck();
	afx_msg void OnBnClickedUnlimitedconnectionsCheck();
	virtual BOOL OnApply();

	DWORD m_MaxUploadRate;
	DWORD m_MaxUploads;
	DWORD m_Port;
	DWORD m_MaxConnections;
	BOOL m_UnlimitedUploadRate;
	BOOL m_UnlimitedConnections;
//DeepB
	afx_msg void OnBnClickedForceIPCheck();
	afx_msg void OnEnChangeForceIPEdit();
	BOOL m_ForceIPCheck;
	CString m_ForceIP;
	CEdit m_cForceIP;
//End DeepB
	CEdit m_cMaxUploadRate;
	CSpinButtonCtrl m_cMaxUploadRateSpin;
	CEdit m_cMaxConnections;
	CSpinButtonCtrl m_cMaxConnectionsSpin;
	CSpinButtonCtrl m_cMaxUploadsSpin;
	CSpinButtonCtrl m_cPortSpin;
};
