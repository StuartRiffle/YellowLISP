// YellowLISP (c) 2019 Stuart Riffle

#pragma once
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

typedef vector<CELL_INDEX> ArgumentList;
typedef CELL_INDEX(Runtime::*PrimitiveFunc)(const ArgumentList& args);

struct PrimitiveInfo
{
    string _name;
    int _numArgs;
    PrimitiveFunc _func;
};

struct RuntimeError : std::exception
{
    string _message;
    RuntimeError(const string& message) : _message(message) {}

    virtual const char* what() const { return _message.c_str(); }
};

#define CELL_TABLE_EXPAND_THRESH (0.9f)

class Runtime
{
    SlotPool<Cell, NO_AUTO_EXPAND> _cell;

    SlotPool<string>        _string;
    SlotPool<PrimitiveInfo> _primitive;
    SlotPool<SymbolInfo>    _symbol;

    std::unordered_map<THASH, SYMBOL_INDEX> _symbolIndex;

    CELL_INDEX  _nil;
    CELL_INDEX  _true;

    SYMBOL_INDEX ResolveSymbol(const char* ident);

    CELL_INDEX AllocateCell();
    void MarkCellsInUse(CELL_INDEX index);
    size_t CollectGarbage();

    int LoadIntLiteral(CELL_INDEX index) const;
    void StoreIntLiteral(CELL_INDEX index, int value);

    float LoadFloatLiteral(CELL_INDEX index) const;
    void StoreFloatLiteral(CELL_INDEX index, float value);

    const char* LoadStringLiteral(CELL_INDEX index) const;
    void StoreStringLiteral(CELL_INDEX index, const char* str);

    // Primitives

    void ValidateArgumentCount(const ArgumentList& args, size_t expected);

    CELL_INDEX Quote(const ArgumentList& args);
    CELL_INDEX Atom(const ArgumentList& args);
    CELL_INDEX Eq(const ArgumentList& args);
    CELL_INDEX Car(const ArgumentList& args);
    CELL_INDEX Cdr(const ArgumentList& args);
    CELL_INDEX Cons(const ArgumentList& args);

    void RegisterPrimitive(const PrimitiveInfo& prim);

public:
    Runtime();
    ~Runtime();

    CELL_INDEX EncodeSyntaxTree(const NodeRef& root);
    string GetPrintedValue(CELL_INDEX index);
};
