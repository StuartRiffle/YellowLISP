// YellowLISP (c) 2019 Stuart Riffle (MIT license)

#include "Yellow.h"
#include "Runtime.h"
#include "Errors.h"

#define IMPLEMENT_BINARY_OP(_FUNCNAME, _EXPR) \
CELL_INDEX Runtime::_FUNCNAME(const ArgumentList& args) \
{ \
    VERIFY_NUM_PARAMETERS(args.size(), 2, #_FUNCNAME); \
    double a = LoadNumericLiteral(args[0]); \
    double b = LoadNumericLiteral(args[1]); \
    double value = (_EXPR); \
    return CreateNumericLiteral(value); \
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
        return CreateNumericLiteral((int)a % (int)b);

    return CreateNumericLiteral(fmod(a, b));
}
