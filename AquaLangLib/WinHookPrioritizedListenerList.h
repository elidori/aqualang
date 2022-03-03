#ifndef _WIN_HOOK_PRIORITIZED_LISTENER_LIST_HEADER_
#define _WIN_HOOK_PRIORITIZED_LISTENER_LIST_HEADER_

#include "WinHookResult.h"

namespace WinHook
{
	template<class T>
	class PrioritizedListenerList
	{
	public:
		PrioritizedListenerList(const PrioritizedListenerList<T> &rRefList);
		PrioritizedListenerList();
		virtual ~PrioritizedListenerList() {}

		size_t GetCount() const	{ return m_List.size(); }
		Result Get(index_t nIndex, T* &rpListener);

		Result Add(T &rListener);
		Result AddLast(T &rListener);
		Result Remove(const T &rListener);

		Result Set(const PrioritizedListenerList<T> &rRefList);
	private:
		int GetIndex(const T &rListener) const;
	private:
		std::vector<T *> m_List;
		
		int m_nHighPriorityCount;
	};

	template<class T>
	inline PrioritizedListenerList<T>::PrioritizedListenerList(const PrioritizedListenerList<T> &rRefList)
	{
		Set(rRefList);
	}

	template<class T>
	inline PrioritizedListenerList<T>::PrioritizedListenerList()
		: m_nHighPriorityCount(0)
	{
	}

	template<class T>
	inline Result PrioritizedListenerList<T>::Get(index_t nIndex, T* &rpListener)
	{
		rpListener = NULL;
		if(nIndex < 0 || nIndex >= (int)m_List.size())
			return Result_InvalidParameter;
		rpListener = m_List.at(nIndex);
		return Result_Success;
	}

	template<class T>
	inline Result PrioritizedListenerList<T>::Add(T &rListener)
	{
		int nIndex = GetIndex(rListener);
		if(nIndex != -1)
			return Result_InvalidParameter; // already in the list
		m_List.insert(m_List.begin() + m_nHighPriorityCount, &rListener);
		m_nHighPriorityCount++;
		return Result_Success;
	}

	template<class T>
	inline Result PrioritizedListenerList<T>::AddLast(T &rListener)
	{
		int nIndex = GetIndex(rListener);
		if(nIndex != -1)
			return Result_InvalidParameter; // already in the list
		m_List.push_back(&rListener);
		return Result_Success;
	}

	template<class T>
	inline Result PrioritizedListenerList<T>::Remove(const T &rListener)
	{
		int nIndex = GetIndex(rListener);
		if(nIndex == -1)
			return Result_InvalidParameter; // not in the list

		if(nIndex < m_nHighPriorityCount)
		{
			m_nHighPriorityCount--;
		}
		m_List.erase(m_List.begin() + nIndex);
		return Result_Success;
	}

	template<class T>
	inline Result PrioritizedListenerList<T>::Set(const PrioritizedListenerList<T> &rRefList)
	{
		m_List.erase(m_List.begin(), m_List.end());

		m_nHighPriorityCount = rRefList.m_nHighPriorityCount;

		for(unsigned int i = 0; i < rRefList.m_List.size(); i++)
		{
			m_List.push_back(rRefList.m_List.at(i));
		}
		return Result_Success;
	}

	template<class T>
	inline int PrioritizedListenerList<T>::GetIndex(const T &rListener) const
	{
		for(unsigned int i = 0; i < m_List.size(); i++)
		{
			if(&rListener == m_List.at(i))
				return i;
		}
		return -1;
	}

} // namespace WinHook
#endif // _WIN_HOOK_PRIORITIZED_LISTENER_LIST_HEADER_