// ==================================================================================================================
// Class: infixNode
//                                  CopyRight (C) MeshiEl Investments Ltd.
//                          Any use / duplication of this code is strictly forbidden 
//                                      and copyright protected
// Author: Eliyahu Mashiah
//
// Description: This class maintains a tree that represents an infix arithmetic expression.
//              A mature tree structure is:   Parent  -> Left-Child
//                                                    -> Right Child
//              Where the parent represents the operation ( + Plus, - Minus, etc.)
//              and the children represnt the arguments. 
//              each object of this class is a single node within the tree.
//              The class public methods implement a final-state-machine FSM that accepts "events"
//              such as "a new number is found", "a new operator" is found, etc. This class does NOT
//              implement the parsing of an infix string
//
// Class Data members: (see infixNode.h)
// Class Public Methods: 
//              evalNode - evaluate the expression represented nu the current node.
//              storeNum - a parser found a new number in the expression and wants to store it 
//              storeOp  - a parser found a new operator in the expression and wants to store it
//              storeFunc - a parser found a new function such as sin(), log() and wants to store it
//              clearTree - free all memory and stuctures starting this node.
//             
// 
// ===================================================================================================================
#include "Pch.h"
#include "infixNode.h"

#define MAXDIGITS


double infixNode::funcOnNum()
{
	double r;

	if ( degMode && ( func == _T('s') || func==_T('c') || func==_T('t')) )
		num = (num/ 180) * _PI;
	switch (this->func) {
		case _T('l'): r =  log(num); 			break;
		case _T('s'): r =  sin(num);			break;
		case _T('c'): r =  cos(num);			break;
		case _T('t'): r =  tan(num);			break;
		case _T('S'): r =  asin(num);			break;
		case _T('C'): r =  acos(num);			break;
		case _T('T'): r =  atan(num);			break;
		default:  r = num;
	}
	if (degMode && (func==_T('S') || func==_T('C') || func==_T('T')))
		r = ( (r / _PI) * 180 ) ; 
	return r;
}


double infixNode::evalNode() {
		double l,r;
		switch (state) {
			case 0: return 0.0;
			case 1: return funcOnNum();
			case 2: return Left->evalNode();
			case 3: case 4:
				l = Left->evalNode();
				r = Right->evalNode();
				switch (op) {
					case _T('+'): num = (l + r); return funcOnNum();
					case _T('-'): num = (l - r); return funcOnNum();
					case _T('*'): num = (l * r); return funcOnNum();
					case _T('/'): num = (l / r); return funcOnNum();
					case _T('^'): num = pow(l, r); return funcOnNum();
					default: return 0;
				}
			default: return 0;
		}
	}

// A new number is thrown to the automate
int infixNode::storeNum(LPCTSTR n) {
		switch (state) {
			case 0:				// we have an empty node, let's fill it.
				   _stscanf_s(n, _T("%lf"), &num);
				   state = 1;
				   return 0;
			case 1:
				   return -1;   // error we already have a stored number
			case 2:				// we have a left number, now we should store a right number
				   Right = new infixNode(degMode);
				   Right->storeNum(n);
				   state = 3;
				   return 0;
			case 3:				// error: we have 2 numbers stored
				   return -1;
			case 4:				// dive to lower level on Right
				   return Right->storeNum(n);
			default:
				  return -2 ;    // Automate is out-of-state
		}
		
	}
	
	// A new Operator is thrown to this automate
int infixNode::storeOperator(TCHAR oprtr)
{
	double d;
	switch (state) {
		case 0:				
			return -1;
		case 1:             //we have 1 number, now it is part of an expression
			Left = new infixNode(degMode);
			Left->setNum(num); Left->setState(1);
			op  = oprtr;
			state = 2;
			return 0;
		case 2:				// we already have an operator....
			return -1;
		case 3:				// we have a+b and now new operator...
		case 4:
			if ( rankOfOp(oprtr)<=rankOfOp(op) ) { // a+b+...
				d = evalNode();          // calculate the Node's value
				this->Left->setNum(d);   // set result to the left argument
				op = oprtr; 
				if (Right!=NULL) { delete(Right); Right = NULL;}
				state = 2;
			}
			else {						// a+b*...
				Right->storeOperator(oprtr);
				state = 4;				// dive to lower level
			}
			return 0;
		default:
			return -1;                 // automate is out of state
	}
}

int infixNode::storeFunc(TCHAR tok)
{
	if (tok==_T('s') ||tok==_T('S') || tok==_T('l') || tok==_T('c') || tok==_T('C') ||tok==_T('t') ||tok==_T('T'))
			this->func = tok; 
	else	this->func = 0;
	return 0;
}


int infixNode::clearTree() {
	num=0;
	op=0;
	func = 0;
	state=0;
	if (Left!=NULL) {
		Left->clearTree();
		delete(Left);
		Left=NULL;
	}
	if (Right!=NULL) {
		Right->clearTree();
		delete(Right);
		Right=NULL;
	}
	return 0;
}

void infixNode::SetDegMode(bool fDeg)
{
	degMode = fDeg;
	if(Left)
	{
		Left->SetDegMode(fDeg);
	}
	if(Right)
	{
		Right->SetDegMode(fDeg);
	}
}
