// YellowLISP (c) 2019 Stuart Riffle (MIT license)

#include "Yellow.h"
#include "Runtime.h"
#include "Errors.h"

CELL_INDEX Runtime::ATOM(const ArgumentList& args)
{
    VERIFY_NUM_PARAMETERS(args.size(), 1, "ATOM");

    CELL_INDEX index = args[0];

    const Cell& cell = _cell[index];
    return (cell._type == TYPE_LIST) ? _nil : _true;
}

CELL_INDEX Runtime::CAR(const ArgumentList& args)
{
    VERIFY_NUM_PARAMETERS(args.size(), 1, "CAR");

    CELL_INDEX index = args[0];

    const Cell& cell = _cell[index];
    if (cell._type != TYPE_LIST)
        return _nil;

    return cell._data;
}

CELL_INDEX Runtime::CDR(const ArgumentList& args)
{
    VERIFY_NUM_PARAMETERS(args.size(), 1, "CDR");

    CELL_INDEX index = args[0];

    const Cell& cell = _cell[index];
    if (VALID_CELL(cell._next))
        return cell._next;

    return _nil;
}

CELL_INDEX Runtime::COND(const ArgumentList& args)
{
    for (auto arg : args)
    {
        vector<CELL_INDEX> elements = ExtractList(arg);
        RAISE_ERROR_IF(elements.size() != 2, ERROR_RUNTIME_INVALID_ARGUMENT, "Arguments to COND must be lists of two elements");

        CELL_INDEX testResult = EvaluateCell(elements[0]);
        if (testResult != _nil)
            return EvaluateCell(elements[1]);
    }

    return _nil;
}

CELL_INDEX Runtime::CONS(const ArgumentList& args)
{
    VERIFY_NUM_PARAMETERS(args.size(), 2, "CONS");
    CELL_INDEX head = args[0];
    CELL_INDEX tail = args[1];

    CELL_INDEX index = (CELL_INDEX)AllocateCell(TYPE_LIST);

    Cell& cell = _cell[index];
    cell._data = head;
    cell._next = tail;

    return index;
}

bool Runtime::TestEquality(CELL_INDEX a, CELL_INDEX b)
{
    if (a == b)
        return true;

    if (_cell[a]._type != _cell[b]._type)
        return true;

    if (_cell[a]._tags & _cell[b]._tags & TAG_EMBEDDED)
        if (_cell[a]._data == _cell[b]._data)
            return true;

    if (IS_NUMERIC_TYPE(a) && IS_NUMERIC_TYPE(b))
    {
        double aval = LoadNumericLiteral(a);
        double bval = LoadNumericLiteral(b);

        if (aval == bval)
            return true;
    }
    else if (_cell[a]._type == TYPE_STRING)
    {
        string vala = LoadStringLiteral(a);
        string valb = LoadStringLiteral(b);

        if (vala == valb)
            return true;
    }
    else if (_cell[a]._type == TYPE_LIST)
    {
        while ((a != _nil) && (b != _nil))
        {
            if (!TestEquality(_cell[a]._data, _cell[b]._data))
                return false;

            a = (CELL_INDEX) _cell[a]._next;
            b = (CELL_INDEX) _cell[b]._next;
        }

        if ((a != _nil) || (b != _nil))
            return false;

        return true;
    }
    else if (_cell[a]._type == TYPE_SYMBOL)
    {
        return TestEquality(_cell[a]._data, _cell[b]._data);
    }

    return false;
}

CELL_INDEX Runtime::EQ(const ArgumentList& args)
{
    RAISE_ERROR_IF(args.size() < 2, ERROR_RUNTIME_WRONG_NUM_PARAMS, "=");

    for (size_t i = 1; i < args.size(); i++)
        if (!TestEquality(args[0], args[i]))
            return _nil;

    return _true;
}

CELL_INDEX Runtime::LESS(const ArgumentList& args)
{
    VERIFY_NUM_PARAMETERS(args.size(), 2, "<");

    double a = LoadNumericLiteral(args[0]);
    double b = LoadNumericLiteral(args[1]);

    if (a < b)
        return _true;

    return _nil;
}

CELL_INDEX Runtime::LIST(const ArgumentList& args)
{
    if (args.empty())
        return _nil;

    vector<CELL_INDEX> listCells;
    listCells.reserve(args.size());

    for (CELL_INDEX elem : args)
    {
        CELL_INDEX elemCell = AllocateCell(TYPE_LIST);
        _cell[elemCell]._data = elem;

        if (!listCells.empty())
            _cell[listCells.back()]._next = elemCell;

        listCells.push_back(elemCell);
    }

    return listCells[0];
}

CELL_INDEX Runtime::SETQ(const ArgumentList& args)
{
    RAISE_ERROR_IF(args.empty(), ERROR_RUNTIME_WRONG_NUM_PARAMS, "SETQ needs at least 2 arguments");
    RAISE_ERROR_IF(args.size() & 1, ERROR_RUNTIME_WRONG_NUM_PARAMS, "SETQ needs an even number of arguments");

    CELL_INDEX valueCell = _nil;

    for (size_t argIdx = 0; argIdx < args.size(); argIdx += 2)
    {
        CELL_INDEX symbolCell = args[argIdx];
        valueCell = EvaluateCell(args[argIdx + 1]);

        if (_cell[symbolCell]._type != TYPE_SYMBOL)
            RAISE_ERROR(ERROR_RUNTIME_TYPE_MISMATCH, "symbol expected");

        SYMBOL_INDEX symbolIndex = _cell[symbolCell]._data;
        SymbolInfo& symbol = _symbol[symbolIndex];
        assert(symbol._symbolCell == symbolCell);

        if (symbol._primIndex)
            RAISE_ERROR(ERROR_RUNTIME_RESERVED_SYMBOL, symbol._ident.c_str());

        symbol._type = SYMBOL_VARIABLE;
        symbol._valueCell = valueCell;
    }


    return valueCell;
}

CELL_INDEX Runtime::PROGN(const ArgumentList& args)
{
    CELL_INDEX result = _nil;

    for (size_t i = 0; i < args.size(); i++)
        result = EvaluateCell(args[i]);

    return result;
}

CELL_INDEX Runtime::EVAL(const ArgumentList& args)
{
    VERIFY_NUM_PARAMETERS(args.size(), 1, "EVAL");

    CELL_INDEX cellIndex = args[0];
    return EvaluateCell(cellIndex);
}

CELL_INDEX Runtime::DEFMACRO(const ArgumentList& args)
{
    RAISE_ERROR_IF(args.size() < 3, ERROR_RUNTIME_WRONG_NUM_PARAMS, "DEFMACRO");

    CELL_INDEX symbolCell = args[0];
    RAISE_ERROR_IF(_cell[symbolCell]._type != TYPE_SYMBOL, ERROR_RUNTIME_TYPE_MISMATCH, "macro name");

    SYMBOL_INDEX symbolIndex = _cell[symbolCell]._data;
    SymbolInfo& macroSymbol = _symbol[symbolIndex];

    CELL_INDEX bindingListCell = args[1];
    RAISE_ERROR_IF((bindingListCell != _nil) && (_cell[bindingListCell]._type != TYPE_LIST), ERROR_RUNTIME_TYPE_MISMATCH, "macro arguments");
    macroSymbol._macroBindings = bindingListCell;

    assert(macroSymbol._symbolCell == symbolCell);

    int onArg = 2;

    CELL_INDEX commentCell = args[onArg];
    if (_cell[commentCell]._type == TYPE_STRING)
    {
        macroSymbol._comment = LoadStringLiteral(commentCell);
        onArg++;
    }

    RAISE_ERROR_IF(args.size() < onArg, ERROR_RUNTIME_WRONG_NUM_PARAMS, "DEFMACRO");
    CELL_INDEX bodyCell = args[onArg];

    RAISE_ERROR_IF((bodyCell != _nil) && (_cell[bodyCell]._type != TYPE_LIST), ERROR_RUNTIME_TYPE_MISMATCH, "macro body");

    macroSymbol._type = SYMBOL_MACRO;
    macroSymbol._valueCell = bodyCell;

    return symbolCell;
}

CELL_INDEX Runtime::DEFUN(const ArgumentList& args)
{
    RAISE_ERROR_IF(args.size() < 3, ERROR_RUNTIME_WRONG_NUM_PARAMS, "DEFUN");

    CELL_INDEX symbolCell = args[0];
    RAISE_ERROR_IF(_cell[symbolCell]._type != TYPE_SYMBOL, ERROR_RUNTIME_TYPE_MISMATCH, "function name");

    CELL_INDEX bindingListCell  = args[1];
    RAISE_ERROR_IF((bindingListCell != _nil) && (_cell[bindingListCell]._type != TYPE_LIST), ERROR_RUNTIME_TYPE_MISMATCH, "function arguments");

    CELL_INDEX functionBodyCell = args[2];
    RAISE_ERROR_IF((functionBodyCell != _nil) && (_cell[functionBodyCell]._type != TYPE_LIST), ERROR_RUNTIME_TYPE_MISMATCH, "function body");

    ArgumentList lambdaArgs = { bindingListCell, functionBodyCell };
    CELL_INDEX lambdaCell = LAMBDA(lambdaArgs);

    SYMBOL_INDEX symbolIndex = _cell[symbolCell]._data;
    SymbolInfo&  functionSymbol = _symbol[symbolIndex];

    functionSymbol._type = SYMBOL_FUNCTION;
    functionSymbol._valueCell = lambdaCell;

    return symbolCell;
}

CELL_INDEX Runtime::LAMBDA(const ArgumentList& args)
{
    RAISE_ERROR_IF(args.size() != 2, ERROR_RUNTIME_WRONG_NUM_PARAMS, "LAMBDA");

    CELL_INDEX bindingListCell = args[0];
    RAISE_ERROR_IF((bindingListCell != _nil) && (_cell[bindingListCell]._type != TYPE_LIST), ERROR_RUNTIME_TYPE_MISMATCH, "lambda binding list");

    CELL_INDEX functionBodyCell = args[1];
    RAISE_ERROR_IF((functionBodyCell != _nil) && (_cell[functionBodyCell]._type != TYPE_LIST), ERROR_RUNTIME_TYPE_MISMATCH, "lambda function body");

    CELL_INDEX lambdaCell = AllocateCell(TYPE_LAMBDA);
    _cell[lambdaCell]._data = bindingListCell;
    _cell[lambdaCell]._next = functionBodyCell;

    return lambdaCell;
}

