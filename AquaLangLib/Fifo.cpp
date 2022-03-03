#include "Pch.h"
#include "Fifo.h"

FifoBase::FifoBase(int ItemSize)
	: m_ItemSize(ItemSize),
	m_pFifo(NULL),
	m_dwFifoItemCount(0),
	m_dwFullness(0),
	m_ReadPosition(0)
{
}

FifoBase::~FifoBase()
{
	Close();
}

bool FifoBase::Set(const FifoBase &r)
{
	if(r.m_dwFifoItemCount > 0)
	{
		if(r.m_dwFifoItemCount * r.m_ItemSize != m_dwFifoItemCount * m_ItemSize)
		{
			Close();
			m_pFifo = new BYTE[r.m_dwFifoItemCount * r.m_ItemSize];
			if(!m_pFifo)
				return false;
		}
		memcpy(m_pFifo, r.m_pFifo, r.m_dwFifoItemCount * r.m_ItemSize);
		m_dwFifoItemCount = r.m_dwFifoItemCount;
		m_ItemSize = r.m_ItemSize;

		m_dwFullness = r.m_dwFullness;
		m_ReadPosition = r.m_ReadPosition;

		m_IndexMarker.Set(r.m_IndexMarker);
	}
	else
	{
		Close();
	}
	return true;
}

bool FifoBase::Open(DWORD FifoItemCount)
{
	Close();
	m_pFifo = new BYTE[FifoItemCount * m_ItemSize];
	if(!m_pFifo)
		return false;
	m_dwFifoItemCount = FifoItemCount;
	return true;
}

void FifoBase::Clear()
{
	m_dwFullness = 0;
	m_ReadPosition = 0;
	m_IndexMarker.Clear();
}

bool FifoBase::PushValue(const LPBYTE pValue, int ValueSize, bool fUpdateMark, bool fRemoveIfFull /*= false*/)
{
	assert(ValueSize == m_ItemSize);
	if(ValueSize != m_ItemSize)
		return false;
	assert(pValue != NULL);
	if(pValue == NULL)
		return false;

	if(m_dwFullness == m_dwFifoItemCount)
	{
		if(!fRemoveIfFull)
		{
			return false;
		}
		else
		{
			// remove oldest
			m_dwFullness--;
			m_ReadPosition++;

			m_IndexMarker.DecreaseOffset();
		}
	}

	SetValue(m_dwFullness, pValue);
	m_dwFullness++;

	m_IndexMarker.AddedLatest(fUpdateMark);

	return true;
}

bool FifoBase::UnPushValue(LPBYTE pValue, int ValueSize)
{
	if(!PeekLatestValue(pValue, ValueSize))
		return false;

	m_dwFullness--;

	m_IndexMarker.RemovedLatest(m_dwFullness);

	return true;
}

bool FifoBase::UnPushValueList(int nValues)
{
	if((int)m_dwFullness < nValues)
		return false;

	m_dwFullness -= nValues;
	m_IndexMarker.RemovedLatest(m_dwFullness);

	return true;
}

bool FifoBase::PopValue(LPBYTE pValue, int ValueSize)
{
	assert(ValueSize == m_ItemSize);
	if(ValueSize != m_ItemSize)
		return false;
	assert(pValue != NULL);
	if(pValue == NULL)
		return false;
	memset(pValue, 0, ValueSize);

	if(m_dwFullness == 0)
		return false;
	GetValue(0, pValue);

	m_dwFullness--;
	m_ReadPosition++;
	m_IndexMarker.DecreaseOffset();

	return true;
}

bool FifoBase::PeekLatestValue(LPBYTE pValue, int ValueSize) const
{
	assert(ValueSize == m_ItemSize);
	if(ValueSize != m_ItemSize)
		return false;
	assert(pValue != NULL);
	if(pValue == NULL)
		return false;
	memset(pValue, 0, ValueSize);

	if(m_dwFullness == 0)
		return false;

	GetValue(m_dwFullness - 1, pValue);
	return true;
}

void FifoBase::MarkCurrentPosition()
{
	m_IndexMarker.Mark(m_dwFullness);
}

int FifoBase::GetMarkOffset() const
{
	return m_IndexMarker.GetOffset();
}

bool FifoBase::IsEmpty(bool fStartAfterMarkedKey) const
{
	return (GetCount(fStartAfterMarkedKey) == 0);
}

int FifoBase::GetCount(bool fStartAfterMarkedKey) const
{
	return ((int)m_dwFullness - GetStartOffset(fStartAfterMarkedKey));
}

bool FifoBase::GetValueInIndex(LPBYTE pValue, int ValueSize, int Index, bool fStartAfterMarkedKey /*= false*/) const
{
	assert(Index < GetCount(fStartAfterMarkedKey));
	assert(m_pFifo != NULL);

	assert(ValueSize == m_ItemSize);
	if(ValueSize != m_ItemSize)
		return false;
	assert(pValue != NULL);
	if(pValue == NULL)
		return false;
	memset(pValue, 0, ValueSize);

	if(Index < 0 || Index >= GetCount(fStartAfterMarkedKey))
		return false;
	GetValue(Index, pValue, fStartAfterMarkedKey);
	return true;
}

bool FifoBase::GetValueList(LPBYTE pValue, int ValueSize, int StartIndex, int nValues, bool fStartAfterMarkedKey /*= false*/) const
{
	if(m_pFifo == NULL)
		return false;
	assert(ValueSize == m_ItemSize);
	if(ValueSize != m_ItemSize)
		return false;
	assert(pValue != NULL);
	if(pValue == NULL)
		return false;

	if(StartIndex + nValues > GetCount(fStartAfterMarkedKey))
		return false;

	int ContinuousItemCount = (m_dwFifoItemCount - GetIndexInFifo(StartIndex, fStartAfterMarkedKey));
	if(ContinuousItemCount >= nValues)
	{
		memcpy(pValue, &m_pFifo[GetIndexInFifo(StartIndex, fStartAfterMarkedKey) * m_ItemSize], nValues * m_ItemSize);
	}
	else
	{
		memcpy(pValue, &m_pFifo[GetIndexInFifo(StartIndex, fStartAfterMarkedKey) * m_ItemSize], ContinuousItemCount * m_ItemSize);
		memcpy(pValue + ContinuousItemCount * m_ItemSize, &m_pFifo[GetIndexInFifo(StartIndex + ContinuousItemCount, fStartAfterMarkedKey) * m_ItemSize], (nValues - ContinuousItemCount) * m_ItemSize);
	}
	return true;
}

void FifoBase::Close()
{
	if(m_pFifo)
	{
		delete[] m_pFifo;
		m_pFifo = NULL;
	}
	m_dwFifoItemCount = 0;
	m_dwFullness = 0;
	m_ReadPosition = 0;
	m_IndexMarker.Clear();
}

int FifoBase::GetIndexInFifo(int UserIndex, bool fStartAfterMarkedKey /*= false*/) const
{
	assert(m_dwFifoItemCount > 0);
	if(m_dwFifoItemCount == 0)
		return 0;

	return ((m_ReadPosition + UserIndex + GetStartOffset(fStartAfterMarkedKey)) % m_dwFifoItemCount);
}

int FifoBase::GetStartOffset(bool fStartAfterMarkedKey) const
{
	if(!fStartAfterMarkedKey)
		return 0;
	return m_IndexMarker.GetOffset();
}

void FifoBase::SetValue(int Position, const LPBYTE pValue)
{
	memcpy(&m_pFifo[GetIndexInFifo(Position) * m_ItemSize], pValue, m_ItemSize);
}

void FifoBase::GetValue(int Position, const LPBYTE pValue, bool fStartAfterMarkedKey /*= false*/) const
{
	memcpy(pValue, &m_pFifo[GetIndexInFifo(Position, fStartAfterMarkedKey) * m_ItemSize], m_ItemSize);
}
