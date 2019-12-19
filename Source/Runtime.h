// YellowLISP (c) 2019 Stuart Riffle

#pragma once
#include "Cell.h"
#include "SyntaxTree.h"
#include "SlotPool.h"
#include "Hash.h"

struct SymbolInfo
{
    string _ident;
    TINDEX _cellIndex;
};

struct FunctionInfo
{
    string _name;
};

typedef TINDEX CELL_INDEX;
typedef TINDEX SYMBOL_INDEX;
typedef TINDEX FUNC_INDEX;
typedef TINDEX STRING_INDEX;

class Runtime
{
    SlotPool<Cell>          _cell;
    SlotPool<SymbolInfo>    _symbol;
    SlotPool<FunctionInfo>  _function;
    SlotPool<string>        _string;

    unordered_map<THASH, SYMBOL_INDEX> _symbolIndex;

    CELL_INDEX          _nil;
    CELL_INDEX          _true;

    void Init();

    SYMBOL_INDEX ResolveSymbol(const char* ident);
    string CellToString(CELL_INDEX index);

    CELL_INDEX Quote(CELL_INDEX index);
    CELL_INDEX Atom(CELL_INDEX index);
    CELL_INDEX Eq(CELL_INDEX a, CELL_INDEX b);
    CELL_INDEX Car(CELL_INDEX index);
    CELL_INDEX Cdr(CELL_INDEX index);
    CELL_INDEX Cons(CELL_INDEX head, CELL_INDEX tail);
};
