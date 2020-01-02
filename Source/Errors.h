// YellowLISP (c) 2020 Stuart Riffle (MIT license)

#pragma once
#include "Yellow.h"

enum ErrorCode
{
    ERROR_NONE,

    ERROR_PARSER_SYNTAX = 100,
    ERROR_PARSER_STRING_UNTERMINATED,
    ERROR_PARSER_LIST_EXPECTED,
    ERROR_PARSER_LIST_UNTERMINATED,
    ERROR_PARSER_INVALID_IDENTIFIER,
    ERROR_PARSER_INVALID_MACRO_EXPANSION,

    ERROR_RUNTIME_NOT_IMPLEMENTED = 200,
    ERROR_RUNTIME_WRONG_NUM_PARAMS,
    ERROR_RUNTIME_VARIABLE_UNBOUND,
    ERROR_RUNTIME_TYPE_MISMATCH,
    ERROR_RUNTIME_RESERVED_SYMBOL,
    ERROR_RUNTIME_UNDEFINED_FUNCTION,
    ERROR_RUNTIME_INVALID_ARGUMENT,

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

    YellowError(ErrorCode code, const char* details = "");
    virtual ~YellowError() {}

    virtual const char* what() const noexcept
    { 
        return _finalMessage.c_str();
    }

    static const char* GetDesc(ErrorCode code);
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
