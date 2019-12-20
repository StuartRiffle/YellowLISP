#pragma once

/*
// static
void Parser::TestParsing(const string& code)
{
    Parser parser;
    ParsingError error;

    list<NodeRef> expressions = parser.ParseExpressions(code, &error);
    assert(expressions.size() == 1);
    assert(expressions.front() != NULL);

    string rebuilt = expressions.front()->Serialize();
    assert(rebuilt == code);
}

// static
void Parser::RunUnitTest()
{
    TestParsing("()");
    TestParsing("'()");

    TestParsing("(())");
    TestParsing("('())");
    TestParsing("'(())");
    TestParsing("'((a))");

    TestParsing("(a)");
    TestParsing("'(a)");
    TestParsing("('a)");
    TestParsing("'('a)");

    TestParsing("(ab)");
    TestParsing("(ab (c de))");
    TestParsing("(abc123)");

    TestParsing("(a b)");
    TestParsing("(a 'b)");
    TestParsing("('a b)");
    TestParsing("'(a b)");

    TestParsing("(a (b))");
    TestParsing("(a () b)");
    TestParsing("((a) b)");
    TestParsing("((a) (b))");
    TestParsing("(a '(b))");
    TestParsing("(a ('b))");

    TestParsing("(a b c)");
    TestParsing("((a) b c)");
    TestParsing("(a (b) c)");
    TestParsing("(a b (c))");
    TestParsing("((a b) c)");
    TestParsing("(a (b c))");
    TestParsing("((a b c))");

    TestParsing("(123)");
    TestParsing("('123)");
    TestParsing("'(123)");

    TestParsing("(123.45)");
    TestParsing("('123.45)");
    TestParsing("'(123.45)");

    TestParsing("(123 45)");
    TestParsing("(123 45.67)");
    TestParsing("(123 four (5.67 eight))");
}
*/
