// YellowLISP (c) 2019 Stuart Riffle (MIT license)

#include "Yellow.h"
#include "Errors.h"

const char* YellowError::GetDesc(ErrorCode code)
{
    switch (code)
    {
        case ERROR_PARSER_SYNTAX:                   return "Syntax error";
        case ERROR_PARSER_STRING_UNTERMINATED:      return "String unterminated";
        case ERROR_PARSER_LIST_EXPECTED:            return "List expected";
        case ERROR_PARSER_LIST_UNTERMINATED:        return "List unterminated";
        case ERROR_PARSER_INVALID_IDENTIFIER:       return "Invalid identifier";
        case ERROR_PARSER_BRACE_MISMATCH:           return "Brace mismatch";
        case ERROR_RUNTIME_NOT_IMPLEMENTED:         return "Not implemented";
        case ERROR_RUNTIME_WRONG_NUM_PARAMS:        return "Wrong number of parameters";
        case ERROR_RUNTIME_VARIABLE_UNBOUND:        return "Variable is unbound";
        case ERROR_RUNTIME_TYPE_MISMATCH:           return "Type mismatch";
        case ERROR_RUNTIME_RESERVED_SYMBOL:         return "Reserved symbol";
        case ERROR_RUNTIME_UNDEFINED_FUNCTION:      return "Undefined function";
        case ERROR_RUNTIME_INVALID_ARGUMENT:        return "Invalid argument";
        case ERROR_INTERNAL_OUT_OF_MEMORY:          return "[INTERNAL] Out of memory";
        case ERROR_INTERNAL_PARSER_FAILURE:         return "[INTERNAL] Parsing failure";
        case ERROR_INTERNAL_HASH_COLLISION:         return "[INTERNAL] Hash collision";
        case ERROR_INTERNAL_SLOT_POOL_RANGE:        return "[INTERNAL] Slot pool access out of range";
        case ERROR_INTERNAL_AST_CORRUPT:            return "[INTERNAL] AST is corrupt";
        case ERROR_INTERNAL_CELL_TABLE_CORRUPT:     return "[INTERNAL] CONS cell is corrupt";
        case ERROR_INTERNAL_STRING_TABLE_CORRUPT:   return "[INTERNAL] String table is corrupt";
    }

    return "UNKNOWN ERROR";
}

YellowError::YellowError(ErrorCode code, const char* details) : 
    _code(code), 
    _details(details)
{
    std::stringstream ss;
    ss << "ERROR " << _code << ": ";

    ss << GetDesc(_code);

    if (_details.length() > 0)
        ss << " (" << _details << ")";

    _finalMessage = ss.str();
}
