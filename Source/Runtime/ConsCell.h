#pragma once

enum
{
    CAR = 0,
    CDR = 1
};


struct Datum
{
    uint32_t    _type : 4;
    uint32_t    _data : 28;
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





