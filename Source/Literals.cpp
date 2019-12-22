#include "Yellow.h"
#include "Runtime.h"

int Runtime::LoadIntLiteral(CELL_INDEX index)
{
    const Cell& cell = _cell[index];
    assert(cell._type == TYPE_INT);
    assert(cell._tags & TAG_EMBEDDED);

    int value = (int)cell._data;
    return value;
}

void Runtime::StoreIntLiteral(CELL_INDEX index, int value)
{
    Cell& cell = _cell[index];
    assert((cell._type == TYPE_INT) || (cell._type == TYPE_VOID));

    cell._data = value;
    cell._type = TYPE_INT;
    cell._tags = TAG_EMBEDDED;
}

float Runtime::LoadFloatLiteral(CELL_INDEX index)
{
    const Cell& cell = _cell[index];
    assert(cell._type == TYPE_FLOAT);
    assert(cell._tags & TAG_EMBEDDED);

    union { TDATA raw; float value; } pun;
    pun.raw = cell._data;

    return(pun.value);
}

void Runtime::StoreFloatLiteral(CELL_INDEX index, float value)
{
    Cell& cell = _cell[index];
    assert((cell._type == TYPE_FLOAT) || (cell._type == TYPE_VOID));

    union { TDATA raw; float value; } pun;
    pun.value = value;

    cell._data = pun.raw;
    cell._type = TYPE_FLOAT;
    cell._tags = TAG_EMBEDDED;
}

string Runtime::LoadStringLiteral(CELL_INDEX index)
{
    const Cell& cell = _cell[index];
    assert(cell._type == TYPE_STRING);

    if (cell._tags & TAG_EMBEDDED)
    {
        char tiny[5] = { 0 };
        const char* chars = (const char*)&cell._data;
        strncpy(tiny, chars, sizeof(cell._data));

        return tiny;
    }

    STRING_INDEX stringIndex = cell._data;
    assert((stringIndex > 0) && (stringIndex < _string.GetPoolSize()));

    const string& value = _string[stringIndex]._str;
    assert(value.length() >= sizeof(TDATA));

    return value;
}

void Runtime::StoreStringLiteral(CELL_INDEX index, const char* value)
{
    Cell& cell = _cell[index];
    assert((cell._type == TYPE_STRING) || (cell._type == TYPE_VOID));

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
            assert(_string[stringIndex]._str == value);
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

