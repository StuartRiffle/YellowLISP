// YellowLISP (c) 2019 Stuart Riffle (MIT license)

#include "Yellow.h"
#include "Runtime.h"
#include "Errors.h"

CELL_INDEX Runtime::AllocateCell(Type type)
{
    float pctFree =_cellFreeCount * 1.0f / _cell.size();
    if (pctFree < GARBAGE_COLLECTION_THRESH)
        _garbageCollectionNeeded = true;

    if (_cellFreeList == 0)
    {
        assert(_cellFreeCount == 0);
        ExpandCellTable();
    }

    if (_cellFreeList == 0)
    {
        RAISE_ERROR(ERROR_INTERNAL_OUT_OF_MEMORY);
        return 0;
    }

    if (_nil)
        assert(_cellFreeList != _nil);

    CELL_INDEX index = _cellFreeList;
    _cellFreeList = _cell[_cellFreeList]._next;
    _cellFreeCount--;

    assert(_cell[index]._type == TYPE_FREE);

    if (_cellFreeCount == 0)
        assert(_cellFreeList == 0);

    if (_cellFreeList == 0)
        assert(_cellFreeCount == 0);

    _cell[index]._type = type;
    _cell[index]._next = _nil;
    _cell[index]._tags = 0;

    return index;
}

void Runtime::FreeCell(CELL_INDEX index)
{
    _cell[index]._type = TYPE_FREE;
    _cell[index]._tags = 0;
    _cell[index]._data = 0;
    _cell[index]._next = _cellFreeList;

    _cellFreeList = index;
    _cellFreeCount++;
}

void Runtime::InitCellTable(size_t size)
{
    _cell.resize(size);
    _cellFreeList = 0;
    _cellFreeCount = 0;

    for (size_t i = size - 1; i > 0; i--)
        FreeCell((CELL_INDEX)i);

    DebugValidateCells();
}


void Runtime::ExpandCellTable()
{
    size_t oldSize = _cell.size();
    size_t newSize = oldSize + (oldSize / 2) + 1;

    _cell.resize(newSize);

    for (size_t i = newSize - 1; i >= oldSize; i--)
        FreeCell((CELL_INDEX)i);

    DebugValidateCells();
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

        assert(cell._next != 0);
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

void Runtime::DebugValidateCells()
{
    CELL_INDEX slot = _cellFreeList;
    int numInFreeList = 0;

    while(slot)
    {
        assert(_cell[slot]._type == TYPE_FREE);
        numInFreeList++;

        slot = _cell[slot]._next;
    }

    int numMarkedFree = 0;

    for (TINDEX i = 1; i < _cell.size(); i++)
    {
        //assert((_cell[i]._tags & TAG_GC_MARK) == 0);
        if (_cell[i]._type == TYPE_FREE)
            numMarkedFree++;
    }

    assert(_cellFreeCount == numInFreeList);
    assert(_cellFreeCount == numMarkedFree);
}

size_t Runtime::CollectGarbage()
{
    DebugValidateCells();

    // Mark everything in the global scope

    for (auto iter : _globalScope)
    {
        SYMBOL_INDEX symbolIndex = iter.second;
        SymbolInfo& symbol = _symbol[symbolIndex];

        //std::cout << "symbol " << symbolIndex << " -> cell " << symbol._symbolCell << std::endl;
        assert(symbol._symbolCell);

        MarkCellsInUse(symbol._symbolCell);
        MarkCellsInUse(symbol._valueCell);
        MarkCellsInUse(symbol._macroBindings);
    }

    // Mark cells with references on the callstack that got us here

    for (auto& scope : _environment)
        for (auto& binding : scope)
            MarkCellsInUse(binding.second);

    // Mark everything on the free list too, so it doesn't get clobbered

    CELL_INDEX slot = _cellFreeList;
    int numFree = 0;

    while(slot)
    {
        assert(_cell[slot]._type == TYPE_FREE);

        _cell[slot]._tags |= TAG_GC_MARK;
        slot = _cell[slot]._next;
        numFree++;
    }

    DebugValidateCells();

    // FIXME
    //
    // GC is dropping elements from the free list
    // numFree 75, cellFreeCount 90


    assert(_cellFreeCount == numFree);
    assert(_cell[_nil]._tags & TAG_GC_MARK);

    // Sweep away anything unreachable

    size_t numCellsFreed = 0;

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

    DebugValidateCells();

    printf("Freed %d of %d cells\n", (int) numCellsFreed, (int) _cell.size());
    return numCellsFreed;
}


void Runtime::HandleGarbage()
{
    if (_garbageCollectionNeeded)
    {
        CollectGarbage();
        _garbageCollectionNeeded = false;

        float pctFree = _cellFreeCount * 1.0f / _cell.size();
        if (pctFree < CELL_TABLE_EXPAND_THRESH)
        {
            // The GC didn't free up much space, so expand the table to avoid thrashing


            ExpandCellTable();
        }
    }
}
