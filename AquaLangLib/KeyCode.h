#ifndef _KEY_CODE_HEADER_
#define _KEY_CODE_HEADER_

class KeyCode
{
public:
	KeyCode(int nVirtualKey, int Operator, bool fCanPrint, bool fSimulated);
	KeyCode(int nKey = -1);
	virtual ~KeyCode();

	void Set(int nKey);
	void Clear();
	int Get() const;

	bool IsValid() const;

	// key properties
	int VirtualKey() const;
	int Operation() const;
	bool IsKeyPressed() const;
	bool IsClearAllKey() const;
	bool IsPrintable() const;
	bool IsShiftDown() const;
	bool IsShiftKey() const;
	
	static bool IsSimulated(int Key);
	static bool IsKeyDownAndUp(int VirtualKey, int Key1, int Key2);

	// key operators
	bool IsMatching(int nRefKey) const;
private:
	int m_nKey;
};

#endif // _KEY_CODE_HEADER_