// YellowLISP (c) 2020 Stuart Riffle (MIT license)

#include "Yellow.h"
#include "Runtime.h"


size_t Runtime::ExtractList(CELLID index, CELLVEC* dest)
{
    while ((index != _nil) && (_cell[index]._type == TYPE_CONS))
    {
        dest->push_back(_cell[index]._data);
        index = _cell[index]._next;
    }

    return dest->size();
}

CELLID Runtime::GenerateList(const CELLVEC& elements)
{
    CELLID head = _nil;

    // FIXME: improper lists?

    int idx = elements.size() - 1;
    for (int i = 0; i < elements.size(); i++)
    {
        CELLID elem = elements[idx--];
        CELLID elemLink = AllocateCell(TYPE_CONS);

        _cell[elemLink]._data = elem;
        _cell[elemLink]._next = head;

        head = elemLink;
    }

    return head;
}


CELLID Runtime::ExpandQuasiquoted(CELLID index, int level)
{
    CELLVEC elements;
    ExtractList(index, &elements);

    if (elements.empty())
    {
        if (level == 0)
            return EvaluateCell(index);

        return index;
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
        //RAISE_ERROR_IF(level < 1, ERROR_RUNTIME_INVALID_MACRO_EXPANSION, "you can't unquote what isn't quoted");

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

    for (int i = 0; i < elements.size(); i++)
        elements[i] = ExpandQuasiquoted(elements[i], level);

    return GenerateList(elements);
}

CELLID Runtime::EvaluateCell(CELLID index)
{
    assert(index.IsValid());
    if (index == _nil)
        return _nil;

#if 1
    static int sDumpDebugGraph = 1;
    static int sExpandSymbols = 1;
    // For debugging, this generates a graph of cell connections for GraphViz to render
    if (index == 55)
        printf("");
    if (sDumpDebugGraph)
        DumpCellGraph(index, sExpandSymbols);
#endif

    CELLID lambdaCell = _nil;
    PRIMIDX primitiveIndex;

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
            assert(head.IsValid());

            if (head == _quote)
            {
                // Special form: quote

                CELLID quoted = _cell[index]._next;
                RAISE_ERROR_IF(quoted == _nil, ERROR_RUNTIME_WRONG_NUM_PARAMS);
                RAISE_ERROR_IF(_cell[quoted]._next != _nil, ERROR_RUNTIME_WRONG_NUM_PARAMS);

                return _cell[quoted]._data;
            }

            if (head == _quasiquote)
            {
                // Special form: quasiquote

                return ExpandQuasiquoted(index);
            }

//            RAISE_ERROR_IF(head == _unquote, ERROR_RUNTIME_INVALID_MACRO_EXPANSION, "you can't unquote what isn't quoted");

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

            RAISE_ERROR_IF((lambdaCell == _nil) && !primitiveIndex.IsValid(), ERROR_RUNTIME_UNDEFINED_FUNCTION, "the first list element must be a function");
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

    if (primitiveIndex.IsValid())
    {
        CELLID argCell    = _cell[index]._next;
        CELLID primResult = CallPrimitive(primitiveIndex, argCell, evaluateArguments);
        return primResult;
    }

    assert(lambdaCell != _nil);

    CELLID bindingListIndex = _cell[lambdaCell]._data;
    CELLID bodyCell = _cell[lambdaCell]._next;

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
        callResult = EvaluateCell(bodyCell);

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

    while (argList != _nil)
    {
        RAISE_ERROR_IF(bindingList == _nil, ERROR_RUNTIME_WRONG_NUM_PARAMS);

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

CELLID Runtime::CallPrimitive(PRIMIDX primIndex, CELLID argCell, bool evaluateArgs)
{
    CELLVEC args;
    ExtractList(argCell, &args);

    PrimitiveInfo& prim = _primitive[primIndex];

    if (evaluateArgs)
        for (int i = 0; i < args.size(); i++)
        {
            CELLID before = args[i];
            args[i] = EvaluateCell(args[i]);
            if (!args[i].IsValid())
                printf("");
        }

    CELLID result = (*this.*prim._func)(args);
    return result;
}


