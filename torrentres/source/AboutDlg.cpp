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
// $Id: AboutDlg.cpp,v 1.1 2004/03/31 20:39:09 deepb Exp $
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
// AboutDlg.cpp
//
// Implementation of the About Dialog Box
//

#include "stdafx.h"
#include "Torment.h"
#include "AboutDlg.h"
#include ".\aboutdlg.h"


// AboutDlg dialog

IMPLEMENT_DYNAMIC(AboutDlg, CDialog)
AboutDlg::AboutDlg(CWnd* pParent /*=NULL*/)
	: CDialog(AboutDlg::IDD, pParent)
	, m_MemoryUsage(_T(""))
{
}

AboutDlg::~AboutDlg()
{
}

void AboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_MEMUSAGE, m_MemoryUsage);
}


BEGIN_MESSAGE_MAP(AboutDlg, CDialog)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// AboutDlg message handlers

BOOL AboutDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetTimer(0, 1000, NULL);

	m_MemoryUsage.Format("Uso de memoria: %s (%d Bytes)", FormatBytes(MUsage()), MUsage());
	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void AboutDlg::OnTimer(UINT nIDEvent)
{
	m_MemoryUsage.Format("Uso de memoria: %s (%d Bytes)", FormatBytes(MUsage()), MUsage());
	UpdateData(FALSE);

	CDialog::OnTimer(nIDEvent);
}
