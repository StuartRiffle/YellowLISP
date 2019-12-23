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
    if (cell._next)
        return cell._next;

    return _nil;
}

CELL_INDEX Runtime::COND(const ArgumentList& args)
{
    (args);
    RAISE_ERROR(ERROR_RUNTIME_NOT_IMPLEMENTED);
    return 0;
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

CELL_INDEX Runtime::EQ(const ArgumentList& args)
{
    VERIFY_NUM_PARAMETERS(args.size(), 2, "EQ");

    CELL_INDEX a = args[0];
    CELL_INDEX b = args[1];

    if (a == b)
        return _true;

    const Cell& ca = _cell[a];
    const Cell& cb = _cell[b];

    if (ca._type == cb._type)
    {
        if (ca._tags & cb._tags & TAG_EMBEDDED)
            if (ca._data == cb._data)
                return _true;

        if (ca._type == TYPE_STRING)
        {
            string vala = LoadStringLiteral(a);
            string valb = LoadStringLiteral(b);

            if (vala == valb)
                return _true;
        }
    }

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

CELL_INDEX Runtime::PRINT(const ArgumentList& args)
{
    (args);
    RAISE_ERROR(ERROR_RUNTIME_NOT_IMPLEMENTED);
    return 0;
}

CELL_INDEX Runtime::EVAL(const ArgumentList& args)
{
    VERIFY_NUM_PARAMETERS(args.size(), 1, "EVAL");

    CELL_INDEX cellIndex = args[0];
    return EvaluateCell(cellIndex);
}

