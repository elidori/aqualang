#include "Pch.h"
#include "BinStorage.h"

#define STORAGE_CHUNK_SIZE (1024 * 1024)

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// WritableBinStorage
///////////////////////////////////////////////////////////////////////////////////////////////////////////
WritableBinStorage::WritableBinStorage()
	: m_pCurrentBuffer(NULL)
{
}

WritableBinStorage::~WritableBinStorage()
{
	Close();
}

bool WritableBinStorage::Put(const LPBYTE pBuffer, int nBytes)
{
	if(m_pCurrentBuffer == NULL || m_pCurrentBuffer->Size - m_pCurrentBuffer->Fullness < nBytes)
	{
		BinBuffer NewBuffer;
		NewBuffer.Size = max(nBytes, STORAGE_CHUNK_SIZE);
		NewBuffer.pBuffer = new BYTE[NewBuffer.Size];
		NewBuffer.Fullness = 0;
		if(NewBuffer.pBuffer == NULL)
			return false;

		m_BufferList.push_back(NewBuffer);
		m_pCurrentBuffer = &m_BufferList.at(m_BufferList.size() - 1);
	}
	memcpy(m_pCurrentBuffer->pBuffer + m_pCurrentBuffer->Fullness, pBuffer, nBytes);
	m_pCurrentBuffer->Fullness += nBytes;
	return true;
}

bool WritableBinStorage::PutString(LPCTSTR String, int Size)
{
	if(!Put((LPBYTE)&Size, sizeof(Size)))
		return false;
	if(!Put((const LPBYTE)String, Size * sizeof(TCHAR)))
		return false;
	return true;
}

bool WritableBinStorage::WriteToFile(LPCTSTR FileName)
{
	FILE *pWriteFile = NULL;
	_tfopen_s(&pWriteFile, FileName, _T("wb"));
	if(pWriteFile == NULL)
		return false;

	bool fSuccess = true;

	BYTE Header[] = { 0x01, 0x02, 0x03, 0x04 };
	if(fwrite(Header, 1, sizeof(Header), pWriteFile) != sizeof(Header))
	{
		fSuccess = false;
	}

	for(unsigned int i = 0; i < m_BufferList.size() && fSuccess; i++)
	{
		if(fwrite(m_BufferList.at(i).pBuffer, 1, m_BufferList.at(i).Fullness, pWriteFile) != (size_t)m_BufferList.at(i).Fullness)
		{
			fSuccess = false;
		}
	}

	fclose(pWriteFile);
	return fSuccess;
}

void WritableBinStorage::Close()
{
	for(unsigned int i = 0; i < m_BufferList.size(); i++)
	{
		delete[] m_BufferList.at(i).pBuffer;
	}
	m_BufferList.erase(m_BufferList.begin(), m_BufferList.end());
	m_pCurrentBuffer = NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// ReadableBinStorage
///////////////////////////////////////////////////////////////////////////////////////////////////////////
ReadableBinStorage::ReadableBinStorage()
	: m_pBuffer(NULL),
	m_Size(0),
	m_NextByteToRead(0)
{
}

ReadableBinStorage::~ReadableBinStorage()
{
	Close();
}

bool ReadableBinStorage::Get(LPBYTE pBuffer, int nBytes)
{
	if(!Peek(pBuffer, nBytes))
		return false;
	m_NextByteToRead += nBytes;
	return true;
}

bool ReadableBinStorage::GetString(_tstring &rString)
{
	int Length;
	if(!Get((LPBYTE)&Length, sizeof(Length)))
		return false;
	if(Length <= 4 * 1024)
	{
		rString.reserve(Length + 1);
		rString.resize(Length);
		if(Length > 0)
		{
			if(!Get((LPBYTE)&rString[0], Length * sizeof(TCHAR)))
				return false;
		}
	}
	else
	{
		if(!Skip(Length * sizeof(TCHAR)))
			return false;
	}
	return true;
}

bool ReadableBinStorage::Peek(LPBYTE pBuffer, int nBytes)
{
	if(m_NextByteToRead + nBytes > m_Size)
		return false;
	memcpy(pBuffer, m_pBuffer + m_NextByteToRead, nBytes);
	return true;
}

bool ReadableBinStorage::PeekString(_tstring &rString)
{
	int Pos = m_NextByteToRead;

	if(!GetString(rString))
		return false;

	m_NextByteToRead = Pos;
	return true;
}

bool ReadableBinStorage::Skip(int nBytes)
{
	if(m_NextByteToRead + nBytes > m_Size)
	{
		m_NextByteToRead= m_Size;
		return false;
	}
	m_NextByteToRead += nBytes;
	return true;
}

bool ReadableBinStorage::CheckFile(LPCTSTR FileName)
{
	FILE *pReadFile = NULL;
	_tfopen_s(&pReadFile, FileName, _T("rb"));
	if(pReadFile == NULL)
		return false;

	BYTE Header[] = { 0x01, 0x02, 0x03, 0x04 };
	BYTE FileHeader[4];
	if(fread(FileHeader, 1, sizeof(FileHeader), pReadFile) != sizeof(FileHeader))
	{
		fclose(pReadFile);
		return false;
	}
	if(memcmp(FileHeader, Header, sizeof(FileHeader)) != 0)
	{
		fclose(pReadFile);
		return false;
	}
	fclose(pReadFile);
	return true;
}

bool ReadableBinStorage::ReadFromFile(LPCTSTR FileName)
{
	FILE *pReadFile = NULL;
	_tfopen_s(&pReadFile, FileName, _T("rb"));
	if(pReadFile == NULL)
		return false;

	fseek(pReadFile, 0, SEEK_END);
	m_Size = ftell(pReadFile);
	fseek(pReadFile, 0, SEEK_SET);

	BYTE Header[] = { 0x01, 0x02, 0x03, 0x04 };
	BYTE FileHeader[4];
	if(fread(FileHeader, 1, sizeof(FileHeader), pReadFile) != sizeof(FileHeader))
	{
		fclose(pReadFile);
		return false;
	}
	if(memcmp(FileHeader, Header, sizeof(FileHeader)) != 0)
	{
		fclose(pReadFile);
		return false;
	}
	m_Size -= sizeof(FileHeader);

	m_pBuffer = new BYTE[m_Size];
	if(m_pBuffer == NULL)
	{
		fclose(pReadFile);
		return false;
	}
	bool fSuccess = (fread(m_pBuffer, 1, m_Size, pReadFile) == (size_t)m_Size);

	fclose(pReadFile);
	return fSuccess;
}

void ReadableBinStorage::Close()
{
	if(m_pBuffer != NULL)
	{
		delete[] m_pBuffer;
		m_pBuffer = NULL;
	}
	m_Size = 0;
	m_NextByteToRead = 0;
}