#pragma warning (disable:4512)
#define UNUSED(a)

#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0500 // this solves among the rest the TTM_ADDTOOL in UNICODE
#include <windows.h>
#include <winsock2.h>
#include <map>
#include <vector>
#include <string>
#include <assert.h>
#include <tchar.h>
#include <stdlib.h>
#include <errno.h>

// for ShellExecute
#include <shellapi.h>
// for getting application data folder
#include <shlobj.h>
// for security attributes
#include <sddl.h>

#include <oleacc.h>

#include <mlang.h>

#include <mshtml.h>
#include <mshtmhst.h>

#include <PsApi.h>
#include <math.h>

typedef std::basic_string<TCHAR, std::char_traits<TCHAR>, std::allocator<TCHAR> > _tstring;
#ifdef  _WIN64
typedef __int64    index_t;
#else
typedef _W64 int   index_t;
#endif

#define SLIM_AQUALANG
