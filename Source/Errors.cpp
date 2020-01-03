// YellowLISP (c) 2020 Stuart Riffle (MIT license)

#include "Yellow.h"
#include "Errors.h"
#include "Coverage.h"

const char* YellowError::GetDesc(ErrorCode code)
{
    switch (code)
    {
        case ERROR_PARSER_SYNTAX:                   RETURN_ASSERT_COVERAGE("syntax error");
        case ERROR_PARSER_STRING_UNTERMINATED:      RETURN_ASSERT_COVERAGE("string unterminated");
        case ERROR_PARSER_LIST_EXPECTED:            RETURN_ASSERT_COVERAGE("list expected");
        case ERROR_PARSER_LIST_UNTERMINATED:        RETURN_ASSERT_COVERAGE("list unterminated");
        case ERROR_PARSER_INVALID_IDENTIFIER:       RETURN_ASSERT_COVERAGE("invalid identifier");
        case ERROR_PARSER_INVALID_MACRO_EXPANSION:  RETURN_ASSERT_COVERAGE("invalid macro expansion");

        case ERROR_RUNTIME_NOT_IMPLEMENTED:         RETURN_ASSERT_COVERAGE("not implemented");
        case ERROR_RUNTIME_WRONG_NUM_PARAMS:        RETURN_ASSERT_COVERAGE("wrong number of parameters");
        case ERROR_RUNTIME_VARIABLE_UNBOUND:        RETURN_ASSERT_COVERAGE("variable is unbound");
        case ERROR_RUNTIME_TYPE_MISMATCH:           RETURN_ASSERT_COVERAGE("type mismatch");
        case ERROR_RUNTIME_RESERVED_SYMBOL:         RETURN_ASSERT_COVERAGE("reserved symbol");
        case ERROR_RUNTIME_UNDEFINED_FUNCTION:      RETURN_ASSERT_COVERAGE("undefined function");
        case ERROR_RUNTIME_INVALID_ARGUMENT:        RETURN_ASSERT_COVERAGE("invalid argument");

        case ERROR_INTERNAL_OUT_OF_MEMORY:          RETURN_ASSERT_COVERAGE("[INTERNAL] out of memory");
        case ERROR_INTERNAL_PARSER_FAILURE:         RETURN_ASSERT_COVERAGE("[INTERNAL] parsing failure");
        case ERROR_INTERNAL_RUNTIME_FAILURE:        RETURN_ASSERT_COVERAGE("[INTERNAL] runtime failure");
        case ERROR_INTERNAL_HASH_COLLISION:         RETURN_ASSERT_COVERAGE("[INTERNAL] hash collision");
        case ERROR_INTERNAL_SLOT_POOL_RANGE:        RETURN_ASSERT_COVERAGE("[INTERNAL] slot pool access out of range");
        case ERROR_INTERNAL_AST_CORRUPT:            RETURN_ASSERT_COVERAGE("[INTERNAL] AST is corrupt");
        case ERROR_INTERNAL_CELL_TABLE_CORRUPT:     RETURN_ASSERT_COVERAGE("[INTERNAL] CONS cell is corrupt");
        case ERROR_INTERNAL_STRING_TABLE_CORRUPT:   RETURN_ASSERT_COVERAGE("[INTERNAL] string table is corrupt");

        default: 
            break;
    }

    return "UNKNOWN ERROR";
}

YellowError::YellowError(ErrorCode code, const char* details) : 
    _code(code), 
    _details(details)
{
    std::stringstream ss;

    ss << GetDesc(_code);
    if (_details.length() > 0)
        ss << " (" << _details << ")";

    _finalMessage = ss.str();
}
