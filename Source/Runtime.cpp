#include "Yellow.h"
#include "Runtime.h"

Runtime::Runtime()
{
    _nil  = ResolveSymbol("nil");
    _true = ResolveSymbol("t");
}

Runtime::~Runtime()
{
}

CELL_INDEX Runtime::Quote(CELL_INDEX index) const
{
    return index;
}

CELL_INDEX Runtime::Atom(CELL_INDEX index) const
{
    const Cell& cell = _cell[index];
    return cell._next ? _true : _nil;
}

CELL_INDEX Runtime::Eq(CELL_INDEX a, CELL_INDEX b) const
{
    if (a == b)
        return _true;

    const Cell& ca = _cell[a];
    const Cell& cb = _cell[b];

    if (ca._type == cb._type)
    {
        if (ca._tags & cb._tags & TAG_EMBEDDED)
            if (ca._data == cb._data)
                return _true;

        if (ca._type == TYPE_STRING)
        {
            const char* vala = LoadStringLiteral(a);
            const char* valb = LoadStringLiteral(b);

            if (!strcmp(vala, valb))
                return _true;
        }
    }

    return _nil;
}

CELL_INDEX Runtime::Car(CELL_INDEX index) const
{
    const Cell& cell = _cell[index];
    if (cell._type != TYPE_CELL_REF)
        return _nil;

    return cell._data;
}

CELL_INDEX Runtime::Cdr(CELL_INDEX index) const
{
    const Cell& cell = _cell[index];
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

int Runtime::LoadIntLiteral(CELL_INDEX index) const
{
    const Cell& cell = _cell[index];
    assert(cell._type == TYPE_INT);
    assert(cell._tags & TAG_EMBEDDED);

    int value = (int) cell._data;
    return value;
}

void Runtime::StoreIntLiteral(CELL_INDEX index, int value)
{
    Cell& cell = _cell[index];
    assert((cell._type == TYPE_INT) || (cell._type == TYPE_VOID));

    cell._data = value;
    cell._type = TYPE_INT;
    cell._tags = TAG_EMBEDDED | TAG_ATOM;
}

float Runtime::LoadFloatLiteral(CELL_INDEX index) const
{
    const Cell& cell = _cell[index];
    assert(cell._type == TYPE_FLOAT);
    assert(cell._tags & TAG_EMBEDDED);

    union { TDATA raw; float value; } pun;
    pun.raw = cell._data;

    return(pun.value);
}

void Runtime::StoreFloatLiteral(CELL_INDEX index, float value)
{
    Cell& cell = _cell[index];
    assert((cell._type == TYPE_FLOAT) || (cell._type == TYPE_VOID));

    union { TDATA raw; float value; } pun;
    pun.value = value;

    cell._data = pun.raw;
    cell._type = TYPE_FLOAT;
    cell._tags = TAG_EMBEDDED | TAG_ATOM;
}

const char* Runtime::LoadStringLiteral(CELL_INDEX index) const
{
    const Cell& cell = _cell[index];
    assert(cell._type == TYPE_STRING);

    if (cell._tags & TAG_EMBEDDED)
    {
        const char* tiny = (const char*)&cell._data;
        assert(strlen(tiny) < sizeof(cell._data));

        return (const char*)&cell._data;
    }

    STRING_INDEX stringIndex = cell._data;
    assert((stringIndex > 0) && (stringIndex < _string.GetPoolSize()));

    const string& value = _string[stringIndex];
    assert(value.length() >= sizeof(TDATA));

    return value.c_str();
}

void Runtime::StoreStringLiteral(CELL_INDEX index, const char* value)
{
    Cell& cell = _cell[index];
    assert((cell._type == TYPE_STRING) || (cell._type == TYPE_VOID));

    cell._type = TYPE_STRING;
    cell._tags = TAG_ATOM;

    size_t length = strlen(value);
    if(length < sizeof(cell._data))
    {
        cell._data = 0;
        strcpy((char*)&cell._data, value);
        cell._tags |= TAG_EMBEDDED;
    }
    else
    {
        STRING_INDEX stringIndex = _string.Alloc();
        _string[stringIndex] = value;
    }
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

CELL_INDEX Runtime::AllocateCell()
{
    CELL_INDEX index = _cell.Alloc();

    if (!index)
    {
        size_t numCellsFreed = CollectGarbage();
        float pctFreed = numCellsFreed * 1.0f / _cell.GetPoolSize();
        float pctUsed = 1 - pctFreed;

        if (pctUsed >= CELL_TABLE_EXPAND_THRESH)
        {
            // Garbage collection didn't help much. We will probably
            // hit the wall again soon, so pre-emptively expand the pool.

            _cell.ExpandPool();
        }

        index = _cell.Alloc();
    }

    return index;
}

void Runtime::MarkCellsInUse(CELL_INDEX index)
{
    Cell& cell = _cell[index];

    if (cell._tags & TAG_GC_MARK)
        return;

    cell._tags |= TAG_GC_MARK;

    if (cell._type == TYPE_CELL_REF)
        MarkCellsInUse(cell._data);

    if (cell._next)
        MarkCellsInUse(cell._next);
}

size_t Runtime::CollectGarbage()
{
    size_t numCellsFreed = 0;

    for (TINDEX i = 1; i < _cell.GetPoolSize(); i++)
        assert(_cell[i]._tags & TAG_GC_MARK == 0);

    // TODO: call MarkCellsInUse here

    for (TINDEX i = 1; i < _cell.GetPoolSize(); i++)
    {
        Cell& cell = _cell[i];

        if (cell._tags & TAG_GC_MARK)
        {
            cell._tags &= ~TAG_GC_MARK;
        }
        else
        {
            _cell.Free(i);
            numCellsFreed++;
        }
    }

    return numCellsFreed;
}



CELL_INDEX EncodeSyntaxTree(const NodeRef& node)
{
    

    // Encode the entire tree into cells
    // Call Eval() on the root

    // If it's a list...

    // Start with one node type: integer

    NodeRef node = root;

    EvalResult result;
}

string Runtime::GetPrintedValue(CELL_INDEX index)
{
    return "FIXME";
}


/*
string Runtime::CellToString(CELL_INDEX index)
{
    if (index == _nil)
        return "()";

    std::stringstream ss;

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
*/
