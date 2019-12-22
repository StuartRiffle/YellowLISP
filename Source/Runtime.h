// YellowLISP (c) 2019 Stuart Riffle

#pragma once
#include "SlotPool.h"
#include "Hash.h"
#include "Parser.h"

enum Type : uint32_t  
{                       // The data stored in the cell is...
    TYPE_VOID,          //   zero, because the cell is uninitialized
    TYPE_LIST,          //   an index into the cell table
    TYPE_SYMBOL,        //   an index into the symbol table
    TYPE_STRING,        //   a tiny string literal, or an index into the string table
    TYPE_INT,           //   a signed integer literal
    TYPE_FLOAT,         //   an IEEE floating point literal

    TYPE_COUNT,
    TYPE_BITS = 3
};

enum Tags
{
    TAG_IN_USE     = 1 << 0,   // Cell has been allocated
    TAG_GC_MARK    = 1 << 1,   // Marks cell as "reachable" during mark-and-sweep garbage collection
    TAG_EMBEDDED   = 1 << 2,   // The value is contained in the cell (as opposed to indexed)

    TAG_BITS = 3
};

typedef uint32_t THEADER;   // Cell tag/type information and link to next
typedef uint32_t TDATA;     // Cell value index, or embedded value if TAG_EMBEDDED

const int HEADER_BITS = sizeof(THEADER) * 8;
const int DATA_BITS = sizeof(TDATA) * 8;
const int METADATA_BITS = TYPE_BITS + TAG_BITS;
const int INDEX_BITS = HEADER_BITS - METADATA_BITS;

static_assert(TYPE_COUNT < (1 << TYPE_BITS), "Not enough type bits");
static_assert(INDEX_BITS <= DATA_BITS, "Not enough data bits to store an index");

typedef TDATA  TINDEX;
typedef TINDEX CELL_INDEX;
typedef TINDEX SYMBOL_INDEX;
typedef TINDEX FUNC_INDEX;
typedef TINDEX STRING_INDEX;

struct Cell
{
    Type     _type : TYPE_BITS;
    uint32_t _tags : TAG_BITS;
    uint32_t _next : INDEX_BITS;
    uint32_t _data;
};

struct SymbolInfo
{
    string _ident;
    TINDEX _primIndex;
    CELL_INDEX _cellIndex;

    SymbolInfo() : _primIndex(0), _cellIndex(0) {}
};

typedef vector<CELL_INDEX> ArgumentList;

class Runtime;
typedef CELL_INDEX (Runtime::*PrimitiveFunc)(const ArgumentList& args);

struct PrimitiveInfo
{
    string _name;
    int _numArgs;
    PrimitiveFunc _func;
};

struct StringInfo
{
    string _str;
    int _refCount;

    StringInfo() : _refCount(0) {}
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
    vector<PrimitiveInfo> _primitive;

    vector<Cell> _cell;
    CELL_INDEX _cellFreeList;

    SlotPool<SymbolInfo> _symbol;
    std::unordered_map<THASH, SYMBOL_INDEX> _globalScope;

    SlotPool<StringInfo> _string;
    std::unordered_map<THASH, STRING_INDEX> _stringTable;

    typedef unordered_map<SYMBOL_INDEX, CELL_INDEX> Scope;
    vector<Scope> _environment;

    CELL_INDEX  _nil;
    CELL_INDEX  _true;
    CELL_INDEX  _quote;

    SYMBOL_INDEX GetSymbolIndex(const char* ident);
    CELL_INDEX   RegisterSymbol(const char* ident);
    CELL_INDEX   RegisterPrimitive(const char* ident, PrimitiveFunc func);
    void RaiseRuntimeError(const char* msg);

    CELL_INDEX AllocateCell(Type Type);
    void ExpandCellTable();
    void FreeCell(CELL_INDEX index);
    void MarkCellsInUse(CELL_INDEX index);
    size_t CollectGarbage();

    // Literals.cpp

    int LoadIntLiteral(CELL_INDEX index);
    void StoreIntLiteral(CELL_INDEX index, int value);

    float LoadFloatLiteral(CELL_INDEX index);
    void StoreFloatLiteral(CELL_INDEX index, float value);

    string LoadStringLiteral(CELL_INDEX index);
    void StoreStringLiteral(CELL_INDEX index, const char* str);

    // Primitives.cpp

    CELL_INDEX ATOM(const ArgumentList& args);
    CELL_INDEX CAR(const ArgumentList& args);
    CELL_INDEX CDR(const ArgumentList& args);
    CELL_INDEX COND(const ArgumentList& args);
    CELL_INDEX CONS(const ArgumentList& args);
    CELL_INDEX EQ(const ArgumentList& args);
    CELL_INDEX EVAL(const ArgumentList& args);
    CELL_INDEX LET(const ArgumentList& args);
    CELL_INDEX PRINT(const ArgumentList& args);
    CELL_INDEX SETQ(const ArgumentList& args);

public:
    Runtime();
    ~Runtime();

    CELL_INDEX EncodeSyntaxTree(const NodeRef& root);
    CELL_INDEX EvaluateCell(CELL_INDEX cellIndex);

    string GetPrintedValue(CELL_INDEX index);
};
