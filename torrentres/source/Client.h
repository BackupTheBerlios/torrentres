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
// $Id: Client.h,v 1.1 2004/03/31 20:39:09 deepb Exp $
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
// Client.h
//
// Stores State information for Clients
//

#pragma once

#include "DataSocket.h"

#define CSTATE_NONE			0
#define CSTATE_CONNECTING	1
#define CSTATE_ACTIVE		2

#define NUMDATASAMPLES		30

class Torrent;

class Client
{
	struct CACHE
	{
		DWORD Block;
		DWORD Piece;

		DWORD Offset;
		DWORD Length;
	};
public:
	Client(void);
	~Client(void);

	void SetIp(const char * Ip);
	void SetPort(WORD Port);
	void SetPeerId(char * PeerId);
	void SetParent(Torrent * Parent);
	void SetList(CListCtrl * List);

	void Attach(DataSocket * s);

	DWORD GetTimeUnchoked(void);

	void Connect(void);

	void Update(int Index);

	DWORD GetBpsOut(void);
	DWORD GetBpsIn(void);
	DWORD GetAvgBpsOut(void);
	DWORD GetAvgBpsIn(void);
	DWORD GetBytesDownloaded(void);
	DWORD GetBytesUploaded(void);

	void SetMaxUploadRate(DWORD Bps);
	DWORD GetMaxUploadRate(void);

	void Send_KeepAlive(void);
	void Send_Choke(void);
	void Send_UnChoke(void);
	void Send_Have(DWORD Block);
	void Send_Cancel(DWORD Block, DWORD Piece, bool Reacquire);	

	bool IsSeed(void);
	bool IsConnected(void);
	const char * GetIp(void);
	bool IsChoked(void);
	bool IsInterested(void);
	bool IsAttached(void);

	void HavePiece(DWORD Block, DWORD Piece);

	float GetPercentage(void);

	DWORD GetState(void);

	void Pause(void);
	void Resume(void);

protected:

	void CheckInterest(void);
	void RequestPiece(void);
	void CheckPeerId(void);

	static void _OnClose(DataSocket * s, void * Custom, int ErrorCode);
	static void _OnConnect(DataSocket * s, void * Custom, int ErrorCode);
	static void _OnReceive(DataSocket * s, void * Custom, int ErrorCode);
	static void _OnSend(DataSocket * s, void * Custom, int ErrorCode);

	void OnClose(int ErrorCode);
	void OnConnect(int ErrorCode);
	void OnReceive(int ErrorCode);
	void OnSend(int ErrorCode);

	void Send_Handshake(void);
	void Send_Interested(void);
	void Send_NotInterested(void);
	void Send_BitField(void);
	void Send_Request(DWORD Block, DWORD Piece);
	void Send_Piece(void);

	void Read_Choke(void);
	void Read_UnChoke(void);
	void Read_Interested(void);
	void Read_NotInterested(void);
	void Read_Have(void);
	void Read_BitField(void);
	void Read_Request(void);
	void Read_Piece(void);
	void Read_Cancel(void);	

	CListCtrl * m_List;

	bool m_Interested;		// My Interest in this Peer
	bool m_Choke;			// Am I choking Peer
	bool m_PeerInterested;	// Peers interest in my file
	bool m_PeerChoke;		// Is Peer Choking Me

	bool m_IsSeed;

	DWORD m_PieceTimeout;
	DWORD m_State;
	BYTE * m_Blocks;
	CString m_Ip;
	WORD m_Port;
	char m_PeerId[20];
	Torrent * m_Parent;
	bool m_IsAuthenticated;
	DWORD m_BpsIn;
	DWORD m_BpsOut;
	DWORD m_AvgBpsOut[NUMDATASAMPLES];
	DWORD m_AvgBpsIn[NUMDATASAMPLES];
	DWORD m_CurSample;
	DWORD m_UnChokeTime;
	bool m_Attached;
	bool m_TimedOut;
	DWORD m_BytesUploaded;
	DWORD m_BytesDownloaded;
	DWORD m_PeerSize;
	CString m_ClientType;

	CArray<CACHE> m_RequestCache;
	CArray<CACHE> m_SendCache;

	DataSocket * m_Socket;
};
