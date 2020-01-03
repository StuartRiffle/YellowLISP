// YellowLISP (c) 2020 Stuart Riffle (MIT license)

#pragma once
#include "Yellow.h"
#include "Console.h"

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

    bool IsIdent(string id) { return (_type == AST_NODE_IDENTIFIER) && (_identifier == id); }
    bool IsNumeric() { return (_type == AST_NODE_INT_LITERAL) || (_type == AST_NODE_FLOAT_LITERAL); }
    double GetNumericValue() { assert(IsNumeric()); return (_type == AST_NODE_INT_LITERAL)? _int : _float; }
};

struct MacroDef
{
    vector<string> _argNames;
    NodeRef _macroBody;
};

class Parser
{
    Console* _console;
    const char* _code;

    map<string, MacroDef> _macros;

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

    NodeRef MakeIdentifierNode(const string& ident);
    NodeRef MakeListNode(const vector<NodeRef>& elems);
    NodeRef MakeIntNode(int value);
    NodeRef MakeFloatNode(float value);
    NodeRef MakeNumericNode(bool isInteger, double value);


    NodeRef UntangleQuasiquotes(NodeRef node, int level = 0);
    NodeRef ExpandMacroBody(NodeRef node, map<string, NodeRef>& argValues);
    NodeRef ExpandMacro(string macroName, vector<NodeRef>& args);
    NodeRef Simplify(NodeRef node);

public:
    Parser(Console* console) : _console(console), _code(nullptr) {}

    list<NodeRef> ParseExpressionList(const string& source);
    void DumpSyntaxTree(NodeRef node, int indent = 0);

    static void TestParsing(const string& code);
    static void RunUnitTest();
};

