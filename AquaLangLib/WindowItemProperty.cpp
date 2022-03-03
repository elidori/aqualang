#include "Pch.h"
#include "WindowItemProperty.h"
#include "PsApiLoader.h"
#include "TimeHelpers.h"
#include "resource.h"

#pragma warning(disable:4355)

#define PROPERTY_ALLOCATION_CHUNK 1000

const WindowItemPropertyTypes WindowItemProperty::m_gPropertyTypes;

static const __int64 OUT_OF_DATE_THRESHOLD_MSEC = __int64(180) * 24 * 3600 * 1000; // half a year

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// WindowItemProperty
/////////////////////////////////////////////////////////////////////////////////////////////////////////
WindowItemProperty::WindowItemProperty(int Type)
	: xmlFixedSizeStructure(-1),
	m_Type(Type)
{
	xmlString *pTypeStringXml = m_gPropertyTypes.FindXmlString(Type);
	assert(pTypeStringXml != NULL);
	if(pTypeStringXml)
	{
		AddItem(*pTypeStringXml);
	}
}

WindowItemProperty *WindowItemProperty::CreateFromXml(
										XmlNode &rxItemNode,
										int NameId,
										int &rnKey
										)
{
	rnKey = -1;

	_tstring TypeString;
	if(!xmlString::CheckValue(rxItemNode, IDS_STRING_TYPE, TypeString))
		return NULL;

	T_WindowItemPropertyCreator Creator = NULL;
	int Type = -1;
	bool fIsReadOnly = false;
	if(!m_gPropertyTypes.Find(TypeString.c_str(), Type, Creator, fIsReadOnly))
		return NULL;

	WindowItemProperty *pNew = (*Creator)(NameId);
	if(!pNew)
		return NULL;

	rnKey = Type;
	return pNew;
}

WindowItemProperty *WindowItemProperty::CreateFromBinStorage(
						ReadableBinStorage &rBinStorage,
						int NameId,
						int &rnKey
						)
{
	rnKey = -1;

	int Type = -1;
	if(!xmlPropertyTypeValue::PeekType(rBinStorage, Type))
		return NULL;

	T_WindowItemPropertyCreator Creator = NULL;
	bool fIsReadOnly = false;
	if(!m_gPropertyTypes.Find(Type, Creator, fIsReadOnly))
		return NULL;

	WindowItemProperty *pNew = (*Creator)(NameId);
	if(!pNew)
		return NULL;

	if(!pNew->GetXmlItem().Deserialize(rBinStorage))
	{
		pNew->Release();
		return NULL;
	}

	rnKey = Type;
	return pNew;
}

bool WindowItemProperty::AddPropertyFromHWND(
						HWND hwnd,
						int TypeIndex,
						int NameId,
						int &rnKey,
						WindowItemProperty* &rpNew
						)
{
	rnKey = -1;
	rpNew = NULL;

	int Type = -1;
	T_WindowItemPropertyCreator Creator = NULL;
	if(!m_gPropertyTypes.GetType(TypeIndex, Type, Creator))
		return false;

	WindowItemProperty *pNew = (*Creator)(NameId);
	if(!pNew)
		return true;
	if(pNew->Initialize(hwnd))
	{
		rnKey = Type;
		rpNew = pNew;
	}
	else
	{
		pNew->Release();
	}

	return true;
}

bool WindowItemProperty::AddPropertyFromId(
						LPCTSTR SubObjectId,
						int NameId,
						int &rnKey,
						WindowItemProperty* &rpNew
						)
{
	rnKey = -1;
	rpNew = NULL;

	WindowItemProperty *pNew = WindowItemSubObjectIdProperty::Create(NameId, SubObjectId);
	if(!pNew)
		return false;

	rnKey = PropertyTypeSubObjectId;
	rpNew = pNew;

	return true;
}

LPCTSTR WindowItemProperty::GetTypeString(_tstring &rString) const
{
	xmlString *pTypeStringXml = m_gPropertyTypes.FindXmlString(m_Type);
	if(pTypeStringXml == NULL)
		return _T("");
	return pTypeStringXml->GetAssignedString(rString);
}

void WindowItemProperty::GetDebugString(_tstring &rString) const
{
	if(IsValid())
	{
		_tstring TypeStr;
		rString += GetTypeString(TypeStr);
		rString += _T("=");
		_tstring WindowString;
		GetValueNode().GetStringValue(WindowString, _T(""));
		rString += WindowString;
		rString += _T(";");
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// WindowItemHWNDProperty
/////////////////////////////////////////////////////////////////////////////////////////////////////////
ObjectPool<WindowItemHWNDProperty> WindowItemPropertyBase<WindowItemHWNDProperty>::m_PropertyPool(PROPERTY_ALLOCATION_CHUNK);

WindowItemHWNDProperty::WindowItemHWNDProperty()
	: WindowItemPropertyBase<WindowItemHWNDProperty>(PropertyTypeHWND),
	m_hwndXmlValue(IDS_STRING_VALUE),
	m_CreationTimeXmlValue(IDS_STRING_CREATIONTIME)
{
	AddItem(m_hwndXmlValue);
	AddItem(m_CreationTimeXmlValue);
}

WindowItemHWNDProperty::~WindowItemHWNDProperty()
{
}

bool WindowItemHWNDProperty::Initialize(HWND hwnd)
{
	if(!::IsWindow(hwnd))
		return false;

	// hwnd
	m_hwndXmlValue.SetInt((__int64)hwnd);

	// process time
	DWORD dwProcessID = 0;
	::GetWindowThreadProcessId (hwnd, &dwProcessID);
	HANDLE hProcess = ::OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, dwProcessID);
	if(hProcess != NULL)
	{
		FILETIME CreationTime;
		FILETIME ExitTime;
		FILETIME KernelTime;
		FILETIME UserTime;
		if(::GetProcessTimes(hProcess, &CreationTime, &ExitTime, &KernelTime, &UserTime))
		{
			m_CreationTimeXmlValue.SetTime(CreationTime);
		}
		::CloseHandle(hProcess);
	}
	return true;
}

void WindowItemHWNDProperty::Invalidate()
{
	m_hwndXmlValue.Clean();
	m_CreationTimeXmlValue.Clean();
}

bool WindowItemHWNDProperty::IsValid() const
{
	return m_hwndXmlValue.IsValid();
}

bool WindowItemHWNDProperty::Update(const WindowItemProperty &rRef)
{
	if(rRef.GetType() != GetType())
		return false;
	const WindowItemHWNDProperty &rHWNDRef = (const WindowItemHWNDProperty &)rRef;

	m_hwndXmlValue.Set(rHWNDRef.m_hwndXmlValue);
	m_CreationTimeXmlValue.Set(rHWNDRef.m_CreationTimeXmlValue);
	return true;
}

bool WindowItemHWNDProperty::IsMatching(const WindowItemProperty &rRef) const
{
	if(rRef.GetType() != GetType())
		return false;
	const WindowItemHWNDProperty &rHWNDRef = (WindowItemHWNDProperty &)rRef;

	if(m_hwndXmlValue.GetInt() == NULL || m_hwndXmlValue.GetInt() != rHWNDRef.m_hwndXmlValue.GetInt())
		return false;
	if(!m_CreationTimeXmlValue.IsEqualeTo(rHWNDRef.m_CreationTimeXmlValue))
		return false;
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// WindowItemCaptionProperty
/////////////////////////////////////////////////////////////////////////////////////////////////////////
ObjectPool<WindowItemCaptionProperty> WindowItemPropertyBase<WindowItemCaptionProperty>::m_PropertyPool(PROPERTY_ALLOCATION_CHUNK);

WindowItemCaptionProperty::WindowItemCaptionProperty()
	: WindowItemPropertyBase<WindowItemCaptionProperty>(PropertyTypeCaption),
	m_CaptionXmlValue(IDS_STRING_VALUE),
	m_fIsMatchingMustXmlValue(IDS_STRING_ISMUST)
{
	AddItem(m_CaptionXmlValue);
	AddItem(m_fIsMatchingMustXmlValue);
}

WindowItemCaptionProperty::~WindowItemCaptionProperty()
{
}

bool WindowItemCaptionProperty::Initialize(HWND hwnd)
{
	_tstring Caption;
	if(!GetWindowCaption(hwnd, Caption))
	{
//		TCHAR ClassName[1024];
//		if(::GetClassName(hwnd, ClassName, sizeof(ClassName) / sizeof(ClassName[0])) == 0)
			return false;
//		m_Caption = ClassName;
	}
	m_CaptionXmlValue.SetString(Caption.c_str());
	m_fIsMatchingMustXmlValue.SetBool((::GetWindowLong(hwnd, GWL_STYLE) & WS_DLGFRAME) != 0);
	return true;
}

bool WindowItemCaptionProperty::Update(const WindowItemProperty &rRef)
{
	if(rRef.GetType() != GetType())
		return false;
	const WindowItemCaptionProperty &rCaptionRef = (const WindowItemCaptionProperty &)rRef;
	m_CaptionXmlValue.Set(rCaptionRef.m_CaptionXmlValue);
	m_fIsMatchingMustXmlValue.Set(rCaptionRef.m_fIsMatchingMustXmlValue);
	return true;
}

void WindowItemCaptionProperty::Invalidate()
{
	m_CaptionXmlValue.Clean();
	m_fIsMatchingMustXmlValue.Clean();
}

bool WindowItemCaptionProperty::IsValid() const
{
	return m_CaptionXmlValue.IsValid();
}

bool WindowItemCaptionProperty::IsMatching(const WindowItemProperty &rRef) const
{
	if(rRef.GetType() != GetType())
		return false;
	return m_CaptionXmlValue.IsEqualeTo(((const WindowItemCaptionProperty &)rRef).m_CaptionXmlValue);
}

bool WindowItemCaptionProperty::IsMatchingMust() const
{
	return m_fIsMatchingMustXmlValue.GetBool(false);
}

bool WindowItemCaptionProperty::GetWindowCaption(HWND hwnd, _tstring &Text)
{
	Text = _T("");

	int TextLength = ::GetWindowTextLength(hwnd);
	if(TextLength == 0)
		return false;
	Text.reserve(TextLength + 1);
	Text.resize(TextLength);
	::GetWindowText(hwnd, &Text[0], TextLength + 1);
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// WindowItemIndexInParentProperty
/////////////////////////////////////////////////////////////////////////////////////////////////////////
ObjectPool<WindowItemIndexInParentProperty> WindowItemPropertyBase<WindowItemIndexInParentProperty>::m_PropertyPool(PROPERTY_ALLOCATION_CHUNK);

WindowItemIndexInParentProperty::WindowItemIndexInParentProperty()
	: WindowItemPropertyBase<WindowItemIndexInParentProperty>(PropertyTypeIndexInParent),
	m_nIndexXmlValue(IDS_STRING_VALUE)
{
	AddItem(m_nIndexXmlValue);
}

WindowItemIndexInParentProperty::~WindowItemIndexInParentProperty()
{
}

bool WindowItemIndexInParentProperty::Initialize(HWND hwnd)
{
	HWND hParentWindow = ::GetParent(hwnd);
	if(!::IsWindow(hParentWindow))
		return false;

	ChildIndexFinder IndexFinder;
	int Index = IndexFinder.GetIndexInParent(hwnd, hParentWindow);
	if(Index == -1)
		return false;

	m_nIndexXmlValue.SetInt(Index);
	return true;
}

bool WindowItemIndexInParentProperty::Update(const WindowItemProperty &rRef)
{
	if(rRef.GetType() != GetType())
		return false;
	const WindowItemIndexInParentProperty &rIndexInParentRef = (const WindowItemIndexInParentProperty &)rRef;
	m_nIndexXmlValue.Set(rIndexInParentRef.m_nIndexXmlValue);
	return true;
}

void WindowItemIndexInParentProperty::Invalidate()
{
	m_nIndexXmlValue.Clean();
}

bool WindowItemIndexInParentProperty::IsValid() const
{
	return m_nIndexXmlValue.IsValid();
}

bool WindowItemIndexInParentProperty::IsMatching(const WindowItemProperty &rRef) const
{
	if(rRef.GetType() != GetType())
		return false;
	return m_nIndexXmlValue.IsEqualeTo(((const WindowItemIndexInParentProperty &)rRef).m_nIndexXmlValue);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// ChildIndexFinder
/////////////////////////////////////////////////////////////////////////////////////////////////////////
int ChildIndexFinder::GetIndexInParent(HWND hwndChild, HWND hwndParent)
{
	m_hwnd = hwndChild;
	m_nNextChildIndex = 0;
	m_nChildIndex = -1;

	::EnumChildWindows(hwndParent, EnumChildProc, (LPARAM)(ChildIndexFinder *)this);

	return m_nChildIndex;
}

BOOL CALLBACK ChildIndexFinder::EnumChildProc(HWND hwndChild, LPARAM lParam)
{
	ChildIndexFinder *pThis = (ChildIndexFinder *)lParam;

	if(pThis != NULL)
	{
		pThis->CheckChildWindow(hwndChild);
	}
	return TRUE;
}

void ChildIndexFinder::CheckChildWindow(HWND hwndChild)
{
	if(m_hwnd == hwndChild)
	{
		m_nChildIndex = m_nNextChildIndex;
	}
	m_nNextChildIndex++;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// WindowItemProcessNameProperty
/////////////////////////////////////////////////////////////////////////////////////////////////////////
ObjectPool<WindowItemProcessNameProperty> WindowItemPropertyBase<WindowItemProcessNameProperty>::m_PropertyPool(PROPERTY_ALLOCATION_CHUNK);

WindowItemProcessNameProperty::WindowItemProcessNameProperty()
	: WindowItemPropertyBase<WindowItemProcessNameProperty>(PropertyTypeProcessName),
	m_ProcessNameXmlValue(IDS_STRING_VALUE)
{
	AddItem(m_ProcessNameXmlValue);
}

WindowItemProcessNameProperty::~WindowItemProcessNameProperty()
{
}

bool WindowItemProcessNameProperty::Initialize(HWND hwnd)
{
	HWND hParentWindow = ::GetParent(hwnd);
	if(::IsWindow(hParentWindow))
		return false;
	PsApiLoader &rPsApiLoader = GetPsApiLoader();
	if(!rPsApiLoader.IsOpen())
		return false;

	DWORD dwProcessID = 0;
	::GetWindowThreadProcessId (hwnd, &dwProcessID);
	HANDLE hProcess = ::OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, dwProcessID);
	if(hProcess != NULL)
	{
		TCHAR szEXEName[1024];
		_tcscpy_s(szEXEName, sizeof(szEXEName) / sizeof(szEXEName[0]), _T(""));
		rPsApiLoader.GetProcessImageFileName(hProcess, szEXEName, sizeof(szEXEName)/sizeof(szEXEName[0]));

		LPTSTR SimpleName = _tcsrchr(szEXEName, _T('\\'));
		if(SimpleName == NULL)
		{
			SimpleName = szEXEName;
		}
		else
		{
			SimpleName++;
		}

		m_ProcessNameXmlValue.SetString(SimpleName);

		::CloseHandle(hProcess);
	}

	return true;
}

bool WindowItemProcessNameProperty::Update(const WindowItemProperty &rRef)
{
	if(rRef.GetType() != GetType())
		return false;
	const WindowItemProcessNameProperty &rProcessNameRef = (const WindowItemProcessNameProperty &)rRef;
	m_ProcessNameXmlValue.Set(rProcessNameRef.m_ProcessNameXmlValue);
	return true;
}

void WindowItemProcessNameProperty::Invalidate()
{
	m_ProcessNameXmlValue.Clean();
}

bool WindowItemProcessNameProperty::IsValid() const
{
	return (m_ProcessNameXmlValue.IsValid());
}

bool WindowItemProcessNameProperty::IsMatching(const WindowItemProperty &rRef) const
{
	if(rRef.GetType() != GetType())
		return false;
	return m_ProcessNameXmlValue.IsCaseInsensitiveEqualeTo(((WindowItemProcessNameProperty &)rRef).m_ProcessNameXmlValue);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// WindowItemUsageTimeProperty
/////////////////////////////////////////////////////////////////////////////////////////////////////////
ObjectPool<WindowItemUsageTimeProperty> WindowItemPropertyBase<WindowItemUsageTimeProperty>::m_PropertyPool(PROPERTY_ALLOCATION_CHUNK);

WindowItemUsageTimeProperty::WindowItemUsageTimeProperty()
	: WindowItemPropertyBase<WindowItemUsageTimeProperty>(PropertyTypeUsageTime),
	m_UsageTimeXmlValue(IDS_STRING_VALUE)
{
	AddItem(m_UsageTimeXmlValue);
}

WindowItemUsageTimeProperty::~WindowItemUsageTimeProperty()
{
}

bool WindowItemUsageTimeProperty::CheckTimeTooOld(const FileTime &rCurrentTime) const
{
	FILETIME UsageTime;
	m_UsageTimeXmlValue.GetTime(UsageTime);

	return (rCurrentTime.GetDiffMsec(UsageTime) > OUT_OF_DATE_THRESHOLD_MSEC);
}

bool WindowItemUsageTimeProperty::Initialize(HWND hwnd)
{
	// the window must be either a dialog or a top window
	if((::GetWindowLong(hwnd, GWL_STYLE) & WS_DLGFRAME) == 0 && ::IsWindow(::GetParent(hwnd)))
		return false;

	FileTime CurrentTime;
	m_UsageTimeXmlValue.SetTime(CurrentTime.GetFILETIME());
	return true;
}

bool WindowItemUsageTimeProperty::Update(const WindowItemProperty &rRef)
{
	if(rRef.GetType() != GetType())
		return false;
	const WindowItemUsageTimeProperty &rUsageTimeRef = (const WindowItemUsageTimeProperty &)rRef;
	m_UsageTimeXmlValue.Set(rUsageTimeRef.m_UsageTimeXmlValue);
	return true;
}

void WindowItemUsageTimeProperty::Invalidate()
{
	m_UsageTimeXmlValue.Clean();
}

bool WindowItemUsageTimeProperty::IsValid() const
{
	return (m_UsageTimeXmlValue.IsValid());
}

bool WindowItemUsageTimeProperty::IsMatching(const WindowItemProperty & UNUSED(rRef)) const
{
	return false; // the property is not used for comparison
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// WindowItemRectProperty
/////////////////////////////////////////////////////////////////////////////////////////////////////////
ObjectPool<WindowItemRectProperty> WindowItemPropertyBase<WindowItemRectProperty>::m_PropertyPool(PROPERTY_ALLOCATION_CHUNK);

WindowItemRectProperty::WindowItemRectProperty()
	: WindowItemPropertyBase<WindowItemRectProperty>(PropertyTypeRect),
	m_RectXmlValue(IDS_STRING_VALUE)
{
	AddItem(m_RectXmlValue);
}

WindowItemRectProperty::~WindowItemRectProperty()
{
}

bool WindowItemRectProperty::Initialize(HWND hwnd)
{
	// don't use this property if window is a top or a dialog window
	if((::GetWindowLong(hwnd, GWL_STYLE) & WS_DLGFRAME) != 0)
		return false;
	if(!::IsWindow(::GetParent(hwnd)))
		return false;

	// get relative rect of the window
	WINDOWPLACEMENT WindowPlacement;
	WindowPlacement.length = sizeof(WindowPlacement);
	if(!::GetWindowPlacement(hwnd, &WindowPlacement))
		return false;
	const RECT &Rect = WindowPlacement.rcNormalPosition;

	TCHAR RectString[256];
	_stprintf_s(RectString, sizeof(RectString) / sizeof(RectString[0]), _T("%d:%d:%d:%d"), Rect.left, Rect.top, Rect.right, Rect.bottom);
	m_RectXmlValue.SetString(RectString);
	return true;
}

bool WindowItemRectProperty::Update(const WindowItemProperty &rRef)
{
	if(rRef.GetType() != GetType())
		return false;
	const WindowItemRectProperty &rRectRef = (const WindowItemRectProperty &)rRef;
	m_RectXmlValue.Set(rRectRef.m_RectXmlValue);
	return true;
}

void WindowItemRectProperty::Invalidate()
{
	m_RectXmlValue.Clean();
}

bool WindowItemRectProperty::IsValid() const
{
	return (m_RectXmlValue.IsValid());
}

bool WindowItemRectProperty::IsMatching(const WindowItemProperty &rRef) const
{
	if(rRef.GetType() != GetType())
		return false;
	return m_RectXmlValue.IsCaseInsensitiveEqualeTo(((WindowItemRectProperty &)rRef).m_RectXmlValue);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// WindowItemSubObjectIdProperty
/////////////////////////////////////////////////////////////////////////////////////////////////////////
ObjectPool<WindowItemSubObjectIdProperty> WindowItemPropertyBase<WindowItemSubObjectIdProperty>::m_PropertyPool(PROPERTY_ALLOCATION_CHUNK);

WindowItemSubObjectIdProperty::WindowItemSubObjectIdProperty()
	: WindowItemPropertyBase<WindowItemSubObjectIdProperty>(PropertyTypeSubObjectId),
	m_IdXmlValue(IDS_STRING_VALUE)
{
	AddItem(m_IdXmlValue);
}

WindowItemSubObjectIdProperty::~WindowItemSubObjectIdProperty()
{
}

void WindowItemSubObjectIdProperty::SetSubObjectId(LPCTSTR SubObjectId)
{
	m_IdXmlValue.SetString(SubObjectId);
}

WindowItemProperty *WindowItemSubObjectIdProperty::Create(int XmlNameId, LPCTSTR SubObjectId)
{
	WindowItemProperty *pNew = WindowItemPropertyBase<WindowItemSubObjectIdProperty>::Create(XmlNameId);
	if(!pNew)
		return NULL;
	((WindowItemSubObjectIdProperty *)pNew)->SetSubObjectId(SubObjectId);
	return pNew;
}

bool WindowItemSubObjectIdProperty::Update(const WindowItemProperty &rRef)
{
	if(rRef.GetType() != GetType())
		return false;
	_tstring AssignedValue;
	const WindowItemSubObjectIdProperty &rIdRef = (const WindowItemSubObjectIdProperty &)rRef;
	m_IdXmlValue.SetString(rIdRef.m_IdXmlValue.GetAssignedString(AssignedValue));
	return true;
}

void WindowItemSubObjectIdProperty::Invalidate()
{
	m_IdXmlValue.Clean();
}

bool WindowItemSubObjectIdProperty::IsValid() const
{
	return (m_IdXmlValue.IsValid());
}

bool WindowItemSubObjectIdProperty::IsMatching(const WindowItemProperty &rRef) const
{
	if(rRef.GetType() != GetType())
		return false;
	return m_IdXmlValue.IsEqualeTo(((WindowItemSubObjectIdProperty &)rRef).m_IdXmlValue);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// WindowItemIsEditableProperty
/////////////////////////////////////////////////////////////////////////////////////////////////////////
ObjectPool<WindowItemIsEditableProperty> WindowItemPropertyBase<WindowItemIsEditableProperty>::m_PropertyPool(PROPERTY_ALLOCATION_CHUNK);

WindowItemIsEditableProperty::WindowItemIsEditableProperty()
	: WindowItemReadOnlyProperty<WindowItemIsEditableProperty>(RuleTypeIsEditable),
	m_IsEditableXmlValue(IDS_STRING_VALUE)
{
	AddItem(m_IsEditableXmlValue);
}

WindowItemIsEditableProperty::~WindowItemIsEditableProperty()
{
}

void WindowItemIsEditableProperty::Invalidate()
{
	m_IsEditableXmlValue.Clean();
}

bool WindowItemIsEditableProperty::IsValid() const
{
	return (m_IsEditableXmlValue.IsValid());
}

bool WindowItemIsEditableProperty::IsMatching(const WindowItemProperty &rRef) const
{
	if(rRef.GetType() != GetType())
		return false;
	return m_IsEditableXmlValue.IsEqualeTo(((WindowItemIsEditableProperty &)rRef).m_IsEditableXmlValue);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// WindowItemClipboardEnabledProperty
/////////////////////////////////////////////////////////////////////////////////////////////////////////
ObjectPool<WindowItemClipboardEnabledProperty> WindowItemPropertyBase<WindowItemClipboardEnabledProperty>::m_PropertyPool(PROPERTY_ALLOCATION_CHUNK);

WindowItemClipboardEnabledProperty::WindowItemClipboardEnabledProperty()
	: WindowItemReadOnlyProperty<WindowItemClipboardEnabledProperty>(RuleTypeClipboardEnabled),
	m_ClipboardEnabledXmlValue(IDS_STRING_VALUE)
{
	AddItem(m_ClipboardEnabledXmlValue);
}

WindowItemClipboardEnabledProperty::~WindowItemClipboardEnabledProperty()
{
}

void WindowItemClipboardEnabledProperty::Invalidate()
{
	m_ClipboardEnabledXmlValue.Clean();
}

bool WindowItemClipboardEnabledProperty::IsValid() const
{
	return (m_ClipboardEnabledXmlValue.IsValid());
}

bool WindowItemClipboardEnabledProperty::IsMatching(const WindowItemProperty &rRef) const
{
	if(rRef.GetType() != GetType())
		return false;
	return m_ClipboardEnabledXmlValue.IsEqualeTo(((WindowItemClipboardEnabledProperty &)rRef).m_ClipboardEnabledXmlValue);
}
