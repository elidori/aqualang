#include "Pch.h"
#include "IndexMarker.h"

IndexMarker::IndexMarker()
	: m_fLatest(false),
	m_PrevMarkIndex(-1),
	m_MarkIndex(-1)
{
}

IndexMarker::~IndexMarker()
{
}

void IndexMarker::Set(const IndexMarker &r)
{
	m_fLatest = r.m_fLatest;
	m_PrevMarkIndex = r.m_PrevMarkIndex;
	m_MarkIndex = r.m_MarkIndex;
}

void IndexMarker::Mark(int Offset)
{
	if(m_MarkIndex != -1 && !m_fLatest)
	{
		m_PrevMarkIndex = m_MarkIndex;
	}
	m_MarkIndex = Offset;
	m_fLatest = true;
}

void IndexMarker::Clear()
{
	m_fLatest = false;
	m_PrevMarkIndex = -1;
	m_MarkIndex = -1;
}

void IndexMarker::DecreaseOffset()
{
	if(m_MarkIndex != -1)
	{
		m_MarkIndex--;
	}
	if(m_PrevMarkIndex != -1)
	{
		m_PrevMarkIndex--;
	}
}

void IndexMarker::AddedLatest(bool fUpdateMark)
{
	if(m_MarkIndex != -1)
	{
		if(fUpdateMark)
		{
			if(m_fLatest)
			{
				m_MarkIndex++;
			}
		}
		else
		{
			m_fLatest = false;
		}
	}
}

void IndexMarker::RemovedLatest(int NewFulllness)
{
	if(m_MarkIndex != -1 && m_fLatest)
	{
		m_MarkIndex--;
	}

	if(m_MarkIndex != -1 && m_MarkIndex > NewFulllness)
	{
		m_MarkIndex = NewFulllness;
	}
	if(m_PrevMarkIndex != -1 && m_PrevMarkIndex > m_MarkIndex)
	{
		m_PrevMarkIndex = -1;
	}
}

int IndexMarker::GetOffset() const
{
	if(m_MarkIndex != -1 && !m_fLatest)
	{
		return m_MarkIndex;
	}
	else if(m_PrevMarkIndex != -1)
	{
		return m_PrevMarkIndex;
	}
	return 0;
}
