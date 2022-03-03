#ifndef _EXPRESSION_FILTER_HEADER_
#define _EXPRESSION_FILTER_HEADER_

// expression filter class
#define MAXFILTERS 30

class filter {
	TCHAR *fltrName;
	TCHAR *fltrPattern;
	TCHAR *fltrLow;
	TCHAR *fltrHigh;
	index_t csi, cpi;		// csi=current-str-index, cpi=current-pattern-index
	index_t bonis, bonip;  // begining of number (in string, in pattern)
	bool insideANumber;

	bool checkLimits(LPCTSTR str);

public:
	// the constructor sets the filter name and pattern
	filter(TCHAR *name, TCHAR *pattern, TCHAR *low, TCHAR *high) { 
			fltrName = name; fltrPattern = pattern;	
			fltrLow = low;
			fltrHigh = high;
			csi = cpi = bonis = bonip = 0;
			insideANumber = false;

	};
	TCHAR *fltrGetName() { return fltrName; };
	bool isMatching(LPCTSTR str);
};

class ExpressionFilter
{
public:
	ExpressionFilter();
	virtual ~ExpressionFilter();

	bool Open();
	LPCTSTR FindFilter(LPCTSTR str);
private:
	void Close();
private:
	filter *m_filters[MAXFILTERS];
	int m_FilterCount;
};

#endif // _EXPRESSION_FILTER_HEADER_