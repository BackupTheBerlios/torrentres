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
// $Id: FileSettingsPage.cpp,v 1.1 2004/03/31 20:39:09 deepb Exp $
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
// FileSettingsPage.cpp
//
// Manages the File Settings Dialog
//

#include "stdafx.h"
#include "Torment.h"
#include "FileSettingsPage.h"

#include "Settings.h"

static int CALLBACK browseCallbackProc(HWND hWnd, UINT uMsg, LPARAM /*lp*/, LPARAM pData) {
	switch(uMsg) {
	case BFFM_INITIALIZED: 
		::SendMessage(hWnd, BFFM_SETSELECTION, TRUE, pData );
		break;
	}
	return 0;
}


// FileSettingsPage dialog

IMPLEMENT_DYNAMIC(FileSettingsPage, CPropertyPage)
FileSettingsPage::FileSettingsPage()
	: CPropertyPage(FileSettingsPage::IDD)
	, m_AskToSave(FALSE)
	, m_RemoveUnderscores(FALSE)
	, m_UseDefaultSaveDir(FALSE)
	, m_DefaultSaveDir(_T(""))
	, m_DownloadAlgorithm(FALSE)
	, m_CreateSubDirs(FALSE)
	, m_AutoLoad(FALSE)
	, m_AutoLoadDir(_T(""))
	, m_History(FALSE)
	, m_HistoryDir(_T(""))
{
}

FileSettingsPage::~FileSettingsPage()
{
}

void FileSettingsPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_ASKTOSAVE_CHECK, m_AskToSave);
	DDX_Check(pDX, IDC_REMOVEUNDERSCORES_CHECK, m_RemoveUnderscores);
	DDX_Check(pDX, IDC_DEFAULTSAVEDIR_CHECK, m_UseDefaultSaveDir);
	DDX_Text(pDX, IDC_DEFAULTSAVE_EDIT, m_DefaultSaveDir);
	DDX_Radio(pDX, IDC_DOWNLOADALGORITHM_RAREST_RADIO, m_DownloadAlgorithm);
	DDX_Control(pDX, IDC_DEFAULTSAVE_EDIT, m_cDefaultSaveDir);
	DDX_Control(pDX, IDC_DEFAULTSAVE_BUTTON, m_cDefaultSaveDirBrowse);
	DDX_Check(pDX, IDC_CREATESUBDIRS_CHECK, m_CreateSubDirs);
	DDX_Check(pDX, IDC_AUTOLOAD_CHECK, m_AutoLoad);
	DDX_Control(pDX, IDC_AUTOLOAD_EDIT, m_cAutoLoadDir);
	DDX_Control(pDX, IDC_AUTOLOAD_BUTTON, m_cAutoLoadDirBrowse);
	DDX_Text(pDX, IDC_AUTOLOAD_EDIT, m_AutoLoadDir);
	DDX_Check(pDX, IDC_HISTORY_CHECK, m_History);
	DDX_Text(pDX, IDC_HISTORY_EDIT, m_HistoryDir);
	DDX_Control(pDX, IDC_HISTORY_EDIT, m_cHistoryDir);
	DDX_Control(pDX, IDC_HISTORY_BUTTON, m_cHistoryDirBrowse);
}


BEGIN_MESSAGE_MAP(FileSettingsPage, CPropertyPage)
	ON_BN_CLICKED(IDC_ASKTOSAVE_CHECK, OnBnClickedAsktosaveCheck)
	ON_BN_CLICKED(IDC_REMOVEUNDERSCORES_CHECK, OnBnClickedRemoveunderscoresCheck)
	ON_BN_CLICKED(IDC_DEFAULTSAVEDIR_CHECK, OnBnClickedDefaultsavedirCheck)
	ON_EN_CHANGE(IDC_DEFAULTSAVE_EDIT, OnEnChangeDefaultsaveEdit)
	ON_BN_CLICKED(IDC_DOWNLOADALGORITHM_RAREST_RADIO, OnBnClickedDownloadalgorithmRarestRadio)
	ON_BN_CLICKED(IDC_DOWNLOADALGORITHM_RANDOM_RADIO, OnBnClickedDownloadalgorithmRandomRadio)
	ON_BN_CLICKED(IDC_DOWNLOADALGORITHM_SEQUENTIAL_RADIO, OnBnClickedDownloadalgorithmSequentialRadio)
	ON_BN_CLICKED(IDC_DEFAULTSAVE_BUTTON, OnBnClickedDefaultsaveButton)
	ON_BN_CLICKED(IDC_CREATESUBDIRS_CHECK, OnBnClickedCreatesubdirsCheck)
	ON_BN_CLICKED(IDC_AUTOLOAD_CHECK, OnBnClickedAutoloadCheck)
	ON_EN_CHANGE(IDC_AUTOLOAD_EDIT, OnEnChangeAutoloadEdit)
	ON_BN_CLICKED(IDC_AUTOLOAD_BUTTON, OnBnClickedAutoloadButton)
	ON_BN_CLICKED(IDC_HISTORY_CHECK, OnBnClickedHistoryCheck)
	ON_EN_CHANGE(IDC_HISTORY_EDIT, OnEnChangeHistoryEdit)
	ON_BN_CLICKED(IDC_HISTORY_BUTTON, OnBnClickedHistoryButton)
END_MESSAGE_MAP()


// FileSettingsPage message handlers

BOOL FileSettingsPage::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	m_AskToSave = Settings::GetBoolValue("AskToSave");
	m_CreateSubDirs = Settings::GetBoolValue("CreateSubDirs");
	m_RemoveUnderscores = Settings::GetBoolValue("RemoveUnderscores");
	m_UseDefaultSaveDir = Settings::GetBoolValue("UseDefaultSaveDir");
	m_DefaultSaveDir = Settings::GetStringValue("DefaultSaveDir");
	m_DownloadAlgorithm = Settings::GetDWordValue("DownloadAlgorithm");
	m_AutoLoad = Settings::GetBoolValue("AutoLoad");
	m_AutoLoadDir = Settings::GetStringValue("AutoLoadDir");
	m_History = Settings::GetBoolValue("History");
	m_HistoryDir = Settings::GetStringValue("HistoryDir");
	
	UpdateData(FALSE);

	OnBnClickedDefaultsavedirCheck();
	OnBnClickedAutoloadCheck();
	OnBnClickedHistoryCheck();

	return TRUE;
}
void FileSettingsPage::OnBnClickedAsktosaveCheck()
{
	SetModified();
}

void FileSettingsPage::OnBnClickedRemoveunderscoresCheck()
{
	SetModified();
}

void FileSettingsPage::OnBnClickedDefaultsavedirCheck()
{
	SetModified();

	UpdateData(TRUE);

	if(m_UseDefaultSaveDir)
	{
		m_cDefaultSaveDir.EnableWindow(TRUE);
		m_cDefaultSaveDirBrowse.EnableWindow(TRUE);
	}else{
		m_cDefaultSaveDir.EnableWindow(FALSE);
		m_cDefaultSaveDirBrowse.EnableWindow(FALSE);
	}
}

void FileSettingsPage::OnEnChangeDefaultsaveEdit()
{
	SetModified();
}

void FileSettingsPage::OnBnClickedDownloadalgorithmRarestRadio()
{
	SetModified();
}

void FileSettingsPage::OnBnClickedDownloadalgorithmRandomRadio()
{
	SetModified();
}

void FileSettingsPage::OnBnClickedDownloadalgorithmSequentialRadio()
{
	SetModified();
}

void FileSettingsPage::OnBnClickedDefaultsaveButton()
{
	BROWSEINFO bi;
	ITEMIDLIST * iid;
	LPMALLOC m;
	char FName[MAX_PATH];

	ZeroMemory(&bi, sizeof(bi));
	CString csAux;
	csAux = Settings::GetStringValue("DefaultSaveDir");
	if (csAux.GetLength() > 0)
	{
		bi.lParam = (LPARAM)(csAux.GetBuffer(1));
		bi.lpfn = (BFFCALLBACK)browseCallbackProc;
		csAux.ReleaseBuffer();
	}
	else
	{
		bi.lpfn   = NULL;
		bi.lParam = 0;
	}
	bi.hwndOwner = *this;
	bi.lpszTitle = "Escoge la carpeta en la que descargar por defecto.";
	bi.ulFlags = BIF_USENEWUI | BIF_SHAREABLE | BIF_DONTGOBELOWDOMAIN | BIF_RETURNONLYFSDIRS;

	iid = SHBrowseForFolder(&bi);

	if(iid == NULL)
	{
		return;
	}

	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	if(SHGetPathFromIDList(iid, FName) == FALSE) return;

	UpdateData(TRUE);
	m_DefaultSaveDir = FName;
	UpdateData(FALSE);

	SHGetMalloc(&m);
	m->Free(iid);
	m->Release();

	CoUninitialize();

	SetModified();
}

BOOL FileSettingsPage::OnApply()
{
	UpdateData(TRUE);

	Settings::SetBoolValue("AskToSave", m_AskToSave ? true : false);
	Settings::SetBoolValue("CreateSubDirs", m_CreateSubDirs ? true : false);
	Settings::SetBoolValue("RemoveUnderscores", m_RemoveUnderscores ? true : false);
	Settings::SetBoolValue("UseDefaultSaveDir", m_UseDefaultSaveDir ? true : false);
	Settings::SetStringValue("DefaultSaveDir", m_DefaultSaveDir);
	Settings::SetDWordValue("DownloadAlgorithm", m_DownloadAlgorithm);
	Settings::SetBoolValue("AutoLoad", m_AutoLoad ? true : false);
	Settings::SetStringValue("AutoLoadDir", m_AutoLoadDir);
	Settings::SetBoolValue("History", m_History ? true : false);
	Settings::SetStringValue("HistoryDir", m_HistoryDir);

	return CPropertyPage::OnApply();
}

void FileSettingsPage::OnBnClickedCreatesubdirsCheck()
{
	SetModified();
}
void FileSettingsPage::OnBnClickedAutoloadCheck()
{
	SetModified();

	UpdateData(TRUE);

	if(m_AutoLoad)
	{
		m_cAutoLoadDir.EnableWindow(TRUE);
		m_cAutoLoadDirBrowse.EnableWindow(TRUE);
	}else{
		m_cAutoLoadDir.EnableWindow(FALSE);
		m_cAutoLoadDirBrowse.EnableWindow(FALSE);
	}
}

void FileSettingsPage::OnEnChangeAutoloadEdit()
{
	SetModified();
}

void FileSettingsPage::OnBnClickedAutoloadButton()
{
	BROWSEINFO bi;
	ITEMIDLIST * iid;
	LPMALLOC m;
	char FName[MAX_PATH];

	ZeroMemory(&bi, sizeof(bi));
	CString csAux;
	csAux = Settings::GetStringValue("AutoLoadDir");
	if (csAux.GetLength() > 0)
	{
		bi.lParam = (LPARAM)(csAux.GetBuffer(1));
		bi.lpfn = (BFFCALLBACK)browseCallbackProc;
		csAux.ReleaseBuffer();
	}
	else
	{
		bi.lpfn   = NULL;
		bi.lParam = 0;
	}

	bi.hwndOwner = *this;
	bi.lpszTitle = "Escoge la carpeta en la que almacenar los torrentes.";
	bi.ulFlags = BIF_NEWDIALOGSTYLE | BIF_EDITBOX;

	iid = SHBrowseForFolder(&bi);

	if(iid == NULL)
	{
		return;
	}

	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	if(SHGetPathFromIDList(iid, FName) == FALSE) return;

	UpdateData(TRUE);
	m_AutoLoadDir = FName;
	UpdateData(FALSE);

	SHGetMalloc(&m);
	m->Free(iid);
	m->Release();

	CoUninitialize();

	SetModified();
}

void FileSettingsPage::OnBnClickedHistoryCheck()
{
	SetModified();

	UpdateData(TRUE);

	if(m_History)
	{
		m_cHistoryDir.EnableWindow(TRUE);
		m_cHistoryDirBrowse.EnableWindow(TRUE);
	}else{
		m_cHistoryDir.EnableWindow(FALSE);
		m_cHistoryDirBrowse.EnableWindow(FALSE);
	}
}

void FileSettingsPage::OnEnChangeHistoryEdit()
{
	SetModified();
}

void FileSettingsPage::OnBnClickedHistoryButton()
{
	BROWSEINFO bi;
	ITEMIDLIST * iid;
	LPMALLOC m;
	char FName[MAX_PATH];

	ZeroMemory(&bi, sizeof(bi));
	CString csAux;
	csAux = Settings::GetStringValue("HistoryDir");
	if (csAux.GetLength() > 0)
	{
		bi.lParam = (LPARAM)(csAux.GetBuffer(1));
		bi.lpfn = (BFFCALLBACK)browseCallbackProc;
		csAux.ReleaseBuffer();
	}
	else
	{
		bi.lpfn   = NULL;
		bi.lParam = 0;
	}

	bi.hwndOwner = *this;
	bi.lpszTitle = "Escoge la carpeta en la que guardar los torrentes.";
	bi.ulFlags = BIF_NEWDIALOGSTYLE | BIF_EDITBOX;

	iid = SHBrowseForFolder(&bi);

	if(iid == NULL)
	{
		return;
	}

	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	if(SHGetPathFromIDList(iid, FName) == FALSE) return;

	UpdateData(TRUE);
	m_HistoryDir = FName;
	UpdateData(FALSE);

	SHGetMalloc(&m);
	m->Free(iid);
	m->Release();

	CoUninitialize();

	SetModified();
}