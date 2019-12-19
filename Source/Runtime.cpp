#include "Yellow.h"
#include "Runtime.h"

void Runtime::Init()
{
    _nil = ResolveSymbol("nil");
    _true = ResolveSymbol("t");
}

CELL_INDEX Runtime::Quote(CELL_INDEX index)
{
    return index;
}

CELL_INDEX Runtime::Atom(CELL_INDEX index)
{
    Cell& cell = _cell[index];
    return cell._next ? _true : _nil;
}

CELL_INDEX Runtime::Eq(CELL_INDEX a, CELL_INDEX b)
{
    if (a == b)
        return _true;

    Cell& ca = _cell[a];
    Cell& cb = _cell[b];

    if (ca._type == cb._type)
    {
        if (ca._data == cb._data)
            return _true;

        if (ca._type == TYPE_STRING)
            if (_string[ca._data] == _string[cb._data])
                return _true;
    }

    return _nil;
}

CELL_INDEX Runtime::Car(CELL_INDEX index)
{
    Cell& cell = _cell[index];
    if (cell._type != TYPE_CELL_REF)
        return _nil;

    return cell._data;
}

CELL_INDEX Runtime::Cdr(CELL_INDEX index)
{
    Cell& cell = _cell[index];
    if (cell._next)
        return cell._next;

    return _nil;
}

CELL_INDEX Runtime::Cons(CELL_INDEX head, CELL_INDEX tail)
{
    CELL_INDEX index = _cell.Alloc();
    Cell& cell = _cell[index];

    cell._type = TYPE_CELL_REF;
    cell._data = head;
    cell._next = tail;

    return index;
}

SYMBOL_INDEX Runtime::ResolveSymbol(const char* ident)
{
    THASH hash = HashString(ident);
    SYMBOL_INDEX symbolIndex = _symbolIndex[hash];

    if (!symbolIndex)
    {
        symbolIndex = _symbol.Alloc();
        _symbolIndex[hash] = symbolIndex;

        CELL_INDEX cellIndex = _cell.Alloc();
        _cell[cellIndex]._type = TYPE_SYMBOL;
        _cell[cellIndex]._data = symbolIndex;

        SymbolInfo& symbol = _symbol[symbolIndex];
        symbol._ident = ident;
        symbol._cellIndex = cellIndex;
    }

    return symbolIndex;
}

string Runtime::CellToString(CELL_INDEX index)
{
    if (index == _nil)
        return "()";

    stringstream ss;

    Cell& cell = _cell[index];
    switch (cell._type)
    {
    case TYPE_CELL_REF: ss << "(" << CellToString(cell._data) << " . " << "??" << ")"; break;
    case TYPE_SYMBOL:   ss << _symbol[cell._data]._ident; break;
    case TYPE_FUNC:     ss << _function[cell._data]._name; break;
    case TYPE_STRING:   ss << "\"" << _string[cell._data] << "\""; break;
    case TYPE_INT:      ss << cell.GetInteger(); break;
    case TYPE_FLOAT:    ss << cell.GetFloat(); break;
    default:            assert(!"Internal error");
    }

    return ss.str();
}
