#pragma once


//  meta
//  symbol table
// 

typedef std::complex<float32_t> complex_t;

struct rational_t
{
    int32_t     _num;
    uint32_t    _den;
};

struct complex_t
{
    float32_t   _real;
    float32_t   _imag;
};

enum ValueType 
{
    TYPE_INVALID = 0,
    TYPE_POINTER,       // index
    TYPE_PROCEDURE,     // pair?
    TYPE_ENVIRONMENT,   // obj

    TYPE_SYMBOL,        // index
    TYPE_VECTOR,        // obj
    TYPE_BLOB,    

    TYPE_CHAR,          // immed
    TYPE_STRING,        // obj

    TYPE_FIXNUM,        // immed
    TYPE_FLONUM,        // immed
    TYPE_NUMERIC,       // obj

    TYPE_EOF_OBJECT,    // obj
    TYPE_RECORD,        // obj

    TYPE_COUNT
};

enum CdrType
{
    CDR_FOLLOWS,
    CDR_SKIPPED,
};

union Field
{
    float       _float;
    int32_t     _int;
    uint32_t    _raw;

    uint32_t GetType() const    { return (_raw & TYPE_MASK) >> TYPE_SHIFT; }
    void SetType(uint32_t type) { _raw = (_raw & ~TYPE_MASK) | (type << TYPE_SHIFT); }

    uint32_t GetFlags(uint32_t mask) const { return (_raw & mask); }
    void SetFlags(uint32_t mask) { _raw |= mask; }

    int32_t GetInt() const   { int32_t val = _raw; val >>= DATA_SHIFT; return val; }
    void SetInt(int32_t val) { _raw = (val << DATA_SHIFT) | (_raw & ~DATA_MASK); }

    float GetFloat() const { }

};



























/*

1 1 5 26 5 26 5/27 for all

8 8 24 24




pointer


index
object


float
float/float
double
double/double


// cdr coded:

[link32][data32]    fixnum, flonum, char
[link32][data64...]  



two byte header
indices are 3 bytes
immediates are 4

Non-chain cells can fit in 8 with one byte header


Index field is 5/27


TYPE_POINTER,       // index
TYPE_PROCEDURE,     // pair?
TYPE_ENVIRONMENT,   // obj
TYPE_SYMBOL,        // index
TYPE_VECTOR,        // obj
TYPE_BLOB,    
TYPE_CHAR,          // immed
TYPE_STRING,        // obj
TYPE_FIXNUM,        // immed
TYPE_FLONUM,        // immed
TYPE_NUMERIC,       // obj
TYPE_EOF_OBJECT,    // obj
TYPE_RECORD,        // obj


*/








/// ValueBox uses "NaN boxing" to implement a union of
/// double precision float point numbers and tagged 
/// pointers. Basically, if the double is encoded as a
/// NaN, the lower 51 bits are unused, and we can use them
/// to represent a pointer.
///
/// A lot of existing hardware only actually uses 48 bits of
/// address space, so 51 bits is enough to store any
/// pointer value directly, and still have a few bits left 
/// over for tag/type information. Newer hardware has
/// a wider effective address space though, and that trick 
/// doesn't work anymore. 
///
/// So instead of storing pointers directly, we encode them
/// as an *offset* into a heap that we manage ourselves.
/// There are multiple heaps (for different object types), so
/// to resolve a pointer the offset has to be added to the base
/// address of the appropriate heap.

union ValueBox
{
private:
    double          _fp;
    uint64_t        _raw;
                                                                
    const uint64_t SIGN_BIT         = 0x8000'0000'0000'0000;    //    1  Ignored by boxed values
    const uint64_t NAN_BITS         = 0x7ff8'0000'0000'0000;    //   12  All bits must be set for boxed values
    const uint64_t BOXED_TYPE_MASK  = 0x0007'8000'0000'0000;    //    4  Boxed value type
    const uint64_t BOXED_DATA_MASK  = 0x0000'7FFF'FFFF'FFFE;    //   46  Boxed value data, format depends on type
    const uint64_t REACHABLE_FLAG   = 0x0000'0000'0000'0001;    //    1  Must be kept in LSB, used by both boxed values and real floats
                                                                // ----
                                                                //   64




/*

*/

public:
    inline bool IsBoxedValue() const 
    {
        bool nanEncoding = (_raw & NAN_BITS) == NAN_BITS;

        const uint64_t payloadMask = (BOXED_TYPE_MASK | BOXED_DATA_MASK);
        bool hasPayload = (_raw & payloadMask) != 0;

        return nanEncoding && hasPayload;
    }

    inline bool IsFloat() const
    {
        return !IsBoxedValue();
    }

    inline double GetFloat() const
    {
        assert(IsFloat());
        assert(_raw & REACHABLE_BIT == 0);

        return _fp;
    }

    inline void SetFloat(double val)
    {
        // The least significant bit is reserved for use during the first pass of
        // garbage collection, to mark this ValueBox as "reachable" and prevent 
        // it from being recycled. During normal operation, the LSB must remain zero,
        // so floating point values are rounded when they are stored using Dekker's
        // algorithm from "A Floating-Point Technique for Extending the Available Precision".

        double t = val * 3.0;
        _fp = t - (t - val);

        assert(_raw & REACHABLE_BIT == 0);
        assert(IsFloat());
    }

    inline int64_t GetInteger() const
    {
        assert(GetType() == TYPE_FIXNUM);

        int64_t signExtended = ((int64_t) _raw << 17) >> 17;
        return signExtended;
    }

    inline ValueType GetType() const
    {
        if (IsBoxedValue())
            return (ValueType) (_raw & BOXED_TYPE_MASK) >> 47;

        return TYPE_FLONUM;
    }

    inline uint64_t GetBoxedData() const
    {
        assert(IsBoxedValue());
        return (_raw & BOXED_DATA_MASK) >> 1;
    }

    inline void SetBoxedData(ValueType type, uint64_t data)
    {
        _raw = 
            NAN_BITS |
            (((uint64_t) type << 47) & BOXED_TYPE_MASK) |
            ((data << 1) & BOXED_DATA_MASK);
    }

    inline void MarkReachable()
    {
        assert(_raw & REACHABLE_BIT == 0);
        _raw |= REACHABLE_BIT;
    }

    inline bool TestAndClearReachable()
    {
        bool reachable = (_raw & REACHABLE_BIT) != 0;
        _raw &= ~REACHABLE_BIT;
        return reachable;
    }
};


struct ValueMeta
{
    uint8_t     _type     : 4;  // TYPE_* enum value
    uint8_t     _exact    : 1;  // This value is exact
    uint8_t     _mutable  : 1;  // This value can be modified
    uint8_t     _embedded : 1;  // This value fits in the cons cell (i.e. not a pointer)
};

struct ValueVector : public RefCounted
{
    size_t          _count;
    Up<ValueData>   _data;
    Up<ValueMeta>   _meta;
};

struct Symbol : public RefCounted
{
    string          _ident;
    hash64_t        _hash;
};



struct TypedValue
{
    uint32_t    _val;
    uint8_t     _type;
};


struct Cell
{
    Value       _car;
    Value       _cdr;
};

struct CellBlock
{
    Cell        _cell[7];       // CONS cells
    ValueMeta   _meta[14];      // Type and tag bits for each half (CAR/CDR) of the cells
    uint8_t     _allocated;     // One bit per cell, marks it as in-use
    uint8_t     _reachable;     // One bit per cell, used as the "mark" during mark-and-sweep garbage collection

    CellBlock() : _allocated(0), _reachable(0) {}
};

inline CellBlock* GetCellBlock(const Cell* cell)
{
    uintptr_t cellAddr = (uintptr_t) cell;
    const uintptr_t cellMask = sizeof(CellBlock) - 1;
    CellBlock* cellBlock = (CellBlock*) (cellAddr & ~cellMask); 

    return cellBlock;
}

inline ValueMeta* GetValueMeta(const Value* value)
{
    CellBlock* cellBlock = GetCellBlock(cell);
    size_t cellIndex = value - (Value*) cellBlock;
    assert(cellIndex < 14);

    return cellBlock->_meta + cellIndex;
}


class TypedMemory
{
    TypedBlock*  _core;

public:
    inline void GetValue(uint32_t addr, TypedValue& dest) const
    {
        uint32_t blockIndex = addr >> 4;
        uint32_t valueOffset = addr & 0xF;

        const CellBlock* __restrict block = _core + blockIndex;
        dest._value = block->_value[valueOffset];

        uint8_t typePair = block->_typeInfo[valueOffset >> 1];
        dest._type = (addr & 1)? (typePair >> 4) : (typePair & 0xF);
    }

    inline void SetValue(uint32_t addr, const TypedValue& src)
    {
        uint32_t blockIndex  = addr >> 4;
        uint32_t valueOffset = addr & 0xF;

        CellBlock* block __restrict = _core + blockIndex;
        block->_value[valueOffset] = src._value;

        uint8_t& typePair = block->_typeInfo[valueOffset >> 1];
        typePair = (addr & 1)? 
            ((typePair & 0x0F) | (src._type << 4)) :
            ((typePair & 0xF0) | src._type);
    }


    inline void GetPair(uint32_t addr, TypedValue& car, TypedValue& cdr) const
    {
        assert(addr & 1 == 0);

        uint32_t blockIndex  = addr >> 4;
        uint32_t valueOffset = addr & 0xF;

        const CellBlock* __restrict block = _core + blockIndex;
        car._value = block->_value[valueOffset];
        cdr._value = block->_value[valueOffset + 1];

        uint8_t typePair = block->_typeInfo[valueOffset >> 1];
        car._type = typePair & 0xF;
        cdr._type = typePair >> 4;
    }


};


class RefCounted
{
    int _refs = 0;
    virtual ~RefCounted() { assert(_refs == 0);}
public:
    void AddRef()  { assert(_refs > 0); _refs++; }
    void Release() { if (--_refs < 1) delete this; }
};


template<typename T>
class Ref
{
    T* _obj = nullptr;

    void Assign(T* rhs)
    {
        if (rhs)
            rhs->AddRef();

        if (_obj)
            obj->Release();

        _obj = rhs;
    }

public:
    Ref(T* obj = nullptr) { this->Assign(obj); }
    Ref(const Ref& rhs) { this->Assign(rhs._obj); }
    ~Ref() { this->Assign(nullptr); }

    T& operator->() { return *_obj; }
    const T& operator->() const { return *_obj; }
};

struct LocalScope : public RefCounted
{
    Ref<LocalScope> _outer;
    vector<Value>   _bound;
};

struct StackFrame : public RefCounted
{
    uint8_t*        _inst;      // Next instruction
    Ref<LocalScope> _scope;     // Scope chain
    Value*          _stack;     // Stack for values
    ValueMeta*      _meta;      // Parallel stack for type/tags
    string          _desc;      // Backtrace text
};



struct BytecodeBlock : public RefCounted
{
    vector<uint8_t>     _code;
    HashTable<hash_t, 
};


struct BytecodePtr
{
    Ref<BytecodeBlock>  _block;
    uint64_t            _offset;
};

struct Environment : public RefCounted
{
    const Ref<Environment> _enclosing;
};

struct Procedure : public RefCounted
{
    Ref<Environment>    _environment;
    vector<Ref<Symbol>> _formals;
    BytecodePtr         _entryPoint;
};



struct Context : public RefCounted
{
    BytecodePtr         _nextInstruction;
    Ref<Environment>    _environment;
    Ref<Procedure>      _continuation;
    Ref<Procedure>      _exceptionHandler;
    Ref<Procedure>      _beforeThunk;
    Ref<Procedure>      _afterThunk;


    vector<ValueData>   _stack;
    vector<ValueMeta>   _meta;



    void EnterLet()
    {
        bool evalBeforeBinding = !isStarForm;
        bool bindAllBeforeEval = isRecForm;
        bool bindAllAfterEval  = isStarForm;

        auto innerEnv = _vm->Alloc<Environment>();
        const auto& outerEnv = _vm->_environment;
        const auto& bindingEnv = (evalBeforeBinding? outerEnv : innerEnv);

        if (isRecForm)
            for (const auto& binding : block->bindings)
                environment->Declare(binding._var);

        for (const auto& binding : block->bindings)
        {
            // FIXME: call/cc these evals
            auto boundValue = _vm->Evaluate(binding._expression, bindingEnv);
            environment->Define(binding._var, boundValue);
        }



        struct Context
        {
            Ref<Context> _continuation;
            int _blockIndex;
            Value[]
            // tos
            // ip
            // regs[] (effectively the stack; size known aot)
            // bindings[] (values only)
        };


        // 
        /*


        (define (fact n)
            (if (< n 2) 1
                (* n (fact (- n 1))))

        @fact:              ; n
            (push-copy 1)   ; n n
            (push-int 2)    ; n n 2
            (call <)        ; n (n<2)?
            (pop-jump @r1)  ; n                 sp++; if (IsTruthy(sp[-1])) goto @R1;
            (push-copy 1)   ; n n               sp--; sp[0] = sp[1];
            (push-int 1)    ; n n 1             sp--; sp[0] = 1;
            (call -)        ; n (n-1)
            (call *)        ; result
            (ret)



            push-copy   1
            push-int    2
            call        <
            pop-jump    

        (define (fact n k)
            (if (< n 2) (k 1)
                (fact (- n 1) (lambda (partial k) (k (* part n)))))



        */



        auto context = _vm->Alloc<Context>();
        context->_outerScope = _vm->_environment;

        auto localEnv = _vm->Alloc<Environment>();
        environment->_parent = _vm->_environment;
        _vm->environment = localEnv;
        _
        auto continuation = _vm->Alloc<Continuation>();
        continuation->_nextInstruction = proc->_entryPoint;
    }
};



struct MachineState
{
    HashTable<stringhash_t, Value>  _globals;
};


/*

    (nop)

    (env-enter)
    (env-set <symbol> <value>)
    (env-leave)

    (push-constant <index>)
    (push-value <32-bit value>)
    (vector-get)
    (push-eof-object)

    (push-current 'sp)
    (push-dynamic 'ip)

    (push-copy <offset>)

    (bind


    (call)
    (call-direct)
    
    (jump)
   
    // bp-relative addressing


(define (fib n)
    (if (< n 2)
        1
        (+ (fib (- n 1)) (fib (- n 2)))))


; =foo  value of foo
; foo 

; fib

; symbol table setup

(push-string "x")

(push-empty-list)
(push-string "x")
(car)
(push-number 123.45)
(car)
(vector-from-list)



case OP_LOOKUP:
    if (EvaluatesToSelf(*_stack))
        break;
    *_stack = EvaluateCell(*_stack);


(cons 



(push-value u32)
(push-addrm
(env-lookup)

(eval %ip)





void 
{
    while (reg[MODULE] == _moduleIndex)
    {
        switch(ip)
        {
        case 514:
            DEBUG_LINE(23);
            PUSH(2, TYPE_INT);
            PUSH(sp[-2]);
            PUSH("<");
            goto L415;


            ; #441 := (< n 2)
            STACK_PUSH_IMMED(2, TYPE_INT);
            STACK_PUSH_COPY(-2);
            STACK_PUSH_IDENT("<");
            SYMBOL_LOOKUP();
            PROC_INVOKE();


            push_immed(2);
            push_copy(-1);
            push_const("<");
            lookup();
            call();

            sp[2] = sp[-1];
            sp[3] = "<";

            *++sp = 2;          *++st = TYPE_INT;
            *++sp = _stack[-2]; *++st = st[-2];
            DEBUG_LINE(26);
            *++sp = "<";
            *sp = _Lookup(*sp);
            reg[CMP] = IsTruthy(*sp--);
            if (reg[CMP]) { module = 3; block = 12; continue; }
            DEBUG_LINE(27);
            ++ip; _Invoke();
        }
    }
}




                    ; =n
(proc-enter "fib")       ; =n
(stack-push-int 2)      ; =n 2
(stack-push-copy -1)      ; =n 2 =n
(stack-push-symbol @<)     ; =n 2 =n @<
(symbol-lookup)            ; =n 2 =n addr
(proc-call)              ; =n #f
(compare-true 0)
(jump-if-so <ret>) ; =n

(stack-push-int 2)      ; =n 2
(stack-push-copy -1)      ; =n 2 =n
(stack-push-symbol @-)     ; =n 2 =n @-
(symbol-lookup)            ; =n 2 =n addr
(proc-call)              ; =n (n-2)

(stack-push-this)   ; =n (n-2) @fib
(proc-call)              ; =n f(n-2)

(stack-push-int 1)      ; =n f(n-2) 1
(stack-push-copy -2)      ; =n f(n-2) 1 =n
(stack-push-symbol @-)     ; =n f(n-2) 1 =n @-
(proc-lookup)            ; =n f(n-2) 1 =n addr-
(proc-call)              ; =n f(n-2) (n-1)

(stack-push-this)   ; =n f(n-2) (n-1) @fib
(proc-call)              ; =n f(n-2) f(n-1)

(stack-push-symbol @+)     ; =n f(n-2) f(n-1) @+
(symbol-lookup)            ; =n f(n-2) f(n-1) addr(+)
(stack-pop-copy 1)              ; result
(proc-leave)
(proc-ret)               ; FIXME: who/when to pop original =n param

<ret>:
(push-immed 1)
(ret)




(push-const <)
(lookup)


; caller

push n
push fib
call





ret is equivalent to restoring the previous state



(define (fib n k)
    (if (< n 2)
        (k 1)
        (fib (- n 1) (lambda (k part) (fib (- n 2))
        (+ (fib (- n 1)) (fib (- n 2)))))


void Fact(int n, FUNC cont)
{
    if (n == 0)
    {
        push 1
        jump cont
    }

    n = n + 1
    k = lambda (x) (k (* x n)
    push n - 1
    push ())
    jump Fact

    Fact(n - 1, [](part) { cont(part * n) });
}


; <


*/

enum OpCode : uint8_t
{
    OP_INVALID = 0,
    OP_NOP,

    OP_ENV_ENTER,         // Create a new environment, linked to the existing one, 
    OP_ENV_SET,
    OP_ENV_LEAVE,

    OP_PUSH=

    TYPE_POINTER,       // A cons cell reference                An index into _cells[]
    TYPE_PROCEDURE,     // A callable procedure                 An index into _cells[] for a pair (bindings . body)
    TYPE_ENVIRONMENT,   // An Environment object reference      An index into _environments[]
    TYPE_SYMBOL,        // A SymbolInfo object reference        An index into _symbols[]
    TYPE_VECTOR,        // A mixed-type vector of values        An index into _vectors[]
    TYPE_BYTEVECTOR,    // A vector of uint8_t                  An index into _bytevectors[]
    TYPE_CHAR,          // A UTF-32 code point                  Immediate value
    TYPE_STRING,        // A vector of TYPE_CHAR code points    An index into _vectors[]
    TYPE_FIXED_INT,     // An exact 28-bit signed integer       Immediate value
    TYPE_NUMBER,        // Any other numeric value              An index into _numbers[]
    TYPE_EOF_OBJECT,    // An end-of-file (port) marker         Type only


    OP_PUSHVALUE,

    OP_PUSHBYTES

    OP_HALT = 0xDD
};
