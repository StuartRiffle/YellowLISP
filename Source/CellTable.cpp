#include "Yellow.h"
#include "Runtime.h"

CELL_INDEX Runtime::AllocateCell(Type type)
{
#if YELLOW_ENABLE_GC
    if (_cellFreeList == 0)
    {
        size_t numCellsFreed = CollectGarbage();

        float pctFreed = numCellsFreed * 1.0f / _cell.size();
        float pctUsed = 1 - pctFreed;
        assert((pctUsed >= 0) && (pctUsed <= 1));

        if (pctUsed >= CELL_TABLE_EXPAND_THRESH)
        {
            // The GC didn't free up much space. Expand the table now to avoid
            // invoking GC over and over again as the working set increases.

            ExpandCellTable();
        }
    }
#endif

    if (_cellFreeList == 0)
        ExpandCellTable();

    if (_cellFreeList == 0)
    {
        assert(!"Out of memory");
        return 0;
    }

    CELL_INDEX index = _cellFreeList;
    _cellFreeList = _cell[_cellFreeList]._next;

    _cell[index]._type = type;
    _cell[index]._next = 0;
    _cell[index]._tags = TAG_IN_USE;

    return index;
}

void Runtime::FreeCell(CELL_INDEX index)
{
    _cell[index]._type = TYPE_VOID;
    _cell[index]._tags = 0;
    _cell[index]._data = 0;
    _cell[index]._next = _cellFreeList;

    _cellFreeList = index;
}

void Runtime::ExpandCellTable()
{
    size_t oldSize = _cell.size();
    size_t newSize = oldSize + (oldSize / 2) + 1;

    _cell.resize(newSize);

    for (size_t i = oldSize; i < newSize; i++)
        FreeCell((CELL_INDEX)i);
}

void Runtime::MarkCellsInUse(CELL_INDEX index)
{
    Cell& cell = _cell[index];

    if (cell._tags & TAG_GC_MARK)
        return;

    cell._tags |= TAG_GC_MARK;

    if (cell._type == TYPE_LIST)
        MarkCellsInUse(cell._data);

    if (cell._next)
        MarkCellsInUse(cell._next);
}

size_t Runtime::CollectGarbage()
{
    size_t numCellsFreed = 0;

    // Mark our core symbols

    MarkCellsInUse(_nil);
    MarkCellsInUse(_true);
    MarkCellsInUse(_quote);

    // Mark everything in the global scope

    for (auto iter : _globalScope)
    {
        SYMBOL_INDEX symbolIndex = iter.second;
        SymbolInfo& symbol = _symbol[symbolIndex];

        if (symbol._cellIndex)
            MarkCellsInUse(symbol._cellIndex);
    }

    // Mark cells with references on the callstack that got us here

    for (auto& scope : _environment)
        for (auto& binding : scope)
            MarkCellsInUse(binding.second);

    // Sweep away anything unreachable

    for (TINDEX i = 1; i < _cell.size(); i++)
    {
        if (_cell[i]._tags & TAG_GC_MARK)
        {
            assert(_cell[i]._tags & TAG_IN_USE);
            _cell[i]._tags &= ~TAG_GC_MARK;
        }
        else
        {
            if (!(_cell[i]._tags & TAG_EMBEDDED))
            {
                if (_cell[i]._type == TYPE_STRING)
                {
                    STRING_INDEX stringIndex = _cell[i]._data;
                    StringInfo& info = _string[stringIndex];

                    assert(info._refCount > 0);
                    info._refCount--;

                    if (info._refCount == 0)
                    {
                        _string.Free(stringIndex);

                        THASH hash = HashString(info._str.c_str());
                        assert(_stringTable[hash] == stringIndex);
                        _stringTable.erase(hash);
                    }
                }
            }

            FreeCell(i);
            numCellsFreed++;
        }
    }

    return numCellsFreed;
}