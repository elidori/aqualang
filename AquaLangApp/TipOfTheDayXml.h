#ifndef _TIP_OF_THE_DAY_XML_HEADER_
#define _TIP_OF_THE_DAY_XML_HEADER_

#include "xmlStructure.h"

typedef xmlItemArray<xmlString> TipListXml;

class TipOfTheDayStatusXml
{
public:
	TipOfTheDayStatusXml();
	virtual ~TipOfTheDayStatusXml();

	bool Open(LPCTSTR UserName);
	bool StoreState();
public:
	xmlInt m_LatestIndexPresented;
	xmlTime m_NextTimeToPresentTip;
private:
	xmlStructure m_TipOfTheDayStatusStructure;
	xmlTree m_xmlTree;
};

class TipOfTheDayListXml : public TipListXml
{
public:
	TipOfTheDayListXml();
	virtual ~TipOfTheDayListXml();

	bool LoadTipList(LPCTSTR UserName);

	LPCTSTR GetText(index_t nIndex, _tstring &rText) const;
protected:
	virtual xmlString *CreateFromXml(
					XmlNode &rxItemNode,
					int NameId
					);
	virtual xmlString *CreateFromBinStorage(
					ReadableBinStorage &rBinStorage,
					int NameId
					);
private:
	xmlStructure m_TipOfTheDayXmlStructure;
	
	xmlTree m_xmlTree;
};

#endif // _TIP_OF_THE_DAY_XML_HEADER_