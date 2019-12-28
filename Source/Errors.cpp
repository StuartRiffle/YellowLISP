// YellowLISP (c) 2019 Stuart Riffle (MIT license)

#include "Yellow.h"
#include "Errors.h"

const char* YellowError::GetDesc(ErrorCode code)
{
    switch (code)
    {
        case ERROR_PARSER_SYNTAX:                   return "syntax error";
        case ERROR_PARSER_STRING_UNTERMINATED:      return "string unterminated";
        case ERROR_PARSER_LIST_EXPECTED:            return "list expected";
        case ERROR_PARSER_LIST_UNTERMINATED:        return "list unterminated";
        case ERROR_PARSER_INVALID_IDENTIFIER:       return "invalid identifier";
        case ERROR_PARSER_BRACE_MISMATCH:           return "brace mismatch";
        case ERROR_RUNTIME_NOT_IMPLEMENTED:         return "not implemented";
        case ERROR_RUNTIME_WRONG_NUM_PARAMS:        return "wrong number of parameters";
        case ERROR_RUNTIME_VARIABLE_UNBOUND:        return "variable is unbound";
        case ERROR_RUNTIME_TYPE_MISMATCH:           return "type mismatch";
        case ERROR_RUNTIME_RESERVED_SYMBOL:         return "reserved symbol";
        case ERROR_RUNTIME_UNDEFINED_FUNCTION:      return "undefined function";
        case ERROR_RUNTIME_INVALID_ARGUMENT:        return "invalid argument";
        case ERROR_INTERNAL_OUT_OF_MEMORY:          return "[INTERNAL] out of memory";
        case ERROR_INTERNAL_PARSER_FAILURE:         return "[INTERNAL] parsing failure";
        case ERROR_INTERNAL_HASH_COLLISION:         return "[INTERNAL] hash collision";
        case ERROR_INTERNAL_SLOT_POOL_RANGE:        return "[INTERNAL] slot pool access out of range";
        case ERROR_INTERNAL_AST_CORRUPT:            return "[INTERNAL] AST is corrupt";
        case ERROR_INTERNAL_CELL_TABLE_CORRUPT:     return "[INTERNAL] CONS cell is corrupt";
        case ERROR_INTERNAL_STRING_TABLE_CORRUPT:   return "[INTERNAL] string table is corrupt";
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
