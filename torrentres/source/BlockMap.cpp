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
// $Id: BlockMap.cpp,v 1.1 2004/03/31 20:39:09 deepb Exp $
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
// BlockMap.cpp
//
// Blockmap Window
//

#include "stdafx.h"
#include "Torment.h"
#include "BlockMap.h"
#include "Settings.h"


// BlockMap

IMPLEMENT_DYNAMIC(BlockMap, CWnd)
BlockMap::BlockMap()
{
//	m_Blocks = new BLOCK [0];
	m_Blocks = MAlloc(BLOCK, 0);
	m_NumBlocks = 0;
}

BlockMap::~BlockMap()
{
	MFree(m_Blocks);
}

BEGIN_MESSAGE_MAP(BlockMap, CWnd)
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_SETCURSOR()
	ON_WM_CREATE()
	ON_WM_TIMER()
	ON_WM_VSCROLL()
	ON_WM_MOUSEWHEEL()
	ON_WM_LBUTTONUP()
END_MESSAGE_MAP()

// BlockMap message handlers

void BlockMap::OnPaint()
{
	CPaintDC pdc(this);
	CDC dc;
	CBitmap bm;
	CBitmap * OldBm;
	CRect cRect;
	DWORD x, y, i, j, f, Offset;
	CFont Font;
	bool First;
	long m;	// Multiplier
	long r, g, b;

	if(!IsWindowVisible()) return;

	GetClientRect(&cRect);

	if(Settings::GetBoolValue("DoubleBlockMap"))
	{
		m = 2;
	}else{
		m = 1;
	}

	bm.CreateCompatibleBitmap(&pdc, cRect.Width(), cRect.Height());

	dc.CreateCompatibleDC(&pdc);
	OldBm = dc.SelectObject(&bm);

	dc.FillSolidRect(&cRect, GetSysColor(COLOR_WINDOW));
//	dc.FillSolidRect(&cRect, RGB(255, 255, 255));

	if(m == 2)
	{
		Font.CreatePointFont(100, "Lucida Console", &dc);
	}else{
		Font.CreatePointFont(80, "Lucida Console", &dc);
	}
	dc.SelectObject(&Font);
	dc.SetTextColor(GetSysColor(COLOR_WINDOWTEXT));
	dc.SetBkMode(TRANSPARENT);

	x = 1;
	y = 1;
	Offset = GetScrollPos(SB_VERT);

	f = 0;
	for(i = 0; i < m_NumBlocks; i++)
	{
		First = true;
		while(f < (DWORD)m_Files.GetSize())
		{
			if(i >= m_Files[f].m_Block)
			{
				if(First)
				{
					First = false;
					if(i != 0) y += BLOCKHEIGHT * m;
				}
				x = 1;
				dc.TextOut(x, (y - Offset) + 1, m_Files[f].m_FileName);
				if(m == 2)
				{
					y += 20;
				}else{
					y += 14;
				}
				f++;
			}else{
				break;
			}
		}

		if(
			(long)y - (long)Offset > -(BLOCKHEIGHT * m) &&
			(long)y - (long)Offset < cRect.Height()
			)
		{
			dc.FillSolidRect(x, y - Offset, (BLOCKWIDTH * m), (BLOCKHEIGHT * m), GetSysColor(COLOR_ACTIVECAPTION));

			switch(m_Blocks[i].m_Mode)
			{
			case BLOCKMAP_SOLID:
				dc.FillSolidRect(x + 1 * m, y + 1 * m - Offset, (BLOCKWIDTH - 2) * m, (BLOCKHEIGHT - 2) * m, m_Blocks[i].m_Primary);
				break;

			case BLOCKMAP_FADE:
				r = (long)((1 - m_Blocks[i].m_Percent) * GetRValue(m_Blocks[i].m_Primary) + m_Blocks[i].m_Percent * GetRValue(m_Blocks[i].m_Secondary));
				if(r > 0xFF) r = 0xFF;
				if(r < 0) r = 0;
				g = (long)((1 - m_Blocks[i].m_Percent) * GetGValue(m_Blocks[i].m_Primary) + m_Blocks[i].m_Percent * GetGValue(m_Blocks[i].m_Secondary));
				if(g > 0xFF) g = 0xFF;
				if(g < 0) g = 0;
				b = (long)((1 - m_Blocks[i].m_Percent) * GetBValue(m_Blocks[i].m_Primary) + m_Blocks[i].m_Percent * GetBValue(m_Blocks[i].m_Secondary));
				if(b > 0xFF) b = 0xFF;
				if(b < 0) b = 0;
				dc.FillSolidRect(x + 1 * m, y + 1 * m - Offset, (BLOCKWIDTH - 2) * m, (BLOCKHEIGHT - 2) * m, RGB(r, g, b));
				break;

			case BLOCKMAP_SPLIT:
				j = (DWORD)(m_Blocks[i].m_Percent * ((BLOCKHEIGHT * m) - 2));
				dc.FillSolidRect(x + 1 * m, y + 1 * m - Offset, (BLOCKWIDTH - 2) * m, (BLOCKHEIGHT - 2) * m, m_Blocks[i].m_Primary);
				dc.FillSolidRect(x + 1 * m, y + 1 * m + (((BLOCKHEIGHT - 2) * m) - j) - Offset, (BLOCKWIDTH - 2) * m, j, m_Blocks[i].m_Secondary);
				break;
			}
		}

		x += (BLOCKWIDTH * m) + BLOCKSPACEX;
		
		if(x + (BLOCKWIDTH * m) > (DWORD)cRect.Width())
		{
			x = 1;
			y += (BLOCKHEIGHT * m) + BLOCKSPACEY * m;
		}
	}

	pdc.BitBlt(0, 0, cRect.Width(), cRect.Height(), &dc, 0, 0, SRCCOPY);

	dc.SelectObject(OldBm);

	if(y + BLOCKHEIGHT + BLOCKSPACEY <= (DWORD)cRect.Height())
	{
		EnableScrollBar(SB_VERT, ESB_DISABLE_BOTH);
		SetScrollRange(SB_VERT, 0, 1000);
		SetScrollPos(SB_VERT, 0);
	}else{
		EnableScrollBar(SB_VERT, ESB_ENABLE_BOTH);
		SetScrollRange(SB_VERT, 0, (y + BLOCKHEIGHT + BLOCKSPACEY) - cRect.Height());
	}
}

void BlockMap::Create(DWORD dwStyle, CWnd * Parent)
{
	CreateEx(0, AfxRegisterWndClass(0, 0, 0, 0), "BlockMap", dwStyle | WS_CHILD | WS_VSCROLL, CRect(0, 0, 0, 0), Parent, 0, NULL);
	ModifyStyleEx(0, WS_EX_CLIENTEDGE);

	SetScrollRange(SB_VERT, 0, 1);
	SetScrollPos(SB_VERT, 0);
}

void BlockMap::SetNumBlocks(DWORD Num)
{
	DWORD i;

	MFree(m_Blocks);
//	m_Blocks = new BLOCK [Num];
	m_Blocks = MAlloc(BLOCK, Num);
	m_NumBlocks = Num;

	for(i = 0; i < m_NumBlocks; i++)
	{
		m_Blocks[i].m_Percent = 0.0f;
		m_Blocks[i].m_Primary = GetSysColor(COLOR_WINDOW);
		m_Blocks[i].m_Secondary = GetSysColor(COLOR_WINDOW);
		m_Blocks[i].m_Mode = BLOCKMAP_SOLID;
	}
}

void BlockMap::SetBlockColors(DWORD Block, COLORREF Primary, COLORREF Secondary)
{
	if(Block >= m_NumBlocks) return;

	m_Blocks[Block].m_Primary = Primary;
	m_Blocks[Block].m_Secondary = Secondary;

	m_Dirty = true;
}

void BlockMap::SetBlockPercent(DWORD Block, float Percent)
{
	if(Block >= m_NumBlocks) return;

	m_Blocks[Block].m_Percent = Percent;

	m_Dirty = true;
}

void BlockMap::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);

	Invalidate(FALSE);
}

void BlockMap::SetBlockMode(DWORD Block, DWORD Mode)
{
	if(Block >= m_NumBlocks) return;

	m_Blocks[Block].m_Mode = Mode;

	m_Dirty = true;
}

BOOL BlockMap::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	SetCursor(LoadCursor(NULL, IDC_ARROW));

	return CWnd::OnSetCursor(pWnd, nHitTest, message);
}

int BlockMap::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	SCROLLINFO si;

	ZeroMemory(&si, sizeof(si));
	si.cbSize = sizeof(si);
	//si.nPage = 3 * (BLOCKHEIGHT + BLOCKSPACEY);
	si.fMask = /*SIF_PAGE |*/ SIF_DISABLENOSCROLL;
	SetScrollInfo(SB_VERT, &si);

	SetTimer(0, 250, NULL);

	return 0;
}

void BlockMap::OnTimer(UINT nIDEvent)
{
	if(m_Dirty)
	{
		Invalidate(FALSE);
	}

	CWnd::OnTimer(nIDEvent);
}

void BlockMap::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	int Min, Max;

	switch(nSBCode)
	{
	case SB_LINEDOWN:
		GetScrollRange(SB_VERT, &Min, &Max);
		if(GetScrollPos(SB_VERT) < Max)
		{
			SetScrollPos(SB_VERT, GetScrollPos(SB_VERT) + (BLOCKHEIGHT + BLOCKSPACEY));
		}
		break;

	case SB_LINEUP:
		GetScrollRange(SB_VERT, &Min, &Max);
		if(GetScrollPos(SB_VERT) > Min)
		{
			SetScrollPos(SB_VERT, GetScrollPos(SB_VERT) - (BLOCKHEIGHT + BLOCKSPACEY));
		}
		break;

	case SB_PAGEDOWN:
		GetScrollRange(SB_VERT, &Min, &Max);
		if(GetScrollPos(SB_VERT) < Max)
		{
			SetScrollPos(SB_VERT, GetScrollPos(SB_VERT) + 3 * (BLOCKHEIGHT + BLOCKSPACEY));
		}
		break;

	case SB_PAGEUP:
		GetScrollRange(SB_VERT, &Min, &Max);
		if(GetScrollPos(SB_VERT) > Min)
		{
			SetScrollPos(SB_VERT, GetScrollPos(SB_VERT) - 3 * (BLOCKHEIGHT + BLOCKSPACEY));
		}
		break;

	case SB_TOP:
		SetScrollPos(SB_VERT, 0);
		break;

	case SB_BOTTOM:
		GetScrollRange(SB_VERT, &Min, &Max);
		SetScrollPos(SB_VERT, Max);
		break;

	case SB_THUMBTRACK:
		SetScrollPos(SB_VERT, nPos);
		break;
	}

	m_Dirty = true;
	Invalidate();

	CWnd::OnVScroll(nSBCode, nPos, pScrollBar);
}

void BlockMap::SetFile(DWORD FirstBlock, const char * FileName)
{
	FILEBLOCK fb;

	fb.m_Block = FirstBlock;
	fb.m_FileName = FileName;

	m_Files.Add(fb);
}

BOOL BlockMap::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	if(zDelta < 0)
	{
		OnVScroll(SB_PAGEDOWN, 0, NULL);
	}else if (zDelta > 0)
	{
		OnVScroll(SB_PAGEUP, 0, NULL);
	}

	return CWnd::OnMouseWheel(nFlags, zDelta, pt);
}

void BlockMap::OnLButtonUp(UINT nFlags, CPoint point)
{
	SetFocus();

	CWnd::OnLButtonUp(nFlags, point);
}
