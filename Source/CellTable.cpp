// YellowLISP (c) 2020 Stuart Riffle (MIT license)

#include "Yellow.h"
#include "Runtime.h"
#include "Errors.h"
#include "Testing.h"

CELLID Runtime::AllocateCell(CellType type)
{
    float pctFree =_cellFreeCount * 1.0f / _cell.size();
    if (pctFree < GARBAGE_COLLECTION_THRESH)
        _garbageCollectionNeeded = true;

    if (_cellFreeList == 0)
    {
        assert(_cellFreeCount == 0);
        ExpandCellTable();
        TEST_COVERAGE;
    }

    if (_cellFreeList == 0)
    {
        RAISE_ERROR(ERROR_INTERNAL_OUT_OF_MEMORY);
        return 0;
    }

    if (_nil)
        assert(_cellFreeList != _nil);

    CELLID index  = _cellFreeList;

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

    RETURN_WITH_COVERAGE(index);
}

void Runtime::FreeCell(CELLID index)
{
    _cell[index]._type = TYPE_FREE;
    _cell[index]._tags = 0;
    _cell[index]._data = 0xFFFFFFF;
    _cell[index]._next = _cellFreeList;

    _cellFreeList = index;
    _cellFreeCount++;
    TEST_COVERAGE;
}

void Runtime::ExpandCellTable()
{
    uint32_t oldSize = (uint32_t) _cell.size();
    uint32_t newSize = oldSize + (oldSize / 2) + 1;

    _cell.resize(newSize);

    for (uint32_t i = newSize - 1; i >= oldSize; i--)
        FreeCell(i);

    DebugValidateCells();
    TEST_COVERAGE;
}

void Runtime::MarkCellsInUse(CELLID index)
{
    if (!index.IsValid())
        VOID_RETURN_WITH_COVERAGE;;

    if (index == _nil)
        VOID_RETURN_WITH_COVERAGE;

    Cell& cell = _cell[index];

    if (cell._tags & TAG_GC_MARK)
        VOID_RETURN_WITH_COVERAGE;

    cell._tags |= TAG_GC_MARK;

    if (cell._type == TYPE_CONS)
    {
        MarkCellsInUse(cell._data);
        MarkCellsInUse(cell._next);
        TEST_COVERAGE;
    }

    if (cell._type == TYPE_LAMBDA)
    {
        CELLID args = cell._data;
        MarkCellsInUse(args);

        CELLID body = cell._next;
        MarkCellsInUse(body);

        TEST_COVERAGE;
    }

    VOID_RETURN_WITH_COVERAGE;
}

void Runtime::DebugValidateCells()
{
#if DEBUG_BUILD
    uint32_t slot = _cellFreeList;
    int numInFreeList = 0;

    while(slot)
    {
        assert(_cell[slot]._type == TYPE_FREE);
        numInFreeList++;

        slot = _cell[slot]._next;
    }

    int numMarkedFree = 0;

    for (size_t i = 0; i < _cell.size(); i++)
    {
        if (_cell[i]._type == TYPE_FREE)
            numMarkedFree++;
    }

    assert(_cellFreeCount == numInFreeList);
    assert(_cellFreeCount == numMarkedFree);
#endif
}

size_t Runtime::CollectGarbage()
{
    DebugValidateCells();

    // Mark everything in the global scope

    for (auto iter : _globalScope)
    {
        SYMBOLIDX symbolIndex = iter.second;
        SymbolInfo& symbol = _symbol[symbolIndex];

        assert(symbol._symbolCell);

        MarkCellsInUse(symbol._symbolCell);
        MarkCellsInUse(symbol._valueCell);
        MarkCellsInUse(symbol._macroBindings);
        TEST_COVERAGE;
    }

    // Mark cells with references on the callstack that got us here

    for (Scope* scope : _environment)
    {
        for (auto& binding : *scope)
        {
            MarkCellsInUse(binding.second);
            TEST_COVERAGE;
        }
    }

    // Mark everything on the free list too, so it doesn't get clobbered

    uint32_t slot = _cellFreeList;
    int numFree = 0;

    while(slot)
    {
        assert(_cell[slot]._type == TYPE_FREE);

        _cell[slot]._tags |= TAG_GC_MARK;
        slot = _cell[slot]._next;
        numFree++;
        TEST_COVERAGE;
    }

    DebugValidateCells();

    assert(_cellFreeCount == numFree);

    // Sweep away anything unreachable

    size_t numCellsFreed = 0;

    for (CELLID i = 1; i < _cell.size(); i = i + 1)
    {
        if (i == _nil)
        {
            TEST_COVERAGE;
            continue;
        }

        if (_cell[i]._tags & TAG_GC_MARK)
        {
            _cell[i]._tags &= ~TAG_GC_MARK;
            TEST_COVERAGE;
        }
        else
        {
            if (!(_cell[i]._tags & TAG_EMBEDDED))
            {
                if (_cell[i]._type == TYPE_STRING)
                {
                    STRINGIDX stringIndex = _cell[i]._data;
                    StringInfo& info = _string[stringIndex];

                    RAISE_ERROR_IF(info._refCount < 1, ERROR_INTERNAL_STRING_TABLE_CORRUPT);
                    info._refCount--;

                    if (info._refCount == 0)
                    {
                        _string.Free(stringIndex);

                        STRINGHASH hash = HashString(info._str.c_str());
                        RAISE_ERROR_IF(_stringTable[hash] != stringIndex, ERROR_INTERNAL_STRING_TABLE_CORRUPT);

                        _stringTable.erase(hash);
                        TEST_COVERAGE;
                    }
                    TEST_COVERAGE;
                }
                TEST_COVERAGE;
            }

            FreeCell(i);
            numCellsFreed++;
            TEST_COVERAGE;
        }
    }

    DebugValidateCells();

    _console->PrintDebug("[GC freed %d of %d cells]\n", (int) numCellsFreed, (int) _cell.size());
    RETURN_WITH_COVERAGE(numCellsFreed);
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
            VOID_RETURN_WITH_COVERAGE;
        }
    }

    VOID_RETURN_WITH_COVERAGE;
}
