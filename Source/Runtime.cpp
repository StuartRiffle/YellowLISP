#include "Yellow.h"
#include "Runtime.h"

Runtime::Runtime()
{
    _primitive.emplace_back(); // element 0 is invalid

    _cellFreeList = 0;
    ExpandCellTable();

    _nil   = RegisterSymbol("nil");
    _true  = RegisterSymbol("t");
    _quote = RegisterSymbol("quote");

    // Language primitives

    RegisterPrimitive("atom",  &Runtime::ATOM);
    RegisterPrimitive("car",   &Runtime::CAR);
    RegisterPrimitive("cdr",   &Runtime::CDR);
    RegisterPrimitive("cond",  &Runtime::COND);
    RegisterPrimitive("cons",  &Runtime::CONS);
    RegisterPrimitive("eq",    &Runtime::EQ);
    RegisterPrimitive("eval",  &Runtime::EVAL);

    // Math stuff

    RegisterPrimitive("add",     &Runtime::ADD);
    RegisterPrimitive("+",       &Runtime::ADD);
    RegisterPrimitive("sub",     &Runtime::SUB);
    RegisterPrimitive("-",       &Runtime::SUB);
    RegisterPrimitive("mul",     &Runtime::MUL);
    RegisterPrimitive("*",       &Runtime::MUL);
    RegisterPrimitive("div",     &Runtime::DIV);
    RegisterPrimitive("/",       &Runtime::DIV);
    RegisterPrimitive("mod",     &Runtime::MOD);
    RegisterPrimitive("%",       &Runtime::MOD);
    RegisterPrimitive("rem",     &Runtime::REM);
    RegisterPrimitive("round",   &Runtime::ROUND);
    RegisterPrimitive("trunc",   &Runtime::TRUNC);
    RegisterPrimitive("floor",   &Runtime::FLOOR);
    RegisterPrimitive("ceiling", &Runtime::CEILING);
    RegisterPrimitive("min",     &Runtime::MIN);
    RegisterPrimitive("max",     &Runtime::MAX);
    RegisterPrimitive("expt",    &Runtime::EXPT);
    RegisterPrimitive("log",     &Runtime::LOG);
    RegisterPrimitive("sqrt",    &Runtime::SQRT);
    RegisterPrimitive("abs",     &Runtime::ABS);
    RegisterPrimitive("sin",     &Runtime::SIN);
    RegisterPrimitive("cos",     &Runtime::COS);
    RegisterPrimitive("tan",     &Runtime::TAN);
    RegisterPrimitive("random",  &Runtime::RANDOM);

    // Interpreter commands

    RegisterPrimitive("help",  &Runtime::Help);
    RegisterPrimitive("exit",  &Runtime::Exit);
    RegisterPrimitive("quit",  &Runtime::Exit);
}

Runtime::~Runtime()
{
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

    RAISE_ERROR(ERROR_RUNTIME_NOT_IMPLEMENTED);
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

        default:          RAISE_ERROR(ERROR_INTERNAL_CELL_TABLE_CORRUPT); break;
    }

    return ss.str();
}

CELL_INDEX Runtime::EvaluateCell(CELL_INDEX cellIndex)
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
        
        if (!_environment.empty())
        {
            Scope& scope = _environment.back();

            auto iter = scope.find(symbolIndex);
            if (iter != scope.end())
            {
                CELL_INDEX localValue = iter->second;
                return EvaluateCell(localValue);
            }
        }

        // Primitive symbols can be used directly

        const SymbolInfo& symbol = _symbol[symbolIndex];
        if (symbol._primIndex > 0)
            return cellIndex;

        // All others need to be evaluated

        if (symbol._cellIndex != cellIndex )
            return EvaluateCell(symbol._cellIndex);

        return cellIndex;
    }

    if (cell._type == TYPE_LIST)
    {
        // Evaluate the function/operator (the first element)

        CELL_INDEX function = EvaluateCell(cell._data);

        // Handle special form: quote

        if (function == _quote)
        {
            RAISE_ERROR_IF(cell._type != TYPE_LIST, ERROR_INTERNAL_CELL_TABLE_CORRUPT);

            CELL_INDEX quoted = cell._next;
            return _cell[quoted]._data;
        }

        // The remaining list elements are arguments

        ArgumentList callArgs;
        CELL_INDEX onArg = cell._next;

        while (onArg)
        {
            RAISE_ERROR_IF(_cell[onArg]._type != TYPE_LIST, ERROR_INTERNAL_CELL_TABLE_CORRUPT);

            CELL_INDEX argValue = EvaluateCell(_cell[onArg]._data);
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
                _environment.emplace_back();
                Scope& scope = _environment.back();

                // TODO: evaluate the function here
                (scope);
                

                _environment.pop_back();

                RAISE_ERROR(ERROR_RUNTIME_NOT_IMPLEMENTED);
            }
        }

        RAISE_ERROR(ERROR_RUNTIME_NOT_IMPLEMENTED);
    }

    return _nil;
}


