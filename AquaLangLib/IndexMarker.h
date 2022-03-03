#ifndef _INDEX_MARKER_HEADER_
#define _INDEX_MARKER_HEADER_

class IndexMarker
{
public:
	IndexMarker();
	virtual ~IndexMarker();

	void Set(const IndexMarker &r);

	void Mark(int Offset);
	void Clear();

	void DecreaseOffset();
	
	void AddedLatest(bool fUpdateMark);
	void RemovedLatest(int Fulllness);

	int GetOffset() const;
private:
	bool m_fLatest;

	int m_PrevMarkIndex;
	int m_MarkIndex;
};

#endif // _INDEX_MARKER_HEADER_