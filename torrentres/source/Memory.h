//////////////////////////////////////////////////
// COPYRIGHT NOTICE:
// 
//////////////////////////////////////////////////
// TorrenTres by Göth^Lorien
// $Id: Memory.h,v 1.1 2004/03/31 20:39:09 deepb Exp $
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
#pragma once

#define MAlloc(Type, Count) Mem_Alloc<Type>(#Type, Count, __LINE__, __FILE__)

struct MemNode
{
	void * Ptr;
	char TypeName[64];
	char FileName[256];
	int Count;
	int Bytes;

	MemNode * Next;
};

extern MemNode * MemoryList;

template <class Type>
Type * Mem_Alloc(const char * TypeName, int Count, int Line, const char * File)
{
	Type * Result;

	Result = new Type [Count];

	MemNode * m;

	m = new MemNode;

	m->Ptr = Result;
	m->Count = Count;
	m->Bytes = sizeof(Type) * Count;
	strcpy(m->TypeName, TypeName);
	sprintf(m->FileName, "%s: Line %d", File, Line);
	m->Next = NULL;

	if(!MemoryList)
	{
		MemoryList = m;
	}else{
		m->Next = MemoryList;
		MemoryList = m;
	}

	return Result;
}

template <class Type>
void MFree(Type * Ptr)
{
	MemNode * p;
	MemNode * m;
	bool Found = false;

	m = MemoryList;
	p = NULL;
	while(m)
	{
		if(Ptr == m->Ptr)
		{
			if(p == NULL)
			{
				MemoryList = m->Next;
			}else{
				p->Next = m->Next;
			}

			delete m;
			Found = true;
			break;
		}
		
		p = m;
		m = m->Next;
	}

	if(Found == false)
	{
		TRACE("Rouge Pointer (%.8X)\n", (unsigned long)Ptr);
	}

	delete [] Ptr;
}

void MDump(void);
DWORD MUsage(void);
