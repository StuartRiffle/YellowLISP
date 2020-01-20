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

enum ValueType : uint8_t
{
    TYPE_INVALID = 0,
    TYPE_POINTER,       
    TYPE_PROCEDURE,     
    TYPE_ENVIRONMENT,   

    TYPE_SYMBOL,        
    TYPE_VECTOR,        
    TYPE_BYTEVECTOR,    

    TYPE_CHAR,          
    TYPE_STRING,        

    TYPE_FIXNUM,
    TYPE_FLONUM,
    TYPE_NUMERIC,      

    TYPE_EOF_OBJECT,    
    TYPE_RECORD,

    TYPE_COUNT
};



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
    const uint64_t NAN_BITS         = 0x7ff8'0000'0000'0000;    //   12  All must be set for boxed values
    const uint64_t BOXED_TYPE_MASK  = 0x0007'8000'0000'0000;    //    4  Boxed value type
    const uint64_t BOXED_DATA_MASK  = 0x0000'7FFF'FFFF'FFFE;    //   46  Boxed value data, format depends on type
    const uint64_t REACHABLE_FLAG   = 0x0000'0000'0000'0001;    //    1  Must be kept in LSB, used by both boxed values and real floats
                                                                // ----
                                                                //   64
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
