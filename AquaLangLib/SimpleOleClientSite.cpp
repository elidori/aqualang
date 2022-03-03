#include "Pch.h"
#include "SimpleOleClientSite.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SimpleOleClientSite
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
STDMETHODIMP SimpleOleClientSite::QueryInterface(REFIID riid,void ** ppvObject)
{
	if(riid == IID_IUnknown || riid == IID_IOleClientSite)
		*ppvObject = (IOleClientSite*)this;
	else if(riid == IID_IOleInPlaceSite) // || riid == IID_IOleInPlaceSiteEx || riid == IID_IOleInPlaceSiteWindowless)
		*ppvObject = (IOleInPlaceSite*)this;
	else {
		*ppvObject = NULL;
		return E_NOINTERFACE;
	}
	return S_OK;
}

STDMETHODIMP_(ULONG) SimpleOleClientSite::AddRef(void)
{
	return 1;
}

STDMETHODIMP_(ULONG) SimpleOleClientSite::Release(void)
{
	return 1;
}


// IOleClientSite

STDMETHODIMP SimpleOleClientSite::SaveObject()
{
	return E_NOTIMPL;
}

STDMETHODIMP SimpleOleClientSite::GetMoniker(DWORD UNUSED(dwAssign), DWORD UNUSED(dwWhichMoniker), IMoniker ** UNUSED(ppmk))
{
	return E_NOTIMPL;
}

STDMETHODIMP SimpleOleClientSite::GetContainer(LPOLECONTAINER FAR* ppContainer)
{
	*ppContainer = NULL;
	return E_NOINTERFACE;
}

STDMETHODIMP SimpleOleClientSite::ShowObject()
{
	return NOERROR;
}

STDMETHODIMP SimpleOleClientSite::OnShowWindow(BOOL UNUSED(fShow))
{
	return E_NOTIMPL;
}

STDMETHODIMP SimpleOleClientSite::RequestNewObjectLayout()
{
	return E_NOTIMPL;
}

// IOleWindow
STDMETHODIMP SimpleOleClientSite::GetWindow(HWND FAR* lphwnd)
{
	*lphwnd = m_hwnd;
	return S_OK;
}

STDMETHODIMP SimpleOleClientSite::ContextSensitiveHelp(BOOL UNUSED(fEnterMode))
{
	return E_NOTIMPL;
}

// IOleInPlaceSite
STDMETHODIMP SimpleOleClientSite::CanInPlaceActivate()
{
	return S_OK;
}

STDMETHODIMP SimpleOleClientSite::OnInPlaceActivate()
{
	return S_OK;
}

STDMETHODIMP SimpleOleClientSite::OnUIActivate()
{
	return S_OK;
}

STDMETHODIMP SimpleOleClientSite::GetWindowContext(
	LPOLEINPLACEFRAME FAR* ppFrame,
	LPOLEINPLACEUIWINDOW FAR* ppDoc,
	LPRECT prcPosRect,
	LPRECT prcClipRect,
	LPOLEINPLACEFRAMEINFO lpFrameInfo)
{
	*ppFrame = NULL;
	*ppDoc = NULL;
	GetClientRect(m_hwnd,prcPosRect);
	GetClientRect(m_hwnd,prcClipRect);

	lpFrameInfo->fMDIApp = FALSE;
	lpFrameInfo->hwndFrame = m_hwnd;
	lpFrameInfo->haccel = NULL;
	lpFrameInfo->cAccelEntries = 0;

	return S_OK;
}

STDMETHODIMP SimpleOleClientSite::Scroll(SIZE UNUSED(scrollExtent))
{
	return E_NOTIMPL;
}

STDMETHODIMP SimpleOleClientSite::OnUIDeactivate(BOOL UNUSED(fUndoable))
{
	return S_OK;
}

STDMETHODIMP SimpleOleClientSite::OnInPlaceDeactivate()
{
	return S_OK;
}

STDMETHODIMP SimpleOleClientSite::DiscardUndoState()
{
	return E_NOTIMPL;
}

STDMETHODIMP SimpleOleClientSite::DeactivateAndUndo()
{
	return E_NOTIMPL;
}

STDMETHODIMP SimpleOleClientSite::OnPosRectChange(LPCRECT UNUSED(lprcPosRect))
{
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CNullStorage
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CNullStorage::QueryInterface(REFIID UNUSED(riid), void ** UNUSED(ppvObject))
{
	return E_NOTIMPL;
}

STDMETHODIMP_(ULONG) CNullStorage::AddRef(void)
{
	return 1;
}

STDMETHODIMP_(ULONG) CNullStorage::Release(void)
{
	return 1;
}

// IStorage
STDMETHODIMP CNullStorage::CreateStream(const WCHAR * UNUSED(pwcsName), DWORD UNUSED(grfMode), DWORD UNUSED(reserved1), DWORD UNUSED(reserved2), IStream ** UNUSED(ppstm))
{
	return E_NOTIMPL;
}

STDMETHODIMP CNullStorage::OpenStream(const WCHAR * UNUSED(pwcsName), void * UNUSED(reserved1), DWORD UNUSED(grfMode), DWORD UNUSED(reserved2), IStream ** UNUSED(ppstm))
{
	return E_NOTIMPL;
}

STDMETHODIMP CNullStorage::CreateStorage(const WCHAR * UNUSED(pwcsName), DWORD UNUSED(grfMode), DWORD UNUSED(reserved1) ,DWORD UNUSED(reserved2), IStorage ** UNUSED(ppstg))
{
	return E_NOTIMPL;
}

STDMETHODIMP CNullStorage::OpenStorage(const WCHAR * UNUSED(pwcsName), IStorage * UNUSED(pstgPriority), DWORD UNUSED(grfMode), SNB UNUSED(snbExclude), DWORD UNUSED(reserved), IStorage ** UNUSED(ppstg))
{
	return E_NOTIMPL;
}

STDMETHODIMP CNullStorage::CopyTo(DWORD UNUSED(ciidExclude), IID const * UNUSED(rgiidExclude), SNB UNUSED(snbExclude), IStorage * UNUSED(pstgDest))
{
	return E_NOTIMPL;
}

STDMETHODIMP CNullStorage::MoveElementTo(const OLECHAR * UNUSED(pwcsName), IStorage * UNUSED(pstgDest), const OLECHAR* UNUSED(pwcsNewName), DWORD UNUSED(grfFlags))
{
	return E_NOTIMPL;
}

STDMETHODIMP CNullStorage::Commit(DWORD UNUSED(grfCommitFlags))
{
	return E_NOTIMPL;
}

STDMETHODIMP CNullStorage::Revert(void)
{
	return E_NOTIMPL;
}

STDMETHODIMP CNullStorage::EnumElements(DWORD UNUSED(reserved1), void * UNUSED(reserved2), DWORD UNUSED(reserved3), IEnumSTATSTG ** UNUSED(ppenum))
{
	return E_NOTIMPL;
}

STDMETHODIMP CNullStorage::DestroyElement(const OLECHAR * UNUSED(pwcsName))
{
	return E_NOTIMPL;
}

STDMETHODIMP CNullStorage::RenameElement(const WCHAR * UNUSED(pwcsOldName), const WCHAR * UNUSED(pwcsNewName))
{
	return E_NOTIMPL;
}

STDMETHODIMP CNullStorage::SetElementTimes(const WCHAR * UNUSED(pwcsName), FILETIME const * UNUSED(pctime), FILETIME const * UNUSED(patime), FILETIME const * UNUSED(pmtime))
{
	return E_NOTIMPL;
}

STDMETHODIMP CNullStorage::SetClass(REFCLSID UNUSED(clsid))
{
	return S_OK;
}

STDMETHODIMP CNullStorage::SetStateBits(DWORD UNUSED(grfStateBits), DWORD UNUSED(grfMask))
{
	return E_NOTIMPL;
}

STDMETHODIMP CNullStorage::Stat(STATSTG * UNUSED(pstatstg), DWORD UNUSED(grfStatFlag))
{
	return E_NOTIMPL;
}
