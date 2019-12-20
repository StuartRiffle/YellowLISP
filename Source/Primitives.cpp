#include "Yellow.h"
#include "Runtime.h"


CELL_INDEX Runtime::ATOM(const ArgumentList& args)
{
    assert(args.size() == 1);
    CELL_INDEX index = args[0];

    const Cell& cell = _cell[index];
    return cell._next ? _true : _nil;
}

CELL_INDEX Runtime::CAR(const ArgumentList& args)
{
    assert(args.size() == 1);
    CELL_INDEX index = args[0];

    const Cell& cell = _cell[index];
    if (cell._type != TYPE_LIST)
        return _nil;

    return cell._data;
}

CELL_INDEX Runtime::CDR(const ArgumentList& args)
{
    assert(args.size() == 1);
    CELL_INDEX index = args[0];

    const Cell& cell = _cell[index];
    if (cell._next)
        return cell._next;

    return _nil;
}

CELL_INDEX Runtime::COND(const ArgumentList& args)
{
    assert(!"Not implemented");
    return 0;
}

CELL_INDEX Runtime::CONS(const ArgumentList& args)
{
    assert(args.size() == 2);
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
    assert(args.size() == 2);
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
            const char* vala = LoadStringLiteral(a);
            const char* valb = LoadStringLiteral(b);

            if (!strcmp(vala, valb))
                return _true;
        }
    }

    return _nil;
}

CELL_INDEX Runtime::PRINT(const ArgumentList& args)
{
    assert(!"Not implemented");
    return 0;
}

CELL_INDEX Runtime::EVAL(const ArgumentList& args)
{
    assert(args.size() == 1);
    CELL_INDEX cellIndex = args[0];

    return EvaluateCell(cellIndex);
}

CELL_INDEX Runtime::QUOTE(const ArgumentList& args)
{
    assert(args.size() == 1);
    return args[0];
}


