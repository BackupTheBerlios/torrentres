//////////////////////////////////////////////////
// COPYRIGHT NOTICE:
// 
//////////////////////////////////////////////////
// TorrenTres by G�th^Lorien
// $Id: Memory.cpp,v 1.1 2004/03/31 20:39:09 deepb Exp $
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
#include "StdAfx.h"
#include "memory.h"

MemNode * MemoryList = NULL;

void MDump(void)
{
	while(MemoryList)
	{
		MFree(MemoryList->Ptr);
	}
}

DWORD MUsage(void)
{
	unsigned long Used;
	MemNode * m;

	m = MemoryList;

	Used = 0;
	while(m)
	{
		Used += m->Bytes;

//		TRACE("%.8X: %s[%d]: %s\n", m->Ptr, m->TypeName, m->Count, m->FileName);
		
		m = m->Next;
	}

	TRACE("Memory Used: %s\n", FormatBytes(Used));

	return Used;
}