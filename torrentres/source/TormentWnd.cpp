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
// $Id: TormentWnd.cpp,v 1.1 2004/03/31 20:39:09 deepb Exp $
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
// TormentWnd.cpp
//
// Main Window Interface
//

#include "stdafx.h"
#include "Torment.h"
#include "TormentWnd.h"

#include "Settings.h"

#include "ConnectionSettingsPage.h"
#include "FileSettingsPage.h"
#include "WindowSettingsPage.h"
#include "AboutDlg.h"

DWORD TormentWnd::m_SortInfo[11];


IMPLEMENT_DYNAMIC(TormentWnd, CWnd)
TormentWnd::TormentWnd()
{
	m_IsInTray = false;
}

TormentWnd::~TormentWnd()
{
	while(m_Connections.GetSize())
	{
		MFree(m_Connections[0]);
		m_Connections.RemoveAt(0);
	}
}

BEGIN_MESSAGE_MAP(TormentWnd, CWnd)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_COMMAND(ID_FILE_EXIT, OnFileExit)
	ON_COMMAND(ID_FILE_OPENTORRENT, OnFileOpentorrent)
	ON_COMMAND(ID_TOOLS_SETTINGS, OnToolsSettings)
	ON_WM_DESTROY()
	ON_NOTIFY(NM_DBLCLK, IDC_TORRENTLIST, OnTorrentListDoubleClick)
	ON_NOTIFY(NM_RCLICK, IDC_TORRENTLIST, OnTorrentListRightClick)
	ON_COMMAND(ID_TORRENT_SHOWINFO, OnTorrentShowinfo)
	ON_COMMAND(ID_TORRENT_CANCELDOWNLOAD, OnTorrentCanceldownload)
	ON_COMMAND(ID_TORRENT_GETNEWCLIENTLIST, OnTorrentGetnewclientlist)
	ON_NOTIFY(HDN_ITEMCHANGED, 0, OnTorrentListHdnItemChanged)
	ON_WM_TIMER()
	ON_WM_DROPFILES()
	ON_COMMAND(ID_HELP_ABOUTTORMENT, OnHelpAbouttorment)
	ON_COMMAND(ID_VIEW_BANDWIDTHMONITOR, OnViewBandwidthmonitor)
	ON_COMMAND(ID_VIEW_LOGWINDOW, OnViewLogWindow)
	ON_COMMAND(ID_VIEW_SELECTCOLUMNS, OnViewSelectcolumns)
	ON_COMMAND(ID_TORRENT_MODIFYFILEPRIORITIES, OnTorrentModifyfilepriorities)
	ON_NOTIFY(HDN_ITEMCLICK, 0, OnHdnTorrentListItemclick)
	ON_COMMAND(ID_TORRENT_PAUSE, OnTorrentPause)
	ON_COMMAND(ID_TORRENT_RESUME, OnTorrentResume)
END_MESSAGE_MAP()

int TormentWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	HICON hIcon;
	UINT Indicators[2] = {0, 1};
	DWORD i;

	m_TorrentImgList.Create(13, 13, ILC_COLOR32 | ILC_MASK, 5, 5);

	hIcon = AfxGetApp()->LoadIcon(IDI_BALL_B);
	m_TorrentImgList.Add(hIcon);
	hIcon = AfxGetApp()->LoadIcon(IDI_BALL_G);
	m_TorrentImgList.Add(hIcon);
	hIcon = AfxGetApp()->LoadIcon(IDI_BALL_O);
	m_TorrentImgList.Add(hIcon);
	hIcon = AfxGetApp()->LoadIcon(IDI_BALL_R);
	m_TorrentImgList.Add(hIcon);
	hIcon = AfxGetApp()->LoadIcon(IDI_BALL_Y);
	m_TorrentImgList.Add(hIcon);
	hIcon = AfxGetApp()->LoadIcon(IDI_PAUSE);
	m_TorrentImgList.Add(hIcon);

	m_cTorrentList.Create(LVS_REPORT | WS_CHILD | WS_VISIBLE, CRect(0, 0, 0, 0), this, IDC_TORRENTLIST);
	m_cTorrentList.ModifyStyleEx(0, WS_EX_CLIENTEDGE);
	m_cTorrentList.SetImageList(&m_TorrentImgList, LVSIL_SMALL);

	m_cTorrentList.InsertColumn(0, STRTBL_TORRENTLIST_FILE, LVCFMT_LEFT, 150, 0);
	m_cTorrentList.InsertColumn(1, STRTBL_TORRENTLIST_PROGRESS, LVCFMT_LEFT, 150, 1);
	m_cTorrentList.InsertColumn(2, STRTBL_TORRENTLIST_STATUS, LVCFMT_LEFT, 150, 2);
	m_cTorrentList.InsertColumn(3, STRTBL_TORRENTLIST_AVGBPSIN, LVCFMT_LEFT, 75, 3);
	m_cTorrentList.InsertColumn(4, STRTBL_TORRENTLIST_AVGBPSOUT, LVCFMT_LEFT, 75, 4);
	m_cTorrentList.InsertColumn(5, STRTBL_TORRENTLIST_PEERSEEDS, LVCFMT_LEFT, 25, 5);
	m_cTorrentList.InsertColumn(6, STRTBL_TORRENTLIST_PEERPARTIALS, LVCFMT_LEFT, 25, 6);
	m_cTorrentList.InsertColumn(7, STRTBL_TORRENTLIST_PEERAVERAGEAVAILABLE, LVCFMT_LEFT, 75, 7);
	m_cTorrentList.InsertColumn(8, STRTBL_TORRENTLIST_ESTIMATEDCOMPLETIONTIME, LVCFMT_LEFT, 200, 8);
	m_cTorrentList.InsertColumn(9, STRTBL_TORRENTLIST_BYTESSENT, LVCFMT_LEFT, 150, 9);
	m_cTorrentList.InsertColumn(10, STRTBL_TORRENTLIST_BYTESRECEIVED, LVCFMT_LEFT, 150, 10);

	for(i = 0; i < 11; i++)
	{
		m_SortInfo[i] = i;
	}

	m_Graph.Create(this);

	m_StatusBar.Create(this);
	m_StatusBar.SetIndicators(Indicators, 2);
	m_StatusBar.SetPaneStyle(0, 0);
	m_StatusBar.SetPaneStyle(1, SBPS_STRETCH);

	m_Log.CreateEx(WS_EX_TOOLWINDOW, AfxRegisterWndClass(0, 0, 0, 0), "Log", WS_OVERLAPPEDWINDOW /*| WS_VISIBLE*/, 0, 0, 640, 240, *this, NULL, 0);

//	Log("Iniciando TorrenTres...\n");

	m_MainMenu.LoadMenu(IDR_MAINMENU);
	SetMenu(&m_MainMenu);

	if(Settings::GetBoolValue("ShowBandwidthMonitor"))
	{
		m_MainMenu.CheckMenuItem(ID_VIEW_BANDWIDTHMONITOR, MF_CHECKED);
	}else{
		m_Graph.ShowWindow(SW_HIDE);
	}

	SetTimer(0, 1000, NULL);
//	SetTimer(1, AUTOLOAD_INTERVAL, NULL);

	ResetListener();

	CheckAutoLoad();

	return 0;
}

void TormentWnd::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);

	m_StatusBar.MoveWindow(0, cy - 16, cx, 16);
	cy -= 16;

	if(Settings::GetBoolValue("ShowBandwidthMonitor"))
	{
		m_cTorrentList.MoveWindow(0, 0, cx, cy - 100);
		m_Graph.MoveWindow(0, cy - 100, cx, 100);
	}else{
		m_cTorrentList.MoveWindow(0, 0, cx, cy);
	}

	if(Settings::GetBoolValue("MinimizeToSystray"))
	{
		if(nType == SIZE_MINIMIZED)
		{
			ShowWindow(SW_HIDE);

			ZeroMemory(&m_NotifyData, sizeof(NOTIFYICONDATA));
			m_NotifyData.cbSize = sizeof(NOTIFYICONDATA);
			m_NotifyData.hWnd = *this;
			m_NotifyData.uID = 0;
			m_NotifyData.uFlags = NIF_MESSAGE | NIF_ICON;
			m_NotifyData.uCallbackMessage = WM_USER_TORMENT_RESTOREFROMTRAY;
			m_NotifyData.hIcon = AfxGetApp()->LoadIcon(IDI_TRAY);

			Shell_NotifyIcon(NIM_ADD, &m_NotifyData);
			m_IsInTray = true;
		}
	}
}

void TormentWnd::OnFileExit()
{
	DestroyWindow();
}

void TormentWnd::OnFileOpentorrent()
{
	OPENFILENAME ofn;
	char FileBuffer[4096];
	char * FilePath;
	char * FileName;

	ZeroMemory(&ofn, sizeof(ofn));
	FileBuffer[0] = 0;

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = m_hWnd;
	ofn.lpstrFile = FileBuffer;
	ofn.nMaxFile =  sizeof(FileBuffer);
	ofn.Flags = OFN_ALLOWMULTISELECT | OFN_EXPLORER | OFN_HIDEREADONLY;
	ofn.lpstrFilter = STRTBL_TORRENT_FILEFILTER;

	if(GetOpenFileName(&ofn))
	{
		FilePath = FileBuffer;
		FileName = &FileBuffer[strlen(FilePath) + 1];

		if(FileName[0] == 0)
		{
			OpenTorrent(FilePath, false);
		}else{
			while(FileName[0] != 0)
			{
				OpenTorrent(CString(FilePath) + "\\" + FileName, false);

				FileName = &FileName[strlen(FileName) + 1];
				UpdateTorrentsProgressPosition();
			}
		}
		UpdateTorrentsProgressPosition();
	}
}

void TormentWnd::OnToolsSettings()
{
	CPropertySheet Sheet(STRTBL_PROPERTYSHEET_TITLE, this);
	ConnectionSettingsPage pConnections;
	FileSettingsPage pFiles;
	WindowSettingsPage pWindows;

	Sheet.AddPage(&pConnections);
	Sheet.AddPage(&pFiles);
	Sheet.AddPage(&pWindows);

	Sheet.DoModal();

	ResetListener();
}

void TormentWnd::OnDestroy()
{
	CRect rect;

	this->GetWindowRect(rect);

	Settings::SetDWordValue("WindowPosX", rect.left);
	Settings::SetDWordValue("WindowPosY", rect.top);
	Settings::SetDWordValue("WindowPosW", rect.right - rect.left);
	Settings::SetDWordValue("WindowPosH", rect.bottom - rect.top);
	
	CWnd::OnDestroy();

	while(m_Torrents.GetSize())
	{
		m_Torrents[0]->DestroyWindow();
		MFree(m_Torrents[0]);
		m_Torrents.RemoveAt(0);
	}

	Restore();
}

void TormentWnd::OnTorrentListDoubleClick(NMHDR *pNMHDR, LRESULT *pResult)
{
	OnTorrentShowinfo();
	//LPNMITEMACTIVATE phdr = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);

	//*pResult = 0;

	//if(phdr->iItem == -1) return;

	//((Torrent *)m_cTorrentList.GetItemData(phdr->iItem))->ShowWindow(SW_SHOW);
	//((Torrent *)m_cTorrentList.GetItemData(phdr->iItem))->ShowWindow(SW_RESTORE);
	//((Torrent *)m_cTorrentList.GetItemData(phdr->iItem))->SetForegroundWindow();
}

void TormentWnd::OnTorrentListRightClick(NMHDR *pNMHDR, LRESULT *pResult)
{
	CMenu Menu;
	CPoint pt;
	POSITION Pos;
	int i;

	GetCursorPos(&pt);

	Pos = m_cTorrentList.GetFirstSelectedItemPosition();
	i = m_cTorrentList.GetNextSelectedItem(Pos);

	if(i == -1) return;

	Menu.LoadMenu(IDR_CONTEXTMENU);
	Menu.GetSubMenu(0)->TrackPopupMenu(TPM_RIGHTBUTTON | TPM_LEFTBUTTON, pt.x, pt.y, this);
}

void TormentWnd::OnTorrentShowinfo()
{
	POSITION Pos;
	int i;

	Pos = m_cTorrentList.GetFirstSelectedItemPosition();

	while(Pos)
	{
		i = m_cTorrentList.GetNextSelectedItem(Pos);

		((Torrent *)m_cTorrentList.GetItemData(i))->Restore();
	}
}

void TormentWnd::OnTorrentCanceldownload()
{
	POSITION Pos;
	int i;
	CArray<Torrent *> t;

	Pos = m_cTorrentList.GetFirstSelectedItemPosition();

	while(Pos)
	{
		i = m_cTorrentList.GetNextSelectedItem(Pos);

		t.Add((Torrent *)m_cTorrentList.GetItemData(i));
	}

	for(i = 0; i < t.GetSize(); i++)
	{
		KillTorrent(t[i]);
	}
}

void TormentWnd::OnTorrentGetnewclientlist()
{
	POSITION Pos;
	int i;

	Pos = m_cTorrentList.GetFirstSelectedItemPosition();

	while(Pos)
	{
		i = m_cTorrentList.GetNextSelectedItem(Pos);

		((Torrent *)m_cTorrentList.GetItemData(i))->OnFileGetnewclientlist();
	}
}

void TormentWnd::UpdateTorrentsProgressPosition(void)
{
	int i;

	for(i = 0; i < m_Torrents.GetSize(); i++)
	{
		m_Torrents[i]->UpdateProgressPosition();
	}
}

void TormentWnd::OnTorrentListHdnItemChanged(NMHDR *pNMHDR, LRESULT *pResult)
{
	UpdateTorrentsProgressPosition();
	m_cTorrentList.Invalidate();
}

void TormentWnd::OnTimer(UINT nIDEvent)
{
	switch(nIDEvent)
	{
	case 0:
		Update();
		break;

	case 1:
		CheckAutoLoad();
		break;
	}

	CWnd::OnTimer(nIDEvent);
}

void TormentWnd::KillTorrent(Torrent * t)
{
	DWORD i;

	t->DestroyWindow();

	for(i = 0; i < (DWORD)m_Torrents.GetSize(); i++)
	{
		if(m_Torrents[i] == t)
		{
			MFree(t);
			m_Torrents.RemoveAt(i);
			break;
		}
	}

	UpdateTorrentsProgressPosition();
}

BOOL TormentWnd::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	//NMHDR * hdr;

	//hdr = (NMHDR *)lParam;

	//switch(hdr->code)
	//{
	//case LVN_ENDSCROLL:
	//	UpdateTorrentsProgressPosition();
	//	break;
	//}

	return CWnd::OnNotify(wParam, lParam, pResult);
}

void TormentWnd::Update(void)
{
	DWORD i;
	DWORD BpsIn;
	DWORD BpsOut;
	DWORD AvgBpsIn;
	DWORD AvgBpsOut;
	CString Temp;
	DWORD MaxUploadRate;

	AvgBpsIn = 0;
	AvgBpsOut = 0;
	BpsIn = 0;
	BpsOut = 0;
	MaxUploadRate = Settings::GetDWordValue("MaxUploadRate");
	if(Settings::GetBoolValue("UnlimitedUploadRate")) MaxUploadRate = 0;

	for(i = 0; i < (DWORD)m_Torrents.GetSize(); i++)
	{
		m_Torrents[i]->Update();

		BpsIn += m_Torrents[i]->GetBpsIn();
		BpsOut += m_Torrents[i]->GetBpsOut();
		AvgBpsIn += m_Torrents[i]->GetAvgBpsIn();
		AvgBpsOut += m_Torrents[i]->GetAvgBpsOut();
	}

	m_cTorrentList.SortItems(SortItems, (DWORD_PTR)this);

	SetMaxUploadRate();

	m_Graph.SetNextBlock(BpsIn, BpsOut);
	m_Graph.Invalidate(FALSE);

	UpdateTorrentsProgressPosition();

	Temp.Format(STRTBL_TORMENT_STATUSBPS, FormatBps(BpsIn), FormatBps(BpsOut));
	m_StatusBar.SetPaneText(0, Temp);

	Temp.Format(STRTBL_TORMENT_STATUSAVGBPS, FormatBps(AvgBpsIn), FormatBps(AvgBpsOut));
	m_StatusBar.SetPaneText(1, Temp);

	m_cTorrentList.Invalidate(FALSE);

	if(m_IsInTray)
	{
		if(Shell_NotifyIcon(NIM_MODIFY, &m_NotifyData) == FALSE)
		{
			Shell_NotifyIcon(NIM_ADD, &m_NotifyData);
		}
	}

	MUsage();
}

void TormentWnd::_OnAccept(DataSocket * s, void * Custom, int ErrorCode)
{
	((TormentWnd *)Custom)->OnAccept(ErrorCode);
}

void TormentWnd::_OnClose(DataSocket * s, void * Custom, int ErrorCode)
{
	((TormentWnd *)Custom)->OnClose(s, ErrorCode);
}

void TormentWnd::_OnReceive(DataSocket * s, void * Custom, int ErrorCode)
{
	((TormentWnd *)Custom)->OnReceive(s, ErrorCode);
}

void TormentWnd::OnAccept(int ErrorCode)
{
	DataSocket * s;
//	s = new DataSocket;
	s = MAlloc(DataSocket, 1);

	s->SetCallbacks(NULL, _OnClose, NULL, _OnReceive, NULL, this);

	m_Connections.Add(s);

	m_Socket.Accept(*s);

	TRACE("OnAccept\n");
}

void TormentWnd::OnClose(DataSocket * s, int ErrorCode)
{
	DWORD i;

	s->Close();

	for(i = 0; i < (DWORD)m_Connections.GetSize(); i++)
	{
		if(m_Connections[i] == s)
		{
			m_Connections.RemoveAt(i);
			break;
		}
	}

	s->Close();

	MFree(s);
}

void TormentWnd::OnReceive(DataSocket * s, int ErrorCode)
{
	DWORD i, j;
	char * Data;
	CCriticalSection cs;

	cs.Lock();

	Data = s->GetData();

	if(s->GetDataLength() >= 68)
	{
		for(i = 0; i < (DWORD)m_Torrents.GetSize(); i++)
		{
			if(
				Data[0] == 19 &&
				memcmp(&Data[1], "BitTorrent protocol", 19) == 0 &&
				memcmp(&Data[28], m_Torrents[i]->GetInfoHash(), 20) == 0
				)
			{
				if(m_Torrents[i]->IsDownloading())
				{
					TRACE("Accepted Connection\n");
					for(j = 0; j < (DWORD)m_Connections.GetSize(); j++)
					{
						if(m_Connections[j] == s)
						{
							m_Connections.RemoveAt(j);
							break;
						}
					}

					m_Torrents[i]->AttachClient(s);
					cs.Unlock();
					return;
				}else{
					break;
				}
			}
		}

		for(j = 0; j < (DWORD)m_Connections.GetSize(); j++)
		{
			if(m_Connections[j] == s)
			{
				TRACE("Rejected Connection\n");
				m_Connections.RemoveAt(j);
				s->Close();
				MFree(s);
				cs.Unlock();
				return;
			}
		}
	}

	cs.Unlock();
}

void TormentWnd::OnDropFiles(HDROP hDropInfo)
{
	char FileName[MAX_PATH];

	DWORD NumFiles;
	DWORD i;

	NumFiles = DragQueryFile(hDropInfo, 0xFFFFFFFF, FileName, sizeof(FileName));

	for(i = 0; i < NumFiles; i++)
	{
		DragQueryFile(hDropInfo, i, FileName, sizeof(FileName));

		OpenTorrent(FileName, false);
	}

	CWnd::OnDropFiles(hDropInfo);
}

void TormentWnd::OnHelpAbouttorment()
{
	AboutDlg Dlg;

	Dlg.DoModal();
}

LRESULT TormentWnd::DefWindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
	switch(Msg)
	{
	case WM_USER_TORMENT_RESTOREFROMTRAY:
		switch(lParam)
		{
		case WM_LBUTTONUP:
			Restore();
			break;
		}
		break;

	case WM_USER_TORMENT_LOADTORRENT:
		{
			char TempPath[256];
            
			GetTempPath(sizeof(TempPath), TempPath);

			OpenTorrent(CString(TempPath) + "Temp.torrent", true);
			DeleteFile(CString(TempPath) + "Temp.torrent");
		}
		break;
	}

	return CWnd::DefWindowProc(Msg, wParam, lParam);
}

void TormentWnd::Restore(void)
{
	if(m_IsInTray)
	{
		ShowWindow(SW_SHOW);
		ShowWindow(SW_RESTORE);
		SetFocus();

		DeleteObject(m_NotifyData.hIcon);
		Shell_NotifyIcon(NIM_DELETE, &m_NotifyData);

		m_IsInTray = false;
	}
}

void TormentWnd::OrganizeTorrent(Torrent * t)
{
	DWORD i;
	CArray<CRect> Rects;
	CRect Rect;
	DWORD sw, sh;
	DWORD ww, wh;
	DWORD Passed;

	for(i = 0; i < (DWORD)m_Torrents.GetSize(); i++)
	{
		if(m_Torrents[i] != t)
		{
			m_Torrents[i]->GetWindowRect(&Rect);
			Rects.Add(Rect);
		}
	}

	t->GetWindowRect(&Rect);

	sw = GetSystemMetrics(SM_CXSCREEN);
	sh = GetSystemMetrics(SM_CYSCREEN);

	ww = Rect.Width();
	wh = Rect.Height();

	for(Rect.top = 0; Rect.top < (LONG)(sh - wh); Rect.top ++)
	{
		Rect.bottom = Rect.top + wh;
		for(Rect.left = 0; Rect.left < (LONG)(sw - ww); Rect.left ++)
		{
			Rect.right = Rect.left + ww;

			Passed = 0;
			for(i = 0; i < (DWORD)Rects.GetSize(); i++)
			{
				if(Rect.left >= Rects[i].right) { Passed++; continue; }
				if(Rect.top >= Rects[i].bottom) { Passed++; continue; }
				if(Rect.right <= Rects[i].left) { Passed++; continue; }
				if(Rect.bottom <= Rects[i].top) { Passed++; continue; }
				
				break;
			}

			if(Passed == (DWORD)Rects.GetSize())
			{
				t->MoveWindow(&Rect);
				return;
			}
		}
	}

	t->CenterWindow();
}

void TormentWnd::ResetListener(void)
{
	m_Socket.Close();
	if(m_Socket.Create((WORD)Settings::GetDWordValue("Port")) == 0)
	{
		MessageBox(STRTBL_TORMENT_SOCKETCREATEERROR, STRTBL_TORMENT_ERROR, MB_OK | MB_ICONWARNING);
		return;
	}

	TRACE("Listening on %d\n", (WORD)Settings::GetDWordValue("Port"));

	m_Socket.SetCallbacks(_OnAccept, NULL, NULL, NULL, NULL, this);
	m_Socket.Listen();
}

bool TormentWnd::IsLoaded(char * Hash)
{
	DWORD i;

	for(i = 0; i < (DWORD)m_Torrents.GetSize(); i++)
	{
		if(memcmp(Hash, m_Torrents[i]->GetInfoHash(), 20) == 0) return true;
	}

	return false;
}

bool TormentWnd::OpenTorrent(const char * FileName, bool IgnoreSettings, bool Silent)
{
	Torrent * t;

//	t = new Torrent;
	t = MAlloc(Torrent, 1);

	t->Create(this, &m_cTorrentList);
	OrganizeTorrent(t);
	
	if(t->LoadTorrent(FileName, IgnoreSettings, Silent))
	{
		m_Torrents.Add(t);
		if(Settings::GetBoolValue("StatusOnLoad")) t->ShowWindow(SW_SHOW);
		return true;
	}else{
		t->DestroyWindow();
		MFree(t);
	}

	return false;
}
void TormentWnd::OnViewBandwidthmonitor()
{
	CRect cRect;

	if(m_MainMenu.CheckMenuItem(ID_VIEW_BANDWIDTHMONITOR, MF_UNCHECKED) == MF_CHECKED)
	{
		m_MainMenu.CheckMenuItem(ID_VIEW_BANDWIDTHMONITOR, MF_UNCHECKED);
		Settings::SetBoolValue("ShowBandwidthMonitor", false);
		m_Graph.ShowWindow(SW_HIDE);
	}else{
		m_MainMenu.CheckMenuItem(ID_VIEW_BANDWIDTHMONITOR, MF_CHECKED);
		Settings::SetBoolValue("ShowBandwidthMonitor", true);
		m_Graph.ShowWindow(SW_SHOW);
	}

	GetClientRect(&cRect);
	OnSize(0, cRect.Width(), cRect.Height());
}

void TormentWnd::OnViewLogWindow()
{
	CRect cRect;

	if(m_MainMenu.CheckMenuItem(ID_VIEW_LOGWINDOW, MF_UNCHECKED) == MF_CHECKED)
	{
		m_MainMenu.CheckMenuItem(ID_VIEW_LOGWINDOW, MF_UNCHECKED);
		m_Log.ShowWindow(SW_HIDE);
	}else{
		m_MainMenu.CheckMenuItem(ID_VIEW_LOGWINDOW, MF_CHECKED);
		m_Log.ShowWindow(SW_SHOW);
	}

}

void TormentWnd::SetMaxUploadRate(void)
{
	long i;
	long * Rate;
	long * At;
	long TotalRate;
	long Distribute;
	DWORD Bps;

	if(Settings::GetBoolValue("UnlimitedUploadRate"))
	{
		for(i = 0; i < m_Torrents.GetSize(); i++)
		{
			m_Torrents[i]->SetMaxUploadRate(0);
		}
		return;
	}

	Bps = Settings::GetDWordValue("MaxUploadRate") * 1024;

//	Rate = new long [m_Torrents.GetSize()];
	Rate = MAlloc(long, m_Torrents.GetSize());
//	At = new long [m_Torrents.GetSize()];
	At = MAlloc(long, m_Torrents.GetSize());

	for(i = 0; i < m_Torrents.GetSize(); i++)
	{
		Rate[i] = m_Torrents[i]->GetMaxUploadRate();
		At[i] = m_Torrents[i]->GetAvgBpsOut();
	}

	Distribute = 0;
	for(i = 0; i < m_Torrents.GetSize(); i++)
	{
		if(Rate[i] - At[i] > 0)
		{
			Distribute += (long)((Rate[i] - At[i]) * 0.5f);
			Rate[i] -= (long)((Rate[i] - At[i]) * 0.5f);
		}
	}

	for(i = 0; i < m_Torrents.GetSize(); i++)
	{
		Rate[i] += Distribute / m_Torrents.GetSize();
	}

	TotalRate = 0;
	for(i = 0; i < m_Torrents.GetSize(); i++)
	{
		TotalRate += Rate[i];
	}

	// Make TotalRate = Bps

	for(i = 0; i < m_Torrents.GetSize(); i++)
	{
		if(Rate[i] + ((long)Bps - TotalRate) / m_Torrents.GetSize() > 0)
		{
			Rate[i] += ((long)Bps - TotalRate) / m_Torrents.GetSize();
		}
	}

	TotalRate = 0;
	for(i = 0; i < m_Torrents.GetSize(); i++)
	{
		TotalRate += Rate[i];
//		TRACE("Rate %d: %d/%d\n", i, At[i], Rate[i]);
		m_Torrents[i]->SetMaxUploadRate(Rate[i]);
	}

//	TRACE("TotalRate: %d\n", TotalRate);

	MFree(At);
	MFree(Rate);
}

void TormentWnd::CheckAutoLoad(void)
{
	WIN32_FIND_DATA fd;
	HANDLE hFind;
	CString Temp;
	CString Temp2;

	if(Settings::GetBoolValue("AutoLoad") == false) return;

	Temp = Settings::GetStringValue("AutoLoadDir");
	if(Temp[Temp.GetLength() - 1] != '\\') Temp += "\\";

	hFind = FindFirstFile(Temp + "*.torrent", &fd);

	if(hFind == INVALID_HANDLE_VALUE) return;

//	KillTimer(1);

	do
	{
        OpenTorrent(Temp + fd.cFileName, true, true);
	}while(FindNextFile(hFind, &fd));

	FindClose(hFind);

//	SetTimer(1, AUTOLOAD_INTERVAL, NULL);
}

void TormentWnd::OnViewSelectcolumns()
{
	// TODO: Add your command handler code here
}

void TormentWnd::OnTorrentModifyfilepriorities()
{
	POSITION Pos;
	int i;

	Pos = m_cTorrentList.GetFirstSelectedItemPosition();

	while(Pos)
	{
		i = m_cTorrentList.GetNextSelectedItem(Pos);

		if(((Torrent *)m_cTorrentList.GetItemData(i))->GetMode() == TMODE_DIRECTORY)
		{
			((Torrent *)m_cTorrentList.GetItemData(i))->OnFileSetfilepriorities();
		}
	}
}

int TormentWnd::SortItems(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	DWORD i;
	TormentWnd * Wnd = (TormentWnd *)lParamSort;
	Torrent * Torrent1 = (Torrent *)lParam1;
	Torrent * Torrent2 = (Torrent *)lParam2;

	CString Item1;
	CString Item2;

	for(i = 0; i < 11; i++)
	{
		Item1 = Wnd->m_cTorrentList.GetItemText(Torrent1->GetTorrentListIndex(), m_SortInfo[i]);
		Item2 = Wnd->m_cTorrentList.GetItemText(Torrent2->GetTorrentListIndex(), m_SortInfo[i]);

		switch(m_SortInfo[i])
		{
			// Text
		case 0:
		case 1:
		case 2:
		case 8:
			if(Item1.CompareNoCase(Item2) != 0)
			{
				return Item1.CompareNoCase(Item2);
			}
			break;

		case 3:
			if(Torrent1->GetAvgBpsIn() > Torrent2->GetAvgBpsIn()) return -1;
			if(Torrent1->GetAvgBpsIn() < Torrent2->GetAvgBpsIn()) return 1;
			break;

		case 4:
			if(Torrent1->GetAvgBpsOut() > Torrent2->GetAvgBpsOut()) return -1;
			if(Torrent1->GetAvgBpsOut() < Torrent2->GetAvgBpsOut()) return 1;
			break;

		case 9:
			if(Torrent1->GetBytesSent() > Torrent2->GetBytesSent()) return -1;
			if(Torrent1->GetBytesSent() < Torrent2->GetBytesSent()) return 1;
			break;

		case 10:
			if(Torrent1->GetBytesRecv() > Torrent2->GetBytesRecv()) return -1;
			if(Torrent1->GetBytesRecv() < Torrent2->GetBytesRecv()) return 1;
			break;

			// Numeric
		default:
			if(atof(Item1) > atof(Item2)) return -1;
			if(atof(Item1) < atof(Item2)) return 1;
			break;
		}
	}

	return 0;
}

void TormentWnd::OnHdnTorrentListItemclick(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMHEADER phdr = reinterpret_cast<LPNMHEADER>(pNMHDR);

	DWORD Item, i;

	Item = phdr->iItem;

	for(i = 0; i < 11; i++)
	{
		if(m_SortInfo[i] == Item)
		{
			for(; i > 0; i--)
			{
				m_SortInfo[i] = m_SortInfo[i - 1];
			}
			m_SortInfo[0] = Item;
			break;
		}
	}

	m_cTorrentList.SortItems(SortItems, (DWORD_PTR)this);

	*pResult = 0;
}

void TormentWnd::OnTorrentPause()
{
	POSITION Pos;
	int i;

	Pos = m_cTorrentList.GetFirstSelectedItemPosition();

	while(Pos)
	{
		i = m_cTorrentList.GetNextSelectedItem(Pos);
		((Torrent *)m_cTorrentList.GetItemData(i))->Pause();
	}
}

void TormentWnd::OnTorrentResume()
{
	POSITION Pos;
	int i;

	Pos = m_cTorrentList.GetFirstSelectedItemPosition();

	while(Pos)
	{
		i = m_cTorrentList.GetNextSelectedItem(Pos);
		((Torrent *)m_cTorrentList.GetItemData(i))->Resume();
	}
}
