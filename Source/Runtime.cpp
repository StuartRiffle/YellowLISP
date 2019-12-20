#include "Yellow.h"
#include "Runtime.h"



class Scope
{
    unordered_map<SYMBOL_INDEX, CELL_INDEX> _binding;

public:
    inline void Set(SYMBOL_INDEX symbolIndex, CELL_INDEX cellIndex)
    {
        _binding[symbolIndex] = cellIndex;
    }

    inline CELL_INDEX Lookup(SYMBOL_INDEX symbolIndex) const
    {
        CELL_INDEX cellIndex = 0;

        auto iter = _binding.find(symbolIndex);
        if (iter != _binding.end())
            cellIndex = iter->second;

        return cellIndex;
    }

    Scope() : _binding(16) {}
};

class Environment
{
    vector<Scope> _scopes;

public:

    Scope& PushScope()
    {
        _scopes.emplace_back(Scope());
    }

    void PopScope()
    {
        assert(_scopes.size() > 0);
        _scopes.pop_back();
    }

    inline void Set(SYMBOL_INDEX symbolIndex, CELL_INDEX cellIndex)
    {
        assert(_scopes.size() > 0);
        _scopes.back().Set(symbolIndex, cellIndex);
    }

    CELL_INDEX Get(SYMBOL_INDEX symbolIndex)
    {
        CELL_INDEX index = 0;

        int count = (int)_scopes.size();
        for( int i = count - 1; i >= 0; i-- )
        {
            index = _scopes[i].Get(symbolIndex);
            if (index)
                break;
        }

        return index;
    }
};


Runtime::Runtime()
{
    _nil  = ResolveSymbol("nil");
    _true = ResolveSymbol("t");

    const PrimitiveInfo primitives[] =
    {
        "atom",  1, &this->Atom,
        "car",   1, &this->Car,
        "cdr",   1, &this->Cdr,
        "cons",  2, &this->Cons,
        "eq",    2, &this->Eq,
        "quote", 1, &this->Quote,
    };

    for (auto& prim : primitives)
        RegisterPrimitive(prim);
}

Runtime::~Runtime()
{
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
    SYMBOL_INDEX symbolIndex = _globalScope[hash];

    if (!symbolIndex)
    {
        symbolIndex = _symbol.Alloc();
        _globalScope[hash] = symbolIndex;

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

    if (cell._type == TYPE_LIST)
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

 define eval(expr, environment):
   if is_literal(expr): return literal_value(expr)
   if is_symbol(expr):  return lookup_symbol(expr, environment)
   ;; other similar cases here
   ;; remaining (and commonest) case: function application
   function  = extract_function(expr)
   arguments = extract_arguments(expr)
   apply(eval(function, environment), eval_list(arguments, environment))

 define apply(function, arguments):
   if is_primitive(function): return apply_primitive(function, arguments)
   environment = augment(function_environment(function),
                         formal_args(function), arguments)
   return eval(function_body(function), environment)

 def eval_list(items, environment):
   return map( { x -> eval(x, environment) }, items)

*/


CELL_INDEX Runtime::EvaluateInScope(CELL_INDEX cellIndex, Scope& scope)
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
        SYMBOL_INDEX symbolMapping = scope.Lookup(symbolIndex);

        if (symbolMapping)
            symbolIndex = symbolMapping;

        const SymbolInfo& symbol = _symbol[symbolIndex];

        // Primitive symbols can be used directly

        if (symbol._primIndex > 0)
            return symbolIndex;

        // All others need to be evaluated

        return EvaluateInScope(symbol._cellIndex, scope);
    }

    if (cell._type == TYPE_LIST)
    {
        // Evaluate the function/operator (the first element)

        CELL_INDEX function = EvaluateInScope(cell._data, scope);

        // Evaluate the arguments (the other elements)

        ArgumentList callArgs;
        CELL_INDEX onArg = cell._next;

        while (onArg)
        {
            CELL_INDEX argValue = EvaluateInScope(onArg, scope);
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
                return prim._func(callArgs);
            }
        }

        assert(0);
    }
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
    case TYPE_LIST: ss << "(" << CellToString(cell._data) << " . " << "??" << ")"; break;
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
