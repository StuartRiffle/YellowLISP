// YellowLISP (c) 2020 Stuart Riffle (MIT license)

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

    if (Consume('('))
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

    /*
    if (result && Consume('.'))
    {
        // Convert A . B to (cons A 'B)

        NodeRef cons  = IdentifierNode("cons");
        NodeRef quote = IdentifierNode("quote");
        NodeRef rhs = ParseElement();
        NodeRef quoted = ListNode({ quote, rhs });

        result = ListNode({ cons, result, quoted });
    }
    */

    return result;
}

NodeRef Parser::ParseList()
{
    NodeRef listNode(new NodeVariant(AST_NODE_LIST));

    while (*_code && !Peek(')'))
        listNode->_list.push_back(ParseElement());

    if (!Consume(')'))
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

    RAISE_ERROR_IF(end == _code, ERROR_PARSER_SYNTAX, "invalid number");

    bool isFloat = false;
    for (const char* c = _code + 1; c < end; c++)
        if ((*c == '.') || (*c == 'e') || (*c == 'E'))
            isFloat = true;

    if (!isFloat)
        assert(val == (int) val);

    _code = end;

    if (isFloat)
    {
        NodeRef floatNode(new NodeVariant(AST_NODE_FLOAT_LITERAL));
        floatNode->_float = val;
        return floatNode;
    }

    NodeRef intNode(new NodeVariant(AST_NODE_INT_LITERAL));
    intNode->_int = (int) val;
    return intNode;
}

NodeRef Parser::ParseIdentifier()
{
    RAISE_ERROR_IF(isspace(*_code), ERROR_INTERNAL_PARSER_FAILURE);

    const char* end = _code;

    if (Consume('|'))
    {
        end = strchr(_code, '|');
        RAISE_ERROR_IF(!end, ERROR_PARSER_INVALID_IDENTIFIER, "pipe-style identifier unterminated");
    }
    else
    {
        while (*end && (isalnum(*end) || strchr(SYMBOL_CHARS, *end)))
            end++;
    }

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

NodeRef Parser::Simplify(NodeRef node)
{
    /*
    // How to resolve constants over multiple calls to parse? Implies the
    // AST needs to hang around?

    switch (node->_type)
    {
        case AST_NODE_IDENTIFIER:
            return node;

        case AST_NODE_LIST:
        {
            for (auto& elem : node->_list)
                elem = Simplify(elem);

            if (node->_list.size() < 1)
                break;

            if (node->IsIdent("defmacro"))
            {
                // Expand macro and store in macro table
            }
            {
                NodeRef funcNode = Simplify(node->_list[0]);
                if (funcNode->IsIdent("+")

                if (funcNode->_type == AST_NODE_IDENTIFIER)
                {
                    string ident = funcNode->_identifier;
                    if (ident == '+')
                }
            }
            if (_node->_identifier == "defmacro")
            {
                string 
            }
        }

        case AST_NODE_INT_LITERAL: 
        case AST_NODE_FLOAT_LITERAL:
        case AST_NODE_STRING_LITERAL:
        default:
            break;
    }

    */
    return node;
}
