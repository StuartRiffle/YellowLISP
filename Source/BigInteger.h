#pragma once




#ifdef _MSC_VER
#include <malloc.h>
#define STACK_ALLOC _alloca
#else
#include <alloca.h>
#define STACK_ALLOC alloca
#endif


template<typename T, typename TT>
class BigInt
{
    static_assert(std::is_unsigned<T>::value);
    static_assert(std::is_signed<TT>::value);
    static_assert(sizeof(TT) >= sizeof(T) * 2);

    std::vector<T> _limb;

    template<SUBTRACT = false>
    static BigInt AddSubImpl(const BigInt& u, const BigInt& v)
    {
        BigInt result;
        result._limb.resize(std::max(u.size(), v.size()) + 1);

        TT carry = SUBTRACT? 1 : 0;
        for (int i = 0; i < result._limb.size(); i++)
        {
            TT ulimb = (i < u.size())? u._limb[i] : 0;
            TT vlimb = (i < v.size())? v._limb[i] : 0;
            TT sum = carry + ulimb + (SUBTRACT? ~vlimb : vlimb); 

            carry = sum >> (sizeof(T) * 8);
            result._limb[i] = (T) sum;
        }

        result.Normalize();
        return result;
    }

    static BigInt MultiplyOneLimb(const BigInt& u, T v)
    {
        BigInt result;
        result._limb.resize(u.size() + 1);

        TT carry = 0;
        for (int i = 0; i < result._limb.size(); i++)
        {
            TT ulimb = (i < u.size())? u._limb[i] : 0;
            TT prod  = (TT) ulimb * v + carry;

            carry = prod >> (sizeof(T) * 8);
            result._limb[i] = (T) sum;
        }

        result.Normalize();
        return result;
    }

    static BigInt Multiply(const BigInt& u, const BigInt& v)
    {
        if (u.size() == 1)
            return MultiplyOneLimb(v, u._limb[0]);

        if (v.size() == 1)
            return MultiplyOneLimb(u, v._limb[0]);

        int split = (ulen < vlen? ulen : vlen) / 2;

        BigInt ulow(u, u + split - 1);
        BigInt uhigh(u + split, u + ulen);

        BigInt ulow(v, v + split - 1);
        BigInt uhigh(v + split, v + ulen);

        BigInt z0 = ulow * vlow;
        BigInt z1 = (ulow + uhigh) * (vlow + vhigh);
        BigInt z2 = uhigh * vhigh;

        BigInt result = (z2 << (split * 2)) + (z1 - z2 - z0) << split + z0;
        return result;
    }


public:

    BigInt operator+(const BigInt& rhs) const { return AddSubImpl<false>(*this, rhs); }
};



struct TempStackNum
{

    TempStackNum(int limbs)
};

template<typename T, typename TT>
void Multiply(T* dest, int destlen, const T* u, int ulen, const T* v, int vlen)
{
    if (ulen == 1)
        return (void) MultiplyOneLimb(dest, destlen, v, vlen, *u);

    if (vlen == 1)
        return (void) MultiplyOneLimb(dest, destlen, u, ulen, *v);

    int split = (ulen < vlen? ulen : vlen) / 2;

    // ulow  = u[0..split-1]
    // uhigh = u[split..ulen]
    // vlow  = v[0..split-1]
    // vhigh = v[split..vlen]



    BigInt ulow(u, u + split - 1);
    BigInt uhigh(u + split, u + ulen);

    BigInt ulow(v, v + split - 1);
    BigInt uhigh(v + split, v + ulen);

    BigInt z0 = ulow * vlow;
    BigInt z1 = (ulow + uhigh) * (vlow + vhigh);
    BigInt z2 = uhigh * vhigh;

    BigInt result = (z2 << (split * 2)) + (z1 - z2 - z0) << split + z0;


#define STACK_BUFFER(_NAME, _ELEMS)\
    int _NAME##len = (_ELEMS); \
    T* _NAME = STACK_ALLOC(_NAME##len * sizeof(T));

    // z0 = ulow * vlow
    STACK_BUFFER(z0, split * 2);
    Multiply(z0, z0len, v, split, u split);

    // z1u = ulow + uhigh
    STACK_BUFFER(z1u, min(split, ulen - split) + 1);
    Add(z1u, z1ulen, u, split, u + split, ulen - split);

    // z1v = vlow + vhigh
    STACK_BUFFER(z1v, min(split, vlen - split) + 1);
    Add(z1v, z1vlen, v, split, v + split, vlen - split);

    // z1 = z1u * z1v
    STACK_BUFFER(z1, z1ulen + z1vlen);
    Multiply(z1, z1len, z1u, z1ulen, z1v, z1vlen);

    // z2 = uhigh * vhigh
    STACK_BUFFER(z2, (ulen - split) + (vlen - split));
    Multiply(z2, z2len, u + split, ulen - split, v + split, vlen - split);

    // r1 = z2 << (split * 2)
    STACK_BUFFER(r1, z2len + split * 2);
    for (int i = 0; i < r1len; i++)
        r1[i] = (i < split * 2)? 0 : z2[i - split * 2];

    // r2a = z1 - z2
    STACK_BUFFER(r2a, max(z0len, max(z1len, z2len)) + 1);
    Subtract(r2a, r2alen, z1,  z1len, z2, z2len);

    // r2b = r2a - z0
    STACK_BUFFER(r2b, r2alen);
    Subtract(r2b, r2blen, r2a, r2alen, z0, z0len);

    // r2 = r2b << split
    STACK_BUFFER(r2, r2blen + split);
    for (int i = 0; i < r2len; i++)
        r2[i] = (i < split)? 0 : r2[i - split];

    // d1 = r1 + r2
    STACK_BUFFER(d1, max(r1len, r2len) + 1);
    Add(d1, d1len, r1, r1len, r2, r2len);

    // dest = d1 + z0
    Add(dest, destlen, d1, d1len, z0, z0len);



    int r2xlen = max(z0len, max(z1len, z2len));
    T* r2a = STACK_ALLOC(r2xlen * sizeof(T));
    T* r2b = STACK_ALLOC(r2xlen * sizeof(T));
    Subtract(r2a, r2xlen, z1,  z1len, z2, z2len);
    Subtract(r2b, r2xlen, r2a, r2len, z0, z0len);

    int r2len = r2xlen + split;
    T* r2 = STACK_ALLOC(r2len * sizeof(T));
    for (int i = 0; i < r2len; i++)
        r2[i] = (i < split)? 0 : r2[i - split];

    // dest = r1 + r2 + z0









    // z0 = ulow * vlow

    int z0len = split * 2;
    T* z0 = STACK_ALLOC(z0len * sizeof(T));
    Multiply(z0, z0len, v, split, u split);

    // z1u = ulow + uhigh

    int z1ulen = min(split, ulen - split) + 1;
    T* z1u = STACK_ALLOC(z1ulen * sizeof(T));
    Add(z1u, z1ulen, u, split, u + split, ulen - split);

    // z1v = vlow + vhigh

    int z1vlen = min(split, vlen - split) + 1;
    T* z1v = STACK_ALLOC(z1vlen * sizeof(T));
    Add(z1v, z1vlen, v, split, v + split, vlen - split);

    // z1 = z1u * z1v

    int z1len = z1ulen + z1vlen;
    T* z1 = STACK_ALLOC(z1len  * sizeof(T));
    Multiply(z1, z1len, z1u, z1ulen, z1v, z1vlen);

    // z2 = uhigh * vhigh

    int z2len = (ulen - split) + (vlen - split);
    T* z2 = STACK_ALLOC(z2len  * sizeof(T));
    Multiply(z2, z2len, u + split, ulen - split, v + split, vlen - split);

    // r1 = z2 << (split * 2)

    int r1len = z2len + split * 2;
    T* r1 = STACK_ALLOC(r1len * sizeof(T));
    for (int i = 0; i < r1len; i++)
        r1[i] = (i < split * 2)? 0 : z2[i - split * 2];

    // r2 = (z1 - z2 - z0) << split

    int r2xlen = max(z0len, max(z1len, z2len));
    T* r2a = STACK_ALLOC(r2xlen * sizeof(T));
    T* r2b = STACK_ALLOC(r2xlen * sizeof(T));
    Subtract(r2a, r2xlen, z1,  z1len, z2, z2len);
    Subtract(r2b, r2xlen, r2a, r2len, z0, z0len);

    int r2len = r2xlen + split;
    T* r2 = STACK_ALLOC(r2len * sizeof(T));
    for (int i = 0; i < r2len; i++)
        r2[i] = (i < split)? 0 : r2[i - split];

    // dest = r1 + r2 + z0

    assert(destlen >= r2len);

    int d1len = r1len;
    T* d1 = STACK_ALLOC(d1len * sizeof(T));




    for (int i = 0; i < 


    // z2 << (split * 2

    /*
    procedure karatsuba(num1, num2)
    if (num1 < 10) or (num2 < 10)
        return num1 × num2
    
    m = min(size_base10(num1), size_base10(num2))
    m2 = floor(m / 2) 
    
    high1, low1 = split_at(num1, m2)
    high2, low2 = split_at(num2, m2)
    
    z0 = karatsuba(low1, low2)
    z1 = karatsuba((low1 + high1), (low2 + high2))
    z2 = karatsuba(high1, high2)
    
    return (z2 × 10 ^ (m2 × 2)) + ((z1 - z2 - z0) × 10 ^ m2) + z0

    */
}




template<typename T, typename TT>
void SubMultiword(const T* u, int ulen, const T* v, int vlen, T* dest)
{
    if (ulen < vlen)
    {
        SubMultiword(v, vlen, u, ulen, dest, borrow);
        return;
    }

    TT borrow = 0;
    for (int i = 0; i < ulen; i++)
    {
        TT n = borrow + (i < vlen)? v[i] : 0;
        TT diff = (n > u[i])?

        carry = sum >> (sizeof(T) * 8);
        dest[i] = (T) sum;
    }


    dest[ulen] = carry;
}


class BigInteger
{
    typedef uint32_t ULIMB;
    typedef int32_t  SLIMB;
    typedef int64_t  

    vector<LIMB> _limbs;
    LIMB _sign;

    inline LIMB operator[](size_t i) const
    {
        return (i < _limbs.size())? _limbs[i] : _sign;
    }

    // unsigned char _subborrow_u64(unsigned char b_in,unsigned __int64 src1,unsigned __int64 src2,unsigned __int64 *diff)

    inline LIMB AddWithCarry(LIMB x, LIMB y, LIMB carry, LIMB* sum)
    {
#ifdef _MSC_VER
#elif 

    }

    signed LIMB

    void Multiply(LIMB a, LIMB b, LIMB& hi, LIMB& lo);

public:
    BigInteger(int64_t val = 0)
    {
        if (val < 0)
        {
            sign = -1;
            val *= -1;
        }

        _limbs.push_back((uint64_t) val);
    }

    //  3 +  7 = 3 + 7      == a + b
    // -3 +  7 = 7 - 3      == b - a
    //  3 + -7 = 3 - 7      == a - b == -(b - a)
    // -3 + -7 = -(3 + 7)   == -(a + b)

    //  3 -  7 = 3 - 7      == a - b
    // -3 -  7 = -(3 + 7)   == -(a + b)
    //  3 - -7 = 3 + 7      == a + b
    // -3 - -7 = 7 - 3      == b - a

    // need primitive +/-

    bool apos = a._sign > 0;
    bool bpos = b._sign > 0;

    BigInteger c;

    if (apos)
    {
        if (bpos)
            c = a + b;
        else
            c = a - b;
    }

    return
        (apos &&  bpos)? (a + b) :
        (apos && !bpos)? (a - b) : 
        (!apos && bpos)? (b - a) :



    void Add(const BigInteger& a, const BigInteger& b, BigInteger& dest )
    {
        size_t maxLength = std::max(a.size(), b.size()) + 1;
        c._limbs.resize(maxLength);

        uint64_t asign = (a._limb.back() >> 31)? ~0ULL : 0;
        uint64_t bsign = (b._limb.back() >> 31)? ~0ULL : 0;
        uint64_t carry = 0;

        for (size_t i = 0; i < maxLength; i++)
        {
            uint64_t alimb = (i < a.size())? a._limb[i] : asign;
            uint64_t blimb = (i < b.size())? b._limb[i] : bsign;
            uint64_t sum   = alimb + blimb + carry;

            c._limbs.push_back(sum);
            carry = sum >> 32ULL;
        }

        if ((c._limbs.back() == 0) || (c._limbs.back() == ~0ULL))

        assert(carry == 0);
        c.Normalize();
        return c;
    }

    BigInteger operator-( const BigInteger& v ) const 
    {
        BigInteger result;
        Add(*this, v, result);
        return result;
    }


    BigInteger   operator-  ( const BigInteger& v )  const { return( _mm_sub_epi64(   vec, v.vec ) ); }
    BigInteger   operator*  ( const BigInteger& v )  const { return( _mm_sub_epi64(   vec, v.vec ) ); }
    BigInteger   operator/  ( const BigInteger& v )  const { return( _mm_sub_epi64(   vec, v.vec ) ); }

    BigInteger   operator<< ( const BigInteger& v )  const { return( _mm_sllv_epi64x( vec, v.vec ) ); }
    BigInteger   operator>> ( const BigInteger& v )  const { return( _mm_sllv_epi64x( vec, v.vec ) ); }
    BigInteger   operator&  ( const BigInteger& v )  const { return( _mm_and_si128(   vec, v.vec ) ); }
    BigInteger   operator|  ( const BigInteger& v )  const { return( _mm_or_si128(    vec, v.vec ) ); }
    BigInteger   operator^  ( const BigInteger& v )  const { return( _mm_xor_si128(   vec, v.vec ) ); }
    BigInteger   operator~  ()                       const { return( _mm_xor_si128(   vec, _mm_set1_epi8(  ~0 ) ) ); }
};  
