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

