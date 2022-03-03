#ifndef _WIN_HOOK_LISTENER_MAP_HEADER_
#define _WIN_HOOK_LISTENER_MAP_HEADER_

namespace WinHook
{
	typedef int Result;

	template<class T_Key, class T>
	class ListenerMap
	{
	public:
		ListenerMap()			{}
		virtual ~ListenerMap()	{ Close(); }

		size_t GetCount() const	{ return m_LisenerListMap.size(); }

		Result Add(T_Key Key, T &rListener);
		Result Remove(const T &rListener);

		Result GetList(T_Key Key, ListenerList<T>* &rpListener);
		Result CopyList(T_Key Key, ListenerList<T> &rTargetListener);

		void Close();
	private:
		typedef std::map<T_Key, ListenerList<T> *> LisenerListMap;
		LisenerListMap m_LisenerListMap;
	};

	template<class T_Key, class T>
	Result ListenerMap<T_Key, T>::Add(T_Key Key, T &rListener)
	{
		ListenerList<T> *pListenerList = NULL;
		LisenerListMap::iterator Iterator = m_LisenerListMap.find(Key);
		if(Iterator == m_LisenerListMap.end())
		{
			pListenerList = new ListenerList<T>;
			if(pListenerList == NULL)
				return Result_MemAllocFailed;
			m_LisenerListMap.insert(LisenerListMap::value_type(Key, pListenerList));
		}
		else
		{
			pListenerList = (*Iterator).second;
		}

		assert(pListenerList != NULL);
		Result res = pListenerList->Add(rListener);
		if(res != Result_Success)
		{
			if(pListenerList->GetCount() == 0)
			{
				Iterator = m_LisenerListMap.find(Key);
				assert(Iterator != m_LisenerListMap.end());
				m_LisenerListMap.erase(Iterator);
				delete pListenerList;
			}
			return res;
		}
		return Result_Success;
	}

	template<class T_Key, class T>
	Result ListenerMap<T_Key, T>::Remove(const T &rListener)
	{
		bool fRepeat = true;

		while(fRepeat)
		{
			fRepeat = false;
			if(m_LisenerListMap.size() > 0)
			{
				LisenerListMap::iterator Iterator = m_LisenerListMap.begin();
				for(; Iterator != m_LisenerListMap.end(); Iterator++)
				{
					ListenerList<T> *pListenerList = (*Iterator).second;
					assert(pListenerList);

					pListenerList->Remove(rListener);
					if(pListenerList->GetCount() == 0)
					{
						m_LisenerListMap.erase(Iterator);
						delete pListenerList;
						fRepeat = true;
						break;
					}
				}
			}
		}
		return Result_Success;
	}

	template<class T_Key, class T>
	Result ListenerMap<T_Key, T>::GetList(T_Key Key, ListenerList<T>* &rpListener)
	{
		rpListener = NULL;
		LisenerListMap::iterator Iterator = m_LisenerListMap.find(Key);
		if(Iterator == m_LisenerListMap.end())
			return Result_InvalidParameter;
		rpListener = (*Iterator).second;
		return Result_Success;
	}

	template<class T_Key, class T>
	Result ListenerMap<T_Key, T>::CopyList(T_Key Key, ListenerList<T> &rTargetListener)
	{
		ListenerList<T> *pListener = NULL;
		Result res = GetList(Key, pListener);
		if(res != Result_Success)
			return res;
		assert(pListener != NULL);
		rTargetListener.AddList(*pListener);
		return Result_Success;
	}

	template<class T_Key, class T>
	void ListenerMap<T_Key, T>::Close()
	{
		LisenerListMap::iterator Iterator = m_LisenerListMap.begin();
		for(; Iterator != m_LisenerListMap.end(); Iterator++)
		{
			delete (*Iterator).second;
		}
		m_LisenerListMap.erase(m_LisenerListMap.begin(), m_LisenerListMap.end());
	}

} // namespace WinHook

#endif // _WIN_HOOK_LISTENER_MAP_HEADER_