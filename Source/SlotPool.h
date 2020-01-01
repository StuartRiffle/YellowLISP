// YellowLISP (c) 2020 Stuart Riffle (MIT license)

#pragma once
#include "Yellow.h"
#include "Errors.h"

template< typename T >
class SlotPool
{
    vector<T> _elems;
    vector<INDEX> _freeSlots;

public:

    inline T& operator[](INDEX index) 
    {
        RAISE_ERROR_IF(!index.IsValid() || (index >= _elems.size()), ERROR_INTERNAL_SLOT_POOL_RANGE);
        return _elems[index];
    }

    inline INDEX Alloc()
    {
        INDEX index;

        if (_freeSlots.empty())
            ExpandPool();

        if (!_freeSlots.empty())
        {
            index = (INDEX) _freeSlots.back();
            _freeSlots.pop_back();
        }

        return(index);
    }

    inline void Free(INDEX index)
    {
        RAISE_ERROR_IF(!index.IsValid || (index >= _elems.size()), ERROR_INTERNAL_SLOT_POOL_RANGE);
        _freeSlots.push_back(index);
    }

    void ExpandPool()
    {
        INDEX firstNewElem = (INDEX) _elems.size();

        _elems.emplace_back();
        _elems.resize(_elems.capacity());

        for (INDEX i = firstNewElem; i < _elems.capacity(); i++)
            Free(i);
    }

    size_t GetPoolSize() const
    {
        return _elems.size();
    }

    size_t GetNumSlotsUsed() const
    {
        return _elems.size() - _freeSlots.size();
    }

    size_t GetMemoryFootprint() const
    {
        return (_elems.capacity() * sizeof(T)) + (_freeSlots.capacity() * sizeof(INDEX));
    }
};

