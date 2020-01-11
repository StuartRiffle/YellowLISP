#pragma once


template<typename T, int PAGE_BITS = 16>
class PagedTable
{
    enum
    {
        PAGE_ELEMS = 1 << PAGE_BITS,
        PAGE_MASK  = PAGE_ELEMS - 1,
    };

    typedef std::unique_ptr<vector<T>> Page;
    vector<Page> _pages;

    int _freeList = -1;
    static_assert(sizeof(T) >= sizeof(int));

    void Expand()
    {
        _pages.emplace_back(new vector<T>());
        vector<T>& elems = _pages.back();

        elems.resize(PAGE_ELEMS);
        for (auto& elem : elems)
            AddToFreeList(&elem);
    }

    inline void AddToFreeList(T* ptr)
    {
        *((int*) ptr) = _freeList;
        _freeList = (intptr_t*) ptr;
    }

    inline T* Locate(int index)
    {
        int page = index & PAGE_MASK;
        assert(page < _pages.size());

        int offset = index >> PAGE_BITS;
        assert(offset >= 0);
        assert(offset < PAGE_ELEMS);

        return &_pages[page][offset];
    }

public:

    inline int Alloc()
    {
        if (_freeList < 0)
            Expand();

        int index = _freeList;
        T* obj = Locate(index);

        int* next = (int*) obj;
        _freeList = *next;

        obj->T();
        return index;
    }

    inline void Free(int index)
    {
        T* obj = Locate(index);

        obj->~T();
        AddToFreeList(index);
    }

    inline T& operator[](size_t index)
    {
        T* obj = Locate(index);
        return *obj;
    }
};

