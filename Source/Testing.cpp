#include "Yellow.h"
#include "Console.h"
#include "Interpreter.h"

void CheckOutput(Interpreter& lisp, const char* source, const char* expectedOutput)
{
    string output = lisp.Evaluate(source);
    if (output != expectedOutput)
    {
        SetTextColor(ANSI_RED);
        printf("SANITY CHECK FAILED: ");
        ResetTextColor();

        printf("\"%s\" evaluates to \"%s\" instead of \"%s\"\n", source, output.c_str(), expectedOutput);
    }
}

void CheckError(Interpreter& lisp, const char* source, ErrorCode expectedError)
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

    if (caughtError != expectedError)
    {
        std::stringstream ss;
        ss << '\"' << source << '\"' << " should raise error " << expectedError;

        if (caughtError)
            ss << ", but raised " << caughtError;
        else
            ss << ", but evaluated to \"" << output << '\"';

        SetTextColor(ANSI_RED);
        printf("SANITY CHECK FAILED: "); 
        ResetTextColor();

        printf("% s\n", ss.str().c_str());
    }
}

void SanityCheck()
{
    InterpreterSettings settings;
    settings._repl = false;

    Interpreter lisp(&settings);

    CheckOutput(lisp, "", "");
    CheckOutput(lisp, ";(", "");
    CheckOutput(lisp, "\n", "");
    CheckOutput(lisp, "1", "1");
    CheckOutput(lisp, " 2 ", "2");
    CheckOutput(lisp, "3.4", "3.4");
    CheckOutput(lisp, "-5.6", "-5.6");
    CheckOutput(lisp, "78e-3", "0.078");
    CheckOutput(lisp, "9 ; (", "9");

    CheckOutput(lisp, "t", "t");
    CheckOutput(lisp, "nil", "nil");
    CheckOutput(lisp, "'nil", "nil");
    CheckOutput(lisp, "()", "nil");
    CheckOutput(lisp, "'()", "nil");

    CheckOutput(lisp, "'foo", "foo");
    CheckOutput(lisp, "'FOO", "FOO");
    CheckOutput(lisp, "\"foo\"", "\"foo\"");
    CheckOutput(lisp, "\"FOO\"", "\"FOO\"");

    CheckOutput(lisp, "(quote ('1 2))", "((quote 1) 2)");

    CheckOutput(lisp, "(atom ())", "t");
    CheckOutput(lisp, "(atom '())", "t");
    CheckOutput(lisp, "(atom 3)", "t");
    CheckOutput(lisp, "(atom 'foo)", "t");
    CheckOutput(lisp, "(atom (atom 3))", "t");
    CheckOutput(lisp, "(atom [atom 3])", "t");
    CheckOutput(lisp, "[atom [atom 3]]", "t");
    CheckOutput(lisp, "[atom (atom 3)]", "t");
    CheckOutput(lisp, "(atom '(atom 3))", "nil");
    CheckOutput(lisp, "(atom (list 1 2))", "nil");

    CheckOutput(lisp, "(car (list 1 2))", "1");
    CheckOutput(lisp, "(cdr (list 1 2))", "(2)");
    CheckOutput(lisp, "(car ())", "nil");
    CheckOutput(lisp, "(cdr ())", "nil");
    CheckOutput(lisp, "(car (cdr (list 1 2 3)))", "2");
    CheckOutput(lisp, "(cdr (cdr (list 1 2 3)))", "(3)");

    CheckOutput(lisp, "(list)", "nil");
    CheckOutput(lisp, "(list 1)", "(1)");
    CheckOutput(lisp, "(list 1 'foo 3)", "(1 foo 3)");
    CheckOutput(lisp, "(list (list ()))", "((nil))");

    CheckOutput(lisp, "(< 1 2)",    "t");
    CheckOutput(lisp, "(< 2 1)",    "nil");
    CheckOutput(lisp, "(< 1 1)",    "nil");
    CheckOutput(lisp, "(< -2 -1)",  "t");
    CheckOutput(lisp, "(< -1 -2)",  "nil");

    settings._catchExceptions = false;

    CheckError(lisp, "\"foo",       ERROR_PARSER_STRING_UNTERMINATED);
    CheckError(lisp, "(",           ERROR_PARSER_LIST_UNTERMINATED);
    CheckError(lisp, "(setq } 3)",  ERROR_PARSER_INVALID_IDENTIFIER);
    CheckError(lisp, "(atom 3]",    ERROR_PARSER_BRACE_MISMATCH);
    CheckError(lisp, "(atom [3)]",  ERROR_PARSER_BRACE_MISMATCH);
    CheckError(lisp, "(quote 1 2)", ERROR_RUNTIME_WRONG_NUM_PARAMS);
    CheckError(lisp, "(atom foo)",  ERROR_RUNTIME_VARIABLE_UNBOUND);
    CheckError(lisp, "(< 1 nil)",   ERROR_RUNTIME_TYPE_MISMATCH);
}

