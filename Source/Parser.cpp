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

        NodeRef simplified = Simplify(element);
        if (simplified != nullptr)
            result.push_back(simplified);
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

        NodeRef quote = MakeIdentifierNode("quote");
        result = MakeListNode({ quote, ParseElement() });
    }
    else if (Consume('`'))
    {
        // `FOO -> (quasiquote FOO)

        NodeRef quasiquote = MakeIdentifierNode("quasiquote");
        result = MakeListNode({ quasiquote, ParseElement() });
    }
    else if (Consume(','))
    {
        // ,FOO -> (unquote FOO)

        NodeRef unquote = MakeIdentifierNode("unquote");
        result = MakeListNode({ unquote, ParseElement() });
    }
    else if (*_code)
    {
        result = ParseAtom();
    }

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

    NodeRef identNode = MakeIdentifierNode(ident);
    return identNode;
}

NodeRef Parser::MakeIdentifierNode(const string& ident)
{
    NodeRef node(new NodeVariant(AST_NODE_IDENTIFIER));
    node->_identifier = ident;
    return node;
}

NodeRef Parser::MakeListNode(const vector<NodeRef>& elems)
{
    NodeRef listNode(new NodeVariant(AST_NODE_LIST));
    listNode->_list = elems;
    return listNode;
}

NodeRef Parser::MakeIntNode(int value)
{
    NodeRef intNode(new NodeVariant(AST_NODE_INT_LITERAL));
    intNode->_int = value;
    return intNode;
}

NodeRef Parser::MakeFloatNode(float value)
{
    NodeRef floatNode(new NodeVariant(AST_NODE_FLOAT_LITERAL));
    floatNode->_float = value;
    return floatNode;
}

NodeRef Parser::MakeNumericNode(bool isInteger, double value)
{
    return isInteger? MakeIntNode((int) value) : MakeFloatNode((float) value);
}

void Parser::DumpSyntaxTree(NodeRef node, int indent)
{
    for (int i = 0; i < indent; i++)
        _console->Print(" ");

    switch (node->_type)
    {
        case AST_NODE_INT_LITERAL: 
            _console->PrintDebug("[int] %d\n", node->_int); 
            break;

        case AST_NODE_FLOAT_LITERAL:
            _console->PrintDebug("[float] %f\n", node->_float);
            break;

        case AST_NODE_STRING_LITERAL:
            _console->PrintDebug("[string] %s\n", node->_string.c_str());
            break;

        case AST_NODE_IDENTIFIER:
            _console->PrintDebug("[symbol] %s\n", node->_identifier.c_str());
            break;

        case AST_NODE_LIST:
            _console->PrintDebug("[list]...\n");
            for (auto& elem : node->_list)
                DumpSyntaxTree(elem, indent + 4);
            break;

        case AST_NODE_INVALID:
        default:
            RAISE_ERROR(ERROR_INTERNAL_AST_CORRUPT);
            break;
    }
}

NodeRef Parser::UntangleQuasiquotes(NodeRef node, int level)
{
    if (node->_type == AST_NODE_LIST)
    {
        if (node->_list.size() == 2)
        {
            NodeRef& head = node->_list[0];
            NodeRef& tail = node->_list[1];

            if (head->IsIdent("quote"))
            {
                if (level == 0)
                    return Simplify(tail);

                NodeRef expanded = UntangleQuasiquotes(tail, level);
                return MakeListNode({ head, expanded });
            }
            else if (head->IsIdent("unquote"))
            {
                RAISE_ERROR_IF(level < 1, ERROR_PARSER_INVALID_MACRO_EXPANSION, "you can't unquote what isn't quoted");

                if (level == 1)
                    return Simplify(tail);

                NodeRef expanded = UntangleQuasiquotes(tail, level - 1);
                return MakeListNode({ head, expanded });
            }
            else if (head->IsIdent("quasiquote"))
            {
                NodeRef expanded = UntangleQuasiquotes(tail, level + 1);

                if (level > 0)
                    return MakeListNode({ head, expanded });

                return expanded;
            }
        }

        NodeRef untangled(new NodeVariant(AST_NODE_LIST));

        for (int i = 0; i < node->_list.size(); i++)
            untangled->_list.push_back(UntangleQuasiquotes(node->_list[i], level));

        return untangled;
    }

    if (level == 0)
        return Simplify(node);

    return node;
}


NodeRef Parser::ExpandMacroBody(NodeRef node, map<string, NodeRef>& argValues)
{
    if (node->_type == AST_NODE_IDENTIFIER)
    {
        auto iter = argValues.find(node->_identifier);
        if (iter != argValues.end())
            return iter->second;
    }

    NodeRef clone(new NodeVariant(node->_type));
    switch(node->_type)
    {
        case AST_NODE_IDENTIFIER:       clone->_identifier = node->_identifier; break;
        case AST_NODE_INT_LITERAL:      clone->_int = node->_int; break;
        case AST_NODE_FLOAT_LITERAL:    clone->_float = node->_float; break;
        case AST_NODE_STRING_LITERAL:   clone->_string = node->_string; break;
        case AST_NODE_LIST:
            for (auto& elem : node->_list)
                clone->_list.push_back(ExpandMacroBody(elem, argValues));
            break;

        default:
            break;
    }

    return clone;
}

NodeRef Parser::Simplify(NodeRef node)
{
    if (node->_type == AST_NODE_LIST)
    {
        for (auto& elem : node->_list)
            elem = Simplify(elem);

        if (node->_list.size() > 0)
        {
            NodeRef& head = node->_list[0];

            if (head->IsIdent("defmacro"))
            {
                // Store macro definitions

                RAISE_ERROR_IF(node->_list.size() != 4, ERROR_PARSER_SYNTAX, "wrong number of paramters to defmacro");

                NodeRef& name = node->_list[1];
                NodeRef& args = node->_list[2];
                NodeRef& body = node->_list[3];

                RAISE_ERROR_IF(name->_type != AST_NODE_IDENTIFIER, ERROR_PARSER_SYNTAX, "expected macro name");
                RAISE_ERROR_IF(args->_type != AST_NODE_LIST, ERROR_PARSER_SYNTAX, "expected macro argument list");

                MacroDef& macroDef = _macros[name->_identifier];
                macroDef._macroBody = UntangleQuasiquotes(body);

                for (NodeRef arg : args->_list)
                {
                    RAISE_ERROR_IF(arg->_type != AST_NODE_IDENTIFIER, ERROR_PARSER_SYNTAX, "expected macro argument name");
                    macroDef._argNames.push_back(arg->_identifier);
                }

                // Macros will be expanded here in the parser, so there's nothing to evaluate now

                return NULL;
            }


            if (head->_type == AST_NODE_IDENTIFIER)
            {
                // Expand macro invocations

                auto iterMacro = _macros.find(head->_identifier);
                if (iterMacro != _macros.end())
                {
                    MacroDef& macroDef = iterMacro->second;
                    if (node->_list.size() != (macroDef._argNames.size() + 1))
                        RAISE_ERROR(ERROR_PARSER_INVALID_MACRO_EXPANSION, "wrong number of macro parameters");

                    map<string, NodeRef> argValues;
                    for (size_t i = 0; i < macroDef._argNames.size(); i++)
                        argValues[macroDef._argNames[i]] = Simplify(node->_list[i + 1]);

                    NodeRef expanded = ExpandMacroBody(macroDef._macroBody, argValues);
                    NodeRef simplified = Simplify(expanded);

                    return simplified;
                }
            }

            // Fold constant expressions

            if (head->IsIdent("cond"))
            {
                for (size_t i = 1; i < node->_list.size(); i++)
                {
                    NodeRef pair = node->_list[i];
                    RAISE_ERROR_IF(pair->_type != AST_NODE_LIST, ERROR_PARSER_SYNTAX, "parameters to COND must be lists");
                    RAISE_ERROR_IF(pair->_list.size() != 2, ERROR_PARSER_SYNTAX, "parameters to COND must be lists of two elements");

                    if (pair->_list[0]->IsIdent("t"))
                        return pair->_list[1];

                    if (!pair->_list[0]->IsIdent("nil"))
                        return node;
                }

                return MakeIdentifierNode("nil");
            }

            if (node->_list.size() == 3)
            {
                NodeRef a = node->_list[1];
                NodeRef b = node->_list[2];

                if (a->IsNumeric() && b->IsNumeric())
                {
                    double lhs = a->GetNumericValue();
                    double rhs = b->GetNumericValue();

                    bool isInteger = ((a->_type == AST_NODE_INT_LITERAL) && (b->_type == AST_NODE_INT_LITERAL));

                    if (head->IsIdent("+"))
                        return MakeNumericNode(isInteger, lhs + rhs);

                    if (head->IsIdent("-"))
                        return MakeNumericNode(isInteger, lhs - rhs);

                    if (head->IsIdent("*"))
                        return MakeNumericNode(isInteger, lhs * rhs);

                    if (head->IsIdent("/"))
                        return MakeNumericNode(isInteger, lhs / rhs);

                    if (head->IsIdent("="))
                        return MakeIdentifierNode((lhs == rhs)? "t" : "nil");

                    if (head->IsIdent("/="))
                        return MakeIdentifierNode((lhs != rhs)? "t" : "nil");

                    if (head->IsIdent("<"))
                        return MakeIdentifierNode((lhs < rhs)?  "t" : "nil");

                    if (head->IsIdent("<="))
                        return MakeIdentifierNode((lhs <= rhs)? "t" : "nil");

                    if (head->IsIdent(">"))
                        return MakeIdentifierNode((lhs > rhs)?  "t" : "nil");

                    if (head->IsIdent(">="))
                        return MakeIdentifierNode((lhs >= rhs)? "t" : "nil");
                }
            }
        }
    }

    return node;
}
