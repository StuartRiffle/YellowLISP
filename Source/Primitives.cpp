// YellowLISP (c) 2020 Stuart Riffle (MIT license)

#include "Yellow.h"
#include "Runtime.h"
#include "Errors.h"
#include "Utility.h"

CELLID Runtime::ATOM(const CELLVEC& args)
{
    VERIFY_NUM_PARAMETERS(args.size(), 1, "ATOM");

    CELLID index = args[0];
    if (index == _nil)
        return _true;

    const Cell& cell = _cell[index];
    if (cell._type == TYPE_CONS)
        return _nil;

    return _true;
}

CELLID Runtime::CAR(const CELLVEC& args)
{
    VERIFY_NUM_PARAMETERS(args.size(), 1, "CAR");

    if (args[0] == _nil)
        return _nil;

    const Cell& cell = _cell[args[0]];
    if (cell._type != TYPE_CONS)
        return _nil;

    return cell._data;
}

CELLID Runtime::CDR(const CELLVEC& args)
{
    VERIFY_NUM_PARAMETERS(args.size(), 1, "CDR");

    if (args[0] == _nil)
        return _nil;

    const Cell& cell = _cell[args[0]];
    if (cell._next != _nil)
        return cell._next;

    return _nil;
}

CELLID Runtime::COND(const CELLVEC& args)
{
    for (int i = 0; i < args.size(); i++)
    {
        CELLVEC elements;
        ExtractList(args[i], &elements);

        RAISE_ERROR_IF(elements.size() != 2, ERROR_RUNTIME_INVALID_ARGUMENT, "Arguments to COND must be lists of two elements");

        CELLID testResult = EvaluateCell(elements[0]);
        if (testResult != _nil)
            return EvaluateCell(elements[1]);
    }

    return _nil;
}

CELLID Runtime::CONS(const CELLVEC& args)
{
    RAISE_ERROR_IF(args.size() < 2, ERROR_RUNTIME_WRONG_NUM_PARAMS, "CONS");

    CELLID head = EvaluateCell(args[0]);
    CELLID tail = EvaluateCell(args[1]);

    // HACK: special case the stupid fucking dot notation

    if (tail == _dot)
    {
        RAISE_ERROR_IF(args.size() != 3, ERROR_RUNTIME_WRONG_NUM_PARAMS, "CONS");

        tail = args[2];
        if (tail != _nil)
            RAISE_ERROR_IF(_cell[tail]._type != TYPE_CONS, ERROR_RUNTIME_INVALID_ARGUMENT, "a list must follow the dot");

        tail = _cell[tail]._data;
    }

    CELLID index = AllocateCell(TYPE_CONS);
    _cell[index]._data = head;
    _cell[index]._next = tail;

    return index;
}

CELLID Runtime::EQLOP(const CELLVEC& args)
{
    RAISE_ERROR_IF(args.size() < 1, ERROR_RUNTIME_WRONG_NUM_PARAMS, "=");

    // Numerical equality (regardless of type!)

    for (int i = 0; i < args.size(); i++)
        RAISE_ERROR_IF(!IS_NUMERIC_TYPE(args[i]), ERROR_RUNTIME_INVALID_ARGUMENT, "the operator '=' only works for numeric types");

    double first = LoadNumericLiteral(args[0]);
    for (int i = 1; i < args.size(); i++)
        if (LoadNumericLiteral(args[i]) != first)
            return _nil;

    return _true;
}

CELLID Runtime::EQ(const CELLVEC& args)
{
    RAISE_ERROR_IF(args.size() < 1, ERROR_RUNTIME_WRONG_NUM_PARAMS, "EQ");

    // EQ is only true if the arguments all refer to the same cell internally

    for (int i = 1; i < args.size(); i++)
        if (args[i] != args[0])
            return _nil;

    return _true;
}

bool Runtime::TestCellsEQL(CELLID a, CELLID b, bool strict)
{
    // EQL is like EQ, but also allows numbers (of the same type) and identical strings

    if (a == b)
        return true;

    if (!strict)
        if (IS_NUMERIC_TYPE(a) && (IS_NUMERIC_TYPE(b)))
            if (LoadNumericLiteral(a) == LoadNumericLiteral(b))
                return true;

    if (_cell[a]._type != _cell[b]._type)
        return false;

    if (IS_NUMERIC_TYPE(a))
        if (_cell[a]._data == _cell[b]._data)
            return true;

    if (_cell[a]._type == TYPE_STRING)
    {
        string astr = LoadStringLiteral(a);
        string bstr = LoadStringLiteral(b);

        if (!strict)
        {
            astr = Uppercase(astr);
            bstr = Uppercase(bstr);
        }

        if (astr == bstr)
            return true;
    }

    return false;
}

bool Runtime::TestStructureEQUAL(CELLID a, CELLID b, bool strict)
{
    // EQUAL is like EQL, but defined recursively for lists

    if (a == b)
        return true;

    if (strict)
        if (_cell[a]._type != _cell[b]._type)
            return false;

    if ((_cell[a]._type == TYPE_CONS) && (_cell[b]._type == TYPE_CONS))
    {
        while ((a != _nil) && (b != _nil))
        {
            if (!TestStructureEQUAL(_cell[a]._data, _cell[b]._data, strict))
                return false;

            a = _cell[a]._next;
            b = _cell[b]._next;

            assert(a.IsValid() && b.IsValid());
        }

        if ((a != _nil) || (b != _nil))
            return false;

        return true;
    }

    if (!TestCellsEQL(a, b, strict))
        return false;

    return true;
}


CELLID Runtime::EQL(const CELLVEC& args)
{
    RAISE_ERROR_IF(args.size() < 1, ERROR_RUNTIME_WRONG_NUM_PARAMS, "EQL");

    for (int i = 1; i < args.size(); i++)
        if (!TestCellsEQL(args[0], args[i], true))
            return _nil;

    return _true;
}

CELLID Runtime::EQUAL(const CELLVEC& args)
{
    RAISE_ERROR_IF(args.size() < 1, ERROR_RUNTIME_WRONG_NUM_PARAMS, "EQUAL");

    for (int i = 1; i < args.size(); i++)
        if (!TestStructureEQUAL(args[0], args[i], true))
            return _nil;

    return _true;
}

CELLID Runtime::EQUALP(const CELLVEC& args)
{
    RAISE_ERROR_IF(args.size() < 1, ERROR_RUNTIME_WRONG_NUM_PARAMS, "EQUALP");

    // The same as EQUAL, but numbers can be different types, and string
    // comparison is not case sensitive

    for (int i = 1; i < args.size(); i++)
        if (!TestStructureEQUAL(args[0], args[i], false))
            return _nil;

    return _true;
}

CELLID Runtime::LESS(const CELLVEC& args)
{
    VERIFY_NUM_PARAMETERS(args.size(), 2, "<");

    double a = LoadNumericLiteral(args[0]);
    double b = LoadNumericLiteral(args[1]);

    if (a < b)
        return _true;

    return _nil;
}

CELLID Runtime::LIST(const CELLVEC& args)
{
    if (args.empty())
        return _nil;

    vector<CELLID> listCells;
    listCells.reserve(args.size());

    for (int i = 0; i < args.size(); i++)
    {
        CELLID elem = args[i];
        CELLID elemCell = _nil;

        if (elem == _dot)
        {
            // HACK: if there is a dot, it must be the second-to-last element,
            // and followed by a list

            RAISE_ERROR_IF(i < 1, ERROR_RUNTIME_INVALID_ARGUMENT, "a list cannot start with a dot");

            // Skip the dot

            i++;

            RAISE_ERROR_IF(i != (args.size() - 1), ERROR_RUNTIME_INVALID_ARGUMENT, "the dot must go before the last argument");

            elem = args[i];
            if (elem == _nil)
                break;

            RAISE_ERROR_IF(_cell[elem]._type != TYPE_CONS, ERROR_RUNTIME_INVALID_ARGUMENT, "the dot must be followed by a list");

            // Evaluate the elements of the tail list (recursively, because it might
            // have a dot as well)

            CELLVEC tailList;
            ExtractList(elem, &tailList);

            elemCell = LIST(tailList);
        }
        else
        {
            elemCell = AllocateCell(TYPE_CONS);
            CELLID value = EvaluateCell(elem);
            _cell[elemCell]._data = value;
        }

        if (!listCells.empty())
            _cell[listCells.back()]._next = elemCell;

        listCells.push_back(elemCell);
    }

    return listCells[0];
}

CELLID Runtime::SETQ(const CELLVEC& args)
{
    RAISE_ERROR_IF(args.empty(), ERROR_RUNTIME_WRONG_NUM_PARAMS, "SETQ needs at least 2 arguments");
    RAISE_ERROR_IF(args.size() & 1, ERROR_RUNTIME_WRONG_NUM_PARAMS, "SETQ needs an even number of arguments");

    CELLID valueCell = _nil;

    for (int argIdx = 0; argIdx < args.size(); argIdx += 2)
    {
        CELLID symbolCell = args[argIdx];
        valueCell = EvaluateCell(args[argIdx + 1]);

        if (_cell[symbolCell]._type != TYPE_SYMBOL)
            RAISE_ERROR(ERROR_RUNTIME_TYPE_MISMATCH, "symbol expected");

        SYMBOLIDX symbolIndex = _cell[symbolCell]._data;
        SymbolInfo& symbol = _symbol[symbolIndex];
        assert(symbol._symbolCell == symbolCell);

        if (symbol._primIndex.IsValid())
            RAISE_ERROR(ERROR_RUNTIME_RESERVED_SYMBOL, symbol._ident.c_str());

        symbol._type = SYMBOL_VARIABLE;
        symbol._valueCell = valueCell;
    }


    return valueCell;
}

CELLID Runtime::PROGN(const CELLVEC& args)
{
    CELLID result = _nil;

    for (int i = 0; i < args.size(); i++)
        result = EvaluateCell(args[i]);

    return result;
}

CELLID Runtime::EVAL(const CELLVEC& args)
{
    VERIFY_NUM_PARAMETERS(args.size(), 1, "EVAL");

    CELLID cellIndex = args[0];
    return EvaluateCell(cellIndex);
}

CELLID Runtime::DEFUN(const CELLVEC& args)
{
    RAISE_ERROR_IF(args.size() < 3, ERROR_RUNTIME_WRONG_NUM_PARAMS, "DEFUN");

    CELLID symbolCell = args[0];
    RAISE_ERROR_IF(_cell[symbolCell]._type != TYPE_SYMBOL, ERROR_RUNTIME_TYPE_MISMATCH, "function name");

    CELLID bindingListCell  = args[1];
    RAISE_ERROR_IF((bindingListCell != _nil) && (_cell[bindingListCell]._type != TYPE_CONS), ERROR_RUNTIME_TYPE_MISMATCH, "function arguments");

    CELLID functionBodyCell = args[2];
    RAISE_ERROR_IF((functionBodyCell != _nil) && (_cell[functionBodyCell]._type != TYPE_CONS), ERROR_RUNTIME_TYPE_MISMATCH, "function body");

    CELLVEC lambdaArgs = { bindingListCell, functionBodyCell };
    CELLID lambdaCell = LAMBDA(lambdaArgs);

    SYMBOLIDX symbolIndex = _cell[symbolCell]._data;
    SymbolInfo&  functionSymbol = _symbol[symbolIndex];

    functionSymbol._type = SYMBOL_FUNCTION;
    functionSymbol._valueCell = lambdaCell;

    return symbolCell;
}

CELLID Runtime::LAMBDA(const CELLVEC& args)
{
    RAISE_ERROR_IF(args.size() != 2, ERROR_RUNTIME_WRONG_NUM_PARAMS, "LAMBDA");

    CELLID bindingListCell = args[0];
    RAISE_ERROR_IF((bindingListCell != _nil) && (_cell[bindingListCell]._type != TYPE_CONS), ERROR_RUNTIME_TYPE_MISMATCH, "lambda binding list");

    CELLID functionBodyCell = args[1];
    RAISE_ERROR_IF((functionBodyCell != _nil) && (_cell[functionBodyCell]._type != TYPE_CONS), ERROR_RUNTIME_TYPE_MISMATCH, "lambda function body");

    CELLID lambdaCell = AllocateCell(TYPE_LAMBDA);
    _cell[lambdaCell]._data = bindingListCell;
    _cell[lambdaCell]._next = functionBodyCell;

    return lambdaCell;
}

