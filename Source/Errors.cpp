#include "Yellow.h"
#include "Errors.h"

YellowError::YellowError(ErrorCode code, const char* details) : 
    _code(code), 
    _details(details)
{
    std::stringstream ss;
    ss << "ERROR " << _code << ": ";

    switch (_code)
    {
        case ERROR_PARSER_STRING_UNTERMINATED:      ss << "String unterminated"; break;
        case ERROR_PARSER_LIST_UNTERMINATED:        ss << "List unterminated"; break;
        case ERROR_PARSER_INVALID_IDENTIFIER:       ss << "Invalid identifier"; break;
        case ERROR_PARSER_BRACE_MISMATCH:           ss << "Brace mismatch"; break;

        case ERROR_RUNTIME_NOT_IMPLEMENTED:         ss << "Not implemented"; break;
        case ERROR_RUNTIME_WRONG_NUM_PARAMS:        ss << "Wrong number of parameters"; break;
        case ERROR_RUNTIME_VARIABLE_UNBOUND:        ss << "Variable is unbound"; break;
        case ERROR_RUNTIME_TYPE_MISMATCH:           ss << "Type mismatch"; break;

        case ERROR_INTERNAL_OUT_OF_MEMORY:          ss << "[INTERNAL] Out of memory"; break;
        case ERROR_INTERNAL_PARSER_FAILURE:         ss << "[INTERNAL] Parsing failure"; break;
        case ERROR_INTERNAL_HASH_COLLISION:         ss << "[INTERNAL] Hash collision"; break;
        case ERROR_INTERNAL_SLOT_POOL_RANGE:        ss << "[INTERNAL] Slot pool access out of range"; break;
        case ERROR_INTERNAL_AST_CORRUPT:            ss << "[INTERNAL] AST is corrupt"; break;
        case ERROR_INTERNAL_CELL_TABLE_CORRUPT:     ss << "[INTERNAL] CONS cell is corrupt"; break;
        case ERROR_INTERNAL_STRING_TABLE_CORRUPT:   ss << "[INTERNAL] String table is corrupt"; break;

        default:                                    ss << "Unknown error"; break;
    }

    if (_details.length() > 0)
        ss << " (" << _details << ")";

    _finalMessage = ss.str();
}
