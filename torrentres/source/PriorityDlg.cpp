//////////////////////////////////////////////////
// COPYRIGHT NOTICE:
// 
//////////////////////////////////////////////////
// TorrenTres by Göth^Lorien
// $Id: PriorityDlg.cpp,v 1.1 2004/03/31 20:39:09 deepb Exp $
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
// PriorityDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Torment.h"
#include "PriorityDlg.h"
#include ".\prioritydlg.h"


// PriorityDlg dialog

IMPLEMENT_DYNAMIC(PriorityDlg, CDialog)
PriorityDlg::PriorityDlg(CWnd* pParent /*=NULL*/)
	: CDialog(PriorityDlg::IDD, pParent)
	, m_Enable(FALSE)
{
//	m_Temp = new DWORD [0];
	m_Temp = MAlloc(DWORD, 0);
}

PriorityDlg::~PriorityDlg()
{
	MFree(m_Temp);
}

void PriorityDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_FILELIST, m_FileList);
	DDX_Control(pDX, IDC_PRIORITYLIST, m_PriorityList);
	DDX_Check(pDX, IDC_ENABLE_CHECK, m_Enable);
}


BEGIN_MESSAGE_MAP(PriorityDlg, CDialog)
	ON_BN_CLICKED(IDC_ADD, OnBnClickedAdd)
	ON_BN_CLICKED(IDC_REMOVE, OnBnClickedRemove)
	ON_NOTIFY(NM_DBLCLK, IDC_FILELIST, OnNMDblclkFilelist)
	ON_NOTIFY(NM_DBLCLK, IDC_PRIORITYLIST, OnNMDblclkPrioritylist)
END_MESSAGE_MAP()


// PriorityDlg message handlers

void PriorityDlg::OnBnClickedAdd()
{
	CArray<DWORD> Move;
	POSITION Pos;
	DWORD Index;
	DWORD i, j;

	Pos = m_FileList.GetFirstSelectedItemPosition();

	if(Pos == NULL) return;

	while(Pos)
	{
		Index = m_FileList.GetNextSelectedItem(Pos);
		Move.Add(m_FileList.GetItemData(Index));
	}

	// Move Items
	for(i = 0; i < (DWORD)Move.GetSize(); i++)
	{
		Index = m_PriorityList.InsertItem(m_PriorityList.GetItemCount(), m_Files[Move[i]]);
		m_PriorityList.SetItemText(Index, 1, FormatBytes(m_FileSizes[Move[i]]));
		m_PriorityList.SetItemData(Index, Move[i]);
	}

	// Remove Items
	for(i = 0; i < (DWORD)Move.GetSize(); i++)
	{
		for(j = 0; j < (DWORD)m_FileList.GetItemCount(); j++)
		{
			TRACE("Removing %d\n", Move[i]);
			if(m_FileList.GetItemData(j) == Move[i])
			{
				m_FileList.DeleteItem(j);
				break;
			}
		}
	}
}

void PriorityDlg::OnBnClickedRemove()
{
	CArray<DWORD> Move;
	POSITION Pos;
	DWORD Index;
	DWORD i, j;

	Pos = m_PriorityList.GetFirstSelectedItemPosition();

	if(Pos == NULL) return;

	while(Pos)
	{
		Index = m_PriorityList.GetNextSelectedItem(Pos);
		Move.Add(m_PriorityList.GetItemData(Index));
	}

	// Move Items
	for(i = 0; i < (DWORD)Move.GetSize(); i++)
	{
		Index = m_FileList.InsertItem(m_FileList.GetItemCount(), m_Files[Move[i]]);
		m_FileList.SetItemText(Index, 1, FormatBytes(m_FileSizes[Move[i]]));
		m_FileList.SetItemData(Index, Move[i]);
	}

	// Remove Items
	for(i = 0; i < (DWORD)Move.GetSize(); i++)
	{
		for(j = 0; j < (DWORD)m_PriorityList.GetItemCount(); j++)
		{
			TRACE("Removing %d\n", Move[i]);
			if(m_PriorityList.GetItemData(j) == Move[i])
			{
				m_PriorityList.DeleteItem(j);
				break;
			}
		}
	}
}

void PriorityDlg::OnNMDblclkFilelist(NMHDR *pNMHDR, LRESULT *pResult)
{
	OnBnClickedAdd();
	*pResult = 0;
}

void PriorityDlg::OnNMDblclkPrioritylist(NMHDR *pNMHDR, LRESULT *pResult)
{
	OnBnClickedRemove();
	*pResult = 0;
}

BOOL PriorityDlg::OnInitDialog()
{
	DWORD i;
	DWORD Index;

	CDialog::OnInitDialog();

	m_FileList.InsertColumn(0, "Archivo", LVCFMT_LEFT, 150, 0);
	m_FileList.InsertColumn(1, "Tamaño", LVCFMT_LEFT, 75, 1);

	m_PriorityList.InsertColumn(0, "Archivo", LVCFMT_LEFT, 150, 0);
	m_PriorityList.InsertColumn(1, "Tamaño", LVCFMT_LEFT, 75, 1);

	MFree(m_Temp);
//	m_Temp = new DWORD [m_Files.GetSize()];
	m_Temp = MAlloc(DWORD, m_Files.GetSize());

	for(i = 0; i < (DWORD)m_Files.GetSize(); i++)
	{
		m_Temp[i] = m_Priorities[i];

		if(i < m_Prioritized)
		{
			Index = m_PriorityList.InsertItem(i, m_Files[m_Temp[i]]);
			m_PriorityList.SetItemText(Index, 1, FormatBytes(m_FileSizes[m_Temp[i]]));
			m_PriorityList.SetItemData(Index, m_Temp[i]);
		}else{
			Index = m_FileList.InsertItem(i, m_Files[m_Temp[i]]);
			m_FileList.SetItemText(Index, 1, FormatBytes(m_FileSizes[m_Temp[i]]));
			m_FileList.SetItemData(Index, m_Temp[i]);
		}
	}

	return TRUE;
}

void PriorityDlg::OnOK()
{
	DWORD i;

	for(i = 0; i < (DWORD)m_PriorityList.GetItemCount(); i++)
	{
		m_Priorities[i] = m_PriorityList.GetItemData(i);
	}

	m_Prioritized = i;

	for(i = 0; i < (DWORD)m_FileList.GetItemCount(); i++)
	{
		m_Priorities[i + m_Prioritized] = m_FileList.GetItemData(i);
	}

	CDialog::OnOK();
}
