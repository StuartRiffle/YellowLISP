// YellowLISP (c) 2019 Stuart Riffle

#include "Parser.h"

const char* SYMBOL_CHARS = "!$%&*+-./:<=>?@^_~";

list<NodeRef> Parser::ParseExpressions(const string& source)
{
    list<NodeRef> result;
    _code = source.c_str();

    try
    {
        while (*_code)
        {
            NodeRef element = ParseElement();
            if (!element)
                break;

            result.push_back(element);
            SkipWhitespace();
        }
    }
    catch (const char* errorMessage)
    {
        throw FormatParsingError(source.c_str(), errorMessage);
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
    if (!Consume('(') && !Consume('['))
        throw "List expected";

    NodeRef listNode(new NodeVariant(AST_NODE_LIST));

    while (*_code && !Peek(')') && !Peek(']'))
        listNode->_list.push_back(ParseElement());

    if (!Consume(')') && !Consume(']'))
        throw "List is unterminated";

    return listNode;
}

NodeRef Parser::ParseAtom()
{
    if (Peek('\"'))
        return ParseString();

    if (isdigit(*_code) || ((_code[0] == '-') && isdigit(_code[1])))
        return ParseNumber();

    return ParseIdentifier();
}

NodeRef Parser::ParseString()
{
    if (!Consume('\"'))
        throw "String expected";

    // TODO: handle escape chars

    const char* end = strchr(_code, '\"');
    if (!end)
        throw "String is unterminated";

    string str(_code, end - _code);
    _code = end + 1;

    NodeRef stringNode(new NodeVariant(AST_NODE_STRING_LITERAL));
    stringNode->_string = str;
    return stringNode;
}

NodeRef Parser::ParseNumber()
{
    assert(!isspace(*_code));

    char* end = NULL;
    float val = strtof(_code, &end);

    if (end == _code)
        throw "Number expected";

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
    assert(!isspace(*_code));

    const char* end = _code;
    while (*end && (isalnum(*end) || strchr(SYMBOL_CHARS, *end)))
        end++;

    if (end == _code)
        throw "Invalid identifier";

    string ident(_code, end - _code);
    _code = end;

    NodeRef identNode(new NodeVariant(AST_NODE_IDENTIFIER));
    identNode->_identifier = ident;
    return identNode;
}

ParsingError Parser::FormatParsingError(const char* source, const char* errorMessage)
{
    ParsingError result;

    int line = 1;
    int column = 1;
    const char* errorLine = _code;

    for (const char* cursor = source; cursor < _code; cursor++)
    {
        if (*cursor == '\n')
        {
            line++;
            column = 1;
            errorLine = cursor + 1;
        }
        else
            column++;
    }

    std::stringstream ss;

    while (*errorLine && *errorLine != '\n')
        ss << *errorLine++;
    ss << std::endl;

    for (int i = 1; i < column - 1; i++)
        ss << ' ';
    ss << '^';
    ss << std::endl;

    result._line = line;
    result._column = column;
    result._message = errorMessage;
    result._extraInfo = ss.str();

    return result;
}
