#pragma once

enum
{
    CAR = 0,
    CDR = 1
};

enum ValueType
{
    TYPE_INVALID = 0,
                        // This value is:                       It is represented by:
                        // --------------                       ---------------------
    TYPE_NULL,          // The empty list ()                    Type only
    TYPE_POINTER,       // A cons cell reference                An index into _cells[]
    TYPE_PROCEDURE,     // A callable procedure                 An index into _cells[] for a pair (bindings . body)
    TYPE_ENVIRONMENT,   // An Environment object reference      An index into _environs[]
    TYPE_SYMBOL,        // A SymbolInfo object reference        An index into _symbols[]
    TYPE_VECTOR,        // A mixed-type vector of values        An index into _vectors[]
    TYPE_CHAR,          // A UTF-32 code point                  Immediate value
    TYPE_STRING,        // A vector of TYPE_CHAR code points    An index into _vectors[]
    TYPE_FIXED_INT,     // An exact 28-bit signed integer       Immediate value
    TYPE_NUMBER,        // Any other numeric value              An index into _numbers[]

    TYPE_COUNT
};

struct Datum
{
    uint32_t    _type : 4;
    uint32_t    _data : 28;
};


enum TowerLevel
{
    LEVEL_INVALID = 0,

    LEVEL_INTEGER,
    LEVEL_RATIONAL,
    LEVEL_REAL,
    LEVEL_COMPLEX
};

struct Scalar
{
    bool        _big;       
    bool        _rational;  

    // The storage format used depends on the flags:
    //
    //                  rational            not rational
    //
    //      big         _bignum/_bigden     _flonum
    //      not big     _fixnum/_fixden     _flonum
    //
    // Simple integers are represented as rational numbers with 
    // a denominator of 1.
    //
    // The storage format is independent of Scheme's "exactness"

    int64_t     _fixnum;
    uint64_t    _fixden;

    bignum_t    _bignum;
    bignum_t    _bigden;

    double      _flonum;
};

class Number
{
    TowerLevel  _level   = LEVEL_INVALID;
    bool        _exact   = true;
    Scalar      _real;
    Scalar      _imag;

public:

    Number Add         (const Number& lhs, const Number& rhs) const;
    Number Subtract    (const Number& lhs, const Number& rhs) const;
    Number Multiply    (const Number& lhs, const Number& rhs) const;
    Number Gcd         (const Number& lhs, const Number& rhs) const;
    Number Lcm         (const Number& lhs, const Number& rhs) const; 

    bool    IsEqualTo
    bool    IsLessThan

    Number Abs         () const;
    Number Numerator   () const; 
    Number Denominator () const; 
    Number Floor       () const; 
    Number Ceiling     () const;
    Number Truncate    () const; 
    Number Round       () const; 
    Number Rationalize () const;

    Number RealPart    () const;
    Number ImagPart    () const;
    Number Magnitude   () const;
    Number Angle       () const;
    Number ToExact
    Number ToInexact


    Number MakeRectangular
    Number MakePolar
    Number FromString 
    Number FromStringRadix 

    bool    IsNumber
    bool    IsComplex
    bool    IsReal
    bool    IsRational
    bool    IsInteger
    bool    IsExact
    bool    IsRealValued
    bool    IsRationalValued
    bool    IsZero
    bool    IsPositive
    bool    IsNegative
    bool    IsFinite
    bool    IsNan

    Number Exp
    Number Expt
    Number Log
    Number LogBase
    Number Sin
    Number Cos
    Number Tan
    Number Asin
    Number Acos
    Number Atan
    Number Atan2
    Number Sqrt
    Number ExactIntegerSqrt

    string ToString 
    string ToStringRadix 
    string ToStringRadixPrecision
};

// Scheme rules:
// - A number is _exact 



    bignum_t    _num;
    bignum_t    _den;
    double      _real;

    //  Non-complex values are stored in _real  
    //
    //  Exact integer       _real._fixnum       _real._bignum
    //  Inexact integer     _real._fixnum       _real._bignum
    //
    //  Exact rational      _real._fixnum/den   _real._bignum/den
    //  Inexact rational    _real._flonum       _real._flonum
    //
    //  Exact real          _real._flonum       _real._flonum       
    //  Inexact real        _real._flonum       _real._flonum    
    //
    //  Exact/inexact complex   (_real, _imag)
    //
    //  Exact complex       (_real, _imag)      (_real, _imag)
    //  Inexact complex
    //  Exact real          _num/_den       _bignum/_bigden     (demoted to rational)
    //  

    // Integers with an exact representation in 64-bits are stored in _num

    int64_t _num = 0;

    // Small, exact rational values are represented as _num/_den

    uint64_t _den = 1;

    // Inexact values of any type are kept 

    double _real = 0;
    BigInteger  _num    = 0;
    BigInteger  _den    = 1;

    // 


    bool        _exact  = true;
    double      _real   = 0;
    double      _imag   = 0;
    std::

    union
    {
        int64_t     _integer;
        double      _real;      


    } _inexactValue;


};




const int TYPE_BITS = 4;
const int DATA_BITS = 28;


union BoxedValue
{
    int64_t     _integer;
    double      _real;      
    char32_t    _char;      
};

struct ConsCell
{
    Field _field[2]; // CAR and CDR

    inline bool IsAllocated() const          { return _field[CAR]._extra; }
    inline bool SetAllocated(bool allocated) { _field[CAR]._extra = allocated? 1 : 0; }

    inline bool IsReachable() const          { return _field[CDR]._extra; }
    inline bool SetReachable(bool marked)    { _field[CDR]._extra = marked? 1 : 0; }
};


static_assert(TYPE_COUNT < (1 << TYPE_BITS));
static_assert(sizeof(ValueVariant)  == 8);
static_assert(sizeof(Field)         == 8);
static_assert(sizeof(ConsCell)      == 8);



template<typename T> using Ref = std::shared_ptr<T>;



typedef int CellIndex;
typedef vector<Value> CellVector;

typedef std::shared_ptr<ValueVector>    ValueVectorRef;
typedef std::shared_ptr<Environment>    EnvironmentRef;
typedef std::shared_ptr<SymbolInfo>     SymbolRef;
typedef std::shared_ptr<SchemeNumber>   NumberRef;


class Storage
{
    PagedTable<ConsCell>        _cells;
    PagedTable<SmallValue>      _boxes;
    PagedTable<EnvironmentRef>  _environs;
    PagedTable<SymbolRef>       _symbols;
    PagedTable<StringRef>       _strings;
    PagedTable<VectorRef>       _vectors;
    PagedTable<NumberRef>       _numbers;

    inline Field* LocateCell(int addr)
    {
        ConsCell* cell;

        if (addr < 0)
            cell = (ConsCell*) &_fats[-addr];
        else
            cell = &cells[addr];

        return cell;
    }

    inline ValueVariant* LocateValueStorage(int addr, int field)
    {
        ConsCell* cell = LocateCell(addr);
        Field* field = &cell->_field[field];

        if (field->_embedded)
            return nullptr;

        if (field->_adjacent)
        {
            assert(field == CAR);
            return (ValueVariant*) (cell + 1);
        }

        int valueIndex = field->_data;
        ValueVariant* value = _values.Get(valueIndex);
        assert(value);

        return value;
    }

    template<typename CELLTYPE> struct ExternalReferenceType {};
    template<> ExternalReferenceType<TYPE_ENVIRONMENT> { typedef Environment Type; };
    template<> ExternalReferenceType<TYPE_SYMBOL> { typedef Environment Type; };
    template<> ExternalReferenceType<TYPE_STRING> { typedef Environment Type; };
    template<> ExternalReferenceType<TYPE_INTEGER> { typedef Environment Type; };
    template<> ExternalReferenceType<TYPE_RATIONAL> { typedef Environment Type; };
    template<> ExternalReferenceType<TYPE_VECTOR> { typedef Environment Type; };



    template<typename T> ReleaseExternalReference(ValueVariant* value);
    template<> ReleaseExternalReference<CELL_ENVIRONMENT>()

    void ReleaseValueStorage(Field* field)
    {
        if (field-)
        if (!field->_embedded)
        {
            int valueIndex = field->_data;
            value = _values.Get(valueIndex);
            assert(value);

            value->Reset(field->_type);
            _values.Free(value);
        }

        field->_type = TYPE_VOID;
        field->_embedded = 1;
        field->_exact = 0;
        field->_immutable = 0;
    }

    template<typename T, int CELLTYPE>
    inline void SetExternalReference(Field* field, const Ref<T>& ref)
    {
        assert(!field->_immutable); // FIXME: should be runtime error

        int valueIndex = _values.Alloc();
        ValueVariant* value = &_values[valueIndex];
        value->_symbol.reset(ref);

        ResetField(field);

        field->_type = CELLTYPE;
        field->_embedded = 1;
        field->_immutable = 1;
        field->_data = valueIndex;
    }

public:

    // TYPE_POINTER

    inline CELLADDR GetPointer(Field* field)
    {
        assert(field->_type == TYPE_POINTER);
        return (CELLADDR) field->_data;
    }

    inline void SetPointer(Field* field, CELLADDR value)
    {
        assert(!field->_immutable); // FIXME: should be runtime error

        ResetField(field);
        field->_type  = TYPE_POINTER;
        field->_data  = value;
        field->_embedded = true;
    }

    // TYPE_PROCEDURE

    inline CELLADDR GetProcedure(Field* field)
    {
        assert(field->_type == TYPE_PROCEDURE);
        return (CELLADDR) field->_data;
    }

    inline void SetProcedure(Field* field, CELLADDR value)
    {
        assert(!field->_immutable); // FIXME: should be runtime error

        ResetField(field);
        field->_type  = TYPE_PROCEDURE;
        field->_data  = value;
        field->_embedded = true;
    }

    // TYPE_ENVIRONMENT

    inline Ref<Environment> GetEnvironment(Field* field)
    {
        Ref<Environment> env = GetExternalReference<TYPE_ENVIRONMENT, Environment>(field);
        return env;
    }

    inline void SetEnvironment(Field* field, const Ref<Environment>& ref)
    {
        assert(!field->_immutable); // FIXME: should be runtime error

        SetExternalReference<TYPE_ENVIRONMENT>(field, ref);
    }

    // TYPE_SYMBOL

    inline Ref<SymbolInfo> GetSymbol(CELLADDR addr, int field)
    {
        Ref<SymbolInfo> env = GetExternalReference<TYPE_SYMBOL, SymbolInfo>(addr, field);
        return env;
    }







    SymbolInfo&     GetSymbol(      CELLADDR addr, int field);
    string          GetString(      CELLADDR addr, int field);
    char32_t        GetChar(        CELLADDR addr, int field);
    int64_t         GetInteger(     CELLADDR addr, int field);
    RationalNumber  GetRational(    CELLADDR addr, int field);
    double          GetReal(        CELLADDR addr, int field);
    ComplexNumber   GetComplex(     CELLADDR addr, int field);

    CELLADDR        GetPointer(     CELLADDR addr, int field);
    CELLADDR        GetProcedure(   CELLADDR addr, int field);
    Environment&    GetEnvironment( CELLADDR addr, int field);
    SymbolInfo&     GetSymbol(      CELLADDR addr, int field);
    string          GetString(      CELLADDR addr, int field);
    char32_t        GetChar(        CELLADDR addr, int field);
    int64_t         GetInteger(     CELLADDR addr, int field);
    RationalNumber  GetRational(    CELLADDR addr, int field);
    double          GetReal(        CELLADDR addr, int field);
    ComplexNumber   GetComplex(     CELLADDR addr, int field);

    CELLADDR        CreateVector(int length);
    CELLADDR        GetVectorElem(CELLADDR addr, int field, int index);
    void            SetVectorElem(CELLADDR addr, int field, int index, CELLADDR value);


};





