// YellowLISP (c) 2019 Stuart Riffle (MIT license)

#include "Yellow.h"
#include "Runtime.h"

CELL_INDEX Runtime::EvaluateCell(CELL_INDEX cellIndex)
{
    if (!cellIndex || (cellIndex == _nil))
        return _nil;

#ifndef NDEBUG
    static int sDumpDebugGraph = 1;
    static int sExpandSymbols = 1;
    if (sDumpDebugGraph)
    {
        //sDumpDebugGraph = 0;
        // For debugging, this generates a graph of cell connections for GraphViz to render

        char filename[80];
        sprintf(filename, "cell%d.dot", cellIndex);
        DumpCellGraph(cellIndex, filename, sExpandSymbols);
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

            auto iter = scope.find(cellIndex);
            if (iter != scope.end())
            {
                CELL_INDEX localValue = iter->second;
                return EvaluateCell(localValue);
            }
        }

        // Primitive symbols are used directly

        const SymbolInfo& symbol = _symbol[symbolIndex];
        assert(symbol._symbolCell == cellIndex);

        bool isPrim     = (symbol._type == SYMBOL_PRIMITIVE) && symbol._primIndex;
        bool isReserved = (symbol._type == SYMBOL_RESERVED);
        bool isMacro    = (symbol._type == SYMBOL_MACRO);

        if (isPrim || isReserved || isMacro)
            return cellIndex;

        CELL_INDEX value = symbol._valueCell;
        if (symbol._type == SYMBOL_MACRO)
            value = EvaluateCell(value);

        return value;
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

            bool callable = (symbol._type == SYMBOL_FUNCTION) || (symbol._type == SYMBOL_PRIMITIVE) || (symbol._type == SYMBOL_MACRO);
            RAISE_ERROR_IF(!callable, ERROR_RUNTIME_UNDEFINED_FUNCTION, symbol._ident.c_str());


            if (symbol._type == SYMBOL_PRIMITIVE)
            {
                assert(symbol._primIndex && (symbol._primIndex != _nil));
                PrimitiveInfo& prim = _primitive[symbol._primIndex];

                CELL_INDEX firstArgCell = cell._next;

                ArgumentList primArgs;
                for (CELL_INDEX argCell = firstArgCell; argCell; argCell = _cell[argCell]._next)
                {
                    CELL_INDEX value = _cell[argCell]._data;

                    // HACK: do not evaluate params for defmacro

                    if (function != _defmacro)
                        value = EvaluateCell(value);

                    primArgs.push_back(value);
                }

                CELL_INDEX primResult = (*this.*prim._func)(primArgs);
                return primResult;
            }



            // Bind the arguments

            Scope callScope;

            if (symbol._bindingListCell)
            {
                CELL_INDEX argSymbolCell = symbol._bindingListCell;
                CELL_INDEX bindingCell = symbol._valueCell;

                while (argSymbolCell)
                {
                    RAISE_ERROR_IF(!bindingCell, ERROR_RUNTIME_WRONG_NUM_PARAMS);

                    CELL_INDEX   argCellIndex   = _cell[argSymbolCell]._data;
                    SYMBOL_INDEX argSymbolIndex = _cell[argCellIndex]._data;
                    SymbolInfo&  argSymbol      = _symbol[argSymbolIndex];

                    // Evaluate the argument (if this isn't a macro)

                    CELL_INDEX boundCell = _cell[cell._next]._data;
                    if (symbol._type != SYMBOL_MACRO)
                        boundCell = EvaluateCell(boundCell);

                    callScope[argSymbol._symbolCell] = boundCell;

                    argSymbolCell = _cell[argSymbolCell]._next;
                    bindingCell = _cell[bindingCell]._next;
                }

                //RAISE_ERROR_IF(bindingCell, ERROR_RUNTIME_WRONG_NUM_PARAMS);
            }

            // Evaluate the function body

            CELL_INDEX callResult = _nil;
            _environment.push_back(std::move(callScope));

            try
            {
                callResult = EvaluateCell(symbol._valueCell);
            }
            catch (...)
            {
                _environment.pop_back();
                throw;
            }

            _environment.pop_back();
            return callResult;
        }

        RAISE_ERROR(ERROR_RUNTIME_UNDEFINED_FUNCTION);
    }

    return _nil;
}


