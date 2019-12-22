// YellowLISP (c) 2019 Stuart Riffle

#pragma once
#include "Yellow.h"

template< typename T >
class SlotPool
{
    vector<T> _elems;
    vector<size_t> _freeSlots;

    static_assert(sizeof(T) >= sizeof(size_t), "Type too small to be linked into a free list");

public:

    SlotPool()
    {
        _elems.push_back(T()); // element 0 is reserved as invalid
    }

    inline T& operator[](size_t index) 
    {
        assert(index > 0);
        assert(index < _elems.size());

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
        assert(index < _elems.size());
        _freeSlots.push_back(index);
    }

    void ExpandPool()
    {
        size_t firstNewElem = _elems.size();

        _elems.emplace_back();
        _elems.resize(_elems.capacity());

        for (size_t i = firstNewElem; i < _elems.capacity(); i++)
            Free((size_t)i);
    }

    size_t GetPoolSize() const
    {
        return _elems.size();
    }
};

