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
// $Id: Torrent.h,v 1.1 2004/03/31 20:39:09 deepb Exp $
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
// Torrent.h
//
// An Instance of a Loaded Torrent
//

#pragma once

#include "Lexicon.h"
#include "BlockMap.h"
#include "ProgressBar.h"
#include "DataSocket.h"

#define TSTATE_NONE				0
#define TSTATE_CHECKING			1
#define TSTATE_WAITING			2
#define TSTATE_GETPEERS			3
#define TSTATE_DOWNLOAD			4
#define TSTATE_FINISHED			5

#define PIECESIZE				(16 * 1024)
#define PIECETIMEOUT			(1000 * 60 * 2)

#define PIECESTATE_UNKNOWN		0
#define PIECESTATE_INCOMPLETE	1
#define PIECESTATE_DOWNLOADING	2
#define PIECESTATE_COMPLETE		3

#define WM_USER_CONNECTTOCLIENTS	WM_USER + 1
#define WM_USER_RESTOREFROMTRAY		WM_USER + 2

#define BLOCKCOLOR_COMPLETE		RGB(0xC0, 0xFF, 0xC0)
#define BLOCKCOLOR_DOWNLOADING	RGB(0x80, 0x80, 0xFF)
#define BLOCKCOLOR_INCOMPLETE	RGB(0xFF, 0x80, 0x80)
#define BLOCKCOLOR_MINFAME		RGB(0x40, 0x00, 0x40)
#define BLOCKCOLOR_MAXFAME		RGB(0xFF, 0xC0, 0xFF)
#define BLOCKCOLOR_MINCOMPLETE	RGB(0x00, 0x40, 0x00)
#define BLOCKCOLOR_MAXCOMPLETE	RGB(0xC0, 0xFF, 0xC0)

#define TMODE_FILE				0
#define TMODE_DIRECTORY			1

class Client;

class Torrent : public CWnd
{
	DECLARE_DYNAMIC(Torrent)

	struct BLOCK
	{
		BLOCK(void)
		{
			m_State = PIECESTATE_UNKNOWN;
			m_Pieces = NULL;
			m_Fame = 0;
			m_Priority = 0;
		}

		BYTE m_State;
		BYTE * m_Pieces;
		DWORD m_Fame;
		DWORD m_Priority;
	};

	struct FILEINFO
	{
		CString m_FileName;
		DWORD m_FileSize;
		QWORD m_Offset;
		DWORD m_FirstBlock;
		DWORD m_NumBlocks;
		HANDLE m_hFile;
	};

public:
	Torrent();
	virtual ~Torrent();

	BOOL Create(CWnd * Parent, CListCtrl * TorrentList);
	bool LoadTorrent(const char * FileName, bool IgnoreSettings, bool Silent);

	bool IsDownloading(void);
	const char * GetInfoHash(void);
	const char * GetPeerId(void);
	DWORD GetNumBlocks(void);
	DWORD GetNumPieces(DWORD Block);
	DWORD GetBlocksLeft(void);
	DWORD GetBlockState(DWORD Block);
	DWORD GetPieceState(DWORD Block, DWORD Piece);
	DWORD GetPieceSize(DWORD Block, DWORD Piece);
	QWORD GetFileSize(void);
	DWORD GetBlockSize(DWORD Block);
	void SetPieceState(DWORD Block, DWORD Piece, BYTE State);
	void UpdateProgressPosition(void);
	void AddBlockFame(DWORD Block);
	void RemoveBlockFame(DWORD Block);
	void WritePiece(BYTE * Data, DWORD Block, DWORD Offset, DWORD Len);
	bool IsBlockComplete(DWORD Block);
	void SetBlockState(DWORD Block, BYTE State);
	void UpdateProgress(void);
	void Update(void);
	void KillClient(Client * c);
	void HaveBlock(DWORD Block);
	void ReadData(char * Data, DWORD Block, DWORD Offset, DWORD Len);
	DWORD GetAvgBpsIn(void);
	DWORD GetAvgBpsOut(void);
	DWORD GetBpsIn(void);
	DWORD GetBpsOut(void);
	void AddToQueue(Client * c);
	void RemoveFromQueue(Client * c);
	void UpdateRarity(void);
	DWORD GetRareBlock(DWORD Block);
	DWORD GetRandomBlock(DWORD Block);
	DWORD GetSequentialBlock(DWORD Block);
	void SetMaxUploadRate(DWORD Bps);
	void NumUploadsInc(void);
	void NumUploadsDec(void);
	void AttachClient(DataSocket * s);
	DWORD GetIncompleteBlocks(void);
	void HavePiece(DWORD Block, DWORD Piece);
	DWORD GetNumClients(void);
	DWORD GetNumCachedBlocks(void);
	DWORD GetCachedBlock(DWORD Block);
	void CacheBlock(DWORD Block);
	void UnCacheBlock(DWORD Block);
	void KeepAlive(void);
	void UpdateTrayProgress(void);
	void AddBytesUploaded(DWORD Bytes);
	void AddBytesDownloaded(DWORD Bytes);
	void Restore(void);
	DWORD GetBytesLeft(void);
	void ReOpen(void);
	DWORD GetMaxUploadRate(void);
	DWORD GetMode(void);
	int GetTorrentListIndex(void);
	void OnFinish(void);
	QWORD GetBytesSent(void);
	QWORD GetBytesRecv(void);
	void Pause(void);
	void Resume(void);
	bool IsPaused(void);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnViewTorrentinfo();
	afx_msg void OnViewBlockmap();
	afx_msg void OnViewClientlist();
	afx_msg void OnClose();
	afx_msg void OnTimer(UINT nIDEvent);

protected:
	DECLARE_MESSAGE_MAP()

	static DWORD WINAPI _CheckFile(LPVOID Parameter);
	static DWORD WINAPI _GetTrackerInfo(LPVOID Parameter);
	void PreCheckFile(void);
	void CheckFile(void);
	bool PreAllocate(const char * FileName, QWORD FileSize, QWORD Offset);
	void SetStatus(const char * Format, ...);
	void GetTrackerInfo(void);
	void ConnectToClients(void);
	void UpdateTorrentSends(void);
	static int __cdecl CmpRarity(const void * a, const void * b);
	static int __cdecl CmpRandom(const void * a, const void * b);
	static int __cdecl CmpSequential(const void * a, const void * b);
	HICON MakePieIcon(float Percent);
	void SetIconPixel(int x, int y, BYTE r, BYTE g, BYTE b, bool Solid, BYTE * And, BYTE * Xor);
	void UpdatePriority(void);
	bool ResizeFile(const char * FileName, QWORD FileSize);
	void SetImage(DWORD Img);
	static int CALLBACK SortClientItems(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
	void UpdateBlockMap(void);
	void UpdateIcon(void);

	BlockMap m_BlockMap;
	CListCtrl m_TorrentInfo;
	ProgressBar m_ParentProgress;
	CListCtrl m_ClientList;

	static CSemaphore m_MaxDownloads;
	CMenu m_MainMenu;
	CListCtrl * m_cTorrentList;
	DWORD m_State;
	Lexicon m_Info;
	CString m_TorrentPath;
	CString m_TorrentName;
	char * m_BlockHash;
	BLOCK * m_Blocks;
	static BLOCK * m_sBlocks;
	static bool m_sPrioritize;
	DWORD * m_Rarity;
	DWORD * m_Random;
	DWORD * m_Sequential;
	DWORD * m_Priority;
	char m_PeerId[20];
	char m_InfoHash[20];
	HANDLE m_hFile;
	DWORD m_MaxFame;
	DWORD m_BpsIn;
	DWORD m_BpsOut;
	DWORD m_AvgBpsOut;
	DWORD m_AvgBpsIn;
	DWORD m_NumUploads;
	DWORD m_Mode;
	NOTIFYICONDATA m_NotifyData;
	bool m_IsInTray;
	DWORD m_LastPiePercent;
	QWORD m_BytesUploaded;
	QWORD m_BytesDownloaded;
	DWORD m_TorrentStartTime;
	DWORD m_TorrentLastTime;
	bool m_ThreadBail;
	DWORD m_NumThreads;
	bool m_Prioritize;
	DWORD m_Prioritized;
	CString m_TorrentFile;
	DWORD m_SortInfo[12];
	static DWORD s_SortInfo[12];
	bool m_Paused;

	DWORD m_NumBlocks;
	DWORD m_NumPieces;
	DWORD m_BlockSize;
	QWORD m_FileSize;

	CArray<Client *> m_Clients;
	CArray<HANDLE> m_Threads;
	CArray<Client *> m_SendQueue;
	CArray<FILEINFO> m_Files;
	CArray<DWORD> m_BlockCache;

	virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);
public:
	afx_msg void OnFileHidewindow();
	afx_msg void OnFileCanceldownload();
	afx_msg void OnFileGetnewclientlist();
	afx_msg void OnFileSetfilepriorities();
	afx_msg void OnClientDisconnect();
	afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
	afx_msg void OnHdnClientListItemclick(NMHDR *pNMHDR, LRESULT *pResult);

};
