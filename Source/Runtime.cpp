// YellowLISP (c) 2019 Stuart Riffle (MIT license)

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

    RegisterPrimitive("atom",    &Runtime::ATOM);
    RegisterPrimitive("car",     &Runtime::CAR);
    RegisterPrimitive("cdr",     &Runtime::CDR);
    RegisterPrimitive("cond",    &Runtime::COND);
    RegisterPrimitive("cons",    &Runtime::CONS);
    RegisterPrimitive("eq",      &Runtime::EQ);
    RegisterPrimitive("eval",    &Runtime::EVAL);
    RegisterPrimitive("list",    &Runtime::LIST);
    RegisterPrimitive("setq",    &Runtime::SETQ);

    RegisterPrimitive("+",       &Runtime::ADD);
    RegisterPrimitive("-",       &Runtime::SUB);
    RegisterPrimitive("*",       &Runtime::MUL);
    RegisterPrimitive("/",       &Runtime::DIV);
    RegisterPrimitive("%",       &Runtime::MOD);
    RegisterPrimitive("<",       &Runtime::LESS);

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
    CELL_INDEX cellIndex = _symbol[symbolIndex]._symbolCell;

    _symbol[symbolIndex]._primIndex = SymbolInfo::RESERVED;
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
        symbol._symbolCell = cellIndex;
        symbol._valueCell = _nil;
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

    return symbol._symbolCell;
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
            return symbol._symbolCell;
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
                if (_cell[curr]._next)
                    ss << " ";
                curr = _cell[curr]._next;
            }
            ss << ')';
            break;
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

#ifndef NDEBUG
    static bool sDumpDebugGraph = false;
    if (sDumpDebugGraph)
    {
        sDumpDebugGraph = false;
        // For debugging, this generates a graph of cell connections for GraphViz to render

        char filename[80];
        sprintf(filename, "cell%d.dot", cellIndex);
        DumpCellGraph(cellIndex, filename);
    }
#endif

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
        assert(symbol._symbolCell == cellIndex);

        if (symbol._primIndex > 0)
        {
            assert(symbol._valueCell == _nil);
            return cellIndex;
        }

        // All others need to be evaluated

        //return EvaluateCell(symbol._valueCell);

        return symbol._valueCell;
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
            RAISE_ERROR_IF(_cell[quoted]._next, ERROR_RUNTIME_WRONG_NUM_PARAMS);

            return _cell[quoted]._data;
        }

        // Call the function

        if (_cell[function]._type == TYPE_SYMBOL)
        {
            SYMBOL_INDEX symbolIndexFunc = _cell[function]._data;
            const SymbolInfo& symbol = _symbol[symbolIndexFunc];

            // The remaining list elements are arguments, so evaluate them

            ArgumentList callArgs;
            CELL_INDEX onArg = cell._next;
            int argIndex = 1;

            while (onArg)
            {
                RAISE_ERROR_IF(_cell[onArg]._type != TYPE_LIST, ERROR_INTERNAL_CELL_TABLE_CORRUPT);

                // HACK! until macros work, implicitly quote the first argument of SETQ

                bool evaluateThisArg = true;
                if (argIndex == 1)
                    if (symbol._ident == "setq")
                        evaluateThisArg = false;

                CELL_INDEX argValue = _cell[onArg]._data;
                if (evaluateThisArg)
                    argValue = EvaluateCell(argValue);

                callArgs.push_back(argValue);

                Cell& argCell = _cell[onArg];
                onArg = argCell._next;
                argIndex++;
            }

            RAISE_ERROR_IF(symbol._primIndex == SymbolInfo::RESERVED, ERROR_RUNTIME_UNDEFINED_FUNCTION, symbol._ident.c_str());

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


