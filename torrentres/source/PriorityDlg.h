//////////////////////////////////////////////////
// COPYRIGHT NOTICE:
// 
//////////////////////////////////////////////////
// TorrenTres by Göth^Lorien
// $Id: PriorityDlg.h,v 1.1 2004/03/31 20:39:09 deepb Exp $
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
#pragma once
#include "afxcmn.h"


// PriorityDlg dialog

class PriorityDlg : public CDialog
{
	DECLARE_DYNAMIC(PriorityDlg)

public:
	PriorityDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~PriorityDlg();

// Dialog Data
	enum { IDD = IDD_PRIORITY_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CListCtrl m_FileList;
	CListCtrl m_PriorityList;
	BOOL m_Enable;
	afx_msg void OnBnClickedAdd();
	afx_msg void OnBnClickedRemove();
	afx_msg void OnNMDblclkFilelist(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMDblclkPrioritylist(NMHDR *pNMHDR, LRESULT *pResult);

	CArray<CString> m_Files;
	CArray<DWORD> m_FileSizes;

	DWORD * m_Priorities;
	DWORD * m_Temp;
	DWORD m_Prioritized;

	virtual BOOL OnInitDialog();
protected:
	virtual void OnOK();
};
