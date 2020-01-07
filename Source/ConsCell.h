#pragma once

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

template<typename T> using ObjectReference = std::shared_ptr<T>;

typedef std::pair<int32_t, uint32_t>    RationalNumber;
typedef std::complex<float>             ComplexNumber;
typedef void*                           BigInteger;  // Placeholder
typedef void*                           ValueVector; // Placeholder

const int TYPE_BITS = 4;
const int DATA_BITS = 24;

struct Field
{
    FieldType _type : TYPE_BITS;
    FieldWord _data : DATA_BITS;

    // The value stored in _data depends on the type:
    // 
    //   TYPE_POINTER:      Cell table index 
    //   TYPE_PROCEDURE:    Cell table index
    //   TYPE_SYMBOL:       Symbol table index
    //   TYPE_STRING:       Value table index
    //   TYPE_CHAR:         Literal value (if _local), otherwise value table index
    //   TYPE_INTEGER:      Literal value (if _local), otherwise value table index
    //   TYPE_RATIONAL:     Value table index
    //   TYPE_REAL:         Value table index
    //   TYPE_COMPLEX:      Value table index
    //   TYPE_VECTOR:       Value table index

    FieldWord _local : 1;       // Value is embedded in _data
    FieldWord _exact : 1;       // Value is numeric and "exact"
    FieldWord _immutable : 1;   // Value is a constant and cannot be changed
    FieldWord _extra : 1;       // Reserved for use by ConsCell
};

union ValueVariant
{
    int64_t         _integer;   // Value of TYPE_INTEGER (if it's not "exact")
    double          _real;      // Value of TYPE_REAL
    char32_t        _char;      // Value of TYPE_CHAR, UTF-32
    RationalNumber  _rational;  // Value of TYPE_RATIONAL
    ComplexNumber   _complex;   // Value of TYPE_COMPLEX

                                // The value stored in _external is a reference counted pointer to:
                                // 
                                //   TYPE_INTEGER:      BigNumber (if it's "exact")
                                //   TYPE_STRING:       UTF-8 encoded std::string
                                //   TYPE_VECTOR:       ValueVector

    ObjectReference _external;  // External resource 
    ValueVariant*   _nextFree;  // When not in use, the next free slot in the chain
};





struct alignas(uint64_t) ConsCell
{
    Field _field[2];

    inline bool IsAllocated() const          { return _field[CAR]._extra; }
    inline bool SetAllocated(bool allocated) { _field[CAR]._extra = allocated? 1 : 0; }

    inline bool IsReachable() const          { return _field[CDR]._extra; }
    inline bool SetReachable(bool marked)    { _field[CDR]._extra = marked? 1 : 0; }
};

static_assert(TYPE_COUNT < (1 << TYPE_BITS));
static_assert(sizeof(ValueVariant)  == 8);
static_assert(sizeof(Field)         == 8);
static_assert(sizeof(ConsCell)      == 8);

template<int PAGE_BITS = 16>
class CellTable
{
    enum
    {
        PAGE_ELEMS = 1 << PAGE_BITS,
        PAGE_MASK  = PAGE_ELEMS - 1,
    };

    typedef std::unique_ptr<ConsCell> CellPage;
    vector<CellPage> _pages;

    inline ConsCell& ResolveCell(int addr)
    {
        int page = addr & PAGE_MASK;
        assert(page < _pages.size());

        int offset = addr >> PAGE_BITS;
        assert(offset >= 0);
        assert(offset < PAGE_ELEMS);

        return _pages[page][offset];
    }
};







