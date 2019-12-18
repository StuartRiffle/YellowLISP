// YellowLISP (c) 2019 Stuart Riffle

#pragma once

#include <stdint.h>
#include <assert.h>
 
#include <vector>
#include <string>
#include <memory>
#include <iostream>
#include <sstream>
#include <unordered_map>

using namespace std;

enum Tag
{
    TAG_USED,
    TAG_MARKED,

    TAG_COUNT,
    TAG_BITS = 3
};

enum Type
{
    TYPE_VOID,      
    TYPE_ATOM,      // Hash of the identifier
    TYPE_CELL,      // Index into cell table
    TYPE_FUNC,      // Index into function table
    TYPE_STRING,    // Index into string table
    TYPE_INT,       // Literal int
    TYPE_FLOAT,     // Literal float

    TYPE_COUNT,
    TYPE_BITS = 3
};

typedef uint64_t TWORD;
typedef uint32_t TINDEX;
typedef uint32_t TDATA;
typedef uint32_t THASH;

const int WORD_BITS = sizeof(TWORD) * 8;
const int DATA_BITS = sizeof(TDATA) * 8;
const int FLOAT_BITS = sizeof(float) * 8;
const int EXTRA_BITS = TYPE_BITS + TAG_BITS;
const int INDEX_BITS = WORD_BITS - (DATA_BITS + EXTRA_BITS);
const int INVALID_INDEX = ~((TINDEX)0);

static_assert(TAG_COUNT < (1 << TAG_BITS), "Not enough tag bits");
static_assert(TYPE_COUNT < (1 << TYPE_BITS), "Not enough type bits");
static_assert(INDEX_BITS <= DATA_BITS, "Not enough data bits to store an index");
static_assert(DATA_BITS <= sizeof(TDATA) * 8, "Too many data bits");
static_assert(INDEX_BITS <= sizeof(TINDEX) * 8, "Too many address bits");
static_assert(sizeof(THASH) <= sizeof(TDATA), "Not enough data bits to store a hash");

#include "Cell.h"
#include "SlotPool.h"
