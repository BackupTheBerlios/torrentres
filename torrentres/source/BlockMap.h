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
// $Id: BlockMap.h,v 1.1 2004/03/31 20:39:09 deepb Exp $
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
// BlockMap.h
//
// Blockmap Window
//

#pragma once


// BlockMap

#define BLOCKMAP_SOLID	0
#define BLOCKMAP_FADE	1
#define BLOCKMAP_SPLIT	2

#define BLOCKWIDTH		8
#define BLOCKHEIGHT		12
#define BLOCKSPACEX		-1
#define BLOCKSPACEY		1

class BlockMap : public CWnd
{
	DECLARE_DYNAMIC(BlockMap)

	struct BLOCK
	{
		COLORREF m_Primary;
		COLORREF m_Secondary;
		float m_Percent;
		DWORD m_Mode;
	};

	struct FILEBLOCK
	{
		DWORD m_Block;
		CString m_FileName;
	};

public:
	BlockMap();
	virtual ~BlockMap();
	void Create(DWORD dwStyle, CWnd * Parent);

	void SetNumBlocks(DWORD Num);
	void SetBlockColors(DWORD Block, COLORREF Primary, COLORREF Secondary);
	void SetBlockPercent(DWORD Block, float Percent);
	void SetBlockMode(DWORD Block, DWORD Mode);
	void SetFile(DWORD FirstBlock, const char * FileName);

protected:
	DECLARE_MESSAGE_MAP()

	DWORD m_NumBlocks;
	BLOCK * m_Blocks;
	bool m_Dirty;
	CArray<FILEBLOCK> m_Files;

public:
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
};


