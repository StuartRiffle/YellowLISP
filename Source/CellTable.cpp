// YellowLISP (c) 2019 Stuart Riffle (MIT license)

#include "Yellow.h"
#include "Runtime.h"
#include "Errors.h"

CELL_INDEX Runtime::AllocateCell(Type type)
{
    float pctUsed = _cellFreeCount * 1.0f / _cell.size();
    assert((pctUsed >= 0) && (pctUsed <= 1));

    if (pctUsed >= CELL_TABLE_EXPAND_THRESH)
    {
        ExpandCellTable();
        _garbageCollectionNeeded = true;
    }

    if (!VALID_CELL(_cellFreeList))
    {
        // This should be rare, because HandleGarbage() should
        // expand the table before we hit the wall.

        ExpandCellTable();
    }

    if (!VALID_CELL(_cellFreeList))
    {
        RAISE_ERROR(ERROR_INTERNAL_OUT_OF_MEMORY);
        return 0;
    }

    CELL_INDEX index = _cellFreeList;
    _cellFreeList = _cell[_cellFreeList]._next;
    _cellFreeCount--;

    if (_cellFreeCount == 0)
        assert(_cellFreeList == _nil);

    if (_cellFreeList == _nil)
        assert(_cellFreeCount == 0);

    _cell[index]._type = type;
    _cell[index]._next = _nil;
    _cell[index]._tags = 0;

    return index;
}

void Runtime::FreeCell(CELL_INDEX index)
{
    _cell[index]._type = TYPE_VOID;
    _cell[index]._tags = 0;
    _cell[index]._data = 0;
    _cell[index]._next = _cellFreeList;

    _cellFreeList = index;
    _cellFreeCount++;
}

void Runtime::ExpandCellTable(size_t minimumSize)
{
    size_t oldSize = _cell.size();
    size_t newSize = oldSize + (oldSize / 2) + 1;

    if (newSize < minimumSize)
        newSize = minimumSize;

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
    {
        MarkCellsInUse(cell._data);

        if (VALID_CELL(cell._next))
            MarkCellsInUse(cell._next);
    }

    if (cell._type == TYPE_LAMBDA)
    {
        CELL_INDEX args = cell._data;
        MarkCellsInUse(args);

        CELL_INDEX body = cell._next;
        MarkCellsInUse(body);
    }
}

size_t Runtime::CollectGarbage()
{
    size_t numCellsFreed = 0;

    // Mark everything in the global scope

    for (auto iter : _globalScope)
    {
        SYMBOL_INDEX symbolIndex = iter.second;
        SymbolInfo& symbol = _symbol[symbolIndex];

        if (VALID_CELL(symbol._symbolCell))
        {
            MarkCellsInUse(symbol._symbolCell);
            MarkCellsInUse(symbol._valueCell);
            MarkCellsInUse(symbol._macroBindings);
        }
    }

    // Mark cells with references on the callstack that got us here

    for (auto& scope : _environment)
        for (auto& binding : scope)
            MarkCellsInUse(binding.second);

    // Mark everything on the free list too, so it doesn't get clobbered

    CELL_INDEX slot = _cellFreeList;
    int numFree = 0;

    while(VALID_CELL(slot))
    {
        assert(_cell[slot]._type == TYPE_VOID);

        _cell[slot]._tags |= TAG_GC_MARK;
        slot = _cell[slot]._next;
        numFree++;
    }

    assert(_cellFreeCount == numFree);

    // Sweep away anything unreachable

    for (TINDEX i = 1; i < _cell.size(); i++)
    {
        if (_cell[i]._tags & TAG_GC_MARK)
        {
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

                    RAISE_ERROR_IF(info._refCount < 1, ERROR_INTERNAL_STRING_TABLE_CORRUPT);
                    info._refCount--;

                    if (info._refCount == 0)
                    {
                        _string.Free(stringIndex);

                        THASH hash = HashString(info._str.c_str());
                        RAISE_ERROR_IF(_stringTable[hash] != stringIndex, ERROR_INTERNAL_STRING_TABLE_CORRUPT);

                        _stringTable.erase(hash);
                    }
                }
            }

            FreeCell(i);
            numCellsFreed++;
        }
    }

    printf("Freed %d of %d cells\n", (int) numCellsFreed, (int) _cell.size());
    return numCellsFreed;
}


void Runtime::HandleGarbage()
{
    if (_garbageCollectionNeeded)
    {
        CollectGarbage();
        _garbageCollectionNeeded = false;

    }
}
