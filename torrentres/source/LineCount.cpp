//////////////////////////////////////////////////
// COPYRIGHT NOTICE:
// 
//////////////////////////////////////////////////
// TorrenTres by Göth^Lorien
// $Id: LineCount.cpp,v 1.1 2004/03/31 20:39:09 deepb Exp $
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
#include "stdio.h"
#include "windows.h"

int Lines = 0;

void CountLines(const char * FileName)
{
        FILE * fp;
        char Buffer[256];

        fp = fopen(FileName, "rt");

        while(!feof(fp))
        {
                Buffer[0] = 0;
                fgets(Buffer, 256, fp);
                if(Buffer[0]!= 0) Lines++;
        }
}

void Count(const char * ext)
{
        WIN32_FIND_DATA fd;
        HANDLE hFind;

        hFind = FindFirstFile(ext, &fd);

        do
        {
                CountLines(fd.cFileName);
        }while(FindNextFile(hFind, &fd));

        FindClose(hFind);             
}

void main()
{
        Count("*.cpp");
        Count("*.h");

        printf("Lines: %d\n", Lines);
}
