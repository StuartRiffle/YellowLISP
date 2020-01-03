// YellowLISP (c) 2020 Stuart Riffle (MIT license)

#include "Yellow.h"
#include "Errors.h"
#include "Testing.h"

const char* YellowError::GetDesc(ErrorCode code)
{
    switch (code)
    {
        case ERROR_PARSER_SYNTAX:                   RETURN_WITH_COVERAGE("syntax error");
        case ERROR_PARSER_STRING_UNTERMINATED:      RETURN_WITH_COVERAGE("string unterminated");
        case ERROR_PARSER_LIST_EXPECTED:            RETURN_WITH_COVERAGE("list expected");
        case ERROR_PARSER_LIST_UNTERMINATED:        RETURN_WITH_COVERAGE("list unterminated");
        case ERROR_PARSER_INVALID_IDENTIFIER:       RETURN_WITH_COVERAGE("invalid identifier");
        case ERROR_PARSER_INVALID_MACRO_EXPANSION:  RETURN_WITH_COVERAGE("invalid macro expansion");

        case ERROR_RUNTIME_NOT_IMPLEMENTED:         RETURN_WITH_COVERAGE("not implemented");
        case ERROR_RUNTIME_WRONG_NUM_PARAMS:        RETURN_WITH_COVERAGE("wrong number of parameters");
        case ERROR_RUNTIME_VARIABLE_UNBOUND:        RETURN_WITH_COVERAGE("variable is unbound");
        case ERROR_RUNTIME_TYPE_MISMATCH:           RETURN_WITH_COVERAGE("type mismatch");
        case ERROR_RUNTIME_RESERVED_SYMBOL:         RETURN_WITH_COVERAGE("reserved symbol");
        case ERROR_RUNTIME_UNDEFINED_FUNCTION:      RETURN_WITH_COVERAGE("undefined function");
        case ERROR_RUNTIME_INVALID_ARGUMENT:        RETURN_WITH_COVERAGE("invalid argument");

        case ERROR_INTERNAL_OUT_OF_MEMORY:          RETURN_WITH_COVERAGE("[INTERNAL] out of memory");
        case ERROR_INTERNAL_PARSER_FAILURE:         RETURN_WITH_COVERAGE("[INTERNAL] parsing failure");
        case ERROR_INTERNAL_RUNTIME_FAILURE:        RETURN_WITH_COVERAGE("[INTERNAL] runtime failure");
        case ERROR_INTERNAL_HASH_COLLISION:         RETURN_WITH_COVERAGE("[INTERNAL] hash collision");
        case ERROR_INTERNAL_SLOT_POOL_RANGE:        RETURN_WITH_COVERAGE("[INTERNAL] slot pool access out of range");
        case ERROR_INTERNAL_AST_CORRUPT:            RETURN_WITH_COVERAGE("[INTERNAL] AST is corrupt");
        case ERROR_INTERNAL_CELL_TABLE_CORRUPT:     RETURN_WITH_COVERAGE("[INTERNAL] CONS cell is corrupt");
        case ERROR_INTERNAL_STRING_TABLE_CORRUPT:   RETURN_WITH_COVERAGE("[INTERNAL] string table is corrupt");

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
