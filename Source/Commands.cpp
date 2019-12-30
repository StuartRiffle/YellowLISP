// YellowLISP (c) 2019 Stuart Riffle (MIT license)

#include "Yellow.h"
#include "Runtime.h"

CELL_INDEX Runtime::Help(const ArgumentList& args)
{
    (args);
    std::cout << "TODO" << std::endl;
    return 0;
}

CELL_INDEX Runtime::Exit(const ArgumentList& args)
{
    int returnValue = RETURN_SUCCESS;

    if (args.size() > 0)
    {
        const Cell& cell = _cell[args[0]];
        if (cell._type == TYPE_INT)
            returnValue = cell._data;
    }

    exit(returnValue);
}

CELL_INDEX Runtime::RunGC(const ArgumentList& args)
{
    (args);

    size_t numReclaimed = CollectGarbage();
    size_t numTotal = _cell.size();
    size_t numFree = 0;

    CELL_INDEX slot = _cellFreeList;
    while(VALID_CELL(slot))
    {
        numFree++;
        slot = _cell[slot]._next;
    }

    printf("%d cells reclaimed, %d/%d now available\n", (int) numReclaimed, (int) numFree, (int) numTotal);
    return 0;
}


