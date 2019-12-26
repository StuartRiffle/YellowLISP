// YellowLISP (c) 2019 Stuart Riffle (MIT license)

#include "Yellow.h"
#include "Parser.h"
#include "Errors.h"

// FIXME: implement in LISP instead

NodeRef Parser::ParseArithmeticExpression()
{
    NodeRef expr = ParseArithmeticAddSub();
    return expr;
}

NodeRef Parser::ParseArithmeticAddSub()
{
    NodeRef expr = ParseArithmeticMulDiv();

    while (Peek('+') || Peek('-'))
    {
        char op = *_code++;

        NodeRef other = ParseArithmeticExpression();

        NodeRef opNode(new NodeVariant(AST_NODE_IDENTIFIER));
        opNode->_identifier = op;

        NodeRef listNode(new NodeVariant(AST_NODE_LIST));
        listNode->_list.push_back(opNode);
        listNode->_list.push_back(expr);
        listNode->_list.push_back(other);

        expr = listNode;
    }


    return expr;
}

NodeRef Parser::ParseArithmeticMulDiv()
{
    NodeRef expr = ParseArithmeticUnary();

    while (Peek('*') || Peek('/') || Peek('%'))
    {
        char op = *_code++;

        NodeRef other = ParseArithmeticExpression();

        NodeRef opNode(new NodeVariant(AST_NODE_IDENTIFIER));
        opNode->_identifier = op;

        NodeRef listNode(new NodeVariant(AST_NODE_LIST));
        listNode->_list.push_back(opNode);
        listNode->_list.push_back(expr);
        listNode->_list.push_back(other);

        expr = listNode;
    }

    return expr;
}

NodeRef Parser::ParseArithmeticUnary()
{
    if (Consume('-'))
    {
        NodeRef opNode(new NodeVariant(AST_NODE_IDENTIFIER));
        opNode->_identifier = '*';

        NodeRef negativeOne(new NodeVariant(AST_NODE_INT_LITERAL));
        negativeOne->_int = -1;

        NodeRef listNode(new NodeVariant(AST_NODE_LIST));
        listNode->_list.push_back(opNode);
        listNode->_list.push_back(negativeOne);
        listNode->_list.push_back(ParseArithmeticTerm());

        return listNode;
    }

    return ParseArithmeticTerm();
}

NodeRef Parser::ParseArithmeticTerm()
{
    NodeRef term;

    if (isdigit(*_code))
    {
        term = ParseNumber();
    }
    else if (Consume('('))
    {
        term = ParseArithmeticExpression();

        if (!Consume(')'))
            RAISE_ERROR(ERROR_PARSER_SYNTAX, "expected ')'");
    }
    else
    {
        term = ParseIdentifier();
    }

    if (!term)
        RAISE_ERROR(ERROR_PARSER_SYNTAX);

    return term;
}

