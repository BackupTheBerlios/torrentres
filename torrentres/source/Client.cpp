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
// $Id: Client.cpp,v 1.1 2004/03/31 20:39:09 deepb Exp $
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
// Client.cpp
//
// Stores State information for Clients
//

#include "StdAfx.h"
#include "client.h"

#include "torrent.h"
#include "settings.h"
#include "logwnd.h"
#include "tormentwnd.h"

Client::Client(void)
{
	m_State = CSTATE_NONE;
	m_IsAuthenticated = false;
	m_Interested = false;
	m_Choke = true;
	m_PeerInterested = false;
	m_PeerChoke = true;
	m_Blocks = NULL;
	m_PieceTimeout = 0;
	m_BpsOut = 0;
	m_BpsIn = 0;
	ZeroMemory(m_AvgBpsOut, sizeof(m_AvgBpsOut));
	ZeroMemory(m_AvgBpsIn, sizeof(m_AvgBpsIn));
	m_CurSample = 0;
	m_IsSeed = false;
//	m_Socket = new DataSocket;
	m_Socket = MAlloc(DataSocket, 1);
	m_Attached = false;
	m_TimedOut = false;
	m_BytesUploaded = 0;
	m_BytesDownloaded = 0;
	m_PeerSize = 0;
	m_ClientType = "Desconocido";
}

Client::~Client(void)
{
	DWORD i;

	Send_Choke();

	m_Socket->Close();
	MFree(m_Socket);

	if(m_Blocks)
	{
		for(i = 0; i < m_Parent->GetNumBlocks(); i++)
		{
			if(m_Blocks[i] == PIECESTATE_COMPLETE)
			{
				m_Parent->RemoveBlockFame(i);
			}
		}

		MFree(m_Blocks);
	}

	m_Parent->RemoveFromQueue(this);
}

void Client::SetIp(const char * Ip)
{
	m_Ip = Ip;
}

const char * Client::GetIp(void)
{
	return m_Ip;
}

void Client::SetPort(WORD Port)
{
	m_Port = Port;
}

void Client::SetPeerId(char * PeerId)
{
	memcpy(m_PeerId, PeerId, 20);
}

void Client::SetParent(Torrent * Parent)
{
	m_Parent = Parent;
}

void Client::Connect(void)
{
	int Index;

	if(m_State != CSTATE_NONE) return;

	m_State = CSTATE_CONNECTING;

	Index = m_List->InsertItem(m_List->GetItemCount(), m_Ip);
	m_List->SetItemData(Index, (DWORD_PTR)this);

//	m_Blocks = new BYTE [m_Parent->GetNumBlocks()];
	m_Blocks = MAlloc(BYTE, m_Parent->GetNumBlocks());
	ZeroMemory(m_Blocks, m_Parent->GetNumBlocks());

	m_Socket->Create();
	m_Socket->SetCallbacks(NULL, _OnClose, _OnConnect, _OnReceive, _OnSend, this);
	m_Socket->Connect(m_Ip, m_Port);

	TRACE("Connecing to %s:%d\n", m_Ip, m_Port);
}

void Client::_OnClose(DataSocket * s, void * Custom, int ErrorCode)
{
	((Client *)Custom)->OnClose(ErrorCode);
}

void Client::_OnConnect(DataSocket * s, void * Custom, int ErrorCode)
{
	((Client *)Custom)->OnConnect(ErrorCode);
}

void Client::_OnReceive(DataSocket * s, void * Custom, int ErrorCode)
{
	((Client *)Custom)->OnReceive(ErrorCode);
}

void Client::_OnSend(DataSocket * s, void * Custom, int ErrorCode)
{
	((Client *)Custom)->OnSend(ErrorCode);
}

void Client::OnClose(int ErrorCode)
{
	TRACE("%s: OnClose (%d)\n", m_Ip, ErrorCode);

	while(m_RequestCache.GetSize())
	{
		TRACE("Closed (%d, %d)\n", m_RequestCache[0].Block, m_RequestCache[0].Piece);
		if(m_Parent->GetPieceState(m_RequestCache[0].Block, m_RequestCache[0].Piece) != PIECESTATE_COMPLETE)
		{
			m_Parent->SetPieceState(m_RequestCache[0].Block, m_RequestCache[0].Piece, PIECESTATE_INCOMPLETE);
		}
		m_RequestCache.RemoveAt(0);
	}

	ZeroMemory(m_Blocks, m_Parent->GetNumBlocks());

	m_Parent->KillClient(this);
}

void Client::OnConnect(int ErrorCode)
{
	if(ErrorCode == 0)
	{
		TRACE("%s: Connected\n", m_Ip);
		m_State = CSTATE_ACTIVE;

		Send_Handshake();
	}else{
		TRACE("%s: Connection Failed\n", m_Ip);
		m_Parent->KillClient(this);
	}
}

void Client::OnReceive(int ErrorCode)
{
	DWORD Len;
	char * Data;

	if(ErrorCode)
	{
		TRACE("%s: OnReceive Error (%d)\n", m_Ip, ErrorCode);
		return;
	}

	Len = m_Socket->GetDataLength();
	Data = m_Socket->GetData();

	if(!m_IsAuthenticated)
	{
		if(m_Parent->GetNumClients() >= Settings::GetDWordValue("MaxConnections"))
		{
			m_Parent->KillClient(this);
			return;
		}

		if(Len >= 68)
		{
			if(
				Data[0] == 19 &&
				memcmp(&Data[1], "BitTorrent protocol", 19) == 0 &&
				memcmp(&Data[28], m_Parent->GetInfoHash(), 20) == 0 &&
				memcmp(&Data[48], m_PeerId, 20) == 0 &&
				memcmp(&Data[48], m_Parent->GetPeerId(), 20) != 0
				)
			{
				TRACE("%s: Authenticated\n", m_Ip);
				m_IsAuthenticated = true;
				m_Socket->Discard(68);
				
				Send_BitField();

				OnReceive(ErrorCode);

				CheckPeerId();

				TRACE("%s: Authenticated\n", m_Ip);
			}else{
				TRACE("%s: Bad Auth/Loopback\n", m_Ip);

				m_Parent->KillClient(this);
				return;
			}
		}
	}else{
		if(Len < 4) return;
		if(Len < SDW(CDW(Data[0])) + 4) return;

		if(SDW(CDW(Data[0])) != 0)
		{
			if(SDW(CDW(Data[0])) != 0)
			{
				switch(Data[4])
				{
				case 0x00:	Read_Choke();			break;
				case 0x01:	Read_UnChoke();			break;
				case 0x02:	Read_Interested();		break;
				case 0x03:	Read_NotInterested();	break;
				case 0x04:	Read_Have();			break;
				case 0x05:	Read_BitField();		break;
				case 0x06:	Read_Request();			break;
				case 0x07:	Read_Piece();			break;
				case 0x08:	Read_Cancel();			break;
				}
			}
		}

		m_Socket->Discard(SDW(CDW(Data[0])) + 4);

		OnReceive(ErrorCode);
	}
}

void Client::OnSend(int ErrorCode)
{
	if(m_SendCache.GetSize() && !m_Choke)
	{
		m_SendCache.RemoveAt(0);
		if(m_SendCache.GetSize()) Send_Piece();
	}
}

void Client::CheckInterest(void)
{
	DWORD i;

	if(!m_Blocks) return;
	if(m_Parent->IsPaused()) return;

	for(i = 0; i < m_Parent->GetNumBlocks(); i++)
	{
		if(m_Parent->GetBlockState(i) != PIECESTATE_COMPLETE && m_Blocks[i] == PIECESTATE_COMPLETE)
		{
			Send_Interested();
			return;
		}
	}

	Send_NotInterested();
}

void Client::RequestPiece(void)
{
	DWORD i, j, k, l, m;
	DWORD Block;
	bool b;
	DWORD NumPieces;

	if(m_PeerChoke) return;

	if(m_Parent->GetIncompleteBlocks() < 10)
	{
		// Attempt to download anything i can get my hands on, randomly selecting pieces to prevent clogging
		for(i = 0; i < m_Parent->GetNumBlocks(); i++)
		{
			NumPieces = m_Parent->GetNumPieces(i);

			if(m_Parent->GetBlockState(i) != PIECESTATE_COMPLETE && m_Blocks[i] == PIECESTATE_COMPLETE)
			{
				m = rand() % NumPieces;
				for(j = 0; j < NumPieces; j++)
				{
					// Random Selection
					if(NumPieces != 0)
					{
						l = (m + j) % NumPieces;
					}else{
						l = j;
					}
					if(m_Parent->GetPieceState(i, l) != PIECESTATE_COMPLETE)
					{
						b = false;
						for(k = 0; k < (DWORD)m_RequestCache.GetSize(); k++)
						{
							if(m_RequestCache[k].Block == i && m_RequestCache[k].Piece == l) b = true;
						}

						if(b == false)
						{
							Send_Request(i, l);
							return;
						}
					}
				}
			}
		}
	}else{
		// Attempt to queue blocks that have been started first
		for(i = 0; i < m_Parent->GetNumCachedBlocks(); i++)
		{
			Block = m_Parent->GetCachedBlock(i);
			if(m_Parent->GetBlockState(Block) != PIECESTATE_COMPLETE && m_Blocks[Block] == PIECESTATE_COMPLETE)
			{
				for(j = 0; j < m_Parent->GetNumPieces(Block); j++)
				{
					if(m_Parent->GetPieceState(Block, j) == PIECESTATE_INCOMPLETE)
					{
						Send_Request(Block, j);
						return;
					}
				}
			}
		}

		// Only download if I haven't cached a request from a peer already
		for(i = 0; i < m_Parent->GetNumBlocks(); i++)
		{
			switch(Settings::GetDWordValue("DownloadAlgorithm"))
			{
			case 0:	// Rarest
				Block = m_Parent->GetRareBlock(i);
				break;

			case 1:	// Random
				Block = m_Parent->GetRandomBlock(i);
				break;

			case 2:	// Sequential
				Block = m_Parent->GetSequentialBlock(i);
				break;
			}

			if(m_Parent->GetBlockState(Block) != PIECESTATE_COMPLETE && m_Blocks[Block] == PIECESTATE_COMPLETE)
			{
				for(j = 0; j < m_Parent->GetNumPieces(Block); j++)
				{
					if(m_Parent->GetPieceState(Block, j) == PIECESTATE_INCOMPLETE)
					{
						m_Parent->CacheBlock(Block);
						Send_Request(Block, j);
						return;
					}
				}
			}
		}
	}
}

//////////////////////////////////////////////////
// Send_*

void Client::Send_Handshake(void)
{
	TRACE("%s: Send_Handshake\n", m_Ip);
	BYTE Out[68];
	BYTE Reserved[8] = {0, 0, 0, 0, 0, 0, 0, 1};

	Out[0] = 19;
	memcpy(&Out[1], "BitTorrent protocol", 19);
	memcpy(&Out[20], Reserved, 8);
	memcpy(&Out[28], m_Parent->GetInfoHash(), 20);
	memcpy(&Out[48], m_Parent->GetPeerId(), 20);
	
	m_Socket->Send(Out, 68);

//	Send_Identifier();
}

void Client::Send_KeepAlive(void)
{
	BYTE Out[4];

	TRACE("%s: Send_KeepAlive\n", m_Ip);

	CDW(Out[0]) = SDW(0x00000000);

	m_Socket->Send(Out, 4);
}

void Client::Send_Choke(void)
{
	BYTE Out[5];

	if(m_Choke == true) return;
	m_Choke = true;

	m_Parent->NumUploadsDec();

	while(m_SendCache.GetSize())
	{
		m_SendCache.RemoveAt(0);
	}

	TRACE("%s: Send_Choke\n", m_Ip);

	CDW(Out[0]) = SDW(0x00000001);
	Out[4] = 0x00;

	m_Socket->Send(Out, 5);

	m_Parent->AddToQueue(this);
}

void Client::Send_UnChoke(void)
{
	BYTE Out[5];

	if(m_Choke == false) return;
	m_Choke = false;

	m_Parent->NumUploadsInc();

	m_UnChokeTime = GetTickCount();

	TRACE("%s: Send_UnChoke\n", m_Ip);

	CDW(Out[0]) = SDW(0x00000001);
	Out[4] = 0x01;

	m_Socket->Send(Out, 5);

	m_Parent->RemoveFromQueue(this);
}

void Client::Send_Interested(void)
{
	BYTE Out[5];

	if(m_Interested == true) return;
	m_Interested = true;

	TRACE("%s: Send_Interested\n", m_Ip);

	CDW(Out[0]) = SDW(0x00000001);
	Out[4] = 0x02;

	m_Socket->Send(Out, 5);
}

void Client::Send_NotInterested(void)
{
	BYTE Out[5];

	if(m_Interested == false) return;
	m_Interested = false;

	TRACE("%s: Send_NotInterested\n", m_Ip);

	CDW(Out[0]) = SDW(0x00000001);
	Out[4] = 0x03;

	m_Socket->Send(Out, 5);
}

void Client::Send_Have(DWORD Block)
{
	BYTE Out[9];

	TRACE("%s: Send_Have\n", m_Ip);

	CDW(Out[0]) = SDW(0x00000005);
	Out[4] = 0x04;
	CDW(Out[5]) = SDW(Block);

	m_Socket->Send(Out, 9);

	CheckInterest();
}

void Client::Send_BitField(void)
{
	TRACE("%s: Send_BitField\n", m_Ip);

	BYTE * Out;
	DWORD i;
	int ByteNum;
	int BitNum;
	DWORD ByteCount;
	DWORD NumBlocks;

	ByteCount = ((m_Parent->GetNumBlocks() - 1) / 8) + 1;
	NumBlocks = m_Parent->GetNumBlocks();

//	Out = new BYTE [5 + ByteCount];
	Out = MAlloc(BYTE, 5 + ByteCount);

	ZeroMemory(Out, 5 + ByteCount);

	for(i = 0; i < NumBlocks; i++)
	{
		ByteNum = i / 8;
		BitNum = 7 - (i % 8);

		if(m_Parent->GetBlockState(i) == PIECESTATE_COMPLETE)
		{
			Out[5 + ByteNum] |= (1 << BitNum);
		}
	}

	CDW(Out[0]) = SDW(ByteCount + 1);
	Out[4] = 0x05;

	m_Socket->Send(Out, 5 + ByteCount);

	MFree(Out);
}

void Client::Send_Request(DWORD Block, DWORD Piece)
{
	BYTE Out[17];
	CACHE c;

	if(Block >= m_Parent->GetNumBlocks()) return;
	if(Piece >= m_Parent->GetNumPieces(Block)) return;
	if(m_Parent->GetPieceState(Block, Piece) == PIECESTATE_COMPLETE) return;

	TRACE("%s: Send_Request (%d, %d, %d)\n", m_Ip, Block, Piece, m_Parent->GetPieceSize(Block, Piece));

	m_Parent->SetPieceState(Block, Piece, PIECESTATE_DOWNLOADING);
	m_Parent->SetBlockState(Block, PIECESTATE_DOWNLOADING);

	if(m_RequestCache.GetSize() == 0)
	{
		m_PieceTimeout = GetTickCount() + PIECETIMEOUT;
	}

	c.Block = Block;
	c.Piece = Piece;
	m_RequestCache.Add(c);

	CDW(Out[0]) = SDW(13);
	Out[4] = 0x06;
	CDW(Out[5]) = SDW(Block);
	CDW(Out[9]) = SDW(Piece * PIECESIZE);
	CDW(Out[13]) = SDW(m_Parent->GetPieceSize(Block, Piece));

	m_Socket->Send(Out, 17);
}

void Client::Send_Piece(void)
{
	char * Out;
	
	if(m_SendCache.GetSize() == 0) return;

	TRACE("%s: Send_Piece (%d, %d, %d)\n", m_Ip, m_SendCache[0].Block, m_SendCache[0].Offset, m_SendCache[0].Length);

//	Out = new char [m_SendCache[0].Length + 13];
	Out = MAlloc(char, m_SendCache[0].Length + 13);

	m_Parent->ReadData(&Out[13], m_SendCache[0].Block, m_SendCache[0].Offset, m_SendCache[0].Length);

	CDW(Out[0]) = SDW(m_SendCache[0].Length + 9);
	Out[4] = 0x07;
	CDW(Out[5]) = SDW(m_SendCache[0].Block);
	CDW(Out[9]) = SDW(m_SendCache[0].Offset);

	m_Socket->Send(Out, m_SendCache[0].Length + 13);

	MFree(Out);

	m_BytesUploaded += m_SendCache[0].Length;
	m_Parent->AddBytesUploaded(m_SendCache[0].Length);
}

void Client::Send_Cancel(DWORD Block, DWORD Piece, bool Reacquire)
{
	BYTE Out[17];
	DWORD i;

	for(i = 0; i < (DWORD)m_RequestCache.GetSize(); i++)
	{
		if(m_RequestCache[i].Block == Block &&
			m_RequestCache[i].Piece == Piece)
		{
			TRACE("%s: Send_Cancel (%d, %d)\n", m_Ip, Block, Piece);

			CDW(Out[0]) = SDW(13);
			Out[4] = 0x08;
			CDW(Out[5]) = SDW(Block);
			CDW(Out[9]) = SDW(Piece * PIECESIZE);
			CDW(Out[13]) = SDW(m_Parent->GetPieceSize(Block, Piece));

			m_Socket->Send(Out, 17);

			m_RequestCache.RemoveAt(i);

			if(m_Parent->GetPieceState(Block, Piece) != PIECESTATE_COMPLETE)
			{
				m_Parent->SetPieceState(Block, Piece, PIECESTATE_INCOMPLETE);
			}

			if(Reacquire) RequestPiece();
			break;
		}
	}
}

//////////////////////////////////////////////////
// Read_*

void Client::Read_Choke(void)
{
	TRACE("%s: Read_Choke\n", m_Ip);
	
	m_PeerChoke = true;

	while(m_RequestCache.GetSize())
	{
		TRACE("Choked (%d, %d)\n", m_RequestCache[0].Block, m_RequestCache[0].Piece);
		if(m_Parent->GetPieceState(m_RequestCache[0].Block, m_RequestCache[0].Piece) != PIECESTATE_COMPLETE)
		{
			m_Parent->SetPieceState(m_RequestCache[0].Block, m_RequestCache[0].Piece, PIECESTATE_INCOMPLETE);
		}

		m_RequestCache.RemoveAt(0);
		//Send_Cancel(m_RequestCache[0].Block, m_RequestCache[0].Piece, false);
	}

	m_PieceTimeout = 0;
}

void Client::Read_UnChoke(void)
{
	TRACE("%s: Read_UnChoke\n", m_Ip);

	m_PeerChoke = false;
	m_TimedOut = false;

	if(m_Parent->IsPaused()) return;

	RequestPiece();
	RequestPiece();
	RequestPiece();
	RequestPiece();
	RequestPiece();
}

void Client::Read_Interested(void)
{
	TRACE("%s: Read_Interested\n", m_Ip);

	m_PeerInterested = true;

	m_Parent->AddToQueue(this);
}

void Client::Read_NotInterested(void)
{
	TRACE("%s: Read_NotInterested\n", m_Ip);

	m_PeerInterested = false;

	m_Parent->RemoveFromQueue(this);

	Send_Choke();
}

void Client::Read_Have(void)
{
	BYTE * Data = (BYTE *)m_Socket->GetData();
	DWORD Block = SDW(CDW(Data[5]));
	DWORD i;

	TRACE("%s: Read_Have (%d)\n", m_Ip, Block);

	if(Block < m_Parent->GetNumBlocks())
	{
		if(m_Blocks[Block] != PIECESTATE_COMPLETE)
		{
			m_Blocks[Block] = PIECESTATE_COMPLETE;
			m_Parent->AddBlockFame(Block);

			CheckInterest();
		}
	}else{
		// Bogus Client
		TRACE("%s: Bogus Data Received. Dumping Client\n", m_Ip);

		m_Socket->Close();
		m_Parent->KillClient(this);
	}

	m_PeerSize = 0;
	for(i = 0; i < m_Parent->GetNumBlocks(); i++)
	{
		if(m_Blocks[i] == PIECESTATE_COMPLETE)
		{
			m_PeerSize += m_Parent->GetBlockSize(i);
		}
	}

	m_IsSeed = true;
	for(i = 0; i < m_Parent->GetNumBlocks(); i++)
	{
		if(m_Blocks[i] != PIECESTATE_COMPLETE)
		{
			m_IsSeed = false;
		}
	}
}

void Client::Read_BitField(void)
{
	TRACE("%s: Read_BitField\n", m_Ip);

	BYTE * Data = (BYTE *)m_Socket->GetData();
	DWORD DataLen = m_Socket->GetDataLength();
	DWORD Len = SDW(CDW(Data[0]));
	DWORD Count;

	DWORD i;

	Count = 0;
	for(i = 0; i < m_Parent->GetNumBlocks(); i++)
	{
		if(Data[5 + (i / 8)] & (1 << (7 - (i % 8))))
		{
			m_Blocks[i] = PIECESTATE_COMPLETE;
			m_Parent->AddBlockFame(i);
			Count++;
		}
	}

	if(Count == m_Parent->GetNumBlocks())
	{
		m_IsSeed = true;
	}

	CheckInterest();

	m_Parent->UpdateRarity();
	m_Parent->UpdateProgress();

	m_PeerSize = 0;
	for(i = 0; i < m_Parent->GetNumBlocks(); i++)
	{
		if(m_Blocks[i] == PIECESTATE_COMPLETE)
		{
			m_PeerSize += m_Parent->GetBlockSize(i);
		}
	}
}

void Client::Read_Request(void)
{
	DWORD Block;
	DWORD Offset;
	DWORD Length;
	CACHE c;
	BYTE * Data;

	if(m_Choke) return;

	Data = (BYTE *)m_Socket->GetData();

	Block = SDW(CDW(Data[5]));
	Offset = SDW(CDW(Data[9]));
	Length = SDW(CDW(Data[13]));

	TRACE("%s: Read_Request (%d, %d, %d)\n", m_Ip, Block, Offset, Length);

	if(
		Block >= m_Parent->GetNumBlocks() ||
		Block * m_Parent->GetBlockSize(0) + Offset + Length > m_Parent->GetFileSize() ||
		Offset + Length > m_Parent->GetBlockSize(Block) ||
		Offset > m_Parent->GetBlockSize(Block)
		)
	{
		// Bogus Client
		TRACE("%s: Bogus Data Received. Dumping Client\n", m_Ip);

		m_Socket->Close();
		m_Parent->KillClient(this);
		return;
	}

	c.Block = Block;
	c.Offset = Offset;
	c.Length = Length;

	m_SendCache.Add(c);

	if(m_SendCache.GetSize() == 1)
	{
		Send_Piece();
	}
}

void Client::Read_Piece(void)
{
	BYTE * Data;
	DWORD Block;
	DWORD Offset;
	DWORD Piece;
	DWORD PieceLen;
	DWORD Count;
	DWORD i;
	CString Temp;

	Data = (BYTE *)m_Socket->GetData();

	Block = SDW(CDW(Data[5]));
	Offset = SDW(CDW(Data[9]));
	Piece = Offset / PIECESIZE;
	PieceLen = SDW(CDW(Data[0])) - 9;

	TRACE("%s: Read_Piece (%d, %d)\n", m_Ip, Block, Piece);

	for(i = 0; i < (DWORD)m_RequestCache.GetSize(); i++)
	{
		if(
			((CACHE)m_RequestCache[i]).Block == Block &&
			((CACHE)m_RequestCache[i]).Piece == Piece
			)
		{
			m_RequestCache.RemoveAt(i);

			m_Parent->HavePiece(Block, Piece);
		}
	}

	if(m_Parent->GetBlockState(Block) == PIECESTATE_COMPLETE) return;
	if(m_Parent->GetPieceState(Block, Piece) == PIECESTATE_COMPLETE) return;

	if(Block * m_Parent->GetBlockSize(0) + Offset + PieceLen > m_Parent->GetFileSize())
	{
		TRACE("Bad Piece\n");
		return;
	}

	m_Parent->WritePiece(&Data[13], Block, Offset, PieceLen);
	m_Parent->SetPieceState(Block, Piece, PIECESTATE_COMPLETE);

	m_BytesDownloaded += PieceLen;
	m_Parent->AddBytesDownloaded(PieceLen);

	Count = 0;
	Temp = "";
	for(i = 0; i < m_Parent->GetNumPieces(Block); i++)
	{
		if(m_Parent->GetPieceState(Block, i) == PIECESTATE_COMPLETE)
		{
			Temp += "1";
			Count++;
		}else{
			Temp += "0";
		}
	}
	TRACE("%s\n", Temp);

	if(Count == m_Parent->GetNumPieces(Block))
	{
		if(m_Parent->IsBlockComplete(Block))
		{
			// Block Complete
			TRACE("Block %d Complete\n", Block);
//			Log("Bloque %d completado\n", Block);
			m_Parent->SetBlockState(Block, PIECESTATE_COMPLETE);
			m_Parent->HaveBlock(Block);
			m_Parent->UnCacheBlock(Block);
		}else{
			// Block Failed SHA1 Check
			TRACE("Block %d Failed Check\n", Block);
//			Log("Comprobación de bloque %d fallida\n", Block);

			m_Parent->SetBlockState(Block, PIECESTATE_INCOMPLETE);
			for(i = 0; i < m_Parent->GetNumPieces(Block); i++)
			{
				m_Parent->SetPieceState(Block, i, PIECESTATE_INCOMPLETE);
			}
		}
	}

	if(m_Parent->GetBlocksLeft() == 0)
	{
		m_Parent->OnFinish();
	}else{
		m_PieceTimeout = GetTickCount() + PIECETIMEOUT;
		RequestPiece();
	}

	m_Parent->UpdateProgress();
}

void Client::Read_Cancel(void)
{
	BYTE * Data;
	DWORD Block;
	DWORD Offset;
	DWORD Length;
	DWORD i;

	Data = (BYTE *)m_Socket->GetData();

	Block = SDW(CDW(Data[5]));
	Offset = SDW(CDW(Data[9]));
	Length = SDW(CDW(Data[13]));

	TRACE("%s: Read_Cancel (%d, %d, %d)\n", m_Ip, Block, Offset, Length);

	for(i = 1; i < (DWORD)m_SendCache.GetSize(); i++)
	{
		if(m_SendCache[i].Block == Block && m_SendCache[i].Offset == Offset && m_SendCache[i].Length == Length)
		{
			m_SendCache.RemoveAt(i);
			break;
		}
	}
}

void Client::Update(int Index)
{
	CString Temp;

	if(m_PieceTimeout != 0)
	{
		if(GetTickCount() > m_PieceTimeout)
		{
			m_PieceTimeout = 0;
			m_TimedOut = true;

			TRACE("%s: Timeout\n", m_Ip);

			while(m_RequestCache.GetSize())
			{
				TRACE("Timeout (%d, %d)\n", m_RequestCache[0].Block, m_RequestCache[0].Piece);
				if(m_Parent->GetPieceState(m_RequestCache[0].Block, m_RequestCache[0].Piece) != PIECESTATE_COMPLETE)
				{
					m_Parent->SetPieceState(m_RequestCache[0].Block, m_RequestCache[0].Piece, PIECESTATE_INCOMPLETE);
				}

				Send_Cancel(m_RequestCache[0].Block, m_RequestCache[0].Piece, false);
			}
		}
	}

	m_Socket->UpdateBps();

	m_BpsIn = m_Socket->GetRecvBps();
	m_BpsOut = m_Socket->GetSendBps();
	m_AvgBpsOut[m_CurSample] = m_Socket->GetSendBps();
	m_AvgBpsIn[m_CurSample] = m_Socket->GetRecvBps();
	m_CurSample = (m_CurSample + 1) % NUMDATASAMPLES;

	// Update Client List

	Temp.Format("%s:%d", m_Socket->GetIp(), m_Socket->GetPort());
	if(m_Attached) Temp += "*"; 
	m_List->SetItemText(Index, 0, Temp);
	m_List->SetItemText(Index, 1, m_TimedOut ? STRTBL_CLIENTSTATUS_TIMEDOUT : m_State == CSTATE_CONNECTING ? STRTBL_CLIENTSTATUS_CONNECTING : (!m_PeerChoke && m_Interested) ? STRTBL_CLIENTSTATUS_DOWNLOADING : !m_Choke ? STRTBL_CLIENTSTATUS_UPLOADING : STRTBL_CLIENTSTATUS_IDLE);
	if(m_State != CSTATE_ACTIVE)
	{
		m_List->SetItemText(Index, 2, "-");
		m_List->SetItemText(Index, 3, "-");
		m_List->SetItemText(Index, 4, "-");
		m_List->SetItemText(Index, 5, "-");
		m_List->SetItemText(Index, 6, "-");
		m_List->SetItemText(Index, 7, "-");
		m_List->SetItemText(Index, 8, "-");
		m_List->SetItemText(Index, 9, "-");
		m_List->SetItemText(Index, 10, "-");
		m_List->SetItemText(Index, 11, "-");
	}else{
		m_List->SetItemText(Index, 2, m_PeerChoke ? STRTBL_YES : STRTBL_NO);
		m_List->SetItemText(Index, 3, m_PeerInterested ? STRTBL_YES : STRTBL_NO);
		m_List->SetItemText(Index, 4, FormatBps(GetAvgBpsIn()));
		m_List->SetItemText(Index, 5, FormatBps(GetAvgBpsOut()));
		m_List->SetItemText(Index, 6, m_Choke ? STRTBL_YES : STRTBL_NO);
		m_List->SetItemText(Index, 7, m_Interested ? STRTBL_YES : STRTBL_NO);
		Temp.Format("%.2f%%", (float)m_PeerSize / m_Parent->GetFileSize() * 100.0f);
		m_List->SetItemText(Index, 8, Temp);
		Temp.Format("%s (%.2f%%)", FormatBytes(m_BytesDownloaded), (float)m_BytesDownloaded / (float)m_Parent->GetFileSize() * 100);
		m_List->SetItemText(Index, 9, Temp);
		Temp.Format("%s (%.2f%%)", FormatBytes(m_BytesUploaded), (float)m_BytesUploaded / (float)m_Parent->GetFileSize() * 100);
		m_List->SetItemText(Index, 10, Temp);
		m_List->SetItemText(Index, 11, m_ClientType);
	}
}

DWORD Client::GetBpsOut(void)
{
	return m_BpsOut;
}

DWORD Client::GetBpsIn(void)
{
	return m_BpsIn;
}

bool Client::IsSeed(void)
{
	return m_IsSeed;
}

bool Client::IsConnected(void)
{
	if(m_State != CSTATE_CONNECTING) return true;
	return false;
}

DWORD Client::GetAvgBpsOut(void)
{
	DWORD Result;
	DWORD i;

	Result = 0;
	for(i = 0; i < NUMDATASAMPLES; i++)
	{
		Result += m_AvgBpsOut[i];
	}

	Result /= NUMDATASAMPLES;

	return Result;
}

DWORD Client::GetAvgBpsIn(void)
{
	DWORD Result;
	DWORD i;

	Result = 0;
	for(i = 0; i < NUMDATASAMPLES; i++)
	{
		Result += m_AvgBpsIn[i];
	}

	Result /= NUMDATASAMPLES;

	return Result;
}

bool Client::IsChoked(void)
{
	return m_Choke;
}

bool Client::IsInterested(void)
{
	return m_PeerInterested;
}

void Client::SetMaxUploadRate(DWORD Bps)
{
	if(Bps < 1024 && Bps != 0) Bps = 1024;
	m_Socket->SetMaxUploadRate(Bps);
}

DWORD Client::GetMaxUploadRate(void)
{
	return m_Socket->GetMaxUploadRate();
}

void Client::SetList(CListCtrl * List)
{
	m_List = List;
}

DWORD Client::GetTimeUnchoked(void)
{
	return GetTickCount() - m_UnChokeTime;
}

void Client::Attach(DataSocket * s)
{
	int Index;

	if(m_Socket) MFree(m_Socket);
	m_Socket = s;
	s->SetCallbacks(NULL, _OnClose, _OnConnect, _OnReceive, _OnSend, this);

	Index = m_List->InsertItem(m_List->GetItemCount(), m_Ip);
	m_List->SetItemData(Index, (DWORD_PTR)this);

	if(m_Blocks) MFree(m_Blocks);
//	m_Blocks = new BYTE [m_Parent->GetNumBlocks()];
	m_Blocks = MAlloc(BYTE, m_Parent->GetNumBlocks());
	ZeroMemory(m_Blocks, m_Parent->GetNumBlocks());

	m_Attached = true;

	_OnConnect(s, this, 0);
}

DWORD Client::GetBytesDownloaded(void)
{
	return m_BytesDownloaded;
}

DWORD Client::GetBytesUploaded(void)
{
	return m_BytesUploaded;
}

void Client::CheckPeerId(void)
{
	// Id Torment
	if(m_PeerId[0] == 'X' && m_PeerId[7] == '-')
	{
		m_ClientType = "Torment ";
		m_ClientType += m_PeerId[1];
		m_ClientType += ".";
		m_ClientType += m_PeerId[2];
		m_ClientType += ".";
		m_ClientType += m_PeerId[3];
		m_ClientType += m_PeerId[4];
		m_ClientType += m_PeerId[5];
		m_ClientType += m_PeerId[6];
	}

	// Shadows
	if(m_PeerId[0] == 'S' && m_PeerId[4] == '-')
	{
		m_ClientType = "TheSHAD0W's ";
		m_ClientType += m_PeerId[1];
		m_ClientType += ".";
		m_ClientType += m_PeerId[2];
		m_ClientType += ".";
		m_ClientType += m_PeerId[3];
	}

	// UPnP?
	if(m_PeerId[0] == 'U' && m_PeerId[4] == '-')
	{
		m_ClientType = "UPnP ";
		m_ClientType += m_PeerId[1];
		m_ClientType += ".";
		m_ClientType += m_PeerId[2];
		m_ClientType += ".";
		m_ClientType += m_PeerId[3];
	}

	// Azureus
	if(memcmp(m_PeerId, "-AZ", 3) == 0 && m_PeerId[7] == '-')
	{
		m_ClientType = "Azureus ";
		m_ClientType += m_PeerId[3];
		m_ClientType += ".";
		m_ClientType += m_PeerId[4];
		m_ClientType += ".";
		m_ClientType += m_PeerId[5];
		m_ClientType += ".";
		m_ClientType += m_PeerId[6];
	}

	// MoonLightTorrent
	if(memcmp(m_PeerId, "-MT", 3) == 0 && m_PeerId[7] == '-')
	{
		m_ClientType = "MoonLightTorrent ";
		m_ClientType += m_PeerId[3];
		m_ClientType += ".";
		m_ClientType += m_PeerId[4];
		m_ClientType += ".";
		m_ClientType += m_PeerId[5];
		m_ClientType += ".";
		m_ClientType += m_PeerId[6];
	}

	// libtorrent
	if(memcmp(m_PeerId, "-LT", 3) == 0 && m_PeerId[7] == '-')
	{
		m_ClientType = "libtorrent ";
		m_ClientType += m_PeerId[3];
		m_ClientType += ".";
		m_ClientType += m_PeerId[4];
		m_ClientType += ".";
		m_ClientType += m_PeerId[5];
		m_ClientType += ".";
		m_ClientType += m_PeerId[6];
	}

	// Bittorrent X
	if(memcmp(m_PeerId, "-BX", 3) == 0 && m_PeerId[7] == '-')
	{
		m_ClientType = "Bittorrent X ";
		m_ClientType += m_PeerId[3];
		m_ClientType += ".";
		m_ClientType += m_PeerId[4];
		m_ClientType += ".";
		m_ClientType += m_PeerId[5];
		m_ClientType += ".";
		m_ClientType += m_PeerId[6];
	}

	if(memcmp(m_PeerId, "Plus", 4) == 0)
	{
		m_ClientType = "Plus";
	}
}

bool Client::IsAttached(void)
{
	return m_Attached;
}

float Client::GetPercentage(void)
{
	DWORD i;
	DWORD Complete;

	Complete = 0;

	if(m_State != CSTATE_ACTIVE) return 0;

	for(i = 0; i < m_Parent->GetNumBlocks(); i++)
	{
		if(m_Blocks[i] == PIECESTATE_COMPLETE)
		{
			Complete++;
		}
	}

	return (float)Complete / (float)m_Parent->GetNumBlocks();
}

DWORD Client::GetState(void)
{
	return m_State;
}

void Client::Pause(void)
{
	Send_Choke();
	Send_NotInterested();
}

void Client::Resume(void)
{
	CheckInterest();
}