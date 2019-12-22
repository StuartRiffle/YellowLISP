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
            ss << ", but returned \"" << output;

        SetTextColor(ANSI_RED);
        printf("SANITY CHECK FAILED: %s\n", ss.str().c_str());
        ResetTextColor();
    }
}

void SanityCheck()
{
    InterpreterSettings settings;
    settings._repl = false;
    settings._catchExceptions = false;

    Interpreter lisp(&settings);

    CheckOutput(lisp, "",                   "");
    CheckOutput(lisp, "\n",                 "");
    CheckOutput(lisp, "1",                  "1");
    CheckOutput(lisp, " 2 ",                "2");
    CheckOutput(lisp, "3.4",                "3.4");
    CheckOutput(lisp, "-5.6",               "-5.6");
    CheckOutput(lisp, "78e-3",              "0.078");

    CheckOutput(lisp, "t",                  "t");
    CheckOutput(lisp, "nil",                "nil");
    CheckOutput(lisp, "'nil",               "nil");
    CheckOutput(lisp, "()",                 "nil");
    CheckOutput(lisp, "'()",                "nil");

    CheckOutput(lisp, "'foo",               "foo");
    CheckOutput(lisp, "'FOO",               "FOO");
    CheckOutput(lisp, "\"foo\"",            "\"foo\"");
    CheckOutput(lisp, "\"FOO\"",            "\"FOO\"");

    CheckOutput(lisp, "(atom ())",          "t");
    CheckOutput(lisp, "(atom '())",         "t");
    CheckOutput(lisp, "(atom 3)",           "t");
    CheckOutput(lisp, "(atom 'atom)",       "t");
    CheckOutput(lisp, "(atom (atom 3))",    "t");
    CheckOutput(lisp, "(atom '(atom 3))",   "nil");

    CheckError(lisp, "(", ERROR_PARSER_LIST_UNTERMINATED);
}

