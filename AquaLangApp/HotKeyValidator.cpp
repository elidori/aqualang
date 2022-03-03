#include "Pch.h"
#include "HotKeyValidator.h"
#include "AquaLangLibLoader.h"
#include "StringConvert.h"

HotKeyValidator::HotKeyValidator(AquaLangLibLoader &rAquaLangLib)
	: m_rAquaLangLib(rAquaLangLib)
{
}

HotKeyValidator::~HotKeyValidator()
{
	Close();
}

bool HotKeyValidator::AddHotKey(LPCSTR HotKey, bool fActive, LPCTSTR DebugString, _tstring &rErrorMessage)
{
	rErrorMessage = _T("");

	HotKeyInfo *pNewItem = new HotKeyInfo;
	if(!pNewItem)
		return true; // memory allocation error

	pNewItem->DebugString = DebugString;
	pNewItem->fActive = fActive;
	pNewItem->Validator = NULL;
	m_HotKeyList.push_back(pNewItem);

	// check hot key string
	if(!m_rAquaLangLib.CreateKeySequenceValidator(HotKey, pNewItem->Validator))
	{
		rErrorMessage = DebugString;
		rErrorMessage += _T(" Hot Key string is illegal. Please verify it's correct and try again");
		return false;
	}

	// check cross reference
	if(fActive)
	{
		for(unsigned int i = 0; i < m_HotKeyList.size() - 1; i++)
		{
			if(m_HotKeyList.at(i)->fActive)
			{
				bool fConflict = false;

				if(!m_rAquaLangLib.AreSequencesEqual(pNewItem->Validator, m_HotKeyList.at(i)->Validator))
				{
					if(m_rAquaLangLib.IsContainedBySequence(pNewItem->Validator, m_HotKeyList.at(i)->Validator))
					{
						fConflict = true;
					}
					if(m_rAquaLangLib.IsContainedBySequence(m_HotKeyList.at(i)->Validator, pNewItem->Validator))
					{
						fConflict = true;
					}
				}
				if(fConflict)
				{
					rErrorMessage = _T("Hot keys ambiguity between ");
					rErrorMessage += pNewItem->DebugString + _T(" and ") + m_HotKeyList.at(i)->DebugString;
					rErrorMessage += _T(". Please verify it's corrected and try again");
					return false;
				}
			}
		}
	}
	return true;
}

void HotKeyValidator::Close()
{
	for(unsigned int i = 0; i < m_HotKeyList.size(); i++)
	{
		m_rAquaLangLib.DeleteKeySequenceValidator(m_HotKeyList.at(i)->Validator);
		delete m_HotKeyList.at(i);
	}
	m_HotKeyList.erase(m_HotKeyList.begin(), m_HotKeyList.end());
}