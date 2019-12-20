// YellowLISP (c) 2019 Stuart Riffle

#include "Parser.h"

const char QUOTE_CHAR = '\'';
const char STRING_DELIM = '\"';
const char OPEN_PAREN = '(';
const char CLOSE_PAREN = ')';
const char MINUS_SIGN = '-';
const char* SYMBOL_CHARS = "!$%&*+-./:<=>?@^_~";

list<NodeRef> Parser::ParseExpressions(const string& source)
{
    list<NodeRef> result;
    _code = source.c_str();

    try
    {
        while (*_code)
        {
            result.push_back(ParseElement());
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
    if (Consume(QUOTE_CHAR))
    {
        NodeRef quotePrimitive(new NodeVariant(AST_NODE_IDENTIFIER));
        quotePrimitive->_identifier = "quote";

        NodeRef quoteNode(new NodeVariant(AST_NODE_LIST));
        quoteNode->_list.push_back(quotePrimitive);
        quoteNode->_list.push_back(ParseElement());

        return quoteNode;
    }

    if (Peek(OPEN_PAREN))
        return ParseList();

    return ParseAtom();
}

NodeRef Parser::ParseList()
{
    if (!Consume(OPEN_PAREN))
        throw "List expected";

    NodeRef listNode(new NodeVariant(AST_NODE_LIST));
    while (!Peek(CLOSE_PAREN))
        listNode->_list.push_back(ParseElement());

    if (!Consume(CLOSE_PAREN))
        throw "List is unterminated";

    return listNode;
}

NodeRef Parser::ParseAtom()
{
    if (Peek(STRING_DELIM))
        return ParseString();

    if (isdigit(*_code) || ((_code[0] == '-') && isdigit(_code[1])))
        return ParseNumber();

    return ParseIdentifier();
}

NodeRef Parser::ParseString()
{
    if (!Consume(STRING_DELIM))
        throw "String expected";

    // TODO: handle escape chars

    const char* end = strchr(_code, STRING_DELIM);
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
