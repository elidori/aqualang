// class for Infix expression eval
#define MAXDIGITS
#define _PI 3.141592653589793

class infixNode {
private:
	int  state; //automate state: 0-empty, 1=num-stored, 2=Op+Left, 3=Op+Left+Right
				// 4=dive-lower-on-right 5=function-stored
	TCHAR op;
	double num;
	TCHAR func;
	infixNode *Left, *Right;			// Left and Right operands
	bool degMode;						// Degrees or Radians

public:
	infixNode(bool fDeg = true){ // constructor;
	  state = 0;
	  op    = 0;
	  num   = 0.0;
	  func  = 0;
	  degMode = fDeg;					// by-default we are in degrees
	  Left = Right = NULL;
	}
	  
private:
	void setNum(double n) { num = n; }
	void setOp(TCHAR o) { op = o; }
	void setState(int i)  {state = i;}
	int rankOfOp(TCHAR o){
		switch (o) {
			case _T('+'): case _T('-'): return 0;
			case _T('*'): case _T('/'): return 1;
			case _T('^'): return 2;
			default: return -1;
		}
	}
	double funcOnNum();
public:
	double evalNode();
	int storeNum(LPCTSTR n);
	int storeOperator(TCHAR oprtr);
	int storeFunc(TCHAR tok);
	int clearTree();

	void SetDegMode(bool fDeg);
} ;