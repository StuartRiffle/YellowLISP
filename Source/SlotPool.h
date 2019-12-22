// YellowLISP (c) 2019 Stuart Riffle

#pragma once
#include "Yellow.h"
#include "Errors.h"

template< typename T >
class SlotPool
{
    vector<T> _elems;
    vector<uint32_t> _freeSlots;

public:

    SlotPool()
    {
        _elems.push_back(T()); // element 0 is reserved as invalid
    }

    inline T& operator[](size_t index) 
    {
        RAISE_ERROR_IF((index < 1) || (index >= _elems.size()), ERROR_INTERNAL_SLOT_POOL_RANGE);

        return _elems[index];
    }

    inline size_t Alloc()
    {
        size_t index = 0;

        if (_freeSlots.empty())
            ExpandPool();

        if (!_freeSlots.empty())
        {
            index = _freeSlots.back();
            _freeSlots.pop_back();
        }

        return(index);
    }

    inline void Free(size_t index)
    {
        assert((uint32_t)index == index);

        RAISE_ERROR_IF((index < 1) || (index >= _elems.size()), ERROR_INTERNAL_SLOT_POOL_RANGE);

        _freeSlots.push_back((uint32_t) index);
    }

    void ExpandPool()
    {
        size_t firstNewElem = _elems.size();

        _elems.emplace_back();
        _elems.resize(_elems.capacity());

        for (size_t i = firstNewElem; i < _elems.capacity(); i++)
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
        return (_elems.capacity() * sizeof(T)) + (_freeSlots.capacity() * sizeof(uint32_t));
    }
};

