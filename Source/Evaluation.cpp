// YellowLISP (c) 2020 Stuart Riffle (MIT license)

#include "Yellow.h"
#include "Runtime.h"
#include "Testing.h"

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
            RETURN_WITH_COVERAGE(EvaluateCell(index));

        RETURN_WITH_COVERAGE(index);
    }

    if (elements[0] == _quote)
    {
        RAISE_ERROR_IF(elements.size() != 2, ERROR_RUNTIME_WRONG_NUM_PARAMS, "quote");

        CELLID quoted = elements[1];

        if (level == 0)
            RETURN_WITH_COVERAGE(EvaluateCell(quoted));

        CELLID expanded = ExpandQuasiquoted(elements[1], level);
        CELLID requoted = GenerateList({ _quote, expanded });

        RETURN_WITH_COVERAGE(requoted);
    }

    if (elements[0] == _unquote)
    {
        RAISE_ERROR_IF(elements.size() != 2, ERROR_RUNTIME_WRONG_NUM_PARAMS, "unquote");
        //RAISE_ERROR_IF(level < 1, ERROR_RUNTIME_INVALID_MACRO_EXPANSION, "you can't unquote what isn't quoted");

        CELLID unquoted = elements[1];

        if (level == 1)
            RETURN_WITH_COVERAGE(EvaluateCell(unquoted));

        CELLID expanded = ExpandQuasiquoted(unquoted, level - 1);
        CELLID reunquoted = GenerateList({ _unquote, expanded });

        RETURN_WITH_COVERAGE(reunquoted);
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
            RETURN_WITH_COVERAGE(requoted);
        }

        RETURN_WITH_COVERAGE(expanded);
    }

    for (int i = 0; i < elements.size(); i++)
        elements[i] = ExpandQuasiquoted(elements[i], level);

    RETURN_WITH_COVERAGE(GenerateList(elements));
}

CELLID Runtime::EvaluateCell(CELLID index)
{
    assert(index.IsValid());
    if (index == _nil)
        return _nil;

#if DEBUG_BUILD
    // For debugging, this generates a graph of cell connections for GraphViz to render
    static int sDumpDebugGraph = 1;
    static int sExpandSymbols = 1;
    if (sDumpDebugGraph)
        DumpCellGraph(index, sExpandSymbols);
#endif

    switch (_cell[index]._type)
    {
        case TYPE_SYMBOL:
        {
            SYMBOLIDX symbolIndex = _cell[index]._data;
            CELLID scopeOverride = GetScopeOverride(symbolIndex);

            if (scopeOverride.IsValid())
                RETURN_WITH_COVERAGE(scopeOverride);

            const SymbolInfo& symbol = _symbol[symbolIndex];
            assert(symbol._symbolCell == index);

            RAISE_ERROR_IF(symbol._type == SYMBOL_INVALID, ERROR_RUNTIME_VARIABLE_UNBOUND, symbol._ident.c_str());

            switch (symbol._type)
            {
                case SYMBOL_RESERVED:   RETURN_WITH_COVERAGE(index);
                case SYMBOL_PRIMITIVE:  RETURN_WITH_COVERAGE(index);
                case SYMBOL_VARIABLE:   RETURN_WITH_COVERAGE(symbol._valueCell);
                case SYMBOL_FUNCTION:   RETURN_WITH_COVERAGE(symbol._valueCell);

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

                RETURN_WITH_COVERAGE(_cell[quoted]._data);
            }

            if (head == _quasiquote)
            {
                // Special form: quasiquote

                RETURN_WITH_COVERAGE(ExpandQuasiquoted(index));
            }

            RAISE_ERROR_IF(head == _unquote, ERROR_RUNTIME_INVALID_ARGUMENT, "you can't unquote what isn't quoted");

            CallTarget callTarget;

            if (_cell[head]._type == TYPE_SYMBOL)
            {
                TEST_COVERAGE;
                SYMBOLIDX headSymbolIndex = _cell[head]._data;
                const SymbolInfo& headSymbol = _symbol[headSymbolIndex];

                if (headSymbol._type == SYMBOL_PRIMITIVE)
                {
                    TEST_COVERAGE;
                    callTarget._primitiveIndex = headSymbol._primIndex;

                    if (headSymbol._flags & SYMBOLFLAG_DONT_EVAL_ARGS)
                    {
                        TEST_COVERAGE;
                        callTarget._evaluateArgs = false;
                    }
                }
                else if (headSymbol._type == SYMBOL_FUNCTION)
                {
                    TEST_COVERAGE;
                    callTarget._lambdaCell = headSymbol._valueCell;
                }
            }
            else if (_cell[head]._type == TYPE_CONS)
            {
                TEST_COVERAGE;
                callTarget._lambdaCell = EvaluateCell(head);
            }

            RAISE_ERROR_IF(!callTarget.IsValid(), ERROR_RUNTIME_UNDEFINED_FUNCTION, "the first list element must be a function");

            CELLID argList = _cell[index]._next;
            RAISE_ERROR_IF((argList != _nil) && _cell[argList]._type != TYPE_CONS, ERROR_RUNTIME_INVALID_ARGUMENT);

            // Down the rabbit hole we go

            CELLID callResult = ApplyFunction(callTarget, argList);

            RETURN_WITH_COVERAGE(callResult);
        }

        case TYPE_INT:      RETURN_WITH_COVERAGE(index);
        case TYPE_FLOAT:    RETURN_WITH_COVERAGE(index);
        case TYPE_STRING:   RETURN_WITH_COVERAGE(index);
        case TYPE_LAMBDA:   RETURN_WITH_COVERAGE(index);

        default:
            assert(!"Invalid cell type");
    }

    // Should never get here

    RAISE_ERROR(ERROR_INTERNAL_RUNTIME_FAILURE, "EvaluateCell failed to do so");
    return CELLID(); // prevent compiler warning
}

void Runtime::BindScopeMappings(CELLID bindingList, CELLID valueList, bool evaluate, Scope* destScope)
{
    while (valueList != _nil)
    {
        TEST_COVERAGE;
        RAISE_ERROR_IF(bindingList == _nil, ERROR_RUNTIME_WRONG_NUM_PARAMS);

        CELLID value = _cell[valueList]._data;
        CELLID boundSymbolCell = _cell[bindingList]._data;
        SYMBOLIDX boundSymbolIndex = _cell[boundSymbolCell]._data;

        RAISE_ERROR_IF(_cell[boundSymbolCell]._type != TYPE_SYMBOL, ERROR_RUNTIME_WRONG_NUM_PARAMS);

        if (evaluate)
        {
            TEST_COVERAGE;
            value = EvaluateCell(value);
        }

        if (value != _symbol[boundSymbolIndex]._symbolCell)
            destScope->insert_or_assign(boundSymbolIndex, value);

        valueList   = _cell[valueList]._next;
        bindingList = _cell[bindingList]._next;
    }

    RAISE_ERROR_IF(bindingList != _nil, ERROR_RUNTIME_WRONG_NUM_PARAMS);
}

CELLID Runtime::GetScopeOverride(SYMBOLIDX symbolIndex)
{
    CELLID result;

    for (auto iterScope = _environment.rbegin(); iterScope != _environment.rend(); ++iterScope)
    {
        Scope* scope = *iterScope;
        assert(scope);

        if (!scope->empty())
        {
            TEST_COVERAGE;
            auto iterSymbol = scope->find(symbolIndex);
            if (iterSymbol != scope->end())
            {
                TEST_COVERAGE;
                CELLID localValue = iterSymbol->second;
                result = localValue;
                break;
            }
        }
    }

    RETURN_WITH_COVERAGE(result);
}

CELLID Runtime::ApplyFunction(const CallTarget& callTarget, CELLID argList)
{
    if (callTarget._primitiveIndex.IsValid())
    {
        CELLID primResult = CallPrimitive(callTarget._primitiveIndex, argList, callTarget._evaluateArgs);
        RETURN_WITH_COVERAGE(primResult);
    }

    CELLID lambdaCell = callTarget._lambdaCell;
    RAISE_ERROR_IF((lambdaCell == _nil) || (_cell[lambdaCell]._type != TYPE_LAMBDA), ERROR_RUNTIME_UNDEFINED_FUNCTION, "not a function");

    CELLID bindingList = _cell[lambdaCell]._data;
    CELLID bodyCell = _cell[lambdaCell]._next;

    RAISE_ERROR_IF(_cell[bindingList]._type != TYPE_CONS, ERROR_RUNTIME_INVALID_ARGUMENT);
    RAISE_ERROR_IF(_cell[argList]._type != TYPE_CONS, ERROR_RUNTIME_INVALID_ARGUMENT);

    Scope callScope;
    BindScopeMappings(bindingList, argList, callTarget._evaluateArgs, &callScope);

    ScopeGuard scopeGuard(_environment, &callScope);
    CELLID result = EvaluateCell(bodyCell);

    RETURN_WITH_COVERAGE(result);
}

/*
digraph G {
    graph[rankdir = "LR"];
    node [fontname = "segoe ui semibold";shape=rectangle];
    EvaluateCell -> EvaluateCell;
    EvaluateCell -> ApplyFunction;
    ApplyFunction -> EvaluateCell;
    ApplyFunction -> CallPrimitive;
    CallPrimitive -> EvaluateCell;
    CallPrimitive -> "(primitive...)";
    "(primitive...)" -> EvaluateCell;
    "(primitive...)" -> ApplyFunction;
}
*/

CELLID Runtime::CallPrimitive(PRIMIDX primIndex, CELLID argCell, bool evaluateArgs)
{
    CELLVEC args;
    ExtractList(argCell, &args);

    PrimitiveInfo& prim = _primitive[primIndex];

    if (evaluateArgs)
    {
        TEST_COVERAGE; 
        for (int i = 0; i < args.size(); i++)
            args[i] = EvaluateCell(args[i]);
    }

    CELLID result = (*this.*prim._func)(args);
    RETURN_WITH_COVERAGE(result);
}


