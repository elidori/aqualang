#ifndef _OBJECT_POOL_HEADER_
#define _OBJECT_POOL_HEADER_

#include "SyncHelpers.h"

template <class T>
class ObjectWrapper;

template <class T>
class IObjectOwner
{
public:
	virtual ~IObjectOwner() {}
	
	virtual void Free(ObjectWrapper<T> *pObject) = 0;
};

template <class T>
class ObjectPool : public IObjectOwner<T>
{
public:
	ObjectPool(int NumberOfObjects);
	virtual ~ObjectPool();

	bool Open();
	virtual ObjectWrapper<T> *Get();
	void Close();
protected:
	// IObjectOwner pure virtuals {
	virtual void Free(ObjectWrapper<T> *pObject);
	// }
private:
	bool Add(int NumberOfObjects);
private:
	const int m_NumberOfAllocatedObjects;

	std::vector< ObjectWrapper<T> * > m_AllocatedWrapperList;
	std::vector< T * > m_AllocatedList;

	ObjectWrapper<T> *m_pFreeObjectsHeader;
};

template <class T>
class LockedObjectPool : public ObjectPool<T>
{
public:
	LockedObjectPool(int NumberOfObjects);

	virtual ObjectWrapper<T> *Get();
private:
	// IObjectOwner pure virtuals {
	virtual void Free(ObjectWrapper<T> *pObject);
	// }
private:
	CriticalSec m_ListLocker;
};

template <class T>
class ObjectWrapper
{
public:
	ObjectWrapper() : m_pOwner(NULL), m_pNext(NULL), m_pInfo(NULL) {}
	virtual ~ObjectWrapper() {}

	T &Info()	{ return *m_pInfo; }
	void Release();
private:
	friend class ObjectPool<T>;

	void Open(T &rInfo, IObjectOwner<T> &rOwner)	{ m_pInfo = &rInfo; m_pOwner = &rOwner; }
	
	ObjectWrapper<T> *GetNext()				{ return m_pNext; }
	void SetNext(ObjectWrapper<T> *pObject)	{ m_pNext = pObject; }
private:
	IObjectOwner<T> *m_pOwner;
	ObjectWrapper<T> *m_pNext;

	T *m_pInfo;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ObjectWrapper
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template <class T>
inline void ObjectWrapper<T>::Release()
{
	assert(m_pOwner != NULL);
	if(m_pOwner)
	{
		m_pOwner->Free(this);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ObjectPool
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template <class T>
inline ObjectPool<T>::ObjectPool(int NumberOfObjects)
	: m_NumberOfAllocatedObjects(NumberOfObjects),
	m_AllocatedList(),
	m_AllocatedWrapperList(),
	m_pFreeObjectsHeader(NULL)
{
}

template <class T>
inline ObjectPool<T>::~ObjectPool()
{
	Close();
}

template <class T>
inline bool ObjectPool<T>::Open()
{
	Close();
	return Add(m_NumberOfAllocatedObjects);
}

template <class T>
inline ObjectWrapper<T> *ObjectPool<T>::Get()
{
	if(m_pFreeObjectsHeader == NULL)
	{
		if(!Add(m_NumberOfAllocatedObjects))
			return NULL;
	}
	ObjectWrapper<T> *pFetchedObject = m_pFreeObjectsHeader;
	m_pFreeObjectsHeader = m_pFreeObjectsHeader->GetNext();
	return pFetchedObject;
}

template <class T>
inline void ObjectPool<T>::Close()
{
	unsigned int i;

	for(i = 0; i < m_AllocatedList.size(); i++)
	{
		delete[] m_AllocatedList.at(i);
	}
	m_AllocatedList.erase(m_AllocatedList.begin(), m_AllocatedList.end());

	for(i = 0; i < m_AllocatedWrapperList.size(); i++)
	{
		delete[] m_AllocatedWrapperList.at(i);
	}
	m_AllocatedWrapperList.erase(m_AllocatedWrapperList.begin(), m_AllocatedWrapperList.end());

	m_pFreeObjectsHeader = NULL;
}

template <class T>
inline void ObjectPool<T>::Free(ObjectWrapper<T> *pObject)
{
	pObject->SetNext(m_pFreeObjectsHeader);
	m_pFreeObjectsHeader = pObject;
}

template <class T>
inline bool ObjectPool<T>::Add(int NumberOfObjects)
{
	if(NumberOfObjects == 0)
		return false;

	T *pList = new T[NumberOfObjects];
	if(pList == NULL)
		return false;
	m_AllocatedList.push_back(pList);

	ObjectWrapper<T> *pWrapperList = new ObjectWrapper<T>[NumberOfObjects];
	if(pWrapperList == NULL)
		return false;
	m_AllocatedWrapperList.push_back(pWrapperList);

	for(int i = 0; i < NumberOfObjects; i++)
	{
		ObjectWrapper<T> *pObject = &pWrapperList[i];
		pObject->Open(pList[i], *this);
		if(i < NumberOfObjects - 1)
		{
			pObject->SetNext(&pWrapperList[i + 1]);
		}
	}

	pWrapperList[NumberOfObjects - 1].SetNext(m_pFreeObjectsHeader);
	m_pFreeObjectsHeader = &pWrapperList[0];

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// LockedObjectPool
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template <class T>
inline LockedObjectPool<T>::LockedObjectPool(int NumberOfObjects)
	: ObjectPool<T>(NumberOfObjects)
{
}

template <class T>
inline ObjectWrapper<T> *LockedObjectPool<T>::Get()
{
	LockScope Lock(m_ListLocker);

	return ObjectPool<T>::Get();
}

template <class T>
inline void LockedObjectPool<T>::Free(ObjectWrapper<T> *pObject)
{
	LockScope Lock(m_ListLocker);

	ObjectPool<T>::Free(pObject);
}

#endif // _OBJECT_POOL_HEADER_
