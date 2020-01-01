// YellowLISP (c) 2020 Stuart Riffle (MIT license)

#pragma once
#include "SlotPool.h"
#include "Hash.h"
#include "Parser.h"
#include "Errors.h"
#include "Console.h"

enum CellType : uint32_t  
{                       // The data stored in the cell is...
    TYPE_VOID,
    TYPE_FREE,          //   a link to the next free cell
    TYPE_CONS,          //   an index into the cell table
    TYPE_LAMBDA,        //   an index into the cell table for the binding list
    TYPE_SYMBOL,        //   an index into the symbol table
    TYPE_STRING,        //   a tiny string literal, or an index into the string table
    TYPE_INT,           //   a signed integer literal
    TYPE_FLOAT,         //   an IEEE floating point literal

    TYPE_COUNT,
    TYPE_BITS = 3
};

enum Tags
{
    TAG_GC_MARK    = 1 << 0,   // Marks cell as "reachable" during mark-and-sweep garbage collection
    TAG_EMBEDDED   = 1 << 1,   // The value is contained in the cell (as opposed to indexed)

    TAG_BITS = 3
};

const int HEADER_BITS   = sizeof(uint32_t) * 8;
const int DATA_BITS     = sizeof(uint32_t) * 8;
const int METADATA_BITS = TYPE_BITS + TAG_BITS;
const int INDEX_BITS    = HEADER_BITS - METADATA_BITS;
const int NIL_CELL      = 1;

static_assert(TYPE_COUNT <= (1 << TYPE_BITS), "Not enough type bits");
static_assert(INDEX_BITS <= DATA_BITS, "Not enough data bits to store an index");

struct Cell
{
    CellType _type : TYPE_BITS;
    uint32_t _tags : TAG_BITS;
    uint32_t _next : INDEX_BITS;
    uint32_t _data;                 

    Cell() : _type(TYPE_VOID), _tags(0), _next(0), _data(0) {}
};

// Indices are strongly typed to help avoid silly mistakes

template<typename TAG = void*>
class INDEX
{
    enum { INVALID = ~uint32_t(0) };
    uint32_t _index;
public:
    inline INDEX(uint32_t index = INVALID) : _index(index) {}
    inline bool IsValid() const { return (_index != INVALID); }
    inline operator uint32_t() { return _index; }
};

class CELLID   : public INDEX<CELLID>   { public: using INDEX::INDEX; };
class SYMBOLIDX : public INDEX<SYMBOLIDX> { public: using INDEX::INDEX; };
class STRINGIDX : public INDEX<STRINGIDX> { public: using INDEX::INDEX; };
class PRIMIDX   : public INDEX<PRIMIDX>   { public: using INDEX::INDEX; };

// 

enum SymbolType
{
    SYMBOL_INVALID,
    SYMBOL_RESERVED,
    SYMBOL_PRIMITIVE,
    SYMBOL_VARIABLE,
    SYMBOL_FUNCTION,
    SYMBOL_MACRO
};

enum SymbolFlags
{
    SYMBOLFLAG_NONE = 0,
    SYMBOLFLAG_DONT_EVAL_ARGS = 1 << 0,  
};

struct SymbolInfo
{
    SymbolType  _type   = SYMBOL_INVALID;
    SymbolFlags _flags  = SYMBOLFLAG_NONE;

    string  _ident;
    string  _comment;
    PRIMIDX _primIndex;
    CELLID _symbolCell;
    CELLID _valueCell;
    CELLID _macroBindings;
};

#define IS_NUMERIC_TYPE(_IDX)   ((_cell[_IDX]._type == TYPE_INT) ||(_cell[_IDX]._type == TYPE_FLOAT))




template<class T, int ELEMENTS = 16>
class StaticVector
{
    T _embedded[ELEMENTS];
    std::vector<T> _overflow;
    int _count = 0;

public:
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

typedef StaticVector<CELLID> CELLVEC; 

class CELLITER
{
    Runtime& _runtime;
    CELLID   _index;

public:
    CELLITER(Runtime& runtime, CELLID index) : _runtime(runtime), _index(index) {}

    inline Cell& operator*()  { return _runtime._cell[_index]; }
    inline Cell& operator->() { return _runtime._cell[_index]; }

    inline Cell& GetNext()
    { 
        CELLID next = _runtime._cell[_index]._next;
        assert(next);

        return _runtime._cell[next];
    }

    inline Cell& operator++(int)
    {
        Cell& curr = _runtime._cell[_index];

        assert(_index);
        assert(_index != NIL_CELL);
        _index = _runtime._cell[_index]._next;
        assert(_index);

        return curr;
    }
};




class Runtime;
typedef CELLID (Runtime::*PrimitiveFunc)(const CELLVEC& args);

struct PrimitiveInfo
{
    string _name;
    PrimitiveFunc _func = nullptr;
};

struct StringInfo
{
    string _str;
    int _refCount = 0;
};

#define STUB_UNIMPLEMENTED(_FUNCNAME) \
    CELLID Runtime::_FUNCNAME(const CELLVEC& args) \
    { \
        if (args.max_size()) /* prevent unreachable code warning */ \
            RAISE_ERROR(ERROR_RUNTIME_NOT_IMPLEMENTED); \
        return 0; \
    }

#define GARBAGE_COLLECTION_THRESH (0.10f)   // GC is triggered when the cell table has this much free space left
#define CELL_TABLE_EXPAND_THRESH  (0.25f)   // If the GC doesn't free up at least this much space, expand the table 

class Runtime
{
    Console* _console;

    friend class CELLITER;
    vector<Cell> _cell;
    CELLID       _cellFreeList;
    int          _cellFreeCount;

    SlotPool<SymbolInfo> _symbol;
    std::unordered_map<THASH, SYMBOLIDX> _globalScope;

    SlotPool<StringInfo> _string;
    std::unordered_map<THASH, STRINGIDX> _stringTable;

    typedef unordered_map<SYMBOLIDX, CELLID> Scope;
    vector<Scope> _environment;

    vector<PrimitiveInfo> _primitive;


    CELLID  _nil;
    CELLID  _dot;
    CELLID  _true;
    CELLID  _eval;
    CELLID  _quote;
    CELLID  _unquote;
    CELLID  _quasiquote;

    bool _garbageCollectionNeeded;

    SYMBOLIDX StoreSymbol(const char* ident, CELLID cellIndex);
    SYMBOLIDX GetSymbolIndex(const char* ident);
    CELLID   RegisterReserved(const char* ident);
    CELLID   RegisterPrimitive(const char* ident, PrimitiveFunc func, SymbolFlags flags = SYMBOLFLAG_NONE);
    vector<CELLID> ExtractList(CELLID index);

    bool TestCellsEQL(CELLID a, CELLID b, bool strict);
    bool TestStructureEQUAL(CELLID a, CELLID b, bool strict);

    Scope BindArguments(CELLID bindingList, CELLID argList, bool evaluateArgs);
    CELLID CallPrimitive(PRIMIDX primIndex, CELLID argCellIndex, bool evaluateArgs);
    CELLID ExpandQuasiquoted(CELLID macroBodyCell, int level = 0);

    // CellTable.cpp

    void InitCellTable(size_t size = 8);
    void ExpandCellTable();

    CELLID AllocateCell(CellType Type);
    CELLID GenerateList(const vector<CELLID>& elements);
    void FreeCell(CELLID index);

    void DebugValidateCells();
    void MarkCellsInUse(CELLID index);
    size_t CollectGarbage();

    // CellGraph.cpp

    void FormatCellLabel(CELLID cellIndex, std::stringstream& ss, set<CELLID>& cellsDone, set<SYMBOLIDX>& symbolsDone, bool expandSymbols = false);
    void FormatSymbolLabel(SYMBOLIDX symbolIndex, std::stringstream& ss, set<CELLID>& cellsDone, set<SYMBOLIDX>& symbolsDone);
    string GenerateCellGraph(CELLID cellIndex, bool expandSymbols);
    void DumpCellGraph(CELLID cellIndex, bool expandSymbols);

    // Literals.cpp

    int LoadIntLiteral(CELLID index);
    void StoreIntLiteral(CELLID index, int value);

    float LoadFloatLiteral(CELLID index);
    void StoreFloatLiteral(CELLID index, float value);

    double LoadNumericLiteral(CELLID index);
    CELLID CreateNumericLiteral(double value, bool storeAsInt = false);

    string LoadStringLiteral(CELLID index);
    void StoreStringLiteral(CELLID index, const char* str);

    // Primitives.cpp

    CELLID ATOM(const CELLVEC& args);
    CELLID CAR(const CELLVEC& args);
    CELLID CDR(const CELLVEC& args);
    CELLID COND(const CELLVEC& args);
    CELLID CONS(const CELLVEC& args);
    CELLID DEFUN(const CELLVEC& args);
    CELLID DEFMACRO(const CELLVEC& args);
    CELLID EQ(const CELLVEC& args);
    CELLID EQL(const CELLVEC& args);
    CELLID EQLOP(const CELLVEC& args);
    CELLID EQUAL(const CELLVEC& args);
    CELLID EQUALP(const CELLVEC& args);
    CELLID EVAL(const CELLVEC& args);
    CELLID LAMBDA(const CELLVEC& args);
    CELLID LESS(const CELLVEC& args);
    CELLID LET(const CELLVEC& args);
    CELLID LIST(const CELLVEC& args);
    CELLID PROGN(const CELLVEC& args);
    CELLID SETQ(const CELLVEC& args);

    // Math.cpp

    CELLID ADD(const CELLVEC& args);
    CELLID SUB(const CELLVEC& args);
    CELLID MUL(const CELLVEC& args);
    CELLID DIV(const CELLVEC& args);
    CELLID MOD(const CELLVEC& args);
    CELLID ROUND(const CELLVEC& args);
    CELLID TRUNCATE(const CELLVEC& args);
    CELLID FLOOR(const CELLVEC& args);
    CELLID CEILING(const CELLVEC& args);
    CELLID MIN(const CELLVEC& args);
    CELLID MAX(const CELLVEC& args);
    CELLID EXP(const CELLVEC& args);
    CELLID EXPT(const CELLVEC& args);
    CELLID LOG(const CELLVEC& args);
    CELLID SQRT(const CELLVEC& args);
    CELLID ABS(const CELLVEC& args);

    CELLID SIN(const CELLVEC& args);
    CELLID SINH(const CELLVEC& args);
    CELLID ASIN(const CELLVEC& args);
    CELLID ASINH(const CELLVEC& args);
    CELLID COS(const CELLVEC& args);
    CELLID COSH(const CELLVEC& args);
    CELLID ACOS(const CELLVEC& args);
    CELLID ACOSH(const CELLVEC& args);
    CELLID TAN(const CELLVEC& args);
    CELLID TANH(const CELLVEC& args);
    CELLID ATAN(const CELLVEC& args);
    CELLID ATANH(const CELLVEC& args);

    CELLID PRINT(const CELLVEC& args);
    CELLID RANDOM(const CELLVEC& args);

    CELLID LISTP(const CELLVEC& args);
    CELLID NUMBERP(const CELLVEC& args);
    CELLID FLOATP(const CELLVEC& args);
    CELLID STRINGP(const CELLVEC& args);
    CELLID ENDP(const CELLVEC& args);

    // Commands.cpp

    CELLID Help(const CELLVEC& args);
    CELLID Exit(const CELLVEC& args);
    CELLID RunGC(const CELLVEC& args);

    CELLID EncodeTreeNode(const NodeRef& root);

public:
    Runtime(Console* console); 
    ~Runtime();

    CELLID EncodeSyntaxTree(const NodeRef& root);
    CELLID EvaluateCell(CELLID cellIndex);
    void HandleGarbage();

    string GetPrintedValue(CELLID index);
};
