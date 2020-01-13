#pragma once

template<typename T>
struct Complex
{
    T r;
    T i;
};

template<typename T>
inline Complex<T> Conjugate(const Complex<T>& u)
{
    return Complex<T>(u.r, -u.i);
}

template<typename T>
inline T Phase(const Complex<T>& u)
{
    return atan2(u.i, u.r);
}

template<typename T>
inline T Magnitude(const Complex<T>& u)
{
    return sqrt(u.r * u.r + u.i * u.i);
}

template<typename T>
inline Complex<T> operator+(const Complex<T>& u, const Complex<T>& v)
{
    return Complex<T>(u.r + v.r, u.i + v.i);
}

template<typename T>
inline Complex<T> operator-(const Complex<T>& u, const Complex<T>& v)
{
    return Complex<T>(u.r - v.r, u.i - v.i);
}

template<typename T>
inline Complex<T> operator*(const Complex<T>& u, const Complex<T>& v)
{
    const T& a = u.r;
    const T& b = u.i;
    const T& c = v.r;
    const T& d = v.i;

    T ac = a * c;
    T bd = b * d;

    return Complex<T>(ac - bd, (a + b) * (c + d) - ac - bd);
}

template<typename T>
inline Complex<T> operator/(const Complex<T>& u, const Complex<T>& v)
{
    const T& a = u.r;
    const T& b = u.i;
    const T& c = v.r;
    const T& d = v.i;

    T div   = (c * c) + (d * d);
    T scale = (T) Rational(1, div);

    return Complex<T>(
        ((a * c) + (b * d)) * invdiv,
        ((b * c) - (a * d)) * invdiv);
}

template<typename T>
inline Complex<T> operator*(const Complex<T>& u, const T& sv)
{
    return Complex<T>(u.r * sv, u.i * sv);
}

template<typename T>
inline Complex<T> operator/(const Complex<T>& u, const T& sv)
{
    T scale = (T) Rational(1, sv);
    return u * scale;
}

template<typename T>
inline Complex<T> pow(const Complex<T>& u, const Complex<T>& v)
{
    const T& a = u.r;
    const T& b = u.i;
    const T& c = v.r;
    const T& d = v.i;

    T aa_bb  = (a * a) + (b * b);
    T uphase = ComplexPhase(u);
    T half   = (T) Rational(1, 2);
    T theta  = (c * uphase) + (d * half * log(aa_bb));

    return Complex<T>(cos(theta), sin(theta)) * (pow(aa_bb, c * half) * exp(-d * uphase));
}

template<typename T>
inline Complex<T> pow(const T& u, const Complex<T>& v)
{
    const T& a = u;
    const T& b = v.r;
    const T& c = v.i;

    T theta = c * log(a);
    return Complex<T>(cos(theta), sin(theta)) * pow(a, b);
}

template<typename T>
inline Complex<T> log(const Complex<T>& u)
{
    return Complex<T>(log(Magnitude(u)), Phase(u));
}

template<typename T>
inline Complex<T> exp(const Complex<T>& u)
{
    return Complex<T>(cos(u.i), sin(u.i)) * exp(u.r);
}


