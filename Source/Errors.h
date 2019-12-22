#pragma once
#include "Yellow.h"

enum ErrorCode
{
    ERROR_NONE,

    ERROR_PARSER_NUMBER_EXPECTED = 100,
    ERROR_PARSER_STRING_EXPECTED,
    ERROR_PARSER_STRING_UNTERMINATED,
    ERROR_PARSER_LIST_EXPECTED,
    ERROR_PARSER_LIST_UNTERMINATED,
    ERROR_PARSER_INVALID_IDENTIFIER,

    ERROR_RUNTIME_NOT_IMPLEMENTED = 200,
    ERROR_RUNTIME_WRONG_NUM_PARAMS,
    ERROR_RUNTIME_INVALID_PARAMETER_TYPE,

    ERROR_INTERNAL_OUT_OF_MEMORY = 300,
    ERROR_INTERNAL_PARSER_FAILURE,
    ERROR_INTERNAL_HASH_COLLISION,
    ERROR_INTERNAL_SLOT_POOL_RANGE,
    ERROR_INTERNAL_AST_CORRUPT,
    ERROR_INTERNAL_CELL_TABLE_CORRUPT,
    ERROR_INTERNAL_STRING_TABLE_CORRUPT,
};

struct YellowError : std::exception
{
    ErrorCode _code;
    string _details;
    string _finalMessage;

    YellowError(ErrorCode code, const char* details = "") : _code(code), _details(details) 
    {
        std::stringstream ss;
        ss << "ERROR " << _code << ": ";

        switch (_code)
        {
            case ERROR_PARSER_NUMBER_EXPECTED:          ss << "Number expected"; break;
            case ERROR_PARSER_STRING_EXPECTED:          ss << "String expected"; break;
            case ERROR_PARSER_STRING_UNTERMINATED:      ss << "String unterminated"; break;
            case ERROR_PARSER_LIST_EXPECTED:            ss << "List expected"; break;
            case ERROR_PARSER_LIST_UNTERMINATED:        ss << "List unterminated"; break;
            case ERROR_PARSER_INVALID_IDENTIFIER:       ss << "Invalid identifier"; break;

            case ERROR_RUNTIME_NOT_IMPLEMENTED:         ss << "Not implemented"; break;
            case ERROR_RUNTIME_WRONG_NUM_PARAMS:        ss << "Wrong number of parameters"; break;
            case ERROR_RUNTIME_INVALID_PARAMETER_TYPE:  ss << "Wrong parameter type"; break;

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

    virtual const char* what() const 
    { 
        return _finalMessage.c_str();
    }

};

inline void RAISE_ERROR(ErrorCode code, const char* details = "")
{
    throw YellowError(code, details);
}

inline void RAISE_ERROR_IF(bool condition, ErrorCode code, const char* details = "")
{
    if (condition)
        RAISE_ERROR(code, details);
}

inline void VERIFY_NUM_PARAMETERS(size_t num, size_t expected, const char* functionName)
{
    RAISE_ERROR_IF(num != expected, ERROR_RUNTIME_WRONG_NUM_PARAMS, functionName);
}
