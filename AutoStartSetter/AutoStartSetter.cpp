#include <windows.h>
#include <string>
#include <shlobj.h>
#include <tchar.h>

class ComInitializer
{
public:
	ComInitializer()
	{
		m_fInitialized = SUCCEEDED(CoInitialize(NULL));
	}
	virtual ~ComInitializer()
	{
		if(m_fInitialized)
		{
			CoUninitialize();
		}
	}

	bool IsInitialized() const { return m_fInitialized; }
private:
	bool m_fInitialized;
};

template <class T>
class ReleasableItem
{
public:
	ReleasableItem() : m_pObject(NULL) {}

	virtual ~ReleasableItem() 	{ if(m_pObject) m_pObject->Release(); }
	T **operator&() { return &m_pObject; }
	T *operator->() { return m_pObject; }
	ReleasableItem &operator=(const ReleasableItem &r)
	{
		return operator=(r.m_pObject);
	}
	ReleasableItem &operator=(const T *pObject)
	{
		T *pOld = m_pObject;
		
		m_pObject = pObject;
		if(m_pObject)
		{
			m_pObject->AddRef();
		}
		if(pOld)
		{
			pOld->Release();
		}
		return *this;
	}
	bool operator==(const ReleasableItem &r)
	{
		return operator==(r.m_pObject);
	}
	bool operator==(const T *pObject)
	{
		return m_pObject == pObject;
	}
private:
	T *m_pObject;
};

void SetString(std::wstring &rTargetString, LPCWSTR SourceString)
{
	if(SourceString == NULL)
	{
		rTargetString = L"";
		return;
	}
	size_t len = wcslen(SourceString);

	rTargetString.reserve(len + 1);
	rTargetString.resize(len);
	
	if(len > 0)
	{
		wcsncpy_s(&rTargetString[0], len + 1, SourceString, len);
	}
}

void Usage()
{
	printf("Usage:\nAutoStartSetter.exe <-a or -r> <link name> [in case of -a <application full path>]\n");
}

int _tmain(int argc, TCHAR *argv[])
{
	if(argc < 2)
	{
		Usage();
		return -1;
	}
	bool fAdd = false;
	if(_tcscmp(argv[1], _T("-a")) == 0)
	{
		fAdd = true;
		if(argc < 4)
		{
			Usage();
			return -1;
		}
	}
	else if(_tcscmp(argv[1], _T("-r")) == 0)
	{
		fAdd = false;
		if(argc < 3)
		{
			Usage();
			return -1;
		}
	}
	else
	{
		Usage();
		return -1;
	}

	// create the link full path
	TCHAR Path[1024];
	HRESULT hr = SHGetFolderPath(NULL, /*CSIDL_COMMON_STARTUP*/CSIDL_STARTUP, NULL, 0, Path);
	if(hr != S_OK)
	{
		printf("Failed getting startup folder path\n");
		return -1;
	}
	_tcscat_s(Path, sizeof(Path) / sizeof(Path[0]), _T("\\"));
	_tcscat_s(Path, sizeof(Path) / sizeof(Path[0]), argv[2]);
	_tcscat_s(Path, sizeof(Path) / sizeof(Path[0]), _T(".lnk"));

	// delete the link if already exists
	::DeleteFile(Path);

	if(fAdd)
	{
		// create shortcut
		ComInitializer ComInit;
		if(!ComInit.IsInitialized())
		{
			printf("Failed initializing com\n");
			return -1;
		}

		ReleasableItem<IShellLink> pShellLink;
		hr = CoCreateInstance(
							CLSID_ShellLink,
							NULL,
							CLSCTX_INPROC_SERVER,
							IID_IShellLink,
							(LPVOID*)&pShellLink
							);
		if(FAILED(hr))
		{
			printf("Failed getting CLSID_ShellLink (hr=0x%x)\n", hr);
			return -1;
		}

		hr = pShellLink->SetPath(argv[3]);
		if(FAILED(hr))
		{
			printf("Failed setting link path (hr=0x%x)\n", hr);
			return -1;
		}

		ReleasableItem<IPersistFile> pPersistFile;
		hr = pShellLink->QueryInterface(IID_IPersistFile, (LPVOID*)&pPersistFile);
		if(FAILED(hr))
		{
			printf("Failed query interface for IID_IPersistFile (hr=0x%x)\n", hr);
			return -1;
		}

		std::wstring wPath;
		SetString(wPath, Path);

		hr = pPersistFile->Save(wPath.c_str(), TRUE);
		if(FAILED(hr))
		{
			printf("Failed saving link (hr=0x%x)\n", hr);
			return -1;
		}
	}

	return 0;
}