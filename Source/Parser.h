// YellowLISP (c) 2019 Stuart Riffle

#pragma once
#include "Yellow.h"

enum NodeType
{
    AST_NODE_INVALID,
    AST_NODE_INT_LITERAL,
    AST_NODE_FLOAT_LITERAL,
    AST_NODE_STRING_LITERAL,
    AST_NODE_IDENTIFIER,
    AST_NODE_LIST,
};

struct NodeVariant;
typedef std::shared_ptr<NodeVariant> NodeRef;

struct NodeVariant
{
    NodeType _type;
    NodeVariant(NodeType type) : _type(type) {}

    vector<NodeRef> _list;
    string _identifier;
    string _string;
    int    _int;
    float  _float;
};

struct ParsingError : std::exception
{
    string _message;
    string _extraInfo;
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
    list<NodeRef> ParseExpressions(const string& source);

    static void TestParsing(const string& code);
    static void RunUnitTest();
};

