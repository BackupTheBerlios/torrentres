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
// $Id: stdafx.cpp,v 1.1 2004/03/31 20:39:09 deepb Exp $
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

// stdafx.cpp : source file that includes just the standard includes
// Torment.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

// Todo: Process These 2 Strings and Build Key3 Out of it, For Copyright Security Purposes
// Then Lock the program out if either of these Strings are changed.
//const static char Key1[] = "REF:MTG:Eternal Dragon:5-5:Flying:5+2P";
//const static char Key2[] = "Copyright © Dark Knight Software. Coded by Jason Zimmer";
//const static DWORD Key3 = 0x00000000;

//////////////////////////////////////////////////
//
// Some Misc Formating functions
// Should really go in thier own header
// But lazyness wins out in the end
//

CString FormatBps(DWORD Bps)
{
	CString Result;

	if(Bps >= 1024 * 1024)
	{
		Result.Format("%.2f MBps", Bps / 1024.0f / 1024.0f);
	}else if(Bps >= 1024)
	{
		Result.Format("%.2f KBps", Bps / 1024.0f);
	}else{
		Result.Format("%d Bps", Bps);
	}

	return Result;
}

CString FormatBytes(QWORD Bytes)
{
	CString Result;

	if(Bytes >= (QWORD)1024 * 1024 * 1024 * 1024)
	{
		Result.Format("%-.2f TB", Bytes / 1024.0f / 1024.0f / 1024.0f / 1024.0f);
	}else if(Bytes >= 1024 * 1024 * 1024)
	{
		Result.Format("%-.2f GB", Bytes / 1024.0f / 1024.0f / 1024.0f);
	}else if(Bytes >= 1024 * 1024)
	{
		Result.Format("%-.2f MB", Bytes / 1024.0f / 1024.0f);
	}else if(Bytes >= 1024)
	{
		Result.Format("%-.2f KB", Bytes / 1024.0f);
	}else{
		Result.Format("%d Bytes", Bytes);
	}

	return Result;
}

CString FormatTime(DWORD Seconds)
{
	CString Result;
	CString Temp;

	if(Seconds >= 60 * 60 * 24 * 365)
	{
		if((Seconds / 60 / 60 / 24 / 365) == 1)
		{
			Temp.Format("1 Año ");
		}else{
			Temp.Format("%d Años ", (Seconds / 60 / 60 / 24 / 365));
		}
		Result += Temp;
	}

	if(Seconds >= 60 * 60 * 24)
	{
		if((Seconds / 60 / 60 / 24) == 1)
		{
			Temp.Format("1 Día ");
		}else{
			Temp.Format("%d Días ", (Seconds / 60 / 60 / 24) % 365);
		}
		Result += Temp;
	}

	if(Seconds >= 60 * 60)
	{
		if((Seconds / 60 / 60) % 24 == 1)
		{
			Temp.Format("1 Hora ");
		}else{
			Temp.Format("%d Horas ", (Seconds / 60 / 60) % 24);
		}
		Result += Temp;
	}

	if(Seconds >= 60)
	{
		if((Seconds / 60) % 60 == 1)
		{
			Temp.Format("1 Minuto ");
		}else{
			Temp.Format("%d Minutos ", (Seconds / 60) % 60);
		}
		Result += Temp;
	}

	if(Seconds % 60 == 1)
	{
		Temp.Format("1 Segundo");
	}else{
		Temp.Format("%d Segundos", Seconds % 60);
	}
	Result += Temp;

	return Result;
}

CString HTTPEncode(const char * Data, int Len)
{
	CString Result;
	CString Temp;
	int i;

	for(i = 0; i < Len; i++)
	{
		Temp.Format("%%%.2X", (unsigned char)Data[i]);
		Result += Temp;
	}

	return Result;
}

int Round(double v)
{
	if(v - (int)v >= .5) return (int)v + 1;
	return (int)v;
}