// YellowLISP (c) 2020 Stuart Riffle (MIT license)

#pragma once
#include "SlotPool.h"
#include "StaticVector.h"
#include "Hash.h"
#include "Parser.h"
#include "Errors.h"
#include "Console.h"
#include "Coverage.h"



enum SymbolType
{
    SYMBOL_INVALID,
    SYMBOL_RESERVED, // keyword?
    SYMBOL_PRIMITIVE,
    SYMBOL_VARIABLE,
    SYMBOL_FUNCTION,
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
    CELLID  _symbolCell;
    CELLID  _valueCell;
    CELLID  _macroBindings;
};

struct SymbolInfo
{
    uint64_t    _immutable : 1;
    uint32_t    _
    uint32_t    _identHash;
};

#define IS_NUMERIC_TYPE(_IDX)   ((_cell[_IDX]._type == TYPE_INT) ||(_cell[_IDX]._type == TYPE_REAL))

struct CallTarget
{
    PRIMIDX _primitiveIndex;
    CELLID _lambdaCell;
    bool _evaluateArgs = true;

    inline bool IsValid() const { return _primitiveIndex.IsValid() || _lambdaCell.IsValid(); }
};


typedef StaticVector<CELLID> CELLVEC; 

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
        (args); \
        static volatile int preventingWarning = 1; \
        if (preventingWarning) \
            RAISE_ERROR(ERROR_RUNTIME_NOT_IMPLEMENTED); \
        return 0; \
    }

#define GARBAGE_COLLECTION_THRESH (0.10f)   // GC is triggered when the cell table has this much free space left
#define CELL_TABLE_EXPAND_THRESH  (0.25f)   // If the GC doesn't free up at least this much space, expand the table 

typedef unordered_map<SYMBOLIDX, CELLID, SYMBOLIDX::Hash> Scope;
typedef vector<Scope*> ScopeStack;

struct ScopeGuard
{
    ScopeStack& _scopeStack;

    ScopeGuard(ScopeStack& scopeStack, Scope* scope) : _scopeStack(scopeStack)
    {
        _scopeStack.push_back(scope);
        
    }
    ~ScopeGuard()
    {
        _scopeStack.pop_back();
        
    }
};

template<typename KEY, typename VAL> using HashTable = std::unordered_map<KEY, VAL>;


class Runtime
{
    Console* _console;

    friend class CELLITER;
    vector<Cell> _cell;
    uint32_t     _cellFreeList;
    int          _cellFreeCount;

    SlotPool<SymbolInfo, SYMBOLIDX> _symbol;
    SlotPool<StringInfo, STRINGIDX> _string;

    HashTable<STRINGHASH, SYMBOLIDX> _globalScope;
    HashTable<STRINGHASH, STRINGIDX> _stringTable;

    ScopeStack _environment;

    vector<PrimitiveInfo> _primitive;

    CELLID  _null;
    CELLID  _dot;
    CELLID  _true;
    CELLID  _false;
    CELLID  _eval;
    CELLID  _quote;
    CELLID  _unquote;
    CELLID  _quasiquote;

    bool    _garbageCollectionNeeded;

    SYMBOLIDX StoreSymbol(const char* ident, CELLID cellIndex, SymbolType symbolType, STRINGHASH hash = 0);
    SYMBOLIDX GetSymbolIndex(const char* ident);
    CELLID   RegisterReserved(const char* ident);
    CELLID   RegisterPrimitive(const char* ident, PrimitiveFunc func, SymbolFlags flags = SYMBOLFLAG_NONE);
    size_t   ExtractList(CELLID index, CELLVEC* dest);

    bool TestCellsEQL(CELLID a, CELLID b, bool strict);
    bool TestStructureEQUAL(CELLID a, CELLID b, bool strict);

    CELLID GetScopeOverride(SYMBOLIDX symbolIndex);
    CELLID ApplyFunction(const CallTarget& callTarget, CELLID argList);
    void   BindScopeMappings(CELLID bindingList, CELLID valueList, bool evaluate, Scope* destScope);
    CELLID CallPrimitive(PRIMIDX primIndex, CELLID argCellIndex, bool evaluateArgs);
    CELLID ExpandQuasiquoted(CELLID macroBodyCell, int level = 0);
    bool IsTruthy(CELLID index);

    // CellTable.cpp

    void ExpandCellTable();

    CELLID AllocateCell(CellType Type);
    CELLID GenerateList(const CELLVEC& elements);
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

    CELLID APPLY(const CELLVEC& args);
    CELLID ATOM(const CELLVEC& args);
    CELLID CAR(const CELLVEC& args);
    CELLID CDR(const CELLVEC& args);
    CELLID COND(const CELLVEC& args);
    CELLID CONS(const CELLVEC& args);
    CELLID DEFUN(const CELLVEC& args);
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

    CELLID Help(const CELLVEC&);
    CELLID Exit(const CELLVEC& args);
    CELLID RunGC(const CELLVEC&);

    CELLID EncodeTreeNode(const NodeRef& root);

public:
    Runtime(Console* console); 
    ~Runtime();

    CELLID EncodeSyntaxTree(const NodeRef& root);
    CELLID EvaluateCell(CELLID cellIndex);
    void HandleGarbage();

    string GetPrintedValue(CELLID index);
};
