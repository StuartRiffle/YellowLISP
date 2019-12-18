// YellowLISP (c) 2019 Stuart Riffle

#pragma once
#include "SyntaxTree.h"

class Runtime
{
	SlotPool<Cell>		_cells;
	SlotPool<void*>		_functions;
	SlotPool<string>	_strings;

	unordered_map<THASH, TINDEX> _symbols;

	TINDEX				_emptyList;
	TINDEX				_symbolTrue;

	void Init()
	{
		_symbolTrue = GetAtom("t");
		_emptyList = _cells.Alloc();
	}

	TINDEX Quote(TINDEX index)
	{
		return index;
	}

	TINDEX Atom(TINDEX index)
	{
		Cell& cell = _cells[index];

		if (cell.IsAtom() || cell.IsEmptyList())
			return _symbolTrue;

		return _emptyList;
	}

	TINDEX Eq(TINDEX a, TINDEX b)
	{
		Cell& ca = _cells[a];
		Cell& cb = _cells[b];

		if (ca.IsAtom() && cb.IsAtom())
			if (ca.GetAtomHash() == cb.GetAtomHash())
				return _symbolTrue;

		if (ca.IsEmptyList() && cb.IsEmptyList())
			return _symbolTrue;

		return _emptyList;
	}

	TINDEX Car(TINDEX index)
	{
	}

	TINDEX Cdr(TINDEX index)
	{

	}

	TINDEX Cons(TINDEX head, TINDEX tail)
	{
		TINDEX index = _cells.Alloc();
		Cell& cell = _cells[index];

		cell.SetCellRef(head);
		cell.SetNext(tail);

		return index;
	}

	TINDEX Parse(const char* code)
	{


		return(INVALID_INDEX);
	}

	string Print(TINDEX index)
	{

	}

	TINDEX Eval(TINDEX index)
	{

	}

	string Eval(const char* code)
	{
		TINDEX expr = Parse(code);
		TINDEX value = Eval(expr);
		string output = Print(value);

		return output;
	}

	void Repl()
	{
		string code;

		while (std::getline(cin, code))
			cout << Eval(code.c_str());
	}

	void CheckOutput(const string& input, const string& result)
	{

	}

	void Test()
	{
		CheckOutput("'a", "a");
		CheckOutput("(quote a)", "a");
		CheckOutput("(quote (a b c))", "(a b c)");


		CheckOutput("(atom 'a)", "t");
		CheckOutput("(atom '(a b c))", "()");
		CheckOutput("(atom '())", "t");
		CheckOutput("(atom (atom 'a))", "t");
		CheckOutput("(atom '(atom 'a))", "()");

		CheckOutput("(eq 'a 'a)", "t");
		CheckOutput("(eq 'a 'b)", "()");
		CheckOutput("(eq '() '())", "t");
	}

	void Run(NodeRef ast)
	{

	}
};
