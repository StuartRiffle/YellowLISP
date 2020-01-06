// YellowLISP (c) 2020 Stuart Riffle (MIT license)

#include "Yellow.h"
#include "Runtime.h"
#include "Errors.h"

int Runtime::LoadIntLiteral(CELLID index)
{
    assert(index.IsValid());
    const Cell& cell = _cell[index];

    RAISE_ERROR_IF(cell._type != TYPE_INT,           ERROR_INTERNAL_CELL_TABLE_CORRUPT, "LoadIntLiteral called on something not an int");
    RAISE_ERROR_IF((cell._tags & TAG_EMBEDDED) == 0, ERROR_INTERNAL_CELL_TABLE_CORRUPT, "LoadIntLiteral expects embedded data");

    int value = (int)cell._data;
    return (value);
}

void Runtime::StoreIntLiteral(CELLID index, int value)
{
    assert(index.IsValid());
    Cell& cell = _cell[index];

    //RAISE_ERROR_IF((cell._type != TYPE_INT) && (cell._type != TYPE_VOID), ERROR_INTERNAL_CELL_TABLE_CORRUPT);

    cell._data = value;
    cell._type = TYPE_INT;
    cell._tags = TAG_EMBEDDED;
    return;
}

float Runtime::LoadFloatLiteral(CELLID index)
{
    assert(index.IsValid());
    const Cell& cell = _cell[index];

    RAISE_ERROR_IF(cell._type != TYPE_FLOAT,         ERROR_INTERNAL_CELL_TABLE_CORRUPT, "LoadFloatLiteral called on something not an int");
    RAISE_ERROR_IF((cell._tags & TAG_EMBEDDED) == 0, ERROR_INTERNAL_CELL_TABLE_CORRUPT, "LoadFloatLiteral expects embedded data");

    union { uint32_t raw; float value; } pun;
    pun.raw = cell._data;

    return (pun.value);
}

void Runtime::StoreFloatLiteral(CELLID index, float value)
{
    Cell& cell = _cell[index];

    //RAISE_ERROR_IF((cell._type != TYPE_FLOAT) && (cell._type != TYPE_VOID), ERROR_INTERNAL_CELL_TABLE_CORRUPT);

    union { uint32_t raw; float value; } pun;
    pun.value = value;

    cell._data = pun.raw;
    cell._type = TYPE_FLOAT;
    cell._tags = TAG_EMBEDDED;
    return;
}

double Runtime::LoadNumericLiteral(CELLID index)
{
    double value = 0;

    switch (_cell[index]._type)
    {
        case TYPE_INT:    value = LoadIntLiteral(index); break;
        case TYPE_FLOAT:  value = LoadFloatLiteral(index); break;

        default: RAISE_ERROR(ERROR_RUNTIME_TYPE_MISMATCH, "numeric type expected");
    }

    return (value);
}

CELLID Runtime::CreateNumericLiteral(double value, bool storeAsInt)
{
    if (storeAsInt)
    {
        int intValue = (int)value;
        assert(intValue == value);

        CELLID index = AllocateCell(TYPE_INT);
        StoreIntLiteral(index, intValue);
        return (index);
    }

    CELLID index = AllocateCell(TYPE_FLOAT);
    StoreFloatLiteral(index, (float) value);
    return (index);
}

string Runtime::LoadStringLiteral(CELLID index)
{
    assert(index.IsValid());
    const Cell& cell = _cell[index];

    RAISE_ERROR_IF(cell._type != TYPE_STRING, ERROR_INTERNAL_CELL_TABLE_CORRUPT, "LoadStringLiteral called on something not a string");

    if (cell._tags & TAG_EMBEDDED)
    {
        char tiny[5] = { 0 };
        const char* chars = (const char*)&cell._data;
        strncpy(tiny, chars, sizeof(cell._data));

        return (tiny);
    }

    STRINGIDX stringIndex = cell._data;
    RAISE_ERROR_IF(!stringIndex.IsValid() || (stringIndex >= _string.GetPoolSize()), ERROR_INTERNAL_CELL_TABLE_CORRUPT, "string index out of range");

    const string& value = _string[stringIndex]._str;
    return (value);
}

void Runtime::StoreStringLiteral(CELLID index, const char* value)
{
    Cell& cell = _cell[index];

    //RAISE_ERROR_IF((cell._type != TYPE_STRING) && (cell._type != TYPE_VOID), ERROR_INTERNAL_CELL_TABLE_CORRUPT);

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
        STRINGIDX stringIndex;
        STRINGHASH hash = HashString(value);

        auto existing = _stringTable.find(hash);
        if (existing != _stringTable.end())
        {
            stringIndex = existing->second;
            assert(stringIndex.IsValid());

            RAISE_ERROR_IF(_string[stringIndex]._str != value, ERROR_INTERNAL_HASH_COLLISION, "64-bit hash collision... how did you do that?");
            
        }
        else
        {
            stringIndex = _string.Alloc();
            _string[stringIndex]._str = value;
            _stringTable[hash] = stringIndex;
            
        }

        cell._data = stringIndex;
        _string[stringIndex]._refCount++;
    }

    return;
}



