#ifndef _FIFO_HEADER_
#define _FIFO_HEADER_

#include "IndexMarker.h"

class FifoBase
{
public:
	FifoBase(int ItemSize);
	virtual ~FifoBase();

	bool Set(const FifoBase &r);

	bool Open(DWORD FifoItemCount);
	
	int GetMaxCount() const	{ return m_dwFifoItemCount; }

	void Clear();
	
	bool PushValue(const LPBYTE pValue, int ValueSize, bool fUpdateMark, bool fRemoveIfFull = false);
	bool UnPushValue(LPBYTE pValue, int ValueSize);
	bool UnPushValueList(int nValues);
	bool PopValue(LPBYTE pValue, int ValueSize);
	bool PeekLatestValue(LPBYTE pValue, int ValueSize) const;

	void MarkCurrentPosition();
	int GetMarkOffset() const;

	bool IsEmpty(bool fStartAfterMarkedKey = false) const;
	int GetCount(bool fStartAfterMarkedKey = false) const;
	bool GetValueInIndex(LPBYTE pValue, int ValueSize, int Index, bool fStartAfterMarkedKey = false) const;
	bool GetValueList(LPBYTE pValue, int ValueSize, int StartIndex, int nValues, bool fStartAfterMarkedKey = false) const;

	void Close();
private:
	int GetIndexInFifo(int UserIndex, bool fStartAfterMarkedKey = false) const;
	int GetStartOffset(bool fStartAfterMarkedKey) const;
	void SetValue(int Position, const LPBYTE pValue);
	void GetValue(int Position, LPBYTE pValue, bool fStartAfterMarkedKey = false) const;
private:
	int m_ItemSize;

	LPBYTE m_pFifo;
	DWORD m_dwFifoItemCount;
	DWORD m_dwFullness;
	int m_ReadPosition;

	IndexMarker m_IndexMarker;
};

template <class T>
class Fifo : public FifoBase
{
public:
	Fifo() : FifoBase(sizeof(T)) {}

	bool Push(T Value, bool fUpdateMark, bool fRemoveIfFull = false)
	{
		return PushValue((LPBYTE)&Value, sizeof(Value), fUpdateMark, fRemoveIfFull);
	}
	bool UnPush(T &Value)
	{
		return UnPushValue((LPBYTE)&Value, sizeof(Value));
	}
	bool Pop(T &Value)
	{
		return PopValue((LPBYTE)&Value, sizeof(Value));
	}
	bool PeekLatest(T &Value) const
	{
		return PeekLatestValue((LPBYTE)&Value, sizeof(Value));
	}
	T Get(int Index, bool fStartAfterMarkedKey = false) const
	{
		T Value;
		GetValueInIndex((LPBYTE)&Value, sizeof(Value), Index, fStartAfterMarkedKey);
		return Value;
	}
	bool GetList(T *pList, int Count, int From, bool fStartAfterMarkedKey = false) const
	{
		return GetValueList((LPBYTE)pList, sizeof(T), From, Count, fStartAfterMarkedKey);
	}
};

#endif // _FIFO_HEADER_