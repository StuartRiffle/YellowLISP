// YellowLISP (c) 2019 Stuart Riffle (MIT license)

#pragma once
#include "SlotPool.h"
#include "Hash.h"
#include "Parser.h"
#include "Errors.h"

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

enum SymbolType
{
    SYMBOL_INVALID,
    SYMBOL_RESERVED,
    SYMBOL_PRIMITIVE,
    SYMBOL_VARIABLE,
    SYMBOL_FUNCTION,
    SYMBOL_MACRO
};

struct SymbolInfo
{
    SymbolType _type;
    string _ident;
    string _comment;
    TINDEX _primIndex;
    CELL_INDEX _symbolCell;
    CELL_INDEX _valueCell;
    CELL_INDEX _bindingListCell;

    SymbolInfo() : _type(SYMBOL_INVALID), _primIndex(0), _symbolCell(0), _valueCell(0), _bindingListCell(0) {}
};

#define VALID_CELL(_IDX) (((_IDX) != 0) && ((_IDX) != _nil))


typedef vector<CELL_INDEX> ArgumentList;

class Runtime;
typedef CELL_INDEX (Runtime::*PrimitiveFunc)(const ArgumentList& args);

struct PrimitiveInfo
{
    string _name;
    int _numArgs;
    PrimitiveFunc _func;
};

struct FunctionInfo
{
    string _name;
    bool _isMacro;
    CELL_INDEX _argListCell;
    CELL_INDEX _bodyCell;

    FunctionInfo() : _isMacro(0), _argListCell(0), _bodyCell(0) {}
};

struct StringInfo
{
    string _str;
    int _refCount;

    StringInfo() : _refCount(0) {}
};

#define STUB_UNIMPLEMENTED(_FUNCNAME) \
    CELL_INDEX Runtime::_FUNCNAME(const ArgumentList& args) \
    { \
        (args); \
        RAISE_ERROR(ERROR_RUNTIME_NOT_IMPLEMENTED); \
        return 0; \
    }

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
    CELL_INDEX  _unquote;
    CELL_INDEX  _quasiquote;
    CELL_INDEX  _defmacro;
    CELL_INDEX  _defun;
    CELL_INDEX  _lambda;

    SYMBOL_INDEX GetSymbolIndex(const char* ident);
    CELL_INDEX   RegisterSymbol(const char* ident);
    CELL_INDEX   RegisterPrimitive(const char* ident, PrimitiveFunc func);
    vector<CELL_INDEX> ExtractList(CELL_INDEX index);

    // CellTable.cpp

    CELL_INDEX AllocateCell(Type Type);
    void ExpandCellTable();
    void FreeCell(CELL_INDEX index);

    void MarkCellsInUse(CELL_INDEX index);
    size_t CollectGarbage();

    // CellGraph.cpp

    void FormatCellLabel(CELL_INDEX cellIndex, std::stringstream& ss, set<CELL_INDEX>& cellsDone, set<SYMBOL_INDEX>& symbolsDone, bool expandSymbols = false);
    void FormatSymbolLabel(SYMBOL_INDEX symbolIndex, std::stringstream& ss, set<CELL_INDEX>& cellsDone, set<SYMBOL_INDEX>& symbolsDone);
    void DumpCellGraph(CELL_INDEX cellIndex, const string& filename, bool expandSymbols = false);

    // Literals.cpp

    int LoadIntLiteral(CELL_INDEX index);
    void StoreIntLiteral(CELL_INDEX index, int value);

    float LoadFloatLiteral(CELL_INDEX index);
    void StoreFloatLiteral(CELL_INDEX index, float value);

    double LoadNumericLiteral(CELL_INDEX index);
    CELL_INDEX CreateNumericLiteral(double value);

    string LoadStringLiteral(CELL_INDEX index);
    void StoreStringLiteral(CELL_INDEX index, const char* str);

    // Primitives.cpp

    CELL_INDEX ATOM(const ArgumentList& args);
    CELL_INDEX CAR(const ArgumentList& args);
    CELL_INDEX CDR(const ArgumentList& args);
    CELL_INDEX COND(const ArgumentList& args);
    CELL_INDEX CONS(const ArgumentList& args);
    CELL_INDEX DEFUN(const ArgumentList& args);
    CELL_INDEX DEFMACRO(const ArgumentList& args);
    CELL_INDEX EQ(const ArgumentList& args);
    CELL_INDEX EVAL(const ArgumentList& args);
    CELL_INDEX LAMBDA(const ArgumentList& args);
    CELL_INDEX LESS(const ArgumentList& args);
    CELL_INDEX LET(const ArgumentList& args);
    CELL_INDEX LIST(const ArgumentList& args);
    CELL_INDEX PROGN(const ArgumentList& args);
    CELL_INDEX SETQ(const ArgumentList& args);

    // Math.cpp

    CELL_INDEX ADD(const ArgumentList& args);
    CELL_INDEX SUB(const ArgumentList& args);
    CELL_INDEX MUL(const ArgumentList& args);
    CELL_INDEX DIV(const ArgumentList& args);
    CELL_INDEX MOD(const ArgumentList& args);
    CELL_INDEX ROUND(const ArgumentList& args);
    CELL_INDEX TRUNCATE(const ArgumentList& args);
    CELL_INDEX FLOOR(const ArgumentList& args);
    CELL_INDEX CEILING(const ArgumentList& args);
    CELL_INDEX MIN(const ArgumentList& args);
    CELL_INDEX MAX(const ArgumentList& args);
    CELL_INDEX EXP(const ArgumentList& args);
    CELL_INDEX EXPT(const ArgumentList& args);
    CELL_INDEX LOG(const ArgumentList& args);
    CELL_INDEX SQRT(const ArgumentList& args);
    CELL_INDEX ABS(const ArgumentList& args);

    CELL_INDEX SIN(const ArgumentList& args);
    CELL_INDEX SINH(const ArgumentList& args);
    CELL_INDEX ASIN(const ArgumentList& args);
    CELL_INDEX ASINH(const ArgumentList& args);
    CELL_INDEX COS(const ArgumentList& args);
    CELL_INDEX COSH(const ArgumentList& args);
    CELL_INDEX ACOS(const ArgumentList& args);
    CELL_INDEX ACOSH(const ArgumentList& args);
    CELL_INDEX TAN(const ArgumentList& args);
    CELL_INDEX TANH(const ArgumentList& args);
    CELL_INDEX ATAN(const ArgumentList& args);
    CELL_INDEX ATANH(const ArgumentList& args);

    CELL_INDEX PRINT(const ArgumentList& args);
    CELL_INDEX RANDOM(const ArgumentList& args);

    CELL_INDEX LISTP(const ArgumentList& args);
    CELL_INDEX NUMBERP(const ArgumentList& args);
    CELL_INDEX FLOATP(const ArgumentList& args);
    CELL_INDEX STRINGP(const ArgumentList& args);
    CELL_INDEX ENDP(const ArgumentList& args);

    /*
    INCF
    DECF

    DECLARE
    DECLAIM

    DEFVAR
    DEFUNC
    DEFCONSTANT
    DEFMACRO

    NOT
    AND
    OR
    WHEN
    UNLESS
    IF
    CASE

    SETF
    SETQ

    LET
    FLET

    PROG
    PROGN

    LOGAND
    LOGIOR
    LOGXOR
    LOGNOR
    LOGEQV

    DOLIST
    DOTIMES
    LOOP


    LIST
    APPEND
    LENGTH
    REVERSE
    MEMBER
    POSITION
    FIND
    COUNT
    REMOVE
    */

    // Commands.cpp

    CELL_INDEX Help(const ArgumentList& args);
    CELL_INDEX Exit(const ArgumentList& args);

public:
    Runtime();
    ~Runtime();

    CELL_INDEX EncodeSyntaxTree(const NodeRef& root);
    CELL_INDEX EvaluateCell(CELL_INDEX cellIndex);

    string GetPrintedValue(CELL_INDEX index);
};
