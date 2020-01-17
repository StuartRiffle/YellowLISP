#pragma once


enum TowerLevel
{
    LEVEL_INVALID = 0,

    LEVEL_INTEGER,
    LEVEL_RATIONAL,
    LEVEL_REAL,
    LEVEL_COMPLEX



    // exact int
    // 
};



// Only rational/rational internal aritm
// One level up (scalar) does coercion into floats
//


struct Scalar
{
    bool        _inexact;
    bool        _isRational;  
    Rational    _rational;
    double      _real;

    template<typename FUNC>
    Scalar Apply(const Scalar& rhs) const
    {
        Scalar result;

        if (_isRational)
        {
            if (rhs._isRational)
            {
                result._isRational = true;
                result._rational = _rational.Apply<FUNC>(rhs._rational);
            }
            else
            {
                result._isRational = false;
                result._real = FUNC(_real, rhs._real);
            }
        }
        else
        {
            if (rhs._isRational)
            {
                result._isRational = false;
                result._real = FUNC(_real, (double)rhs);
            }
            else
            {
                result._isRational = false;
                result._real = FUNC(_real, rhs._real);
            }
        }

        return result;
    }
};


class Number
{
    TowerLevel  _level   = LEVEL_INVALID;
    bool        _exact   = true;
    Scalar      _real;
    Scalar      _imag;

public:


    /*
    (if (< (numeric-tower-level b) (numeric-tower-level a))
        (func b a)
        (


    rational        real        complex
    rational    rational        real            
    real
    complex


    First:  bring to same tower level
    Second: 

    */


    template<typename FUNC>
    inline Number DoBinaryOp(const Number& lhs, const Number& rhs) const
    {
        const Number* u = &lhs;
        const Number* v = &rhs;
        Number temp;

        TowerLevel level = MakeSameLevel(const Number*& u, const Number*& v, Number* temp);

        Number result;
        FUNC(result, u, v, level);
        Normalize(result);

        result._exact = lhs._exact && rhs._exact;
        return result;
    }

    static void Add(Number& dest, Number* u, Number* v, TowerLevel level)
    {
        switch(level)
        {
            case LEVEL_INTEGER:
                dest._num = u._num + v._num;
                break;

            case LEVEL_RATIONAL:
                dest._num = u._num + v._num;
                dest._den = u._den + v._den;
                break;

            case LEVEL_REAL:
                dest._val = u._val+ v._val;
                break;

            case LEVEL_COMPLEX:
        }
    }




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

        /*
        One level up:

        bool    IsNumber
        bool    IsComplex
        bool    IsReal
        bool    IsRational
        bool    IsInteger
        bool    IsExact

        bool    IsRealValued
        bool    IsRationalValued
        */

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

