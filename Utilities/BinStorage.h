#ifndef _BIN_STORAGE_HEADER_
#define _BIN_STORAGE_HEADER_

class WritableBinStorage
{
public:
	WritableBinStorage();
	virtual ~WritableBinStorage();

	bool Put(const LPBYTE pBuffer, int nBytes);
	bool PutString(LPCTSTR String, int Size);

	bool WriteToFile(LPCTSTR FileName);
private:
	void Close();
private:
	struct BinBuffer
	{
		LPBYTE pBuffer;
		int Size;
		int Fullness;
	};

	std::vector<BinBuffer> m_BufferList;
	BinBuffer *m_pCurrentBuffer;
};

class ReadableBinStorage
{
public:
	ReadableBinStorage();
	virtual ~ReadableBinStorage();

	bool Get(LPBYTE pBuffer, int nBytes);
	bool GetString(_tstring &rString);
	
	bool Peek(LPBYTE pBuffer, int nBytes);
	bool PeekString(_tstring &rString);

	bool Skip(int nBytes);

	static bool CheckFile(LPCTSTR FileName);
	bool ReadFromFile(LPCTSTR FileName);
private:
	void Close();
private:
	LPBYTE m_pBuffer;
	int m_Size;
	int m_NextByteToRead;
};

#endif // _BIN_STORAGE_HEADER_