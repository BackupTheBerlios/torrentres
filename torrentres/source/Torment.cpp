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
// $Id: Torment.cpp,v 1.1 2004/03/31 20:39:09 deepb Exp $
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
// Torment.cpp
//
// Application Initialization and Shutdown
//

#include "stdafx.h"
#include "Torment.h"
#include "Splash.h"
#include "Settings.h"
#include "direct.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

TormentApp theApp;
CMutex theMutex(0, "dBTorrenTresMutex");

BEGIN_MESSAGE_MAP(TormentApp, CWinApp)
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

TormentApp::TormentApp()
{
}

BOOL TormentApp::InitInstance()
{
	if(theMutex.Lock(100) == 0)
	{
#ifndef MULTI_INSTANCE
	HWND TormentWindow;
	char TempPath[256];

		if(GetNumArgs() == 2)
		{
			TormentWindow = FindWindow(NULL, "TorrenTres");
			if(TormentWindow == NULL)
			{
				MessageBox(NULL, STRTBL_TORMENT_BOGUSINSTANCE, STRTBL_TORMENT_ERROR, MB_OK | MB_ICONERROR);
			}else{
				GetTempPath(sizeof(TempPath), TempPath);
				DeleteFile(CString(TempPath) + "Temp.torrent");
				CopyFile(GetArg(1), CString(TempPath) + "Temp.torrent", FALSE);
				SetForegroundWindow(TormentWindow);
				SetFocus(TormentWindow);
				SendMessage(TormentWindow, WM_USER_TORMENT_LOADTORRENT, 0, 0);
			}
		}else{
			MessageBox(NULL, STRTBL_TORMENT_DUPLICATEINSTANCE, STRTBL_TORMENT_ERROR, MB_OK | MB_ICONERROR);
		}
		return TRUE;
#endif
	}

	InitCommonControls();

	CWinApp::InitInstance();

	if (!AfxSocketInit())
	{
//		Log("Error inicializando Sockets\n");
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		return FALSE;
	}

	AfxEnableControlContainer();

#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_CHECK_ALWAYS_DF | _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG));
	//_CrtSetBreakAlloc(388);
#endif

	// Connections
	Settings::InitBool("UnlimitedUploadRate", true);
	Settings::InitBool("UnlimitedConnections", true);

	Settings::InitDWord("MaxUploadRate", 10, 1, 9999);
	Settings::InitDWord("MaxUploads", 4, 4, 9999);
	Settings::InitDWord("MaxConnections", 50, 25, 9999);
	Settings::InitDWord("Port", 6881, 0, 65535);
//DeepB
	Settings::InitString("ForcedIP", "");
	Settings::InitBool("ForceIP", false);
//END DeepB

	// Files
	Settings::InitBool("AskToSave", true);
	Settings::InitBool("CreateSubDirs", true);
	Settings::InitBool("RemoveUnderscores", true);
	Settings::InitBool("UseDefaultSaveDir", false);
	Settings::InitString("DefaultSaveDir", "");
	Settings::InitDWord("DownloadAlgorithm", 0, 0, 2);
	Settings::InitBool("AutoLoad", false);
	Settings::InitString("AutoLoadDir", "");
	Settings::InitBool("History", false);
	Settings::InitString("HistoryDir", "");

	// Windows
	Settings::InitBool("MinimizeToSystray", false);
	Settings::InitBool("StatusMinimizeToSystray", false);
	Settings::InitBool("StatusOnLoad", false);
	Settings::InitBool("DoubleBlockMap", false);

	// Misc
	Settings::InitBool("ShowBandwidthMonitor", true);

	// WindowPos
	Settings::InitDWord("WindowPosX", 0, 0, 65535);
	Settings::InitDWord("WindowPosY", 0, 0, 65535);
	Settings::InitDWord("WindowPosW", 0, 0, 65535);
	Settings::InitDWord("WindowPosH", 0, 0, 65535);
	srand(time(NULL));

	//Splash s;

	//s.CreateEx(0, AfxRegisterWndClass(0, 0, 0, 0), STRTBL_TORMENT_TITLE, WS_POPUP, CRect(0, 0, 256, 256), NULL, 0, NULL);
	//s.CenterWindow();
	//s.ShowWindow(SW_SHOW);
	//s.OnPaint();
	//Sleep(1000);
	//s.DestroyWindow();

	m_pMainWnd = &m_Wnd;

	if(m_Wnd.CreateEx(WS_EX_ACCEPTFILES, AfxRegisterWndClass(0, 0, 0, LoadIcon(IDR_MAINFRAME)), STRTBL_TORMENT_TITLE, WS_OVERLAPPEDWINDOW, CRect(0, 0, 790, 400), NULL, 0, NULL))
	{
		if ( (Settings::GetDWordValue("WindowPosW") == 0) || (Settings::GetDWordValue("WindowPosH") == 0) )
			m_Wnd.CenterWindow();
		else
			m_Wnd.SetWindowPos(&CWnd::wndTop, Settings::GetDWordValue("WindowPosX"),Settings::GetDWordValue("WindowPosY"),Settings::GetDWordValue("WindowPosW"),Settings::GetDWordValue("WindowPosH"),SWP_SHOWWINDOW);
		m_Wnd.ShowWindow(SW_SHOW);
	}

	if(GetNumArgs() == 2)
	{
		m_Wnd.OpenTorrent(GetArg(1), true);
	}

	return TRUE;
}

int TormentApp::ExitInstance()
{
	theMutex.Unlock();

	MDump();

	return CWinApp::ExitInstance();
}

DWORD TormentApp::GetNumArgs(void)
{
	bool InQuotes = false;
	char * CmdLine;
	DWORD i;
	DWORD Result = 1;

	CmdLine = GetCommandLine();

	for(i = 0; CmdLine[i] != 0; i++)
	{
		switch(CmdLine[i])
		{
		case '"':
			InQuotes = !InQuotes;
			break;
		}

		if(!InQuotes)
		{
			if(CmdLine[i] == ' ' && CmdLine[i + 1] != ' ' && CmdLine[i + 1] != 0)
			{
				Result++;
			}
		}
	}

	TRACE("Args: %d\n", Result);
	return Result;
}

CString TormentApp::GetArg(DWORD Arg)
{
	bool InQuotes = false;
	char * CmdLine;
	DWORD i;
	DWORD Curr;
	CString Result;

	CmdLine = GetCommandLine();

	Curr = 0;
	for(i = 0; CmdLine[i] != 0; i++)
	{
		switch(CmdLine[i])
		{
		case '"':
			InQuotes = !InQuotes;
			break;
		}
		
		if(!InQuotes)
		{
			if(CmdLine[i] == ' ' && CmdLine[i + 1] != ' ')
			{
				Curr++;
				continue;
			}
		}

		if(Curr == Arg && CmdLine[i] != '"')
		{
			Result += CmdLine[i];
		}
	}

//	Log("Argumento: %d : %s\n", Arg, Result);
	TRACE("Arg %d: %s\n", Arg, Result);
	return Result;
}