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
// $Id: TormentWnd.h,v 1.1 2004/03/31 20:39:09 deepb Exp $
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
// TormentWnd.h
//
// Main Window Interface
//

#pragma once

#include "Torrent.h"
#include "DataGraph.h"
#include "DataSocket.h"
#include "LogWnd.h"

#define BALLSTATUS_B		0
#define BALLSTATUS_G		1
#define BALLSTATUS_O		2
#define BALLSTATUS_R		3
#define BALLSTATUS_Y		4
#define BALLSTATUS_PAUSE	5

#define WM_USER_TORMENT_RESTOREFROMTRAY	WM_USER + 1
#define WM_USER_TORMENT_LOADTORRENT		WM_USER + 2

#define AUTOLOAD_INTERVAL	10 * 1000

class TormentWnd : public CWnd
{
	DECLARE_DYNAMIC(TormentWnd)

public:
	TormentWnd();
	virtual ~TormentWnd();

	void KillTorrent(Torrent * t);
	bool IsLoaded(char * Hash);
	bool OpenTorrent(const char * FileName, bool IgnoreSettings, bool Silent = false);

	LogWnd m_Log;

protected:

	void UpdateTorrentsProgressPosition(void);
	void Update(void);
	static void _OnAccept(DataSocket * s, void * Custom, int ErrorCode);
	static void _OnClose(DataSocket * s, void * Custom, int ErrorCode);
	static void _OnReceive(DataSocket * s, void * Custom, int ErrorCode);
	void OnAccept(int ErrorCode);
	void OnClose(DataSocket * s, int ErrorCode);
	void OnReceive(DataSocket * s, int ErrorCode);
	void Restore(void);
	void OrganizeTorrent(Torrent * t);
	void SetMaxUploadRate(void);
	void CheckAutoLoad(void);
	static int CALLBACK SortItems(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

	DECLARE_MESSAGE_MAP()

	CListCtrl m_cTorrentList;
	CMenu m_MainMenu;
	CArray<Torrent *> m_Torrents;
	DataGraph m_Graph;
	DataSocket m_Socket;
	CStatusBar m_StatusBar;
	CArray<DataSocket *> m_Connections;
	NOTIFYICONDATA m_NotifyData;
	bool m_IsInTray;
	CImageList m_TorrentImgList;
	static DWORD m_SortInfo[11];

public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnFileExit();
	afx_msg void OnFileOpentorrent();
	afx_msg void OnToolsSettings();
	afx_msg void OnDestroy();
	afx_msg void OnTorrentListDoubleClick(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTorrentListRightClick(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTorrentShowinfo();
	afx_msg void OnTorrentCanceldownload();
	afx_msg void OnTorrentGetnewclientlist();
	afx_msg void OnTorrentListHdnItemChanged(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnHdnTorrentListItemclick(NMHDR *pNMHDR, LRESULT *pResult);
	void ResetListener(void);
	afx_msg void OnDropFiles(HDROP hDropInfo);
	afx_msg void OnHelpAbouttorment();
	afx_msg void OnViewBandwidthmonitor();
	afx_msg void OnViewLogWindow();
	afx_msg void OnViewSelectcolumns();
	afx_msg void OnTorrentModifyfilepriorities();

protected:
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);
public:
	afx_msg void OnTorrentPause();
	afx_msg void OnTorrentResume();
};