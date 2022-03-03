#ifndef _COM_HELPER_HEADER_
#define _COM_HELPER_HEADER_

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

#endif // _COM_HELPER_HEADER_