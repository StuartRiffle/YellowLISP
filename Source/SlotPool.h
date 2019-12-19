// YellowLISP (c) 2019 Stuart Riffle

#pragma once
#include "Yellow.h"

template< typename T >
class SlotPool
{
    vector<T> _elems;
    TINDEX _freeIndex;

    static_assert(sizeof(T) >= sizeof(TINDEX), "Type too small to be linked into a free list");

    void Expand()
    {
        assert(_elems.size() == _elems.capacity());
        size_t firstNewElem = _elems.size();

        _elems.push_back();
        _elems.resize(_elems.capacity());

        for (size_t i = firstNewElem; i < _elems.capacity(); i++)
            Free((TINDEX)i);
    }

public:

    SlotPool()
    {
        _elems.push_back(T());
        _freeIndex = 0;
    }

    inline T& operator[](TINDEX index) const
    {
        assert(index > 0);
        assert(index < _elems.size());

        return _elems[index];
    }

    inline TINDEX Alloc()
    {
        if (!_freeIndex)
        {
            Expand();
            assert(_freeIndex);
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

    size_t GetPoolSize() const
    {
        return _elems.size();
    }
};

