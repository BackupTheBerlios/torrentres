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
// $Id: DataSocket.cpp,v 1.1 2004/03/31 20:39:09 deepb Exp $
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
// DataSocket.cpp
//
// Custom Socket class
//
// Current issues: Makes alot of listening sockets
// when connecting to peers for some odd reason
//

#include "stdafx.h"
#include "DataSocket.h"

HWND DataSocket::m_hWnd = NULL;
DWORD DataSocket::m_RefCount = 0;
CArray<DataSocket *> DataSocket::m_Sockets;

DataSocket::DataSocket()
{
	m_Socket = INVALID_SOCKET;

	m_OnAccept = NULL;
	m_OnClose = NULL;
	m_OnConnect = NULL;
	m_OnReceive = NULL;
	m_OnSend = NULL;
	m_Custom = 0;

//	m_RecvBuf = new char [0];
	m_RecvBuf = MAlloc(char, 0);
	m_RecvLen = 0;
	m_RecvMaxLen = 0;
//	m_SendBuf = new char [0];
	m_SendBuf = MAlloc(char, 0);
	m_SendLen = 0;
	m_SendMaxLen = 0;

	m_Listening = false;
	m_Connecting = false;
	m_Connected = false;

	m_RecvBps = 0;
	m_SendBps = 0;

	m_RecvBytes = 0;
	m_SendBytes = 0;

	m_MaxSendBps = 0;

	m_Sockets.Add(this);
}

DataSocket::~DataSocket()
{
	DWORD i;

	Close();

	for(i = 0; i < (DWORD)m_Sockets.GetSize(); i++)
	{
		if(m_Sockets[i] == this)
		{
			m_Sockets.RemoveAt(i);
			break;
		}
	}

	MFree(m_RecvBuf);
	MFree(m_SendBuf);
}

bool DataSocket::Create(WORD Port)
{
	sockaddr_in addr;

	Close();

	m_Socket = socket(AF_INET, SOCK_STREAM, 0);

	if(m_Socket == INVALID_SOCKET) return false;

	ZeroMemory(&addr, sizeof(addr));

	addr.sin_family = AF_INET;
	addr.sin_port = htons(Port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if(bind(m_Socket, (sockaddr *)&addr, sizeof(addr)) != 0) return false;

	InitWindow();

	WSAAsyncSelect(m_Socket, m_hWnd, WM_USER + 1, FD_READ | FD_ACCEPT | FD_CONNECT | FD_CLOSE);

	return true;
}

void DataSocket::Close(void)
{
	DWORD i;

	if(m_Socket == INVALID_SOCKET) return;

	shutdown(m_Socket, SD_BOTH);
	closesocket(m_Socket);
	m_Socket = INVALID_SOCKET;
	m_Listening = false;
	m_Connecting = false;
	m_Connected = false;

	for(i = 0; i < (DWORD)m_Sockets.GetSize(); i++)
	{
		if(m_Sockets[i] == this)
		{
			m_Sockets.RemoveAt(i);
			break;
		}
	}

	FreeWindow();
}

void DataSocket::SetCallbacks(SOCKET_CALLBACK OnAccept, SOCKET_CALLBACK OnClose, SOCKET_CALLBACK OnConnect, SOCKET_CALLBACK OnReceive, SOCKET_CALLBACK OnSend, void * Custom)
{
	m_OnAccept = OnAccept;
	m_OnClose = OnClose;
	m_OnConnect = OnConnect;
	m_OnReceive = OnReceive;
	m_OnSend = OnSend;

	m_Custom = Custom;
}

bool DataSocket::Listen(void)
{
	if(m_Socket == INVALID_SOCKET) return false;
	
	if(listen(m_Socket, SOMAXCONN) != 0) return false;

	m_Listening = true;

	return true;
}

bool DataSocket::Connect(const char * Address, WORD Port)
{
	sockaddr_in addr;
	hostent * host;

	if(m_Socket == INVALID_SOCKET) return false;

	host = gethostbyname(Address);
	if(host == NULL) return false;

	ZeroMemory(&addr, sizeof(addr));

	addr.sin_family = AF_INET;
	addr.sin_port = htons(Port);
	addr.sin_addr.s_addr = *(u_long *)host->h_addr;

	m_PeerIp = Address;
	m_PeerPort = Port;

	m_Connecting = true;
	connect(m_Socket, (sockaddr *)&addr, sizeof(addr));

	return false;
}

void DataSocket::Accept(DataSocket & s)
{
	sockaddr_in addr;
	int addrsize;

    s.Attach(accept(m_Socket, NULL, NULL));

	addrsize = sizeof(addr);
	getpeername(s.m_Socket, (sockaddr *)&addr, &addrsize);

	s.m_PeerIp = inet_ntoa(addr.sin_addr);
	s.m_PeerPort = ntohs(addr.sin_port);

	InitWindow();

	WSAAsyncSelect(m_Socket, m_hWnd, WM_USER + 1, FD_READ | FD_ACCEPT | FD_CONNECT | FD_CLOSE);
}

void DataSocket::Attach(SOCKET s)
{
	Close();
	m_Socket = s;
	m_Connected = true;
}

char * DataSocket::GetData(void)
{
	return m_RecvBuf;
}

DWORD DataSocket::GetDataLength(void)
{
	return m_RecvLen;
}

void DataSocket::Discard(DWORD Bytes)
{
//	char * NewBuf;

	if(m_RecvLen - Bytes <= 0)
	{
		m_RecvLen = 0;
		return;
	}

	memmove(m_RecvBuf, &m_RecvBuf[Bytes], m_RecvLen - Bytes);

//	NewBuf = new char [m_RecvLen - Bytes];
//	memcpy(NewBuf, &m_RecvBuf[Bytes], m_RecvLen - Bytes);
//	delete [] m_RecvBuf;
//	m_RecvBuf = NewBuf;

	m_RecvLen -= Bytes;
}

void DataSocket::Send(const void * Buf, DWORD Len)
{
	char * NewBuf;

	if(!m_Connected) return;

	if((long)m_SendLen < 0) __asm int 3;

	if(m_SendLen + Len < m_SendMaxLen)
	{
		memcpy(&m_SendBuf[m_SendLen], Buf, Len);
		m_SendLen += Len;
	}else{
//		NewBuf = new char [m_SendLen + Len];
		NewBuf = MAlloc(char, m_SendLen + Len);

		memcpy(NewBuf, m_SendBuf, m_SendLen);
		memcpy(&NewBuf[m_SendLen], Buf, Len);

		MFree(m_SendBuf);
		m_SendBuf = NewBuf;

		m_SendLen += Len;
		m_SendMaxLen = m_SendLen;
	}
}

void DataSocket::SetMaxUploadRate(DWORD Bps)
{
	m_MaxSendBps = Bps;
}

DWORD DataSocket::GetMaxUploadRate(void)
{
	return m_MaxSendBps;
}

//DWORD WINAPI DataSocket::_SocketThread(LPVOID Parameter)
//{
//	TIMEVAL tv;
//	fd_set fdread;
//	fd_set fdwrite;
//	fd_set fdexcept;
//	DWORD i;
//	int Result;
//	DataSocket * s;
//
//	while(1)
//	{
//		Sleep(100);
//
//		//m_CriticalSection.Lock();
//		for(i = 0; i < (DWORD)m_Sockets.GetSize(); i++)
//		{
//			m_Sockets[i]->SendData();
//		}
//
//		tv.tv_sec = 0;
//		tv.tv_usec = 0;
//
//		FD_ZERO(&fdread);
//		FD_ZERO(&fdwrite);
//		FD_ZERO(&fdexcept);
//
//		for(i = 0; i < (DWORD)m_Sockets.GetSize(); i++)
//		{
//			if(m_Sockets[i]->m_Socket != INVALID_SOCKET)
//			{
//				FD_SET(m_Sockets[i]->m_Socket, &fdread);
//				if(m_Sockets[i]->m_Connecting)
//				{
//					FD_SET(m_Sockets[i]->m_Socket, &fdwrite);
//				}
//				FD_SET(m_Sockets[i]->m_Socket, &fdexcept);
//			}
//		}
//
//		Result = select(0, &fdread, &fdwrite, &fdexcept, &tv);
//
//		//m_CriticalSection.Unlock();
//
//		if(Result == 0)
//		{
//			continue;
//		}
//		if(Result == SOCKET_ERROR)
//		{
//			continue;
//		}
//
//		for(i = 0; i < (DWORD)m_Sockets.GetSize(); i++)
//		{
//			s = m_Sockets[i];
//
//			// OnReceive/OnAccept
//			if(FD_ISSET(s->m_Socket, &fdread))
//			{
//				if(s->m_Listening)
//				{
//					if(s->m_OnAccept) s->m_OnAccept(s, s->m_Custom, 0);
//				}else{
//					s->ReadData();
//				}
//			}
//
//			// OnConnect
//			if(FD_ISSET(s->m_Socket, &fdwrite))
//			{
//				if(s->m_Connecting)
//				{
//					s->m_Connecting = false;
//					s->m_Connected = true;
//
//					if(s->m_OnConnect) s->m_OnConnect(s, s->m_Custom, 0);
//				}else{
//				}
//			}
//
//			// OnConnect (Error)
//			if(FD_ISSET(s->m_Socket, &fdexcept))
//			{
//				if(s->m_Connecting)
//				{
//					s->m_Connecting = false;
//					s->m_Connected = false;
//
//					if(s->m_OnConnect) s->m_OnConnect(s, s->m_Custom, -1);
//				}
//			}
//		}
//	}
//
//	return 0;
//}

void DataSocket::UpdateBps(void)
{
	m_RecvBps = m_RecvBytes;
	m_SendBps = m_SendBytes;

	m_RecvBytes = 0;
	m_SendBytes = 0;
}

DWORD DataSocket::GetRecvBps(void)
{
	return m_RecvBps;
}

DWORD DataSocket::GetSendBps(void)
{
	return m_SendBps;
}

void DataSocket::ReadData(void)
{
	int Result;
	char Buf[1024];
	char * NewBuf;

	for(;;)
	{
		Result = recv(m_Socket, Buf, sizeof(Buf), 0);

		if(Result == 0)
		{
			m_Connected = false;
			if(m_OnClose) m_OnClose(this, m_Custom, 0);
			return;
		}
		if(Result == SOCKET_ERROR)
		{
			switch(GetLastError())
			{
			case WSAEMSGSIZE:
				MessageBox(NULL, "The message was too large to fit into the specified buffer and was truncated.", "", MB_OK);
				break;

			case WSAENETRESET:
			case WSAECONNABORTED:
			case WSAETIMEDOUT:
			case WSAECONNRESET:
				m_Connected = false;
				if(m_OnClose) m_OnClose(this, m_Custom, 0);
				return;
			}
			break;
		}

		if(m_RecvLen + Result < m_RecvMaxLen)
		{
			memcpy(&m_RecvBuf[m_RecvLen], Buf, Result);
			m_RecvLen += Result;
		}else{
//			NewBuf = new char [m_RecvLen + Result];
			NewBuf = MAlloc(char, m_RecvLen + Result);

			memcpy(NewBuf, m_RecvBuf, m_RecvLen);
			memcpy(&NewBuf[m_RecvLen], Buf, Result);

			MFree(m_RecvBuf);
			m_RecvBuf = NewBuf;

			m_RecvLen += Result;
			m_RecvMaxLen = m_RecvLen;
		}
		m_RecvBytes += Result;
	}

	if(m_OnReceive) m_OnReceive(this, m_Custom, 0);
}

void DataSocket::SendData(void)
{
	int Result;
//	char * NewBuf;
	DWORD Len;

	if(m_SendLen == 0)
	{
		return;
	}

	if(m_MaxSendBps == 0)
	{
		Result = send(m_Socket, m_SendBuf, m_SendLen, 0);
	}else{
		Len = m_MaxSendBps / 2;
		if(Len < 1024 / 2) Len = 1024 / 2;
		if(Len > m_SendLen) Len = m_SendLen;
		Result = send(m_Socket, m_SendBuf, Len, 0);
	}

	if(Result == 0)
	{
		return;
	}
	if(Result == SOCKET_ERROR)
	{
		return;
	}

	memmove(m_SendBuf, &m_SendBuf[Result], m_SendLen - Result);
	m_SendLen -= Result;

//	NewBuf = new char [m_SendLen - Result];
//	memcpy(NewBuf, &m_SendBuf[Result], m_SendLen - Result);
//	delete [] m_SendBuf;
//	m_SendBuf = NewBuf;
//	m_SendLen -= Result;

	m_SendBytes += Result;

	if(m_SendLen == 0)
	{
		if(m_OnSend) m_OnSend(this, m_Custom, 0);
	}	
}

const char * DataSocket::GetIp(void)
{
	return m_PeerIp;
}

WORD DataSocket::GetPort()
{
	return m_PeerPort;
}

void DataSocket::InitWindow(void)
{
	WNDCLASS wc;

	if(m_RefCount != 0)
	{
		m_RefCount++;
		return;
	}
	
	ZeroMemory(&wc, sizeof(wc));

	wc.hInstance = AfxGetInstanceHandle();
	wc.lpfnWndProc = WndProc;
	wc.lpszClassName = "XSocket";

	RegisterClass(&wc);

	m_hWnd = CreateWindowEx(0, "XSocket", "XSocket", WS_OVERLAPPEDWINDOW, 0, 0, 0, 0, NULL, NULL, AfxGetInstanceHandle(), NULL);

	SetTimer(m_hWnd, 0, 500, NULL);

	m_RefCount++;
}

void DataSocket::FreeWindow(void)
{
	m_RefCount--;

	if(m_RefCount == 0)
	{
		DestroyWindow(m_hWnd);
		m_hWnd = NULL;
	}
}

LRESULT WINAPI DataSocket::WndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	switch(Msg)
	{
	case WM_USER + 1:
		{
			SOCKET s = wParam;
			WORD Event = LOWORD(lParam);
			WORD Error = HIWORD(lParam);
			DWORD i;

			for(i = 0; i < (DWORD)m_Sockets.GetSize(); i++)
			{
				if(m_Sockets[i]->m_Socket == s)
				{
					switch(Event)
					{
					case FD_READ:
						m_Sockets[i]->ReadData();
						break;

					case FD_ACCEPT:
						if(m_Sockets[i]->m_OnAccept) m_Sockets[i]->m_OnAccept(m_Sockets[i], m_Sockets[i]->m_Custom, Error);
						break;

					case FD_CONNECT:
						m_Sockets[i]->m_Connecting = false;
						if(Error == 0) m_Sockets[i]->m_Connected = true;
						if(m_Sockets[i]->m_OnConnect) m_Sockets[i]->m_OnConnect(m_Sockets[i], m_Sockets[i]->m_Custom, Error);
						break;

					case FD_CLOSE:
						m_Sockets[i]->m_Connected = false;
						if(m_Sockets[i]->m_OnClose) m_Sockets[i]->m_OnClose(m_Sockets[i], m_Sockets[i]->m_Custom, Error);
						break;
					}
				}
			}
		}
		break;

	case WM_TIMER:
		{
			DWORD i;

			for(i = 0; i < (DWORD)m_Sockets.GetSize(); i++)
			{
				m_Sockets[i]->SendData();
			}
		}
		break;

	default:
		return DefWindowProc(hWnd, Msg, wParam, lParam);
	}

	return 0;
}