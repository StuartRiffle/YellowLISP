// YellowLISP (c) 2019 Stuart Riffle (MIT license)

#include "Yellow.h"
#include "Runtime.h"
#include "Errors.h"

int Runtime::LoadIntLiteral(CELL_INDEX index)
{
    const Cell& cell = _cell[index];

    RAISE_ERROR_IF(cell._type != TYPE_INT,           ERROR_INTERNAL_CELL_TABLE_CORRUPT);
    RAISE_ERROR_IF((cell._tags & TAG_EMBEDDED) == 0, ERROR_INTERNAL_CELL_TABLE_CORRUPT);

    int value = (int)cell._data;
    return value;
}

void Runtime::StoreIntLiteral(CELL_INDEX index, int value)
{
    Cell& cell = _cell[index];

    RAISE_ERROR_IF((cell._type != TYPE_INT) && (cell._type != TYPE_VOID), ERROR_INTERNAL_CELL_TABLE_CORRUPT);

    cell._data = value;
    cell._type = TYPE_INT;
    cell._tags = TAG_EMBEDDED;
}

float Runtime::LoadFloatLiteral(CELL_INDEX index)
{
    const Cell& cell = _cell[index];

    RAISE_ERROR_IF(cell._type != TYPE_FLOAT,         ERROR_INTERNAL_CELL_TABLE_CORRUPT);
    RAISE_ERROR_IF((cell._tags & TAG_EMBEDDED) == 0, ERROR_INTERNAL_CELL_TABLE_CORRUPT);

    union { TDATA raw; float value; } pun;
    pun.raw = cell._data;

    return(pun.value);
}

void Runtime::StoreFloatLiteral(CELL_INDEX index, float value)
{
    Cell& cell = _cell[index];

    RAISE_ERROR_IF((cell._type != TYPE_FLOAT) && (cell._type != TYPE_VOID), ERROR_INTERNAL_CELL_TABLE_CORRUPT);

    union { TDATA raw; float value; } pun;
    pun.value = value;

    cell._data = pun.raw;
    cell._type = TYPE_FLOAT;
    cell._tags = TAG_EMBEDDED;
}

double Runtime::LoadNumericLiteral(CELL_INDEX index)
{
    const Cell& cell = _cell[index];
    double value = 0;

    switch (cell._type)
    {
        case TYPE_INT:   value = LoadIntLiteral(index); break;
        case TYPE_FLOAT: value = LoadFloatLiteral(index); break;

        default: RAISE_ERROR(ERROR_RUNTIME_TYPE_MISMATCH, "numeric type expected");

    }

    return value;
}

CELL_INDEX Runtime::CreateNumericLiteral(double value)
{
    int asInt = (int)value;
    if (asInt == value)
    {
        CELL_INDEX index = AllocateCell(TYPE_INT);
        StoreIntLiteral(index, asInt);
        return index;
    }

    CELL_INDEX index = AllocateCell(TYPE_FLOAT);
    StoreFloatLiteral(index, (float) value);
    return index;
}

string Runtime::LoadStringLiteral(CELL_INDEX index)
{
    const Cell& cell = _cell[index];

    RAISE_ERROR_IF(cell._type != TYPE_STRING, ERROR_INTERNAL_CELL_TABLE_CORRUPT);

    if (cell._tags & TAG_EMBEDDED)
    {
        char tiny[5] = { 0 };
        const char* chars = (const char*)&cell._data;
        strncpy(tiny, chars, sizeof(cell._data));

        return tiny;
    }

    STRING_INDEX stringIndex = cell._data;
    RAISE_ERROR_IF((stringIndex == 0) || (stringIndex >= _string.GetPoolSize()), ERROR_INTERNAL_CELL_TABLE_CORRUPT);

    const string& value = _string[stringIndex]._str;
    RAISE_ERROR_IF(value.length() <= sizeof(TDATA), ERROR_INTERNAL_STRING_TABLE_CORRUPT);

    return value;
}

void Runtime::StoreStringLiteral(CELL_INDEX index, const char* value)
{
    Cell& cell = _cell[index];

    RAISE_ERROR_IF((cell._type != TYPE_STRING) && (cell._type != TYPE_VOID), ERROR_INTERNAL_CELL_TABLE_CORRUPT);

    cell._type = TYPE_STRING;

    size_t length = strlen(value);
    if (length <= sizeof(cell._data))
    {
        cell._data = 0;
        strncpy((char*)&cell._data, value, sizeof(cell._data));
        cell._tags |= TAG_EMBEDDED;
    }
    else
    {
        STRING_INDEX stringIndex = 0;

        THASH hash = HashString(value);
        auto existing = _stringTable.find(hash);
        if (existing != _stringTable.end())
        {
            stringIndex = existing->second;

            RAISE_ERROR_IF(_string[stringIndex]._str != value, ERROR_INTERNAL_HASH_COLLISION);
        }
        else
        {
            stringIndex = (STRING_INDEX)_string.Alloc();
            _string[stringIndex]._str = value;
            _stringTable[hash] = stringIndex;
        }

        _string[stringIndex]._refCount++;
    }
}

