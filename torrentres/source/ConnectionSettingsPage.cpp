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
// $Id: ConnectionSettingsPage.cpp,v 1.1 2004/03/31 20:39:09 deepb Exp $
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
// ConnectionSettingsPage.cpp
//
// Manages Settings for the Connections
//

#include "stdafx.h"
#include "Torment.h"
#include "ConnectionSettingsPage.h"

#include "Settings.h"


// ConnectionSettingsPage dialog

IMPLEMENT_DYNAMIC(ConnectionSettingsPage, CPropertyPage)
ConnectionSettingsPage::ConnectionSettingsPage()
	: CPropertyPage(ConnectionSettingsPage::IDD)
	, m_MaxUploadRate(0)
	, m_MaxUploads(0)
	, m_Port(0)
	, m_MaxConnections(0)
	, m_UnlimitedUploadRate(FALSE)
	, m_UnlimitedConnections(FALSE)
//DeepB
	, m_ForceIP(_T(""))
	, m_ForceIPCheck(FALSE)
//END DeepB
{
}

ConnectionSettingsPage::~ConnectionSettingsPage()
{
}

void ConnectionSettingsPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_MAXUPLOADRATE_EDIT, m_MaxUploadRate);
	DDV_MinMaxUInt(pDX, m_MaxUploadRate, 1, 9999);
	DDX_Text(pDX, IDC_MAXUPLOADS_EDIT, m_MaxUploads);
	DDV_MinMaxUInt(pDX, m_MaxUploads, 1, 9999);
	DDX_Text(pDX, IDC_PORT_EDIT, m_Port);
	DDV_MinMaxUInt(pDX, m_Port, 0, 65535);
	DDX_Text(pDX, IDC_MAXCONNECTIONS_EDIT, m_MaxConnections);
	DDV_MinMaxUInt(pDX, m_MaxConnections, 5, 9999);
	DDX_Check(pDX, IDC_UNLIMITEDUPLOADRATE_CHECK, m_UnlimitedUploadRate);
	DDX_Check(pDX, IDC_UNLIMITEDCONNECTIONS_CHECK, m_UnlimitedConnections);
	DDX_Control(pDX, IDC_MAXUPLOADRATE_EDIT, m_cMaxUploadRate);
	DDX_Control(pDX, IDC_MAXUPLOADRATE_SPIN, m_cMaxUploadRateSpin);
	DDX_Control(pDX, IDC_MAXCONNECTIONS_EDIT, m_cMaxConnections);
	DDX_Control(pDX, IDC_MAXCONNECTIONS_SPIN, m_cMaxConnectionsSpin);
	DDX_Control(pDX, IDC_MAXUPLOADS_SPIN, m_cMaxUploadsSpin);
	DDX_Control(pDX, IDC_PORT_SPIN, m_cPortSpin);
//DeepB
	DDX_Text(pDX, IDC_FORCEIP_EDIT, m_ForceIP);
	DDX_Check(pDX, IDC_FORCEIP_CHECK, m_ForceIPCheck);
	DDX_Control(pDX, IDC_FORCEIP_EDIT, m_cForceIP);
//END DeepB
}


BEGIN_MESSAGE_MAP(ConnectionSettingsPage, CPropertyPage)
	ON_EN_CHANGE(IDC_MAXUPLOADRATE_EDIT, OnEnChangeMaxuploadrateEdit)
	ON_EN_CHANGE(IDC_MAXUPLOADS_EDIT, OnEnChangeMaxuploadsEdit)
	ON_EN_CHANGE(IDC_PORT_EDIT, OnEnChangePortEdit)
	ON_EN_CHANGE(IDC_MAXCONNECTIONS_EDIT, OnEnChangeMaxconnectionsEdit)
	ON_BN_CLICKED(IDC_UNLIMITEDUPLOADRATE_CHECK, OnBnClickedUnlimiteduploadrateCheck)
	ON_BN_CLICKED(IDC_UNLIMITEDCONNECTIONS_CHECK, OnBnClickedUnlimitedconnectionsCheck)
//DeepB
	ON_EN_CHANGE(IDC_FORCEIP_EDIT, OnEnChangeForceIPEdit)
	ON_BN_CLICKED(IDC_FORCEIP_CHECK, OnBnClickedForceIPCheck)
//END DeepB
END_MESSAGE_MAP()


// ConnectionSettingsPage message handlers

BOOL ConnectionSettingsPage::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	m_MaxUploadRate = Settings::GetDWordValue("MaxUploadRate");
	m_cMaxUploadRateSpin.SetRange32(1, 9999);
	m_MaxUploads = Settings::GetDWordValue("MaxUploads");
	m_cMaxUploadsSpin.SetRange32(1, 9999);
	m_Port = Settings::GetDWordValue("Port");
	m_cPortSpin.SetRange32(0, 65535);
	m_MaxConnections = Settings::GetDWordValue("MaxConnections");
	m_cMaxConnectionsSpin.SetRange32(5, 9999);

	m_UnlimitedUploadRate = Settings::GetBoolValue("UnlimitedUploadRate");
	m_UnlimitedConnections = Settings::GetBoolValue("UnlimitedConnections");

//DeepB
	m_ForceIP = Settings::GetStringValue("ForcedIP");
	m_ForceIPCheck = Settings::GetBoolValue("ForceIP");
//END DeepB

	UpdateData(FALSE);

	OnBnClickedUnlimiteduploadrateCheck();
	OnBnClickedUnlimitedconnectionsCheck();
//DeepB
	OnBnClickedForceIPCheck();
//END DeepB
	return TRUE;
}

void ConnectionSettingsPage::OnEnChangeMaxuploadrateEdit()
{
	SetModified();
}

void ConnectionSettingsPage::OnEnChangeMaxuploadsEdit()
{
	SetModified();
}

void ConnectionSettingsPage::OnEnChangePortEdit()
{
	SetModified();
}

void ConnectionSettingsPage::OnEnChangeMaxconnectionsEdit()
{
	SetModified();
}

void ConnectionSettingsPage::OnBnClickedUnlimiteduploadrateCheck()
{
	SetModified();

	UpdateData(TRUE);

	if(m_UnlimitedUploadRate)
	{
		m_cMaxUploadRate.EnableWindow(FALSE);
	}else{
		m_cMaxUploadRate.EnableWindow(TRUE);
	}
	m_cMaxUploadRateSpin.Invalidate();
}

void ConnectionSettingsPage::OnBnClickedUnlimitedconnectionsCheck()
{
	SetModified();

	UpdateData(TRUE);

	if(m_UnlimitedConnections)
	{
		m_cMaxConnections.EnableWindow(FALSE);
	}else{
		m_cMaxConnections.EnableWindow(TRUE);
	}
	m_cMaxConnectionsSpin.Invalidate();
}

//DeepB {
void ConnectionSettingsPage::OnBnClickedForceIPCheck()
{
	SetModified();

	UpdateData(TRUE);

	if(m_ForceIPCheck)
	{
		m_cForceIP.EnableWindow(TRUE);
	}else{
		m_cForceIP.EnableWindow(FALSE);
	}
}

void ConnectionSettingsPage::OnEnChangeForceIPEdit()
{
	SetModified();
}
//}END DeepB

BOOL ConnectionSettingsPage::OnApply()
{
	Settings::SetDWordValue("MaxUploadRate", m_MaxUploadRate);
	Settings::SetDWordValue("MaxUploads", m_MaxUploads);
	Settings::SetDWordValue("Port", m_Port);
	Settings::SetDWordValue("MaxConnections", m_MaxConnections);

	Settings::SetBoolValue("UnlimitedUploadRate", m_UnlimitedUploadRate ? true : false);
	Settings::SetBoolValue("UnlimitedConnections", m_UnlimitedConnections ? true : false);

//DeepB {
	Settings::SetBoolValue("ForceIP", m_ForceIPCheck ? true : false);
	Settings::SetStringValue("ForcedIP", m_ForceIP);
//}END DeepB

	return CPropertyPage::OnApply();
}
