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

#ifndef NDEBUG
    static int sDumpDebugGraph = 1;
    static int sExpandSymbols = 1;
    // For debugging, this generates a graph of cell connections for GraphViz to render
    if (sDumpDebugGraph)
        DumpCellGraph(cellIndex, sExpandSymbols);
#endif

    Cell& cell = _cell[cellIndex];
    CELL_INDEX lambdaCell = _nil;
    TINDEX primitiveIndex = 0;
    bool evaluateArguments = true;

    switch (cell._type)
    {
        case TYPE_SYMBOL:
        {
            SYMBOL_INDEX symbolIndex = cell._data;

            // Symbols in the current scope override globals

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

            const SymbolInfo& symbol = _symbol[symbolIndex];
            assert(symbol._symbolCell == cellIndex);

            RAISE_ERROR_IF(symbol._type == SYMBOL_INVALID, ERROR_RUNTIME_VARIABLE_UNBOUND, symbol._ident.c_str());

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
                    return cellIndex;
                    //primitiveIndex = symbol._primIndex;
                    //evaluateArguments = ((cellIndex != _defmacro) && (cellIndex != _defun) && (cellIndex != _setq));
                    //break;
                }

                default:
                    assert(!"Invalid symbol type");
            }

            break;
        }

        case TYPE_LIST:
        {
            CELL_INDEX head = cell._data;
            assert(VALID_CELL(head));

            if (head == _quote)
            {
                // Special form: quote

                CELL_INDEX quoted = cell._next;
                RAISE_ERROR_IF(!VALID_CELL(quoted), ERROR_RUNTIME_WRONG_NUM_PARAMS);
                RAISE_ERROR_IF(VALID_CELL(_cell[quoted]._next), ERROR_RUNTIME_WRONG_NUM_PARAMS);

                return _cell[quoted]._data;
            }

            if (_cell[head]._type == TYPE_SYMBOL)
            {
                SYMBOL_INDEX headSymbolIndex = _cell[head]._data;
                const SymbolInfo& headSymbol = _symbol[headSymbolIndex];

                if (headSymbol._type == SYMBOL_PRIMITIVE)
                {
                    primitiveIndex = headSymbol._primIndex;
                    evaluateArguments = ((head != _defmacro) && (head != _defun) && (head != _setq) && (head != _lambda));
                }
                else if (headSymbol._type == SYMBOL_FUNCTION)
                {
                    lambdaCell = headSymbol._valueCell;
                }
            }
            else if (_cell[head]._type == TYPE_LIST)
            {
                lambdaCell = EvaluateCell(head);
            }

            RAISE_ERROR_IF(!VALID_CELL(lambdaCell) && !primitiveIndex, ERROR_RUNTIME_UNDEFINED_FUNCTION, "first list element must be a function");
            break;
        }

        case TYPE_INT:
        case TYPE_FLOAT:
        case TYPE_STRING:
        case TYPE_LAMBDA:
            return cellIndex;

        default:
            assert(!"Invalid cell type");
    }

    // At this point we [should] have something callable

    if (primitiveIndex)
    {
        CELL_INDEX argCellIndex = cell._next;
        CELL_INDEX primResult = CallPrimitive(primitiveIndex, argCellIndex, evaluateArguments);
        return primResult;
    }

    assert(VALID_CELL(lambdaCell));

    CELL_INDEX bindingListIndex = _cell[lambdaCell]._data;
    CELL_INDEX bodyCellIndex = _cell[lambdaCell]._next;

    // Bind the arguments

    assert(_cell[bindingListIndex]._type == TYPE_LIST);

    CELL_INDEX argList = cell._next;
    assert(_cell[argList]._type == TYPE_LIST);

    Scope callScope = BindArguments(bindingListIndex, argList, evaluateArguments);

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

CELL_INDEX Runtime::CallPrimitive(TINDEX primIndex, CELL_INDEX argCellIndex, bool evaluateArgs)
{
    PrimitiveInfo& prim = _primitive[primIndex];
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


