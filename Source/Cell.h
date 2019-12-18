// YellowLISP (c) 2019 Stuart Riffle

#pragma once
#include "Yellow.h"

class Cell
{
	TWORD _tags : TAG_BITS;
	TWORD _type : TYPE_BITS;
	TWORD _next : INDEX_BITS;
	TWORD _data : DATA_BITS;

public:
	inline bool IsAtom() const		{ return (_type == TYPE_ATOM); }
	inline bool IsEmptyList() const { return (_type == TYPE_VOID); }
	inline bool IsValue() const		{ return (_type == TYPE_VOID); }


	inline bool HasTag(Tag tag) const { return(_tags & (TWORD(1) << tag)); }
	inline void SetTag(Tag tag) { _tags |= (TWORD(1) << tag); }
	inline void ClearTag(Tag tag) { _tags &= ~(TWORD(1) << tag); }
	inline void ResetTags() { _tags = 0; }

	inline TINDEX GetNext() const { return _next; }
	inline void SetNext(TINDEX addr) { _next = addr; }

	inline TINDEX GetCellRef() const
	{
		assert(_type == TYPE_CELL);
		return (TINDEX)_data;
	}

	inline void SetCellRef(TINDEX addr)
	{
		_data = addr;
		_type = TYPE_CELL;
	}

	inline int GetIntLiteral() const
	{
		assert(_type == TYPE_INT);
		int value = (int)_data;

		int valueBits = sizeof(value) * 8;
		int signExtend = valueBits - DATA_BITS;
		if (signExtend > 0)
			value = (value << signExtend) >> signExtend;

		return value;
	}

	inline void SetIntLiteral(int value)
	{
		int valueBits = sizeof(value) * 8;
		int signExtend = DATA_BITS - valueBits;
		if (signExtend > 0)
			value = (value << signExtend) >> signExtend;

		_data = value;
		_type = TYPE_INT;
	}

	union FloatUnion
	{
		TDATA raw;
		float value;
	};

	inline float GetFloatLiteral() const
	{
		assert(_type == TYPE_FLOAT);

		FloatUnion pun;
		pun.raw = _data;

		int floatBits = sizeof(float) * 8;
		int zeroPad = (floatBits - DATA_BITS);
		if (zeroPad > 0)
			pun.raw <<= zeroPad;

		return(pun.value);
	}

	inline void SetFloatLiteral(float value)
	{
		FloatUnion pun;
		pun.value = value;

		int floatBits = sizeof(float) * 8;
		int truncateBits = (floatBits - DATA_BITS);
		if (truncateBits > 0)
			pun.raw >>= truncateBits;

		_data = pun.raw;
		_type = TYPE_FLOAT;
	}

	inline int GetStringIndex() const
	{
		assert(_type == TYPE_STRING);
		return (int)_data;
	}

	inline int GetFunctionIndex() const
	{
		assert(_type == TYPE_FUNC);
		return (int)_data;
	}
};
