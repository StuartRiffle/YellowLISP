#pragma once


template<typename T>
T AbsoluteValue(const T& a)
{
    return (a < 0)? -a : a;
}

template<typename T>
T GreatestCommonDivisor(const T& a, const T& b)
{
    if (a == 0)
        return a;

    return GreatestCommonDivisor(b, a % b);
}

template<typename T>
T LeastCommonMultiple(const T& a, const T& b)
{
    return AbsoluteValue(a * b) / GreatestCommonDivisor(a, b);
}

template<typename T>
class Rational
{
    T _num;
    T _den;

public:
    Rational(const T& num = 0, const T& den = 1)
    {
        *this = Reduced(num, den);
    }

    Rational operator+(const Rational& rhs) const 
    {
        return Rational((_num * rhs._den) + (_den * rhs._num), (_den * rhs._den));
    }

    Rational operator-(const Rational& rhs) const 
    {
        return Rational((_num * rhs._den) - (_den * rhs._num), (_den * rhs._den));
    }

    Rational operator*(const Rational& rhs) const 
    {
        return Rational((_num * rhs._num), (_den * rhs._den));
    }

    Rational operator/(const Rational& rhs) const 
    {
        return Rational((_num * rhs._den), (_den * rhs._num));
    }

    Rational operator%(const Rational& rhs) const 
    {
        return Rational(*this - (*this / rhs));
    }

    bool operator<(const Rational& rhs) const
    {
        return (_num * rhs._den) < (_den * rhs._num);
    }

    bool operator==(const Rational& rhs) const
    {
        return (_num == rhs._num) && (_den == rhs._den);
    }

    template<typename U>
    explicit operator(U)() const
    {
        return (U) _num / (U) _den;
    }

    static Rational Reduced(T num, T den)
    {
        if (den < 0)
        {
            num = -num;
            den = -den;
        }

        if (den != 1)
        {
            T gcd = GreatestCommonDivisor(num, den);

            if (gcd != den)
            {
                num = num / gcd;
                den = den / gcd;
            }
        }

        Rational r = { num, den };
        return r;
    }
};


