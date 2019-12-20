#include "Yellow.h"
#include "Runtime.h"


CELL_INDEX Runtime::Atom(const ArgumentList& args)
{
    assert(args.size() == 1);
    CELL_INDEX index = args[0];

    const Cell& cell = _cell[index];
    return cell._next ? _true : _nil;
}

CELL_INDEX Runtime::Car(const ArgumentList& args)
{
    assert(args.size() == 1);
    CELL_INDEX index = args[0];

    const Cell& cell = _cell[index];
    if (cell._type != TYPE_CELL_REF)
        return _nil;

    return cell._data;
}

CELL_INDEX Runtime::Cdr(const ArgumentList& args)
{
    assert(args.size() == 1);
    CELL_INDEX index = args[0];

    const Cell& cell = _cell[index];
    if (cell._next)
        return cell._next;

    return _nil;
}

CELL_INDEX Runtime::Cons(const ArgumentList& args)
{
    assert(args.size() == 2);
    CELL_INDEX head = args[0];
    CELL_INDEX tail = args[1];

    CELL_INDEX index = _cell.Alloc();
    Cell& cell = _cell[index];

    cell._type = TYPE_CELL_REF;
    cell._data = head;
    cell._next = tail;

    return index;
}

CELL_INDEX Runtime::Eq(const ArgumentList& args)
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

CELL_INDEX Runtime::Quote(const ArgumentList& args)
{
    assert(args.size() == 1);
    return args[0];
}


