// YellowLISP (c) 2020 Stuart Riffle (MIT license)

#include "Yellow.h"
#include "Runtime.h"
#include "Errors.h"

#define IMPLEMENT_BINARY_OP(_OPNAME, _EXPR) \
CELL_INDEX Runtime::_OPNAME(const ArgumentList& args) \
{ \
    VERIFY_NUM_PARAMETERS(args.size(), 2, #_OPNAME); \
    double a = LoadNumericLiteral(args[0]); \
    double b = LoadNumericLiteral(args[1]); \
    double value = (_EXPR); \
    bool isInt = ((_cell[args[0]]._type == TYPE_INT) && (_cell[args[1]]._type == TYPE_INT)); \
    return CreateNumericLiteral(value, isInt); \
}

IMPLEMENT_BINARY_OP(ADD, (a + b))
IMPLEMENT_BINARY_OP(SUB, (a - b))
IMPLEMENT_BINARY_OP(MUL, (a * b))
IMPLEMENT_BINARY_OP(DIV, (a / b))

CELL_INDEX Runtime::MOD(const ArgumentList& args)
{
    VERIFY_NUM_PARAMETERS(args.size(), 2, "MOD");
    double a = LoadNumericLiteral(args[0]);
    double b = LoadNumericLiteral(args[1]);

    if ((_cell[args[0]]._type == TYPE_INT) && (_cell[args[1]]._type == TYPE_INT))
        return CreateNumericLiteral((int)a % (int)b, true);

    return CreateNumericLiteral(fmod(a, b));
}

#define IMPLEMENT_MATH_FUNCTION(_FUNCNAME, _EXPR) \
CELL_INDEX Runtime::_FUNCNAME(const ArgumentList& args) \
{ \
    VERIFY_NUM_PARAMETERS(args.size(), 1, #_FUNCNAME); \
    double a = LoadNumericLiteral(args[1]); \
    double value = (_EXPR); \
    return CreateNumericLiteral(value); \
}


IMPLEMENT_MATH_FUNCTION(ROUND,     round(a));
IMPLEMENT_MATH_FUNCTION(TRUNCATE,  trunc(a));
IMPLEMENT_MATH_FUNCTION(FLOOR,     floor(a));
IMPLEMENT_MATH_FUNCTION(CEILING,   ceil(a));
IMPLEMENT_MATH_FUNCTION(EXP,       exp(a));
IMPLEMENT_MATH_FUNCTION(LOG,       log(a));
IMPLEMENT_MATH_FUNCTION(SQRT,      sqrt(a));
IMPLEMENT_MATH_FUNCTION(ABS,       fabs(a));

IMPLEMENT_MATH_FUNCTION(SIN,       sin(a));
IMPLEMENT_MATH_FUNCTION(SINH,      sinh(a));
IMPLEMENT_MATH_FUNCTION(ASIN,      asin(a));
IMPLEMENT_MATH_FUNCTION(ASINH,     asinh(a));
IMPLEMENT_MATH_FUNCTION(COS,       cos(a));
IMPLEMENT_MATH_FUNCTION(COSH,      cosh(a));
IMPLEMENT_MATH_FUNCTION(ACOS,      acos(a));
IMPLEMENT_MATH_FUNCTION(ACOSH,     acosh(a));
IMPLEMENT_MATH_FUNCTION(TAN,       tan(a));
IMPLEMENT_MATH_FUNCTION(TANH,      tanh(a));
IMPLEMENT_MATH_FUNCTION(ATAN,      atan(a));
IMPLEMENT_MATH_FUNCTION(ATANH,     atanh(a));

STUB_UNIMPLEMENTED(RANDOM);
STUB_UNIMPLEMENTED(EXPT);
STUB_UNIMPLEMENTED(MIN);
STUB_UNIMPLEMENTED(MAX);

