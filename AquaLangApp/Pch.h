#pragma warning (disable:4512)
#define UNUSED(a)

#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0500
#include <windows.h>
#include <winsock2.h>
#include <tchar.h>
#include <assert.h>
#include <vector>
#include <map>
#include <string>
// for notification area support
#include <shellapi.h>
// for getting application data folder
#include <shlobj.h>
// for security attributes
#include <sddl.h>

#include <math.h>
#include <memory.h>
#include <stdio.h>

typedef std::basic_string<TCHAR, std::char_traits<TCHAR>, std::allocator<TCHAR> > _tstring;
#ifdef  _WIN64
typedef __int64    index_t;
#else
typedef _W64 int   index_t;
#endif

#define SLIM_AQUALANG