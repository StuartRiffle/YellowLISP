// YellowLISP (c) 2020 Stuart Riffle (MIT license)

#pragma once
#include "Yellow.h"
#include "Errors.h"

template<typename T, typename TINDEX>
class SlotPool
{
    vector<T>      _elems;
    vector<TINDEX> _freeSlots;

public:

    inline T& operator[](TINDEX index) 
    {
        RAISE_ERROR_IF(!index.IsValid() || (index >= _elems.size()), ERROR_INTERNAL_SLOT_POOL_RANGE, "attempt to index out of bounds");
        return _elems[index];
    }

    inline TINDEX Alloc()
    {
        TINDEX index;

        if (_freeSlots.empty())
            ExpandPool();

        if (!_freeSlots.empty())
        {
            index = _freeSlots.back();
            _freeSlots.pop_back();
        }

        return(index);
    }

    inline void Free(TINDEX index)
    {
        RAISE_ERROR_IF(!index.IsValid() || (index >= _elems.size()), ERROR_INTERNAL_SLOT_POOL_RANGE, "attempt to index out of bounds");
        _freeSlots.push_back(index);
    }

    void ExpandPool()
    {
        size_t firstNewElem = _elems.size();

        _elems.emplace_back();
        _elems.resize(_elems.capacity());

        for (size_t i = firstNewElem; i < _elems.capacity(); i++)
            Free((uint32_t) i);
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
        return (_elems.capacity() * sizeof(T)) + (_freeSlots.capacity() * sizeof(TINDEX));
    }
};

