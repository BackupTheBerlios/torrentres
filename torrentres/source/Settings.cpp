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
// $Id: Settings.cpp,v 1.1 2004/03/31 20:39:09 deepb Exp $
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
// Settings.cpp
//
// Manages All Major Settings
//

#include "StdAfx.h"
#include "settings.h"
#include "atlbase.h"

CArray<Settings::KV> Settings::m_Keys;

void Settings::InitBool(const char * Var, bool Default)
{
	CRegKey Key;
	char Buffer[32];
	ULONG BufLen;
	KV k;

	Key.Create(HKEY_CURRENT_USER, "Software\\DeepB\\TorrenTres");

	k.Min = 0;
	k.Max = 1;

	BufLen = sizeof(Buffer);
	if(Key.QueryStringValue(Var, Buffer, &BufLen) == ERROR_SUCCESS)
	{
		if(stricmp(Buffer, "true") == 0)
		{
			k.Key = Var;
			k.Value = 1;
			m_Keys.Add(k);
		}else{
			k.Key = Var;
			k.Value = 0;
			m_Keys.Add(k);
		}
	}else{
		if(Default)
		{
			Key.SetStringValue(Var, "true");
			k.Key = Var;
			k.Value = 1;
			m_Keys.Add(k);
		}else{
			Key.SetStringValue(Var, "false");
			k.Key = Var;
			k.Value = 0;
			m_Keys.Add(k);
		}
	}

	Key.Close();
}

void Settings::InitDWord(const char * Var, DWORD Default, DWORD Min, DWORD Max)
{
	CRegKey Key;
	DWORD Result;
	KV k;

	Key.Create(HKEY_CURRENT_USER, "Software\\DeepB\\TorrenTres");

	k.Min = Min;
	k.Max = Max;

	if(Key.QueryDWORDValue(Var, Result) == ERROR_SUCCESS)
	{
		if(Result < Min) Result = Min;
		if(Result > Max) Result = Max;
		k.Key = Var;
		k.Value = Result;
		m_Keys.Add(k);
	}else{
		Key.SetDWORDValue(Var, Default);
		k.Key = Var;
		k.Value = Default;
		m_Keys.Add(k);
	}

	Key.Close();
}

void Settings::InitString(const char * Var, const char * Default)
{
	CRegKey Key;
	char Buffer[256];
	DWORD BufLen;
	KV k;

	Key.Create(HKEY_CURRENT_USER, "Software\\DeepB\\TorrenTres");

	BufLen = sizeof(Buffer);
	if(Key.QueryStringValue(Var, Buffer, &BufLen) == ERROR_SUCCESS)
	{
		k.Key = Var;
		k.Str= Buffer;
		m_Keys.Add(k);
	}else{
		Key.SetStringValue(Var, Default);
		k.Key = Var;
		k.Str = Default;
		m_Keys.Add(k);
	}

	Key.Close();
}

bool Settings::GetBoolValue(const char * Var)
{
	DWORD i;

	for(i = 0; i < (DWORD)m_Keys.GetSize(); i++)
	{
		if(m_Keys[i].Key.CompareNoCase(Var) == 0)
		{
			if(m_Keys[i].Value)
			{
				return true;
			}else{
				return false;
			}
		}
	}

	return false;
}

DWORD Settings::GetDWordValue(const char * Var)
{
	DWORD i;

	for(i = 0; i < (DWORD)m_Keys.GetSize(); i++)
	{
		if(m_Keys[i].Key.CompareNoCase(Var) == 0)
		{
			return m_Keys[i].Value;
		}
	}

	return 0;
}

CString Settings::GetStringValue(const char * Var)
{
	DWORD i;

	for(i = 0; i < (DWORD)m_Keys.GetSize(); i++)
	{
		if(m_Keys[i].Key.CompareNoCase(Var) == 0)
		{
			return m_Keys[i].Str;
		}
	}

	return "";
}

void Settings::SetBoolValue(const char * Var, bool Value)
{
	CRegKey Key;
	DWORD i;

	Key.Create(HKEY_CURRENT_USER, "Software\\DeepB\\TorrenTres");

	if(Value)
	{
		Key.SetStringValue(Var, "true");
	}else{
		Key.SetStringValue(Var, "false");
	}

	Key.Close();

	for(i = 0; i < (DWORD)m_Keys.GetSize(); i++)
	{
		if(m_Keys[i].Key.CompareNoCase(Var) == 0)
		{
			m_Keys[i].Value = Value ? 1 : 0;
			break;
		}
	}
}

void Settings::SetDWordValue(const char * Var, DWORD Value)
{
	CRegKey Key;
	DWORD i;

	Key.Create(HKEY_CURRENT_USER, "Software\\DeepB\\TorrenTres");

	Key.SetDWORDValue(Var, Value);

	Key.Close();

	for(i = 0; i < (DWORD)m_Keys.GetSize(); i++)
	{
		if(m_Keys[i].Key.CompareNoCase(Var) == 0)
		{
			m_Keys[i].Value = Value;
			break;
		}
	}
}

void Settings::SetStringValue(const char * Var, const char * Value)
{
	CRegKey Key;
	DWORD i;

	Key.Create(HKEY_CURRENT_USER, "Software\\DeepB\\TorrenTres");

	Key.SetStringValue(Var, Value);

	Key.Close();

	for(i = 0; i < (DWORD)m_Keys.GetSize(); i++)
	{
		if(m_Keys[i].Key.CompareNoCase(Var) == 0)
		{
			m_Keys[i].Str = Value;
			break;
		}
	}
}
