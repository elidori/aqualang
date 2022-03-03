#ifndef _WEB_BROWSER_XML_HEADER_
#define _WEB_BROWSER_XML_HEADER_

#include "xmlStructure.h"

class WebBrowserXml
{
public:
	WebBrowserXml();
	virtual ~WebBrowserXml();

	bool Load(LPCTSTR UserName);
	bool Store();

	bool ShouldOpenAsSearcher() const;
	void OpenAsSearcher(bool fSearcher);
private:
	xmlTree m_xmlTree;
	xmlStructure m_CurrentStateStruct;

	xmlBool m_fOpenAsSearcher;
};

#endif // _WEB_BROWSER_XML_HEADER_