// YellowLISP (c) 2019 Stuart Riffle (MIT license)

#include "Yellow.h"
#include "Console.h"
#include "Interpreter.h"

void CheckOutput(Interpreter& lisp, const char* source, const char* expectedOutput, ErrorCode expectedError = ERROR_NONE)
{
    ErrorCode caughtError = ERROR_NONE;
    string output;

    try
    {
        output = lisp.Evaluate(source);
    }
    catch (YellowError error)
    {
        caughtError = error._code;
    }

    string shouldHave;
    string actually;

    std::stringstream ss;
    ss << '\"' << source << '\"';

    if (expectedOutput)
    {
        if (output == expectedOutput)
            return;

        ss << " should output \"" << expectedOutput << "\", ";
    }
    else if (expectedError)
    {
        if (caughtError == expectedError)
            return;

        ss << " should raise error " << expectedError << " (" << YellowError::GetDesc(expectedError) << "), ";
    }
    else
        assert(0);

    if(caughtError)
        ss << "but raised error " << caughtError << " (" << YellowError::GetDesc(caughtError) << ")";
    else 
        ss << "but output \"" << output << "\"";

    SetTextColor(ANSI_RED);
    printf("SANITY CHECK FAILED: "); 
    ResetTextColor();

    printf("% s\n", ss.str().c_str());
}

void CheckOutput(Interpreter& lisp, const char* source, ErrorCode expectedError)
{
    CheckOutput(lisp, source, NULL, expectedError);
}

#define VERIFY(_IN, _OUT) CheckOutput(lisp, _IN, _OUT)

void SanityCheck()
{
    InterpreterSettings settings;
    settings._repl = false;
    settings._catchExceptions = false;

    Interpreter lisp(&settings);

    VERIFY("", "");
    VERIFY(";(", "");
    VERIFY("\n", "");
    VERIFY("1", "1");
    VERIFY("-1", "-1");
    VERIFY(" 2 ", "2");
    VERIFY("3.4", "3.4");
    VERIFY("-5.6", "-5.6");
    VERIFY("78e-3", "0.078");
    VERIFY("9 ; (", "9");

    VERIFY("t", "t");
    VERIFY("nil", "nil");
    VERIFY("'nil", "nil");
    VERIFY("()", "nil");
    VERIFY("'()", "nil");

    VERIFY("'foo", "foo");
    VERIFY("'FOO", "FOO");
    VERIFY("\"foo\"", "\"foo\"");
    VERIFY("\"FOO\"", "\"FOO\"");

    VERIFY("(quote ('1 2))", "((quote 1) 2)");
    VERIFY("'('1 2)", "((quote 1) 2)");

    VERIFY("(atom nil)", "t");
    VERIFY("(atom ())", "t");
    VERIFY("(atom '())", "t");
    VERIFY("(atom 3)", "t");
    VERIFY("(atom 'foo)", "t");
    VERIFY("(atom (atom 3))", "t");
    VERIFY("(atom [atom 3])", "t");
    VERIFY("[atom [atom 3]]", "t");
    VERIFY("[atom (atom 3)]", "t");
    VERIFY("(atom '(atom 3))", "nil");
    VERIFY("(atom (list 1 2))", "nil");
    VERIFY("(atom (cons 1 2))", "nil");

    VERIFY("(car (list 1 2))", "1");
    VERIFY("(cdr (list 1 2))", "(2)");
    VERIFY("(car ())", "nil");
    VERIFY("(cdr ())", "nil");
    VERIFY("(car (cdr (list 1 2 3)))", "2");
    VERIFY("(cdr (cdr (list 1 2 3)))", "(3)");

    VERIFY("(list)", "nil");
    VERIFY("(list 1)", "(1)");
    VERIFY("(list 1 'foo 3)", "(1 foo 3)");
    VERIFY("(list (list ()))", "((nil))");

    VERIFY("(setq x (list 4 5 6))", "(4 5 6)");
    VERIFY("(setq x '(4 5 6))", "(4 5 6)");
    VERIFY("(atom x)", "nil");
    VERIFY("(atom 'x)", "t");
    VERIFY("(setq y x)", "(4 5 6)");
    VERIFY("(setq y 'x)", "x");
    VERIFY("(atom y)", "t");
    VERIFY("y", "x");
    VERIFY("(eval y)", "(4 5 6)");


    VERIFY("(< 1 2)", "t");
    VERIFY("(< 2 1)", "nil");
    VERIFY("(< 1 1)", "nil");
    VERIFY("(< -2 -1)", "t");
    VERIFY("(< -1 -2)", "nil");
    VERIFY("(< 1.1 2.1)", "t");
    VERIFY("(< 1 2.1)", "t");
    VERIFY("(< -1.1 2)", "t");

    VERIFY("(defun sqr (x) (* x x))", "sqr");
    VERIFY("(sqr 5)", "25");

    VERIFY("(cons 'a 'b)", "(a . b)");
    VERIFY("(cons (list 'a) 'b)", "((a) . b)");
    VERIFY("(cons 'a (list 'b))", "(a b)");
    VERIFY("(cons (list 'a) (list 'b))", "((a) b)");

    //VERIFY("(append '(a b c) '())", "(a b c)");
    //VERIFY("(append '() '(a b c))", "(a b c)");
    //VERIFY("(append '(a b) '(c d))", "(a b c d)");
    //VERIFY("(append '(a b) 'c)", "(a b . c)");
    //VERIFY("(append '(a b c) '(d e f) '() '(g))", "(a b c d e f g)");
    //VERIFY("(append '(a b c) 'd)", "(a b c . d)");
    //VERIFY("(setq lst '(a b c))", "(a b c)");
    //VERIFY("(append lst '(d))", "(a b c d)");
    //VERIFY("lst", "(a b c)");
    //VERIFY("(append)", "nil");
    //VERIFY("(append 'a)", "a");

    // TODO: dot notation
    // TODO: backquote

    // The error section needs a *lot* more test cases

    VERIFY("\"foo",       ERROR_PARSER_STRING_UNTERMINATED);
    VERIFY("(",           ERROR_PARSER_LIST_UNTERMINATED);
    VERIFY("(setq } 3)",  ERROR_PARSER_INVALID_IDENTIFIER);
    VERIFY("(setq quote 3)", ERROR_RUNTIME_RESERVED_SYMBOL);
    VERIFY("(foo 1 2)",   ERROR_RUNTIME_UNDEFINED_FUNCTION);
    VERIFY("(atom 3]",    ERROR_PARSER_BRACE_MISMATCH);
    VERIFY("(atom [3)]",  ERROR_PARSER_BRACE_MISMATCH);
    VERIFY("(quote 1 2)", ERROR_RUNTIME_WRONG_NUM_PARAMS);
    VERIFY("(atom unbound)", ERROR_RUNTIME_VARIABLE_UNBOUND);
    VERIFY("(< 1 nil)",   ERROR_RUNTIME_TYPE_MISMATCH);
}


/*

*/
