// YellowLISP (c) 2020 Stuart Riffle (MIT license)

#include "Yellow.h"
#include "Runtime.h"


vector<CELLID> Runtime::ExtractList(CELLID cellIndex)
{
    vector<CELLID> result;

    // FIXME: wtf is this
    while (VALID_CELL(cellIndex) && (_cell[cellIndex]._type == TYPE_CONS))
    {
        result.push_back(_cell[cellIndex]._data);
        cellIndex = _cell[cellIndex]._next;
    }

    return result;
}

CELLID Runtime::GenerateList(const vector<CELLID>& elements)
{
    CELLID head = _nil;

    for (auto iter = elements.crbegin(); iter != elements.crend(); ++iter)
    {
        CELLID elem = *iter;
        CELLID elemLink = AllocateCell(TYPE_CONS);

        _cell[elemLink]._data = elem;
        _cell[elemLink]._next = head;

        head = elemLink;
    }

    return head;
}


CELLID Runtime::ExpandQuasiquoted(CELLID cellIndex, int level)
{
    //DumpCellGraph(cellIndex, true);

    vector<CELLID> elements = ExtractList(cellIndex);
    if (elements.empty())
    {
        if (level == 0)
            return EvaluateCell(cellIndex);

        return cellIndex;
    }

    if (elements[0] == _quote)
    {
        RAISE_ERROR_IF(elements.size() != 2, ERROR_RUNTIME_WRONG_NUM_PARAMS, "quote");

        CELLID quoted = elements[1];

        if (level == 0)
            return EvaluateCell(quoted);

        CELLID expanded = ExpandQuasiquoted(elements[1], level);
        CELLID requoted = GenerateList({ _quote, expanded });
        return requoted;
    }

    if (elements[0] == _unquote)
    {
        RAISE_ERROR_IF(elements.size() != 2, ERROR_RUNTIME_WRONG_NUM_PARAMS, "unquote");
        RAISE_ERROR_IF(level < 1, ERROR_RUNTIME_INVALID_MACRO_EXPANSION, "you can't unquote what isn't quoted");

        CELLID unquoted = elements[1];

        if (level == 1)
            return EvaluateCell(unquoted);

        CELLID expanded = ExpandQuasiquoted(unquoted, level - 1);
        CELLID reunquoted = GenerateList({ _unquote, expanded });
        return reunquoted;
    }

    if (elements[0] == _quasiquote)
    {
        RAISE_ERROR_IF(elements.size() != 2, ERROR_RUNTIME_WRONG_NUM_PARAMS, "quasiquote");
        //RAISE_ERROR_IF(level > 0, ERROR_RUNTIME_INVALID_MACRO_EXPANSION, "only one level of quasiquote is supported");

        // (quasiquote foo) => (expand foo) at higher level

        CELLID quasiquoted = elements[1];

        CELLID expanded = ExpandQuasiquoted(quasiquoted, level + 1);

        if (level > 0)
        {
            CELLID requoted = GenerateList({ _quasiquote, expanded });
            return requoted;
        }

        return expanded;
    }

    for (size_t i = 0; i < elements.size(); i++)
        elements[i] = ExpandQuasiquoted(elements[i], level);

    return GenerateList(elements);
}

CELLID Runtime::EvaluateCell(CELLID index)
{
    assert(VALID_INDEX(index));
    if (index == _nil)
        return _nil;

#if 1
    static int sDumpDebugGraph = 1;
    static int sExpandSymbols = 1;
    // For debugging, this generates a graph of cell connections for GraphViz to render
    if (sDumpDebugGraph)
        DumpCellGraph(index, sExpandSymbols);
#endif

    CELLID lambdaCell = _nil;
    TINDEX primitiveIndex = INVALID_INDEX;
    bool evaluateArguments = true;
    bool isMacro = false; // HACK

    switch (_cell[index]._type)
    {
        case TYPE_SYMBOL:
        {
            SYMBOLIDX symbolIndex = _cell[index]._data;

            // Symbols in the current scope override globals

            if (!_environment.empty())
            {
                Scope& scope = _environment.back();

                auto iter = scope.find(symbolIndex);
                if (iter != scope.end())
                {
                    CELLID localValue = iter->second;
                    return EvaluateCell(localValue);
                }
            }

            const SymbolInfo& symbol = _symbol[symbolIndex];
            assert(symbol._symbolCell == index);

            RAISE_ERROR_IF(symbol._type == SYMBOL_INVALID, ERROR_RUNTIME_VARIABLE_UNBOUND, symbol._ident.c_str());

            switch (symbol._type)
            {
                case SYMBOL_RESERVED:
                    return index;

                case SYMBOL_PRIMITIVE:
                    return index;

                case SYMBOL_VARIABLE:
                    return symbol._valueCell;

                case SYMBOL_FUNCTION:
                    lambdaCell = symbol._valueCell;
                    break;

                case SYMBOL_MACRO:
                    // FIXME: macros should be expanded at compile time
                    lambdaCell = symbol._valueCell;
                    break;

                default:
                    assert(!"Invalid symbol type");
            }

            break;
        }

        case TYPE_CONS:
        {
            CELLID head = _cell[index]._data;
            assert(VALID_CELL(head));

            if (head == _quote)
            {
                // Special form: quote

                CELLID quoted = _cell[index]._next;
                RAISE_ERROR_IF(!VALID_CELL(quoted), ERROR_RUNTIME_WRONG_NUM_PARAMS);
                RAISE_ERROR_IF(VALID_CELL(_cell[quoted]._next), ERROR_RUNTIME_WRONG_NUM_PARAMS);

                return _cell[quoted]._data;
            }

            if (head == _quasiquote)
            {
                // Special form: quasiquote

                //CELLID quasiquoted = _cell[index]._next;
                //RAISE_ERROR_IF(!VALID_CELL(quasiquoted), ERROR_RUNTIME_WRONG_NUM_PARAMS);

                //DumpCellGraph(_cell[quasiquoted]._data, sExpandSymbols);
                return ExpandQuasiquoted(index);
            }

            RAISE_ERROR_IF(head == _unquote, ERROR_RUNTIME_INVALID_MACRO_EXPANSION, "you can't unquote what isn't quoted");

            if (_cell[head]._type == TYPE_SYMBOL)
            {
                SYMBOLIDX headSymbolIndex = _cell[head]._data;
                const SymbolInfo& headSymbol = _symbol[headSymbolIndex];

                if (headSymbol._type == SYMBOL_PRIMITIVE)
                {
                    primitiveIndex = headSymbol._primIndex;

                    if (headSymbol._flags & SYMBOLFLAG_DONT_EVAL_ARGS)
                        evaluateArguments = false;
                }
                else if (headSymbol._type == SYMBOL_FUNCTION)
                {
                    lambdaCell = headSymbol._valueCell;
                }
                else if (headSymbol._type == SYMBOL_MACRO)
                {
                    lambdaCell = headSymbol._valueCell;
                    isMacro = true;
                }
            }
            else if (_cell[head]._type == TYPE_CONS)
            {
                lambdaCell = EvaluateCell(head);
            }

            RAISE_ERROR_IF(!VALID_CELL(lambdaCell) && !primitiveIndex, ERROR_RUNTIME_UNDEFINED_FUNCTION, "the first list element must be a function");
            break;
        }

        case TYPE_INT:
        case TYPE_FLOAT:
        case TYPE_STRING:
        case TYPE_LAMBDA:
            return index;

        default:
            assert(!"Invalid cell type");
    }

    // At this point we [should] have something callable

    if (VALID_INDEX(primitiveIndex))
    {
        CELLID argCellIndex = _cell[index]._next;
        CELLID primResult = CallPrimitive(primitiveIndex, argCellIndex, evaluateArguments);
        return primResult;
    }

    assert(lambdaCell != _nil);

    CELLID bindingListIndex = _cell[lambdaCell]._data;
    CELLID bodyCellIndex = _cell[lambdaCell]._next;

    // Bind the arguments

    RAISE_ERROR_IF(_cell[bindingListIndex]._type != TYPE_CONS, ERROR_RUNTIME_INVALID_ARGUMENT);

    CELLID argList = _cell[index]._next;
    RAISE_ERROR_IF(_cell[argList]._type != TYPE_CONS, ERROR_RUNTIME_INVALID_ARGUMENT);

    Scope callScope = BindArguments(bindingListIndex, argList, evaluateArguments);

    // Evaluate the function body

    CELLID callResult = _nil;
    _environment.push_back(std::move(callScope));

    try
    {
        callResult = EvaluateCell(bodyCellIndex);

        if (isMacro)
            callResult = EvaluateCell(callResult);
    }
    catch (...)
    {
        _environment.pop_back();
        throw;
    }

    _environment.pop_back();
    return callResult;
}

Runtime::Scope Runtime::BindArguments(CELLID bindingList, CELLID argList, bool evaluateArgs)
{
    Scope scope;

    while (VALID_CELL(argList))
    {
        RAISE_ERROR_IF(!VALID_CELL(bindingList), ERROR_RUNTIME_WRONG_NUM_PARAMS);

        CELLID boundSymbolCell = _cell[bindingList]._data;
        SYMBOLIDX boundSymbolIndex = _cell[boundSymbolCell]._data;

        CELLID value = _cell[argList]._data;
        if (evaluateArgs)
            value = EvaluateCell(value);

        scope[boundSymbolIndex] = value;

        argList = _cell[argList]._next;
        bindingList = _cell[bindingList]._next;
    }

    return scope;
}

CELLID Runtime::CallPrimitive(TINDEX primIndex, CELLID argCellIndex, bool evaluateArgs)
{
    PrimitiveInfo& prim = _primitive[primIndex];
    CELLVEC primArgs = ExtractList(argCellIndex);

    if (evaluateArgs)
    {
        for (size_t i = 0; i < primArgs.size(); i++)
        {
            CELLID value = primArgs[i];//_cell[primArgs[i]]._data;
            value = EvaluateCell(value);

            primArgs[i] = value;
        }
    }

    CELLID primResult = (*this.*prim._func)(primArgs);
    return primResult;
}


