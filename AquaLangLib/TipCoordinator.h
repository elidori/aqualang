#ifndef _TIP_COORDINATOR_HEADER_
#define _TIP_COORDINATOR_HEADER_

class TipCoordinator
{
public:
	TipCoordinator() : m_fLocked(false)	{}
	virtual ~TipCoordinator()			{}

	bool Lock()
	{
		if(m_fLocked)
			return false;
		m_fLocked = true;
		return true;
	}

	void Unlock()						{ m_fLocked = false; }
private:
	bool m_fLocked;
};

class TipCoordinatorClient
{
public:
	TipCoordinatorClient(TipCoordinator *pTipCoordinator) : m_pTipCoordinator(pTipCoordinator), m_fLocked(false) {}
	virtual ~TipCoordinatorClient()	{ Unlock(); }

	bool Lock()	
	{ 
		Unlock();
		if(m_pTipCoordinator == NULL)
			return true;
		m_fLocked = m_pTipCoordinator->Lock(); 
		return m_fLocked; 
	}

	void Unlock() 
	{ 
		if(m_fLocked)
		{
			assert(m_pTipCoordinator != NULL); 
			m_pTipCoordinator->Unlock();
			m_fLocked = false;
		}
	}
private:
	TipCoordinator *m_pTipCoordinator;
	bool m_fLocked;
};

#endif // _TIP_COORDINATOR_HEADER_