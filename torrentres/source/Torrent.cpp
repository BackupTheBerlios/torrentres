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
// $Id: Torrent.cpp,v 1.1 2004/03/31 20:39:09 deepb Exp $
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
// Torrent.cpp
//
// An Instance of a Loaded Torrent
//

#include "stdafx.h"
#include "Common.h"
#include "Torment.h"
#include "Torrent.h"
#include "Sha1.h"
#include "client.h"
#include "Settings.h"
#include "math.h"
#include "torrent.h"
#include "prioritydlg.h"

#define VERSION "XTORR302-"

IMPLEMENT_DYNAMIC(Torrent, CWnd)

Torrent::BLOCK * Torrent::m_sBlocks = NULL;
bool Torrent::m_sPrioritize;
DWORD Torrent::s_SortInfo[12];

static int CALLBACK browseCallbackProc(HWND hWnd, UINT uMsg, LPARAM /*lp*/, LPARAM pData) {
	switch(uMsg) {
	case BFFM_INITIALIZED: 
		::SendMessage(hWnd, BFFM_SETSELECTION, TRUE, pData );
		break;
	}
	return 0;
}
Torrent::Torrent()
{
	DWORD i;

	m_State = TSTATE_NONE;
	m_BlockHash = NULL;
	m_Blocks = NULL;
	m_hFile = INVALID_HANDLE_VALUE;

	for(i = 0; i < 20; i++)
	{
		m_PeerId[i] = rand() % 256;
	}

	memcpy(m_PeerId, VERSION, strlen(VERSION));

	m_MaxFame = 0;
	m_BpsIn = 0;
	m_BpsOut = 0;
	m_NumUploads = 0;
	m_Rarity = NULL;
	m_Random = NULL;
	m_Sequential = NULL;
	m_Priority = NULL;
	m_IsInTray = false;
	m_LastPiePercent = -1;
	m_BytesUploaded = 0;
	m_BytesDownloaded = 0;

	m_TorrentStartTime = 0;
	m_TorrentLastTime = 0;

	m_NumThreads = 0;
	m_Prioritize = false;
	m_Prioritized = 0;
	m_Paused = false;
}

Torrent::~Torrent()
{
}

BOOL Torrent::Create(CWnd * Parent, CListCtrl * TorrentList)
{
	m_cTorrentList = TorrentList;

	return CreateEx(0, AfxRegisterWndClass(0, 0, 0, AfxGetApp()->LoadIcon(IDR_MAINFRAME)), "Torrent", WS_OVERLAPPEDWINDOW, CRect(0, 0, 500, 200), NULL, 0, NULL);
}

BEGIN_MESSAGE_MAP(Torrent, CWnd)
	ON_WM_DESTROY()
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_COMMAND(ID_VIEW_TORRENTINFO, OnViewTorrentinfo)
	ON_COMMAND(ID_VIEW_BLOCKMAP, OnViewBlockmap)
	ON_COMMAND(ID_VIEW_CLIENTLIST, OnViewClientlist)
	ON_WM_CLOSE()
	ON_WM_TIMER()
	ON_COMMAND(ID_FILE_HIDEWINDOW, OnFileHidewindow)
	ON_COMMAND(ID_FILE_CANCELDOWNLOAD, OnFileCanceldownload)
	ON_COMMAND(ID_FILE_GETNEWCLIENTLIST, OnFileGetnewclientlist)
	ON_COMMAND(ID_FILE_SETFILEPRIORITIES, OnFileSetfilepriorities)
	ON_COMMAND(ID_CLIENT_DISCONNECT, OnClientDisconnect)
	ON_WM_CONTEXTMENU()
	ON_NOTIFY(HDN_ITEMCLICK, 0, OnHdnClientListItemclick)
END_MESSAGE_MAP()


// Silent is only to be used by the AutoLoader
bool Torrent::LoadTorrent(const char * FileName, bool IgnoreSettings, bool Silent)
{
	HANDLE hFile;
	DWORD FileSize;
	char * Buffer;
	char NameBuf[256];
	DWORD Dummy;
	HANDLE hThread;
	DWORD ThreadId;
	char FDrive[_MAX_DRIVE];
	char FPath[_MAX_PATH];
	char FName[_MAX_FNAME];
	char FExt[_MAX_EXT];
	DWORD i, j, k;
	CString Temp;
	CString Temp2;
	FILEINFO fi;
	OPENFILENAME ofn;
	BROWSEINFO bi;
	ITEMIDLIST * iid;
	LPMALLOC m;
	char ErrMsg[1024];

	_splitpath(FileName, FDrive, FPath, FName, FExt);
	m_TorrentPath = CString(FDrive) + FPath;

	// Build m_Info
    hFile = CreateFile(FileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);

	if(hFile == INVALID_HANDLE_VALUE)
	{
//		Log("Error abriendo %s\n", FileName);
		MessageBox(CString("Error Abriendo '") + FileName + "'", "Error", MB_OK | MB_ICONERROR);
		SetStatus("Error");
		return false;
	}

	FileSize = ::GetFileSize(hFile, NULL);

//	Buffer = new char [FileSize];
	Buffer = MAlloc(char, FileSize);

//	Log("Leyendo %s, tamaño: %d\n", FileName, FileSize);
	ReadFile(hFile, Buffer, FileSize, &Dummy, NULL);

	m_Info.Build(Buffer, FileSize);

	MFree(Buffer);

	m_Info.GetInfoHash(m_InfoHash);

	CloseHandle(hFile);

	if(((TormentWnd *)AfxGetMainWnd())->IsLoaded(m_InfoHash))
	{
//		Log("Torrente %s ya cargado\n", FileName);
		sprintf(ErrMsg, STRTBL_TORRENT_FILEALREADYLOADED, FileName);
		if(!Silent) MessageBox(ErrMsg, STRTBL_TORMENT_ERROR, MB_OK | MB_ICONERROR);
		return false;
	}

	if(m_Info.ToString("failure reason") != "")
	{
//		Log("Error en %s, razón: %s\n", FileName, m_Info.ToString("failure reason"));
		sprintf(ErrMsg, STRTBL_TORRENT_TORRENTLOADFAILED, FileName, m_Info.ToString("failure reason"));
		MessageBox(ErrMsg, STRTBL_TORMENT_ERROR, MB_OK | MB_ICONERROR);
		return false;
	}

	if(Settings::GetBoolValue("History") && Silent == false)
	{
		Temp = Settings::GetStringValue("HistoryDir");
		if(Temp[Temp.GetLength() - 1] != '\\') Temp += "\\";
		CreateDirectory(Temp, NULL);
		CopyFile(FileName, Temp + m_Info.ToString("info.name") + ".torrent", TRUE);
		m_TorrentFile = Temp + m_Info.ToString("info.name") + ".torrent";
	}

	if(m_Info.ToInteger("info.files.count") != 0)
	{
		m_Mode = TMODE_DIRECTORY;

		m_TorrentName = m_Info.ToString("info.name");
		if(Settings::GetBoolValue("RemoveUnderscores")) m_TorrentName.Replace("_", " ");

		if(Settings::GetBoolValue("CreateSubDirs")) m_TorrentPath += m_TorrentName + "\\";

		if(Settings::GetBoolValue("UseDefaultSaveDir"))
		{
			m_TorrentPath = Settings::GetStringValue("DefaultSaveDir");
			if(m_TorrentPath[m_TorrentPath.GetLength() - 1] != '\\')
			{
				m_TorrentPath += '\\';
			}

			if(Settings::GetBoolValue("CreateSubDirs")) m_TorrentPath += m_TorrentName + "\\";
		}

		if(Settings::GetBoolValue("AskToSave") || (IgnoreSettings && !Settings::GetBoolValue("UseDefaultSaveDir")))
		{
			SetForegroundWindow();

			ZeroMemory(&bi, sizeof(bi));
			CString csAux;
			CString csSaveDir;
			csAux.Append(STRTBL_TORRENT_DESTINATIONFOLDER);
			csAux.Append("\n");
			csAux.Append(m_TorrentName);

			bi.hwndOwner = *AfxGetMainWnd();
			bi.lpszTitle = (LPSTR)csAux.GetString();
			bi.pszDisplayName = (LPSTR)m_TorrentName.GetString();
			bi.ulFlags = BIF_USENEWUI | BIF_SHAREABLE | BIF_DONTGOBELOWDOMAIN | BIF_RETURNONLYFSDIRS;
			csSaveDir = Settings::GetStringValue("DefaultSaveDir");
			if (csSaveDir.GetLength() > 0)
			{
				bi.lParam = (LPARAM)(csSaveDir.GetBuffer(1));
				bi.lpfn = (BFFCALLBACK)browseCallbackProc;
				csSaveDir.ReleaseBuffer();
			}
			else
			{
				bi.lpfn   = NULL;
				bi.lParam = 0;
			}

			iid = SHBrowseForFolder(&bi);

			if(iid == NULL)
			{
				return false;
			}

			CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
			if(SHGetPathFromIDList(iid, FName) == FALSE) return false;

			m_TorrentPath = CString(FName) + "\\";
			if(Settings::GetBoolValue("CreateSubDirs")) m_TorrentPath += m_TorrentName + "\\";

			SHGetMalloc(&m);
			m->Free(iid);
			m->Release();

			CoUninitialize();
		}

		CreateDirectory(m_TorrentPath, NULL);

		i = m_TorrentInfo.InsertItem(m_TorrentInfo.GetItemCount(), "Directory");
		m_TorrentInfo.SetItemText(i, 1, m_Info.ToString("info.name"));

		m_BlockSize = m_Info.ToInteger("info.piece length");

		m_NumBlocks = 0;
		m_FileSize = 0;
//		m_Priority = new DWORD [m_Info.ToInteger("info.files.count")];
		m_Priority = MAlloc(DWORD, m_Info.ToInteger("info.files.count"));
		for(i = 0; i < m_Info.ToInteger("info.files.count"); i++)
		{
			m_Priority[i] = i;

			fi.m_FileName = "";
			Temp.Format("info.files.%d.path.count", i);
			for(j = 0; j < m_Info.ToInteger(Temp); j++)
			{
				Temp2.Format("info.files.%d.path.%d", i, j);
				fi.m_FileName += m_Info.ToString(Temp2);
				if(j != m_Info.ToInteger(Temp) - 1)
				{
					fi.m_FileName += '\\';
					CreateDirectory(m_TorrentPath + fi.m_FileName, NULL);
				}
			}
			TRACE("File: %s\n", fi.m_FileName);
			if(Settings::GetBoolValue("RemoveUnderscores")) fi.m_FileName.Replace("_", " ");

			Temp.Format("info.files.%d.length", i);
			fi.m_FileSize = m_Info.ToInteger(Temp);
			fi.m_Offset = m_FileSize;
			m_FileSize += fi.m_FileSize;

			fi.m_hFile = INVALID_HANDLE_VALUE;

			if(fi.m_Offset != 0)
			{
				fi.m_FirstBlock = (DWORD)(fi.m_Offset / m_BlockSize);
				fi.m_NumBlocks = (DWORD)(((fi.m_Offset + fi.m_FileSize - 1) / m_BlockSize + 1) - fi.m_FirstBlock);
			}else{
				fi.m_FirstBlock = 0;
				fi.m_NumBlocks = (fi.m_FileSize - 1) / m_BlockSize + 1;
			}
			
			m_Files.Add(fi);

			Temp.Format(STRTBL_TORRENTINFO_BATCHFILENAME, i);
			j = m_TorrentInfo.InsertItem(m_TorrentInfo.GetItemCount(), Temp);
			m_TorrentInfo.SetItemText(j, 1, fi.m_FileName);

			Temp.Format(STRTBL_TORRENTINFO_BATCHFILELENGTH, i);
			Temp2.Format("%d", fi.m_FileSize);
			j = m_TorrentInfo.InsertItem(m_TorrentInfo.GetItemCount(), Temp);
			m_TorrentInfo.SetItemText(j, 1, Temp2 + " (" + FormatBytes(fi.m_FileSize) + ")");
		}

		m_NumBlocks = (DWORD)((m_FileSize - 1) / m_BlockSize + 1);

		m_NumPieces = (m_Info.ToInteger("info.piece length") - 1) / PIECESIZE + 1;

		m_BlockMap.SetNumBlocks(m_NumBlocks);

		for(i = 0; i < (DWORD)m_Files.GetSize(); i++)
		{
			m_BlockMap.SetFile((DWORD)(m_Files[i].m_Offset / m_BlockSize), m_Files[i].m_FileName);
		}

	}else{

		m_Mode = TMODE_FILE;

		m_MainMenu.EnableMenuItem(ID_FILE_SETFILEPRIORITIES, MF_GRAYED);

		// Store some usefull values
		m_TorrentName = m_Info.ToString("info.name");
		if(Settings::GetBoolValue("RemoveUnderscores")) m_TorrentName.Replace("_", " ");

		if(Settings::GetBoolValue("UseDefaultSaveDir"))
		{
			m_TorrentPath = Settings::GetStringValue("DefaultSaveDir");
			if(m_TorrentPath[m_TorrentPath.GetLength() - 1] != '\\')
			{
				m_TorrentPath += '\\';
			}
		}

		if(Settings::GetBoolValue("AskToSave") || (IgnoreSettings && !Settings::GetBoolValue("UseDefaultSaveDir")))
		{
			SetForegroundWindow();

			// Ask the user what to save the file as
			ZeroMemory(&ofn, sizeof(ofn));
			CString csAux;
			csAux.Append(STRTBL_TORRENT_DESTINATIONFILE);
			csAux.Append(" ");
			csAux.Append(m_TorrentName);
			
			ofn.lpstrTitle = (LPSTR)csAux.GetString();
			ofn.hwndOwner = *AfxGetMainWnd();
			ofn.lStructSize = sizeof(ofn);
			ofn.lpstrFile = NameBuf;
			ofn.nMaxFile = sizeof(NameBuf);
			ofn.lpstrFilter = STRTBL_TORRENT_ALLFILESFILTER;

			strcpy(NameBuf, m_TorrentName);

			if(GetSaveFileName(&ofn))
			{
				_splitpath(NameBuf, FDrive, FPath, FName, FExt);

				m_TorrentPath = CString(FDrive) + FPath;
				m_TorrentName = CString(FName) + FExt;
			}else{
				return false;
			}
		}

		m_NumBlocks = (m_Info.ToInteger("info.length") - 1) / m_Info.ToInteger("info.piece length") + 1;
		m_NumPieces = (m_Info.ToInteger("info.piece length") - 1) / PIECESIZE + 1;
		m_BlockSize = m_Info.ToInteger("info.piece length");
		m_FileSize = m_Info.ToInteger("info.length");

		// Fill out TorrentInfo
		i = m_TorrentInfo.InsertItem(m_TorrentInfo.GetItemCount(), STRTBL_TORRENTINFO_FILENAME);
		m_TorrentInfo.SetItemText(i, 1, m_Info.ToString("info.name"));

		i = m_TorrentInfo.InsertItem(m_TorrentInfo.GetItemCount(), STRTBL_TORRENTINFO_FILELENGTH);
		m_TorrentInfo.SetItemText(i, 1, m_Info.ToString("info.length") + " (" + FormatBytes(m_Info.ToInteger("info.length")) + ")");

		m_BlockMap.SetNumBlocks(m_NumBlocks);
		m_BlockMap.SetFile(0, m_TorrentName);
	}

//	m_Blocks = new BLOCK [m_NumBlocks];
	m_Blocks = MAlloc(BLOCK, m_NumBlocks);
//	m_Rarity = new DWORD [m_NumBlocks];
	m_Rarity = MAlloc(DWORD, m_NumBlocks);
//	m_Random = new DWORD [m_NumBlocks];
	m_Random = MAlloc(DWORD, m_NumBlocks);
//	m_Sequential = new DWORD [m_NumBlocks];
	m_Sequential = MAlloc(DWORD, m_NumBlocks);
	for(i = 0; i < m_NumBlocks; i++)
	{
//		m_Blocks[i].m_Pieces = new BYTE [m_NumPieces];
		m_Blocks[i].m_Pieces = MAlloc(BYTE, m_NumPieces);
		for(j = 0; j < m_NumPieces; j++)
		{
			m_Blocks[i].m_Pieces[j] = PIECESTATE_INCOMPLETE;
		}
		m_Rarity[i] = i;
		m_Random[i] = i;
		m_Sequential[i] = i;

		m_Blocks[i].m_Priority = 0;
	}

	// Randomize
	for(i = 0; i < m_NumBlocks; i++)
	{
		j = rand() % m_NumBlocks;
		k = m_Random[i];
		m_Random[i] = m_Random[j];
		m_Random[j] = k;
	}

	if(m_Mode == TMODE_DIRECTORY)
	{
		UpdatePriority();
	}

//	m_BlockHash = new char [m_NumBlocks * 20];
	m_BlockHash = MAlloc(char, m_NumBlocks * 20);
	m_Info.ToBinary("info.pieces", m_BlockHash, m_NumBlocks * 20);

	// Fill out TorrentInfo
	i = m_TorrentInfo.InsertItem(m_TorrentInfo.GetItemCount(), STRTBL_TORRENTINFO_TRACKERURL);
	m_TorrentInfo.SetItemText(i, 1, m_Info.ToString("announce"));

	Temp = "";
	for(i = 0; i < 20; i++)
	{
		Temp2.Format("%.2x", (BYTE)m_InfoHash[i]);
		Temp += Temp2;
	}

	i = m_TorrentInfo.InsertItem(m_TorrentInfo.GetItemCount(), STRTBL_TORRENTINFO_INFOHASH);
	m_TorrentInfo.SetItemText(i, 1, Temp);

	i = m_TorrentInfo.InsertItem(m_TorrentInfo.GetItemCount(), STRTBL_TORRENTINFO_BLOCKSIZE);
	m_TorrentInfo.SetItemText(i, 1, m_Info.ToString("info.piece length") + " (" + FormatBytes(m_Info.ToInteger("info.piece length")) + ")");

	i = m_TorrentInfo.InsertItem(m_TorrentInfo.GetItemCount(), STRTBL_TORRENTINFO_NUMBEROFBLOCKS);
	Temp.Format("%d", m_NumBlocks);
	m_TorrentInfo.SetItemText(i, 1, Temp);

	// Setup stuff
	m_cTorrentList->SetItemText(GetTorrentListIndex(), 0, m_TorrentName);
	SetWindowText(m_TorrentName);

	// Check File
	hThread = CreateThread(NULL, 0, _CheckFile, (LPVOID)this, 0, &ThreadId);
	m_Threads.Add(hThread);

	return true;
}

int Torrent::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	int Index;
	DWORD i;

	Index = m_cTorrentList->InsertItem(m_cTorrentList->GetItemCount(), "");
	m_cTorrentList->SetItemData(Index, (DWORD_PTR)this);

	m_BlockMap.Create(WS_CHILD | WS_VISIBLE | WS_TABSTOP, this);
	m_BlockMap.SetFocus();

	m_TorrentInfo.Create(WS_CHILD | LVS_REPORT | LVS_NOSORTHEADER | WS_TABSTOP, CRect(0, 0, 0, 0), this, 0);
	m_TorrentInfo.ModifyStyleEx(0, WS_EX_CLIENTEDGE);
	m_TorrentInfo.InsertColumn(0, STRTBL_TORRENTINFO_HEADERINFO, LVCFMT_LEFT, 150, 0);
	m_TorrentInfo.InsertColumn(1, STRTBL_TORRENTINFO_HEADERVALUE, LVCFMT_LEFT, 350, 1);

	m_ClientList.Create(WS_CHILD | LVS_REPORT | WS_TABSTOP, CRect(0, 0, 0, 0), this, 0);
	m_ClientList.ModifyStyleEx(0, WS_EX_CLIENTEDGE);
	m_ClientList.InsertColumn(0, STRTBL_CLIENTLIST_IP, LVCFMT_LEFT, 150, 0);
	m_ClientList.InsertColumn(1, STRTBL_CLIENTLIST_STATUS, LVCFMT_LEFT, 100, 1);
	m_ClientList.InsertColumn(2, STRTBL_CLIENTLIST_CHOKING, LVCFMT_LEFT, 30, 2);
	m_ClientList.InsertColumn(3, STRTBL_CLIENTLIST_INTERESTED, LVCFMT_LEFT, 30, 3);
	m_ClientList.InsertColumn(4, STRTBL_CLIENTLIST_BPSIN, LVCFMT_LEFT, 75, 4);
	m_ClientList.InsertColumn(5, STRTBL_CLIENTLIST_BPSOUT, LVCFMT_LEFT, 75, 5);
	m_ClientList.InsertColumn(6, STRTBL_CLIENTLIST_CHOKED, LVCFMT_LEFT, 30, 6);
	m_ClientList.InsertColumn(7, STRTBL_CLIENTLIST_INTERESTEDIN, LVCFMT_LEFT, 30, 7);
	m_ClientList.InsertColumn(8, STRTBL_CLIENTLIST_PERCENTAGE, LVCFMT_LEFT, 60, 8);
	m_ClientList.InsertColumn(9, STRTBL_CLIENTLIST_BYTESRECEIVED, LVCFMT_LEFT, 150, 9);
	m_ClientList.InsertColumn(10, STRTBL_CLIENTLIST_BYTESSENT, LVCFMT_LEFT, 150, 10);
	m_ClientList.InsertColumn(11, STRTBL_CLIENTLIST_CLIENT, LVCFMT_LEFT, 150, 11);

	for(i = 0; i < 12; i++)
	{
		m_SortInfo[i] = i;
	}

	m_ParentProgress.Create(m_cTorrentList);
	m_ParentProgress.SetBar(IDB_PROGRESSBAR_B);
	SetImage(BALLSTATUS_B);

	m_MainMenu.LoadMenu(IDR_TORRENTMENU);
	SetMenu(&m_MainMenu);

	SetTimer(1, 1000 * 10, NULL);	// Choke Timer (10s)
	SetTimer(2, 1000 * 60 * 2, NULL);		// Keep Alive (2m)

	return 0;
}

void Torrent::OnDestroy()
{
	CWnd::OnDestroy();

	DWORD i;

	m_cTorrentList->DeleteItem(GetTorrentListIndex());

	while(m_Threads.GetSize())
	{
		SuspendThread(m_Threads[0]);
		TerminateThread(m_Threads[0], 1);
		m_Threads.RemoveAt(0);
	}

	while(m_Clients.GetSize())
	{
		MFree(m_Clients[0]);
		m_Clients.RemoveAt(0);
	}

	if(m_BlockHash) MFree(m_BlockHash);
	if(m_Blocks)
	{
		for(i = 0; i < m_NumBlocks; i++)
		{
			MFree(m_Blocks[i].m_Pieces);
		}
		MFree(m_Blocks);
	}

	if(m_Rarity) MFree(m_Rarity);
	if(m_Random) MFree(m_Random);
	if(m_Sequential) MFree(m_Sequential);
	if(m_Priority) MFree(m_Priority);

	if(m_hFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hFile);
	}

	while(m_Files.GetSize())
	{
		if(m_Files[0].m_hFile != INVALID_HANDLE_VALUE) CloseHandle(m_Files[0].m_hFile);
		m_Files.RemoveAt(0);
	}

	if(m_IsInTray)
	{
		DeleteObject(m_NotifyData.hIcon);
		Shell_NotifyIcon(NIM_DELETE, &m_NotifyData);
	}
}

int Torrent::GetTorrentListIndex(void)
{
	int i;

	for(i = 0; i < m_cTorrentList->GetItemCount(); i++)
	{
		if((Torrent *)m_cTorrentList->GetItemData(i) == this)
		{
			return i;
		}
	}

	return -1;
}

DWORD WINAPI Torrent::_CheckFile(LPVOID Parameter)
{
	((Torrent *)Parameter)->m_NumThreads++;
	((Torrent *)Parameter)->PreCheckFile();
	((Torrent *)Parameter)->m_NumThreads--;
	return 0;
}

DWORD WINAPI Torrent::_GetTrackerInfo(LPVOID Parameter)
{
	((Torrent *)Parameter)->m_NumThreads++;
	((Torrent *)Parameter)->GetTrackerInfo();
	((Torrent *)Parameter)->m_NumThreads--;
	return 0;
}

void Torrent::PreCheckFile(void)
{
	DWORD i;
	DWORD Count;
	bool Check = true;
	bool WrongSize;
	CMutex MuteEx(0, "CheckFileMutex");
	char ErrMsg[1024];

	SetStatus(STRTBL_PROGRESSSTATUS_WAITING);

	m_State = TSTATE_WAITING;

	MuteEx.Lock();

	m_State = TSTATE_CHECKING;

	if(m_Mode == TMODE_DIRECTORY)
	{
		Count = 0;
		for(i = 0; i < (DWORD)m_Files.GetSize(); i++)
		{
			m_Files[i].m_hFile = CreateFile(m_TorrentPath + m_Files[i].m_FileName, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);

			WrongSize = false;
			if(m_Files[i].m_hFile != INVALID_HANDLE_VALUE)
			{
				if(::GetFileSize(m_Files[i].m_hFile, NULL) != m_Files[i].m_FileSize)
				{
					WrongSize = true;
					CloseHandle(m_Files[i].m_hFile);
				}
			}

			if(m_Files[i].m_hFile == INVALID_HANDLE_VALUE || WrongSize == true)
			{
				if(WrongSize)
				{
					sprintf(ErrMsg, STRTBL_TORMENT_RESIZEFILE, m_Files[i].m_FileName, m_Files[i].m_FileName);

					if(MessageBox(ErrMsg, STRTBL_TORMENT_WARNING, MB_YESNO | MB_ICONWARNING) == IDNO)
					{
						SetStatus(STRTBL_PROGRESSSTATUS_ABORTED);
						return;
					}

					ResizeFile(m_TorrentPath + m_Files[i].m_FileName, m_Files[i].m_FileSize);
				}else{
					Count++;
					if(!PreAllocate(m_Files[i].m_FileName, m_Files[i].m_FileSize, m_Files[i].m_Offset))
					{
						SetStatus(STRTBL_PROGRESSSTATUS_ABORTED);
						return;
					}
				}

				m_Files[i].m_hFile = CreateFile(m_TorrentPath + m_Files[i].m_FileName, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);

				if(m_Files[i].m_hFile == INVALID_HANDLE_VALUE)
				{
					SetStatus(STRTBL_TORMENT_OPENWRITEFAILED, m_Files[i].m_FileName);
					return;
				}
			}

			if(Count == m_Files.GetSize())
			{
				Check = false;
			}
		}
	}else{
		m_hFile = CreateFile(m_TorrentPath + m_TorrentName, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
		
		WrongSize = false;
		if(m_hFile != INVALID_HANDLE_VALUE)
		{
			if(::GetFileSize(m_hFile, NULL) != m_FileSize)
			{
				WrongSize = true;
				CloseHandle(m_hFile);
			}
		}

		if(m_hFile == INVALID_HANDLE_VALUE || WrongSize)
		{
			if(WrongSize)
			{
				sprintf(ErrMsg, STRTBL_TORMENT_RESIZEFILE, m_TorrentName, m_TorrentName);
				if(MessageBox(ErrMsg, STRTBL_TORMENT_WARNING, MB_YESNO | MB_ICONWARNING) == IDNO)
				{
					SetStatus(STRTBL_PROGRESSSTATUS_ABORTED);
					return;
				}

				ResizeFile(m_TorrentPath + m_TorrentName, m_FileSize);
			}else{
				Check = false;
				if(!PreAllocate(m_TorrentName, m_FileSize, 0))
				{
					SetStatus(STRTBL_PROGRESSSTATUS_ABORTED);
					return;
				}
			}

			m_hFile = CreateFile(m_TorrentPath + m_TorrentName, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_ALWAYS, 0, NULL);

			if(m_hFile == INVALID_HANDLE_VALUE)
			{
				SetStatus(STRTBL_TORMENT_OPENWRITEFAILED, m_TorrentPath + m_TorrentName);
				return;
			}
		}
	}

	if(Check) CheckFile();

	if(GetBlocksLeft() == 0)
	{
		ReOpen();
		m_ParentProgress.SetBar(IDB_PROGRESSBAR_O);
		SetImage(BALLSTATUS_O);
		SetStatus(STRTBL_PROGRESSSTATUS_FINISHED);
	}

	m_ParentProgress.SetIdle();

	MuteEx.Unlock();

	GetTrackerInfo();

	m_TorrentStartTime = clock();
	m_TorrentLastTime = m_TorrentStartTime;
}

void Torrent::CheckFile(void)
{
	char * Block;
	DWORD i;
	char Hash[20];
   
	m_State = TSTATE_CHECKING;

	SetStatus(STRTBL_PROGRESSSTATUS_WAITING);

//	Block = new char [m_BlockSize];
	Block = MAlloc(char, m_BlockSize);

	m_ParentProgress.SetRange(m_NumBlocks);
	m_ParentProgress.SetPos(0);

//	Log("Comprobando %s, bloques: %d\n", m_TorrentName.GetString(), m_NumBlocks);

	for(i = 0; i < m_NumBlocks; i++)
	{
		ReadData(Block, i, 0, GetBlockSize(i));

		Sha1((unsigned char *)Hash, (unsigned char *)Block, GetBlockSize(i));

		if(memcmp(Hash, &m_BlockHash[i * 20], 20) == 0)
		{
			m_BlockMap.SetBlockColors(i, BLOCKCOLOR_COMPLETE, 0);
			m_Blocks[i].m_State = PIECESTATE_COMPLETE;
		}else{
			m_BlockMap.SetBlockColors(i, BLOCKCOLOR_INCOMPLETE, 0);
			m_Blocks[i].m_State = PIECESTATE_INCOMPLETE;
		}

		SetStatus(STRTBL_PROGRESSSTATUS_CHECKING, i * 100.0f / m_NumBlocks);

		m_ParentProgress.SetPos(i);

		Sleep(1);
	}

	MFree(Block);
}

bool Torrent::PreAllocate(const char * FileName, QWORD FileSize, QWORD Offset)
{
	DWORD i;
	DWORD FirstBlock;
	DWORD NumBlocks;
	char ErrMsg[1024];
	char FDrive[_MAX_DRIVE];
	char FDir[_MAX_DIR];
	char FName[_MAX_FNAME];
	char FExt[_MAX_EXT];
	CString Path;

	DWORD High;
	DWORD Low;

	SetStatus(STRTBL_PROGRESSSTATUS_PREALLOCATING);

	_splitpath(m_TorrentPath + FileName, FDrive, FDir, FName, FExt);

	Path = FDrive;
	Path += FDir;

	CreateDirectory(Path, NULL);

	m_hFile = CreateFile(m_TorrentPath + FileName, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, 0, NULL);
	NumBlocks = (DWORD)((FileSize - 1) / m_BlockSize + 1);
	FirstBlock = 0;
	if(Offset != 0)
	{
		FirstBlock = (DWORD)((Offset) / m_BlockSize);
		NumBlocks = (DWORD)(((FileSize + Offset - 1) / m_BlockSize + 1) - FirstBlock);
	}

	for(i = 0; i < NumBlocks; i++)
	{
		m_BlockMap.SetBlockColors(FirstBlock + i, BLOCKCOLOR_INCOMPLETE, 0);
		m_Blocks[FirstBlock + i].m_State = PIECESTATE_INCOMPLETE;
	}

	High = HighDWord(FileSize);
	Low = LowDWord(FileSize);

	SetFilePointer(m_hFile, Low, (PLONG)&High, FILE_BEGIN);
	if(SetEndOfFile(m_hFile) == 0)
	{
		int Error = GetLastError();
		switch(Error)
		{
		case ERROR_DISK_FULL:
			sprintf(ErrMsg, STRTBL_TORMENT_DISKFULL, FileName);
			MessageBox(ErrMsg, STRTBL_TORMENT_ERROR, MB_OK | MB_ICONERROR);
			return false;

		default:
			__asm int 3;
			sprintf(ErrMsg, STRTBL_TORMENT_UNKNOWNPREALLOC, GetLastError(), strerror(GetLastError()));
			if(MessageBox(ErrMsg, STRTBL_TORMENT_WARNING, MB_YESNO | MB_ICONWARNING) == IDNO)
			{
				return false;
			}
		}
	}

	CloseHandle(m_hFile);
	m_hFile = INVALID_HANDLE_VALUE;

	return true;
}

void Torrent::SetStatus(const char * Format, ...)
{
	va_list Args;
	char Buffer[1024];

	va_start(Args, Format);
	vsprintf(Buffer, Format, Args);
	va_end(Args);

	m_cTorrentList->SetItemText(GetTorrentListIndex(), 2, Buffer);
}

void Torrent::GetTrackerInfo(void)
{
	CInternetSession Session;
	CString TrackerUrl;
	char Hash[20];
	CHttpFile * File;
	char Buffer[256];
	char * a, * b;
	DWORD Len;
	DWORD Size;
	Lexicon Peers;
	DWORD i, j;
	Client * c;
	bool Found;
	CString Ip;

	if(m_State < TSTATE_GETPEERS) m_State = TSTATE_GETPEERS;

	if(m_State == TSTATE_GETPEERS && GetBlocksLeft() != 0) SetStatus(STRTBL_PROGRESSSTATUS_CONNECTING);

	TrackerUrl =
		m_Info.ToString("announce") +
		"?info_hash=" + HTTPEncode(m_InfoHash, 20) +
		"&peer_id=" + HTTPEncode(m_PeerId, 20) +
		"&port=" + itoa(Settings::GetDWordValue("Port"), Buffer, 10) +
		"&uploaded=" + _i64toa(m_BytesUploaded, Buffer, 10) +
		"&downloaded=" + _i64toa(m_BytesDownloaded, Buffer, 10) +
		"&left=" + itoa(GetBytesLeft(), Buffer, 10);

//DeepB
	if((Settings::GetStringValue("ForcedIP") != "") && (Settings::GetBoolValue("ForceIP")))
	{
		TrackerUrl += "&ip=" + Settings::GetStringValue("ForcedIP");
	}
//END DeepB
	if(GetBytesLeft() != 0 && m_State == TSTATE_GETPEERS)
	{
		TrackerUrl += "&event=started";
	}
	if(GetBytesLeft() == 0 && m_State != TSTATE_GETPEERS && m_State != TSTATE_FINISHED)
	{
		TrackerUrl += "&event=completed";
	}

//	Log("Enviando al tracker %s\n", TrackerUrl);
	TRACE("Tracker Url: %s\n", TrackerUrl);

	try
	{
		File = (CHttpFile *)Session.OpenURL(TrackerUrl, 1, INTERNET_FLAG_TRANSFER_BINARY | INTERNET_FLAG_DONT_CACHE, NULL, 0);

		if(File == NULL) return;

//		a = new char [0];
		a = MAlloc(char, 0);
		Len = 0;
		for(;;)
		{
			Size = File->Read(Buffer, sizeof(Buffer));
			
			if(Size == 0) break;

//			b = new char [Len + Size];
			b = MAlloc(char, Len + Size);
			memcpy(b, a, Len);
			memcpy(&b[Len], Buffer, Size);
			Len += Size;
			MFree(a);
			a = b;
		}

		File->Close();
		MFree(File);

		Peers.Build(a, Len);

		MFree(a);
	}
	catch(CException * e)
	{
		char Error[1024];
		e->GetErrorMessage(Error, sizeof(Error));

		SetStatus(STRTBL_PROGRESSSTATUS_TRACKERFAILED, Error);

		e->Delete();

		m_ParentProgress.SetBar(IDB_PROGRESSBAR_R);
		SetImage(BALLSTATUS_R);

		SetTimer(0, 1000 * 60, NULL);

		if(m_State < TSTATE_DOWNLOAD) m_State = TSTATE_DOWNLOAD;

		return;
	}

	if(Peers.ToString("failure reason") != "")
	{
		SetStatus(STRTBL_PROGRESSSTATUS_TRACKERFAILED, Peers.ToString("failure reason"));
		m_ParentProgress.SetBar(IDB_PROGRESSBAR_R);
		SetImage(BALLSTATUS_R);
		return;
	}

	if(GetBlocksLeft() == 0)
	{
		UpdateProgress();
	}else{
		if(m_State == TSTATE_CHECKING) SetStatus(STRTBL_PROGRESSSTATUS_CONNECTINGTOPEERS);
	}

	if(Peers.ToInteger("interval") != 0)
	{
		SetTimer(0, 1000 * Peers.ToInteger("interval"), NULL);
	}else{
		SetTimer(0, 1000 * 60 * 5, NULL);
	}

	for(i = 0; i < Peers.ToInteger("peers.count"); i++)
	{
		sprintf(Buffer, "peers.%d.ip", i);
		Ip = Peers.ToString(Buffer);

		Found = false;
		for(j = 0; j < (DWORD)m_Clients.GetSize(); j++)
		{
			if(m_Clients[j]->GetIp() == Ip)
			{
				Found = true;
				break;
			}
		}

		if(!Found)
		{
//			c = new Client;
			c = MAlloc(Client, 1);

			c->SetIp(Ip);

			sprintf(Buffer, "peers.%d.peer id", i);
			Peers.ToBinary(Buffer, Hash, 20);
			c->SetPeerId(Hash);
			
			sprintf(Buffer, "peers.%d.port", i);
			c->SetPort((WORD)Peers.ToInteger(Buffer));

			c->SetParent(this);
			c->SetList(&m_ClientList);

			m_Clients.Add(c);
		}
	}

	PostMessage(WM_USER_CONNECTTOCLIENTS);

//	if(GetBlocksLeft() != 0) SetStatus("Success");
	if(m_State == TSTATE_GETPEERS && GetBlocksLeft() != 0) SetStatus(STRTBL_PROGRESSSTATUS_CONNECTINGTOPEERS);

	if(m_State < TSTATE_DOWNLOAD) m_State = TSTATE_DOWNLOAD;
	
	UpdateProgress();
}

void Torrent::UpdateProgress(void)
{
	DWORD i, j;
	QWORD Bytes;
	bool Unavailable;
	bool Attached;

	if(m_State == TSTATE_WAITING)
	{
		SetStatus(STRTBL_PROGRESSSTATUS_WAITING);
	}else if(m_State == TSTATE_CHECKING)
	{
	}else{
		if(m_NumBlocks == 0)
		{
			SetStatus(STRTBL_PROGRESSSTATUS_NOBLOCKS);
		}else{
			Bytes = 0;
			Unavailable = false;
			for(i = 0; i < m_NumBlocks; i++)
			{
				if(m_Blocks[i].m_State == PIECESTATE_COMPLETE)
				{
					Bytes += GetBlockSize(i);
				}else if(m_Blocks[i].m_State == PIECESTATE_DOWNLOADING)
				{
					for(j = 0; j < GetNumPieces(i); j++)
					{
						if(m_Blocks[i].m_Pieces[j] == PIECESTATE_COMPLETE)
						{
							Bytes += GetPieceSize(i, j);
						}
					}
				}else if(m_Blocks[i].m_State == PIECESTATE_INCOMPLETE)
				{
					if(m_Blocks[i].m_Fame == 0) Unavailable = true;
				}
			}

			Attached = false;
			for(i = 0; i < (DWORD)m_Clients.GetSize(); i++)
			{
				if(m_Clients[i]->IsAttached())
				{
					Attached = true;
					break;
				}
			}

			if(Bytes == m_FileSize)
			{
				m_ParentProgress.SetBar(IDB_PROGRESSBAR_O);
				SetImage(BALLSTATUS_O);
				SetStatus(STRTBL_PROGRESSSTATUS_FINISHED);
			}else{
				if(Unavailable)
				{
					m_ParentProgress.SetBar(IDB_PROGRESSBAR_R);
					SetImage(BALLSTATUS_R);
				}else{
					if(Attached)
					{
						m_ParentProgress.SetBar(IDB_PROGRESSBAR_G);
						SetImage(BALLSTATUS_G);
					}else{
						m_ParentProgress.SetBar(IDB_PROGRESSBAR_Y);
						SetImage(BALLSTATUS_Y);
					}
				}
				SetStatus(STRTBL_PROGRESSSTATUS_DOWNLOADING, Bytes * 100.0f / m_FileSize);
			}
			m_ParentProgress.SetRange(m_FileSize);
			m_ParentProgress.SetPos(Bytes);
		}
	}

	if(m_Paused)
	{
		SetImage(BALLSTATUS_PAUSE);
	}

	UpdateProgressPosition();
	UpdateTrayProgress();
}

LRESULT Torrent::DefWindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
{
	switch(Msg)
	{
	case WM_USER_CONNECTTOCLIENTS:
		ConnectToClients();
		break;

	case WM_USER_RESTOREFROMTRAY:
		switch(lParam)
		{
		case WM_LBUTTONUP:
			ShowWindow(SW_SHOW);
			ShowWindow(SW_RESTORE);
			SetFocus();
			Shell_NotifyIcon(NIM_DELETE, &m_NotifyData);
			DeleteObject(m_NotifyData.hIcon);
			m_IsInTray = false;
			m_LastPiePercent = -1;
			break;
		}
		break;
	}

	return CWnd::DefWindowProc(Msg, wParam, lParam);
}

void Torrent::ConnectToClients(void)
{
	DWORD i;
	Client * c;

	for(i = 0; i < (DWORD)m_Clients.GetSize(); i++)
	{
		c = m_Clients[i];

		c->Connect();
	}
}

const char * Torrent::GetInfoHash(void)
{
	return m_InfoHash;
}

const char * Torrent::GetPeerId(void)
{
	return m_PeerId;
}

DWORD Torrent::GetNumBlocks(void)
{
	return m_NumBlocks;
}

DWORD Torrent::GetNumPieces(DWORD Block)
{
	DWORD Result;

	if(Block == m_NumBlocks - 1)
	{
		Result = (DWORD)((((m_FileSize % m_BlockSize) - 1) / PIECESIZE) + 1);
		if(Result > m_NumPieces || Result == 0) return m_NumPieces;
		return Result;
	}

	return m_BlockSize / PIECESIZE;
}

DWORD Torrent::GetBlocksLeft(void)
{
	DWORD i, Count;

	Count = 0;
	for(i = 0; i < m_NumBlocks; i++)
	{
		if(m_Blocks[i].m_State != PIECESTATE_COMPLETE)
		{
			Count++;
		}
	}

	return Count;
}

DWORD Torrent::GetBlockState(DWORD Block)
{
	if(Block >= m_NumBlocks) return PIECESTATE_UNKNOWN;
	return m_Blocks[Block].m_State;
}

DWORD Torrent::GetPieceState(DWORD Block, DWORD Piece)
{
	if(Block >= m_NumBlocks) return PIECESTATE_UNKNOWN;
	if(Piece >= GetNumPieces(Block)) return PIECESTATE_UNKNOWN;

	return m_Blocks[Block].m_Pieces[Piece];
}

DWORD Torrent::GetPieceSize(DWORD Block, DWORD Piece)
{
	DWORD PieceSize;
		
	PieceSize = (DWORD)(m_FileSize - (Block * m_BlockSize + Piece * PIECESIZE));
	if(PieceSize > PIECESIZE) PieceSize = PIECESIZE;

	return PieceSize;
}

DWORD Torrent::GetBlockSize(DWORD Block)
{
	DWORD BlockSize;
		
	BlockSize = (DWORD)(m_FileSize - (Block * m_BlockSize));
	if(BlockSize > m_BlockSize) BlockSize = m_BlockSize;

	return BlockSize;
}

void Torrent::SetPieceState(DWORD Block, DWORD Piece, BYTE State)
{
	DWORD i;
	bool Empty;
	DWORD Count;

	if(Block >= m_NumBlocks) return;
	if(Piece >= GetNumPieces(Block)) return;

	m_Blocks[Block].m_Pieces[Piece] = State;

	Empty = true;
	Count = 0;
	for(i = 0; i < GetNumPieces(Block); i++)
	{
		if(m_Blocks[Block].m_Pieces[i] != PIECESTATE_INCOMPLETE) Empty = false;
		if(m_Blocks[Block].m_Pieces[i] == PIECESTATE_COMPLETE) Count++;
	}

	m_BlockMap.SetBlockPercent(Block, (float)Count / GetNumPieces(Block));
	m_BlockMap.SetBlockMode(Block, BLOCKMAP_SPLIT);

	if(Empty)
	{
		SetBlockState(Block, PIECESTATE_INCOMPLETE);
	}
}

void Torrent::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);

	m_BlockMap.MoveWindow(0, 0, cx, cy);
	m_TorrentInfo.MoveWindow(0, 0, cx, cy);
	m_ClientList.MoveWindow(0, 0, cx, cy);

	if(Settings::GetBoolValue("StatusMinimizeToSystray"))
	{
		if(nType == SIZE_MINIMIZED)
		{
			ShowWindow(SW_HIDE);
			m_LastPiePercent = -1;	// Reset the Pie chart

			ZeroMemory(&m_NotifyData, sizeof(NOTIFYICONDATA));
			m_NotifyData.cbSize = sizeof(NOTIFYICONDATA);
			m_NotifyData.hWnd = *this;
			m_NotifyData.uID = 0;
			m_NotifyData.uFlags = NIF_MESSAGE | NIF_ICON;
			m_NotifyData.uCallbackMessage = WM_USER_RESTOREFROMTRAY;
			m_NotifyData.hIcon = MakePieIcon(0);

			Shell_NotifyIcon(NIM_ADD, &m_NotifyData);
			m_IsInTray = true;

			UpdateTrayProgress();
		}
	}
}

void Torrent::UpdateProgressPosition(void)
{
	CRect iRect;

	m_cTorrentList->GetSubItemRect(GetTorrentListIndex(), 1, LVIR_LABEL, iRect);

	m_ParentProgress.MoveWindow(iRect);
}

void Torrent::AddBlockFame(DWORD Block)
{
	if(Block > m_NumBlocks) return;
	m_Blocks[Block].m_Fame++;

	if(m_Blocks[Block].m_Fame > m_MaxFame)
	{
		UpdateBlockMap();
		return;
	}

	if(m_Blocks[Block].m_State == PIECESTATE_INCOMPLETE && m_MaxFame != 0)
	{
		m_BlockMap.SetBlockColors(Block, BLOCKCOLOR_MINFAME, BLOCKCOLOR_MAXFAME);
		m_BlockMap.SetBlockPercent(Block, m_Blocks[Block].m_Fame / (float)m_MaxFame);
		m_BlockMap.SetBlockMode(Block, BLOCKMAP_FADE);
	}

	if(m_Blocks[Block].m_State == PIECESTATE_COMPLETE && m_MaxFame != 0)
	{
		m_BlockMap.SetBlockColors(Block, BLOCKCOLOR_MINCOMPLETE, BLOCKCOLOR_MAXCOMPLETE);
		m_BlockMap.SetBlockPercent(Block, m_Blocks[Block].m_Fame / (float)m_MaxFame);
		m_BlockMap.SetBlockMode(Block, BLOCKMAP_FADE);
	}
}

void Torrent::RemoveBlockFame(DWORD Block)
{
	DWORD i;
	DWORD Temp;

	if(Block > m_NumBlocks) return;
	m_Blocks[Block].m_Fame--;

	if(m_Blocks[Block].m_Fame == 0 && m_Blocks[Block].m_State == PIECESTATE_INCOMPLETE)
	{
		m_BlockMap.SetBlockColors(Block, BLOCKCOLOR_INCOMPLETE, 0);
		m_BlockMap.SetBlockMode(Block, BLOCKMAP_SOLID);
		m_BlockMap.SetBlockPercent(Block, 0.0f);
	}

	Temp = 0;
	for(i = 0; i < m_NumBlocks; i++)
	{
		if(m_Blocks[i].m_Fame > Temp) Temp = m_Blocks[i].m_Fame;
	}

	if(Temp < m_MaxFame)
	{
		UpdateBlockMap();
		return;
	}

	if(m_Blocks[Block].m_Fame != 0 && (m_Blocks[Block].m_State == PIECESTATE_INCOMPLETE || m_Blocks[Block].m_State == PIECESTATE_COMPLETE))
	{
		m_BlockMap.SetBlockPercent(Block, m_Blocks[Block].m_Fame / (float)m_MaxFame);
	}
}

void Torrent::OnViewTorrentinfo()
{
	m_TorrentInfo.ShowWindow(SW_SHOW);

	m_BlockMap.ShowWindow(SW_HIDE);
	m_ClientList.ShowWindow(SW_HIDE);

	m_MainMenu.CheckMenuItem(ID_VIEW_TORRENTINFO, MF_CHECKED);
	m_MainMenu.CheckMenuItem(ID_VIEW_BLOCKMAP, MF_UNCHECKED);
	m_MainMenu.CheckMenuItem(ID_VIEW_CLIENTLIST, MF_UNCHECKED);

	m_TorrentInfo.SetFocus();
}

void Torrent::OnViewBlockmap()
{
	m_BlockMap.ShowWindow(SW_SHOW);

	m_TorrentInfo.ShowWindow(SW_HIDE);
	m_ClientList.ShowWindow(SW_HIDE);

	m_MainMenu.CheckMenuItem(ID_VIEW_TORRENTINFO, MF_UNCHECKED);
	m_MainMenu.CheckMenuItem(ID_VIEW_BLOCKMAP, MF_CHECKED);
	m_MainMenu.CheckMenuItem(ID_VIEW_CLIENTLIST, MF_UNCHECKED);

	m_BlockMap.SetFocus();
}

void Torrent::OnViewClientlist()
{
	m_ClientList.ShowWindow(SW_SHOW);

	m_BlockMap.ShowWindow(SW_HIDE);
	m_TorrentInfo.ShowWindow(SW_HIDE);

	m_MainMenu.CheckMenuItem(ID_VIEW_TORRENTINFO, MF_UNCHECKED);
	m_MainMenu.CheckMenuItem(ID_VIEW_BLOCKMAP, MF_UNCHECKED);
	m_MainMenu.CheckMenuItem(ID_VIEW_CLIENTLIST, MF_CHECKED);

	m_ClientList.SetFocus();
}

void Torrent::OnClose()
{
	ShowWindow(SW_HIDE);
}

void Torrent::ReadData(char * Data, DWORD Block, DWORD Offset, DWORD Len)
{
	QWORD Pos;
	DWORD i;
	DWORD NumBytes;
	QWORD DataStart;
	DWORD ToRead;

	QWORD q;
	DWORD Low;
	DWORD High;

	if(m_Mode == TMODE_DIRECTORY)
	{
		DataStart = (QWORD)Block * (QWORD)m_BlockSize + Offset;
		Pos = 0;

		for(i = 0; i < (DWORD)m_Files.GetSize(); i++)
		{
			if(DataStart >= m_Files[i].m_Offset && DataStart < m_Files[i].m_Offset + m_Files[i].m_FileSize)
			{
				ToRead = (DWORD)(m_Files[i].m_FileSize - (DataStart - m_Files[i].m_Offset));
				if(ToRead > Len - Pos) ToRead = (DWORD)(Len - Pos);
				q = DataStart - m_Files[i].m_Offset;
				High = HighDWord(q);
				Low = LowDWord(q);
				SetFilePointer(m_Files[i].m_hFile, Low, (PLONG)&High, FILE_BEGIN);
				ReadFile(m_Files[i].m_hFile, &Data[Pos], ToRead, &NumBytes, NULL);

				Pos += NumBytes;
				DataStart += NumBytes;

				if(Pos >= Len) return;
			}
		}
	}else{
		if(m_hFile == INVALID_HANDLE_VALUE) return;

		SetFilePointer(m_hFile, Block * m_BlockSize + Offset, NULL, FILE_BEGIN);
		ReadFile(m_hFile, Data, Len, &NumBytes, NULL);
	}
}

// Note: Offset is the Offset into the Block
void Torrent::WritePiece(BYTE * Data, DWORD Block, DWORD Offset, DWORD Len)
{
	DWORD NumBytes;
	DWORD i;
	QWORD Pos;
	QWORD DataStart;
	DWORD ToWrite;

	QWORD q;
	DWORD High;
	DWORD Low;

	if(m_Mode == TMODE_DIRECTORY)
	{
		DataStart = (QWORD)Block * (QWORD)m_BlockSize + Offset;
		Pos = 0;

		for(i = 0; i < (DWORD)m_Files.GetSize(); i++)
		{
			if(DataStart >= m_Files[i].m_Offset && DataStart < m_Files[i].m_Offset + m_Files[i].m_FileSize)
			{
				ToWrite = (DWORD)(m_Files[i].m_FileSize - (DataStart - m_Files[i].m_Offset));
				if(ToWrite > Len - Pos) ToWrite = (DWORD)(Len - Pos);
				q = DataStart - m_Files[i].m_Offset;
				High = HighDWord(q);
				Low = LowDWord(q);
				SetFilePointer(m_Files[i].m_hFile, Low, (PLONG)&High, FILE_BEGIN);
				WriteFile(m_Files[i].m_hFile, &Data[Pos], ToWrite, &NumBytes, NULL);

				Pos += NumBytes;
				DataStart += NumBytes;

				if(Pos >= Len) return;
			}
		}
	}else{
		if(m_hFile == INVALID_HANDLE_VALUE) return;

		SetFilePointer(m_hFile, Block * m_BlockSize + Offset, NULL, FILE_BEGIN);
		WriteFile(m_hFile, Data, Len, &NumBytes, NULL);
	}
}

bool Torrent::IsBlockComplete(DWORD Block)
{
	char * BlockData;
	char Hash[20];
	bool Result;

//	BlockData = new char [m_BlockSize];
	BlockData = MAlloc(char, m_BlockSize);

	ReadData(BlockData, Block, 0, GetBlockSize(Block));

	Sha1((unsigned char *)Hash, (unsigned char *)BlockData, GetBlockSize(Block));

	if(memcmp(Hash, &m_BlockHash[Block * 20], 20) == 0)
	{
		Result = true;
	}else{
		Result = false;
	}

	MFree(BlockData);

	return Result;
}

void Torrent::SetBlockState(DWORD Block, BYTE State)
{
	if(Block >= m_NumBlocks) return;

	// Todo: Move this to a Function
	switch(State)
	{
	case PIECESTATE_COMPLETE:
		//m_BlockMap.SetBlockPercent(Block, 0.0f);
		//m_BlockMap.SetBlockColors(Block, BLOCKCOLOR_COMPLETE, 0);
		//m_BlockMap.SetBlockMode(Block, BLOCKMAP_SOLID);
		if(m_Blocks[Block].m_Fame != 0)
		{
			m_BlockMap.SetBlockPercent(Block, m_Blocks[Block].m_Fame / (float)m_MaxFame);
			m_BlockMap.SetBlockColors(Block, BLOCKCOLOR_MINCOMPLETE, BLOCKCOLOR_MAXCOMPLETE);
			m_BlockMap.SetBlockMode(Block, BLOCKMAP_FADE);
		}else{
			m_BlockMap.SetBlockPercent(Block, 0.0f);
			m_BlockMap.SetBlockColors(Block, BLOCKCOLOR_COMPLETE, 0);
			m_BlockMap.SetBlockMode(Block, BLOCKMAP_SOLID);
		}
		break;

	case PIECESTATE_DOWNLOADING:
		m_BlockMap.SetBlockColors(Block, BLOCKCOLOR_DOWNLOADING, BLOCKCOLOR_COMPLETE);
		m_BlockMap.SetBlockMode(Block, BLOCKMAP_SPLIT);
		break;

	case PIECESTATE_INCOMPLETE:
		if(m_Blocks[Block].m_Fame != 0)
		{
			m_BlockMap.SetBlockPercent(Block, m_Blocks[Block].m_Fame / (float)m_MaxFame);
			m_BlockMap.SetBlockColors(Block, BLOCKCOLOR_MINFAME, BLOCKCOLOR_MAXFAME);
			m_BlockMap.SetBlockMode(Block, BLOCKMAP_FADE);
		}else{
			m_BlockMap.SetBlockPercent(Block, 0.0f);
			m_BlockMap.SetBlockColors(Block, BLOCKCOLOR_INCOMPLETE, 0);
			m_BlockMap.SetBlockMode(Block, BLOCKMAP_SOLID);
		}
		break;
	}

	m_Blocks[Block].m_State = State;
}

void Torrent::Update(void)
{
	DWORD i;
	int j;
	DWORD Seeds;
	DWORD Partials;
	DWORD Bps;
	float AvgPartials;
	static CCriticalSection Lock;

	CString Temp;

	m_BpsOut = 0;
	m_BpsIn = 0;
	m_AvgBpsOut = 0;
	m_AvgBpsIn = 0;
	Seeds = 0;
	Partials = 0;
	AvgPartials = 0;

	for(i = 0; i < (DWORD)m_Clients.GetSize(); i++)
	{
		for(j = 0; j < m_ClientList.GetItemCount(); j++)
		{
			if((Client *)m_ClientList.GetItemData(j) == m_Clients[i]) break;
		}

		m_Clients[i]->Update(j);

		m_BpsOut += m_Clients[i]->GetBpsOut();
		m_BpsIn += m_Clients[i]->GetBpsIn();
		m_AvgBpsOut += m_Clients[i]->GetAvgBpsOut();
		m_AvgBpsIn += m_Clients[i]->GetAvgBpsIn();

		if(m_Clients[i]->IsSeed())
		{
			Seeds++;
		}else if(m_Clients[i]->IsConnected() && m_Clients[i]->GetState() == CSTATE_ACTIVE)
		{
			Partials++;
			AvgPartials += m_Clients[i]->GetPercentage();
		}
	}

	Lock.Lock();
	for(i = 0; i < 12; i++)
	{
		s_SortInfo[i] = m_SortInfo[i];
	}
	m_ClientList.SortItems(SortClientItems, (DWORD_PTR)this);
	Lock.Unlock();

	if(Partials != 0)
	{
		AvgPartials /= Partials;
	}

	m_cTorrentList->SetItemText(GetTorrentListIndex(), 3, FormatBps(m_AvgBpsIn));
	m_cTorrentList->SetItemText(GetTorrentListIndex(), 4, FormatBps(m_AvgBpsOut));
	
	Temp.Format("%d", Seeds);
	m_cTorrentList->SetItemText(GetTorrentListIndex(), 5, Temp);

	Temp.Format("%d", Partials);
	m_cTorrentList->SetItemText(GetTorrentListIndex(), 6, Temp);

	Temp.Format("%.2f%%", AvgPartials * 100);
	m_cTorrentList->SetItemText(GetTorrentListIndex(), 7, Temp);

	Bps = (DWORD)(m_BytesDownloaded / ((m_TorrentLastTime - (float)m_TorrentStartTime) / 1000));
	if(m_TorrentStartTime != 0 && (clock() - m_TorrentStartTime) != 0 && Bps != 0)
	{
		m_cTorrentList->SetItemText(GetTorrentListIndex(), 8, FormatTime(GetBytesLeft() / Bps));
	}else{
		m_cTorrentList->SetItemText(GetTorrentListIndex(), 8, STRTBL_CANTCALCULATE);
	}

	Temp.Format("%s (%.2f%%)", FormatBytes(m_BytesUploaded), m_BytesUploaded / (float)m_FileSize * 100);
	m_cTorrentList->SetItemText(GetTorrentListIndex(), 9, Temp);
	Temp.Format("%s (%.2f%%)", FormatBytes(m_BytesDownloaded), m_BytesDownloaded / (float)m_FileSize * 100);
	m_cTorrentList->SetItemText(GetTorrentListIndex(), 10, Temp);

	UpdateIcon();
}

void Torrent::KillClient(Client * c)
{
	DWORD i;
	DWORD State;

	for(i = 0; i < (DWORD)m_ClientList.GetItemCount(); i++)
	{
		if((Client *)m_ClientList.GetItemData(i) == c)
		{
			m_ClientList.DeleteItem(i);
			break;
		}
	}

	m_MaxFame = 0;
	for(i = 0; i < (DWORD)m_Clients.GetSize(); i++)
	{
		if(m_Blocks[i].m_Fame > m_MaxFame) m_MaxFame = m_Blocks[i].m_Fame;
	}

	for(i = 0; i < (DWORD)m_Clients.GetSize(); i++)
	{
		if(m_Clients[i] == c)
		{
			State = c->GetState();
			MFree(c);
			m_Clients.RemoveAt(i);
			if(State == CSTATE_ACTIVE)
			{
				UpdateRarity();
				UpdateProgress();
				UpdateBlockMap();
			}

			return;
		}
	}

	// Should not get here
//	Log("Aviso cliente no encontrado\n");
	TRACE("Warning: Client Not Found\n");
}

void Torrent::HaveBlock(DWORD Block)
{
	DWORD i;

	for(i = 0; i < (DWORD)m_Clients.GetSize(); i++)
	{
		m_Clients[i]->Send_Have(Block);
	}
}

DWORD Torrent::GetAvgBpsIn(void)
{
	return m_AvgBpsIn;
}

DWORD Torrent::GetAvgBpsOut(void)
{
	return m_AvgBpsOut;
}

DWORD Torrent::GetBpsIn(void)
{
	return m_BpsIn;
}

DWORD Torrent::GetBpsOut(void)
{
	return m_BpsOut;
}
void Torrent::OnTimer(UINT nIDEvent)
{
	DWORD ThreadId;
	HANDLE hThread;

	switch(nIDEvent)
	{
	case 0:		// Update Traker Info
		KillTimer(0);
		hThread = CreateThread(0, 0, _GetTrackerInfo, (LPVOID)this, 0, &ThreadId);
		m_Threads.Add(hThread);
		break;

	case 1:		// Update Chokes
		UpdateTorrentSends();
		break;

	case 2:		// KeepAlive
		KeepAlive();
		break;
	}

	CWnd::OnTimer(nIDEvent);
}

void Torrent::OnFileHidewindow()
{
	OnClose();
}

void Torrent::OnFileCanceldownload()
{
	((TormentWnd *)AfxGetMainWnd())->KillTorrent(this);
}

void Torrent::OnFileGetnewclientlist()
{
	HANDLE hThread;
	DWORD ThreadId;

	if(m_State < TSTATE_DOWNLOAD) return;

	hThread = CreateThread(0, 0, _GetTrackerInfo, (LPVOID)this, 0, &ThreadId);
	m_Threads.Add(hThread);
}

void Torrent::UpdateTorrentSends(void)
{
	CArray<Client *> Best;
	DWORD i, j;
	DWORD MaxUploads;
	bool b;

	if(m_Paused)
	{
		return;
	}

	MaxUploads = Settings::GetDWordValue("MaxUploads");

	if(m_NumUploads < MaxUploads)
	{
		// Max Uploads not yet reached... Just unchoke the next in the queue
		for(i = 0; i < MaxUploads && m_NumUploads < MaxUploads; i++)
		{
			if(m_SendQueue.GetSize())
			{
				m_SendQueue[0]->Send_UnChoke();
				TRACE("Initial Unchoke\n");
			}
		}

		return;
	}
	
	// Make a List of Best clients that are choked and interested
	for(i = 0; i < (DWORD)m_Clients.GetSize(); i++)
	{
		b = false;
		for(j = 0; j < (DWORD)Best.GetSize(); j++)
		{
			if(m_Clients[i]->GetAvgBpsIn() > Best[j]->GetAvgBpsIn())
			{
				b = true;
				Best.InsertAt(j, m_Clients[i]);
				break;
			}
		}

		if(!b)
		{
			Best.Add(m_Clients[i]);
		}
	}

	j = 0;
	for(i = 0; j < MaxUploads - 1 && i < (DWORD)Best.GetSize(); i++)
	{
		if(Best[i]->IsInterested())
		{
			Best[i]->Send_UnChoke();
			j++;
		}else{
			Best[i]->Send_Choke();
		}
	}

	for(; i < (DWORD)Best.GetSize(); i++)
	{
		Best[i]->Send_Choke();
	}

	m_SendQueue[0]->Send_UnChoke();

	//NumLockedClients = 0;
	//for(i = 0; i < (DWORD)Best.GetSize(); i++)
	//{
	//	if(!Best[i]->IsChoked() && Best[i]->GetTimeUnchoked() < 30 * 1000)
	//	{
	//		NumLockedClients++;
	//	}
	//}

	//NumToUnchoke = MaxUploads - NumLockedClients;
	//j = 0;
	//for(i = 0; i < (DWORD)Best.GetSize() && j < NumToUnchoke; i++)
	//{
	//	if(Best[i]->IsChoked() && Best[i]->IsInterested())
	//	{
	//		Best[i]->Send_UnChoke();
	//		j++;
	//	}
	//}

	//for(k = (DWORD)Best.GetSize() - 1; k >= 0 && m_NumUploads > MaxUploads; k--)
	//{
	//	if(!Best[k]->IsChoked() && Best[k]->GetTimeUnchoked() >= 30 * 1000)
	//	{
	//		Best[k]->Send_Choke();
	//	}
	//}
}

void Torrent::AddToQueue(Client * c)
{
	DWORD i;

	for(i = 0; i < (DWORD)m_SendQueue.GetSize(); i++)
	{
		if(m_SendQueue[i] == c) return;
	}

	m_SendQueue.Add(c);
}

void Torrent::RemoveFromQueue(Client * c)
{
	DWORD i;

	for(i = 0; i < (DWORD)m_SendQueue.GetSize(); i++)
	{
		if(m_SendQueue[i] == c)
		{
			//if(!c->IsChoked())
			//{
			//	c->Send_Choke();
			//}
			m_SendQueue.RemoveAt(i);
			return;
		}
	}
}

QWORD Torrent::GetFileSize(void)
{
	return m_FileSize;
}

void Torrent::UpdateRarity(void)
{
	static CCriticalSection cs;

	cs.Lock();

	m_sBlocks = m_Blocks;
	m_sPrioritize = m_Prioritize;
	qsort(m_Rarity, m_NumBlocks, sizeof(DWORD), CmpRarity);
	qsort(m_Random, m_NumBlocks, sizeof(DWORD), CmpRandom);
	qsort(m_Sequential, m_NumBlocks, sizeof(DWORD), CmpSequential);

	cs.Unlock();
}

int __cdecl Torrent::CmpRarity(const void * a, const void * b)
{
	DWORD A, B;

	A = *(DWORD *)a;
	B = *(DWORD *)b;

	if(m_sPrioritize)
	{
		if(m_sBlocks[A].m_Priority > m_sBlocks[B].m_Priority) return 1;
		if(m_sBlocks[A].m_Priority < m_sBlocks[B].m_Priority) return -1;
	}

	if(m_sBlocks[A].m_Fame > m_sBlocks[B].m_Fame) return 1;
	if(m_sBlocks[A].m_Fame < m_sBlocks[B].m_Fame) return -1;

	if(A > B) return 1;
	return -1;
}

int __cdecl Torrent::CmpRandom(const void * a, const void * b)
{
	DWORD A, B;

	A = *(DWORD *)a;
	B = *(DWORD *)b;

	if(m_sPrioritize)
	{
		if(m_sBlocks[A].m_Priority > m_sBlocks[B].m_Priority) return 1;
		if(m_sBlocks[A].m_Priority < m_sBlocks[B].m_Priority) return -1;
	}

	return 0;
}

int __cdecl Torrent::CmpSequential(const void * a, const void * b)
{
	DWORD A, B;

	A = *(DWORD *)a;
	B = *(DWORD *)b;

	if(m_sPrioritize)
	{
		if(m_sBlocks[A].m_Priority > m_sBlocks[B].m_Priority) return 1;
		if(m_sBlocks[A].m_Priority < m_sBlocks[B].m_Priority) return -1;
	}

	if(A > B) return 1;
	return -1;
}

DWORD Torrent::GetRareBlock(DWORD Block)
{
	return m_Rarity[Block];
}

DWORD Torrent::GetRandomBlock(DWORD Block)
{
	return m_Random[Block];
}

DWORD Torrent::GetSequentialBlock(DWORD Block)
{
	return m_Sequential[Block];
}

void Torrent::SetMaxUploadRate(DWORD Bps)
{
	long i, j;
	long * Rate;
	long * At;
	long * Clients;
	long TotalRate;
	long Distribute;

    if(m_NumUploads == 0) return;
	if(Bps == 0)
	{
		for(i = 0; i < m_Clients.GetSize(); i++)
		{
			m_Clients[i]->SetMaxUploadRate(0);
		}
		return;
	}

//	Rate = new long [m_NumUploads];
	Rate = MAlloc(long, m_NumUploads);
//	Clients = new long [m_NumUploads];
	Clients = MAlloc(long, m_NumUploads);
//	At = new long [m_NumUploads];
	At = MAlloc(long, m_NumUploads);

	j = 0;
	for(i = 0; i < m_Clients.GetSize() && j < (long)m_NumUploads; i++)
	{
		if(!m_Clients[i]->IsChoked())
		{
			Rate[j] = m_Clients[i]->GetMaxUploadRate();
			At[j] = m_Clients[i]->GetAvgBpsOut();
			Clients[j] = i;
			j++;
		}
	}

	if(j == 0) return;

	Distribute = 0;
	for(i = 0; i < j; i++)
	{
		if(Rate[i] - At[i] > 0)
		{
			Distribute += (long)((Rate[i] - At[i]) * 0.5f);
			Rate[i] -= (long)((Rate[i] - At[i]) * 0.5f);
		}
	}

	for(i = 0; i < j; i++)
	{
		Rate[i] += Distribute / j;
	}

	TotalRate = 0;
	for(i = 0; i < j; i++)
	{
		TotalRate += Rate[i];
	}

	// Make TotalRate = Bps

	for(i = 0; i < j; i++)
	{
		if(Rate[i] + ((long)Bps - TotalRate) / j > 0)
		{
			Rate[i] += ((long)Bps - TotalRate) / j;
		}
	}

	TotalRate = 0;
	for(i = 0; i < j; i++)
	{
		TotalRate += Rate[i];
//		TRACE("Rate %d: %d/%d\n", i, At[i], Rate[i]);
		m_Clients[Clients[i]]->SetMaxUploadRate(Rate[i]);
	}

//	TRACE("TotalRate: %d\n", TotalRate);

	MFree(At);
	MFree(Clients);
	MFree(Rate);
}

void Torrent::NumUploadsInc(void)
{
	m_NumUploads++;
}

void Torrent::NumUploadsDec(void)
{
	m_NumUploads--;
}

void Torrent::AttachClient(DataSocket * s)
{
	Client * c;
	char Hash[20];
	char * Data;
	DWORD i;

	Data = s->GetData();

//	c = new Client;
	c = MAlloc(Client, 1);

	c->SetIp(s->GetIp());
	c->SetPort(s->GetPort());

	memcpy(Hash, &Data[48], 20);
	c->SetPeerId(Hash);
	
	c->SetParent(this);
	c->SetList(&m_ClientList);

	c->Attach(s);

	for(i = 0; i < (DWORD)m_Clients.GetSize(); i++)
	{
		if(strcmp(m_Clients[i]->GetIp(), s->GetIp()) == 0 && m_Clients[i]->IsConnected() == true)
		{
			m_Clients.Add(c);
			KillClient(c);
			return;
		}
	}

	m_Clients.Add(c);
}

bool Torrent::IsDownloading(void)
{
	if(m_State >= TSTATE_DOWNLOAD) return true;
	return false;
}

DWORD Torrent::GetIncompleteBlocks(void)
{
	DWORD i, Count;

	Count = 0;
	for(i = 0; i < m_NumBlocks; i++)
	{
		if(m_Blocks[i].m_State == PIECESTATE_INCOMPLETE)
		{
			Count++;
		}
	}

	return Count;
}

void Torrent::HavePiece(DWORD Block, DWORD Piece)
{
	DWORD i;

	for(i = 0; i < (DWORD)m_Clients.GetSize(); i++)
	{
		m_Clients[i]->Send_Cancel(Block, Piece, true);
	}
}

DWORD Torrent::GetNumClients(void)
{
	return m_Clients.GetSize();
}

DWORD Torrent::GetNumCachedBlocks(void)
{
	return m_BlockCache.GetSize();
}

DWORD Torrent::GetCachedBlock(DWORD Block)
{
	return m_BlockCache[Block];
}

void Torrent::CacheBlock(DWORD Block)
{
	DWORD i;
	bool Found;

	Found = false;
	for(i = 0; i < (DWORD)m_BlockCache.GetSize(); i++)
	{
		if(m_BlockCache[i] == Block)
		{
			Found = true;
			break;
		}
	}

	if(!Found)
	{
		m_BlockCache.Add(Block);
	}
}

void Torrent::UnCacheBlock(DWORD Block)
{
	DWORD i;

	for(i = 0; i < (DWORD)m_BlockCache.GetSize(); i++)
	{
		if(m_BlockCache[i] == Block)
		{
			m_BlockCache.RemoveAt(i);
			return;
		}
	}
}

void Torrent::KeepAlive(void)
{
	DWORD i;

	for(i = 0; i < (DWORD)m_Clients.GetSize(); i++)
	{
		m_Clients[i]->Send_KeepAlive();
	}
}

HICON Torrent::MakePieIcon(float Percent)
{
	ICONINFO ii;
	HICON Result;

	BYTE * And;
	BYTE * Xor;

//	And = new BYTE [16 * 16 / 8];
	And = MAlloc(BYTE, 16 * 16 / 8);
//	Xor = new BYTE [16 * 16 * 4];
	Xor = MAlloc(BYTE, 16 * 16 * 4);
	int x, y, z, r;

	memset(And, 0, 16 * 16 / 8);
	memset(Xor, 0xFF, 16 * 16 * 4);

	HBITMAP Clr;
	HBITMAP Mask;

	for(y = 0; y < 16; y++)
	{
		for(x = 0; x < 16; x++)
		{
			if((x - 7.5f) * (x - 7.5f) + (y - 7.5f) * (y - 7.5f) < 7 * 7)
			{
				SetIconPixel(x, y, 0xFF, 0x00, 0x00, true, And, Xor);
			}else{
				SetIconPixel(x, y, 0, 0, 0, false, And, Xor);
			}
		}
	}

	for(r = 0; r < 360 * Percent; r += 2)
	{
		for(z = 0; z < 8; z++)
		{
			x = Round(-cos((r - 90) * 3.1415f / 180) * z + 7.5f);
			y = Round(sin((r - 90) * 3.1415f / 180) * z + 7.5f);

			SetIconPixel(x, y, 0x00, 0xFF, 0x00, true, And, Xor);
		}
	}

	Mask = CreateBitmap(16, 16, 1, 1, And);
	Clr = CreateBitmap(16, 16, 1, 32, Xor);

	ii.fIcon = TRUE;
	ii.hbmColor = Clr;
	ii.hbmMask = Mask;
	ii.xHotspot = 0;
	ii.yHotspot = 0;

	Result = CreateIconIndirect(&ii);

	DeleteObject(Clr);
	DeleteObject(Mask);

	MFree(Xor);
	MFree(And);

	return Result;
}

void Torrent::SetIconPixel(int x, int y, BYTE r, BYTE g, BYTE b, bool Solid, BYTE * And, BYTE * Xor)
{
	Xor[(x + y * 16) * 4 + 0] = b;
	Xor[(x + y * 16) * 4 + 1] = g;
	Xor[(x + y * 16) * 4 + 2] = r;

	if(!Solid)
	{
		And[(x + y * 16) / 8] |= 1 << (7 - (x + y * 16) % 8);
		Xor[(x + y * 16) * 4 + 3] = 0x00;
	}else{
		And[(x + y * 16) / 8] &= ~(1 << (7 - (x + y * 16) % 8));
		Xor[(x + y * 16) * 4 + 3] = 0xFF;
	}
}

void Torrent::UpdateTrayProgress(void)
{
	float Percent;
	DWORD Size;
	DWORD i;
	CString Temp;

	if(!m_IsInTray) return;

	Size = 0;
	if(m_State >= TSTATE_DOWNLOAD)
	{
		for(i = 0; i < m_NumBlocks; i++)
		{
			if(m_Blocks[i].m_State == PIECESTATE_COMPLETE)
			{
				Size += GetBlockSize(i);
			}
		}
	}

	Percent = Size / (float)GetFileSize();

	if(Round(Percent * 100) != m_LastPiePercent)
	{
		m_LastPiePercent = Round(Percent * 100);
		DeleteObject(m_NotifyData.hIcon);
		m_NotifyData.hIcon = MakePieIcon(Percent);
	}

	Temp.Format("%.2f%% ", Percent * 100.0f);
	if(Temp.GetLength() + m_TorrentName.GetLength() > 128)
	{
		Temp += m_TorrentName.Left(128 - Temp.GetLength() - 4) + "...";
	}else{
		Temp += m_TorrentName;
	}
	strncpy(m_NotifyData.szTip, Temp, sizeof(m_NotifyData.szTip) - 1);
	m_NotifyData.uFlags |= NIF_TIP;

	Shell_NotifyIcon(NIM_MODIFY, &m_NotifyData);
}

void Torrent::AddBytesUploaded(DWORD Bytes)
{
	m_BytesUploaded += Bytes;
}

void Torrent::AddBytesDownloaded(DWORD Bytes)
{
	m_BytesDownloaded += Bytes;
	m_TorrentLastTime = clock();
}

void Torrent::Restore(void)
{
	ShowWindow(SW_SHOW);
	ShowWindow(SW_RESTORE);
	SetFocus();
	
	if(m_IsInTray)
	{
		DeleteObject(m_NotifyData.hIcon);
		Shell_NotifyIcon(NIM_DELETE, &m_NotifyData);
	}
}

DWORD Torrent::GetBytesLeft(void)
{
	DWORD Result;
	DWORD i, j;

	Result = 0;
	for(i = 0; i < m_NumBlocks; i++)
	{
		if(m_Blocks[i].m_State == PIECESTATE_INCOMPLETE)
		{
			Result += GetBlockSize(i);
		}else if(m_Blocks[i].m_State == PIECESTATE_DOWNLOADING)
		{
			for(j = 0; j < GetNumPieces(i); j++)
			{
				if(GetPieceState(i, j) != PIECESTATE_COMPLETE)
				{
					Result += GetPieceSize(i, j);
				}
			}
		}
	}

	return Result;
}

void Torrent::ReOpen(void)
{
	DWORD i;
	CString Temp;

	if(m_Mode == TMODE_DIRECTORY)
	{
		for(i = 0; i < (DWORD)m_Files.GetSize(); i++)
		{
			CloseHandle(m_Files[i].m_hFile);
			m_Files[i].m_hFile = CreateFile(m_TorrentPath + m_Files[i].m_FileName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
		}
	}else{
		CloseHandle(m_hFile);
		m_hFile = CreateFile(m_TorrentPath + m_TorrentName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	}

	m_State = TSTATE_FINISHED;
	m_MainMenu.ModifyMenu(ID_FILE_CANCELDOWNLOAD, MF_STRING, ID_FILE_CANCELDOWNLOAD, STRTBL_MENU_FINISH);
}

void Torrent::OnFileSetfilepriorities()
{
	PriorityDlg Dlg;
	DWORD i;

	for(i = 0; i < (DWORD)m_Files.GetSize(); i++)
	{
		Dlg.m_Files.Add(m_Files[i].m_FileName);
		Dlg.m_FileSizes.Add(m_Files[i].m_FileSize);
	}

	Dlg.m_Priorities = m_Priority;
	Dlg.m_Enable = m_Prioritize ? TRUE : FALSE;
	Dlg.m_Prioritized = m_Prioritized;

	if(Dlg.DoModal() == IDOK)
	{
		m_Prioritized = Dlg.m_Prioritized;
		if(Dlg.m_Enable)
		{
			m_Prioritize = Dlg.m_Enable ? true : false;
			UpdatePriority();
            UpdateRarity();
		}
	}
}

void Torrent::UpdatePriority(void)
{
	long i;
	QWORD j;

	for(i = m_Files.GetSize() - 1; i >= 0; i--)
	{
		TRACE("%d: %d-%d %d\n", i, m_Files[m_Priority[i]].m_FirstBlock, m_Files[m_Priority[i]].m_NumBlocks, m_Priority[i]);
		for(j = m_Files[m_Priority[i]].m_FirstBlock; j < m_Files[m_Priority[i]].m_FirstBlock + m_Files[m_Priority[i]].m_NumBlocks; j++)
		{
			m_Blocks[j].m_Priority = i;
		}
	}
}

void Torrent::OnClientDisconnect()
{
	POSITION Pos;
	CArray<Client *> c;
	int i;

	Pos = m_ClientList.GetFirstSelectedItemPosition();

	if(Pos == NULL) return;

	do
	{
		i = m_ClientList.GetNextSelectedItem(Pos);

		c.Add((Client *)m_ClientList.GetItemData(i));
	}while(Pos);

	for(i = 0; i < c.GetSize(); i++)
	{
		KillClient(c[i]);
	}
}

void Torrent::OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/)
{
	CMenu Menu;
	CMenu * Sub;
	CPoint pt;
	POSITION Pos;
	if(m_ClientList.IsWindowVisible())
	{

		GetCursorPos(&pt);

		Pos = m_ClientList.GetFirstSelectedItemPosition();

		Menu.LoadMenu(IDR_CONTEXTMENU);
		Sub = Menu.GetSubMenu(1);

		if(Pos == NULL)
		{
			Sub->EnableMenuItem(ID_CLIENT_DISCONNECT, MF_GRAYED);
		}

		Sub->TrackPopupMenu(TPM_RIGHTBUTTON | TPM_LEFTBUTTON, pt.x, pt.y, this);
	}
}

DWORD Torrent::GetMaxUploadRate(void)
{
	long i, j;
	DWORD Result;

	Result = 0;
	j = 0;
	for(i = 0; i < m_Clients.GetSize() && j < (long)m_NumUploads; i++)
	{
		if(!m_Clients[i]->IsChoked())
		{
			Result += m_Clients[i]->GetMaxUploadRate();
		}
	}

	return Result;
}

bool Torrent::ResizeFile(const char * FileName, QWORD FileSize)
{
	char ErrMsg[1024];

	DWORD High;
	DWORD Low;

	m_hFile = CreateFile(FileName, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, 0, NULL);

	if(m_hFile == INVALID_HANDLE_VALUE)
	{
		return false;
	}

	High = HighDWord(FileSize);
	Low = LowDWord(FileSize);

	SetFilePointer(m_hFile, Low, (PLONG)&High, FILE_BEGIN);
	if(SetEndOfFile(m_hFile) == 0)
	{
		int Error = GetLastError();
		switch(Error)
		{
		case ERROR_DISK_FULL:
			CloseHandle(m_hFile);
			sprintf(ErrMsg, STRTBL_TORMENT_DISKFULL, FileName);
			MessageBox(ErrMsg, STRTBL_TORMENT_ERROR, MB_OK | MB_ICONERROR);
			return false;

		default:
			sprintf(ErrMsg, STRTBL_TORMENT_UNKNOWNRESIZE, GetLastError(), strerror(GetLastError()));
			if(MessageBox(ErrMsg, STRTBL_TORMENT_WARNING, MB_YESNO | MB_ICONWARNING) == IDNO)
			{
				CloseHandle(m_hFile);
				return false;
			}
		}
	}

	CloseHandle(m_hFile);
	m_hFile = INVALID_HANDLE_VALUE;

	return true;
}

void Torrent::SetImage(DWORD Img)
{
	LVITEM i;

	ZeroMemory(&i, sizeof(i));

	i.iItem = GetTorrentListIndex();
	i.mask = LVIF_IMAGE;
	i.iImage = Img;

	m_cTorrentList->SetItem(&i);
}

DWORD Torrent::GetMode(void)
{
	return m_Mode;
}

void Torrent::OnHdnClientListItemclick(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMHEADER phdr = reinterpret_cast<LPNMHEADER>(pNMHDR);

	DWORD Item, i;

	Item = phdr->iItem;

	for(i = 0; i < 12; i++)
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

	m_ClientList.SortItems(SortClientItems, (DWORD_PTR)this);

	*pResult = 0;
}

int Torrent::SortClientItems(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	DWORD i, j, k;
	Torrent * Wnd = (Torrent *)lParamSort;

	CString Item1;
	CString Item2;
	CString Item1Tok;
	CString Item2Tok;
	int Item1Pos = 0;
	int Item2Pos = 0;

	for(i = 0; i < 11; i++)
	{

		for(j = 0; j < (DWORD)Wnd->m_ClientList.GetItemCount(); j++)
		{
			if((Client *)Wnd->m_ClientList.GetItemData(j) == (Client *)lParam1) break;
		}

		for(k = 0; k < (DWORD)Wnd->m_ClientList.GetItemCount(); k++)
		{
			if((Client *)Wnd->m_ClientList.GetItemData(k) == (Client *)lParam2) break;
		}

		Item1 = Wnd->m_ClientList.GetItemText(j, s_SortInfo[i]);
		Item2 = Wnd->m_ClientList.GetItemText(k, s_SortInfo[i]);

		switch(s_SortInfo[i])
		{
			// Text
		case 1:
		case 2:
		case 3:
		case 6:
		case 7:
		case 11:
			if(Item1.CompareNoCase(Item2) != 0)
			{
				return Item1.CompareNoCase(Item2);
			}
			break;

			// Specialized
		case 0:	// Ip
			for(j = 0; j < 4; j++)
			{
                Item1Tok = Item1.Tokenize(".", Item1Pos);
                Item2Tok = Item2.Tokenize(".", Item2Pos);

				if(atoi(Item1Tok) > atoi(Item2Tok)) return 1;
				if(atoi(Item1Tok) < atoi(Item2Tok)) return -1;
			}
			break;
		case 4:	// Avg Bps In
			if(((Client *)lParam1)->GetAvgBpsIn() > ((Client *)lParam2)->GetAvgBpsIn()) return -1;
			if(((Client *)lParam1)->GetAvgBpsIn() < ((Client *)lParam2)->GetAvgBpsIn()) return 1;
			break;

		case 5:	// Avg Bps Out
			if(((Client *)lParam1)->GetAvgBpsOut() > ((Client *)lParam2)->GetAvgBpsOut()) return -1;
			if(((Client *)lParam1)->GetAvgBpsOut() < ((Client *)lParam2)->GetAvgBpsOut()) return 1;
			break;

		case 9:	// Bytes Received
			if(((Client *)lParam1)->GetBytesDownloaded() > ((Client *)lParam2)->GetBytesDownloaded()) return -1;
			if(((Client *)lParam1)->GetBytesDownloaded() < ((Client *)lParam2)->GetBytesDownloaded()) return 1;
			break;

		case 10:	// Bytes Sent
			if(((Client *)lParam1)->GetBytesUploaded() > ((Client *)lParam2)->GetBytesUploaded()) return -1;
			if(((Client *)lParam1)->GetBytesUploaded() < ((Client *)lParam2)->GetBytesUploaded()) return 1;
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

void Torrent::OnFinish(void)
{
	m_BlockCache.RemoveAll();
	ReOpen();
}

void Torrent::UpdateBlockMap(void)
{
	DWORD i;

	m_MaxFame = 0;
	for(i = 0; i < m_NumBlocks; i++)
	{
		if(m_Blocks[i].m_Fame > m_MaxFame) m_MaxFame = m_Blocks[i].m_Fame;
	}

	for(i = 0; i < m_NumBlocks; i++)
	{
		switch(m_Blocks[i].m_State)
		{
		case PIECESTATE_COMPLETE:
//			m_BlockMap.SetBlockMode(i, BLOCKMAP_SOLID);
//			m_BlockMap.SetBlockColors(i, BLOCKCOLOR_COMPLETE, 0);
			if(m_Blocks[i].m_Fame > 0 && m_MaxFame > 0)
			{
				m_BlockMap.SetBlockPercent(i, m_Blocks[i].m_Fame / (float)m_MaxFame);
				m_BlockMap.SetBlockColors(i, BLOCKCOLOR_MINCOMPLETE, BLOCKCOLOR_MAXCOMPLETE);
				m_BlockMap.SetBlockMode(i, BLOCKMAP_FADE);
			}else{
				m_BlockMap.SetBlockPercent(i, 0.0f);
				m_BlockMap.SetBlockMode(i, BLOCKMAP_SOLID);
				m_BlockMap.SetBlockColors(i, BLOCKCOLOR_COMPLETE, 0);
			}
			break;

		case PIECESTATE_INCOMPLETE:
			if(m_Blocks[i].m_Fame > 0 && m_MaxFame > 0)
			{
				m_BlockMap.SetBlockPercent(i, m_Blocks[i].m_Fame / (float)m_MaxFame);
				m_BlockMap.SetBlockColors(i, BLOCKCOLOR_MINFAME, BLOCKCOLOR_MAXFAME);
				m_BlockMap.SetBlockMode(i, BLOCKMAP_FADE);
			}else{
				m_BlockMap.SetBlockPercent(i, 0.0f);
				m_BlockMap.SetBlockMode(i, BLOCKMAP_SOLID);
				m_BlockMap.SetBlockColors(i, BLOCKCOLOR_INCOMPLETE, 0);
			}
			break;
		}
	}
}

QWORD Torrent::GetBytesSent(void)
{
	return m_BytesUploaded;
}

QWORD Torrent::GetBytesRecv(void)
{
	return m_BytesDownloaded;
}

void Torrent::Pause(void)
{
	DWORD i;
	
	m_Paused = true;

	for(i = 0; i < (DWORD)m_Clients.GetSize(); i++)
	{
		m_Clients[i]->Pause();
	}
}

void Torrent::Resume(void)
{
	DWORD i;
	
	m_Paused = false;

	for(i = 0; i < (DWORD)m_Clients.GetSize(); i++)
	{
		m_Clients[i]->Resume();
	}
}

bool Torrent::IsPaused(void)
{
	return m_Paused;
}

void Torrent::UpdateIcon(void)
{
	DWORD i, j;
	QWORD Bytes;
	bool Unavailable;
	bool Attached;

	if(m_State == TSTATE_WAITING)
	{
	}else if(m_State == TSTATE_CHECKING)
	{
	}else{
		if(m_NumBlocks == 0)
		{
		}else{
			Bytes = 0;
			Unavailable = false;
			for(i = 0; i < m_NumBlocks; i++)
			{
				if(m_Blocks[i].m_State == PIECESTATE_COMPLETE)
				{
					Bytes += GetBlockSize(i);
				}else if(m_Blocks[i].m_State == PIECESTATE_DOWNLOADING)
				{
					for(j = 0; j < GetNumPieces(i); j++)
					{
						if(m_Blocks[i].m_Pieces[j] == PIECESTATE_COMPLETE)
						{
							Bytes += GetPieceSize(i, j);
						}
					}
				}else if(m_Blocks[i].m_State == PIECESTATE_INCOMPLETE)
				{
					if(m_Blocks[i].m_Fame == 0) Unavailable = true;
				}
			}

			Attached = false;
			for(i = 0; i < (DWORD)m_Clients.GetSize(); i++)
			{
				if(m_Clients[i]->IsAttached())
				{
					Attached = true;
					break;
				}
			}

			if(Bytes == m_FileSize)
			{
				SetImage(BALLSTATUS_O);
			}else{
				if(Unavailable)
				{
					SetImage(BALLSTATUS_R);
				}else{
					if(Attached)
					{
						SetImage(BALLSTATUS_G);
					}else{
						SetImage(BALLSTATUS_Y);
					}
				}
				SetStatus(STRTBL_PROGRESSSTATUS_DOWNLOADING, Bytes * 100.0f / m_FileSize);
			}
		}
	}

	if(m_Paused)
	{
		SetImage(BALLSTATUS_PAUSE);
	}
}