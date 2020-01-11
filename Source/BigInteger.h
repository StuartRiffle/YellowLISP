#pragma once






template<typename T = uint32_t, typename TT = int64_t>
class BigIntT
{
    static_assert(std::is_unsigned<T>::value);
    static_assert(std::is_signed<TT>::value);
    static_assert(sizeof(TT) >= sizeof(T) * 2);

    std::vector<T> _limb;

public:

    BigIntT(T val = 0) { _limb.push_back(val); }
    BigIntT(const T* begin, const T* end) : _limb(begin, end) {}
    BigIntT(const BigIntT& rhs) : _limb(rhs._limb) {}

    BigIntT operator+(const BigIntT& rhs) const
    {
        const BigIntT& lhs = *this;

        BigIntT result;
        result._limb.resize(std::max(lhs._limb.size(), rhs._limb.size()) + 1);

        TT carry = 0;
        for (size_t i = 0; i < result._limb.size(); i++)
        {
            TT ulimb = (i < lhs._limb.size())? lhs._limb[i] : 0;
            TT vlimb = (i < rhs._limb.size())? rhs._limb[i] : 0;
            TT sum = ulimb + vlimb + carry; 

            carry = sum >> (sizeof(T) * 8);
            result._limb[i] = (T) sum;
        }

        result.Normalize();
        return result;
    }

    BigIntT operator~() const
    {
        BigIntT result = *this;
        for()
        return ~(*this) + 1;
    }

    BigIntT operator-() const
    {
        return ~(*this) + 1;
    }

    BigIntT operator-(const BigIntT& rhs) const
    {
        return *this + -rhs;
    }

    BigIntT operator*(T v) const
    {
        BigIntT result;
        result._limb.resize(_limb.size() + 1);

        TT carry = 0;
        for (size_t i = 0; i < result._limb.size(); i++)
        {
            TT ulimb = (i < _limb.size())? _limb[i] : 0;
            TT prod  = (TT) ulimb * v + carry;

            carry = prod >> (sizeof(T) * 8);
            result._limb[i] = (T) sum;
        }

        result.Normalize();
        return result;
    }

    BigIntT operator*(const BigIntT& rhs) const
    {
        const BigIntT& lhs = *this;

        if (lhs.size() == 1)
            return rhs * lhs._limb[0];

        if (rhs.size() == 1)
            return lhs * rhs._limb[0];

        size_t split = std::min(lhs.size(), rhs.size()) / 2;

        BigIntT ulow( lhs._limb.begin(), lhs._limb.begin() + split);
        BigIntT uhigh(lhs._limb.begin() + split, lhs._limb.end());

        BigIntT vlow( rhs._limb.begin(), rhs._limb.begin() + split);
        BigIntT vhigh(rhs._limb.begin() + split, rhs._limb.end());

        BigIntT z0 = ulow * vlow;
        BigIntT z1 = (ulow + uhigh) * (vlow + vhigh);
        BigIntT z2 = uhigh * vhigh;

        BigIntT result = 
            (z2 << (split * 2)) + 
            (z1 - z2 - z0) << split +
            z0;

        result.Normalize();
        return result;
    }
};





