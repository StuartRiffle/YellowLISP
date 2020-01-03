// YellowLISP (c) 2020 Stuart Riffle (MIT license)

#include "Yellow.h"
#include "Runtime.h"

CELLID Runtime::Help(const CELLVEC&)
{
    _console->Print("TODO\n");
    return 0;
}

CELLID Runtime::Exit(const CELLVEC& args)
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

CELLID Runtime::RunGC(const CELLVEC&)
{
    size_t numReclaimed = CollectGarbage();
    size_t numTotal = _cell.size();
    size_t numFree = 0;

    uint32_t slot = _cellFreeList;
    while(slot)
    {
        numFree++;
        slot = _cell[slot]._next;
    }

    _console->PrintDebug("%d cells reclaimed, %d/%d now available\n", (int) numReclaimed, (int) numFree, (int) numTotal);
    RETURN_ASSERT_COVERAGE(0);
}


