// YellowLISP (c) 2019 Stuart Riffle

#pragma once
#include "Yellow.h"

template< typename T >
class SlotPool
{
    std::vector<T> _elems;
    TINDEX _freeIndex;

    static_assert(sizeof(T) >= sizeof(TINDEX), "Type too small to be linked into a free list");

    void Expand()
    {
        size_t firstNewElem = _elems.size();
        _elems.push_back();

        for (size_t i = firstNewElem; i < _elems.capacity(); i++)
            Free((TINDEX)i);
    }

public:

    SlotPool()
    {
        _freeIndex = INVALID_INDEX;
    }

    inline T& operator[](TINDEX index) const
    {
        assert(index < _elems.size());
        return _elems[index];
    }

    inline TINDEX Alloc()
    {
        if (_freeIndex == INVALID_INDEX)
        {
            Expand();
            assert(_freeIndex != INVALID_INDEX);
        }

        int index = _freeIndex;
        TINDEX* freeLink = (TINDEX*)&_elems[index];
        _freeIndex = *freeLink;

        _elems[index] = T();
        return(index);
    }

    inline void Free(TINDEX index)
    {
        assert(index < _elems.size());

        TINDEX* freeLink = (TINDEX*)&_elems[index];
        *freeLink = _freeIndex;
        _freeIndex = index;
    }
};

