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
// $Id: Splash.cpp,v 1.1 2004/03/31 20:39:09 deepb Exp $
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
// Splash.cpp
//
// Draws a Translucent Logo
//

#include "stdafx.h"
#include "Torment.h"
#include "Splash.h"

IMPLEMENT_DYNAMIC(Splash, CWnd)
Splash::Splash()
{
}

Splash::~Splash()
{
}

BEGIN_MESSAGE_MAP(Splash, CWnd)
	ON_WM_PAINT()
END_MESSAGE_MAP()

void Splash::OnPaint()
{
#if 0
	CPaintDC dc(this);
	CDC pdc;
	CBitmap bm;

	DWORD x, y;
	int a, c, i, r, g, b;
	float t, ct, mt;
	COLORREF Clr;
	BYTE * Data;

	BYTE Alpha[256][256];
	BYTE Color[256][256];
	HRSRC rs;
	HGLOBAL hg;
	
	pdc.CreateCompatibleDC(&dc);
	bm.CreateCompatibleBitmap(&dc, 256, 256);

	pdc.SelectObject(&bm);

	rs = FindResource(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_LOGO), "Binary");
	hg = LoadResource(AfxGetInstanceHandle(), rs);
	Data = (BYTE *)LockResource(hg);

	//i = 0;
	//for(y = 0; y < 256; y++)
	//{
	//	for(x = 0; x < 256; x++)
	//	{
	//		Alpha[x][y] = Data[i++];
	//		Color[x][y] = Data[i++];
	//	}
	//}

	x = 0;
	y = 0;
	i = 0;
	while(y < 256)
	{
		r = Data[i++];
		for(c = 0; c < r; c++)
		{
			Color[x][y] = Data[i];
			x++;
			if(x >= 256)
			{
				x = 0;
				y++;
			}
		}
		i++;
	}

	x = 0;
	y = 0;
	while(y < 256)
	{
		r = Data[i++];
		for(c = 0; c < r; c++)
		{
			Alpha[x][y] = Data[i];
			x++;
			if(x >= 256)
			{
				x = 0;
				y++;
			}
		}
		i++;
	}

	for(y = 0; y < 256; y++)
	{
		for(x = 0; x < 256; x++)
		{
			a = Alpha[x][y];
			c = Color[x][y];

			if(a != 0)
			{
				if(a == 0xFF)
				{
					pdc.SetPixel(x, y, RGB(c, c, c));
				}else{
					Clr = dc.GetPixel(x, y);

					t = (float)a / 255.0f;
					ct = c * t;
					mt = 1 - t;

					r = (int)(ct + mt * GetRValue(Clr));
					g = (int)(ct + mt * GetGValue(Clr));
					b = (int)(ct + mt * GetBValue(Clr));

					if(r > 255) r = 255;
					if(g > 255) g = 255;
					if(b > 255) b = 255;

					pdc.SetPixel(x, y, RGB(r, g, b));
				}
			}else{
				pdc.SetPixel(x, y, dc.GetPixel(x, y));
			}
		}
	}

	dc.BitBlt(0, 0, 256, 256, &pdc, 0, 0, SRCCOPY);
#endif
}
