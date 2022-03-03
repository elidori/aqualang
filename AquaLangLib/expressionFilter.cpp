#include "Pch.h"
#include "expressionFilter.h"

// returns NULL if no match. or pointer to char array with filterName if matched.
int takeNum(LPCTSTR str, index_t start, index_t end) 
{
	index_t i;
	int r = 0;
	for (i=start; i<end; i++) 
	{
		if (i==start)
		{
			if ( str[i]>=_T('0') && str[i]<=_T('9'))
				r = (int) (str[i] - _T('0'));
			else
				return 0;
		}
		else if (str[i]>=_T('0') && str[i]<=_T('9'))
		{ 
			r *=10; 
			r+= (int) (str[i]-_T('0')); 
		}
	}
	return r;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// filter
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool filter::checkLimits(LPCTSTR str) {

	int tmpNum, tmpLow, tmpHigh;
	
	tmpNum = takeNum(str, bonis, csi); 
	if (fltrLow != NULL) {
		insideANumber = false;
		tmpLow = takeNum(fltrLow, bonip, cpi);
		if (tmpNum < tmpLow) return false;
	}
	if (fltrHigh != NULL) {
		insideANumber = false;
		tmpHigh = takeNum(fltrHigh, bonip, cpi);
		if (tmpNum > tmpHigh) return false;
	}
	return true;
}

bool filter::isMatching(LPCTSTR str) {
	bool stillMatching = true;
	size_t l = _tcslen(str);
	size_t lp = _tcslen(this->fltrPattern);
	if (l > lp) stillMatching = false;               // a string longer than pattern impossible !
	csi = cpi = 0;
	bonip = 0;   // begining-of-number-index-pattern
	bonis = 0;   // begining-of-number-idex-string
	insideANumber = false;

	while (stillMatching && cpi<(index_t)lp && csi<(index_t)l) {
		if (str[csi]==_T(' ')) {csi++; continue;}
		switch (this->fltrPattern[cpi]) {
			case _T('#'):
				if (!insideANumber) {bonip=cpi; bonis=csi; insideANumber = true;}
				if (str[csi]>=_T('0') && str[csi]<=_T('9')) {
					csi++; cpi++;
					continue;
				}
				else stillMatching = false;
				break;
			case _T('@'):
				if (!insideANumber) {bonip=cpi; bonis=csi;insideANumber = true;}
				if (str[csi]>=_T('0') && str[csi]<=_T('9')) {
					csi++; cpi++;
					continue;
				}
				else {
					while (this->fltrPattern[cpi]==_T('@')) cpi++; // skip all @ chars in pattern
					if (str[csi] == this->fltrPattern[cpi]) {  // next char should be matching
						cpi++; csi++;
						stillMatching = checkLimits(str) ;
						continue;
					}
					else stillMatching = false;
				}
				break;
			default:
				if (insideANumber)	// we must check the limites of the former number
					stillMatching = checkLimits(str);
				if (str[csi] == this->fltrPattern[cpi]) {
					csi++; cpi++;
					continue;
				}
				else stillMatching = false;
		}
	}

	if (insideANumber)
		while (fltrPattern[cpi]==_T('@')) cpi++;	// exaust any trailing @'s
	if (cpi<(index_t)lp) stillMatching = false;			// if pattern is not complete it's not a match
	while(str[csi]==_T(' '))
		csi++                 ;                 // exaust trailing blanks
	if (csi<(index_t)l)  stillMatching = false;			// if pattern matches only part of string it's not a match
	if (stillMatching && insideANumber) stillMatching = checkLimits(str);  // perhaps the last num is not within limits
	return stillMatching;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ExpressionFilter
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ExpressionFilter::ExpressionFilter()
	: m_FilterCount(0)
{
}

ExpressionFilter::~ExpressionFilter()
{
	Close();
}

bool ExpressionFilter::Open()
{
	if(m_FilterCount != 0)
		return true;

	m_filters[0] = new filter(_T("Cell Phone"),	_T("05#-#######"), NULL, NULL);
	m_filters[1] = new filter(_T("Line Phone"),	_T("0#-#######"), NULL, NULL);
	m_filters[2] = new filter(_T("dd/mm/yyyy"),	_T("#@/#@/####"),
											_T("01/01/1900"),
											_T("31/12/2099"));
	m_filters[3] = new filter(_T("dd/mm/yy"),		_T("#@/#@/##"),
											_T("01/01/00"),
											_T("31/12/99"));
	m_filters[4] = new filter(_T("mm/dd/yyyy"),	_T("#@/#@/####"),
											_T("01/01/1900"),
											_T("12/31/2099"));
	m_filters[5] = new filter(_T("mm/dd/yy"),		_T("#@/#@/##"),
											_T("01/01/00"),
											_T("12/31/99"));
	m_filters[6] = new filter(_T("Integer"),		_T("@@@@@@@@@@@@"), NULL, NULL);
	m_filters[7] = new filter(_T("Pos Int"),		_T("+@@@@@@@@@@@@"), NULL, NULL);
	m_filters[8] = new filter(_T("Neg Int"),		_T("-@@@@@@@@@@@@"), NULL, NULL);
	m_filters[9] = new filter(_T("Decimal"),		_T("@@@@@@@@@@@@.@@@@@@@@@@@@"), NULL, NULL);
	m_filters[10] = new filter(_T("Index"),		_T("@@)"), _T("@@-"), NULL);
	m_filters[11] = new filter(_T("CreditCard"),	_T("################"), NULL, NULL);
	m_filters[12] = new filter(_T("DashedCredit"),	_T("####-####-####-####"), _T("@@-"), NULL);

	m_FilterCount = 13;
	return true;
}

LPCTSTR ExpressionFilter::FindFilter(LPCTSTR str)
{
	for(int i = 0; i < m_FilterCount; i++)
	{
		if(m_filters[i]->isMatching(str))
		{
			return m_filters[i]->fltrGetName();
		}
	}
	return NULL;
}

void ExpressionFilter::Close()
{
	for(int i = 0; i < m_FilterCount; i++)
	{
		delete m_filters[i];
	}
	m_FilterCount = 0;
}
