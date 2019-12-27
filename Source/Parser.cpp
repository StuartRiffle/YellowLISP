// YellowLISP (c) 2019 Stuart Riffle (MIT license)

#include "Parser.h"
#include "Errors.h"

const char* SYMBOL_CHARS = "!$%&*+-./:<=>?@^_~";

list<NodeRef> Parser::ParseExpressionList(const string& source)
{
    list<NodeRef> result;
    _code = source.c_str();

    while (*_code)
    {
        SkipWhitespace();

        NodeRef element = ParseElement();
        if (!element)
            break;

        result.push_back(element);

        //DumpSyntaxTree(element);
    }

    return result;
}

NodeRef Parser::ParseElement()
{
    NodeRef result;

    if (Peek('(') || Peek('['))
    {
        result = ParseList();
    }
    else if (Consume('\''))
    {
        // 'FOO -> (quote FOO)

        NodeRef quote = IdentifierNode("quote");
        result = ListNode({ quote, ParseElement() });
    }
    else if (Consume('`'))
    {
        // `FOO -> (quasiquote FOO)

        NodeRef quasiquote = IdentifierNode("quasiquote");
        result = ListNode({ quasiquote, ParseElement() });
    }
    else if (Consume(','))
    {
        // ,FOO -> (unquote FOO)

        NodeRef unquote = IdentifierNode("unquote");
        result = ListNode({ unquote, ParseElement() });
    }
    else if (*_code)
    {
        result = ParseAtom();
    }

    if (result && Consume('.'))
    {
        // Convert A . B to (cons A B)

        NodeRef cons = IdentifierNode("cons");
        result = ListNode({ cons, result, ParseElement() });
    }

    return result;
}

NodeRef Parser::ParseList()
{
    char matchingBrace = Consume('(') ? ')' : Consume('[') ? ']' : 0;
    char wrongBrace = (matchingBrace == ')') ? ']' : ')';

    RAISE_ERROR_IF(!matchingBrace, ERROR_PARSER_LIST_EXPECTED);

    NodeRef listNode(new NodeVariant(AST_NODE_LIST));

    while (*_code && !Peek(')') && !Peek(']'))
        listNode->_list.push_back(ParseElement());

    if (Consume(wrongBrace))
        RAISE_ERROR(ERROR_PARSER_BRACE_MISMATCH);

    if (!Consume(matchingBrace))
        RAISE_ERROR(ERROR_PARSER_LIST_UNTERMINATED);
    
    return listNode;
}

NodeRef Parser::ParseAtom()
{
    if (Consume('\"'))
        return ParseString();

    if (isdigit(*_code) || ((_code[0] == '-') && isdigit(_code[1])))
        return ParseNumber();

    return ParseIdentifier();
}

NodeRef Parser::ParseString()
{
    // TODO: handle escape chars

    const char* end = strchr(_code, '\"');
    if (!end)
        RAISE_ERROR(ERROR_PARSER_STRING_UNTERMINATED);

    string str(_code, end - _code);
    _code = end + 1;

    NodeRef stringNode(new NodeVariant(AST_NODE_STRING_LITERAL));
    stringNode->_string = str;
    return stringNode;
}

NodeRef Parser::ParseNumber()
{
    RAISE_ERROR_IF(isspace(*_code), ERROR_INTERNAL_PARSER_FAILURE);

    char* end = NULL;
    float val = strtof(_code, &end);

    assert(end > _code);
    _code = end;

    int integer = (int)val;
    if (integer == val)
    {
        NodeRef intNode(new NodeVariant(AST_NODE_INT_LITERAL));
        intNode->_int = integer;
        return intNode;
    }

    NodeRef floatNode(new NodeVariant(AST_NODE_FLOAT_LITERAL));
    floatNode->_float = val;
    return floatNode;
}

NodeRef Parser::ParseIdentifier()
{
    RAISE_ERROR_IF(isspace(*_code), ERROR_INTERNAL_PARSER_FAILURE);

    const char* end = _code;
    while (*end && (isalnum(*end) || strchr(SYMBOL_CHARS, *end)))
        end++;

    if (end == _code)
        RAISE_ERROR(ERROR_PARSER_INVALID_IDENTIFIER);

    string ident(_code, end - _code);
    _code = end;

    NodeRef identNode = IdentifierNode(ident);
    return identNode;
}

NodeRef Parser::IdentifierNode(const string& ident)
{
    NodeRef node(new NodeVariant(AST_NODE_IDENTIFIER));
    node->_identifier = ident;

    return node;
}

NodeRef Parser::ListNode(const vector<NodeRef>& elems)
{
    NodeRef listNode(new NodeVariant(AST_NODE_LIST));
    listNode->_list = elems;

    return listNode;
}

void Parser::DumpSyntaxTree(NodeRef node, int indent)
{
    for (int i = 0; i < indent; i++)
        std::cout << ' ';

    switch (node->_type)
    {
        case AST_NODE_INT_LITERAL: 
            std::cout << "[int]    " << node->_int << std::endl; 
            break;

        case AST_NODE_FLOAT_LITERAL:
            std::cout << "[float]  " << node->_float << std::endl;
            break;

        case AST_NODE_STRING_LITERAL:
            std::cout << "[string] " << node->_string << std::endl;
            break;

        case AST_NODE_IDENTIFIER:
            std::cout << "[symbol] " << node->_identifier << std::endl;
            break;

        case AST_NODE_LIST:
            std::cout << "[list]" << std::endl;
            for (auto& elem : node->_list)
                DumpSyntaxTree(elem, indent + 4);
            break;

        case AST_NODE_INVALID:
        default:
            RAISE_ERROR(ERROR_INTERNAL_AST_CORRUPT);
            break;
    }
}

