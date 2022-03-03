#include "Pch.h"
#include "WebBrowserXml.h"
#include "ApplicationDataPath.h"
#include "resource.h"

#define WEBBROWSSTATEFILENAME _T("WebBrowserState.xml")

WebBrowserXml::WebBrowserXml()
	: m_xmlTree(),
	m_CurrentStateStruct(IDS_STRING_WEBBROWSERSTATE),
	m_fOpenAsSearcher(IDS_STRING_OPENASSEARCHER, true)
{
	m_CurrentStateStruct.AddItem(m_fOpenAsSearcher);
}

WebBrowserXml::~WebBrowserXml()
{
}

bool WebBrowserXml::Load(LPCTSTR UserName)
{
	ApplicationDataPath PathFinder(UserName);
	_tstring FileName;
	PathFinder.ComposePath(WEBBROWSSTATEFILENAME, FileName);

	m_xmlTree.Initialize(m_CurrentStateStruct, FileName.c_str(), IDS_STRING_APPCONFIG);

	return m_xmlTree.LoadXml();
}

bool WebBrowserXml::Store()
{
	return m_xmlTree.StoreXml(ENCODING_TYPE_UTF_16);
}

bool WebBrowserXml::ShouldOpenAsSearcher() const
{
	return m_fOpenAsSearcher.GetBool();
}

void WebBrowserXml::OpenAsSearcher(bool fSearcher)
{
	m_fOpenAsSearcher.SetBool(fSearcher);
}
