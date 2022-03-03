#ifndef _WEB_BROWSE_SHARED_MEM_STRUCT_HEADER_
#define _WEB_BROWSE_SHARED_MEM_STRUCT_HEADER_

#define MAX_PAGE_NAME_LENGTH	1024	// Common practice

struct BrowserParams
{
	char Url[MAX_PAGE_NAME_LENGTH];
	int ScrollDown;
};

struct WebBrowseSharedMemStruct
{
	int SequenceCount;
	bool fTerminate;
	bool fClientActive; // this is the only field modified by the client

	TCHAR DialogTitle[256];

	BrowserParams Search;
	BrowserParams Translate;

	TCHAR SearchString[MAX_PAGE_NAME_LENGTH];
	RECT CaretRect;
};

#endif // _WEB_BROWSE_SHARED_MEM_STRUCT_HEADER_