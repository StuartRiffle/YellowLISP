// YellowLISP (c) 2019 Stuart Riffle

#pragma once
#include "SyntaxTree.h"
#include "SlotPool.h"
#include "Hash.h"

enum Type           // The data stored in the cell is...
{                   // 
    TYPE_VOID,      // zero, because the cell is uninitialized
    TYPE_CELL_REF,  // an index into the cell table
    TYPE_SYMBOL,    // an index into the symbol table
    TYPE_FUNC,      // an index into the function table
    TYPE_STRING,    // a tiny string literal, or an index into the string table
    TYPE_INT,       // a signed integer literal
    TYPE_FLOAT,     // an IEEE floating point literal

    TYPE_COUNT,
    TYPE_BITS = 3
};

enum Tag
{
    TAG_ATOM,       // Can be deduced from the type, but tagged for convenience
    TAG_EMBEDDED,   // The value is contained in the cell (as opposed to indexed)
    TAG_GC_MARK,    // Marks cell as "in use" during mark-and-sweep garbage collection

    TAG_COUNT,
    TAG_BITS = 3
};

typedef uint32_t THEADER;   // Cell tag/type information and link to next
typedef uint32_t TDATA;     // Cell value index, or embedded value if TAG_EMBEDDED

const int HEADER_BITS = sizeof(THEADER) * 8;
const int DATA_BITS = sizeof(TDATA) * 8;
const int EXTRA_BITS = TYPE_BITS + TAG_BITS;
const int INDEX_BITS = HEADER_BITS - EXTRA_BITS;

static_assert(TAG_COUNT  < (1 << TAG_BITS), "Not enough tag bits");
static_assert(TYPE_COUNT < (1 << TYPE_BITS), "Not enough type bits");
static_assert(INDEX_BITS <= DATA_BITS, "Not enough data bits to store an index");

typedef TDATA  TINDEX;
typedef TINDEX CELL_INDEX;
typedef TINDEX SYMBOL_INDEX;
typedef TINDEX FUNC_INDEX;
typedef TINDEX STRING_INDEX;

struct Cell
{
    uint32_t _tags : TAG_BITS;
    uint32_t _type : TYPE_BITS;
    uint32_t _next : INDEX_BITS;
    uint32_t _data;
};

struct SymbolInfo
{
    string _ident;
    TINDEX _cellIndex;
};

struct FunctionInfo
{
    string _name;
};

class Runtime
{
    SlotPool<Cell>          _cell;
    SlotPool<SymbolInfo>    _symbol;
    SlotPool<FunctionInfo>  _function;
    SlotPool<string>        _string;

    unordered_map<THASH, SYMBOL_INDEX> _symbolIndex;

    CELL_INDEX          _nil;
    CELL_INDEX          _true;

    void Init();

    SYMBOL_INDEX ResolveSymbol(const char* ident);
    string CellToString(CELL_INDEX index);

    CELL_INDEX Quote(CELL_INDEX index) const;
    CELL_INDEX Atom(CELL_INDEX index) const;
    CELL_INDEX Eq(CELL_INDEX a, CELL_INDEX b) const;
    CELL_INDEX Car(CELL_INDEX index) const;
    CELL_INDEX Cdr(CELL_INDEX index) const;
    CELL_INDEX Cons(CELL_INDEX head, CELL_INDEX tail);

    int LoadIntLiteral(CELL_INDEX index) const;
    void StoreIntLiteral(CELL_INDEX index, int value);

    float LoadFloatLiteral(CELL_INDEX index) const;
    void StoreFloatLiteral(CELL_INDEX index, float value);

    const char* LoadStringLiteral(CELL_INDEX index) const;
    void StoreStringLiteral(CELL_INDEX index, const char* str);
};
