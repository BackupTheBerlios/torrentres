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
// $Id: DataSocket.h,v 1.1 2004/03/31 20:39:09 deepb Exp $
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
// DataSocket.h
//
// Custom Socket class
//
// Current issues: Makes alot of listening sockets
// when connecting to peers for some odd reason
//

#pragma once

#define SDW(d)	(((d & 0xFF000000) >> 24) | ((d & 0x00FF0000) >> 8) | ((d & 0x000000FF) << 24) | ((d & 0x0000FF00) << 8))
#define CDW(d)	(*(DWORD *)&(d))

class DataSocket
{
	typedef void (*SOCKET_CALLBACK)(DataSocket * s, void * Custom, int ErrorCode);
public:
	DataSocket();
	virtual ~DataSocket();

	bool Create(WORD Port = 0);
	void Close(void);

	void SetCallbacks(SOCKET_CALLBACK OnAccept, SOCKET_CALLBACK OnClose, SOCKET_CALLBACK OnConnect, SOCKET_CALLBACK OnReceive, SOCKET_CALLBACK OnSend, void * Custom);

	bool Listen(void);
	bool Connect(const char * Address, WORD Port);
	void Accept(DataSocket & s);
	void Attach(SOCKET s);

	DWORD GetDataLength(void);
	char * GetData(void);
	void Discard(DWORD Bytes);

	void Send(const void * Buf, DWORD Len);
	void SetMaxUploadRate(DWORD Bps);
	DWORD GetMaxUploadRate(void);

	void UpdateBps(void);
	DWORD GetRecvBps(void);
	DWORD GetSendBps(void);

	const char * GetIp(void);
	WORD GetPort(void);

protected:

//	static DWORD WINAPI _SocketThread(LPVOID Parameter);
	void ReadData(void);
	void SendData(void);

	void InitWindow(void);
	void FreeWindow(void);

	static LRESULT WINAPI WndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

	SOCKET m_Socket;

	SOCKET_CALLBACK m_OnAccept;
	SOCKET_CALLBACK m_OnClose;
	SOCKET_CALLBACK m_OnConnect;
	SOCKET_CALLBACK m_OnReceive;
	SOCKET_CALLBACK m_OnSend;
	void * m_Custom;

	// Data Stuff
	char * m_RecvBuf;
	DWORD m_RecvLen;
	DWORD m_RecvBps;
	DWORD m_RecvBytes;
	DWORD m_RecvMaxLen;

	char * m_SendBuf;
	DWORD m_SendLen;
	DWORD m_SendBps;
	DWORD m_SendBytes;
	DWORD m_SendMaxLen;

	DWORD m_MaxSendBps;

	// Connection State
	bool m_Listening;
	bool m_Connecting;
	bool m_Connected;

	// Peer Info
	CString m_PeerIp;
	WORD m_PeerPort;

	static HWND m_hWnd;
	static DWORD m_RefCount;
	static CArray<DataSocket *> m_Sockets;
};