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
// $Id: ProgressBar.cpp,v 1.1 2004/03/31 20:39:09 deepb Exp $
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
// ProgressBar.cpp
//
// Custom Progress Bar Control
//

#include "stdafx.h"
#include "Torment.h"
#include "ProgressBar.h"


// ProgressBar

IMPLEMENT_DYNAMIC(ProgressBar, CWnd)
ProgressBar::ProgressBar()
{
	m_Min = 0;
	m_Max = 1;
	m_Pos = 0;
	m_IdleCount = -1;
	m_BarId = IDB_PROGRESSBAR_B;
}

ProgressBar::~ProgressBar()
{
}


BEGIN_MESSAGE_MAP(ProgressBar, CWnd)
	ON_WM_PAINT()
	ON_WM_TIMER()
END_MESSAGE_MAP()



// ProgressBar message handlers

void ProgressBar::Create(CWnd * Parent)
{
	CreateEx(0, AfxRegisterWndClass(0, 0, 0, 0), "ProgressBar", WS_CHILD | WS_VISIBLE, CRect(0, 0, 0, 0), Parent, 0, NULL);
}

void ProgressBar::SetRange(QWORD Max)
{
	m_Max = Max;
	KillTimer(0);
	m_IdleCount = -1;
	Invalidate(FALSE);
}

void ProgressBar::SetPos(QWORD Pos)
{
	m_Pos = Pos;
	KillTimer(0);
	m_IdleCount = -1;
	Invalidate(FALSE);
}

void ProgressBar::OnPaint()
{
	CPaintDC PDc(this);
	CRect cRect;
	CPen BorderPen(PS_SOLID, 1, GetSysColor(COLOR_ACTIVEBORDER));
	CDC Dc;
	CBitmap Buffer;
	CDC BarDc;
	CBitmap BarBm;
	DWORD Width;
	long x;
	DWORD d;
	DWORD BarWidth;
	BITMAP bm;

	GetClientRect(&cRect);

	Dc.CreateCompatibleDC(&PDc);
	Buffer.CreateCompatibleBitmap(&PDc, cRect.Width(), cRect.Height());
	Dc.SelectObject(&Buffer);

	BarDc.CreateCompatibleDC(&Dc);
	BarBm.LoadBitmap(m_BarId);
	BarDc.SelectObject(&BarBm);
	BarBm.GetBitmap(&bm);

	Dc.FillSolidRect(&cRect, GetSysColor(COLOR_WINDOW));
	Dc.SelectObject(&BorderPen);
	Dc.Rectangle(&cRect);

	if(	m_IdleCount == -1)
	{
		Width = (DWORD)(m_Pos / (float)m_Max * (cRect.Width() - 2));
		BarWidth = bm.bmWidth * (cRect.Height() - 2) / 12;

		for(x = 0; x < (long)Width; x += BarWidth)
		{
			d = BarWidth;
			if(Width - x < BarWidth) d = Width - x;
			Dc.StretchBlt(x + 1, 1, d, cRect.Height() - 2, &BarDc, 0, 0, bm.bmWidth * d / BarWidth, bm.bmHeight, SRCCOPY);
		}
	}else{
		Width = cRect.Width() - 2;
		BarWidth = bm.bmWidth * (cRect.Height() - 2) / 12;

		for(x = -(long)(m_IdleCount * bm.bmWidth / 10); x < (long)Width; x += BarWidth)
		{
			d = BarWidth;
			if(Width - x < BarWidth) d = Width - x;
			if(x < 0)
			{
				Dc.StretchBlt(1, 1, d, cRect.Height() - 2, &BarDc, -x, 0, bm.bmWidth * d / BarWidth, bm.bmHeight, SRCCOPY);
			}else{
				Dc.StretchBlt(x + 1, 1, d, cRect.Height() - 2, &BarDc, 0, 0, bm.bmWidth * d / BarWidth, bm.bmHeight, SRCCOPY);
			}
		}
	}

	PDc.BitBlt(0, 0, cRect.Width(), cRect.Height(), &Dc, 0, 0, SRCCOPY);
}

void ProgressBar::SetIdle(void)
{
	m_IdleCount = 0;
	SetTimer(0, 100, NULL);
}

void ProgressBar::OnTimer(UINT nIDEvent)
{
	m_IdleCount = (m_IdleCount + 1) % 10;
	Invalidate(FALSE);

	CWnd::OnTimer(nIDEvent);
}

void ProgressBar::SetBar(DWORD ResId)
{
	m_BarId = ResId;
}