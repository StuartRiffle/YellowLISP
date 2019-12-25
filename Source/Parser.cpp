// YellowLISP (c) 2019 Stuart Riffle

#include "Parser.h"
#include "Errors.h"

const char* SYMBOL_CHARS = "!$%&*+-./:<=>?@^_~";

list<NodeRef> Parser::ParseExpressions(const string& source)
{
    list<NodeRef> result;
    _code = source.c_str();

    while (*_code)
    {
        NodeRef element = ParseElement();
        if (!element)
            break;

        result.push_back(element);
        SkipWhitespace();

        //DumpSyntaxTree(element);
    }

    return result;
}

NodeRef Parser::ParseElement()
{
    NodeRef result;

    if (Consume('\''))
    {
        // Sugar: convert 'FOO to (quote FOO)

        NodeRef quote(new NodeVariant(AST_NODE_IDENTIFIER));
        quote->_identifier = "quote";

        NodeRef quoteNode(new NodeVariant(AST_NODE_LIST));
        quoteNode->_list.push_back(quote);
        quoteNode->_list.push_back(ParseElement());

        result = quoteNode;
    }
    else if (Consume('{'))
    {
        // Sugar: evaluate expressions between curly braces
        // using normal arithmetic, using the normal meaning
        // of parentheses as grouping, like normal people.
        //
        // These expressions are equivalent in YellowLISP:
        //
        //      (+ (* (+ a b) c) 123) 
        //      { (a + b) * c + 123 }

        result = ParseArithmeticExpression();

        if (!Consume('}'))
            RAISE_ERROR(ERROR_PARSER_SYNTAX, "expected '}'");
    }
    else if (Peek('(') || Peek('['))
    {
        result = ParseList();
    }
    else if (*_code)
    {
        result = ParseAtom();
    }

    if (Consume('.'))
    {
        // Sugar: convert A . B to (cons A B)

        NodeRef cons(new NodeVariant(AST_NODE_IDENTIFIER));
        cons->_identifier = "cons";

        NodeRef consNode(new NodeVariant(AST_NODE_LIST));
        consNode->_list.push_back(cons);
        consNode->_list.push_back(result);
        consNode->_list.push_back(ParseElement());

        result = consNode;
    }

    return result;
}

NodeRef Parser::ParseList()
{
    char matchingBrace = Consume('(') ? ')' : Consume('[') ? ']' : 0;
    char wrongBrace = (matchingBrace == ')') ? ']' : ')';
    assert(matchingBrace);

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

    NodeRef identNode(new NodeVariant(AST_NODE_IDENTIFIER));
    identNode->_identifier = ident;
    return identNode;
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

