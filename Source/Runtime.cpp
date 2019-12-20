#include "Yellow.h"
#include "Runtime.h"

Runtime::Runtime()
{
    _nil  = ResolveSymbol("nil");
    _true = ResolveSymbol("t");

    RegisterPrimitive("atom",  &Runtime::ATOM);
    RegisterPrimitive("car",   &Runtime::CAR);
    RegisterPrimitive("cdr",   &Runtime::CDR);
    RegisterPrimitive("cond",  &Runtime::COND);
    RegisterPrimitive("cons",  &Runtime::CONS);
    RegisterPrimitive("eq",    &Runtime::EQ);
    RegisterPrimitive("eval",  &Runtime::EVAL);
    RegisterPrimitive("quote", &Runtime::QUOTE);
}

Runtime::~Runtime()
{
}

int Runtime::LoadIntLiteral(CELL_INDEX index) 
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

float Runtime::LoadFloatLiteral(CELL_INDEX index) 
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

const char* Runtime::LoadStringLiteral(CELL_INDEX index) 
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
        strncpy((char*)&cell._data, value, sizeof(cell._data));
        cell._tags |= TAG_EMBEDDED;
    }
    else
    {
        STRING_INDEX stringIndex = (STRING_INDEX) _string.Alloc();
        _string[stringIndex] = value;
    }
}

SYMBOL_INDEX Runtime::ResolveSymbol(const char* ident)
{
    THASH hash = HashString(ident);
    SYMBOL_INDEX symbolIndex = _globalScope[hash];

    if (!symbolIndex)
    {
        symbolIndex = (SYMBOL_INDEX) _symbol.Alloc();
        _globalScope[hash] = symbolIndex;

        CELL_INDEX cellIndex = AllocateCell(TYPE_SYMBOL);
        _cell[cellIndex]._data = symbolIndex;

        SymbolInfo& symbol = _symbol[symbolIndex];
        symbol._ident = ident;
        symbol._cellIndex = cellIndex;
    }

    return symbolIndex;
}

SYMBOL_INDEX Runtime::RegisterPrimitive(const char* ident, PrimitiveFunc func)
{
    SYMBOL_INDEX symbolIndex = ResolveSymbol(ident);
    SymbolInfo& symbol = _symbol[symbolIndex];

    symbol._primIndex = (TINDEX) _primitive.Alloc();
    PrimitiveInfo& primInfo = _primitive[symbol._primIndex];

    primInfo._name = ident;
    primInfo._func = func;

    return symbolIndex;
}

CELL_INDEX Runtime::AllocateCell(Type type)
{
    CELL_INDEX index = (CELL_INDEX) _cell.Alloc();
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

        index = (CELL_INDEX) _cell.Alloc();
    }

    if (index)
        _cell[index]._type = type;

    return index;
}

void Runtime::MarkCellsInUse(CELL_INDEX index)
{
    Cell& cell = _cell[index];

    if (cell._tags & TAG_GC_MARK)
        return;

    cell._tags |= TAG_GC_MARK;

    if (cell._type == TYPE_LIST)
        MarkCellsInUse(cell._data);

    if (cell._next)
        MarkCellsInUse(cell._next);
}

size_t Runtime::CollectGarbage()
{
    size_t numCellsFreed = 0;

    for (TINDEX i = 1; i < _cell.GetPoolSize(); i++)
        assert((_cell[i]._tags & TAG_GC_MARK) == 0);

    // FIXME: right now can't distinguish cells in use, add a tag for that
    // and implement the cell pool by chaining indices instead of pointers

    for (auto iter : _globalScope)
    {
        SYMBOL_INDEX symbolIndex = iter.second;
        SymbolInfo& symbol = _symbol[symbolIndex];

        if (symbol._cellIndex)
            MarkCellsInUse(symbol._cellIndex);
    }

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

CELL_INDEX Runtime::EncodeSyntaxTree(const NodeRef& node)
{
    switch (node->_type)
    {
        case AST_NODE_INT_LITERAL:
        {
            CELL_INDEX intCell = AllocateCell(TYPE_INT);
            StoreIntLiteral(intCell, node->_int);
            return intCell;
        }
        case AST_NODE_FLOAT_LITERAL:
        {
            CELL_INDEX floatCell = AllocateCell(TYPE_FLOAT);
            StoreFloatLiteral(floatCell, node->_float);
            return floatCell;
        }
        case AST_NODE_STRING_LITERAL:
        {
            CELL_INDEX stringCell = AllocateCell(TYPE_STRING);
            StoreStringLiteral(stringCell, node->_string.c_str());
            return stringCell;

        }
        case AST_NODE_IDENTIFIER:
        {
            CELL_INDEX symbolCell = ResolveSymbol(node->_identifier.c_str());
            return symbolCell;
        }
        case AST_NODE_LIST:
        {
            if (node->_list.empty())
                return _nil;

            CELL_INDEX listHeadCell = 0;
            CELL_INDEX listPrevCell = 0;

            for (auto& elemNode : node->_list)
            {
                CELL_INDEX listCell = AllocateCell(TYPE_LIST);
                _cell[listCell]._data = EncodeSyntaxTree(elemNode);

                if (listPrevCell)
                    _cell[listPrevCell]._next = listCell;
                else
                    listHeadCell = listCell;

                listPrevCell = listCell;
            }

            return listHeadCell;
        }
    }

    assert(!"TODO: raise runtime error here");
    return 0;
}

string Runtime::GetPrintedValue(CELL_INDEX index)
{
    return "?";
}

CELL_INDEX Runtime::EvaluateCell(CELL_INDEX cellIndex, const Scope& scope)
{
    if (!cellIndex || (cellIndex == _nil))
        return _nil;

    const Cell& cell = _cell[cellIndex];

    // Literals evaluate to themselves

    bool isLiteral =
        (cell._type == TYPE_INT) ||
        (cell._type == TYPE_FLOAT) ||
        (cell._type == TYPE_STRING);

    if (isLiteral)
        return cellIndex;

    if (cell._type == TYPE_SYMBOL)
    {
        // Symbols in the current scope override globals

        SYMBOL_INDEX symbolIndex = cell._data;
        SYMBOL_INDEX symbolOverride = scope.Lookup(symbolIndex);

        if (symbolOverride)
            symbolIndex = symbolOverride;

        const SymbolInfo& symbol = _symbol[symbolIndex];

        // Primitive symbols can be used directly

        if (symbol._primIndex > 0)
            return symbolIndex;

        // All others need to be evaluated

        return EvaluateCell(symbol._cellIndex, scope);
    }

    if (cell._type == TYPE_LIST)
    {
        // Evaluate the function/operator (the first element)

        CELL_INDEX function = EvaluateCell(cell._data, scope);

        // Evaluate the arguments (all the other elements)

        ArgumentList callArgs;
        CELL_INDEX onArg = cell._next;

        while (onArg)
        {
            CELL_INDEX argValue = EvaluateCell(onArg, scope);
            callArgs.push_back(argValue);

            Cell& argCell = _cell[onArg];
            onArg = argCell._next;
        }

        // Call the function

        if (_cell[function]._type == TYPE_SYMBOL)
        {
            SYMBOL_INDEX symbolIndexFunc = cell._data;
            const SymbolInfo& symbol = _symbol[symbolIndexFunc];

            if (symbol._primIndex)
            {
                // This is a primitive operation

                PrimitiveInfo& prim = _primitive[symbol._primIndex];
                return (*this.*prim._func)(callArgs);
            }
        }

        

        assert(0);
    }

    return _nil;
}


