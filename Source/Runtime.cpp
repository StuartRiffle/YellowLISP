#include "Yellow.h"
#include "Runtime.h"

Runtime::Runtime()
{
    _cellFreeList = 0;
    ExpandCellTable();

    _nil   = RegisterSymbol("nil");
    _true  = RegisterSymbol("t");
    _quote = RegisterSymbol("quote");

    RegisterPrimitive("atom",  &Runtime::ATOM);
    RegisterPrimitive("car",   &Runtime::CAR);
    RegisterPrimitive("cdr",   &Runtime::CDR);
    RegisterPrimitive("cond",  &Runtime::COND);
    RegisterPrimitive("cons",  &Runtime::CONS);
    RegisterPrimitive("eq",    &Runtime::EQ);
    RegisterPrimitive("eval",  &Runtime::EVAL);
}

Runtime::~Runtime()
{
}

int Runtime::LoadIntLiteral(CELL_INDEX index)
{
    const Cell& cell = _cell[index];
    assert(cell._type == TYPE_INT);
    assert(cell._tags & FLAG_EMBEDDED);

    int value = (int)cell._data;
    return value;
}

void Runtime::StoreIntLiteral(CELL_INDEX index, int value)
{
    Cell& cell = _cell[index];
    assert((cell._type == TYPE_INT) || (cell._type == TYPE_VOID));

    cell._data = value;
    cell._type = TYPE_INT;
    cell._tags = FLAG_EMBEDDED;
}

float Runtime::LoadFloatLiteral(CELL_INDEX index)
{
    const Cell& cell = _cell[index];
    assert(cell._type == TYPE_FLOAT);
    assert(cell._tags & FLAG_EMBEDDED);

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
    cell._tags = FLAG_EMBEDDED;
}

string Runtime::LoadStringLiteral(CELL_INDEX index)
{
    const Cell& cell = _cell[index];
    assert(cell._type == TYPE_STRING);

    if (cell._tags & FLAG_EMBEDDED)
    {
        char tiny[5] = { 0 };
        const char* chars = (const char*) &cell._data;
        strncpy(tiny, chars, sizeof(cell._data));

        return tiny;
    }

    STRING_INDEX stringIndex = cell._data;
    assert((stringIndex > 0) && (stringIndex < _string.GetPoolSize()));

    const string& value = _string[stringIndex]._str;
    assert(value.length() >= sizeof(TDATA));

    return value;
}

void Runtime::StoreStringLiteral(CELL_INDEX index, const char* value)
{
    Cell& cell = _cell[index];
    assert((cell._type == TYPE_STRING) || (cell._type == TYPE_VOID));

    cell._type = TYPE_STRING;

    size_t length = strlen(value);
    if (length <= sizeof(cell._data))
    {
        cell._data = 0;
        strncpy((char*)&cell._data, value, sizeof(cell._data));
        cell._tags |= FLAG_EMBEDDED;
    }
    else
    {
        STRING_INDEX stringIndex = 0;

        THASH hash = HashString(value);
        auto existing = _stringTable.find(hash);
        if (existing != _stringTable.end())
        {
            stringIndex = existing->second;
        }
        else
        {
            stringIndex = (STRING_INDEX)_string.Alloc();
            _string[stringIndex]._str = value;
            _stringTable[hash] = stringIndex;
        }

        _string[stringIndex]._refCount++;
    }
}

CELL_INDEX Runtime::RegisterSymbol(const char* ident)
{
    SYMBOL_INDEX symbolIndex = GetSymbolIndex(ident);
    CELL_INDEX cellIndex = _symbol[symbolIndex]._cellIndex;
    return cellIndex;
}

SYMBOL_INDEX Runtime::GetSymbolIndex(const char* ident)
{
    THASH hash = HashString(ident);
    SYMBOL_INDEX symbolIndex = _globalScope[hash];

    if (!symbolIndex)
    {
        symbolIndex = (SYMBOL_INDEX)_symbol.Alloc();
        _globalScope[hash] = symbolIndex;

        CELL_INDEX cellIndex = AllocateCell(TYPE_SYMBOL);
        _cell[cellIndex]._data = symbolIndex;

        SymbolInfo& symbol = _symbol[symbolIndex];
        symbol._ident = ident;
        symbol._cellIndex = cellIndex;
    }

    return symbolIndex;
}

CELL_INDEX Runtime::RegisterPrimitive(const char* ident, PrimitiveFunc func)
{
    SYMBOL_INDEX symbolIndex = GetSymbolIndex(ident);
    SymbolInfo& symbol = _symbol[symbolIndex];

    symbol._primIndex = (TINDEX)_primitive.size();
    _primitive.emplace_back();
    PrimitiveInfo& primInfo = _primitive.back();

    primInfo._name = ident;
    primInfo._func = func;

    return symbol._cellIndex;
}


CELL_INDEX Runtime::AllocateCell(Type type)
{
    if (_cellFreeList == 0)
    {
        /*
        size_t numCellsFreed = CollectGarbage();

        float pctFreed = numCellsFreed * 1.0f / _cell.size();
        float pctUsed = 1 - pctFreed;
        assert((pctUsed >= 0) && (pctUsed <= 1));

        if (pctUsed >= CELL_TABLE_EXPAND_THRESH)
            ExpandCellTable();
            */
        ExpandCellTable();
    }

    if (_cellFreeList == 0)
        return 0;

    CELL_INDEX index = _cellFreeList;
    _cellFreeList = _cell[_cellFreeList]._next;

    _cell[index]._type = type;
    _cell[index]._next = 0;
    _cell[index]._tags = FLAG_IN_USE;

    return index;
}

void Runtime::FreeCell(CELL_INDEX index)
{
    _cell[index]._type = TYPE_VOID;
    _cell[index]._tags = 0;
    _cell[index]._data = 0;
    _cell[index]._next = _cellFreeList;

    _cellFreeList = index;
}

void Runtime::ExpandCellTable()
{
    size_t oldSize = _cell.size();
    size_t newSize = oldSize + (oldSize / 2) + 1;

    _cell.resize(newSize);

    for (size_t i = oldSize; i < newSize; i++)
        FreeCell((CELL_INDEX) i);
}

void Runtime::MarkCellsInUse(CELL_INDEX index)
{
    Cell& cell = _cell[index];

    if (cell._tags & FLAG_GC_MARK)
        return;

    cell._tags |= FLAG_GC_MARK;

    if (cell._type == TYPE_LIST)
        MarkCellsInUse(cell._data);

    if (cell._next)
        MarkCellsInUse(cell._next);
}

size_t Runtime::CollectGarbage()
{
    size_t numCellsFreed = 0;

    for (TINDEX i = 1; i < _cell.size(); i++)
        assert((_cell[i]._tags & FLAG_GC_MARK) == 0);

    // Mark

    for (auto iter : _globalScope)
    {
        SYMBOL_INDEX symbolIndex = iter.second;
        SymbolInfo& symbol = _symbol[symbolIndex];

        if (symbol._cellIndex)
            MarkCellsInUse(symbol._cellIndex);
    }

    // TODO: also mark cells used by all enclosing scopes

    // Sweep

    for (TINDEX i = 1; i < _cell.size(); i++)
    {
        if (_cell[i]._tags & FLAG_GC_MARK)
        {
            assert(_cell[i]._tags & FLAG_IN_USE);
            _cell[i]._tags &= ~FLAG_GC_MARK;
        }
        else
        {
            if (!(_cell[i]._tags & FLAG_EMBEDDED))
            {
                if (_cell[i]._type == TYPE_STRING)
                {
                    STRING_INDEX stringIndex = _cell[i]._data;
                    StringInfo& info = _string[stringIndex];

                    assert(info._refCount > 0);
                    info._refCount--;

                    if (info._refCount == 0)
                    {
                        _string.Free(stringIndex);

                        THASH hash = HashString(info._str.c_str());
                        assert(_stringTable[hash] == stringIndex);
                        _stringTable.erase(hash);
                    }
                }
            }

            FreeCell(i);
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
            CELL_INDEX symbolCell = GetSymbolIndex(node->_identifier.c_str());
            SymbolInfo& symbol = _symbol[symbolCell];
            return symbol._cellIndex;
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
    if (index == 0)
        return "";

    Cell& cell = _cell[index];
    std::stringstream ss;

    switch (cell._type)
    {
        case TYPE_LIST:
        {
            ss << '(';
            CELL_INDEX curr = index;
            while (curr)
            {
                ss << GetPrintedValue(_cell[curr]._data);
                curr = _cell[curr]._next;
            }
            ss << ')';
        }

        case TYPE_INT:    ss << LoadIntLiteral(index); break;
        case TYPE_FLOAT:  ss << LoadFloatLiteral(index); break;
        case TYPE_STRING: ss << '\"' << LoadStringLiteral(index) << '\"'; break;
        case TYPE_SYMBOL: ss << _symbol[cell._data]._ident; break;
        default:          assert(!"Internal error");
    }

    return ss.str();
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
            return cellIndex;

        // All others need to be evaluated

        if (symbol._cellIndex != cellIndex )
            return EvaluateCell(symbol._cellIndex, scope);

        return cellIndex;
    }

    if (cell._type == TYPE_LIST)
    {
        // Evaluate the function/operator (the first element)

        CELL_INDEX function = EvaluateCell(cell._data, scope);

        // Special form: quote

        if (function == _quote)
        {
            CELL_INDEX quoted = cell._next;
            return quoted;
        }

        // The remaining list elements are arguments

        ArgumentList callArgs;
        CELL_INDEX onArg = cell._next;

        while (onArg)
        {
            assert(_cell[onArg]._type == TYPE_LIST);

            CELL_INDEX argValue = EvaluateCell(_cell[onArg]._data, scope);
            callArgs.push_back(argValue);

            Cell& argCell = _cell[onArg];
            onArg = argCell._next;
        }

        // Call the function

        if (_cell[function]._type == TYPE_SYMBOL)
        {
            SYMBOL_INDEX symbolIndexFunc = _cell[function]._data;
            const SymbolInfo& symbol = _symbol[symbolIndexFunc];

            if (symbol._primIndex)
            {
                // This is a primitive operation

                PrimitiveInfo& prim = _primitive[symbol._primIndex];
                CELL_INDEX primResult = (*this.*prim._func)(callArgs);
                return primResult;
            }
            else
            {
                assert(!"Not implemented");
            }
        }

        assert(0);
    }

    return _nil;
}


