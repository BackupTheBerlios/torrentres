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
// $Id: DataGraph.cpp,v 1.1 2004/03/31 20:39:09 deepb Exp $
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
// DataGraph.cpp
//
// This is the Bandwidth Monitor in the main window
//

#include "stdafx.h"
#include "Torment.h"
#include "DataGraph.h"


// DataGraph

IMPLEMENT_DYNAMIC(DataGraph, CWnd)
DataGraph::DataGraph()
{
//	m_Data = new DWORD [100 * 2];
	m_Data = MAlloc(DWORD, 100 * 2);
	ZeroMemory(m_Data, sizeof(DWORD) * 100 * 2);
}

DataGraph::~DataGraph()
{
	MFree(m_Data);
}


BEGIN_MESSAGE_MAP(DataGraph, CWnd)
	ON_WM_PAINT()
	ON_WM_SETCURSOR()
	ON_WM_SIZE()
END_MESSAGE_MAP()

// DataGraph message handlers

void DataGraph::Create(CWnd * Parent)
{
	CreateEx(WS_EX_CLIENTEDGE, AfxRegisterWndClass(0, 0, 0, 0), "DataGraph", WS_CHILD | WS_VISIBLE, CRect(0, 0, 0, 0), Parent, 0, NULL);
}

void DataGraph::OnPaint()
{
	CPaintDC pdc(this);
	CDC dc;
	CBitmap bm;
	CBitmap * OldBm;
	CRect cRect;
	DWORD i, j;
	DWORD High;
	CPen Red(PS_SOLID, 1, RGB(255, 0, 0));
	CPen Green(PS_SOLID, 1, RGB(0, 255, 0));
	CPen Gray(PS_SOLID, 1, RGB(0x40, 0x40, 0x40));
	CPen * OldPen;
	CFont Font;
	DWORD Height;

	if(!IsWindowVisible()) return;

	GetClientRect(&cRect);

	bm.CreateCompatibleBitmap(&pdc, cRect.Width(), cRect.Height());

	dc.CreateCompatibleDC(&pdc);
	OldBm = dc.SelectObject(&bm);

	dc.FillSolidRect(&cRect, GetSysColor(COLOR_ACTIVECAPTION));

	High = 0;

	for(i = 0; i < 100; i++)
	{
		if(m_Data[i * 2 + 0] > High) High = m_Data[i * 2 + 0];
		if(m_Data[i * 2 + 1] > High) High = m_Data[i * 2 + 1];
	}

	//High /= 10 * 1024;
	//High++;
	//High *= 10 * 1024;

	if(High != 0)
	{
		OldPen = dc.SelectObject(&Gray);

		Height = cRect.Height() - 1;

		i = 10 * 1024;
		while(i < High)
		{
			dc.MoveTo(0, Height - (i * Height / High));
			dc.LineTo(cRect.Width(), Height - (i * Height / High));

			i += 10 * 1024;
		}

		dc.SelectObject(OldPen);
		OldPen = dc.SelectObject(&Red);

		j = m_Data[0 * 2 + 1] * Height / High;
		dc.MoveTo(0, Height - j);

		for(i = 1; i < 100; i++)
		{
			j = m_Data[i * 2 + 1] * Height / High;
			dc.LineTo(i * cRect.Width() / 99, Height - j);
		}

		dc.SelectObject(OldPen);
		OldPen = dc.SelectObject(&Green);

		j = m_Data[0 * 2 + 0] * Height / High;
		dc.MoveTo(0, Height - j);

		for(i = 1; i < 100; i++)
		{
			j = m_Data[i * 2 + 0] * Height / High;
			dc.LineTo(i * cRect.Width() / 99, Height - j);
		}

		dc.SelectObject(OldPen);
	}

	Font.CreatePointFont(80, "Trebuchet MS", &dc);
	dc.SelectObject(&Font);
	dc.SetTextColor(GetSysColor(COLOR_CAPTIONTEXT));
	dc.SetBkMode(TRANSPARENT);
	dc.TextOut(0, 0, FormatBps(High));

	dc.SelectObject(&Green);
	dc.MoveTo(5, cRect.Height() - 20);
	dc.LineTo(35, cRect.Height() - 20);
	dc.TextOut(40, cRect.Height() - 25, STRTBL_DATAGRAPH_IN);
	dc.SelectObject(&Red);
	dc.MoveTo(5, cRect.Height() - 10);
	dc.LineTo(35, cRect.Height() - 10);
	dc.TextOut(40, cRect.Height() - 15, STRTBL_DATAGRAPH_OUT);

	pdc.BitBlt(0, 0, cRect.Width(), cRect.Height(), &dc, 0, 0, SRCCOPY);

	dc.SelectObject(OldBm);
}

void DataGraph::SetNextBlock(DWORD In, DWORD Out)
{
	DWORD i;

	for(i = 0; i < 99; i++)
	{
		m_Data[i * 2 + 0] = m_Data[(i + 1) * 2 + 0];
		m_Data[i * 2 + 1] = m_Data[(i + 1) * 2 + 1];
	}

	m_Data[99 * 2 + 0] = In;
	m_Data[99 * 2 + 1] = Out;
}

BOOL DataGraph::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	SetCursor(AfxGetApp()->LoadStandardCursor(IDC_ARROW));

	return CWnd::OnSetCursor(pWnd, nHitTest, message);
}

void DataGraph::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);

	Invalidate(FALSE);
}
