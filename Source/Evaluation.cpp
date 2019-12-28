// YellowLISP (c) 2019 Stuart Riffle (MIT license)

#include "Yellow.h"
#include "Runtime.h"


vector<CELL_INDEX> Runtime::ExtractList(CELL_INDEX cellIndex)
{
    vector<CELL_INDEX> result;

    while (VALID_CELL(cellIndex))
    {
        result.push_back(cellIndex);
        if (_cell[cellIndex]._type != TYPE_LIST)
            break;

        cellIndex = _cell[cellIndex]._next;
    }

    return result;
}

CELL_INDEX Runtime::ExpandMacro(CELL_INDEX bindingListCell, CELL_INDEX argListCell)
{
    (bindingListCell);
    (argListCell);
    return _nil;
}

CELL_INDEX Runtime::EvaluateCell(CELL_INDEX cellIndex)
{
    if (!VALID_CELL(cellIndex))
        return _nil;

    // Symbols in the current scope override globals

    if (!_environment.empty())
    {
        Scope& scope = _environment.back();

        auto iter = scope.find(cellIndex);
        if (iter != scope.end())
        {
            CELL_INDEX localValue = iter->second;
            cellIndex = localValue;
        }
    }

#ifndef NDEBUG
    static int sDumpDebugGraph = 1;
    static int sExpandSymbols = 1;
    // For debugging, this generates a graph of cell connections for GraphViz to render
    if (sDumpDebugGraph)
        DumpCellGraph(cellIndex, sExpandSymbols);
#endif

    const Cell& cell = _cell[cellIndex];
    CELL_INDEX lambdaCell = _nil;
    bool evaluateArguments = true;

    if (cellIndex == _quote)
    {
        // Special form: quote

        CELL_INDEX quoted = cell._next;
        RAISE_ERROR_IF(!VALID_CELL(quoted), ERROR_RUNTIME_WRONG_NUM_PARAMS);

        return _cell[quoted]._data;
    }

    switch (cell._type)
    {
        case TYPE_SYMBOL:
        {
            SYMBOL_INDEX symbolIndex = cell._data;
            const SymbolInfo& symbol = _symbol[symbolIndex];

            assert(symbol._symbolCell == cellIndex);
            if (!VALID_CELL(symbol._valueCell))
                return _nil;

            switch (symbol._type)
            {
                case SYMBOL_RESERVED:
                    return cellIndex;

                case SYMBOL_VARIABLE:
                    return symbol._valueCell;

                case SYMBOL_FUNCTION:
                    lambdaCell = symbol._valueCell;
                    break;

                case SYMBOL_MACRO:
                    lambdaCell = ExpandMacro(symbol._macroBindings, symbol._valueCell);
                    evaluateArguments = false;
                    break;

                case SYMBOL_PRIMITIVE:
                {
                    bool evaluateArgs = ((cellIndex != _defmacro) && (cellIndex != _defun));
                    CELL_INDEX argCellIndex = cell._next;

                    return CallPrimitive(symbolIndex, argCellIndex, evaluateArgs);
                }
            }

            break;
        }

        case TYPE_LIST:
        {
            lambdaCell = EvaluateCell(cell._data);
            RAISE_ERROR_IF(_cell[lambdaCell]._type != TYPE_LAMBDA, ERROR_RUNTIME_INVALID_ARGUMENT, "first element not a function");
            break;
        }

        case TYPE_INT:
        case TYPE_FLOAT:
        case TYPE_STRING:
        case TYPE_LAMBDA:
            return cellIndex;
    }

    assert(VALID_CELL(lambdaCell));

    CELL_INDEX bindingCellIndex  = _cell[lambdaCell]._data;
    CELL_INDEX bodyCellIndex = _cell[lambdaCell]._next;

    // Bind the arguments

    CELL_INDEX bindingList = _cell[bindingCellIndex]._data;
    assert(_cell[bindingList]._type == TYPE_LIST);

    CELL_INDEX argList = cell._next;
    assert(_cell[argList]._type == TYPE_LIST);

    Scope callScope = BindArguments(bindingList, argList, evaluateArguments);

    // Evaluate the function body

    CELL_INDEX callResult = _nil;
    _environment.push_back(std::move(callScope));

    try
    {
        callResult = EvaluateCell(bodyCellIndex);
    }
    catch (...)
    {
        _environment.pop_back();
        throw;
    }

    _environment.pop_back();
    return callResult;
}

Runtime::Scope Runtime::BindArguments(CELL_INDEX bindingList, CELL_INDEX argList, bool evaluateArgs)
{
    Scope scope;

    while (VALID_CELL(argList))
    {
        RAISE_ERROR_IF(!VALID_CELL(bindingList), ERROR_RUNTIME_WRONG_NUM_PARAMS);

        CELL_INDEX boundSymbolCell = _cell[bindingList]._data;
        SYMBOL_INDEX boundSymbolIndex = _cell[boundSymbolCell]._data;

        CELL_INDEX value = _cell[argList]._data;
        if (evaluateArgs)
            value = EvaluateCell(value);

        scope[boundSymbolIndex] = value;

        argList = _cell[argList]._next;
        bindingList = _cell[bindingList]._next;
    }

    return scope;
}

CELL_INDEX Runtime::CallPrimitive(SYMBOL_INDEX symbolIndex, CELL_INDEX argCellIndex, bool evaluateArgs)
{
    const SymbolInfo& symbol = _symbol[symbolIndex];

    assert(symbol._primIndex && (symbol._primIndex != _nil));
    PrimitiveInfo& prim = _primitive[symbol._primIndex];

    ArgumentList primArgs;

    vector<CELL_INDEX> elements = ExtractList(argCellIndex);
    for (auto argCell : elements)
    {
        CELL_INDEX value = _cell[argCell]._data;

        if (evaluateArgs)
            value = EvaluateCell(value);

        primArgs.push_back(value);
    }

    CELL_INDEX primResult = (*this.*prim._func)(primArgs);
    return primResult;
}


