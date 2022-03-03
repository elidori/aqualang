#include "Pch.h"
#include "infixString.h"
#include "infixNode.h"

TCHAR *dblToStr(double d)
{
	static TCHAR retStr[MAXOPLEN]={0};
	TCHAR tmpStr[MAXOPLEN]={0};
	int i;
	for (i=0; i<MAXOPLEN; i++)	retStr[i]=0; // clean it
	
	if(_stprintf_s(tmpStr, sizeof(tmpStr) / sizeof(tmpStr[0]), _T("%lf"), d) != -1)
	{
		for (i=0; i<MAXOPLEN && tmpStr[i]; i++) retStr[i]=tmpStr[i];
	    return(retStr);
	}
	else return NULL;
}

bool isOperator(TCHAR c)
{
	if (c==_T('+') || c==_T('-') || c==_T('*') || c==_T('/') || c==_T('^'))
		return true;
	return false;
}

TCHAR isToken(LPCTSTR Str, int i)
{
	static TCHAR *tokens[]= {_T("PI"), _T("[e]"), _T("log("), _T("sin("), _T("asin("), _T("cos("), _T("acos("), _T("tan("), _T("atan(")	};
	static TCHAR opCode[] =	  { _T('P'), _T('e'), _T('l'), _T('s'), _T('S'), _T('c'), _T('C'), _T('t'), _T('T') };
	TCHAR tmp[10];
	int t, j;
	j = sizeof(tokens) / sizeof(tokens[0]);
	for (t=0; t<5; t++) {
		if ( Str[i+t]==0 || Str[i+t]==' ' || isOperator(Str[i+t]) || Str[i+t]==_T(')') )
			break;
		else {
			tmp[t]= Str[i+t];
			if (tmp[t]==_T('(')) {t++; break;}
		}
	}
	tmp[t]=0;
	for (t=0; t<j; t++)  { // foreach token
		if (_tcscmp( tokens[t], tmp)==0)
			return opCode[t];
	}
	return 0;
}



bool isConstant(TCHAR tok)
{
	if (tok==_T('P') || tok==_T('e'))
		return true;
	else
		return false;
}

void setConstant(TCHAR *operand, TCHAR tok)
{
	TCHAR PI[]    =  _T("3.141592653589793");
	TCHAR  e[]    =  _T("2.718281828459045");
	unsigned int i;

	switch (tok) {
		case _T('P'): for(i=0; i<MAXOPLEN && i<_tcslen(PI); i++)
					  operand[i] = PI[i];
			operand[i]=0;
			break;
		case _T('e'): for(i=0; i<MAXOPLEN && i<_tcslen(e); i++)
					  operand[i]=e[i];
			operand[i]=0;
			break;
		default: ;
	}
}

int toklen(TCHAR tok)
{
	switch (tok){
		case _T('e'):
			return 2;
		case _T('P'):
			return 1;
		case _T('l'):
		case _T('s'):
		case _T('c'):
		case _T('t'): 
			return 3;
		case _T('S'):
		case _T('C'):
		case _T('T'):
			return 4;
		default:
			return 0;
	}
}

bool numberStart(TCHAR c) 
{
	if ( (c>=_T('0') && c<=_T('9') ) || (c==_T('+')) || (c==_T('-')) || (c==_T('.')) )
		return true;
	else 
		return false;
}

void clearAll(infixNode topLevel[], int ct, TCHAR operand[])
{
	int i;
	for(i=0; i<=ct; i++) 
		topLevel[i].clearTree();
	for (i=0; i<MAXOPLEN; i++)
		operand[i]=0;
}


double evalInfixString(LPCTSTR Str, bool fDeg, int *offset)
{
#define MAXLEVELS 10
#define NULL 0
	double r;
	unsigned int i, state;
	TCHAR operand[MAXOPLEN]={0};
	TCHAR oprtr, c;
	int     operandIndex;
	infixNode topLevel[MAXLEVELS];	 //stack of expressions
	int ct = 0;						 // current topLevel
	TCHAR tok;

	for(i=0; i<MAXLEVELS; i++){
		topLevel[i].SetDegMode(fDeg);
	}
	state = 0; // 0=expecting operand, 1=On-operand, 2=expecting operator
	operandIndex = 0;
	oprtr = 0;
	*offset = 0;

	for (i=0; i<_tcslen(Str); i++){
		c = Str[i];
		switch (state) {
			case 0:   // expecting operand
				if (c!=_T(' '))  {
					if ( numberStart(c) ){
						operand[operandIndex++]=c;
						state = 1; // on op1
					}
					else if (c==_T('(')) 
							if (ct < MAXLEVELS - 1) {
								ct++;
								topLevel[ct].clearTree(); // clear any former data
							}
							else return 0;				  // too many nested levels
					else if ( (tok = isToken(Str, i)) != 0 ) {
							i = i + toklen(tok);
							if ( isConstant(tok) ) { 
								setConstant(operand, tok); 
								topLevel[ct].storeNum(operand);
								state = 2; 
							}
							else {	if (ct < MAXLEVELS - 1) {
										 ct++; topLevel[ct].clearTree(); topLevel[ct].storeFunc(tok); }
									else return 0;
								 }
						 }
						else {
							*offset=i+1;					// on any other char advance offset and zero-out
							clearAll(topLevel, ct, operand);
							operandIndex = ct = 0;
						}
				}
			    break;
			case 1:  // on operand
				if (c==_T(' ') || c==_T(')') ) {
					state = 2;        //expecting operator
					operand[operandIndex++]=0;
					topLevel[ct].storeNum(operand);
					if (c==_T(')')) {
						r = topLevel[ct].evalNode() ;
						if (ct>0) {
						    topLevel[ct].clearTree(); // clear for future use
							ct--;
						}
						_tcscpy_s(operand, MAXOPLEN, dblToStr(r));
						topLevel[ct].storeNum(operand);
					}
				}
				else if ( isOperator(c) ) {
						oprtr = c;
						operand[operandIndex++] = 0;
						topLevel[ct].storeNum(operand);
						topLevel[ct].storeOperator(oprtr);
						state = 0;      // now back to expecting operand
						for (operandIndex=0; operandIndex<MAXOPLEN; operandIndex++)
							operand[operandIndex]=0;
						operandIndex=0;
						}
				     else
					    if ( (c>=_T('0') && c<=_T('9')) || (c == _T('.')) )
							operand[operandIndex++]=c;
						else { // on a number with alphabet? impossible !
							*offset = i;
							clearAll(topLevel, ct, operand);
							operandIndex = ct = 0;
							state = 0;
						}
				break;
			case 2:                      //expecting operator
				if (c==_T(' ')) break;
				if (isOperator(c)) {
					oprtr = c;
					topLevel[ct].storeOperator(oprtr);
					state = 0;           //again to expecting operand
					for (operandIndex=0; operandIndex<MAXOPLEN; operandIndex++) // clear the operand
						operand[operandIndex]=0;
					operandIndex = 0;
				}
				else if (c==_T(')')) {
						r = topLevel[ct].evalNode() ;
						if (ct>0) {
							topLevel[ct].clearTree();
							ct--;
						}
						_tcscpy_s(operand, MAXOPLEN, dblToStr(r) );
						topLevel[ct].storeNum(operand);
					 }
					 else {						// we got some alphabet instead of operator
							*offset = i;
							clearAll(topLevel, ct, operand); 
							operandIndex = 0;
							if ( numberStart(c) ) {
								state = 1;    // if new number started we're "on-operand"
								operand[operandIndex++]=c;
							}
							else state =0;    // otherwise we're now expecting an operand
					 }
				break;
		}
	}
	if (state==1){ // if we are out of loop but we were on-operand we should store it
		operand[operandIndex++]=0;
		topLevel[0].storeNum(operand);
	}
	if (ct>0) {		// we're inside unclosed parenthesis
		clearAll(topLevel, ct, operand);
		r = 0;
	}
	else {			// The normal case
			r = topLevel[0].evalNode();
			topLevel[0].clearTree(); // free-up alocated memory
	}
    return r;
}
