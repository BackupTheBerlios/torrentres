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
// $Id: WindowSettingsPage.cpp,v 1.1 2004/03/31 20:39:09 deepb Exp $
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
// WindowSettingsPage.cpp
//
// Manages the Windows Settings
//

#include "stdafx.h"
#include "Torment.h"
#include "WindowSettingsPage.h"

#include "Settings.h"
#include ".\windowsettingspage.h"

// WindowSettingsPage dialog

IMPLEMENT_DYNAMIC(WindowSettingsPage, CPropertyPage)
WindowSettingsPage::WindowSettingsPage()
	: CPropertyPage(WindowSettingsPage::IDD)
	, m_MinimizeToSysTray(FALSE)
	, m_StatusOnLoad(FALSE)
	, m_StatusMinimizeToSysTray(FALSE)
	, m_DoubleBlockMap(FALSE)
{
}

WindowSettingsPage::~WindowSettingsPage()
{
}

void WindowSettingsPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_MINIMIZETOSYSTRAY_CHECK, m_MinimizeToSysTray);
	DDX_Check(pDX, IDC_STATUSONLOAD_CHECK, m_StatusOnLoad);
	DDX_Check(pDX, IDC_STATUSMINIMIZETOSYSTRAY_CHECK, m_StatusMinimizeToSysTray);
	DDX_Check(pDX, IDC_DOUBLEBLOCKMAP_CHECK, m_DoubleBlockMap);
}


BEGIN_MESSAGE_MAP(WindowSettingsPage, CPropertyPage)
	ON_BN_CLICKED(IDC_MINIMIZETOSYSTRAY_CHECK, OnBnClickedMinimizetosystrayCheck)
	ON_BN_CLICKED(IDC_STATUSONLOAD_CHECK, OnBnClickedStatusonloadCheck)
	ON_BN_CLICKED(IDC_STATUSMINIMIZETOSYSTRAY_CHECK, OnBnClickedStatusminimizetosystrayCheck)
	ON_BN_CLICKED(IDC_DOUBLEBLOCKMAP_CHECK, OnBnClickedDoubleblockmapCheck)
END_MESSAGE_MAP()


// WindowSettingsPage message handlers

void WindowSettingsPage::OnBnClickedMinimizetosystrayCheck()
{
	SetModified();
}

void WindowSettingsPage::OnBnClickedStatusonloadCheck()
{
	SetModified();
}

BOOL WindowSettingsPage::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	m_MinimizeToSysTray = Settings::GetBoolValue("MinimizeToSystray");
	m_StatusOnLoad = Settings::GetBoolValue("StatusOnLoad");
	m_StatusMinimizeToSysTray = Settings::GetBoolValue("StatusMinimizeToSystray");
	m_DoubleBlockMap = Settings::GetBoolValue("DoubleBlockMap");

	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

BOOL WindowSettingsPage::OnApply()
{
	Settings::SetBoolValue("MinimizeToSystray", m_MinimizeToSysTray ? true : false);
	Settings::SetBoolValue("StatusMinimizeToSystray", m_StatusMinimizeToSysTray ? true : false);
	Settings::SetBoolValue("StatusOnLoad", m_StatusOnLoad ? true : false);
	Settings::SetBoolValue("DoubleBlockMap", m_DoubleBlockMap ? true : false);

	return CPropertyPage::OnApply();
}

void WindowSettingsPage::OnBnClickedStatusminimizetosystrayCheck()
{
	SetModified();
}

void WindowSettingsPage::OnBnClickedDoubleblockmapCheck()
{
	SetModified();
}
