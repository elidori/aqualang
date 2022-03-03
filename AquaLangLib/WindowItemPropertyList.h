#ifndef _WINDOW_ITEM_PROPERTY_LIST_HEADER_
#define _WINDOW_ITEM_PROPERTY_LIST_HEADER_

#include "xmlStructure.h"
#include "WindowItemProperty.h"

typedef xmlFixedSizeMap<WindowItemProperty, MaxNumberOfProperties> PropertyListXml;

class WindowItemPropertyList : public PropertyListXml
{
public:
	WindowItemPropertyList(int xmlNameId);
	virtual ~WindowItemPropertyList();

	const xmlValue *GetPropertyValue(WindowItemPropertyType_t Type) const;

	bool Initialize(HWND hwnd);
	bool Initialize(LPCTSTR SubObjectId);

	bool Update(const WindowItemPropertyList &rRef);

	bool IsMatching(const WindowItemPropertyList &rRef, bool fEmptyListMatches) const;
	void GetDebugString(_tstring &rString) const;

	bool IsOutOfDate(const FileTime &rCurrentTime) const;
protected:
	virtual WindowItemProperty *CreateFromXml(
									XmlNode &rxItemNode,
									int NameId,
									int &rnKey
									);
	virtual WindowItemProperty *CreateFromBinStorage(
									ReadableBinStorage &rBinStorage,
									int NameId,
									int &rnKey
									);
};

#endif // _WINDOW_ITEM_PROPERTY_LIST_HEADER_
