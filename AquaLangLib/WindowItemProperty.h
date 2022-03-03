#ifndef _WINDOW_ITEM_PROPERTY_HEADER_
#define _WINDOW_ITEM_PROPERTY_HEADER_

#include "xmlStructure.h"
#include "xmlValueTypes.h"
#include "WindowItemPropertyTypes.h"
#include "ObjectPool.h"

enum WindowItemPropertyType
{
	// window property types
	PropertyTypeHWND = 0,
	PropertyTypeCaption = 1,
	PropertyTypeIndexInParent = 2,
	PropertyTypeProcessName = 3,
	PropertyTypeUsageTime = 7,
	PropertyTypeRect = 4,

	// sub object id type
	PropertyTypeSubObjectId = 5,

	// rules
	RuleTypeIsEditable = 6,
	RuleTypeClipboardEnabled = 8,

	MaxNumberOfProperties = 9,

	MaxNumberOfPropertyItems = 3, // In the future this may need to increase
};

typedef int WindowItemPropertyType_t;

class WindowItemProperty : public xmlFixedSizeStructure<xmlItem, MaxNumberOfPropertyItems>
{
protected:
	WindowItemProperty(int Type);
	virtual ~WindowItemProperty()	{}
public:
	virtual xmlItem &GetXmlItem()	{ return *this; }

	static WindowItemProperty *CreateFromXml(
									XmlNode &rxItemNode,
									int NameId,
									int &rnKey
									);
	static WindowItemProperty *CreateFromBinStorage(
									ReadableBinStorage &rBinStorage,
									int NameId,
									int &rnKey
									);

	// this one returns true only if the typeindex is valid
	static bool AddPropertyFromHWND(
						HWND hwnd,
						int TypeIndex,
						int NameId,
						int &rnKey,
						WindowItemProperty* &rpNew
						);
	static bool AddPropertyFromId(
						LPCTSTR SubObjectId,
						int NameId,
						int &rnKey,
						WindowItemProperty* &rpNew
						);

	int GetType() const				{ return m_Type; }
	LPCTSTR GetTypeString(_tstring &rString) const;

	virtual void Release() = 0;

	virtual bool Initialize(HWND hwnd) = 0;

	virtual WindowItemProperty *Duplicate() const = 0;
	virtual bool Update(const WindowItemProperty &rRef) = 0;
	virtual void Invalidate() = 0;

	virtual bool IsMatching(const WindowItemProperty &rRef) const = 0;
	virtual bool IsMatchingMust() const = 0;
	
	virtual const xmlValue &GetValueNode() const = 0;
	void GetDebugString(_tstring &rString) const;
private:
	const int m_Type;

	static const WindowItemPropertyTypes m_gPropertyTypes;
};

template <class T>
class WindowItemPropertyBase : public WindowItemProperty
{
protected:
	friend class ObjectPool<T>;

	WindowItemPropertyBase(int Type) : WindowItemProperty(Type) {}
public:
	static WindowItemProperty *Create(int XmlNameId);

	virtual WindowItemProperty *Duplicate() const;
	virtual void Release();
private:
	static ObjectPool<T> m_PropertyPool;
	ObjectWrapper<T> *m_pWrapper;
};

template <class T>
class WindowItemReadOnlyProperty : public WindowItemPropertyBase<T>
{
public:
	WindowItemReadOnlyProperty(int Type)
		: WindowItemPropertyBase<T>(Type)					{}
	virtual ~WindowItemReadOnlyProperty()					{}

	virtual bool Initialize(HWND UNUSED(hwnd))				{ assert(0); return false; }
	virtual WindowItemProperty *Duplicate() const			{ assert(0); return NULL; }
	virtual bool Update(const WindowItemProperty & UNUSED(rRef)) { assert(0); return false; }
	virtual bool IsMatchingMust() const						{ return false; }
};

template <class T>
inline WindowItemProperty *WindowItemPropertyBase<T>::Create(int XmlNameId)
{
	ObjectWrapper<T> *pNewWrapper = m_PropertyPool.Get();
	if(pNewWrapper == NULL)
		return NULL;
	WindowItemPropertyBase<T> &rNew = pNewWrapper->Info();
	rNew.m_pWrapper = pNewWrapper;
	rNew.SetNameId(XmlNameId);

	return &rNew;
}

template <class T>
inline WindowItemProperty *WindowItemPropertyBase<T>::Duplicate() const
{
	ObjectWrapper<T> *pNewWrapper = m_PropertyPool.Get();
	if(pNewWrapper == NULL)
		return NULL;
	WindowItemPropertyBase<T> &rNew = pNewWrapper->Info();
	rNew.m_pWrapper = pNewWrapper;
	rNew.SetNameId(GetXmlNameId());

	rNew.Update(*this);

	return &rNew;
}

template <class T>
inline void WindowItemPropertyBase<T>::Release()
{
	if(m_pWrapper != NULL)
	{
		Invalidate();
		m_pWrapper->Release();
	}
	else
	{
		delete this;
	}
}


class WindowItemHWNDProperty : public WindowItemPropertyBase<WindowItemHWNDProperty>
{
private:
	friend class ObjectPool<WindowItemHWNDProperty>;
	
	WindowItemHWNDProperty();
	virtual ~WindowItemHWNDProperty();
public:
	virtual bool Initialize(HWND hwnd);
	virtual bool Update(const WindowItemProperty &rRef);
	virtual void Invalidate();
	virtual bool IsValid() const;
	virtual bool IsMatching(const WindowItemProperty &rRef) const;
	virtual bool IsMatchingMust() const		{ return false; }
	virtual const xmlValue &GetValueNode() const	{ return m_hwndXmlValue; }
private:
	xmlInt m_hwndXmlValue;
	xmlTime m_CreationTimeXmlValue;
};

class WindowItemCaptionProperty : public WindowItemPropertyBase<WindowItemCaptionProperty>
{
private:
	friend class ObjectPool<WindowItemCaptionProperty>;

	WindowItemCaptionProperty();
	virtual ~WindowItemCaptionProperty();
public:
	virtual bool Initialize(HWND hwnd);
	virtual bool Update(const WindowItemProperty &rRef);
	virtual void Invalidate();
	virtual bool IsValid() const;
	virtual bool IsMatching(const WindowItemProperty &rRef) const;
	virtual bool IsMatchingMust() const;
	virtual const xmlValue &GetValueNode() const	{ return m_CaptionXmlValue; }

	static bool GetWindowCaption(HWND hwnd, _tstring &Text);
private:
	xmlString m_CaptionXmlValue;
	xmlBool m_fIsMatchingMustXmlValue;
};

class WindowItemIndexInParentProperty : public WindowItemPropertyBase<WindowItemIndexInParentProperty>
{
private:
	friend class ObjectPool<WindowItemIndexInParentProperty>;

	WindowItemIndexInParentProperty();
	virtual ~WindowItemIndexInParentProperty();
public:
	virtual bool Initialize(HWND hwnd);
	virtual bool Update(const WindowItemProperty &rRef);
	virtual void Invalidate();
	virtual bool IsValid() const;
	virtual bool IsMatching(const WindowItemProperty &rRef) const;
	virtual bool IsMatchingMust() const		{ return false; }
	virtual const xmlValue &GetValueNode() const	{ return m_nIndexXmlValue; }
private:
	xmlInt m_nIndexXmlValue;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ChildIndexFinder:
// WindowItemIndexInParentProperty helper
////////////////////////////////////////////////////////////////////////////////////////////////////////////
class ChildIndexFinder
{
public:
	ChildIndexFinder() {}
	virtual ~ChildIndexFinder() {}
	
	int GetIndexInParent(HWND hwndChild, HWND hwndParent);
private:
	static BOOL CALLBACK EnumChildProc(HWND hwndChild, LPARAM lParam);
	void CheckChildWindow(HWND hwndChild);
private:
	HWND m_hwnd;
	int m_nNextChildIndex;
	int m_nChildIndex;
};

class WindowItemProcessNameProperty : public WindowItemPropertyBase<WindowItemProcessNameProperty>
{
private:
	friend class ObjectPool<WindowItemProcessNameProperty>;

	WindowItemProcessNameProperty();
	virtual ~WindowItemProcessNameProperty();

public:
	virtual bool Initialize(HWND hwnd);
	virtual bool Update(const WindowItemProperty &rRef);
	virtual void Invalidate();
	virtual bool IsValid() const;
	virtual bool IsMatching(const WindowItemProperty &rRef) const;
	virtual bool IsMatchingMust() const		{ return true; }
	virtual const xmlValue &GetValueNode() const	{ return m_ProcessNameXmlValue; }
private:
	xmlString m_ProcessNameXmlValue;
};

class WindowItemUsageTimeProperty : public WindowItemPropertyBase<WindowItemUsageTimeProperty>
{
private:
	friend class ObjectPool<WindowItemUsageTimeProperty>;

	WindowItemUsageTimeProperty();
	virtual ~WindowItemUsageTimeProperty();
public:
	bool CheckTimeTooOld(const FileTime &rCurrentTime) const;

	virtual bool Initialize(HWND hwnd);
	virtual bool Update(const WindowItemProperty &rRef);
	virtual void Invalidate();
	virtual bool IsValid() const;
	virtual bool IsMatching(const WindowItemProperty &rRef) const;
	virtual bool IsMatchingMust() const		{ return false; }
	virtual const xmlValue &GetValueNode() const	{ return m_UsageTimeXmlValue; }
private:
	xmlTime m_UsageTimeXmlValue;
};

class WindowItemRectProperty : public WindowItemPropertyBase<WindowItemRectProperty>
{
private:
	friend class ObjectPool<WindowItemRectProperty>;

	WindowItemRectProperty();
	virtual ~WindowItemRectProperty();
public:
	virtual bool Initialize(HWND hwnd);
	virtual bool Update(const WindowItemProperty &rRef);
	virtual void Invalidate();
	virtual bool IsValid() const;
	virtual bool IsMatching(const WindowItemProperty &rRef) const;
	virtual bool IsMatchingMust() const		{ return false; }
	virtual const xmlValue &GetValueNode() const	{ return m_RectXmlValue; }
private:
	xmlString m_RectXmlValue;
};

class WindowItemSubObjectIdProperty : public WindowItemPropertyBase<WindowItemSubObjectIdProperty>
{
private:
	friend class ObjectPool<WindowItemSubObjectIdProperty>;

	WindowItemSubObjectIdProperty();
	virtual ~WindowItemSubObjectIdProperty();

	void SetSubObjectId(LPCTSTR SubObjectId);
public:
	static WindowItemProperty *Create(int XmlNameId, LPCTSTR SubObjectId);

	virtual bool Initialize(HWND UNUSED(hwnd))	{ assert(0); return true; } // not relevant here
	virtual bool Update(const WindowItemProperty &rRef);
	virtual void Invalidate();
	virtual bool IsValid() const;
	virtual bool IsMatching(const WindowItemProperty &rRef) const;
	virtual bool IsMatchingMust() const		{ return true; }
	virtual const xmlValue &GetValueNode() const	{ return m_IdXmlValue; }
private:
	xmlString m_IdXmlValue;
};

class WindowItemIsEditableProperty : public WindowItemReadOnlyProperty<WindowItemIsEditableProperty>
{
private:
	friend class ObjectPool<WindowItemIsEditableProperty>;

	WindowItemIsEditableProperty();
	virtual ~WindowItemIsEditableProperty();
public:
	virtual void Invalidate();
	virtual bool IsValid() const;
	virtual bool IsMatching(const WindowItemProperty &rRef) const;
	virtual const xmlValue &GetValueNode() const	{ return m_IsEditableXmlValue; }
private:
	xmlBool m_IsEditableXmlValue;
};

class WindowItemClipboardEnabledProperty : public WindowItemReadOnlyProperty<WindowItemClipboardEnabledProperty>
{
private:
	friend class ObjectPool<WindowItemClipboardEnabledProperty>;

	WindowItemClipboardEnabledProperty();
	virtual ~WindowItemClipboardEnabledProperty();
public:
	virtual void Invalidate();
	virtual bool IsValid() const;
	virtual bool IsMatching(const WindowItemProperty &rRef) const;
	virtual const xmlValue &GetValueNode() const	{ return m_ClipboardEnabledXmlValue; }
private:
	xmlBool m_ClipboardEnabledXmlValue;
};

#endif // _WINDOW_ITEM_PROPERTY_HEADER_