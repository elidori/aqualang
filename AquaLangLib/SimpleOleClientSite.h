#ifndef _SIMPLE_OLE_CLIENT_SITE_HEADER_
#define _SIMPLE_OLE_CLIENT_SITE_HEADER_

class SimpleOleClientSite :
	public IOleClientSite,
	public IOleInPlaceSite
{
public:
	SimpleOleClientSite(HWND hwnd) : m_hwnd(hwnd) {}

	// IUnknown pure virtuals {
	STDMETHODIMP QueryInterface(REFIID riid,void ** ppvObject);
	STDMETHODIMP_(ULONG) AddRef(void);
	STDMETHODIMP_(ULONG) Release(void);
	// }
	// IOleClientSite pure virtuals {
	STDMETHODIMP SaveObject();
	STDMETHODIMP GetMoniker(DWORD dwAssign,DWORD dwWhichMoniker,IMoniker ** ppmk);
	STDMETHODIMP GetContainer(LPOLECONTAINER FAR* ppContainer);
	STDMETHODIMP ShowObject();
	STDMETHODIMP OnShowWindow(BOOL fShow);
	STDMETHODIMP RequestNewObjectLayout();
	// }
	// IOleWindow pure virtuals {
	STDMETHODIMP GetWindow(HWND FAR* lphwnd);
	STDMETHODIMP ContextSensitiveHelp(BOOL fEnterMode);
	// }
	// IOleInPlaceSite pure virtuals {
	STDMETHODIMP CanInPlaceActivate();
	STDMETHODIMP OnInPlaceActivate();
	STDMETHODIMP OnUIActivate();
	STDMETHODIMP GetWindowContext(LPOLEINPLACEFRAME FAR* lplpFrame,LPOLEINPLACEUIWINDOW FAR* lplpDoc,LPRECT lprcPosRect,LPRECT lprcClipRect,LPOLEINPLACEFRAMEINFO lpFrameInfo);
	STDMETHODIMP Scroll(SIZE scrollExtent);
	STDMETHODIMP OnUIDeactivate(BOOL fUndoable);
	STDMETHODIMP OnInPlaceDeactivate();
	STDMETHODIMP DiscardUndoState();
	STDMETHODIMP DeactivateAndUndo();
	STDMETHODIMP OnPosRectChange(LPCRECT lprcPosRect);
	// }
private:
	HWND m_hwnd;
};


class CNullStorage : public IStorage
{
public:
	// IUnknown pure virtuals {
	STDMETHODIMP QueryInterface(REFIID riid,void ** ppvObject);
	STDMETHODIMP_(ULONG) AddRef(void);
	STDMETHODIMP_(ULONG) Release(void);
	// }
	// IStorage pure virtuals {
	STDMETHODIMP CreateStream(const WCHAR * pwcsName,DWORD grfMode,DWORD reserved1,DWORD reserved2,IStream ** ppstm);
	STDMETHODIMP OpenStream(const WCHAR * pwcsName,void * reserved1,DWORD grfMode,DWORD reserved2,IStream ** ppstm);
	STDMETHODIMP CreateStorage(const WCHAR * pwcsName,DWORD grfMode,DWORD reserved1,DWORD reserved2,IStorage ** ppstg);
	STDMETHODIMP OpenStorage(const WCHAR * pwcsName,IStorage * pstgPriority,DWORD grfMode,SNB snbExclude,DWORD reserved,IStorage ** ppstg);
	STDMETHODIMP CopyTo(DWORD ciidExclude,IID const * rgiidExclude,SNB snbExclude,IStorage * pstgDest);
	STDMETHODIMP MoveElementTo(const OLECHAR * pwcsName,IStorage * pstgDest,const OLECHAR* pwcsNewName,DWORD grfFlags);
	STDMETHODIMP Commit(DWORD grfCommitFlags);
	STDMETHODIMP Revert(void);
	STDMETHODIMP EnumElements(DWORD reserved1,void * reserved2,DWORD reserved3,IEnumSTATSTG ** ppenum);
	STDMETHODIMP DestroyElement(const OLECHAR * pwcsName);
	STDMETHODIMP RenameElement(const WCHAR * pwcsOldName,const WCHAR * pwcsNewName);
	STDMETHODIMP SetElementTimes(const WCHAR * pwcsName,FILETIME const * pctime,FILETIME const * patime,FILETIME const * pmtime);
	STDMETHODIMP SetClass(REFCLSID clsid);
	STDMETHODIMP SetStateBits(DWORD grfStateBits,DWORD grfMask);
	STDMETHODIMP Stat(STATSTG * pstatstg,DWORD grfStatFlag);
	// }
};

#endif //_SIMPLE_OLE_CLIENT_SITE_HEADER_
