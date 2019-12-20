// YellowLISP (c) 2019 Stuart Riffle

#pragma once
#include "Yellow.h"

#define NO_AUTO_EXPAND (0)
#define ENABLE_AUTO_EXPAND (1)

template< 
    typename T, 
    int AUTO_EXPAND = ENABLE_AUTO_EXPAND,
    int INITIAL_CAPACITY = 1024
>
class SlotPool
{
    vector<T> _elems;
    size_t _freeIndex;

    static_assert(sizeof(T) >= sizeof(size_t), "Type too small to be linked into a free list");

public:

    SlotPool()
    {
        _elems.reserve(INITIAL_CAPACITY);

        _elems.push_back(T()); // element 0 is reserved as invalid
        _freeIndex = 0;
    }

    inline T& operator[](size_t index) 
    {
        assert(index > 0);
        assert(index < _elems.size());

        return _elems[index];
    }

    inline size_t Alloc()
    {
        if (!_freeIndex)
            if (AUTO_EXPAND)
                ExpandPool();

        int index = (int) _freeIndex;
        if (index)
        {
            size_t* freeLink = (size_t*)&_elems[index];
            _freeIndex = *freeLink;
            _elems[index] = T();
        }

        return(index);
    }

    inline void Free(size_t index)
    {
        assert(index < _elems.size());

        size_t* freeLink = (size_t*)&_elems[index];
        *freeLink = _freeIndex;
        _freeIndex = index;
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

