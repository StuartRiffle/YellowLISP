// YellowLISP (c) 2020 Stuart Riffle (MIT license)

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
    NodeRef _dotted;

    bool IsIdent(string id) { return (_type == AST_NODE_IDENTIFIER) && (_identifier == id); }
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

    NodeRef ParseExpression();
    NodeRef ParseElement();
    NodeRef ParseList();
    NodeRef ParseAtom();
    NodeRef ParseString();
    NodeRef ParseNumber();
    NodeRef ParseIdentifier();

    NodeRef IdentifierNode(const string& ident);
    NodeRef ListNode(const vector<NodeRef>& elems);

    NodeRef Simplify(NodeRef node);

public:
    list<NodeRef> ParseExpressionList(const string& source);
    void DumpSyntaxTree(NodeRef node, int indent = 0);

    static void TestParsing(const string& code);
    static void RunUnitTest();
};

