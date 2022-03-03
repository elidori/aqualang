#ifndef _STREAM_CONVERT_HEADER_
#define _STREAM_CONVERT_HEADER_

void SetString(std::string &rTargetString, LPCSTR SourceString, size_t SourceStringSize = -1, int CodePage = CP_UTF8);
void SetString(std::string &rTargetString, LPCWSTR SourceString, size_t SourceStringSize = -1, int CodePage = CP_UTF8);
void SetString(std::wstring &rTargetString, LPCSTR SourceString, size_t SourceStringSize = -1, int CodePage = CP_UTF8);
void SetString(std::wstring &rTargetString, LPCWSTR SourceString, size_t SourceStringSize = -1, int CodePage = CP_UTF8);

void SetString(char *TargetString, size_t TargetSize, LPCSTR SourceString, size_t SourceStringSize = -1, int CodePage = CP_UTF8);
void SetString(char *TargetString, size_t TargetSize, LPCWSTR SourceString, size_t SourceStringSize = -1, int CodePage = CP_UTF8);
void SetString(WCHAR *TargetString, size_t TargetSize, LPCSTR SourceString, size_t SourceStringSize = -1, int CodePage = CP_UTF8);
void SetString(WCHAR *TargetString, size_t TargetSize, LPCWSTR SourceString, size_t SourceStringSize = -1, int CodePage = CP_UTF8);

#endif // _STREAM_CONVERT_HEADER_