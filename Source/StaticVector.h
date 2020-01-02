// YellowLISP (c) 2020 Stuart Riffle (MIT license)

#pragma once
#include "Yellow.h"

template<class T, int ELEMENTS = 16>
class StaticVector
{
    T _embedded[ELEMENTS];
    std::vector<T> _overflow;
    int _count = 0;

public:
    StaticVector() {}
    StaticVector(const std::initializer_list<T>& elems) 
    { 
        for (auto elem : elems) 
            this->push_back(elem); 
    }

    inline T& operator[](size_t idx)             { return (idx < ELEMENTS)? _embedded[idx] : _overflow[idx - ELEMENTS]; }
    inline const T& operator[](size_t idx) const { return (idx < ELEMENTS)? _embedded[idx] : _overflow[idx - ELEMENTS]; }

    inline int  size() const  { return _count; }
    inline bool empty() const { return (_count == 0); }

    inline void push_back(const T& elem)
    {
        if (_count < ELEMENTS)
        {
            assert(_overflow.empty());
            _embedded[_count] = elem;
        }
        else
        {
            assert(_count == ELEMENTS + (int) _overflow.size());
            if (_overflow.capacity() == 0)
                _overflow.reserve(ELEMENTS);

            _overflow.push_back(elem);
        }

        _count++;
    }
};
