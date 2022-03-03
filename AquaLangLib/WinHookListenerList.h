#ifndef _WIN_HOOK_LISTENER_LIST_HEADER_
#define _WIN_HOOK_LISTENER_LIST_HEADER_

#include "WinHookResult.h"

namespace WinHook
{
	template<class T>
	class ListenerList
	{
	public:
		ListenerList(const ListenerList<T> &rRefList);
		ListenerList();
		virtual ~ListenerList() {}

		size_t GetCount() const	{ return m_ListMap.size(); }

		Result Add(T &rListener);
		Result Remove(const T &rListener);

		Result Set(const ListenerList<T> &rRefList);
		Result AddList(const ListenerList<T> &rRefList);

		Result First(T* &rpListener);
		Result Next(T* &rpListener);
	private:
		typedef std::map<int, T *> ListMap;
		ListMap m_ListMap;

		typename ListMap::iterator m_Next;
	};

	template<class T>
	inline ListenerList<T>::ListenerList(const ListenerList<T> &rRefList)
	{
		Set(rRefList);
	}

	template<class T>
	inline ListenerList<T>::ListenerList()
	{
		m_Next = m_ListMap.begin();
	}

	template<class T>
	inline Result ListenerList<T>::Add(T &rListener)
	{
		int Key = (int)(&rListener);
		if(m_ListMap.find(Key) != m_ListMap.end())
			return Result_InvalidParameter; // already in the list
		m_ListMap.insert(ListMap::value_type(Key, &rListener));

		m_Next = m_ListMap.begin(); // reset the iterator as it is no longer relevant

		return Result_Success;
	}

	template<class T>
	inline Result ListenerList<T>::Remove(const T &rListener)
	{
		int Key = (int)(&rListener);
		ListMap::iterator Iterator = m_ListMap.find(Key);
		if(Iterator == m_ListMap.end())
			return Result_InvalidParameter;
		m_ListMap.erase(Iterator);

		m_Next = m_ListMap.begin(); // reset the iterator as it is no longer relevant

		return Result_Success;
	}

	template<class T>
	inline Result ListenerList<T>::Set(const ListenerList<T> &rRefList)
	{
		m_ListMap.erase(m_ListMap.begin(), m_ListMap.end());
		m_Next = m_ListMap.begin();

		return AddList(rRefList);
	}

	template<class T>
	inline Result ListenerList<T>::AddList(const ListenerList<T> &rRefList)
	{
		ListMap::const_iterator Iterator = rRefList.m_ListMap.begin();
		for(; Iterator != rRefList.m_ListMap.end(); Iterator++)
		{
			Add(*(*Iterator).second);
		}

		m_Next = m_ListMap.begin();

		return Result_Success;
	}

	template<class T>
	inline Result ListenerList<T>::First(T* &rpListener)
	{
		m_Next = m_ListMap.begin();
		return Next(rpListener);
	}
	
	template<class T>
	inline Result ListenerList<T>::Next(T* &rpListener)
	{
		rpListener = NULL;
		if(m_Next == m_ListMap.end())
			return Result_False;
		rpListener = (*m_Next).second;
		m_Next++;
		return Result_Success;
	}

} // namespace WinHook
#endif // _WIN_HOOK_LISTENER_LIST_HEADER_