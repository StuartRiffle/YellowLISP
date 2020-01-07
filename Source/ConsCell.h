#pragma once

//
//    CELL TABLE           VALUE TABLE          LARGE OBJECTS
//    24-bit index x2      8 bytes of data      Reference counted
//                       
//    :     :     :        :           :       
//    |=====|=====|        |===========|        
//    | CAR | CDR |------->|  variant  |---+--> bignum
//    |=====|=====|        |===========|   |
//    :     :     :        :     ^     :   +--> string
//                               |         |
//                               |         +--> vector  [0][1][2] ...
//                               |         :             |  |  |
//                               |                       |  |  |
//                               +-----------------------+--+--+- ...                            
//  
//
//  A cons cell is 64 bits, and after type/tags we have 24 bits left over
//  (for each of CAR and CDR) to store their data.
//
//  Chars and small integers are stored directly in the cons cell, but
//  other types are given 8 bytes of storage in a value table,
//  and the cons cell only contains an index into that table.
//
//  Large objects are allocated on the heap, and the value table is
//  used to hold a reference counted pointer. The objects are eventually
//  deleted by the garbage collector when they become unreachable.
//  Big integer objects are allowed to grow without bounds. Caveat auctor!
// 
//  A vector can hold any mix of types, and uses the value table for storage.
//  Vector elements are 32 bit value table indices.
//
//  There is a special case though (of course), which is "fat" cells:
//
//    :     :     :           :       
//    |=====+=====+===========|        
//    | CAR | CDR | CAR value | 
//    |=====+=====+===========|
//    :     :     :           :
//
//  A "fat" cell is immediately followed by the value of CAR. This is an
//  optimization for simple list nodes, where CDR is just an embedded
//  pointer, to avoid the [potential] cache miss of doing a value table
//  for CAR. Fat cells are stored in their own table.
//  
//  If a fat cell is ever mutated into something that is NOT a garden variety 
//  list node, it falls back to being normal cell, and just ignores that 
//  extra word of storage. The code calls that a "degenerate" fat cell, and 
//  I'd expect them to be rare.
//  
//  Fat cells are created opportunistically, at cons time, as a happy surprise. 
//  A normal cell is never be promoted to a fat cell, and a degenerate fat
//  cell is never demoted to normal storage.
//  
//      TODO: gather metrics about this to validate assumptions
//
//  The cell table, fat cell table, and value table /themselves/ are allocated
//  in pages, and expand by just adding more pages. The pages are never released,
//  so total memory usage will reach some high-water mark at runtime and stay there.
//

enum
{
    CAR = 0,
    CDR = 1
};

typedef uint32_t FieldWord;
typedef uint64_t CellWord;

enum FieldType : FieldWord
{
    TYPE_INVALID = 0,
    TYPE_NULL, // Only the empty list is TYPE_NULL

    TYPE_POINTER,
    TYPE_PROCEDURE,
    TYPE_ENVIRONMENT,
    TYPE_SYMBOL,
    TYPE_STRING,
    TYPE_CHAR,
    TYPE_INTEGER,
    TYPE_RATIONAL,
    TYPE_REAL,
    TYPE_COMPLEX,
    TYPE_VECTOR,

    TYPE_COUNT
};

template<typename T> using Ref = std::shared_ptr<T>;

class BigInteger;
class RationalNumber;


typedef std::complex<float>             ComplexNumber;
typedef void*                           BigInteger;  // Placeholder
typedef void*                           ValueVector; // Placeholder

const int TYPE_BITS = 4;
const int DATA_BITS = 24;

struct Field
{
    FieldType _type : TYPE_BITS;
    FieldWord _data : DATA_BITS;

    // The value stored in _data is an index into the value table, unless
    // the _embedded flag is set. This happens for a couple of cases:
    // 
    //   - TYPE_POINTER and TYPE_PROCEDURE hold a cell table address
    //   - TYPE_CHAR and TYPE_INTEGER store the value directly, if it
    //     is small enough to fit.

    FieldWord _embedded : 1;    // Value is 24 bits or less, and embedded in _data
    FieldWord _exact    : 1;    // Value is numeric and "exact"
    FieldWord _reserved : 2;    // These bits are reserved for ConsCell to use as flags
};

template<typename T> bool CanBeEmbedded


// Values 

union ValueUnion
{
    uintptr_t           _raw;
    double              _real;      
    char32_t            _char;      
    ComplexNumber       _complex;
    uintptr_t           _external;

    template<typename T>
    inline Ref<T>& AsExternalReference()
    {
        return *((Ref<T>*) _external);
    }
};

struct ValueVariant
{
    ValueUnion  _value;
};


struct ConsCell
{
    Field _field[2]; // CAR and CDR

    // 

    inline bool IsAllocated() const          { return _field[CAR]._extra; }
    inline bool SetAllocated(bool allocated) { _field[CAR]._extra = allocated? 1 : 0; }

    inline bool IsReachable() const          { return _field[CDR]._extra; }
    inline bool SetReachable(bool marked)    { _field[CDR]._extra = marked? 1 : 0; }
};


struct FatCell : public ConsCell
{
    ValueVariant _car;
};

static_assert(TYPE_COUNT < (1 << TYPE_BITS));
static_assert(sizeof(ValueVariant)  == 8);
static_assert(sizeof(Field)         == 8);
static_assert(sizeof(ConsCell)      == 8);



class Storage
{
    PagedTable<ConsCell>     _cells;
    PagedTable<FatCell>      _fats;
    PagedTable<ValueVariant> _values;

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





