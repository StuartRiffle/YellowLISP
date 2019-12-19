// YellowLISP (c) 2019 Stuart Riffle

#pragma once
#include "Yellow.h"

enum Type
{                       // The data stored in the cell is...
    TYPE_CELL_REF,      //     an index into the cell table
    TYPE_SYMBOL,        //     an index into the symbol table
    TYPE_FUNC,          //     an index into the function table
    TYPE_STRING,        //     an index into the string table
    TYPE_INT,           //     a signed integer value
    TYPE_FLOAT,         //     an IEEE floating point value

    TYPE_COUNT,
    TYPE_BITS = 3
};

enum Tag
{
    TAG_GC_MARK,        // Marks cell as "in use" during mark-and-sweep garbage collection

    TAG_COUNT,
    TAG_BITS = 3
};

typedef uint64_t TWORD;
typedef uint32_t TINDEX;
typedef uint32_t TDATA;

const int CELL_BITS  = sizeof(TWORD) * 8;
const int DATA_BITS  = sizeof(TDATA) * 8;
const int FLOAT_BITS = sizeof(float) * 8;
const int EXTRA_BITS = TYPE_BITS + TAG_BITS;
const int INDEX_BITS = CELL_BITS - (DATA_BITS + EXTRA_BITS);

static_assert(TAG_COUNT < (1 << TAG_BITS), "Not enough tag bits");
static_assert(TYPE_COUNT < (1 << TYPE_BITS), "Not enough type bits");
static_assert(INDEX_BITS <= DATA_BITS, "Not enough data bits to store an index");
static_assert(DATA_BITS <= sizeof(TDATA) * 8, "Too many data bits");
static_assert(INDEX_BITS <= sizeof(TINDEX) * 8, "Too many address bits");

struct Cell
{
    TWORD _tags : TAG_BITS;
    TWORD _type : TYPE_BITS;
    TWORD _next : INDEX_BITS;
    TWORD _data : DATA_BITS;

    union FloatUnion
    {
        TDATA raw;
        float value;
    };

    inline float GetFloat() const
    {
        assert(_type == TYPE_FLOAT);

        FloatUnion pun;
        pun.raw = _data;

        int floatBits = sizeof(float) * 8;
        int zeroPad = (floatBits - DATA_BITS);
        if (zeroPad > 0)
            pun.raw <<= zeroPad;

        return(pun.value);
    }

    inline void SetFloat(float value)
    {
        FloatUnion pun;
        pun.value = value;

        int floatBits = sizeof(float) * 8;
        int truncateBits = (floatBits - DATA_BITS);
        if (truncateBits > 0)
            pun.raw >>= truncateBits;

        _data = pun.raw;
        _type = TYPE_FLOAT;
    }

    inline int GetInteger() const
    {
        assert(_type == TYPE_INT);
        int value = (int)_data;

        int valueBits = sizeof(value) * 8;
        int signExtend = valueBits - DATA_BITS;
        if (signExtend > 0)
            value = (value << signExtend) >> signExtend;

        return value;
    }

    inline void SetInteger(int value)
    {
        int valueBits = sizeof(value) * 8;
        int signExtend = DATA_BITS - valueBits;
        if (signExtend > 0)
            value = (value << signExtend) >> signExtend;

        _data = value;
        _type = TYPE_INT;
    }
};
