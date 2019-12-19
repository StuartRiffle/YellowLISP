// YellowLISP (c) 2019 Stuart Riffle

#pragma once
#include "SyntaxTree.h"

struct ParsingError : std::exception
{
    string _message;
    int _line;
    int _column;

    virtual const char* what() const { return _message.c_str(); }
};

class Parser
{
    const char* _code;

    inline void SkipWhitespace()
    {
        while (isspace(*_code))
            _code++;
    }

    inline bool Peek(char c)
    {
        for (;;)
        {
            SkipWhitespace();

            if (*_code != ';')
                break;

            while (*_code && *_code != '\n')
                _code++;
        }

        return (*_code == c);
    }

    inline bool Consume(char c)
    {
        if (!Peek(c))
            return false;

        _code++;
        return true;
    }

    NodeRef ParseElement();
    NodeRef ParseList();
    NodeRef ParseAtom();
    NodeRef ParseString();
    NodeRef ParseNumber();
    NodeRef ParseIdentifier();

    ParsingError FormatParsingError(const char* source, const char* errorMessage);

public:
    ParsingError ParseExpressions(const string& source);

    static void TestParsing(const string& code);
    static void RunUnitTest();
};

