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
// $Id: Lexicon.cpp,v 1.1 2004/03/31 20:39:09 deepb Exp $
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
// Lexicon.cpp
//
// Parses the NASTY Bencoded data into a useable form for C++
//

#include "StdAfx.h"
#include "lexicon.h"
#include "sha1.h"

Lexicon::Lexicon(void)
{
}

Lexicon::~Lexicon(void)
{
	while(m_Keys.GetSize())
	{
		if(m_Keys[0]->m_Value) MFree(m_Keys[0]->m_Value);
		MFree(m_Keys[0]);
		m_Keys.RemoveAt(0);
	}
}

void Lexicon::Build(const char * Data, DWORD DataLen)
{
	DWORD Pos;
	bool IsKey;

	Pos = 0;
	IsKey = true;

	ReadBlock(Data, DataLen, Pos, IsKey);
}

void Lexicon::ReadBlock(const char * Data, DWORD DataLen, DWORD & Pos, bool & IsKey)
{
	switch(Data[Pos])
	{
	case 'd':
		ReadDictionary(Data, DataLen, Pos, IsKey);
		return;

	case 'l':
		ReadList(Data, DataLen, Pos, IsKey);
		return;

	case 'i':
		ReadInteger(Data, DataLen, Pos, IsKey);
		return;
	}

	if(Data[Pos] >= '0' && Data[Pos] <= '9')
	{
		ReadString(Data, DataLen, Pos, IsKey);
		return;
	}

//	MessageBox(NULL, "Error parsing data. Unexpected token found while parsing bencoded data.", "Error", MB_OK | MB_ICONERROR);
	Pos = DataLen;

	KV * k;

//	k = new KV;
	k = MAlloc(KV, 1);
	
	k->m_Key = "failure reason";

//	k->m_Value = new char [256];
	k->m_Value = MAlloc(char, 256);
	sprintf(k->m_Value, "Datos corruptos: Token no esperado en el %d byte", Pos - 1);
	k->m_ValueLen = 256;

	m_Keys.Add(k);

	TRACE("Unknown Token: '%c' (0x%.2X)\n", Data[Pos], (unsigned char)Data[Pos]);

	Pos++;
}

void Lexicon::ReadDictionary(const char * Data, DWORD DataLen, DWORD & Pos, bool & IsKey)
{
	CString OldPath;
	CString OldKey;
	DWORD InfoStart;

	if(IsKey)
	{
		Pos++;

		while(Pos < DataLen && Data[Pos] != 'e')
		{
			ReadBlock(Data, DataLen, Pos, IsKey);
		}

		Pos++;
	}else{
		IsKey = true;
		
		OldPath = m_CurPath;
		OldKey = m_CurKey;
		
		m_CurPath += m_CurKey + ".";

		if(m_CurKey == "info")
		{
			InfoStart = Pos;
			ReadDictionary(Data, DataLen, Pos, IsKey);

			Sha1((BYTE *)m_InfoHash, (BYTE *)&Data[InfoStart], Pos - InfoStart);

			TRACE("InfoHash: ");
			for(int i = 0; i < 20; i++)
			{
				TRACE("%.2x", (BYTE)m_InfoHash[i]);
			}
			TRACE("\n");
		}else{
			ReadDictionary(Data, DataLen, Pos, IsKey);
		}

		m_CurPath = OldPath;
		m_CurKey = OldKey;
	}
}

void Lexicon::ReadList(const char * Data, DWORD DataLen, DWORD & Pos, bool & IsKey)
{
	int Item = 0;
	CString OldKey;
	CString OldPath;
	KV * k;

	if(IsKey)
	{
		Pos++;

		while(Pos < DataLen && Data[Pos] != 'e')
		{
			IsKey = false;

			m_CurKey.Format("%d", Item);

			ReadBlock(Data, DataLen, Pos, IsKey);

			Item++;
		}

//		k = new KV;
		k = MAlloc(KV, 1);
		k->m_Key = m_CurPath + "count";
//		k->m_Value = new char [8];
		k->m_Value = MAlloc(char, 8);
		sprintf(k->m_Value, "%d", Item);
		k->m_ValueLen = (DWORD)strlen(k->m_Value);

		m_Keys.Add(k);

		TRACE("%s=%s\n", k->m_Key, k->m_Value);

		Pos++;
	}else{
		IsKey = true;

		OldPath = m_CurPath;
		OldKey = m_CurKey;

		m_CurPath += m_CurKey + ".";

		ReadList(Data, DataLen, Pos, IsKey);

		m_CurPath = OldPath;
		m_CurKey = OldKey;
	}
}

void Lexicon::ReadInteger(const char * Data, DWORD DataLen, DWORD & Pos, bool & IsKey)
{
	int Value = 0;
	int Count = 0;
	KV * k;

	Pos++;

	while(Pos < DataLen && Data[Pos] != 'e')
	{
		Value *= 10;
		Value += Data[Pos++] - '0';
		Count++;
	}

	Pos++;

//	k = new KV;
	k = MAlloc(KV, 1);
	
	k->m_Key = m_CurPath + m_CurKey;

//	k->m_Value = new char [Count + 1];
	k->m_Value = MAlloc(char, Count + 1);
	sprintf(k->m_Value, "%d", Value);
	k->m_ValueLen = Count + 1;

	m_Keys.Add(k);

	TRACE("%s%s=%d\n", m_CurPath, m_CurKey, Value);

	IsKey = true;
}

void Lexicon::ReadString(const char * Data, DWORD DataLen, DWORD & Pos, bool & IsKey)
{
	int Len = 0;
	char * Value;
	KV * k;

	while(Pos < DataLen && Data[Pos] != ':')
	{
		Len *= 10;
		Len += Data[Pos++] - '0';
	}

	Pos++;

//	Value = new char [Len + 1];
	Value = MAlloc(char, Len + 1);
	memcpy(Value, &Data[Pos], Len);
	Value[Len] = 0;
	Pos += Len;

	if(IsKey)
	{
		m_CurKey = Value;

		MFree(Value);
	}else{
		TRACE("%s%s='%s'\n", m_CurPath, m_CurKey, Value);

//		k = new KV;
		k = MAlloc(KV, 1);

		k->m_Key = m_CurPath + m_CurKey;
		k->m_Value = Value;
		k->m_ValueLen = Len;

		m_Keys.Add(k);
	}

	IsKey = !IsKey;
}

CString Lexicon::ToString(const char * KeyName)
{
	int i;

	for(i = 0; i < m_Keys.GetSize(); i++)
	{
		if(m_Keys[i]->m_Key == KeyName)
		{
			return m_Keys[i]->m_Value;
		}
	}

	return "";
}

DWORD Lexicon::ToInteger(const char * KeyName)
{
	int i;

	for(i = 0; i < m_Keys.GetSize(); i++)
	{
		if(m_Keys[i]->m_Key == KeyName)
		{
			return atoi(m_Keys[i]->m_Value);
		}
	}

	return 0;
}

void Lexicon::ToBinary(const char * KeyName, char * Buffer, DWORD BufLen)
{
	int i;

	for(i = 0; i < m_Keys.GetSize(); i++)
	{
		if(m_Keys[i]->m_Key == KeyName)
		{
			DWORD v;
			v = m_Keys[i]->m_ValueLen;
			if(BufLen < m_Keys[i]->m_ValueLen)
			{
				throw;
			}

			memcpy(Buffer, m_Keys[i]->m_Value, m_Keys[i]->m_ValueLen);
			return;
		}
	}
}

void Lexicon::GetInfoHash(char * Hash)
{
	memcpy(Hash, m_InfoHash, 20);
}
